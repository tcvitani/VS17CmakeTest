/* --------------------------------------------------------------------
 * Sanef ITS(C) - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFDMI_L2
 * FILE       : QFDMI_L2.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Couche 2 QFDMI_L2
 * --------------------------------------------------------------------
 * DESCRIPTION:
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <winsock2.h>
// Link to mswsock.lib
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>
#include <csr_queue.h>
#include <csr_timer.h>

#include "auto.h"
#include "csr_tag_QFDMI.h"
#include "QFDMI_http_client.h"


#include <memclass.h>


//
// Vérification des pointeurs et de leurs valeurs
//
#define CHK_PTR(x)      if ( (x) == NULL ) __leave; else if ( ! QFDMI_L7_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)      if ( ! QFDMI_L7_IS_VALID(x) ) __leave;




//
// Manipulation des buffers
//
#define GET_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { (v) = *(BYTE*)&pbBuf[dwPos] ; dwPos += sizeof(BYTE); }
#define GET_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( (v), pbBuf+dwPos, (s) ) ; dwPos += (s); }
#define ADD_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { *(BYTE*)&pbBuf[dwPos]  = (BYTE)(v); dwPos += sizeof(BYTE); }
#define ADD_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( pbBuf+dwPos, (v), (s) ); dwPos += (s); }




//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects et qui
// serviront à provoquer des événements sur l'automate protocolaire
//
typedef enum{
	QFDMI_L2_END_INDEX = 0,
	QFDMI_L2_CMD_QUEUE_INDEX,
	QFDMI_L2_LISTEN_SOCK_INDEX,
	QFDMI_L2_ACCEPTED_SOCK_INDEX,
	QFDMI_L2_DMI_TIMER_INDEX,
	QFDMI_L2_TRS_TIMER_INDEX,
	QFDMI_CLIENT_READ_QUEUE_INDEX,
	QFDMI_L2_EVT_COUNT
};


typedef enum
{
	QFDMI_MSG_UNKNOWN,
	QFDMI_MSG_LIFE,
	QFDMI_MSG_LIFE_ACK,
	QFDMI_MSG_VST,
	QFDMI_MSG_DSRC_FRAME,
	QFDMI_MSG_RELEASE,
	QFDMI_MSG_EVENT,
	QFDMI_MSG_ACTION_RESPONSE,
	QFDMI_MSG_TRACKING

}
enum_QFDMI_type_msg;



//
// Hook utilisé pour l'espionnage de la liaison
//
#define QFDMI_L2_CALL_HOOK(ctx, size, ptr)  if (psInst->sParams.pfHook != NULL) QFDMI_L2Track(psInst, ctx, size, ptr); else


//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et LRC).
//
#define QFDMI_L2_IN_BUFFER_BYTES      8192
#define QFDMI_L2_OUT_BUFFER_BYTES     500
#define QFDMI_L2_HOOK_BUFFER_BYTES    8192




//
// Définition d'un packet pour la file des messages en sortie
// et en entrée
//
typedef struct _QFDMI_L2_PACKET
{
    void              * pvContext;
    DWORD             * pdwErrCode;
    DWORD               dwBufferBytes;
	BYTE                tbBuffer[255];
}
    QFDMI_L2_PACKET;

    
//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _QFDMI_L2_INSTANCE
{
    // Paramètres de l'instance
	QFR_DMI_PARAMS        sParams;

	QFDMI_HTTP_CLIENT_INSTANCE * psClientThrInst;

    // Handle du thread de gestion du protocole
    HANDLE              hThread;

	// Objet "timer" pour la gestion des délais dans le protocole
	TIMER_INSTANCE    * psTimer;

	//timer to detect transaction DSRC timeouts 
	TIMER_INSTANCE    * psTrsTimer; 



	// Objet "automate" pour la gestion de l'automate
	H_AUT               hAut;

    // File d'attente pour les messages sortant
    QUEUE_INSTANCE    * psWriteQueue;

    // File d'attente pour les messages entrant
    QUEUE_INSTANCE    * psReadQueue;

    // Evénement signalé si la file psReadQueue a au moins un élément
    // et non signalé si la psReadQueue est vide
    HANDLE              hEndEvent;

	// Handle of the event for server listen
	HANDLE              hListenEvent;
	// Handle of the event for async i/o
	HANDLE              hReceiveEvent;

    // Tableau des handles signalables servant à animer l'automate
    HANDLE              thEvents[QFDMI_L2_EVT_COUNT];

	BOOLEAN             fWSAInitialized;// Server Socket Initialized
	BOOLEAN             fListenSocketInitialized;
	BOOLEAN             fAcceptSocketInitialized;
	BOOLEAN             fAcceptSocketConnected;
	BOOLEAN             fSocketReceiveInitiated;

	SOCKET	socListenSocket;
	SOCKET	socAcceptSocket;
	CHAR	cAcceptBuffer[2 * (sizeof(SOCKADDR_IN) + 16)];
	WSAOVERLAPPED ovrListenOverlapped;

	WSAOVERLAPPED ovrReceptionOverlapped;
	WSABUF		  sWsaDataBuf;

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // En phase d'émission, packet courant à émettre
	QFDMI_L2_PACKET			*psCurrentPacket;

	// Message from application level that possibly contains the L2 level packet to be emitted to the beacon 
	QFDMI_INTERNAL_MSG			sCurrentDecodedOutL7Message;


    //DWORD               dwInBytes;
    //BYTE                tbIn[ QFDMI_L2_IN_BUFFER_BYTES ];

    // Taille occupée dans le buffer de réception
	BYTE				bufrec[QFDMI_L2_IN_BUFFER_BYTES]; // Reception buffer
    // Buffer de réception
	DWORD				dwBufRecLen;

	BYTE				bufWholeMessage[QFDMI_L2_IN_BUFFER_BYTES]; //Whole msg Reception buffer
	DWORD				dwBufWholeMessageLen;
	
	//Last received MR packages data
	UCHAR				szStreamId[MAX_PATH];
	UCHAR				szLid[MAX_PATH];
	UCHAR				szTrsStatus[MAX_PATH];
	UCHAR				szInDsrcHexData[QFDMI_L2_IN_BUFFER_BYTES];

	UCHAR				szTrsInProgressLid[MAX_PATH];
	UCHAR				szTrsInProgressLastStreamId[MAX_PATH];

	// Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ QFDMI_L2_HOOK_BUFFER_BYTES ];

	BOOLEAN  fLastMessage;
	BOOLEAN  fLastTrsPackageDirectionIN; //which direction was sent the last communication package?
}
    QFDMI_L2_INSTANCE;


//
// Autres fonctions privées
//
PRIVATE void WINAPI QFDMI_L2MakeError( QFDMI_L2_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE DWORD WINAPI QFDMI_L2RunThread( IN QFDMI_L2_INSTANCE * psInst );
PRIVATE void WINAPI QFDMI_L2TerminateCurrentPacket( QFDMI_L2_INSTANCE * psInst, DWORD dwErr );
PRIVATE void WINAPI QFDMI_L2Track( QFDMI_L2_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );

PRIVATE DWORD WINAPI QFDMI_L2InitializeServerSock(QFDMI_L2_INSTANCE * psInst);

PRIVATE void WINAPI QFDMI_L2HandleCmdQueue(QFDMI_L2_INSTANCE * psInst);
PRIVATE void WINAPI QFDMI_L2HandleNewConnection(QFDMI_L2_INSTANCE * psInst);
PRIVATE void WINAPI QFDMI_L2HandleAcceptedSock(QFDMI_L2_INSTANCE * psInst);
PRIVATE void WINAPI QFDMI_L2HandleDMITimer(QFDMI_L2_INSTANCE * psInst);

PRIVATE DWORD QFDMI_L2Reset_AcceptSocket(QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD QFDMI_L2Initialize_AcceptSocket(QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD QFDMI_L2Force_CloseConnection(QFDMI_L2_INSTANCE * psInst);

PRIVATE BOOL QFDMI_L2RecognizeWholeMessage(QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD QFDMI_L2ProcessWholeMessage(QFDMI_L2_INSTANCE * psInst);
PROTECTED BOOL QFDMI_L2LanTransmit(QFDMI_L2_INSTANCE * psInst, SOCKET mon_socket, char *buffer, int length);
//-----------------------------------------------------------------------------
PRIVATE void PrepareLifeMsgAckBody(QFDMI_L2_INSTANCE * psInst, IN UCHAR *szStreamId, OUT UCHAR *pucMsgBody, OUT DWORD *pdwMsgLen);
PRIVATE void PrepareMsgHeader(IN DWORD dwContentLen, OUT UCHAR *pucMsgHeader, OUT DWORD *pdwMsgLen);
PRIVATE void PrepareDMIMsgBody(QFDMI_L2_INSTANCE * psInst,
	IN UCHAR *tbPacketBuffer,
	IN DWORD dwPacketBufferBytes,
	IN BOOL fLastMessage,
	IN UCHAR *szStreamId,
	IN UCHAR *szLid,
	OUT UCHAR *pucMsgBody,
	OUT DWORD *pdwMsgLen);
PRIVATE void PrepareDMI_SAP_CONTINUE_MsgBody(QFDMI_L2_INSTANCE * psInst,
	IN UCHAR *szStreamId,
	IN UCHAR *szLid,
	OUT UCHAR *pucMsgBody,
	OUT DWORD *pdwMsgLen);
//-----------------------------------------------------------------------------
PRIVATE void WINAPI QFDMI_Trans_ACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_DISCONNECTED(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT_AND_ACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT_AND_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_VST_NEW_TRS(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_DSRC_FRAME_IN(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_DSRC_FRAME_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_LAST_DSCR_FRAME_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_DMI_TIMEOUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ACTIVATED_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_DEACTIVATED_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_TerminateTRS_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_TerminateTRS_NO_INPUT_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_EchoREQ_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_EmptyREQ_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_ON_Trs_TIMEOUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_Trans_EchoRSP_IN(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);


//-----------------------------------------------------------------------------

PRIVATE DWORD WINAPI QFDMI_ProcessTerminateSession(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD WINAPI QFDMI_ProcessStatusReq(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD WINAPI QFDMI_ProcessModeTransparentReq(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD WINAPI QFDMI_ProcessL2_BST_Message(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD WINAPI QFDMI_ProcessL2Message(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE DWORD WINAPI QFDMI_ProcessL2_EmptyReq(IN QFDMI_L2_INSTANCE * psInst);

PRIVATE DWORD WINAPI QFDMI_SendL2Req(IN QFDMI_L2_INSTANCE * psInst, BOOL fLastMessage);
PRIVATE DWORD WINAPI QFDMI_SendL2EchoReq(IN QFDMI_L2_INSTANCE * psInst, BOOL bEndTrsInProgress); 
PRIVATE void WINAPI QFDMI_ResetDMITimer(IN QFDMI_L2_INSTANCE * psInst);

PRIVATE void WINAPI QFDMI_StartTrsTimer(IN QFDMI_L2_INSTANCE * psInst);
PRIVATE void WINAPI QFDMI_StopTrsTimer(IN QFDMI_L2_INSTANCE * psInst);
//-----------------------------------------------------------------------------
PRIVATE DWORD WINAPI QFDMI_EncodeAndReplyWithL7Packet(IN QFDMI_L2_INSTANCE * psInst, QFDMI_INTERNAL_MSG * psNewInputL7Message);
PRIVATE void WINAPI QFDMI_AutSendEvent(QFDMI_L2_INSTANCE   * psInst, H_AUT hAut, DWORD  dwEventId, void * pvParam);


//
// Définitions des événements de l'automate protocolaire
//
typedef enum
{
	QFDMI_EVT_LIFE_EVT = 0,
	QFDMI_EVT_VST_FRAME_IN,
	QFDMI_EVT_DSRC_FRAME_IN,
	QFDMI_EVT_DSRC_FRAME_OUT,
	QFDMI_EVT_ECHO_RSP_IN,
	QFDMI_EVT_LAST_DSRC_FRAME_OUT,
	QFDMI_EVT_EMPTY_REQ_OUT,
	QFDMI_EVT_DMI_TIMEOUT,
	QFDMI_EVT_TRS_TIMEOUT,
	QFDMI_EVT_DISCONNECTED,
	QFDMI_EVT_ACTIVATE,
	QFDMI_EVT_ACTIVATED_OK,
	QFDMI_EVT_DEACTIVATE,
	QFDMI_EVT_DEACTIVATED_OK,
	QFDMI_EVT_ABORT_TRS,
	QFDMI_MAX_AUT_EVENTS
}
QFDMI_AUT_EVENTS;

char *gaszEventDesc[] =
{
	"QFDMI_EVT_LIFE_EV",
	"QFDMI_EVT_VST_FRAME_IN",
	"QFDMI_EVT_DSRC_FRAME_IN",
	"QFDMI_EVT_DSRC_FRAME_OUT",
	"QFDMI_EVT_ECHO_RSP_IN",
	"QFDMI_EVT_LAST_DSRC_FRAME_OUT",
	"QFDMI_EVT_EMPTY_REQ_OUT",
	"QFDMI_EVT_DMI_TIMEOUT",
	"QFDMI_EVT_TRS_TIMEOUT",
	"QFDMI_EVT_DISCONNECTED",
	"QFDMI_EVT_ACTIVATE",
	"QFDMI_EVT_ACTIVATED_OK",
	"QFDMI_EVT_DEACTIVATE",
	"QFDMI_EVT_DEACTIVATED_OK",
	"QFDMI_EVT_ABORT_TRS",
	"QFDMI_MAX_AUT_EVENTS"
};

//
// Définitions des états de l'automate protocolaire
//
typedef enum
{
	QFDMI_ST_NO_CONN_DEACTIVE_REQ = 0,
	QFDMI_ST_NO_CONN_ACTIVATE_REQ,
	QFDMI_ST_CONNECTED_DEACTIVATE_REQ,
	QFDMI_ST_CONNECTED_ACTIVATING,
	QFDMI_ST_CONNECTED_ACTIVE,
	QFDMI_ST_TRS_IN_PROGRESS,
	QFDMI_ST_CONNECTED_DEACTIVATING,
	QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,

	QFDMI_MAX_AUT_STATES
}
QFDMI_AUT_STATES;





PRIVATE void AddSpacesToHexStr(IN OUT UCHAR *pszStr)
{
	DWORD	i, j, k;
	UCHAR	szStr[QFDMI_L2_IN_BUFFER_BYTES];


	for (i = 0, j = 0, k = 0; i <= strlen(pszStr); i++, k++)
	{
		if (k == 2 && i != strlen(pszStr))
		{
			k = 0;
			szStr[j++] = 0x20;
		}

		szStr[j++] = pszStr[i];
	}

	strcpy(pszStr, szStr);
}


PRIVATE void ConvertHEXToASCII(IN const BYTE *tbHex, IN DWORD dwLen, OUT UCHAR *szAscii)
{
	DWORD	dwH, dwA;
	UCHAR	chCh;

	for (dwH = 0, dwA = 0; dwH < dwLen; dwH++, dwA += 2)
	{
		szAscii[dwA] = (chCh = (tbHex[dwH] & 0xF0) >> 4) < 10 ? (chCh | 0x30) : (chCh + 55);
		szAscii[dwA + 1] = (chCh = tbHex[dwH] & 0x0F) < 10 ? (chCh | 0x30) : (chCh + 55);
	}
	szAscii[dwA] = '\0';
}

PRIVATE void ConvertASCIIToHEX(IN const UCHAR *szAscii, OUT BYTE *tbHex, OUT DWORD *dwLen)
{
	DWORD	dwH, dwA;
	UCHAR	chCh;
	DWORD	iLen;

	iLen = strlen(szAscii) / 2;

	for (dwH = 0, dwA = 0; dwH < iLen; dwH++, dwA += 2)
	{
		tbHex[dwH] = ((((chCh = szAscii[dwA]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)) << 4) |
			((((chCh = szAscii[dwA + 1]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)));
	}
	*dwLen = dwH;
}

PRIVATE void RemoveSpacesFromHexStr(IN OUT UCHAR *pszStr)
{
	DWORD	i, j;
	UCHAR	szStr[QFDMI_L2_IN_BUFFER_BYTES];


	for (i = 0, j = 0; i <= strlen(pszStr); i++)
	{
		if (pszStr[i] != 0x20 && pszStr[i] != '\n' && pszStr[i] != '\t' && pszStr[i] != '\r')
			szStr[j++] = pszStr[i];
	}

	strcpy(pszStr, szStr);
}



PRIVATE void ConvertHex2Bin(IN const UCHAR *pszSource, OUT UCHAR *pszDest, OUT DWORD *pnbBytes)
{
	DWORD	i, j;
	UCHAR pucTemp[QFDMI_L2_IN_BUFFER_BYTES];
	*pnbBytes = 0;

	strcpy(pucTemp, pszSource);
	RemoveSpacesFromHexStr(pucTemp);

	ConvertASCIIToHEX(pucTemp, pszDest, pnbBytes);

	return;
}


PRIVATE void ConvertBin2Hex(IN const UCHAR *pszSource, IN DWORD nbLen, OUT UCHAR *pszDest)
{
	ConvertHEXToASCII(pszSource, nbLen, pszDest);

	AddSpacesToHexStr(pszDest);

	return;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFDMI_L2Open( 
 *                      OUT QFDMI_L2_INSTANCE  ** ppsInst,
 *                      IN  QFREE_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication QFDMI_L2
 *             (utilisé pour la communication HOTE<->BADGE par liaison filaire)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFDMI_L2Open( 
        OUT QFDMI_L2_INSTANCE  ** ppsInst,
		IN  QFR_DMI_PARAMS     * psParams)
{
    DWORD               dwErr       = NO_ERROR;
    QFDMI_L2_INSTANCE      * psInst      = NULL;
    DWORD               dwId;

    __try
    {
        //
        // Valeur par défaut retournée : NULL
        //
        (*ppsInst) = NULL;

        //
        // Allouer la place pour la structure d'instance et la remplir de 0
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(QFDMI_L2_INSTANCE) );
        if ( psInst == NULL ) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Reporter les valeurs des paramètres une fois qu'on s'est assuré que la
        // taille de la structure est compatible.
        //
        if ( psParams->dwStructBytes != sizeof(*psParams) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        psInst->sParams = (*psParams);

        //
        // Créer l'événement utilisé pour signaler la fin.
        //
        psInst->hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psInst->hEndEvent == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Create event for new server connections
        //
		psInst->hListenEvent = WSACreateEvent();
		if (psInst->hListenEvent == NULL)
        {
            dwErr = GetLastError();
            __leave;
        }


		//
		// Create event tolisten events from accepted socket
		//
		psInst->hReceiveEvent = WSACreateEvent();
		if (psInst->hReceiveEvent == NULL)
		{
			dwErr = GetLastError();
			__leave;
		}
			

        //
        // Initialisation des files d'attente
        //
        dwErr = QueueOpen( &psInst->psReadQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;
        dwErr = QueueOpen( &psInst->psWriteQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;

 
		//
		// Creation du timer intercaractère
		//
		dwErr = TimerOpenEx(TIMER_TYPE_WAITABLE | TIMER_MAKE_RESOLUTION(4), &psInst->psTimer);
		if (dwErr != NO_ERROR)
			__leave;

		
		dwErr = TimerOpenEx(TIMER_TYPE_WAITABLE | TIMER_MAKE_RESOLUTION(4), &psInst->psTrsTimer);
		if (dwErr != NO_ERROR)
			__leave;

		//
        // Initialisation de la liaison
        //
        dwErr = QFDMI_L2InitializeServerSock( psInst );
        if ( dwErr != NO_ERROR )
            __leave;

		dwErr = QFDMI_L2Initialize_AcceptSocket(psInst);
		if (dwErr != ERROR_IO_PENDING)
			__leave;



		dwErr = QFDMI_HTTP_CLIENT_Open(psInst, &psInst->sParams,&psInst->psClientThrInst);
		if (dwErr != NO_ERROR)
			__leave;


        //
        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        //
        psInst->thEvents[QFDMI_L2_END_INDEX] = psInst->hEndEvent;
		psInst->thEvents[QFDMI_L2_CMD_QUEUE_INDEX] = QueueGetWaitableHandle(psInst->psWriteQueue);
		psInst->thEvents[QFDMI_L2_LISTEN_SOCK_INDEX] = psInst->hListenEvent;
		psInst->thEvents[QFDMI_L2_ACCEPTED_SOCK_INDEX] = psInst->hReceiveEvent;
		psInst->thEvents[QFDMI_L2_DMI_TIMER_INDEX] = TimerGetWaitableHandle(psInst->psTimer);
		psInst->thEvents[QFDMI_L2_TRS_TIMER_INDEX] = TimerGetWaitableHandle(psInst->psTrsTimer);
		
		psInst->thEvents[QFDMI_CLIENT_READ_QUEUE_INDEX] = QueueGetWaitableHandle(psInst->psClientThrInst->psClientReadQueue);
	
		//
		// Création de l'automate protocolaire
		//
		psInst->hAut = AutOpen(QFDMI_MAX_AUT_STATES, psInst);
		if (psInst->hAut == NULL)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}


		//
		// Définition des états de l'automate protocolaire
		//
		if ((!AutAddState(psInst->hAut, QFDMI_ST_NO_CONN_DEACTIVE_REQ)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_NO_CONN_ACTIVATE_REQ)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS)) ||
			(!AutAddState(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING))||
			(!AutAddState(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY))
			)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}

		//
		// Définition des transitions de l'automate protocolaire
		//
		if (
			(!AutAddTransition(psInst->hAut, QFDMI_ST_NO_CONN_DEACTIVE_REQ,		QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_DEACTIVATE_REQ,	QFDMI_Trans_LIFE_EVT_AND_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_NO_CONN_DEACTIVE_REQ,		QFDMI_EVT_ACTIVATE,			QFDMI_ST_NO_CONN_ACTIVATE_REQ,		AUT_EMPTY_FUNCTION)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_NO_CONN_ACTIVATE_REQ,		QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_Trans_LIFE_EVT_AND_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_NO_CONN_ACTIVATE_REQ,		QFDMI_EVT_DEACTIVATE,		QFDMI_ST_NO_CONN_DEACTIVE_REQ,		AUT_EMPTY_FUNCTION)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ,	QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_DEACTIVATE_REQ,				QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ, QFDMI_EVT_VST_FRAME_IN,		QFDMI_ST_CONNECTED_DEACTIVATING,				QFDMI_Trans_TerminateTRS_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ, QFDMI_EVT_DSRC_FRAME_IN,	QFDMI_ST_CONNECTED_DEACTIVATING,				QFDMI_Trans_TerminateTRS_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ,	QFDMI_EVT_DISCONNECTED,		QFDMI_ST_NO_CONN_DEACTIVE_REQ,					QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ,	QFDMI_EVT_ACTIVATE,			QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ, QFDMI_EVT_DMI_TIMEOUT,		QFDMI_ST_CONNECTED_DEACTIVATE_REQ,				QFDMI_Trans_ON_DMI_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATE_REQ, QFDMI_EVT_ECHO_RSP_IN,		QFDMI_ST_CONNECTED_DEACTIVATE_REQ,				QFDMI_Trans_EchoRSP_IN)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_VST_FRAME_IN,		QFDMI_ST_TRS_IN_PROGRESS,						QFDMI_Trans_ON_VST_NEW_TRS)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_DSRC_FRAME_IN,	QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_DISCONNECTED,		QFDMI_ST_NO_CONN_ACTIVATE_REQ,					QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_DMI_TIMEOUT,		QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_ON_DMI_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_DEACTIVATE,		QFDMI_ST_CONNECTED_DEACTIVATE_REQ,				QFDMI_Trans_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_EMPTY_REQ_OUT,	QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,		AUT_EMPTY_FUNCTION)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_ACTIVATE,			QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_ACTIVATED_OK,		QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_ACTIVATED_OK)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVATING,		QFDMI_EVT_ECHO_RSP_IN,		QFDMI_ST_CONNECTED_ACTIVATING,					QFDMI_Trans_EchoRSP_IN)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_VST_FRAME_IN,		QFDMI_ST_TRS_IN_PROGRESS,						QFDMI_Trans_ON_VST_NEW_TRS)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_DSRC_FRAME_IN,	QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_DISCONNECTED,		QFDMI_ST_NO_CONN_ACTIVATE_REQ,					QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_DEACTIVATE,		QFDMI_ST_CONNECTED_DEACTIVATING,				QFDMI_Trans_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_ACTIVATE,			QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_EMPTY_REQ_OUT,	QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,		AUT_EMPTY_FUNCTION)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_DMI_TIMEOUT,		QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_ON_DMI_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE,			QFDMI_EVT_ECHO_RSP_IN,		QFDMI_ST_CONNECTED_ACTIVE,						QFDMI_Trans_EchoRSP_IN)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_LIFE_EVT,				QFDMI_ST_TRS_IN_PROGRESS,					QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_VST_FRAME_IN,			QFDMI_ST_TRS_IN_PROGRESS,					QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_DSRC_FRAME_IN,		QFDMI_ST_TRS_IN_PROGRESS,					QFDMI_Trans_ON_DSRC_FRAME_IN)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_DSRC_FRAME_OUT,		QFDMI_ST_TRS_IN_PROGRESS,					QFDMI_Trans_ON_DSRC_FRAME_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_LAST_DSRC_FRAME_OUT,	QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_ON_LAST_DSCR_FRAME_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_EMPTY_REQ_OUT,		QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_EmptyREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_DISCONNECTED,			QFDMI_ST_NO_CONN_ACTIVATE_REQ,				QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_DEACTIVATE,			QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_TerminateTRS_NO_INPUT_DEACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_TRS_TIMEOUT,			QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_ON_Trs_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_ACTIVATE,				QFDMI_ST_TRS_IN_PROGRESS,					AUT_EMPTY_FUNCTION)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_TRS_IN_PROGRESS,			QFDMI_EVT_ECHO_RSP_IN,			QFDMI_ST_TRS_IN_PROGRESS,					QFDMI_Trans_EchoRSP_IN)) ||
			
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_LIFE_EVT,				QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_DSRC_FRAME_IN,		QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_VST_FRAME_IN,			QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_DMI_TIMEOUT,			QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_ON_DMI_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_DISCONNECTED,			QFDMI_ST_NO_CONN_DEACTIVE_REQ,				QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_ACTIVATE,				QFDMI_ST_CONNECTED_ACTIVATING,				QFDMI_Trans_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_DEACTIVATED_OK,		QFDMI_ST_CONNECTED_DEACTIVATE_REQ,			QFDMI_Trans_DEACTIVATED_OK)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_ECHO_RSP_IN,			QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_EchoRSP_IN)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_DEACTIVATING,	QFDMI_EVT_DEACTIVATE,			QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_DEACTIVATE)) ||

			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_LIFE_EVT,			QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_LIFE_EVT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_VST_FRAME_IN,		QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_DSRC_FRAME_IN,		QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_EchoREQ_OUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_DMI_TIMEOUT,		QFDMI_ST_NO_CONN_ACTIVATE_REQ,				QFDMI_Trans_ON_DMI_TIMEOUT)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_DISCONNECTED,		QFDMI_ST_NO_CONN_ACTIVATE_REQ,				QFDMI_Trans_ON_DISCONNECTED)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_ACTIVATE,			QFDMI_ST_CONNECTED_ACTIVATING,				QFDMI_Trans_ACTIVATE)) ||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_DEACTIVATE,		QFDMI_ST_CONNECTED_DEACTIVATING,			QFDMI_Trans_DEACTIVATE))||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_ECHO_RSP_IN,		QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_EchoRSP_IN))||
			(!AutAddTransition(psInst->hAut, QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY, QFDMI_EVT_TRS_TIMEOUT,		QFDMI_ST_CONNECTED_ACTIVE_USER_NOT_READY,	QFDMI_Trans_ON_Trs_TIMEOUT))
			
			)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}

		//
		// Placer l'automate dans son état initial
		//
		if (!AutSetCurrentState(psInst->hAut, QFDMI_ST_NO_CONN_DEACTIVE_REQ))
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}


        //
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFDMI_L2RunThread, (void*)psInst, 0, &dwId, "TAG_QFDMI_L2_PROTOCOL" );
        psInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFDMI_L2RunThread, (void*)psInst, 0, &dwId );
        if ( psInst->hThread == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Pour améliorer la réactivité globale, on diminue le délai de préemption
        // à une milliseconde.
        //
        timeBeginPeriod( 1 );

        //
        // Mettre à jour les données à retourner
        //
        (*ppsInst) = psInst;
        psInst = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psInst != NULL )
            QFDMI_L2Close( psInst, 0 );

    }

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI QFDMI_L2Close( 
 *                      IN QFDMI_L2_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par QFDMI_L2Open
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             QFDMI_L2Open(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI QFDMI_L2Close( 
        IN QFDMI_L2_INSTANCE   * psInst,
        IN DWORD            dwTimeout )
{
    if ( psInst != NULL )
    {
		if(psInst->psClientThrInst!=NULL)
			QFDMI_HTTP_CLIENT_Close(psInst->psClientThrInst,10);

		
		if ( psInst->hThread != NULL )
        {
            timeEndPeriod( 1 );
            SetEvent( psInst->hEndEvent );
            WaitForSingleObject( psInst->hThread, dwTimeout );
            TerminateThread( psInst->hThread, NO_ERROR );
            CloseHandle( psInst->hThread );
        }
        if ( psInst->psReadQueue != NULL )
            QueueClose( psInst->psReadQueue );
        if ( psInst->psWriteQueue != NULL )
            QueueClose( psInst->psWriteQueue );
        if ( psInst->hEndEvent != NULL )
            CloseHandle( psInst->hEndEvent );
		if (psInst->hListenEvent != NULL)
			WSACloseEvent(psInst->hListenEvent);
		if (psInst->hReceiveEvent != NULL)
			WSACloseEvent(psInst->hReceiveEvent);

		if (psInst->hAut != NULL)
			AutClose(psInst->hAut);
		if (psInst->psTimer != NULL)
			TimerClose(psInst->psTimer);
		if (psInst->psTrsTimer != NULL)
			TimerClose(psInst->psTrsTimer);

		//TO DO close sockets if open ...
		//if (psInst->fLinkOpen)
  //          CloseHandle( psInst->hPipe );
		if (psInst->fWSAInitialized == TRUE)
			WSACleanup();

		psInst->fWSAInitialized = FALSE;

        HeapFree( GetProcessHeap(), 0, psInst );
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFDMI_L2SendMessage( 
 *                      IN              QFDMI_L2_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par QFDMI_L2Open
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
EXPORT DWORD WINAPI QFDMI_L2SendMessage( 
        IN              QFDMI_L2_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    QFDMI_L2_PACKET    * psPacket = NULL;

    __try
    {
        //
        // Mise à jour immédiate du code de complétion à 0xFFFFFFFF
        //
        if ( pdwErr != NULL )
            (*pdwErr) = 0xFFFFFFFF;

        //
        // Prepare un "packet" contenant le message à émettre
        //
        psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + dwMsgBytes + 5);
        if ( psPacket == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        psPacket->dwBufferBytes = dwMsgBytes;
        psPacket->pdwErrCode = pdwErr;
        psPacket->pvContext = hEvent;
        CopyMemory( psPacket->tbBuffer, pbMsg, dwMsgBytes );

        //
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
 * SYNTAX    : EXPORT HANDLE WINAPI QFDMI_L2GetWaitableHandle(
 *                      IN QFDMI_L2_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par QFDMI_L2Open
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
EXPORT HANDLE WINAPI QFDMI_L2GetWaitableHandle(
        IN QFDMI_L2_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI QFDMI_L2GetCommInstance(
 *                      IN QFDMI_L2_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par QFDMI_L2Open
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI QFDMI_L2GetCommInstance(
        IN QFDMI_L2_INSTANCE * psInst )
{
    return NULL;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFDMI_L2ReceiveMessage(
 *                      IN      QFDMI_L2_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par QFDMI_L2Open
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
 *             à QFDMI_L2FreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFDMI_L2ReceiveMessage(
        IN      QFDMI_L2_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    QFDMI_L2_PACKET    * psPacket;

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
 * SYNTAX    : EXPORT void WINAPI QFDMI_L2FreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par QFDMI_L2ReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à QFDMI_L2ReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI QFDMI_L2FreeMessage(
        IN BYTE * pbMsg )
{
    QFDMI_L2_PACKET * psPacket = NULL;

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
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI QFDMI_L2SetOption(
 *                      IN QFDMI_L2_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par QFDMI_L2Open
 *             dwOptionId  : Identifiant d'option (aucun dispo pour QFDMI_L2)
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
EXPORT DWORD WINAPI WINAPI QFDMI_L2SetOption(
        IN QFDMI_L2_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue )
{
	if (dwOptionId == QFDMI_L2_OPTION_F_LAST_MESSAGE)
	{
		if (pvValue != NULL)
		{
			if (dwValue == 1)
				psInst->fLastMessage = (*(BYTE*)pvValue != 0);
			else if (dwValue == 2)
				psInst->fLastMessage = (*(WORD*)pvValue != 0);
			else if (dwValue == 4)
				psInst->fLastMessage = (*(DWORD*)pvValue != 0);
			else
				return ERROR_INVALID_PARAMETER;
		}
		else
			psInst->fLastMessage = (dwValue != 0);

		return NO_ERROR;
	}
	else
	{
		return ERROR_INVALID_DATA;
	}
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI QFDMI_L2GetOption(
 *                      IN QFDMI_L2_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par QFDMI_L2Open
 *             dwOptionId  : Identifiant d'option (aucun dispo pour QFDMI_L2)
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
EXPORT DWORD WINAPI WINAPI QFDMI_L2GetOption(
        IN QFDMI_L2_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}




PRIVATE void WINAPI QFDMI_L2MakeError( QFDMI_L2_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    QFDMI_L2TerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
    strcpy( psInst->szLastError, pcText );
}

PRIVATE void WINAPI QFDMI_L2ResetInputBuffer(QFDMI_L2_INSTANCE * psInst)
{
	memset(psInst->bufrec, 0, sizeof(psInst->bufrec));
	psInst->dwBufRecLen = 0;

	memset(psInst->bufWholeMessage, 0, sizeof(psInst->bufWholeMessage));
	psInst->dwBufWholeMessageLen = 0;

}

PRIVATE int WINAPI QFDMI_L2PrepareAcceptedSocket(QFDMI_L2_INSTANCE * psInst)
{
	unsigned long	ul = 1;
	char FAR	*   pucRcvBufSize;
	char FAR	*   pucSndBufSize;
	char FAR	*	pbNoDelayOpt;
	int				iErr = NO_ERROR;

	__try
	{
		memset(psInst->bufrec, 0, sizeof(psInst->bufrec));
		psInst->dwBufRecLen = 0;

		//-------------------------------------------------------------------
		//If connected set some socket options	
		//-------------------------------------------------------------------

		//SET socket options: timeout, buffering ...

		////Sock timeout
		//iErr = setsockopt(psInst->socAcceptSocket,
		//	SOL_SOCKET,
		//	SO_RCVTIMEO,
		//	(char *)&psInst->sParams.dwSocketDelayMs,
		//	sizeof(psInst->sParams.dwSocketDelayMs));

		//if (iErr != NO_ERROR)
		//{
		//	QFDMI_L2MakeError(psInst, WSAGetLastError(), "QFDMI_L2PrepareAcceptedSocket() - setsockopt setsockopt(SOL_SOCKET, SO_RCVTIMEO,...) error!");
		//	__leave;
		//}


		//SO_RCVBUF
		pucRcvBufSize = malloc(sizeof(int));

		*pucRcvBufSize = 0;
		ZeroMemory((char *)pucRcvBufSize, sizeof(int));


		iErr = setsockopt(psInst->socAcceptSocket,
			SOL_SOCKET,
			SO_RCVBUF,
			pucRcvBufSize,
			sizeof(int));

		if (iErr != NO_ERROR)
		{
			QFDMI_L2MakeError(psInst, WSAGetLastError(), "QFDMI_L2PrepareAcceptedSocket() - setsockopt(SOL_SOCKET, SO_RCVBUF,...) error!");
			__leave;
		}


		free(pucRcvBufSize);


		//SO_SNDBUF
		pucSndBufSize = malloc(sizeof(int));

		*pucSndBufSize = 1;
		ZeroMemory((char *)pucSndBufSize, sizeof(int));

		iErr = setsockopt(psInst->socAcceptSocket,
			SOL_SOCKET,
			SO_SNDBUF,
			(char *)pucSndBufSize,
			sizeof(int));

		if (iErr != NO_ERROR)
		{
			QFDMI_L2MakeError(psInst, WSAGetLastError(), "QFDMI_L2PrepareAcceptedSocket() - setsockopt(SOL_SOCKET, SO_SNDBUF,...)  error!");
			__leave;
		}

		free(pucSndBufSize);

		//	TCP_NODELAY
		pbNoDelayOpt = malloc(sizeof(BOOL));
		*pbNoDelayOpt = TRUE; //If TRUE, the Nagle algorithm is disabled on the socket

		iErr = setsockopt(psInst->socAcceptSocket,
			IPPROTO_TCP,
			TCP_NODELAY,
			(char *)pbNoDelayOpt,
			sizeof(BOOL));

		if (iErr != NO_ERROR)
		{
			QFDMI_L2MakeError(psInst, WSAGetLastError(), "QFDMI_L2PrepareAcceptedSocket() - setsockopt(IPPROTO_TCP, TCP_NODELAY,...) error!");
			__leave;
		}

		free(pbNoDelayOpt);

		psInst->fSocketReceiveInitiated = FALSE;
	}
	__finally
	{

	}

	return iErr;
}

PRIVATE DWORD WINAPI QFDMI_L2InitiateWSARecv(QFDMI_L2_INSTANCE * psInst)
{
	DWORD        dwBytesTransferred = 0;
	DWORD		 dwFlags = 0;
	DWORD			dwErr = NO_ERROR;

	__try
	{
		if (!psInst->fSocketReceiveInitiated)
		{ 
			ZeroMemory(&(psInst->ovrReceptionOverlapped), sizeof(WSAOVERLAPPED));
			psInst->ovrReceptionOverlapped.hEvent = psInst->hReceiveEvent;
			WSAResetEvent(psInst->hReceiveEvent);
			psInst->sWsaDataBuf.len = sizeof(psInst->bufrec);
			psInst->sWsaDataBuf.buf = psInst->bufrec;

			// Post a WSARecv request to to begin receiving data on the socket
			if (WSARecv(psInst->socAcceptSocket, &psInst->sWsaDataBuf, 1, &dwBytesTransferred, &dwFlags,
				&(psInst->ovrReceptionOverlapped), NULL) == SOCKET_ERROR)
			{
				dwErr = WSAGetLastError();
				if (dwErr != ERROR_IO_PENDING && dwErr != NO_ERROR)
					QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2HandleNewConnection() - WSARecv() failed with error!");
				
				__leave;
			}

			psInst->fSocketReceiveInitiated = TRUE;
		}
	}
	__finally
	{
		if (dwErr != ERROR_IO_PENDING && dwErr != NO_ERROR)
		{
			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2InitiateWSARecv:QFDMI_L2Force_CloseConnection- Connection closed..."));
			QFDMI_L2Force_CloseConnection(psInst);
		}
	}

	return dwErr;
}

PRIVATE void WINAPI QFDMI_L2HandleNewConnection(QFDMI_L2_INSTANCE * psInst)
{
	DWORD        dwBytesTransferred =  0;
	DWORD dwFlags = 0;
	DWORD dwErr = NO_ERROR;
	__try
	{
		
		if (WSAGetOverlappedResult(psInst->socListenSocket, &(psInst->ovrListenOverlapped), &dwBytesTransferred, FALSE, &dwFlags) == FALSE)
		{
			//printf("WSAGetOverlappedResult() failed with error %d\n", WSAGetLastError());
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2HandleNewConnection() - WSAGetOverlappedResult() failed with error!");
			__leave;
		}
		else
		{
			//printf("WSAGetOverlappedResult() is OK!\n");
			
			psInst->fAcceptSocketConnected = TRUE;
			dwErr = QFDMI_L2PrepareAcceptedSocket(psInst);
			if (dwErr != NO_ERROR)
			{
				QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2HandleNewConnection() - WSARecv() failed with error!");
				__leave;
			}

			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleNewConnection: New Connection!"));

			QFDMI_L2InitiateWSARecv(psInst);
			
		}
	}
	__finally
	{
		if (dwErr != NO_ERROR)
		{
			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleNewConnection:QFDMI_L2Force_CloseConnection- Connection closed..."));

			QFDMI_L2Force_CloseConnection(psInst);
		}
	}
}




PRIVATE BOOL TakeUsefulContent(IN UCHAR *pszInBuff, OUT UCHAR *pszOutBuff)
{
	UCHAR	*pucStart, *pucEnd;

	if(pszInBuff == NULL)
		return FALSE;

	// get content between quotation marks id, e.g: <stream id="123465">
	if (pucStart = strchr(pszInBuff, '\"'))
	{
		if (pucEnd = strchr(pucStart + 1, '\"'))
		{
			memcpy(pszOutBuff, pucStart + 1, (pucEnd - pucStart) - 1);
			pszOutBuff[(pucEnd - pucStart) - 1] = '\0';
		}
		else
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

PRIVATE BOOL TakeUsefulContentBracket(IN UCHAR *pszInBuff, OUT UCHAR *pszOutBuff)
{
	UCHAR	*pucStart, *pucEnd;

	// get content between quotation marks id, e.g: <stream id="123465">
	if (pucStart = strchr(pszInBuff, '>'))
	{
		if (pucEnd = strchr(pucStart + 1, '<'))
		{
			memcpy(pszOutBuff, pucStart + 1, (pucEnd - pucStart) - 1);
			pszOutBuff[(pucEnd - pucStart) - 1] = '\0';
		}
		else
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}


PRIVATE BOOL WINAPI QFDMI_IsEchoRsp(UCHAR* pszSource)
{
	UCHAR pucTemp[QFDMI_L2_IN_BUFFER_BYTES];

	strcpy(pucTemp, pszSource);
	RemoveSpacesFromHexStr(pucTemp);
	if (strncmp(pucTemp, "911400", 6) == 0 || strncmp(pucTemp, "911600", 6) == 0)
		return TRUE;
	else
		return FALSE;
}


PRIVATE DWORD QFDMI_L2ProcessWholeMessage(QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	char szTemp[QFDMI_L2_HOOK_BUFFER_BYTES] = { 0 };
	char szDSRC_UIType[10] = { 0 };
	char * pPos = NULL;
	char * pPos2 = NULL;
	char * pPosStreamId = NULL;
	enum_QFDMI_type_msg	eMsgType = QFDMI_MSG_UNKNOWN;
	BOOL bIsTrsLid = FALSE;

	__try
	{
		if (psInst->dwBufWholeMessageLen > 0)
		{
			pPosStreamId = strstr(psInst->bufWholeMessage, DMI_STREAM_ID);
			if (pPosStreamId == NULL)
			{
				eMsgType = QFDMI_MSG_UNKNOWN;
				sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:text:[%s] not found in msg:[%s]!", DMI_STREAM_ID, psInst->bufWholeMessage);
				QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
				__leave;
			}

			// get steram id, e.g: <stream id="123465">
			if (!TakeUsefulContent(pPosStreamId, psInst->szStreamId))
			{
				eMsgType = QFDMI_MSG_UNKNOWN;
				sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Error taking stream ID in msg [%s]!", psInst->bufWholeMessage);
				QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
				__leave;
			}

			pPos = strstr(psInst->bufWholeMessage, DMI_DSRC_FRAME_LID);
			if (pPos != NULL)
			{
				// take lid number
				if (!TakeUsefulContent(pPos, psInst->szLid))
				{
					eMsgType = QFDMI_MSG_UNKNOWN;
					sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Error taking LID in msg [%s]!", psInst->bufWholeMessage);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
					__leave;
				}

				// take type
				pPos2 = strstr(pPos, DMI_DSRC_FRAME_TYPE);
				if (!TakeUsefulContent(pPos2, szDSRC_UIType))
				{
					eMsgType = QFDMI_MSG_UNKNOWN;
					sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Error taking type in msg [%s]!", psInst->bufWholeMessage);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
					__leave;
				}

				pPos2 = strstr(pPos, DMI_DSRC_FRAME_STATUS);
				if (pPos2!=NULL)
				{
					if (TakeUsefulContentBracket(pPos2, psInst->szTrsStatus))
					{
						if (strstr(psInst->szTrsStatus, STATUS_ACK_TIMEOUT) || strstr(psInst->szTrsStatus, STATUS_RETRY_LIMIT))
						{
							sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Status timeout or retry timeout in [%s]!", psInst->bufWholeMessage);
							QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
						}
						else
						{
							eMsgType = QFDMI_MSG_UNKNOWN;
							sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Some unhandled error [%s]!", psInst->bufWholeMessage);
							QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
							__leave;
						}
					}
				}
				
				if (!TakeUsefulContentBracket(pPos, psInst->szInDsrcHexData))
				{
					eMsgType = QFDMI_MSG_UNKNOWN;
					sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Error taking data from msg [%s]!", psInst->bufWholeMessage);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
					__leave;
				}
				else
				{

					if(atoi(szDSRC_UIType) == 1)
					{
						eMsgType = QFDMI_MSG_VST;
					}
					else
					{
						if(strcmp(psInst->szLid, psInst->szTrsInProgressLid)==0)
						{
							strcpy_s(psInst->szTrsInProgressLastStreamId, sizeof(psInst->szTrsInProgressLastStreamId), psInst->szStreamId);
							bIsTrsLid = TRUE;
						}

						eMsgType = QFDMI_MSG_DSRC_FRAME;
					}

				}

			}
			else
			{
				if (strstr(pPosStreamId, DMI_TRACKING))
					eMsgType = QFDMI_MSG_TRACKING; 
				else if (strstr(pPosStreamId, DMI_END_STREAM) || strstr(pPosStreamId, DMI_MONITOR) || strstr(pPosStreamId, DMI_DSCR_TRANSACTION_RESULT))
				{
					eMsgType = QFDMI_MSG_LIFE;
				}
				else
				{
					eMsgType = QFDMI_MSG_UNKNOWN;
					sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage:Error identifying msg [%s]!", psInst->bufWholeMessage);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
					__leave;
				}
			}
		}
		else
		{
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2ProcessWholeMessage() - dwBufWholeMessageLen<=0!");
			__leave;
		}


		switch (eMsgType)
		{
			case QFDMI_MSG_LIFE:
			{
				QFDMI_ResetDMITimer(psInst);
				QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_LIFE_EVT, NULL);
			}
			break;

			case QFDMI_MSG_DSRC_FRAME:
			{
				QFDMI_ResetDMITimer(psInst);
				
				if(!QFDMI_IsEchoRsp(psInst->szInDsrcHexData) && bIsTrsLid)
					QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DSRC_FRAME_IN, NULL);
				else
					QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_ECHO_RSP_IN, NULL);
			}
			break;
			
			case QFDMI_MSG_VST:
			{
				QFDMI_ResetDMITimer(psInst);
				QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_VST_FRAME_IN, NULL);
				QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_VST_RECEIVED, 0);
			}
			break;
			default:
				break;

		}

	}
	__finally
	{
		psInst->dwBufWholeMessageLen = 0;
		memset(psInst->bufWholeMessage, 0, sizeof(psInst->bufWholeMessage));
	}

	return dwErr;

}


PRIVATE void WINAPI QFDMI_L2HandleAcceptedSock(QFDMI_L2_INSTANCE * psInst)
{
	int				iErr = NO_ERROR;
	DWORD			dwTransfer;
	DWORD			dwFlags = 0;
	char szTemp[QFDMI_L2_HOOK_BUFFER_BYTES] = { 0 };

	__try
	{

		//  Determine the status of the overlapped
		//  request
		if (WSAGetOverlappedResult(psInst->socAcceptSocket, &(psInst->ovrReceptionOverlapped), &dwTransfer, FALSE, &dwFlags) == TRUE)
		{

			// First check to see whether the peer has closed
			// the connection, and if so, close the
			// socket
			if (dwTransfer == 0)
			{
				iErr = WSAGetLastError();

				if (iErr != NO_ERROR)
				{
					sprintf_s(szTemp, sizeof(szTemp), "FDMI_L2HandleAcceptedSock: dwTransfer = 0 => Error:[%d]!", iErr);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
				}

			}
			else
			{
				if (dwTransfer >= sizeof(psInst->bufrec) - psInst->dwBufRecLen)
				{
					iErr = ERROR_NOT_ENOUGH_MEMORY;
					QFDMI_L2MakeError(psInst, iErr, "QFDMI_L2HandleAcceptedSock() - dwTransfer > sizeof(PPT_QFR[inst_id].bufrec) - PPT_QFR[inst_id].dwBufRecLen  error!");
					__leave;
				}

				memcpy(&psInst->bufrec[psInst->dwBufRecLen], psInst->sWsaDataBuf.buf, dwTransfer);
				psInst->dwBufRecLen += dwTransfer;

				psInst->fSocketReceiveInitiated = FALSE;

				//try recognize another package
				while(QFDMI_L2RecognizeWholeMessage(psInst))
				{
					//Process new message
					sprintf_s(szTemp, sizeof(szTemp), "IN: [%s]!", psInst->bufWholeMessage);
					QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));

					QFDMI_L2_CALL_HOOK(QFDMI_L2_HOOK_INPUT | QFDMI_L2_HOOK_FLUSH,
						psInst->dwBufWholeMessageLen,
						psInst->bufWholeMessage);

					iErr = QFDMI_L2ProcessWholeMessage(psInst);
					if (iErr != NO_ERROR)
					{
						QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleAcceptedSock: QFDMI_L2ProcessWholeMessage failed:\n"));
					}
				}

			}


		}
		else
		{
			iErr = WSAGetLastError();

			if (iErr != WSA_IO_PENDING)
			{
				sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2HandleAcceptedSock: WSAGetOverlappedResult failed! Error:[%d]!", iErr);
				QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
			}
		}

	}
	__finally
	{
		if (iErr != NO_ERROR && iErr != WSA_IO_PENDING)
		{
			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleAcceptedSock:QFDMI_L2Force_CloseConnection- Connection closed..."));
			QFDMI_L2Force_CloseConnection(psInst);
		}
		else if (!psInst->fSocketReceiveInitiated)
		{
			QFDMI_L2InitiateWSARecv(psInst);
		}

			

	}
}

PRIVATE void WINAPI QFDMI_L2HandleDMITimer(QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	__try
	{
		QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DMI_TIMEOUT, NULL);
	}
	__finally
	{

	}
}



PRIVATE void WINAPI QFDMI_L2HandleTrsTimer(QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	__try
	{
		QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_TRS_TIMEOUT, NULL);
	}
	__finally
	{

	}
}

PRIVATE void WINAPI QFDMI_L2HandleCmdQueue(QFDMI_L2_INSTANCE * psInst)
{
	DWORD        dwErr, dwPos, dwLen;
	BOOL		 fResult;
	QFDMI_INTERNAL_MSG * psL7Packet;

    __try
    {

        if ( psInst->psCurrentPacket == NULL )
        {
            //
            // Si le message n'a pas encore été construit, on l'extrait de la file d'attente
            //
            dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
            if ( dwErr != NO_ERROR )
            {
                QFDMI_L2MakeError( psInst, dwErr, "QFDMI_L2HandleIRP : QueueReadItem" );
                __leave;
            }
        }

        //
        // Dans le mode où les buffers sont vidés automatiquement,
        // on vide la file de réception chaque fois qu'on émet un
        // nouveau message (mode question/réponse).
        //
        if ( psInst->sParams.dwAutoClearBuffers != 0 )
        {
            while ( ! QueueIsEmpty( psInst->psReadQueue ) )
            {
				dwErr = QueueReadItem(psInst->psReadQueue, &psL7Packet, 0);
                if ( dwErr == NO_ERROR )
					HeapFree(GetProcessHeap(), 0, psL7Packet);
            }
        }

		//Decode the message sent from tag.c to identify message type and perform accordingly
		dwLen = psInst->psCurrentPacket->dwBufferBytes;
		dwPos = 0;

		fResult = QFDMI_DecodeInternalMessage(
			psInst,
			psInst->psCurrentPacket->tbBuffer,
			&dwPos,
			&dwLen,
			&psInst->sCurrentDecodedOutL7Message);

		if (!fResult)
		{
			QFDMI_L2MakeError(psInst, ERROR_INVALID_BLOCK, "IPGEAHandleIRP : QFDMI_DecodeInternalMessage");
			__leave;
		}

		switch (psInst->sCurrentDecodedOutL7Message.bMessageType)
		{
		case QFDMI_INTF_REQ_ABORT:
			dwErr = QFDMI_ProcessTerminateSession(psInst);
			break;
		case QFDMI_INTF_REQ_GETSTATUS:
			dwErr = QFDMI_ProcessStatusReq(psInst);
			break;
		case QFDMI_INTF_REQ_MODE_TRANSPARENT:
			dwErr = QFDMI_ProcessModeTransparentReq(psInst);
			break;
		case QFDMI_SEND_L2_FRAME:
			dwErr = QFDMI_ProcessL2Message(psInst);
			break;
		case QFDMI_SEND_L2_FRAME_BST:
			dwErr = QFDMI_ProcessL2_BST_Message(psInst);
			break;
		case QFDMI_SEND_L2_FRAME_END:
			dwErr = QFDMI_ProcessL2_EmptyReq(psInst);
			break;
		}

		if (dwErr != NO_ERROR)
		{
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2HandleCmdQueue : Error processing internal message!");
		}
		else
		{
			QFDMI_L2TerminateCurrentPacket(psInst, NO_ERROR);
		}

	}
	__finally
	{

	}

}



PRIVATE void WINAPI QFDMI_HandleCLIENTReadQueue(QFDMI_L2_INSTANCE * psInst)
{
	DWORD        dwErr;
	enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE        eMsgType;
	DWORD             dwData;

	__try
	{
		dwErr = QFDMI_HTTP_CLIENT_ReceiveMessage(psInst->psClientThrInst, &eMsgType, &dwData, 0);

		if(dwErr!= NO_ERROR)
		{
			__leave;
		}
		else
		{
			switch (eMsgType)
			{
				case 	eMsg_QFDMI_ACTIVATE_RSP:
					QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_ACTIVATED_OK, NULL); 
					break;
				case 	eMsg_QFDMI_DEACTIVATE_RSP:
					QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DEACTIVATED_OK, NULL);
					break;

				default:
					QFDMI_L2MakeError(psInst, dwErr, "QFDMI_HandleCLIENTReadQueue : Unknown message type!");

					break;
			}
		}

	}
	__finally
	{
		if (dwErr != WAIT_TIMEOUT)
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_HandleCLIENTReadQueue : Error!");

	}

}


PRIVATE DWORD WINAPI QFDMI_L2RunThread( 
        IN QFDMI_L2_INSTANCE * psInst )
{
    DWORD       dwErr;
    DWORD       dwRes;
    DWORD       dwIndex;
	char szTemp[QFDMI_L2_HOOK_BUFFER_BYTES];

    SetThreadPriorityBoost( GetCurrentThread(), TRUE );
    SetThreadPriority( GetCurrentThread(), psInst->sParams.dwPriority );

    //
    // Animer l'automate
    //
    while ( TRUE )
    {
		dwRes = WaitForMultipleObjects(QFDMI_L2_EVT_COUNT, psInst->thEvents, FALSE, INFINITE);
        dwIndex = dwRes - WAIT_OBJECT_0;

        if ( dwIndex == QFDMI_L2_END_INDEX )
        {
            //
            // Signal de fin reçu, on sort de la boucle
            //
            break;
        }

		else if (dwIndex == QFDMI_L2_CMD_QUEUE_INDEX)
        {
            QFDMI_L2HandleCmdQueue( psInst );
        }
		else if (dwIndex == QFDMI_L2_LISTEN_SOCK_INDEX)
		{
			WSAResetEvent(psInst->thEvents[QFDMI_L2_LISTEN_SOCK_INDEX]);
			QFDMI_L2HandleNewConnection(psInst);
		}
		else if (dwIndex == QFDMI_L2_ACCEPTED_SOCK_INDEX)
		{
			WSAResetEvent(psInst->thEvents[QFDMI_L2_ACCEPTED_SOCK_INDEX]);
			QFDMI_L2HandleAcceptedSock(psInst);
		}
		else if (dwIndex == QFDMI_L2_DMI_TIMER_INDEX)
		{
			// Timer event is trigered by the windows system 
			// if set to be triggered using CSR_TIMER.h function
			// named TimerActivate
			TimerAcknowledge(psInst->psTimer); //Acknoledge the timer so the event will not be detected as signaled again
			
			QFDMI_L2HandleDMITimer(psInst);
		}
		else if (dwIndex == QFDMI_L2_TRS_TIMER_INDEX)
		{
			// Timer event is trigered by the windows system 
			// if set to be triggered using CSR_TIMER.h function
			// named TimerActivate
			TimerAcknowledge(psInst->psTrsTimer); //Acknoledge the timer so the event will not be detected as signaled again

			QFDMI_L2HandleTrsTimer(psInst);
		}

		else if (dwIndex == QFDMI_CLIENT_READ_QUEUE_INDEX)
		{
			QFDMI_HandleCLIENTReadQueue(psInst);
			
		}
		else
        {
			sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2RunThread: WaitForMultipleObjects returned %d \n!", dwRes);
			QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
			
			//
            // Ignorer
            //
        }
    }

    //
    // Avant la fin du thread, vider la file d'attente en emission
    //

    if ( psInst->psCurrentPacket == NULL )
    {
        //
        // Si aucun packet en cours, récuperer le suivant si présent
        //
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
    }

    //
    // Tant qu'il reste des packets pour l'envoi de messages
    //
    while ( psInst->psCurrentPacket != NULL && dwErr == NO_ERROR )
    {
        //
        // Terminer le packet avec signal d'annulation
        //
        QFDMI_L2TerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

        //
        // Passer à l'élément suivant de la file
        //
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
    }

    //
    // Vider la file d'attente en reception :
    // Récupérer le premier élément de la file de réception
    //
    dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentPacket, 0 );

    //
    // Tant qu'il y des packets
    //
    while ( psInst->psCurrentPacket != NULL && dwErr == NO_ERROR )
    {
        //
        // Libérer le packet
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentPacket );

        //
        // Récupérer le suivant
        //
        dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentPacket, 0 );
    }

    ExitThread( NO_ERROR );

    return NO_ERROR;
}




PRIVATE void WINAPI QFDMI_L2TerminateCurrentPacket( QFDMI_L2_INSTANCE * psInst, DWORD dwErr )
{
    // Si un packet est en cours d'émission
    if ( psInst->psCurrentPacket != NULL )
    {
        // Signaler le code de résultat associé
        if ( psInst->psCurrentPacket->pdwErrCode != NULL )
            (*(psInst->psCurrentPacket->pdwErrCode)) = dwErr;

         // Signaler l'événement associé
        if ( psInst->psCurrentPacket->pvContext != NULL )
            SetEvent( psInst->psCurrentPacket->pvContext );

        // Libérer le bloc
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentPacket );

        // Plus de packet en cours
        psInst->psCurrentPacket = NULL;
    }
}


PRIVATE void WINAPI QFDMI_L2Track(QFDMI_L2_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes)
{
	BOOL    fFlush;
	DWORD   dwRemain;

	if ( psInst->sParams.pfHook == NULL )
		return;


	fFlush = ((dwContext & QFDMI_L2_HOOK_FLUSH) != 0);
	dwContext = (dwContext & (~QFDMI_L2_HOOK_FLUSH));

	if (dwContext != psInst->dwHookContext)
	{
		if ((psInst->dwHookBytes > 0) &&
			(psInst->dwHookContext != 0))
		{
			psInst->sParams.pfHook(
				psInst->sParams.pvHookContext,
				psInst->dwHookContext,
				psInst->tbHook,
				psInst->dwHookBytes);

		}

		psInst->dwHookBytes = 0;
		psInst->dwHookContext = dwContext;
	}

	if (psInst->dwHookBytes < sizeof(psInst->tbHook))
	{
		dwRemain = sizeof(psInst->tbHook) - psInst->dwHookBytes;
		if (dwBytes > dwRemain)
			dwBytes = dwRemain;
		memcpy(&psInst->tbHook[psInst->dwHookBytes], pbBytes, dwBytes);
		psInst->dwHookBytes += dwBytes;
	}

	if (fFlush)
	{
		if ((psInst->dwHookBytes > 0) &&
			(psInst->dwHookContext != 0))
		{
			psInst->sParams.pfHook(
				psInst->sParams.pvHookContext,
				psInst->dwHookContext,
				psInst->tbHook,
				psInst->dwHookBytes);
		}

		psInst->dwHookBytes = 0;
		psInst->dwHookContext = dwContext;
	}
}


void WINAPI QFDMI_TraceComment(QFDMI_L2_INSTANCE * pInst, char * szStrToShow, ...)
{
	int iSize;
	va_list args;
	char szOutputBuffer[4096] = {0};
	char szTimestamp[256] = {0};
	int iTimestampSize;

	if (pInst != NULL)
		if (pInst->sParams.pfHook != NULL)
		{
			sprintf_s(szTimestamp, sizeof(szTimestamp), "%d", timeGetTime());
			szOutputBuffer[0] = '\0';
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), "\n");
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), szTimestamp);
			strcat_s(szOutputBuffer, sizeof(szOutputBuffer), " -");
			iTimestampSize = (int)strlen(szOutputBuffer);

			va_start(args, szStrToShow);
			iSize = _vsnprintf_s(szOutputBuffer + iTimestampSize, (sizeof(szOutputBuffer) - iTimestampSize), sizeof(szOutputBuffer) - iTimestampSize, szStrToShow, args);
			va_end(args);


			if (iSize > 0 && iSize < sizeof(szOutputBuffer) - iTimestampSize)
			{
				QFDMI_L2Track(pInst, QFDMI_L2_HOOK_COMMENT | QFDMI_L2_HOOK_FLUSH, iSize + iTimestampSize, szOutputBuffer);
			}
		}
}

/* -------------------------------------------------------------------------
SOCKET Functions
------------------------------------------------------------------------- */


PRIVATE DWORD WINAPI QFDMI_L2InitializeServerSock(QFDMI_L2_INSTANCE * psInst)
{
	DWORD dwErr = NO_ERROR;
	WORD				wVersionRequested;
	WSADATA				wsaData;
	int					iErr;

	__try
	{
		wVersionRequested = MAKEWORD(2, 2);

		dwErr = WSAStartup(wVersionRequested, &wsaData);

		if (dwErr != 0)
		{
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2InitializeServerSock : WSAStartup");
			__leave;
		}


		if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2)
		{
			dwErr = ERROR_NOT_SUPPORTED;
			WSACleanup();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2InitializeServerSock : WSAStartup() requested version not supported!");
			__leave;
		}

		psInst->fWSAInitialized = TRUE;

	}
	__finally
	{


	}

	return dwErr;
}



PRIVATE DWORD QFDMI_L2Initialize_AcceptSocket(QFDMI_L2_INSTANCE * psInst)
{
	DWORD dwErr = NO_ERROR;
	unsigned long	ul = 1;
	int				nClientLength;
	char FAR	*   pucRcvBufSize;
	char FAR	*   pucSndBufSize;
	char FAR	*	pbNoDelayOpt;
	struct sockaddr_in	sServerAddress, sClientAddress;
	DWORD dwBytesCount;

	__try
	{
		//Cleanup and Init with 0 the ServerAddress
		ZeroMemory((char *)&sServerAddress, sizeof(sServerAddress));

		sServerAddress.sin_family = AF_INET;
		sServerAddress.sin_port = htons((USHORT)psInst->sParams.dwListenPort);
		sServerAddress.sin_addr.s_addr = INADDR_ANY; //htonl(*((ULONG *) psInst->config_parameters.aucIP_address));

		psInst->socListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (psInst->socListenSocket == INVALID_SOCKET)
		{
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "ConnectSocket() - open socket error!");
			__leave;
		}

		psInst->fListenSocketInitialized = TRUE;

		if ((bind(psInst->socListenSocket, (struct sockaddr *)&sServerAddress, sizeof(sServerAddress))) == SOCKET_ERROR)
		{
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - assign address socket error!");
			QFDMI_L2Reset_AcceptSocket(psInst);
			__leave;
		}

		//Make the socket a listening socket
		if (SOCKET_ERROR == listen(psInst->socListenSocket, (int)SOMAXCONN))
		{
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - Error occurred while listening!");
			QFDMI_L2Reset_AcceptSocket(psInst);
			__leave;
		}

		// Setup the listening socket for connections
		if ((psInst->socAcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		{
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - Failed to get a socket!");
			QFDMI_L2Reset_AcceptSocket(psInst);
			__leave;
		}

		psInst->fAcceptSocketInitialized = TRUE;

		ZeroMemory(&psInst->ovrListenOverlapped, sizeof(OVERLAPPED));
		WSAResetEvent(psInst->hListenEvent);
		psInst->ovrListenOverlapped.hEvent = psInst->hListenEvent;

		if (AcceptEx(psInst->socListenSocket, psInst->socAcceptSocket, (PVOID)psInst->cAcceptBuffer, 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytesCount, &psInst->ovrListenOverlapped) == FALSE)
		{
			dwErr = WSAGetLastError();

			if (dwErr != ERROR_IO_PENDING)
			{
				QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - AcceptEx() failed with error!");
				QFDMI_L2Reset_AcceptSocket(psInst);
				__leave;
			}
		}
		else
		{
			dwErr = WSAGetLastError();
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - AcceptEx() returned false!");
			QFDMI_L2Reset_AcceptSocket(psInst);
			__leave;
		}

		
		psInst->fAcceptSocketInitialized = TRUE;

		/*
		//MTO
		if (psInst->dwEnableQfrActivation)
		{
			SendReqToInitializeBeaconId(inst_id);
		}
		//END MTO
		*/

	}
	__finally
	{


	}

	return dwErr;
}



PRIVATE DWORD QFDMI_L2Reset_AcceptSocket(QFDMI_L2_INSTANCE * psInst)
{
	if (psInst->fListenSocketInitialized)
		closesocket(psInst->socListenSocket);
	psInst->fListenSocketInitialized = FALSE;

	if (psInst->fAcceptSocketInitialized)
		closesocket(psInst->socAcceptSocket);

	psInst->fAcceptSocketInitialized = FALSE;

	psInst->fAcceptSocketConnected = FALSE;
	psInst->fSocketReceiveInitiated = FALSE;

	////MTO
	//if (psInst->dwEnableQfrActivation)
	//{
	//	SendReqToCloseBeaconIdSocket(inst_id);
	//}
	////END MTO

	return NO_ERROR;
}


PRIVATE DWORD QFDMI_L2Force_CloseConnection(QFDMI_L2_INSTANCE * psInst)
{
	DWORD dwErr = NO_ERROR;

	dwErr = QFDMI_L2Reset_AcceptSocket(psInst);
	dwErr = QFDMI_L2Initialize_AcceptSocket(psInst);
	
	QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DISCONNECTED, NULL);

	return dwErr;
}



PRIVATE BOOL QFDMI_L2RecognizeWholeMessage(QFDMI_L2_INSTANCE * psInst)
{
	//If not all bytes of the current message is received 
	char	szMsg[QFDMI_L2_IN_BUFFER_BYTES];
	char	acLine[QFDMI_L2_IN_BUFFER_BYTES];
	DWORD	dwLineLen = 0, dwNextLineStart = 0;
	DWORD	dwContentLen = 0, dwNumBytesRead = 0;
	DWORD		i;
	BOOL	bSomeBytesReceived = FALSE;
	BOOL	bValidDataFound = FALSE;
	static DWORD dwMsgNotRcognCnt = 0; //Counter for message recognition attempts
	char * pPos = NULL;
	char szTemp[QFDMI_L2_HOOK_BUFFER_BYTES + 100] = { 0 };
	char szTemp2[QFDMI_L2_HOOK_BUFFER_BYTES + 100] = { 0 };

	memset(acLine, 0, sizeof(acLine));
	dwNextLineStart = 0;

	pPos = strstr(psInst->bufrec, DMI_END_STREAM);

	if (pPos != NULL)
	{
		//memcpy(szTemp2, psInst->bufrec, psInst->dwBufRecLen);
		//sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2ProcessWholeMessage: bufrec:[%s]!", szTemp2);
		//QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));

		dwContentLen = ( pPos - psInst->bufrec) + strlen(DMI_END_STREAM);

		if (dwContentLen <= psInst->dwBufRecLen)
		{ 
			bValidDataFound = TRUE;
			// this is the end of complete message
			// we are interested in body content only
				dwMsgNotRcognCnt = 0; //Reset counter
				psInst->dwBufWholeMessageLen = dwContentLen;
				//Detected whole message
				memcpy(psInst->bufWholeMessage, &psInst->bufrec, psInst->dwBufWholeMessageLen);

				//purge the received message from the buffer
				if (psInst->dwBufRecLen >= psInst->dwBufWholeMessageLen)
				{
					psInst->dwBufRecLen = psInst->dwBufRecLen - (psInst->dwBufWholeMessageLen);
					memmove(psInst->bufrec, &psInst->bufrec[psInst->dwBufWholeMessageLen], psInst->dwBufRecLen);
						
					memset(&psInst->bufrec[psInst->dwBufRecLen], 0, sizeof(psInst->bufrec) - psInst->dwBufRecLen);
						
					if (psInst->dwBufRecLen > 0 && strstr(psInst->bufrec, DMI_XML_VERSION) != NULL)
					{
						memcpy(szTemp2, psInst->bufrec, psInst->dwBufRecLen);
						sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2RcognizeWholeMessage() - More than one message in the buffer bufrec:[%s]!", szTemp2);
						QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));

					}
					//else
					//	memset(psInst->bufrec, 0, sizeof(psInst->bufrec));
				}
				else
				{
					QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2RcognizeWholeMessage: psInst->dwBufRecLen < psInst->dwBufWholeMessageLen + dwNextLineStart !!!! \n"));
					//PptQfrDisconnect(inst_id);
					return FALSE;
				}

				//SaveTrace(inst_id, psInst->bufWholeMessage, psInst->dwBufWholeMessageLen, FALSE);
						

				return TRUE;
		}
	}


	if (bValidDataFound == FALSE && psInst->dwBufRecLen > MIN_MSG_HEADER_LENGTH)
	{
		QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2RcognizeWholeMessage: bValidDataFound = FALSE and  psInst->dwBufRecLen > 140!"));
		//PptQfrDisconnect(inst_id);
		return FALSE;
	}


	return FALSE;
}



PROTECTED BOOL QFDMI_L2LanTransmit(QFDMI_L2_INSTANCE * psInst, SOCKET mon_socket, char *buffer, int length)
{
	struct timeval	sDelay;
	fd_set			fdwrite;
	char szTemp[QFDMI_L2_HOOK_BUFFER_BYTES] = { 0 };
	int				iRet, nBytesSent, nSumBytesSent = 0, iLenLeft;


	//initialize delay 
	sDelay.tv_sec = 0; //seconds
	sDelay.tv_usec = 20; //miliseconds

	// Always clear the read set before calling 
	// select()
	FD_ZERO(&fdwrite);

	// Add socket s to the read set
	FD_SET(mon_socket, &fdwrite);
	if ((iRet = select(0, NULL, &fdwrite, NULL, &sDelay)) == SOCKET_ERROR)
	{
		// Error condition
		//FichierTrace(inst_id, "LanTransmit(), select failed");
		return FALSE;
	}

	if (iRet > 0)
	{

		if (FD_ISSET(mon_socket, &fdwrite))
		{
			iLenLeft = length;

			while (nSumBytesSent<length)
			{
				nBytesSent = send(mon_socket, buffer + nSumBytesSent, iLenLeft, 0);

				if (SOCKET_ERROR == nBytesSent)
				{
					// Error condition
					//FichierTrace(inst_id, "LanTransmit(), SOCKET_ERROR");
					return FALSE;
				}
				else
				{
					//EnvoiMessageDop(inst_id, SRV_ESPION_MESSAGE_SERVICE, SRV_ESPION_SORTANT, buffer, 1);
					nSumBytesSent = nSumBytesSent + nBytesSent;
					iLenLeft = length - nSumBytesSent;
				}
			}

			QFDMI_L2_CALL_HOOK(QFDMI_L2_HOOK_OUTPUT | QFDMI_L2_HOOK_FLUSH,
				length,
				buffer);
			sprintf_s(szTemp, sizeof(szTemp), "OUT: [%s]!", buffer);
			QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));

			//SaveTrace(inst_id, buffer, length, TRUE);

			return TRUE;
		}
	}

	//WriteFichierError(inst_id, "LanTransmit(): FAILED!");
	return FALSE;

}




//---------------------------------------------------------------------



PRIVATE void WINAPI QFDMI_Trans_ACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	
	QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_ACTIVATE, 0);
}


PRIVATE void WINAPI QFDMI_Trans_ON_DISCONNECTED(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_StopTrsTimer(psInst);
}	

PRIVATE void WINAPI QFDMI_Trans_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_DEACTIVATE, 0);

}


PRIVATE void WINAPI QFDMI_Trans_ON_DMI_TIMEOUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	QFDMI_L2Force_CloseConnection(psInst);

}


PRIVATE BOOL WINAPI QFDMI_Send_LIFE_EVT(QFDMI_L2_INSTANCE * psInst)
{
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	// first prepare body since content length of the body goes to header
	PrepareLifeMsgAckBody(psInst, psInst->szStreamId, tbBuffBody, &dwBodyLen);
	PrepareMsgHeader(dwBodyLen, tbBuff2Send, &dwBuff2SendLen);

	memcpy(&tbBuff2Send[dwBuff2SendLen], tbBuffBody, dwBodyLen);
	dwBuff2SendLen += dwBodyLen;

	if (!QFDMI_L2LanTransmit(psInst, psInst->socAcceptSocket, tbBuff2Send, dwBuff2SendLen))
	{
		QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_Trans_LIFE_EVT:QFDMI_L2LanTransmit failed!"));
		QFDMI_L2Force_CloseConnection(psInst);
		return FALSE;
	}

	return TRUE;
}


PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	QFDMI_Send_LIFE_EVT(psInst);
}


PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT_AND_ACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	if(QFDMI_Send_LIFE_EVT(psInst))
	{
		QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_ACTIVATE,0);
	}
}

PRIVATE void WINAPI QFDMI_Trans_LIFE_EVT_AND_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	if (QFDMI_Send_LIFE_EVT(psInst))
	{
		QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_DEACTIVATE, 0);
	}
}


PRIVATE void WINAPI QFDMI_Trans_ON_VST_NEW_TRS(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_INTERNAL_MSG sNewInputL7Message;
	
	psInst->fLastTrsPackageDirectionIN = TRUE;

	strcpy_s(psInst->szTrsInProgressLid, sizeof(psInst->szTrsInProgressLid)-1, psInst->szLid);
	strcpy_s(psInst->szTrsInProgressLastStreamId, sizeof(psInst->szTrsInProgressLastStreamId)-1, psInst->szStreamId);

	sNewInputL7Message.bMessageType = QFDMI_SEND_L2_FRAME;
	ConvertHex2Bin(psInst->szInDsrcHexData, sNewInputL7Message.tbL2Bytes, &sNewInputL7Message.dwL2BytesLen);

	dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);

	QFDMI_StartTrsTimer(psInst);
}


PRIVATE void WINAPI QFDMI_Trans_ON_DSRC_FRAME_IN(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_INTERNAL_MSG sNewInputL7Message;

	psInst->fLastTrsPackageDirectionIN = TRUE;

	sNewInputL7Message.bMessageType = QFDMI_SEND_L2_FRAME;
	ConvertHex2Bin(psInst->szInDsrcHexData, sNewInputL7Message.tbL2Bytes, &sNewInputL7Message.dwL2BytesLen);

	dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
}


PRIVATE void WINAPI QFDMI_Trans_ON_DSRC_FRAME_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	psInst->fLastTrsPackageDirectionIN = FALSE;
	
	QFDMI_SendL2Req(psInst, FALSE);

	QFDMI_StopTrsTimer(psInst);
	QFDMI_StartTrsTimer(psInst);
}

PRIVATE void WINAPI QFDMI_Trans_TerminateTRS_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	
	QFDMI_SendL2EchoReq(psInst, FALSE);

	QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_DEACTIVATE, 0);
}


PRIVATE void WINAPI QFDMI_Trans_TerminateTRS_NO_INPUT_DEACTIVATE(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	QFDMI_StopTrsTimer(psInst);

	QFDMI_HTTP_CLIENT_SendMessage(psInst->psClientThrInst, eMsg_QFDMI_DEACTIVATE, 0);
}

PRIVATE void WINAPI QFDMI_Trans_EchoREQ_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_SendL2EchoReq(psInst, FALSE);
}


PRIVATE void WINAPI QFDMI_Trans_EmptyREQ_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	
	if(psInst->fLastTrsPackageDirectionIN)
	{
		QFDMI_SendL2EchoReq(psInst, TRUE);
		psInst->fLastTrsPackageDirectionIN = FALSE;
	}

	QFDMI_StopTrsTimer(psInst);
}


PRIVATE void WINAPI QFDMI_Trans_EchoRSP_IN(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_SendSAP_END(psInst, FALSE);
}




PRIVATE void WINAPI QFDMI_Trans_ON_LAST_DSCR_FRAME_OUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);
	QFDMI_SendL2Req(psInst, TRUE);

	psInst->fLastTrsPackageDirectionIN = FALSE;

	//Reset trs in progress params
	strcpy_s(psInst->szTrsInProgressLid, sizeof(psInst->szTrsInProgressLid) - 1, "");
	strcpy_s(psInst->szTrsInProgressLastStreamId, sizeof(psInst->szTrsInProgressLastStreamId) - 1, "");

	// Since QFREE beacon cannot return response on last message(always Release.Req) we reply with immediate QFDMI_L2_TIMEOUT  to speed up the reactivation
	{
		QFDMI_INTERNAL_MSG		sNewInputL7Message;
		sNewInputL7Message.bMessageType = QFDMI_L2_TIMEOUT;
		sNewInputL7Message.dwL2BytesLen = 0;
		dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
	}

	QFDMI_StopTrsTimer(psInst);
}


PRIVATE void WINAPI QFDMI_Trans_ON_Trs_TIMEOUT(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

	//QFDMI_SendL2EchoReq(psInst, TRUE);

	{
		QFDMI_INTERNAL_MSG		sNewInputL7Message;
		sNewInputL7Message.bMessageType = QFDMI_L2_TIMEOUT;
		sNewInputL7Message.dwL2BytesLen = 0;
		dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
	}

	QFDMI_StopTrsTimer(psInst);
}


PRIVATE void WINAPI QFDMI_Trans_ACTIVATED_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

}

PRIVATE void WINAPI QFDMI_Trans_DEACTIVATED_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_L2_INSTANCE * psInst = AutGetContext(hAut);

}

//------------------------------------------------------------------------------------

PRIVATE DWORD WINAPI QFDMI_ProcessTerminateSession(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_ProcessTerminateSession..."));
	
	QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DEACTIVATE, NULL);

	return dwErr;
}



PRIVATE void  WINAPI QFDMI_getCurrentStatus(IN QFDMI_L2_INSTANCE * psInst, QFDMI_STATUS_RSP * pStatus)
{
	//BOOL bOK = FALSE;
	DWORD dwState = AutGetCurrentState(psInst->hAut);

	BOOL bIsConnected = (dwState < QFDMI_ST_CONNECTED_DEACTIVATE_REQ);

	pStatus->bError = (!bIsConnected)?0:1;
	//pStatus->bMode = ;
	//pStatus->bTransacting ;
}


PRIVATE DWORD WINAPI QFDMI_ProcessStatusReq(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	QFDMI_INTERNAL_MSG		sNewInputL7Message;
	QFDMI_STATUS_RSP sStatus;

	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_ProcessStatusReq..."));

	QFDMI_getCurrentStatus(psInst, &sStatus);

	if(sStatus.bError == 0) //reply only if no connection error - otherwise PRT will assume the wrong beacon state not disconnected
	{
		sNewInputL7Message.bMessageType = QFDMI_INTF_REQ_GETSTATUS;
		sNewInputL7Message.dwL2BytesLen = 1;
		sNewInputL7Message.tbL2Bytes[0] = sStatus.bError;
		//sNewInputL7Message.tbL2Bytes[1] = QFDMI_isTransparentModeOK(psInst);
		//sNewInputL7Message.tbL2Bytes[2] = isTransactionInProgress(psInst);
		//sNewInputL7Message.tbL2Bytes[3] = isTransactionInProgress(psInst);

		dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);
	}

	return dwErr;
}



PRIVATE DWORD WINAPI QFDMI_ProcessModeTransparentReq(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	QFDMI_INTERNAL_MSG		sNewInputL7Message;
	QFDMI_STATUS_RSP sStatus;

	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_ProcessModeTransparentReq..."));

	QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_ACTIVATE, NULL);

	//reply with mode status message
	QFDMI_getCurrentStatus(psInst, &sStatus);
	sNewInputL7Message.bMessageType = QFDMI_INTF_REQ_MODE_TRANSPARENT;
	sNewInputL7Message.dwL2BytesLen = 1;
	sNewInputL7Message.tbL2Bytes[0] = sStatus.bError;
	
	//TODO Add other status flags...
	//sNewInputL7Message.tbL2Bytes[1] = QFDMI_isTransparentModeOK(psInst);

	dwErr = QFDMI_EncodeAndReplyWithL7Packet(psInst, &sNewInputL7Message);

	return dwErr;
}


PRIVATE DWORD WINAPI QFDMI_ProcessL2_BST_Message(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	//BCM_ERR			iLastBCMErr = BCM_NoErr;
	//BYTE			byBeaconIDMode;

	//QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_ProcessL2_BST_Message: "));

	//TO DO 
	//if the beacon is already polling than only return status
	//if not polling send request to port 80 to enable polling and eventually change beaconId 
	QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_ACTIVATE, NULL);

	return dwErr;
}


PRIVATE DWORD WINAPI QFDMI_ProcessL2Message(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	if (psInst->fLastTrsPackageDirectionIN)
	{
		if (!psInst->fLastMessage)
			QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_DSRC_FRAME_OUT, NULL);
		else
			QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_LAST_DSRC_FRAME_OUT, NULL);
		psInst->fLastTrsPackageDirectionIN = FALSE;
	}
	else
	{
		QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_TRS_TIMEOUT, NULL);
	}

	return dwErr;
}



PRIVATE DWORD WINAPI QFDMI_ProcessL2_EmptyReq(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	QFDMI_AutSendEvent(psInst, psInst->hAut, QFDMI_EVT_EMPTY_REQ_OUT, NULL);

	return dwErr;
}

//--------------------------------------------------------------------

PRIVATE void WINAPI QFDMI_ResetDMITimer(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	TimerCancel(psInst->psTimer);
	
	dwErr = TimerActivate(psInst->psTimer, psInst->sParams.dwDMITimerDelayMs);
	if (dwErr != NO_ERROR)
	{
		QFDMI_L2MakeError(psInst, dwErr, "QFDMI_ResetDMITimer:  TimerActivate");
		return;
	}

}


PRIVATE void WINAPI QFDMI_StartTrsTimer(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	dwErr = TimerActivate(psInst->psTrsTimer, psInst->sParams.dwTrsTimerDelayMs);
	if (dwErr != NO_ERROR)
	{
		QFDMI_L2MakeError(psInst, dwErr, "QFDMI_StartTrsTimer:  TimerActivate");
		return;
	}

}

PRIVATE void WINAPI QFDMI_StopTrsTimer(IN QFDMI_L2_INSTANCE * psInst)
{
	DWORD			dwErr = NO_ERROR;

	TimerCancel(psInst->psTrsTimer);

}



PRIVATE DWORD WINAPI QFDMI_SendL2Req(IN QFDMI_L2_INSTANCE * psInst, BOOL fLastMessage)
{
	DWORD			dwErr = NO_ERROR;
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };


	// first prepare body since content length of the body goes to header
	PrepareDMIMsgBody(psInst, psInst->sCurrentDecodedOutL7Message.tbL2Bytes, psInst->sCurrentDecodedOutL7Message.dwL2BytesLen, 
					  fLastMessage, psInst->szTrsInProgressLastStreamId, psInst->szTrsInProgressLid,
					  tbBuffBody, &dwBodyLen);

	PrepareMsgHeader(dwBodyLen, tbBuff2Send, &dwBuff2SendLen);

	memcpy(&tbBuff2Send[dwBuff2SendLen], tbBuffBody, dwBodyLen);
	dwBuff2SendLen += dwBodyLen;

	if (!QFDMI_L2LanTransmit(psInst, psInst->socAcceptSocket, tbBuff2Send, dwBuff2SendLen))
	{
		QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_SendL2Req:QFDMI_L2LanTransmit failed!"));
		QFDMI_L2Force_CloseConnection(psInst);
	}

	return dwErr;
}


PRIVATE DWORD WINAPI QFDMI_SendL2EchoReq(IN QFDMI_L2_INSTANCE * psInst, BOOL bEndTrsInProgress)
{
	DWORD			dwErr = NO_ERROR;
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };
	BYTE	cmdEcho[6] = { 0x91, 0x05, 0x00, 0x0F, 0x02, 0x00 };
	//BYTE	cmdEcho[9] = { 0x91, 0x05, 0x00, 0x0F, 0x02, 0x03, 0xAA, 0x55, 0xAA };

	// first prepare body since content length of the body goes to header
	if(!bEndTrsInProgress)
		PrepareDMIMsgBody(psInst, cmdEcho, sizeof(cmdEcho), FALSE, psInst->szStreamId, psInst->szLid , tbBuffBody, &dwBodyLen);
	else
		PrepareDMIMsgBody(psInst, cmdEcho, sizeof(cmdEcho), FALSE, psInst->szTrsInProgressLastStreamId, psInst->szTrsInProgressLid, tbBuffBody, &dwBodyLen);


	PrepareMsgHeader(dwBodyLen, tbBuff2Send, &dwBuff2SendLen);

	memcpy(&tbBuff2Send[dwBuff2SendLen], tbBuffBody, dwBodyLen);
	dwBuff2SendLen += dwBodyLen;

	if (!QFDMI_L2LanTransmit(psInst, psInst->socAcceptSocket, tbBuff2Send, dwBuff2SendLen))
	{
		QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_SendL2EchoReq:QFDMI_L2LanTransmit failed!"));
		QFDMI_L2Force_CloseConnection(psInst);
	}

	return dwErr;
}


PRIVATE DWORD WINAPI QFDMI_SendSAP_END(IN QFDMI_L2_INSTANCE * psInst, BOOL bEndTrsInProgress)
{
	DWORD			dwErr = NO_ERROR;
	DWORD					dwBodyLen = 0;
	UCHAR					tbBuffBody[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	DWORD					dwBuff2SendLen = 0;
	UCHAR					tbBuff2Send[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };
	BYTE	cmdEcho[6] = { 0x91, 0x05, 0x00, 0x0F, 0x02, 0x00 };

	// first prepare body since content length of the body goes to header
	if (!bEndTrsInProgress)
		PrepareDMI_SAP_CONTINUE_MsgBody(psInst, psInst->szStreamId, psInst->szLid, tbBuffBody, &dwBodyLen);
	else
		PrepareDMI_SAP_CONTINUE_MsgBody(psInst, psInst->szTrsInProgressLastStreamId, psInst->szTrsInProgressLid, tbBuffBody, &dwBodyLen);

	
	PrepareMsgHeader(dwBodyLen, tbBuff2Send, &dwBuff2SendLen);

	memcpy(&tbBuff2Send[dwBuff2SendLen], tbBuffBody, dwBodyLen);
	dwBuff2SendLen += dwBodyLen;

	if (!QFDMI_L2LanTransmit(psInst, psInst->socAcceptSocket, tbBuff2Send, dwBuff2SendLen))
	{
		QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_SendL2EchoReq:QFDMI_L2LanTransmit failed!"));
		QFDMI_L2Force_CloseConnection(psInst);
	}

	return dwErr;
}



PRIVATE DWORD WINAPI QFDMI_EncodeAndReplyWithL7Packet(IN QFDMI_L2_INSTANCE * psInst, QFDMI_INTERNAL_MSG * psNewInputL7Message)
{
	DWORD			dwErr, dwPos;
	QFDMI_L2_PACKET * psL7Packet;
	BOOL			fResult;

	psL7Packet = HeapAlloc(GetProcessHeap(), 0, sizeof(*psL7Packet) + psInst->dwBufWholeMessageLen);

	if (psL7Packet != NULL)
	{
		ZeroMemory(psL7Packet, sizeof(*psL7Packet));

		dwPos = 0;
		psL7Packet->dwBufferBytes = sizeof(psL7Packet->tbBuffer);

		fResult = QFDMI_BuildInternalMessage(psInst, psL7Packet->tbBuffer, &dwPos, &psL7Packet->dwBufferBytes, psNewInputL7Message);
		if (!fResult)
		{
			QFDMI_L2MakeError(psInst, ERROR_INVALID_DATA, "QFDMI_EncodeAndReplyWithL7Packet : ...");
			return ERROR_INVALID_DATA;
		}

		// Repartir à zéro avec le buffer et l'état de reception du message
		psInst->dwBufWholeMessageLen = 0;

		// Ajouter l'item dans la file d'attente
		dwErr = QueueWriteItem(psInst->psReadQueue, psL7Packet, INFINITE);
		if (dwErr != NO_ERROR)
		{
			// Echec dans la file d'attente : On libère le buffer alloué
			// La reception a échouée, on provoque une erreur
			HeapFree(GetProcessHeap(), 0, psL7Packet);
			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_EncodeAndReplyWithL7Packet : QueueWriteItem");
		}
	}

	return dwErr;
}



/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : EXPORT BOOL WINAPI QFDMI_L7BuildInternalMessage(
*                      IN      QFDMI_L7_INSTANCE    * psInst,
*                      IN OUT  BYTE              * pbBuf,
*                      IN OUT  DWORD             * pdwBufBytePos,
*                      IN OUT  DWORD             * pdwBufByteLen,
*                      IN      QFDMI_L7_API         * psDef )
* PARAMETRES: psInst        : Handle d'instance créée avec QFDMI_L7Open().
*             pbBuf         : Pointe sur le buffer destiné à recevoir le message généré
*             pdwBufBytePos : En entrée, position du premiere octet du buffer
*                      à utiliser. En sortie, position dans le buffer de l'octet
*                      immédiatement aprés le message généré.
*             pdwBufByteLen : En entrée, nombre d'octet disponibles dans le
*                      buffer à partir de la position donnée.
*             psDef         : Définition du message à générer.
* RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à la génération.
* --------------------------------------------------------------------
* ROLE      : Construit un message à destination d'un communicateur.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT BOOL WINAPI QFDMI_BuildInternalMessage(
	IN      QFDMI_L2_INSTANCE    * psInst,
	IN OUT  BYTE              * pbBuf,
	IN OUT  DWORD             * pdwBufBytePos,
	IN OUT  DWORD             * pdwBufByteLen,
	IN      QFDMI_INTERNAL_MSG         *  psDef)
{
	BOOL            fReturn = FALSE;
	DWORD           dwBufLen;
	DWORD           dwPos;
	BYTE            bLen;

	__try
	{
		//
		// S'assurer de la validité des pointeurs et des valeurs
		//
		CHK_PTR(pdwBufBytePos)
		CHK_PTR(pdwBufByteLen)

			dwBufLen = (*pdwBufByteLen);
		dwPos = (*pdwBufBytePos);

		//
		// Verifiy the parameters are coherent
		//
		if (psDef->dwL2BytesLen > sizeof(psDef->tbL2Bytes))
			__leave;

		bLen = (BYTE)psDef->dwL2BytesLen + 2;

		ADD_BYTE(psDef->bMessageType);
		ADD_BYTE(bLen);
		ADD_BYTES(psDef->dwL2BytesLen, psDef->tbL2Bytes);

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




/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : EXPORT BOOL WINAPI QFDMI_DecodeInternalMessage(
*                      IN      QFDMI_L2_INSTANCE    * psInst,
*                      IN OUT  BYTE              * pbBuf,
*                      IN OUT  DWORD             * pdwBufBytePos,
*                      IN OUT  DWORD             * pdwBufByteLen,
*                      OUT     QFDMI_L7_API         * psDef )
* PARAMETRES: psInst        : Handle d'instance créée avec QFDMI_L7Open().
*             pbBuf         : Pointe sur le buffer contenant le message reçu
*             pdwBufBytePos : En entrée, position du premiere octet du buffer
*                      à utiliser. En sortie, position dans le buffer de l'octet
*                      immédiatement aprés le message lu.
*             pdwBufByteLen : En entrée, nombre d'octets disponibles dans le
*                      buffer à partir de la position donnée.
*             psDef         : Définition du message analysé.
* RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à l'analyse.
* --------------------------------------------------------------------
* ROLE      : Analyse un message à reçu d'un communicateur.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT BOOL WINAPI QFDMI_DecodeInternalMessage(
	IN      QFDMI_L2_INSTANCE   * psInst,
	IN OUT  BYTE              * pbBuf,
	IN OUT  DWORD             * pdwBufBytePos,
	IN OUT  DWORD             * pdwBufByteLen,
	OUT     QFDMI_INTERNAL_MSG       * psDef)
{
	BOOL            fReturn = FALSE;
	DWORD           dwBufLen;
	DWORD           dwPos;
	BYTE            bLen;

	__try
	{
		//
		// S'assurer de la validité des pointeurs et des valeurs
		//
		CHK_PTR(pdwBufBytePos)
		CHK_PTR(pdwBufByteLen)

			dwBufLen = (*pdwBufByteLen);
		dwPos = (*pdwBufBytePos);

		//
		// Get the header
		//
		GET_BYTE(psDef->bMessageType); // Response type
		GET_BYTE(bLen);                // Length

		//
		// Make sure the size is correct
		//
		if (bLen < 2)
			__leave;

		psDef->dwL2BytesLen = (DWORD)bLen - 2;
		GET_BYTES(psDef->dwL2BytesLen, psDef->tbL2Bytes);

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


PRIVATE void WINAPI QFDMI_AutSendEvent(QFDMI_L2_INSTANCE   * psInst, H_AUT hAut, DWORD  dwEventId, void * pvParam)
{
	DWORD dwOldState = AutGetCurrentState(psInst->hAut);

	if (dwEventId < QFDMI_EVT_LIFE_EVT || dwEventId > QFDMI_MAX_AUT_EVENTS)
		dwEventId = QFDMI_MAX_AUT_EVENTS;

	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_AutSendEvent... CurrentState:%d ,Event:%s", dwOldState, gaszEventDesc[dwEventId]));

	AutSendEvent(hAut, dwEventId, pvParam);

	DWORD dwNewState = AutGetCurrentState(psInst->hAut);

	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_AutSendEvent... NewState:%d", dwNewState));

}

//-------------------------------------------------------------------

PRIVATE void PrepareMsgHeader(IN DWORD dwContentLen, OUT UCHAR *pucMsgHeader, OUT DWORD *pdwMsgLen)
{
	DWORD	dwCnt = 0;
	char	szTmp[100] = { 0 };


	sprintf(szTmp, "%d", dwContentLen);

	strcpy(pucMsgHeader, DMI_HEADER_LINE_1);
	dwCnt = strlen(pucMsgHeader);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	strcpy(&pucMsgHeader[dwCnt], DMI_HEADER_LINE_2);
	dwCnt += strlen(&pucMsgHeader[dwCnt]);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	strcpy(&pucMsgHeader[dwCnt], DMI_HEADER_LINE_3);
	dwCnt += strlen(&pucMsgHeader[dwCnt]);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	strcpy(&pucMsgHeader[dwCnt], DMI_HEADER_LINE_4);
	dwCnt += strlen(&pucMsgHeader[dwCnt]);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	strcpy(&pucMsgHeader[dwCnt], DMI_HEADER_LINE_5);
	dwCnt += strlen(&pucMsgHeader[dwCnt]);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	strcpy(&pucMsgHeader[dwCnt], DMI_HEADER_LINE_6);
	dwCnt += strlen(&pucMsgHeader[dwCnt]);
	strcpy(&pucMsgHeader[dwCnt], szTmp);
	dwCnt += strlen(szTmp);
	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	pucMsgHeader[dwCnt++] = 0x0D;
	pucMsgHeader[dwCnt++] = 0x0A;

	*pdwMsgLen = dwCnt;
}


PRIVATE void PrepareXMLVersionLine(IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	strcpy(pucMsgBody, DMI_XML_VERSION);
	(*pdwPos) = strlen(pucMsgBody);
	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;

	return;
}


PRIVATE void PrepareSTREAMHeaderLine(QFDMI_L2_INSTANCE * psInst,IN UCHAR *szStreamId , IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	UCHAR	szTmp[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	sprintf(szTmp, "%s\"%s\">", DMI_STREAM_ID, szStreamId);
	strcpy(&pucMsgBody[*pdwPos], szTmp);
	(*pdwPos) += strlen(szTmp);

	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;

	return;
}

PRIVATE void PrepareSTREAMFooterLine(IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	strcpy(&pucMsgBody[(*pdwPos)], DMI_END_STREAM);
	(*pdwPos) += strlen(&pucMsgBody[(*pdwPos)]);
	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;
	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;

	return;
}

PRIVATE void PrepareDSRC_FRAMEHeaderLine(QFDMI_L2_INSTANCE * psInst, IN UCHAR *szLid, IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos, BOOL fLastMessage)
{
	UCHAR	szTmp[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	sprintf(szTmp, "  %s\"%s\" type=\"%d\" >", DMI_DSRC_FRAME_LID, szLid, fLastMessage?1:3);
	strcpy(&pucMsgBody[*pdwPos], szTmp);
	(*pdwPos) += strlen(szTmp);


	return;
}


PRIVATE void PrepareSAP_CONTINUE_Line(QFDMI_L2_INSTANCE * psInst, IN UCHAR *szLid, IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	UCHAR	szTmp[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };

	sprintf(szTmp, " %s\"%s\" request=\"continue\"/>", DMI_SAP_FRAME_LID, szLid);
	strcpy(&pucMsgBody[*pdwPos], szTmp);
	(*pdwPos) += strlen(szTmp);


	return;
}


PRIVATE void PrepareDSRCFooterLine(IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;
	pucMsgBody[(*pdwPos)++] = 0x20;
	pucMsgBody[(*pdwPos)++] = 0x20;
	strcpy(&pucMsgBody[(*pdwPos)], DMI_END_DSRC_FRAME);
	(*pdwPos) += strlen(&pucMsgBody[(*pdwPos)]);
	pucMsgBody[(*pdwPos)++] = 0x0D;
	pucMsgBody[(*pdwPos)++] = 0x0A;

	return;
}


PRIVATE void PrepareAppendDataLine(IN UCHAR *pucData, IN OUT UCHAR *pucMsgBody, IN OUT DWORD *pdwPos)
{
	strcat(&pucMsgBody[(*pdwPos)], pucData);
	(*pdwPos) += strlen(&pucMsgBody[(*pdwPos)]);

	return;
}


PRIVATE void PrepareLifeMsgAckBody(QFDMI_L2_INSTANCE * psInst, IN UCHAR *szStreamId, OUT UCHAR *pucMsgBody, OUT DWORD *pdwMsgLen)
{
	DWORD	dwCnt = 0;

	// xml_version
	PrepareXMLVersionLine(pucMsgBody, &dwCnt);//<?xml version="1.0"?>

	// stream id
	PrepareSTREAMHeaderLine(psInst, szStreamId, pucMsgBody, &dwCnt);//<stream id="11512">

	// end stream
	PrepareSTREAMFooterLine(pucMsgBody, &dwCnt);//</stream>

	*pdwMsgLen = dwCnt;
}



/*
<?xml version="1.0"?>
<stream id="10168">
<dsrc_frame lid="F4 BA 4A F3" type="3" (or type="1" if last message )>
****HEX STR DATA*****
</dsrc_frame>
</stream>
*/
PRIVATE void PrepareDMIMsgBody(QFDMI_L2_INSTANCE * psInst,
	IN UCHAR *tbPacketBuffer,
	IN DWORD dwPacketBufferBytes,
	IN BOOL fLastMessage,
	IN UCHAR *szStreamId,
	IN UCHAR *szLid,
	OUT UCHAR *pucMsgBody,
	OUT DWORD *pdwMsgLen)

{
	DWORD	dwCnt = 0;
	UCHAR	szTmp[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };


	// xml_version
	PrepareXMLVersionLine(pucMsgBody, &dwCnt);//<?xml version="1.0"?>

	// stream id
	PrepareSTREAMHeaderLine(psInst, szStreamId, pucMsgBody, &dwCnt);//<stream id="11512">

	// dsrc frame lid
	PrepareDSRC_FRAMEHeaderLine(psInst, szLid, pucMsgBody, &dwCnt, fLastMessage);//<dsrc_frame lid="18 B0 90 0D" type=3 (3 or 1 if release) >
	
	//Append DSRC FRAME data
	ConvertBin2Hex(tbPacketBuffer, dwPacketBufferBytes, szTmp);
	PrepareAppendDataLine(szTmp, pucMsgBody, &dwCnt);

	// end dsrc frame
	PrepareDSRCFooterLine(pucMsgBody, &dwCnt);//</dsrc_frame>

	// end stream
	PrepareSTREAMFooterLine(pucMsgBody, &dwCnt);//</stream>


	*pdwMsgLen = dwCnt;
}



/*
<?xml version="1.0"?>
<stream id="10168">
<sap lid="F4 BA 4A F3" request="continue"/>
</stream>
*/
PRIVATE void PrepareDMI_SAP_CONTINUE_MsgBody(QFDMI_L2_INSTANCE * psInst,
	IN UCHAR *szStreamId,
	IN UCHAR *szLid,
	OUT UCHAR *pucMsgBody,
	OUT DWORD *pdwMsgLen)

{
	DWORD	dwCnt = 0;
	UCHAR	szTmp[QFDMI_L2_OUT_BUFFER_BYTES] = { 0 };


	// xml_version
	PrepareXMLVersionLine(pucMsgBody, &dwCnt);//<?xml version="1.0"?>

	// stream id
	PrepareSTREAMHeaderLine(psInst, szStreamId, pucMsgBody, &dwCnt);//<stream id="11512">

	// SAP frame lid
	PrepareSAP_CONTINUE_Line(psInst, szLid, pucMsgBody, &dwCnt);//<sap lid="F4 BA 4A F3" request="continue"/>

	// end stream
	PrepareSTREAMFooterLine(pucMsgBody, &dwCnt);//</stream>


	*pdwMsgLen = dwCnt;
}

