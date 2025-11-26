/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : IPGEA
 * FILE       :	IPGEA.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>
#include <csr_queue.h>

#include "auto.h"
#include "csr_tag_ipgea.h"
#include "BeaconManager.h"
#include <memclass.h>


//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects et qui
// serviront à provoquer des événements sur l'automate protocolaire
//
enum{
	IPGEA_END_EVENT_INDEX = 0,
	IPGEA_IRP_EVENT_INDEX,
	IPGEA_CALLBACK_EVENT_INDEX,
	IPGEA_EVT_COUNT
} enuTEventType;


typedef enum {
	enuCALLBACK_UnknownStatus,
	enuCALLBACK_FLAG_CONNECTION,
	enuCALLBACK_FLAG_DISCONNECTION,
	enuCALLBACK_FLAG_VST,
	enuCALLBACK_FLAG_RESET,
	enuCALLBACK_FLAG_ALARM_BEACON,
	enuCALLBACK_FLAG_BEACON_OK,
	enuCALLBACK_FLAG_TIMEOUT_BEACON_OK
}enuCallbackFlagStatus;

//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et LRC).
//
#define IPGEA_IN_BUFFER_BYTES      256
#define IPGEA_OUT_BUFFER_BYTES     256
#define IPGEA_HOOK_BUFFER_BYTES    256

#define IPGEA_SLEEP_AFTER_BEACON_RESET_MS   1000


//
// Hook utilisé pour l'espionnage de la liaison
//
#define IPGEA_CALL_HOOK(ctx,size,ptr)  if ( psInst->sParams.pfHook != NULL ) IPGEATrack( psInst, ctx, size, ptr ); else

#ifdef _WITH_TRACES
	#define IPGEA_CALL_HOOK_COMMENT(x) TraceComment x;
#else
	#define IPGEA_CALL_HOOK_COMMENT(x) ;
#endif

// Définition d'un packet pour la file des messages en sortie
// et en entrée
typedef struct _IPGEA_PACKET
{
	HANDLE          hEndPacketEvent;
    DWORD           * pdwErrCode;
    DWORD           dwBufferBytes;
    BYTE            tbBuffer[255];
}
IPGEA_PACKET;


//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _IPGEA_INSTANCE
{
    // Paramètres de l'instance
    IPGEA_PARAMS          sParams;

    CRITICAL_SECTION    sSerialize;

    HMODULE             hDll;

    // Pointeurs de fonctions couches basses
	BCM_LPFN_GetLibVersion		lpfnGetLibVersion;
	BCM_LPFN_InitManagerWND		lpfnInitManagerWND;
	BCM_LPFN_InitManagerTHD		lpfnInitManagerTHD;
	BCM_LPFN_InitManagerFNC		lpfnInitManagerFNC;
	BCM_LPFN_InitManagerWND_IP	lpfnInitManagerWND_IP;
	BCM_LPFN_InitManagerTHD_IP	lpfnInitManagerTHD_IP;
	BCM_LPFN_InitManagerFNC_IP	lpfnInitManagerFNC_IP;
	BCM_LPFN_CloseManager		lpfnCloseManager;
	BCM_LPFN_ChangeMode			lpfnChangeMode;
	BCM_LPFN_StartBST			lpfnStartBST;
	BCM_LPFN_GetVST				lpfnGetVST;
	BCM_LPFN_GetUserParams		lpfnGetUserParams;
	BCM_LPFN_SendCmd			lpfnSendCmd;
	BCM_LPFN_StopBST			lpfnStopBST;
	BCM_LPFN_CheckState			lpfnCheckState;
	BCM_LPFN_Reset				lpfnReset;
	BCM_LPFN_SetConfig			lpfnSetConfig;
	BCM_LPFN_GetConfig			lpfnGetConfig;
	BCM_LPFN_GetBeaconID		lpfnGetBeaconID;


	ST_BCM_REG_PTR		myBcmRegPtr;
	ST_BCM_STATE		myState;


    // Handle du thread de gestion du protocole
    HANDLE              hThread;

	
    BOOLEAN             fLinkOpen;// Flag link status
	BOOLEAN				bIsTrxInProgress; //is transaction in progress

    HANDLE				hTimerBeaconOK;
    BOOLEAN				bTimerBeaconOKStarted;
    HANDLE				hTimerQueue;
	
	DWORD				dwPollOKTimeoutsCount;
	BOOLEAN				bIsBSTPolling; //is BST polling in progress

	BOOLEAN				bChangeBeaconID;
	BOOLEAN				bBeaconBSTCmdOK;
	BOOLEAN				bBeaconOK;

    // File d'attente pour les messages sortant
    QUEUE_INSTANCE    * psWriteQueue;

    // File d'attente pour les messages entrant
    QUEUE_INSTANCE    * psReadQueue;

    // Queue to wait for the callback signals
    QUEUE_INSTANCE    * psCallbackQueue;

    // Indicateur que le prochain message est le
    // dernier d'une séquence.
    BOOL                fLastMessage;

	//Event signaled when IPGEAClose is called which is closing the interface instance...
    HANDLE              hEndEvent;


    // Tableau des handles signalables servant à animer l'automate
    HANDLE              thEvents[IPGEA_EVT_COUNT];

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // packet emitted from L7 level (includes interface requests)
    IPGEA_PACKET        * psCurrentL7Packet;

    // Message from application level that possibly contains the L2 level packet to be emitted to the beacon 
  	IPGEAL7_API			sCurrentDecodedOutL7Message;

    // Taille occupée dans le buffer de réception
    DWORD               dwInBytes;

    // Buffer de réception
    BYTE                tbIn[ IPGEA_IN_BUFFER_BYTES ];

    // Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ IPGEA_HOOK_BUFFER_BYTES ];


}
IPGEA_INSTANCE;


//
// Autres fonctions privées
//
PRIVATE void WINAPI IPGEAMakeError( IPGEA_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE void WINAPI IPGEAHandleIRP( IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEARunThread( IN IPGEA_INSTANCE * psInst );
PRIVATE void WINAPI IPGEATerminateCurrentPacket( IPGEA_INSTANCE * psInst, DWORD dwErr );
PRIVATE void WINAPI IPGEATrack( IPGEA_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );
PRIVATE DWORD WINAPI IPGEAVerifyOpenPort( IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEATranslateError( int iL7Status );
PRIVATE	BOOL GetLibProc(IN IPGEA_INSTANCE   * psInst);
PRIVATE DWORD WINAPI IPGEAProcessL2Message(IN IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEAProcessModeTransparentReq(IN IPGEA_INSTANCE * psInst);
PRIVATE DWORD WINAPI IPGEAProcessStatusReq(IN IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEAEncodeAndReplyWithL7Packet(IN IPGEA_INSTANCE * psInst, IPGEAL7_API * psNewInputL7Message);
PRIVATE void WINAPI IPGEAHandleVSTEvent( IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEAProcessL2_BST_Message(IN IPGEA_INSTANCE * psInst );
PRIVATE DWORD WINAPI IPGEATerminateSession(IN IPGEA_INSTANCE * psInst);
PRIVATE DWORD WINAPI IPGEAProcessL2EmptyReq(IN IPGEA_INSTANCE * psInst);
PRIVATE void TraceConsole(IPGEA_INSTANCE * pInst, char * szStrToShow, ...);


PRIVATE DWORD EnqueueCALLBACK_PACKET(  IPGEA_INSTANCE   * psInst,
								enuCallbackFlagStatus eCallbackStatus);
PRIVATE BOOL UnqueueCALLBACK_PACKET(  IPGEA_INSTANCE   * psInst,
							  enuCallbackFlagStatus* peCallbackStatus);

PRIVATE void WINAPI StopBeaconOKEventTimer(IN IPGEA_INSTANCE * psInst);
PRIVATE void WINAPI ResetBeaconOKEventTimer(IN IPGEA_INSTANCE * psInst);

static void BCM_EXPORT Callback( ST_BCM_REG_PTR argBcmRegPtr, BCM_CALLBACK argTypeCB, DWORD argParam );
static void BCM_EXPORT Alarm( ST_BCM_REG_PTR argBcmRegPtr, BCM_ALARMS argAlarm, DWORD argState );
//-------------------------------------------------------------------
//Instance handling to be able to get instance data in callback functions
//-------------------------------------------------------------------

CRITICAL_SECTION    gcsGlobalInstanceProtection;

typedef struct _GLOBAL_IPGEA_LST_INSTANCE_ELEMENT
{
	void * pNextEl;
	IPGEA_INSTANCE * pCurrentInstance;

}GLOBAL_IPGEA_LST_INSTANCE_ELEMENT;


GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * gpGlobalInstListHeadEl = NULL;


EXPORT void WINAPI IPGEAInitGlobalInstanceHandling()
{
   InitializeCriticalSection(&gcsGlobalInstanceProtection);
}


EXPORT void WINAPI IPGEADeinitGlobalInstanceHandling()
{
   DeleteCriticalSection(&gcsGlobalInstanceProtection);
}
	
PRIVATE DWORD WINAPI IPGEAAddNewInstance(IPGEA_INSTANCE * pInst)
{
    DWORD               dwErr       = NO_ERROR;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT ** pNewTargetElement;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pNewElement;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pCurrentEl;

	__try
    {
		pCurrentEl = gpGlobalInstListHeadEl;

		if(pCurrentEl!=NULL)	
		{
			//if the head element is not null
			while(pCurrentEl->pNextEl != NULL)
			{
				pCurrentEl = pCurrentEl->pNextEl;
			}

			pNewTargetElement = &((GLOBAL_IPGEA_LST_INSTANCE_ELEMENT*)gpGlobalInstListHeadEl->pNextEl);
		}
		else
		{ 
			pNewTargetElement = &gpGlobalInstListHeadEl;
		}

 		pNewElement = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GLOBAL_IPGEA_LST_INSTANCE_ELEMENT) );
		if ( pNewElement == NULL ) 
		{
			dwErr = ERROR_NOT_ENOUGH_MEMORY;
			__leave;
		}

		pNewElement->pCurrentInstance = pInst;
		*pNewTargetElement = pNewElement;
	
	}
	__finally
    {

    }
	
	return dwErr;
}	


void TraceComment(IPGEA_INSTANCE * pInst, char * szStrToShow, ...)
{
	int iSize;
	va_list args;
	char szOutputBuffer[4096];
	char szTimestamp[256];
	int iTimestampSize;
	
	if(pInst!= NULL)
		if ( pInst->sParams.pfHook != NULL ) 	
		{		
			sprintf_s(szTimestamp, sizeof(szTimestamp), "%d", timeGetTime());
			szOutputBuffer[0]='\0';
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), "\n");
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), szTimestamp);
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), " -");
			iTimestampSize = (int)strlen(szOutputBuffer);

			va_start(args, szStrToShow );     
			iSize = _vsnprintf_s(szOutputBuffer + iTimestampSize, (sizeof(szOutputBuffer) - iTimestampSize), sizeof(szOutputBuffer) - iTimestampSize, szStrToShow, args);
			va_end(args);
			

			if(iSize>0 && iSize < sizeof(szOutputBuffer) - iTimestampSize )
			{
				IPGEATrack( pInst, IPGEA_HOOK_COMMENT | IPGEA_HOOK_FLUSH, iSize + iTimestampSize, szOutputBuffer);
			}
		}
}


PRIVATE BOOL WINAPI IPGEARemoveInstance(IPGEA_INSTANCE * pInst)
{
    DWORD               dwErr       = NO_ERROR;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pPreviousElement;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pNextElement;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pCurrentEl;
	BOOL  bFound = FALSE;

	
		pCurrentEl = gpGlobalInstListHeadEl;

		if(gpGlobalInstListHeadEl!=NULL)	
		{
			if(gpGlobalInstListHeadEl->pCurrentInstance == pInst) //remove head
			{
				pNextElement = gpGlobalInstListHeadEl->pNextEl;
				
				HeapFree( GetProcessHeap(), 0, gpGlobalInstListHeadEl );
				
				gpGlobalInstListHeadEl = pNextElement;
				bFound = TRUE;
			}
			else
			{
				//if the head element is not the one
				while(pCurrentEl->pNextEl != NULL && bFound==FALSE)
				{
					pPreviousElement = pCurrentEl;
					pCurrentEl = pCurrentEl->pNextEl; //move the pointer to the next in the list
					
					if(pCurrentEl->pCurrentInstance == pInst)
					{	
						pPreviousElement->pNextEl = pCurrentEl->pNextEl; //connect previous and next element (even if the next is NULL)
						HeapFree( GetProcessHeap(), 0, pCurrentEl );
						bFound = TRUE;
						break;
					}
				}

			}

		}


	return bFound;
}	


PRIVATE IPGEA_INSTANCE * WINAPI IPGEAGetInstanceFor(ST_BCM_REG_PTR	myBcmRegPtr)
{
	IPGEA_INSTANCE * pFoundInstance = NULL;
	GLOBAL_IPGEA_LST_INSTANCE_ELEMENT * pCurrentEl;

	__try
    {
		EnterCriticalSection(&gcsGlobalInstanceProtection);
		
		pCurrentEl = gpGlobalInstListHeadEl;

		while(pFoundInstance == NULL &&  pCurrentEl != NULL)
		{
			if(pCurrentEl->pCurrentInstance->myBcmRegPtr == myBcmRegPtr)
			{
				pFoundInstance = pCurrentEl->pCurrentInstance;
				break;
			}
			
			pCurrentEl = pCurrentEl->pNextEl; //move the pointer to the next in the list
		}

	}
	__finally
    {
		LeaveCriticalSection(&gcsGlobalInstanceProtection);
    }

	return pFoundInstance;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

VOID CALLBACK TimerCallbackRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL)
    {
        //printf("TimerRoutine lpParam is NULL\n");
    }
    else
    {
		IPGEA_INSTANCE * psInst = (IPGEA_INSTANCE *)lpParam;

        // lpParam points to the argument; in this case it is an int
        if(TimerOrWaitFired)
        {
			IPGEA_CALL_HOOK_COMMENT((psInst, "Callback TimerCallbackRoutine: enuCALLBACK_FLAG_TIMEOUT_BEACON_OK ... "));        
			EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_TIMEOUT_BEACON_OK);
        }
		else
        {
            //printf("The wait event was signaled.\n");
        }
    }

}


PRIVATE void WINAPI StopBeaconOKEventTimer(IN IPGEA_INSTANCE * psInst)
{
	if(psInst->bTimerBeaconOKStarted)
	{
		DeleteTimerQueueTimer( psInst->hTimerQueue, psInst->hTimerBeaconOK, NULL);
		psInst->bTimerBeaconOKStarted = FALSE;
	}
	
}

//Will start the Beacon OK event timer if not started
//otherwise restart the timer
PRIVATE void WINAPI ResetBeaconOKEventTimer(IN IPGEA_INSTANCE * psInst)
{
	if(psInst->bTimerBeaconOKStarted)
		DeleteTimerQueueTimer( psInst->hTimerQueue, psInst->hTimerBeaconOK, NULL);

	CreateTimerQueueTimer( &psInst->hTimerBeaconOK, psInst->hTimerQueue, 
	            (WAITORTIMERCALLBACK)TimerCallbackRoutine, psInst , psInst->sParams.dwCheckPoolingPeriodTimeout, 0, WT_EXECUTEDEFAULT);
	psInst->bTimerBeaconOKStarted = TRUE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI IPGEAOpen( 
 *                      OUT IPGEA_INSTANCE  ** ppsInst,
 *                      IN  IPGEA_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication IPGEA
 *             (utilisé pour la communication HOTE<->BADGE par liaison filaire)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI IPGEAOpen( 
        OUT IPGEA_INSTANCE  ** ppsInst,
        IN  IPGEA_PARAMS     * psParams )
{
    DWORD               dwErr       = NO_ERROR;
    IPGEA_INSTANCE      * psInst      = NULL;
    DWORD               dwId;
	BCM_ERR				result;


	__try
    {
		EnterCriticalSection(&gcsGlobalInstanceProtection);

        //
        // Valeur par défaut retournée : NULL
        //
        (*ppsInst) = NULL;

        //
        // Allouer la place pour la structure d'instance et la remplir de 0
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IPGEA_INSTANCE) );
        if ( psInst == NULL ) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

		IPGEAAddNewInstance(psInst);

        InitializeCriticalSection( &psInst->sSerialize );

        // Reporter les valeurs des paramètres une fois qu'on s'est assuré que la
        // taille de la structure est compatible.
        if ( psParams->dwStructBytes != sizeof(*psParams) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        psInst->sParams = (*psParams);

		// Load the BeaconManager.Dll
		if ( !GetLibProc(psInst) )
        {
            dwErr = GetLastError();
            __leave;
        }

		//Create thread - callback  communication events
        psInst->hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psInst->hEndEvent == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

		psInst->hTimerQueue = CreateTimerQueue();
		if (psInst->hTimerQueue == NULL)
		{
            dwErr = GetLastError();
            __leave;
		}

        // Initialisation des files d'attente
        dwErr = QueueOpen( &psInst->psReadQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;
        dwErr = QueueOpen( &psInst->psWriteQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;
        dwErr = QueueOpen( &psInst->psCallbackQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;
		
		//Initialize the Beacon OK event timeout to 3 times of the configured dwCheckPoolingPeriod,
		// since GEA dll actually send the event 50% milliseconds latter then initialized and  
		// to avoid possible problems of forced Beacon reset because of wrongly assumed beacon status 
		//It was noticed that sometimes the OK event is late more than 2X if BST polling is in progress
 		psInst->sParams.dwCheckPoolingPeriodTimeout = psInst->sParams.dwCheckPoolingPeriod*3;

       // Initialisation de la liaison
		result = psInst->lpfnInitManagerFNC_IP( &psInst->myBcmRegPtr, 0, NULL, psInst->sParams.szIpAdress, (WORD)psInst->sParams.dwPort, BCM_Secondary,
														psInst->sParams.dwCheckPoolingPeriod, TRUE, Callback, Alarm );

		psInst->fLinkOpen = FALSE;
		psInst->bBeaconOK = FALSE;
		psInst->bTimerBeaconOKStarted = FALSE;

		if ( result != BCM_NoErr )
		{
            dwErr = IPGEATranslateError( result );
            __leave;
        }


        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        psInst->thEvents[IPGEA_END_EVENT_INDEX] = psInst->hEndEvent;
        psInst->thEvents[IPGEA_IRP_EVENT_INDEX] = QueueGetWaitableHandle( psInst->psWriteQueue );
		psInst->thEvents[IPGEA_CALLBACK_EVENT_INDEX] = QueueGetWaitableHandle( psInst->psCallbackQueue );

        // Lancement du thread d'animation du protocole
        psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)IPGEARunThread, (void*)psInst, 0, &dwId, "TAG_IPGEA_PROTOCOL" );
        if ( psInst->hThread == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        // Pour améliorer la réactivité globale, on diminue le délai de préemption
        // à une milliseconde.
        timeBeginPeriod( 1 );

        //
        // Mettre à jour les données à retourner
        (*ppsInst) = psInst;
        psInst = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
		LeaveCriticalSection(&gcsGlobalInstanceProtection);

        if ( psInst != NULL )
            IPGEAClose( psInst, 0 );

        
    }

	return dwErr;
}














/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI IPGEAClose( 
 *                      IN IPGEA_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par IPGEAOpen
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             IPGEAOpen(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI IPGEAClose( 
        IN IPGEA_INSTANCE   * psInst,
        IN DWORD            dwTimeout )
{
	BCM_ERR	result = BCM_NoErr;

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAClose..."));   

 	__try
    {
		EnterCriticalSection(&gcsGlobalInstanceProtection);

		if ( psInst != NULL )
		{

			if ( psInst->hThread != NULL )
			{
				timeEndPeriod( 1 );
				SetEvent( psInst->hEndEvent );
				WaitForSingleObject( psInst->hThread, dwTimeout );
				TerminateThread( psInst->hThread, NO_ERROR );
				CloseHandle( psInst->hThread );
				IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAClose...Thread ended!"));   
			}

			if ( psInst->psReadQueue != NULL )
				QueueClose( psInst->psReadQueue );
			if ( psInst->psWriteQueue != NULL )
				QueueClose( psInst->psWriteQueue );
			if ( psInst->psWriteQueue != NULL )
				QueueClose( psInst->psCallbackQueue );
        
			if ( psInst->fLinkOpen)
			 		psInst->lpfnChangeMode(psInst->myBcmRegPtr, BCM_MOD_Stopped);

			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAClose...lpfnChangeMode ended!"));   

			// Terminate the BeaconManager
			result = psInst->lpfnCloseManager(&psInst->myBcmRegPtr);
				
			
			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAClose...lpfnCloseManager ended!"));   
			
			if ( result != BCM_NoErr )
			{
				//to do trace error ... ( "Fail to terminate the BeaconManager (Err : %d)\n", result );
			}

			if ( psInst->hEndEvent != NULL )
				CloseHandle( psInst->hEndEvent );


			if ( psInst->hDll != NULL )
			{
				FreeLibrary( psInst->hDll );
				psInst->hDll = NULL;
			}
			
			DeleteTimerQueue(psInst->hTimerQueue);

			DeleteCriticalSection( &psInst->sSerialize );
			
			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAClose...ENDED"));   

			IPGEARemoveInstance(psInst);
			HeapFree( GetProcessHeap(), 0, psInst );
		}

    }
    __finally
    {
		LeaveCriticalSection(&gcsGlobalInstanceProtection);
    }


}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI IPGEASendMessage( 
 *                      IN              IPGEA_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             pbMsg       : Pointe sun un buffer contenant les données à émettre.
 *             dwMsgBytes  : Taille des données à émettre
 *             hEvent      : Handle d'événement à signaler lorsque le message a été réellement envoyé.
 *                           Ce paramètre peut être NULL.
 *             pdwErr      : Pointe sur un DWORD qui reçoit 0xFFFFFFFF lors de la sortie de
 *                           la fonction et qui est mis à jours lorsque l'émission est terminée.
 *                           Si la valeur est alors NO_ERROR, l'émission a réussie, sinon, elle
 *                           a échoué.
 *                           Ce paramètre peut être NULL.
 * RETURN    : NO_ERROR si le message a été placé dans la file d'attente d'émission.
 *             Sinon, un code d'erreur standard.
 * --------------------------------------------------------------------
 * ROLE      : Place un message dans la file d'attente d'émission de l'instance.
 *             Lorsque le message est effectivement envoyé, l'événement (s'il est fourni) est
 *             signalé et le résultat (si pdwErr n'est pas NULL) est NO_ERROR.
 *             Si le message ne peut être envoyé, l'événement (s'il est fourni) est
 *             signalé et le résultat (si pdwErr n'est pas NULL) est une erreur standard.
 *             Lorsque la file d'attente d'émission est pleine, la fonction bloque jusqu'à ce
 *             que le message ait pu y être placé.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI IPGEASendMessage( 
        IN              IPGEA_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    IPGEA_PACKET    * psPacket = NULL;

    __try
    {
        //
        // Mise à jour immédiate du code de complétion à 0xFFFFFFFF
        //
        if ( pdwErr != NULL )
            (*pdwErr) = 0xFFFFFFFF;


        // Prepare un "packet" contenant le message à émettre
        //
        psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + dwMsgBytes + 5);
        if ( psPacket == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

		//TO DO missing information on type of message
        psPacket->dwBufferBytes = dwMsgBytes;
        psPacket->pdwErrCode = pdwErr;
        psPacket->hEndPacketEvent = hEvent;

        CopyMemory( psPacket->tbBuffer, pbMsg, dwMsgBytes );

        // Placer le "packet" en file d'attente (attente infinie en cas de saturation
        // de la file).
        //
        dwErr = QueueWriteItem( psInst->psWriteQueue, psPacket, INFINITE );
        if ( dwErr != NO_ERROR )
            __leave;

        psPacket = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psPacket != NULL )
            HeapFree( GetProcessHeap(), 0, psPacket );

        
    }

	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI IPGEAGetWaitableHandle(
 *                      IN IPGEA_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par IPGEAOpen
 * RETURN    : Un handle sur lequel un opération d'attente peut être effectuée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle permettant d'éffectuer des réceptions asynchrones.
 *             Ce handle est signalé lorsqu'au moins un message est dans la file
 *             de réception. Il est automatiquement réinitialisé lorsque le file
 *             est vide.
 *             Ce handle peut être utilisé avec les fonctions WaitForXXX de l'API Win32.
 *             Attention, ne pas essayer de réinitialiser ce handle.
 * --------------------------------------------------------------------
 */
EXPORT HANDLE WINAPI IPGEAGetWaitableHandle(
        IN IPGEA_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI IPGEAGetCommInstance(
 *                      IN IPGEA_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par IPGEAOpen
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI IPGEAGetCommInstance(
        IN IPGEA_INSTANCE * psInst )
{
    return NULL;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI IPGEAReceiveMessage(
 *                      IN      IPGEA_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             ppbMsg      : Retourne un pointeur sur le buffer où seront stockées
 *                           les données reçues.
 *             pdwMsgBytes : Retourne la taille des données reçues
 *             dwTimeout   : Si aucun message n'est présent dans la file d'attente, la fonction
 *                           attent un message pendant ce délai (en ms).
 * RETURN    : NO_ERROR si un message a été récu ou en cas de dépassement de délai.
 *             Sinon, un code d'erreur standard.
 * --------------------------------------------------------------------
 * ROLE      : Récupère le message suivant dans la file de réception. Si le délai imparti
 *             est dépassé, la fonction retourne sans erreur et avec (*pbMsg) = NULL
 *             et (*pdwMsgBytes) = 0.
 *             Si un message a été recu, le buffer doit être désalloué par un appel
 *             à IPGEAFreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI IPGEAReceiveMessage(
        IN      IPGEA_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    IPGEA_PACKET    * psPacket;

    __try
    {
        //
        // Initialiser avec les valeurs par défaut (aucun message en attente).
        //
        (*ppbMsg) = NULL;
        (*pdwMsgBytes) = 0;

        //
        // Extraire un élément de la file d'attente
        //
        dwErr = QueueReadItem( psInst->psReadQueue, &psPacket, dwTimeout );
        if ( dwErr != NO_ERROR )
        {
            if ( dwErr == WAIT_TIMEOUT )
                dwErr = NO_ERROR;
            __leave;
        }

        //
        // Mise à jour de la valeur de retour
        //
        (*ppbMsg) = psPacket->tbBuffer;
        (*pdwMsgBytes) = psPacket->dwBufferBytes;
    }
    __finally
    {
		;
    }

	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI IPGEAFreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par IPGEAReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à IPGEAReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI IPGEAFreeMessage(
        IN BYTE * pbMsg )
{
    IPGEA_PACKET * psPacket = NULL;

    //
    // Réajustement de la position pointeur
    //
    pbMsg -= (DWORD)(&psPacket->tbBuffer[0]);

    //
    // Libération du bloc
    //
    HeapFree( GetProcessHeap(), 0, pbMsg );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI IPGEASetOption(
 *                      IN IPGEA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour IPGEA)
 *             dwValue     : Valeur de l'option (si entier) ou taille
 *                           pointée par pvValue
 *             pvValue     : Si non entier, pointe sur la valeur de
 *                           l'option, la taille du bloc pointé étant
 *                           donnée par dwValue.
 * RETURN    : NO_ERROR si l'option existe et a été mise à jour correctement.
 * --------------------------------------------------------------------
 * ROLE      : Change une option de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI WINAPI IPGEASetOption(
        IN IPGEA_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue )
{
    DWORD dwErr = NO_ERROR;

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );


        if ( dwOptionId == IPGEA_OPTION_F_LAST_MESSAGE )
        {
			psInst->fLastMessage = (dwValue!=0);
		}
		else if ( dwOptionId == IPGEA_OPTION_DW_BEACON_ID )
        {
			psInst->bChangeBeaconID = (dwValue!=0);
        }
        else
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );

        
    }	

	return dwErr;
	
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI IPGEAGetOption(
 *                      IN IPGEA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour IPGEA)
 *             pdwValue    : Si pvValue est NULL, récupère la valeur de l'option
 *                           lorsque celle-ci est entière.
 *                           Sinon, en entrée : taille du bloc pointé par pvValue,
 *                           en sortie : taille de l'option écrite dans pvValue.
 *             pvValue     : Si non NULL, pointe sur le buffer récupérant l'option.
 * RETURN    : NO_ERROR si l'option existe et a été lue correctement.
 * --------------------------------------------------------------------
 * ROLE      : Lit une option de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI WINAPI IPGEAGetOption(
        IN IPGEA_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
		IN void         * pvValue,
		IN size_t		  pvValueSize)
{
    int   iL7Status;
    DWORD dwErr = NO_ERROR;
    char  szAnyStr[255+1];
	ST_BCM_CONFIG	bcmConfig;


    __try
    {
        EnterCriticalSection( &psInst->sSerialize );


        if ( ! psInst->fLinkOpen )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

       if ( dwOptionId == IPGEA_OPTION_F_LAST_MESSAGE )
        {
            if ( pvValue != NULL )
            {
                if      ( *pdwValue == 1 )
                {
                    *(BYTE*)pvValue = ( psInst->fLastMessage ? 1 : 0 );
                }
                else if ( *pdwValue == 2 )
                {
                    *(WORD*)pvValue = ( psInst->fLastMessage ? 1 : 0 );
                }
                else if ( *pdwValue > 4 )
                {
                    *(DWORD*)pvValue  = ( psInst->fLastMessage ? 1 : 0 );
                    *pdwValue = 4;
                }
                else
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
            }
            else
                *pdwValue = ( psInst->fLastMessage ? 1 : 0 );
        }

        else if ( dwOptionId == IPGEA_OPTION_SZ_VERSION )
        {
            if ( pvValue == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            if ( *pdwValue == 0 )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            iL7Status = psInst->lpfnGetConfig(psInst->myBcmRegPtr,  &bcmConfig);
            dwErr = IPGEATranslateError( iL7Status );
            if ( dwErr != NO_ERROR )
            {
                __leave;
            }

            _snprintf_s( (char*)pvValue, pvValueSize, *pdwValue, "%s", bcmConfig.version);

            ((char*)pvValue)[(*pdwValue)-1] = 0;
			*pdwValue = (DWORD)strlen((char*)pvValue) + 1;
        }

        else if ( dwOptionId == IPGEA_OPTION_DW_BEACON_ID )
        {
            if ( pvValue != NULL )
            {
                if ( *pdwValue < 4 )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
            }

            iL7Status = psInst->lpfnGetBeaconID(psInst->myBcmRegPtr, szAnyStr );
            dwErr = IPGEATranslateError( iL7Status );
            if ( dwErr != NO_ERROR )
            {
                __leave;
            }

            szAnyStr[6] = 0;
            if ( pvValue == NULL )
                *pdwValue = atol( szAnyStr );
            else
            {
                *(DWORD*)pvValue = atol( szAnyStr );
                *pdwValue = 4;
            }
        }

        else
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );

        
    }

	return dwErr;
}




PRIVATE void WINAPI IPGEAMakeError( IPGEA_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    IPGEATerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
	strcpy_s(psInst->szLastError, sizeof(psInst->szLastError), pcText);
}



PRIVATE void WINAPI IPGEAHandleConnectionEvent( IPGEA_INSTANCE * psInst , BOOL bConnected)
{

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleConnectionEvent: CONNECTED:%d", bConnected ));   
	
	__try
    {
         EnterCriticalSection( &psInst->sSerialize );
			
		 psInst->fLinkOpen = bConnected;

		 if(bConnected)
			 ResetBeaconOKEventTimer(psInst);
		 else
			 StopBeaconOKEventTimer(psInst);

		 psInst->bIsBSTPolling = FALSE;
		 psInst->bBeaconBSTCmdOK = FALSE;
		 psInst->bIsTrxInProgress = FALSE;
		 psInst->dwPollOKTimeoutsCount = 0;
		 psInst->bBeaconOK = FALSE;
    }
    __finally
    {
         LeaveCriticalSection( &psInst->sSerialize );
	}
}


PRIVATE DWORD getTimeDiffMilisecs(DWORD dwTIME1, DWORD dwTIME2)
{
	DWORD dwRetVal;

	if(dwTIME1 < dwTIME2)
	{
		dwRetVal = (0xFFFFFFFF - dwTIME2) + dwTIME1;
	}
	else
	{
		dwRetVal = dwTIME1 - dwTIME2;
	}
			
	
	return dwRetVal;
} 


PRIVATE void WINAPI IPGEAHandleAlarmBeaconEvent( IPGEA_INSTANCE * psInst )
{
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleAlarmBeaconEvent..."));   
	
	__try
    {
         EnterCriticalSection( &psInst->sSerialize );
			
		 psInst->bIsBSTPolling = FALSE;
		 psInst->bBeaconBSTCmdOK = FALSE;
		 psInst->bBeaconOK = FALSE;	
	}
    __finally
    {
         LeaveCriticalSection( &psInst->sSerialize );
	}
}


PRIVATE void WINAPI IPGEAHandleBeaconOKEvent( IPGEA_INSTANCE * psInst )
{
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleBeaconOKEvent... "));   
	
	__try
    {
         EnterCriticalSection( &psInst->sSerialize );
			
		 psInst->bBeaconOK = TRUE;
		 ResetBeaconOKEventTimer(psInst);
		 psInst->dwPollOKTimeoutsCount = 0;
	}
    __finally
    {
         LeaveCriticalSection( &psInst->sSerialize );
	}
}


PRIVATE void WINAPI IPGEAHandleBeaconOKEventTimeout( IPGEA_INSTANCE * psInst )
{
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleBeaconOKEventTimeout..."));   
	
	__try
    {
         EnterCriticalSection( &psInst->sSerialize );
			
		 psInst->bIsBSTPolling = FALSE;
		 psInst->bBeaconBSTCmdOK = FALSE;
		 psInst->bBeaconOK = FALSE;
		 ResetBeaconOKEventTimer(psInst);
		 psInst->dwPollOKTimeoutsCount++;
		 
		 if(psInst->dwPollOKTimeoutsCount == psInst->sParams.dwResetAfterTimeouts ) 
		 {
	  		 int iLastBCMErr;
			 
			 psInst->dwPollOKTimeoutsCount = 0;

			 iLastBCMErr = psInst->lpfnReset(psInst->myBcmRegPtr);
			 IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleBeaconOKEventTimeout:Reset:%d", iLastBCMErr));
			 
			 //NOTE: The beacon should not receive any command while reseting otherwise it might stop communicate
		 }
			
	}
    __finally
    {
         LeaveCriticalSection( &psInst->sSerialize );
	}
}


PRIVATE void WINAPI IPGEAHandleResetEvent( IPGEA_INSTANCE * psInst )
{
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleResetEvent..."));   
	
	__try
    {
         EnterCriticalSection( &psInst->sSerialize );
			
		 psInst->bIsBSTPolling = FALSE;
		 psInst->bBeaconBSTCmdOK = FALSE;
		 psInst->bIsTrxInProgress = FALSE;
		 psInst->dwPollOKTimeoutsCount = 0;
		 psInst->bBeaconOK = FALSE;
    }
    __finally
    {
         LeaveCriticalSection( &psInst->sSerialize );
	}
}


PRIVATE void WINAPI IPGEAHandleVSTEvent( IPGEA_INSTANCE * psInst )
{
    DWORD        dwErr;
	BCM_ERR		 iLastBCMErr= BCM_NoErr;
	IPGEAL7_API	 sNewInputL7Message;

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleVSTEvent..."));   

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

		psInst->bIsBSTPolling = FALSE;
		psInst->bBeaconBSTCmdOK = FALSE;
		psInst->bIsTrxInProgress = TRUE;

 		// Get the VST
		iLastBCMErr = psInst->lpfnGetVST( psInst->myBcmRegPtr, psInst->tbIn, &psInst->dwInBytes, sizeof( psInst->tbIn ));
		
		dwErr = IPGEATranslateError(iLastBCMErr);
		if(dwErr == NO_ERROR)
		{
			sNewInputL7Message.bMessageType = GEAIP_SEND_L2_FRAME;
			sNewInputL7Message.dwNumL2Bytes = psInst->dwInBytes;
			CopyMemory( sNewInputL7Message.tbL2Bytes, psInst->tbIn, sNewInputL7Message.dwNumL2Bytes);

            IPGEA_CALL_HOOK( IPGEA_HOOK_INPUT | IPGEA_HOOK_DATA | IPGEA_HOOK_FLUSH,
                psInst->dwInBytes, 
                psInst->tbIn );

			dwErr = IPGEAEncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
		}
		else
		{
			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAHandleVSTEvent: Error lpfnGetVST returned: %d", iLastBCMErr));   

		}
	
	}
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }
}


PRIVATE void WINAPI IPGEAHandleIRP( IPGEA_INSTANCE * psInst )
{
    DWORD        dwErr, dwPos, dwLen;
	BOOL		 fResult;
    IPGEA_PACKET * psL7Packet;


    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( psInst->psCurrentL7Packet == NULL )
        {
            // Si le message n'a pas encore été construit, on l'extrait de la file d'attente
            dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentL7Packet, 0 );
            if ( dwErr != NO_ERROR )
            {
                IPGEAMakeError( psInst, dwErr, "IPGEAHandleIRP : QueueReadItem" );
                __leave;
            }
        }

        // Dans le mode où les buffers sont vidés automatiquement,
        // on vide la file de réception chaque fois qu'on émet un
        // nouveau message (mode question/réponse).
        if ( psInst->sParams.dwAutoClearBuffers != 0 )
        {
            while ( ! QueueIsEmpty( psInst->psReadQueue ) )
            {
                dwErr = QueueReadItem( psInst->psReadQueue, &psL7Packet, 0 );
                if ( dwErr == NO_ERROR )
                    HeapFree( GetProcessHeap(), 0, psL7Packet );
            }
        }


		//if no link to the device everything fails
        if ( ! psInst->fLinkOpen || !psInst->bBeaconOK)
        {
            IPGEAMakeError( psInst, ERROR_DEVICE_NOT_CONNECTED, "IPGEAHandleIRP : ..." );
            __leave;
        }

		//Decode the message sent from tag.c to identify message type and perform accordingly
	    dwLen = psInst->psCurrentL7Packet->dwBufferBytes;
        dwPos = 0;

        fResult = IPGEAL7_DecodeMessage(
							psInst,
							psInst->psCurrentL7Packet->tbBuffer,
							&dwPos,
							&dwLen,
							&psInst->sCurrentDecodedOutL7Message );
        if ( ! fResult )
        {
			IPGEAMakeError( psInst, ERROR_DEVICE_NOT_CONNECTED, "IPGEAHandleIRP : ..." );
            __leave;
        }
				
		switch(psInst->sCurrentDecodedOutL7Message.bMessageType)
		{
			case GEAIP_INTF_REQ_ABORT:
				dwErr = IPGEATerminateSession(psInst);
				break;
			case GEAIP_INTF_REQ_GETSTATUS:
				dwErr = IPGEAProcessStatusReq(psInst);
				break;
			case GEAIP_INTF_REQ_MODE_TRANSPARENT:
				dwErr = IPGEAProcessModeTransparentReq(psInst);
				break;
			case GEAIP_SEND_L2_FRAME:
				dwErr = IPGEAProcessL2Message(psInst);
				break;
			case GEAIP_SEND_L2_FRAME_BST:
				dwErr = IPGEAProcessL2_BST_Message(psInst);
				break;
			case GEAIP_SEND_L2_FRAME_END:
				dwErr = IPGEAProcessL2EmptyReq(psInst);
				break;
		}
		
	    if ( dwErr != NO_ERROR )
		{	
			IPGEAMakeError( psInst, dwErr, "IPGEAHandleIRP : ..." );
		}
		else
		{
			IPGEATerminateCurrentPacket( psInst, NO_ERROR );
		}

    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }
}


PRIVATE DWORD WINAPI IPGEAProcessL2EmptyReq(IN IPGEA_INSTANCE * psInst )
{
	ST_BCM_STATE	myState;
	BYTE	buffer[256];
	DWORD	lgBuffer,dwErr = NO_ERROR;
	BCM_ERR	iLastBCMErr = BCM_NoErr;
	BYTE	cmdEcho[9] = {0x91,0x05,0x00,0x0F,0x02,0x03,0xAA,0x55,0xAA};

 	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq..."));   

	iLastBCMErr = psInst->lpfnCheckState( psInst->myBcmRegPtr, &myState );
	if ( iLastBCMErr == BCM_NoErr )
	{
		IPGEA_CALL_HOOK_COMMENT((psInst,"IPGEAProcessL2EmptyReq... state:%d; mode:%d; trxProgress:%d", 
					myState.state, myState.mode, myState.trxInProgress));

		// If a transaction is in progress send an echo to close the connection (on timeout)
		if (myState.trxInProgress == 1 && myState.mode == BCM_MOD_Transparent)
		{
 				IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq: Send an echo.Req .."));   
				iLastBCMErr = psInst->lpfnSendCmd( psInst->myBcmRegPtr, cmdEcho, sizeof(cmdEcho),
													buffer, &lgBuffer, sizeof(buffer), TRUE );
				psInst->bIsTrxInProgress  = FALSE;
				dwErr = IPGEATranslateError(iLastBCMErr);
		}
		else
		{
			psInst->bBeaconOK = TRUE;
			ResetBeaconOKEventTimer(psInst);
			
			if(psInst->bIsBSTPolling)
			{
 				IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq: Stop BST.."));   
				iLastBCMErr = psInst->lpfnStopBST( psInst->myBcmRegPtr );
				
				if(iLastBCMErr == BCM_NoErr)
				{
					psInst->bIsBSTPolling = FALSE;
					psInst->bBeaconBSTCmdOK = TRUE;
					dwErr = NO_ERROR;
				}
				else
				{
					IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq: Error lpfnStopBST: %d", iLastBCMErr));   
					dwErr = ERROR_NOT_READY;
				}
	
			}
			else
			{
				IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq: Error: %d", iLastBCMErr));   
				dwErr = NO_ERROR;
			}
		}

	}
	else
	{
		IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2EmptyReq: Error lpfnCheckState: %d", iLastBCMErr));   
		dwErr = IPGEATranslateError(iLastBCMErr);
	}


	return dwErr;
}


PRIVATE DWORD WINAPI IPGEATerminateSession(IN IPGEA_INSTANCE * psInst )
{
	ST_BCM_STATE	myState;
	BYTE	buffer[256];
	DWORD	lgBuffer,dwErr = NO_ERROR;
	BCM_ERR	iLastBCMErr = BCM_NoErr;
	BYTE	cmdEcho[9] = {0x91,0x05,0x00,0x0F,0x02,0x03,0xAA,0x55,0xAA};

 	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEATerminateSession..."));   

	iLastBCMErr = psInst->lpfnCheckState( psInst->myBcmRegPtr, &myState );
	if ( iLastBCMErr == BCM_NoErr )
	{
		IPGEA_CALL_HOOK_COMMENT((psInst,"IPGEATerminateSession... state:%d; mode:%d; trxProgress:%d", 
					myState.state, myState.mode, myState.trxInProgress));
		// If the emission BST is activated
		if(myState.trxInProgress == 0 && myState.mode == BCM_MOD_Transparent && psInst->bIsBSTPolling)
		{
 			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEATerminateSession: Stop BST.."));   
			iLastBCMErr = psInst->lpfnStopBST( psInst->myBcmRegPtr );
			
			if(iLastBCMErr == BCM_NoErr)
			{
				psInst->bIsBSTPolling = FALSE;
				psInst->bBeaconBSTCmdOK = TRUE;
				psInst->bBeaconOK = TRUE;
				ResetBeaconOKEventTimer(psInst);
			}
			else
			{
				 IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEATerminateSession: Error lpfnStopBST: %d", iLastBCMErr));   
			}
		}
		else if (myState.trxInProgress != 0 && myState.mode == BCM_MOD_Transparent) 
		{  
			// If a transaction is in progress send a echo to close the connection (on timeout)
 			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEATerminateSession: Send echo .."));   
			iLastBCMErr = psInst->lpfnSendCmd( psInst->myBcmRegPtr, cmdEcho, sizeof(cmdEcho),
												buffer, &lgBuffer, sizeof(buffer), TRUE );
			if(iLastBCMErr == BCM_NoErr)
			{
				psInst->bIsTrxInProgress  = FALSE;
				psInst->bIsBSTPolling = FALSE;
				psInst->bBeaconBSTCmdOK = TRUE;
				psInst->bBeaconOK = TRUE;
				ResetBeaconOKEventTimer(psInst);
				dwErr = NO_ERROR;
			}
			else
			{
				 IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEATerminateSession: Error lpfnStopBST: %d", iLastBCMErr));   
				dwErr = IPGEATranslateError(iLastBCMErr);
			}
		}
		else if(myState.mode != BCM_MOD_Transparent)
		{
			dwErr = ERROR_NOT_READY;
		}

	}
	else
	{
		IPGEA_CALL_HOOK_COMMENT((psInst,"IPGEATerminateSession... Error checkState!"));
		dwErr = IPGEATranslateError(iLastBCMErr);
	}


	return dwErr;
}






PRIVATE DWORD WINAPI IPGEAProcessStatusReq(IN IPGEA_INSTANCE * psInst )
{
	BCM_ERR	iLastBCMErr= BCM_NoErr;
	DWORD	dwErr;
	IPGEAL7_API		sNewInputL7Message;
	ST_BCM_STATE	myState;

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessStatusReq..."));

	if((psInst->bIsBSTPolling && psInst->bBeaconBSTCmdOK) || (!psInst->bIsBSTPolling))
	{
		iLastBCMErr = psInst->lpfnCheckState(psInst->myBcmRegPtr, &myState );
		dwErr = IPGEATranslateError(iLastBCMErr);
		
		if(dwErr != NO_ERROR)
		{
			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessStatusReq err:%d", iLastBCMErr));
		}
	}
	else
	{
		IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessStatusReq:In this state the CheckState does not respond!"));
		iLastBCMErr = BCM_NoErr;
		myState.state = BCM_PbBeacon;
		myState.mode = BCM_MOD_Stopped;
		myState.trxInProgress = 1;
	}

    // Préparer le packet de lecture
    psInst->dwInBytes = sizeof( psInst->tbIn );
	sNewInputL7Message.bMessageType = psInst->sCurrentDecodedOutL7Message.bMessageType;
	sNewInputL7Message.dwNumL2Bytes = 3;
	
	psInst->tbIn[0] = myState.state;
    
	psInst->tbIn[1] = myState.mode;
    psInst->tbIn[2] = myState.trxInProgress;

	psInst->bIsTrxInProgress = (myState.trxInProgress!=0)?TRUE:FALSE;

	CopyMemory( sNewInputL7Message.tbL2Bytes, psInst->tbIn, sNewInputL7Message.dwNumL2Bytes);
	
	IPGEA_CALL_HOOK_COMMENT((psInst,"IPGEAProcessStatusReq... state:%d; mode:%d; trxProgress:%d", 
				myState.state, myState.mode, myState.trxInProgress));
	
	if(myState.mode == BCM_MOD_Transparent && !psInst->bIsTrxInProgress)
	{
		psInst->bBeaconBSTCmdOK = TRUE;
		psInst->bBeaconOK = TRUE;
		ResetBeaconOKEventTimer(psInst);
	}
	else
		psInst->bBeaconBSTCmdOK = FALSE;
	
	dwErr = IPGEAEncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);

	return dwErr;
}

PRIVATE DWORD WINAPI IPGEAProcessModeTransparentReq(IN IPGEA_INSTANCE * psInst )
{
	BCM_ERR			iLastBCMErr;
	IPGEAL7_API		sNewInputL7Message;
	DWORD			dwErr;
	BYTE	buffer[256];
	DWORD	lgBuffer;
	BYTE	cmdEcho[9] = {0x91,0x05,0x00,0x0F,0x02,0x03,0xAA,0x55,0xAA};
	ST_BCM_STATE	myState;
	
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessModeTransparentReq..."));
	
	myState.state = BCM_PbBeacon;
	myState.mode = BCM_MOD_Stopped;
	myState.trxInProgress = FALSE;

	if(psInst->bBeaconOK)
	{
		// Try set the beacon to Transparent mode
		iLastBCMErr = psInst->lpfnChangeMode( psInst->myBcmRegPtr, BCM_MOD_Transparent);
		IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessModeTransparentReq:ChangeMode to transparent Error:%d", iLastBCMErr));
		dwErr = IPGEATranslateError( iLastBCMErr );

		if(iLastBCMErr == BCM_TrxInProgress)
		{
			iLastBCMErr = psInst->lpfnSendCmd( psInst->myBcmRegPtr, cmdEcho, sizeof(cmdEcho),
												buffer, &lgBuffer, sizeof(buffer), TRUE );
			psInst->bIsTrxInProgress  = FALSE;
		}
		else if(iLastBCMErr == BCM_NoErr)
		{
			//assure that it really is transparent the mode ....
			iLastBCMErr = psInst->lpfnCheckState(psInst->myBcmRegPtr, &myState );
			 
			if(iLastBCMErr == BCM_NoErr && myState.state == BCM_NoErr && myState.mode == BCM_MOD_Transparent)
			{
				IPGEA_CALL_HOOK_COMMENT((psInst,"Transparent mode OK!"));
				psInst->bBeaconBSTCmdOK = TRUE;
				psInst->bBeaconOK = TRUE;
				ResetBeaconOKEventTimer(psInst);
				psInst->dwPollOKTimeoutsCount = 0;

				dwErr = NO_ERROR;
			}
		}
		else
		{
			IPGEA_CALL_HOOK_COMMENT((psInst,"ChangeMode returned error: %d!", iLastBCMErr));

			; //will reply with preset myState structure data
		}
	}

	//reply with mode status message
        psInst->dwInBytes = sizeof( psInst->tbIn );
		sNewInputL7Message.bMessageType = GEAIP_INTF_REQ_MODE_TRANSPARENT;
		sNewInputL7Message.dwNumL2Bytes = 3;
		psInst->tbIn[0] = myState.state;
		psInst->tbIn[1] = myState.mode;
        psInst->tbIn[2] = myState.trxInProgress;

		CopyMemory( sNewInputL7Message.tbL2Bytes, psInst->tbIn, sNewInputL7Message.dwNumL2Bytes);
		
		IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessModeTransparentReq... state:%d; mode:%d; trxProgress:%d", 
							myState.state, myState.mode, myState.trxInProgress));
		dwErr = IPGEAEncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);

		if(myState.mode == BCM_MOD_Transparent && !psInst->bIsTrxInProgress)
			psInst->bBeaconBSTCmdOK = TRUE;
		else
			psInst->bBeaconBSTCmdOK = FALSE;

	return dwErr;
}




PRIVATE DWORD WINAPI IPGEAProcessL2_BST_Message(IN IPGEA_INSTANCE * psInst )
{
	BCM_ERR			iLastBCMErr = BCM_NoErr;
	DWORD			dwErr = NO_ERROR;
	BYTE			byBeaconIDMode;

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2_BST_Message: bIsBSTPolling:%d, bIsTrxInProgress:%d, bBeaconBSTCmdOK:%d",
					psInst->bIsBSTPolling, psInst->bIsTrxInProgress, psInst->bBeaconBSTCmdOK));

	
	if(psInst->bBeaconBSTCmdOK)
	{
	//do the actual start BST call only if the request was not already sent 
	//or if some error was detected 
		if(psInst->bChangeBeaconID)	
			byBeaconIDMode = BCM_BST_ChangeBID;
		else
			byBeaconIDMode = BCM_BST_Normal;

		// Start BST polling 
		iLastBCMErr = psInst->lpfnStartBST( psInst->myBcmRegPtr, 
											 psInst->sCurrentDecodedOutL7Message.tbL2Bytes,
											 psInst->sCurrentDecodedOutL7Message.dwNumL2Bytes,
											 byBeaconIDMode);

		dwErr = IPGEATranslateError( iLastBCMErr );

		if (dwErr == NO_ERROR)
		{
			psInst->bBeaconOK = TRUE;
			ResetBeaconOKEventTimer(psInst);
			psInst->fLastMessage = FALSE;
			psInst->bIsBSTPolling = TRUE;
			
		// Le message a été émis et une réponse a été reçue
			IPGEA_CALL_HOOK( IPGEA_HOOK_OUTPUT | IPGEA_HOOK_DATA | IPGEA_HOOK_FLUSH,
				psInst->sCurrentDecodedOutL7Message.dwNumL2Bytes, 
				psInst->sCurrentDecodedOutL7Message.tbL2Bytes );
		}
		else
		{
			if( psInst->bIsBSTPolling && !psInst->bIsTrxInProgress && psInst->sParams.bChangeBeaconID)
			{
				IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2_BST_Message: iLastBCMErr:[%d] - BST polling already in progress...", iLastBCMErr ));
				dwErr = NO_ERROR;
			}

		}
	}
	else
	{
		dwErr = ERROR_NOT_READY;
	}

	if (dwErr == NO_ERROR)
	{
		IPGEATerminateCurrentPacket( psInst, NO_ERROR );
	}
	else
	{
		IPGEA_CALL_HOOK_COMMENT((psInst, "Error: IPGEAProcessL2_BST_Message... iLastBCMErr:%d ,dwErr:%d"
									,iLastBCMErr, dwErr));
	}	

	return dwErr;
}


PRIVATE DWORD WINAPI IPGEAProcessL2Message(IN IPGEA_INSTANCE * psInst )
{
	IPGEAL7_API		sNewInputL7Message;
	BCM_ERR			iLastBCMErr;
	DWORD			dwErr;

	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2Message... psInst->fLastMessage:%d", psInst->fLastMessage));
		// Envoyer le message using beacon manager send function
        psInst->dwInBytes = sizeof( psInst->tbIn );
        iLastBCMErr = psInst->lpfnSendCmd( psInst->myBcmRegPtr, 
											 psInst->sCurrentDecodedOutL7Message.tbL2Bytes,
                                             psInst->sCurrentDecodedOutL7Message.dwNumL2Bytes,
                                             psInst->tbIn,
                                             &psInst->dwInBytes,
											 sizeof(psInst->tbIn),
                                             (char)( psInst->fLastMessage ? 1 : 0 ) );
        psInst->fLastMessage = FALSE;

        
		if ( ( iLastBCMErr == BCM_NoErr ) && ( psInst->dwInBytes != 0 ) )
        {
            // Le message a été émis et une réponse a été reçue
            IPGEA_CALL_HOOK( IPGEA_HOOK_OUTPUT | IPGEA_HOOK_DATA | IPGEA_HOOK_FLUSH,
                psInst->sCurrentDecodedOutL7Message.dwNumL2Bytes, 
                psInst->sCurrentDecodedOutL7Message.tbL2Bytes );


			IPGEA_CALL_HOOK( IPGEA_HOOK_INPUT | IPGEA_HOOK_DATA | IPGEA_HOOK_FLUSH,
					psInst->dwInBytes, 
					psInst->tbIn );

            // Préparer le packet de lecture
			sNewInputL7Message.bMessageType = psInst->sCurrentDecodedOutL7Message.bMessageType;
			sNewInputL7Message.dwNumL2Bytes = psInst->dwInBytes;
            CopyMemory( sNewInputL7Message.tbL2Bytes, psInst->tbIn, sNewInputL7Message.dwNumL2Bytes);

			dwErr = IPGEAEncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
         }
        else if ( (iLastBCMErr == BCM_TmoOBE) ||
                  ( ( iLastBCMErr == BCM_NoErr ) && ( psInst->dwInBytes == 0 ) ) )
        {
           // Le message a été émis mais aucune réponse n'a été renvoyée
            IPGEA_CALL_HOOK( IPGEA_HOOK_OUTPUT | IPGEA_HOOK_DATA | IPGEA_HOOK_FLUSH,
                psInst->sCurrentDecodedOutL7Message.dwNumL2Bytes, 
                psInst->sCurrentDecodedOutL7Message.tbL2Bytes);
            
 			IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEAProcessL2Message... tag timeout..."));

			sNewInputL7Message.bMessageType = GEAIP_L2_TIMEOUT;
			sNewInputL7Message.dwNumL2Bytes = 0;
			
			//we have to reply with empty message of type L2 timeout otherwise the user might assume that 
			//the beacon went off-line 
			dwErr = IPGEAEncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
        }
		else
		{
			dwErr = IPGEATranslateError( iLastBCMErr );
			IPGEA_CALL_HOOK_COMMENT((psInst, "Error: IPGEAProcessL2Message... iLastBCMErr:%d ,dwErr:%d",iLastBCMErr, dwErr));
		}
		
	return dwErr;
}



PRIVATE DWORD WINAPI IPGEAEncodeAndReplyWithL7Packet(IN IPGEA_INSTANCE * psInst, IPGEAL7_API * psNewInputL7Message)
{
	DWORD			dwErr,dwPos;
    IPGEA_PACKET * psL7Packet;
    BOOL			fResult;

	psL7Packet = HeapAlloc( GetProcessHeap(), 0, sizeof(*psL7Packet) + psInst->dwInBytes );

    if ( psL7Packet != NULL )
    {
        ZeroMemory( psL7Packet, sizeof( *psL7Packet ) );
		
		dwPos = 0;
		psL7Packet->dwBufferBytes = sizeof( psL7Packet->tbBuffer );

		fResult = IPGEAL7_EncodeMessage(psInst, psL7Packet->tbBuffer,&dwPos,&psL7Packet->dwBufferBytes, psNewInputL7Message);
		if ( ! fResult )
		{
			IPGEAMakeError( psInst, ERROR_INVALID_DATA, "IPGEAHandleIRP : ..." );
			return ERROR_INVALID_DATA;
		}

        // Repartir à zéro avec le buffer et l'état de reception du message
        psInst->dwInBytes = 0;

		// Ajouter l'item dans la file d'attente
        dwErr = QueueWriteItem( psInst->psReadQueue, psL7Packet, INFINITE );
        if ( dwErr != NO_ERROR )
        {
            // Echec dans la file d'attente : On libère le buffer alloué
            // La reception a échouée, on provoque une erreur
            HeapFree( GetProcessHeap(), 0, psL7Packet );
            IPGEAMakeError( psInst, dwErr, "IPGEAHandleIRP : QueueWriteItem" );
        }
    }
	
	return dwErr;
}



PRIVATE DWORD WINAPI IPGEARunThread( 
        IN IPGEA_INSTANCE * psInst )
{
    DWORD       dwErr;
    DWORD       dwRes;
    DWORD       dwIndex;
	enuCallbackFlagStatus eCallbackStatus;

    SetThreadPriorityBoost( GetCurrentThread(), TRUE );
    SetThreadPriority( GetCurrentThread(), psInst->sParams.dwPriority );

    // Animer l'automate
    while ( TRUE )
    {
        dwRes =  WaitForMultipleObjects( IPGEA_EVT_COUNT, psInst->thEvents, FALSE, INFINITE);
        dwIndex = dwRes - WAIT_OBJECT_0;

        if ( dwIndex == IPGEA_END_EVENT_INDEX )
        {
            // Signal de fin reçu, on sort de la boucle
            break;
        }

        else if ( dwIndex == IPGEA_IRP_EVENT_INDEX )
        {
            IPGEAHandleIRP( psInst );
        }
        else if ( dwIndex == IPGEA_CALLBACK_EVENT_INDEX )
        {
			//extract the next flag in the queue
			while(UnqueueCALLBACK_PACKET(psInst , &eCallbackStatus))
			{
				switch(eCallbackStatus)
				{
				case enuCALLBACK_FLAG_CONNECTION: //connection flag extracted
					{
						IPGEAHandleConnectionEvent( psInst , TRUE);
					}
					break;
				case enuCALLBACK_FLAG_DISCONNECTION: //disconnection flag extracted
					{
						IPGEAHandleConnectionEvent( psInst , FALSE);
					}
					break;
				case enuCALLBACK_FLAG_VST:
					{
						IPGEAHandleVSTEvent(psInst);	
					}
					break;
				case enuCALLBACK_FLAG_RESET:
					{
						IPGEAHandleResetEvent(psInst);	
					}
					break;
				case enuCALLBACK_FLAG_BEACON_OK:
					{
						IPGEAHandleBeaconOKEvent(psInst);
					}
					break;
				case enuCALLBACK_FLAG_ALARM_BEACON:
					{
						IPGEAHandleAlarmBeaconEvent(psInst);
					}
					break;
				case enuCALLBACK_FLAG_TIMEOUT_BEACON_OK:
					{
						IPGEAHandleBeaconOKEventTimeout(psInst);
					}
					break;

				default:
 					IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEA_CALLBACK_EVENT_INDEX processing... error extracting callback flag %d",eCallbackStatus));
					break;
				}
			}	
		}
		else
        {
			IPGEA_CALL_HOOK_COMMENT((psInst, "ERROR:IPGEARunThread::WaitForMultipleObjects exited with index:%d!", dwIndex));
		}
    }
	
	IPGEA_CALL_HOOK_COMMENT((psInst, "IPGEARunThread...Thread loop!"));   


    // Avant la fin du thread, vider la file d'attente en emission
    if ( psInst->psCurrentL7Packet == NULL )
    {
        // Si aucun packet en cours, récuperer le suivant si présent
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentL7Packet, 0 );
    }

    // Tant qu'il reste des packets pour l'envoi de messages
    while ( psInst->psCurrentL7Packet != NULL && dwErr == NO_ERROR )
    {
        // Terminer le packet avec signal d'annulation
        IPGEATerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

        // Passer à l'élément suivant de la file
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentL7Packet, 0 );
    }

    // Vider la file d'attente en reception :
    // Récupérer le premier élément de la file de réception
    dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentL7Packet, 0 );

    // Tant qu'il y des packets
    while ( psInst->psCurrentL7Packet != NULL && dwErr == NO_ERROR )
    {
        //
        // Libérer le packet
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentL7Packet );

        //
        // Récupérer le suivant
        //
        dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentL7Packet, 0 );
    }


    ExitThread( NO_ERROR );

    return NO_ERROR;
}




PRIVATE void WINAPI IPGEATerminateCurrentPacket( IPGEA_INSTANCE * psInst, DWORD dwErr )
{
    //
    // Si un packet est en cours d'émission
    //
    if ( psInst->psCurrentL7Packet != NULL )
    {
        //
        // Signaler le code de résultat associé
        //
        if ( psInst->psCurrentL7Packet->pdwErrCode != NULL )
            (*(psInst->psCurrentL7Packet->pdwErrCode)) = dwErr;

        //
        // Signaler l'événement associé
        //
        if ( psInst->psCurrentL7Packet->hEndPacketEvent != NULL )
            SetEvent( psInst->psCurrentL7Packet->hEndPacketEvent );

        //
        // Libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentL7Packet );

        //
        // Plus de packet en cours
        //
        psInst->psCurrentL7Packet = NULL;
    }
}



PRIVATE void WINAPI IPGEATrack( IPGEA_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes )
{
    BOOL    fFlush;
    DWORD   dwRemain;

    fFlush = ( ( dwContext & IPGEA_HOOK_FLUSH ) != 0 );
    dwContext = ( dwContext & ( ~ IPGEA_HOOK_FLUSH ) );

    if ( dwContext != psInst->dwHookContext )
    {
        if ( ( psInst->dwHookBytes    > 0    ) && 
             ( psInst->dwHookContext != 0    ) )
        {
            psInst->sParams.pfHook(
                psInst->sParams.pvHookContext,
                psInst->dwHookContext,
                psInst->tbHook,
                psInst->dwHookBytes );
        }

        psInst->dwHookBytes = 0;
        psInst->dwHookContext = dwContext;
    }

    if ( psInst->dwHookBytes < sizeof(psInst->tbHook) )
    {
        dwRemain = sizeof(psInst->tbHook) - psInst->dwHookBytes;
        if ( dwBytes > dwRemain )
            dwBytes = dwRemain;
        memcpy( &psInst->tbHook[psInst->dwHookBytes], pbBytes, dwBytes );
        psInst->dwHookBytes += dwBytes;
    }

    if ( fFlush )
    {
        if ( ( psInst->dwHookBytes    > 0    ) && 
             ( psInst->dwHookContext != 0    ) )
        {
            psInst->sParams.pfHook(
                psInst->sParams.pvHookContext,
                psInst->dwHookContext,
                psInst->tbHook,
                psInst->dwHookBytes );
        }

        psInst->dwHookBytes = 0;
        psInst->dwHookContext = dwContext;
    }
}







PRIVATE DWORD WINAPI IPGEATranslateError( int iL7Status )
{
    switch ( iL7Status )
    {
		case BCM_NoErr                          : return NO_ERROR;

		case BCM_BadParam						: return ERROR_INVALID_PARAMETER;
		case BCM_CommTimeout					: return ERROR_TIMEOUT; //	: return ERROR_WRITE_FAULT;
		case BCM_ErrResponse					: return ERROR_BAD_FORMAT;		
		case BCM_ErrCreateEvent					: return ERROR_BAD_FORMAT;
		case BCM_ErrCreateMutex					: return ERROR_BAD_FORMAT;
		case BCM_ErrCreateTimer					: return ERROR_BAD_FORMAT;
		case BCM_ErrEvent						: return ERROR_BAD_FORMAT;
		case BCM_ErrMemory                      : return ERROR_GEN_FAILURE;
		case BCM_TmoOBE							: return ERROR_TIMEOUT;
		case BCM_PbBeacon						: return ERROR_NOT_READY;
    }

    return 0xE000FFFF;

}


PRIVATE BOOL GetLibProc(IN IPGEA_INSTANCE   * psInst)
{
	if(psInst->hDll!=NULL)
		return TRUE;
	else
	{
		psInst->hDll = LoadLibrary( "BeaconManager.Dll" );
		if ( psInst->hDll == NULL )
			return( FALSE );

		// Get all the library functions
		psInst->lpfnGetLibVersion =		(BCM_LPFN_GetLibVersion)		GetProcAddress( psInst->hDll, BCM_FN_GetLibVersion );
		psInst->lpfnInitManagerWND =	(BCM_LPFN_InitManagerWND)		GetProcAddress( psInst->hDll, BCM_FN_InitManagerWND );
		psInst->lpfnInitManagerTHD =	(BCM_LPFN_InitManagerTHD)		GetProcAddress( psInst->hDll, BCM_FN_InitManagerTHD );
		psInst->lpfnInitManagerFNC =	(BCM_LPFN_InitManagerFNC)		GetProcAddress( psInst->hDll, BCM_FN_InitManagerFNC );
		psInst->lpfnInitManagerWND_IP =	(BCM_LPFN_InitManagerWND_IP)	GetProcAddress( psInst->hDll, BCM_FN_InitManagerWND_IP );
		psInst->lpfnInitManagerTHD_IP =	(BCM_LPFN_InitManagerTHD_IP)	GetProcAddress( psInst->hDll, BCM_FN_InitManagerTHD_IP );
		psInst->lpfnInitManagerFNC_IP =	(BCM_LPFN_InitManagerFNC_IP)	GetProcAddress( psInst->hDll, BCM_FN_InitManagerFNC_IP );
		psInst->lpfnCloseManager =		(BCM_LPFN_CloseManager)			GetProcAddress( psInst->hDll, BCM_FN_CloseManager );
		psInst->lpfnChangeMode =		(BCM_LPFN_ChangeMode)			GetProcAddress( psInst->hDll, BCM_FN_ChangeMode );
		psInst->lpfnStartBST =			(BCM_LPFN_StartBST)				GetProcAddress( psInst->hDll, BCM_FN_StartBST );
		psInst->lpfnGetVST =			(BCM_LPFN_GetVST)				GetProcAddress( psInst->hDll, BCM_FN_GetVST );
		psInst->lpfnGetUserParams =		(BCM_LPFN_GetUserParams)		GetProcAddress( psInst->hDll, BCM_FN_GetUserParams );
		psInst->lpfnSendCmd =			(BCM_LPFN_SendCmd)				GetProcAddress( psInst->hDll, BCM_FN_SendCmd );
		psInst->lpfnStopBST =			(BCM_LPFN_StopBST)				GetProcAddress( psInst->hDll, BCM_FN_StopBST );
		psInst->lpfnCheckState =		(BCM_LPFN_CheckState)			GetProcAddress( psInst->hDll, BCM_FN_CheckState );
		psInst->lpfnReset =				(BCM_LPFN_Reset)				GetProcAddress( psInst->hDll, BCM_FN_Reset );
		psInst->lpfnSetConfig =			(BCM_LPFN_SetConfig)			GetProcAddress( psInst->hDll, BCM_FN_SetConfig );
		psInst->lpfnGetConfig =			(BCM_LPFN_GetConfig)			GetProcAddress( psInst->hDll, BCM_FN_GetConfig );
		psInst->lpfnGetBeaconID =		(BCM_LPFN_GetBeaconID)			GetProcAddress( psInst->hDll, BCM_FN_GetBeaconID );

		// Check that all functions are present in the Dll
		if ( psInst->lpfnGetLibVersion == NULL ||
			 psInst->lpfnInitManagerWND == NULL ||
			 psInst->lpfnInitManagerTHD == NULL ||
			 psInst->lpfnInitManagerFNC == NULL ||
			 psInst->lpfnInitManagerWND_IP == NULL ||
			 psInst->lpfnInitManagerTHD_IP == NULL ||
			 psInst->lpfnInitManagerFNC_IP == NULL ||
			 psInst->lpfnCloseManager == NULL ||
			 psInst->lpfnChangeMode == NULL ||
			 psInst->lpfnStartBST == NULL ||
			 psInst->lpfnGetVST == NULL ||
			 psInst->lpfnGetUserParams == NULL ||
			 psInst->lpfnSendCmd == NULL ||
			 psInst->lpfnStopBST == NULL ||
			 psInst->lpfnCheckState == NULL ||
			 psInst->lpfnReset == NULL ||
			 psInst->lpfnSetConfig == NULL ||
			 psInst->lpfnGetConfig == NULL ||
			 psInst->lpfnGetBeaconID == NULL )
		{
			
			if ( psInst->hDll != NULL )
				FreeLibrary( psInst->hDll );
			psInst->hDll = NULL;

			return( FALSE );
		}

		return( TRUE );

	}
}


/*------------------------------------------------------------------------------------------------------*/
void BCM_EXPORT Callback( ST_BCM_REG_PTR argBcmRegPtr, BCM_CALLBACK argTypeCB, DWORD argParam )
{
	IPGEA_INSTANCE  * psInst = IPGEAGetInstanceFor(argBcmRegPtr);

	if(psInst!=NULL)
	{
		switch( argTypeCB )
		{
			case BCM_CB_IN :
				// Declare the transaction in progress
				EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_VST);
				IPGEA_CALL_HOOK_COMMENT((psInst, "Callback: VST received... "));
			break;
			case BCM_CB_ERR:
				{
					EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_ALARM_BEACON);
					IPGEA_CALL_HOOK_COMMENT((psInst, "Callback: BCM_CB_ERR : %d", argParam ));
				}
				break;

			default:
				{
					EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_ALARM_BEACON);
					IPGEA_CALL_HOOK_COMMENT((psInst, "Callback: argTypeCB:%d argParam:%d", argTypeCB, argParam ));
				
				}
				break;
		}

	}

}


/*------------------------------------------------------------------------------------------------------*/
void BCM_EXPORT Alarm( ST_BCM_REG_PTR argBcmRegPtr, BCM_ALARMS argAlarm, DWORD argState )
{
	static char *gLabel[] = {"","Beacon not ready","Beacon KO","Beacon reset","Beacon OK"};
	IPGEA_INSTANCE  * psInst = IPGEAGetInstanceFor(argBcmRegPtr);

	if(psInst!=NULL)
	{
			// If it is an event
			if ( argAlarm > BCM_AlarmBeacon )
			{
				IPGEA_CALL_HOOK_COMMENT((psInst, "### EVENT (Reg:%d, Param:%d - %s) ###", argAlarm, argState, gLabel[argAlarm]));

				// If the beacon has reset, terminate
				if ( argAlarm == BCM_EventReset)
				{
					EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_RESET);
				}
				else if( argAlarm == BCM_EventPollingOK)
				{
					EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_BEACON_OK);
				}
			}
			// If it is an alarm
			else
			{
	 			if ( argState )
				{
					IPGEA_CALL_HOOK_COMMENT((psInst,"### ALARM (%s - ON) ###", gLabel[argAlarm] ));
				}
				else
				{
					IPGEA_CALL_HOOK_COMMENT((psInst,"### ALARM (%s - OFF) ###", gLabel[argAlarm] ));
				}
				
				// Check for the connection
				if ( argAlarm == BCM_AlarmPeriph )
				{
					if ( argState )
						EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_DISCONNECTION);
					else
						EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_CONNECTION);
				}
				else if(argAlarm == BCM_AlarmBeacon)
				{
					if(argState)
					{
						EnqueueCALLBACK_PACKET(psInst, enuCALLBACK_FLAG_ALARM_BEACON);
					}

					IPGEA_CALL_HOOK_COMMENT((psInst, "ALARM BCM_AlarmBeacon:%d", argState ));

				}
			}
	}
}


PRIVATE DWORD WINAPI IPGEAVerifyOpenPort( IN IPGEA_INSTANCE  * psInst )
{
    DWORD dwErr = NO_ERROR;

    __try
    {

        EnterCriticalSection( &psInst->sSerialize );

        if (!psInst->fLinkOpen || !psInst->bBeaconOK)
        {
			dwErr = ERROR_TIMEOUT;
        }
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );

        
    }

	return dwErr;
}


//----------------------------------------------

//
// Vérification des pointeurs et de leurs valeurs
//
#define IPGEAL7_IS_VALID(x)         ((x)<0x80000000)
#define CHK_PTR(x)      if ( (x) == NULL ) __leave; else if ( ! IPGEAL7_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)      if ( ! IPGEAL7_IS_VALID(x) ) __leave;

//
// Manipulation des buffers
//
#define GET_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { (v) = *(BYTE*)&pbBuf[dwPos] ; dwPos += sizeof(BYTE); }
#define GET_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( (v), pbBuf+dwPos, (s) ) ; dwPos += (s); }
#define ADD_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { *(BYTE*)&pbBuf[dwPos]  = (BYTE)(v); dwPos += sizeof(BYTE); }
#define ADD_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( pbBuf+dwPos, (v), (s) ); dwPos += (s); }



EXPORT BOOL WINAPI IPGEAL7_EncodeMessage( 
        IN      IPGEA_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      IPGEAL7_API       *  psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;
    BYTE            bLen;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // Verifiy the parameters are coherent
        //
        if ( psDef->dwNumL2Bytes > sizeof(psDef->tbL2Bytes) )
            __leave;

        bLen = (BYTE)psDef->dwNumL2Bytes + 2;

        ADD_BYTE ( psDef->bMessageType );
        ADD_BYTE ( bLen                );
        ADD_BYTES( psDef->dwNumL2Bytes, psDef->tbL2Bytes );

        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwPos;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




EXPORT BOOL WINAPI IPGEAL7_DecodeMessage( 
        IN      IPGEA_INSTANCE   * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     IPGEAL7_API       * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;
    BYTE            bLen;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // Get the header
        //
        GET_BYTE( psDef->bMessageType); // Response type
        GET_BYTE( bLen);                // Length

        //
        // Make sure the size is correct
        //
        if ( bLen < 2 )
            __leave;

        psDef->dwNumL2Bytes = (DWORD)bLen - 2;
        GET_BYTES( psDef->dwNumL2Bytes, psDef->tbL2Bytes );
        
        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwBufLen;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}



//-----------------------------------------------

DWORD EnqueueCALLBACK_PACKET(  IPGEA_INSTANCE   * psInst,
								enuCallbackFlagStatus eCallbackStatus)
{
	DWORD dwErr = NO_ERROR;

    dwErr = QueueWriteItem( psInst->psCallbackQueue, (void*)eCallbackStatus, INFINITE );
    if ( dwErr != NO_ERROR )
	{
		IPGEA_CALL_HOOK_COMMENT((psInst, "EnqueueCALLBACK_PACKET error writing to Callback Queue event flag:%d!!!", eCallbackStatus));
	}
    
	return dwErr;
}

BOOL UnqueueCALLBACK_PACKET(  IPGEA_INSTANCE   * psInst,
							  enuCallbackFlagStatus* peCallbackStatus)
{
	DWORD dwErr = NO_ERROR;
	void * eCallbackStatusFlag;

    dwErr = QueueReadItem( psInst->psCallbackQueue, &eCallbackStatusFlag, 0 );

	if ( dwErr == NO_ERROR )
			*peCallbackStatus = (DWORD)eCallbackStatusFlag;

	return (dwErr==NO_ERROR);
}


//----------------------------------------------
