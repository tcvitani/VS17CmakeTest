//--------------------------------------------------------------------
// Sanef ITS(C) - All rights reserved
//--------------------------------------------------------------------
 //--------------------------------------------------------------------
 // QFR HTTP client used to ACTIVATE/DEACTIVATE/CHANGE BEACON ID
 //---------------------------------------------------------------------


#include <winsock2.h>
// Link to mswsock.lib
#include <mswsock.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>
#include <csr_queue.h>
#include <csr_timer.h>

#include "auto.h"
#include "csr_tag_QFDMI.h"
#include "QFDMI_http_client.h"


#include <memclass.h>



PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_RunThread(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ClearQueue(QUEUE_INSTANCE * psQueue);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_InitializeConnectSock(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_HandleNewConnection(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE void WINAPI QFDMI_HTTP_CLIENT_HandleCmdQueue(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE void WINAPI QFDMI_HTTP_CLIENT_ReplyOnCmd(IN              QFDMI_HTTP_CLIENT_INSTANCE  * psClientInst,
		IN              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         eMsgType,
		IN              DWORD             dwData);

PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ResetSocket(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_InitiateWSARecv(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE void WINAPI QFDMI_HTTP_CLIENT_HandleClientSock(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE BOOL WINAPI QFDMI_HTTP_CLIENT_RecognizeWholeMessage(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE void WINAPI QFDMI_HTTP_CLIENT_ProcessWholeMessage(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);

PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ForceResetConnection(IN QFDMI_HTTP_CLIENT_INSTANCE* psClientInst);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ProcessActivate(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);
PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ProcessDeactivate(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);

PRIVATE void WINAPI QFDMI_CTRL_PrepareNextBeaconID(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst);

//---------------------------------------------------------------------------
//
// Définitions des événements of actiation/deactivation control automaton
//
typedef enum
{
	QFDMI_CTRL_EVT_SOCK_CONNECTED = 0,
	QFDMI_CTRL_EVT_ACTIVATE_REQ,
	QFDMI_CTRL_EVT_DEACTIVATE_REQ,
	QFDMI_CTRL_EVT_HTTP_RSP_OK,
	QFDMI_CTRL_EVT_VST_RECEIVED,
	QFDMI_CTRL_MAX_AUT_EVENTS

}
QFDMI_AUT_EVENTS;

char *gaszQFDMI_CTRL_EventDesc[] =
{
	"QFDMI_CTRL_EVT_SOCK_CONNECTED",
	"QFDMI_CTRL_EVT_ACTIVATE_REQ",
	"QFDMI_CTRL_EVT_DEACTIVATE_REQ",
	"QFDMI_CTRL_EVT_HTTP_RSP_OK",
	"QFDMI_CTRL_EVT_VST_RECEIVED"
};

//
// Sates of the automaton with MDREnable functionality 
//
typedef enum
{
	QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK = 0,
	QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK,
	QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,
	QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,
	QFDMI_CTRL_ATM1_ST_ACTIVATED,
	QFDMI_CTRL_ATM1_ST_DEACTIVATING,
	QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID,

	QFDMI_CTRL_ATM1_MAX_AUT_STATES
}
QFDMI_CTRL_AUT1_STATES;


//
// Sates of the automaton without MDREnable functionality (only Change beacon ID)
//
typedef enum
{
	QFDMI_CTRL_ATM2_ST_INACTIVE = 0,
	QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,
	QFDMI_CTRL_ATM2_ST_ACTIVATED,
	QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,

	QFDMI_CTRL_ATM2_MAX_AUT_STATES
}
QFDMI_CTRL_AUT2_STATES;


PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeBeaconID(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeMDR_ON(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeMDR_OFF(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_CTRL_Trans_ResetConn(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_CTRL_Trans_Activated_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);
PRIVATE void WINAPI QFDMI_CTRL_Trans_Dectivated_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam);


#define			HTTP_1_1							" HTTP/1.1\n\n"

//---------------------------------------------------------------------------
struct mswsock_s {
	LPFN_CONNECTEX ConnectEx;
} mswsock;
//---------------------------------------------------------------------------

static BOOL load_mswsock(void)
{
	SOCKET sock;
	DWORD dwBytes;
	int rc;

	/* Dummy socket needed for WSAIoctl */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return FALSE;

	{
		GUID guid = WSAID_CONNECTEX;
		rc = WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &guid, sizeof(guid),
					  &mswsock.ConnectEx, sizeof(mswsock.ConnectEx),
					  &dwBytes, NULL, NULL);
		if (rc != 0)
			return FALSE;
	}

	rc = closesocket(sock);
	if (rc != 0)
		return FALSE;

	return TRUE;
}




PRIVATE void WINAPI QFDMI_HTTP_CLIENT_AutSendEvent(QFDMI_HTTP_CLIENT_INSTANCE * psClientInst, H_AUT hAut, DWORD  dwEventId, void * pvParam)
{
	DWORD dwOldState = AutGetCurrentState(psClientInst->hAut);

	if (dwEventId < QFDMI_CTRL_EVT_SOCK_CONNECTED || dwEventId > QFDMI_CTRL_MAX_AUT_EVENTS)
		dwEventId = QFDMI_CTRL_MAX_AUT_EVENTS;

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_AutSendEvent... CLIENT_CurrentState:%d ,Event:%s", dwOldState, gaszQFDMI_CTRL_EventDesc[dwEventId]));

	AutSendEvent(hAut, dwEventId, pvParam);

	DWORD dwNewState = AutGetCurrentState(psClientInst->hAut);

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_AutSendEvent... CLIENT_NEWState:%d", dwNewState));

}



DWORD WINAPI QFDMI_HTTP_CLIENT_Open( 
		IN QFDMI_L2_INSTANCE * psInst,
		IN QFR_DMI_PARAMS * psParams,
        OUT QFDMI_HTTP_CLIENT_INSTANCE ** ppsClientThrInst)
{
    DWORD               dwErr       = NO_ERROR;
    QFDMI_HTTP_CLIENT_INSTANCE      * psClientInst      = NULL;
    DWORD               dwId;

    __try
    {
        //
        // Valeur par défaut retournée : NULL
        //
        (*ppsClientThrInst) = NULL;

        //
        // Allouer la place pour la structure d'instance et la remplir de 0
        //
        psClientInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(QFDMI_HTTP_CLIENT_INSTANCE) );
        if ( psClientInst == NULL ) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

		psClientInst->psInst = psInst;
        psClientInst->sParams = *psParams;

        //
        // Créer l'événement utilisé pour signaler la fin.
        //
        psClientInst->hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psClientInst->hEndEvent == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Create event for new connections
        //
		psClientInst->hConnectEvent = WSACreateEvent();
		if (psClientInst->hConnectEvent == NULL)
        {
            dwErr = GetLastError();
            __leave;
        }


		//
		// Create event to listen events from connected socket
		//
		psClientInst->hReceiveEvent = WSACreateEvent();
		if (psClientInst->hReceiveEvent == NULL)
		{
			dwErr = GetLastError();
			__leave;
		}
			

        //
        // Initialisation des files d'attente
        //
        dwErr = QueueOpen( &psClientInst->psClientReadQueue, psClientInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;

		dwErr = QueueOpen(&psClientInst->psClientWriteQueue, psClientInst->sParams.dwMaxPendingMsg);
		if (dwErr != NO_ERROR)
			__leave;


		//
		// Creation du timer 
		//
		dwErr = TimerOpenEx(TIMER_TYPE_WAITABLE | TIMER_MAKE_RESOLUTION(4), &psClientInst->psTimer);
		if (dwErr != NO_ERROR)
			__leave;

		//
        // Initialisation de la liaison
        //
		if (!load_mswsock()) {
			//printf("Error loading mswsock functions: %d\n", WSAGetLastError());
			dwErr = ERROR_BAD_ENVIRONMENT;
			_leave;
		}

        dwErr = QFDMI_HTTP_CLIENT_InitializeConnectSock(psClientInst);
        if ( dwErr != NO_ERROR)// && dwErr != ERROR_IO_PENDING)
            __leave;


        //
        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        //
		psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_END_INDEX] = psClientInst->hEndEvent;
		psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_SOCK_CONNECT_INDEX] = psClientInst->hConnectEvent;
		psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_SOCK_RECEIVE_INDEX] = psClientInst->hReceiveEvent;
		psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_TIMER_INDEX] = TimerGetWaitableHandle(psClientInst->psTimer);
		psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_CMD_QUEUE_INDEX] = QueueGetWaitableHandle(psClientInst->psClientWriteQueue);


		if(psClientInst->sParams.dwUseMDREnable!=0)
		{
			QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_Open Creating dwUseMDREnable!=0 -Using ATM type 1 "));

			// Creation of the automate that will use MDREnable parameter
			psClientInst->hAut = AutOpen(QFDMI_CTRL_ATM1_MAX_AUT_STATES, psClientInst);
			if (psClientInst->hAut == NULL)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			// Définition des états de l'automate protocolaire
			if ((!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING))||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID))
			
				)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			// Définition des transitions de l'automate protocolaire
			if ((!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK, QFDMI_CTRL_EVT_SOCK_CONNECTED,		QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK,	QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK, QFDMI_CTRL_EVT_ACTIVATE_REQ,			QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK, AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK, QFDMI_CTRL_EVT_DEACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK, QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK, QFDMI_CTRL_EVT_VST_RECEIVED,			QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_Trans_ResetConn)) ||
															  																				   
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK, QFDMI_CTRL_EVT_SOCK_CONNECTED,			QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK, QFDMI_CTRL_EVT_ACTIVATE_REQ,			QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_Trans_ChangeMDR_ON)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK, QFDMI_CTRL_EVT_DEACTIVATE_REQ,			QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK, QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_OK, QFDMI_CTRL_EVT_VST_RECEIVED,			QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_Trans_ResetConn)) ||
															  																				   
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_EVT_SOCK_CONNECTED,		QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK, QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_EVT_ACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK, AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_EVT_DEACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK,  QFDMI_CTRL_EVT_VST_RECEIVED,		QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_Trans_ResetConn)) ||
															  																				  
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_EVT_SOCK_CONNECTED,		QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_Trans_ChangeMDR_ON)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_EVT_ACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_EVT_DEACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_Trans_ResetConn)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_Trans_Activated_OK)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_OK,	QFDMI_CTRL_EVT_VST_RECEIVED,		QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_Trans_ResetConn)) ||
															  																				   
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_EVT_SOCK_CONNECTED,		QFDMI_CTRL_ATM1_ST_ACTIVATED,				AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_EVT_ACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_EVT_DEACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_Trans_ChangeMDR_OFF)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_ACTIVATED,				AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED,				QFDMI_CTRL_EVT_VST_RECEIVED,		QFDMI_CTRL_ATM1_ST_ACTIVATED,				AUT_EMPTY_FUNCTION)) ||
															  																				   
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_EVT_SOCK_CONNECTED,		QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_Trans_ChangeMDR_OFF)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_EVT_ACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_ACTIVATING_BEACON_ID_NOK, QFDMI_CTRL_Trans_ResetConn)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_EVT_DEACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_DEACTIVATING,				AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_EVT_HTTP_RSP_OK,			QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK,	QFDMI_CTRL_Trans_Dectivated_OK)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_DEACTIVATING,				QFDMI_CTRL_EVT_VST_RECEIVED,		QFDMI_CTRL_ATM1_ST_DEACTIVATING,				AUT_EMPTY_FUNCTION)) ||
															  																				   
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID, QFDMI_CTRL_EVT_SOCK_CONNECTED,	QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID,		QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID, QFDMI_CTRL_EVT_ACTIVATE_REQ,		QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID,		AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID, QFDMI_CTRL_EVT_DEACTIVATE_REQ,	QFDMI_CTRL_ATM1_ST_DEACTIVATING,					QFDMI_CTRL_Trans_ChangeMDR_OFF)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID, QFDMI_CTRL_EVT_HTTP_RSP_OK,		QFDMI_CTRL_ATM1_ST_ACTIVATED,						QFDMI_CTRL_Trans_Activated_OK))  ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_ACTIVATED_CHANGE_BEACON_ID, QFDMI_CTRL_EVT_VST_RECEIVED,		QFDMI_CTRL_ATM1_ST_ACTIVATED,						QFDMI_CTRL_Trans_ResetConn))
				)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			//
			// Placer l'automate dans son état initial
			//
			if (!AutSetCurrentState(psClientInst->hAut, QFDMI_CTRL_ATM1_ST_INACTIVE_BEACON_ID_NOK))
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}
		}
		else
		{
			QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_Open Creating dwUseMDREnable==0 -Using ATM type 2 "));

			// Creation of the automate that will use MDREnable parameter
			psClientInst->hAut = AutOpen(QFDMI_CTRL_ATM2_MAX_AUT_STATES, psClientInst);
			if (psClientInst->hAut == NULL)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			// Définition des états de l'automate protocolaire
			if ((!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED)) ||
				(!AutAddState(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID)) 
				)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			// Définition des transitions de l'automate protocolaire
			if (
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE,						QFDMI_CTRL_EVT_SOCK_CONNECTED,	QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE,						QFDMI_CTRL_EVT_ACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE,						QFDMI_CTRL_EVT_DEACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE,						QFDMI_CTRL_EVT_HTTP_RSP_OK,		QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE,						QFDMI_CTRL_EVT_VST_RECEIVED,	QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||

				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,		QFDMI_CTRL_EVT_SOCK_CONNECTED,	QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,	QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,		QFDMI_CTRL_EVT_ACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,	AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,		QFDMI_CTRL_EVT_DEACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,		QFDMI_CTRL_EVT_HTTP_RSP_OK,		QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_Trans_Activated_OK)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATING_BEACON_ID_NOK,		QFDMI_CTRL_EVT_VST_RECEIVED,	QFDMI_CTRL_ATM2_ST_ACTIVATED,					AUT_EMPTY_FUNCTION)) ||

				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_EVT_SOCK_CONNECTED,	QFDMI_CTRL_ATM2_ST_ACTIVATED,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_EVT_ACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_EVT_DEACTIVATE_REQ,  QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_EVT_HTTP_RSP_OK,		QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_Trans_Activated_OK)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_EVT_VST_RECEIVED,	QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_Trans_ResetConn)) ||

				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_EVT_SOCK_CONNECTED,	QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,  QFDMI_CTRL_Trans_ChangeBeaconID)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_EVT_ACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,  AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_EVT_DEACTIVATE_REQ,	QFDMI_CTRL_ATM2_ST_INACTIVE,					AUT_EMPTY_FUNCTION)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_EVT_HTTP_RSP_OK,		QFDMI_CTRL_ATM2_ST_ACTIVATED,					QFDMI_CTRL_Trans_Activated_OK)) ||
				(!AutAddTransition(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_ACTIVATED_CHANGE_BEACON_ID,	QFDMI_CTRL_EVT_VST_RECEIVED,	QFDMI_CTRL_ATM2_ST_ACTIVATED,					AUT_EMPTY_FUNCTION)) 

				)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			//
			// Placer l'automate dans son état initial
			//
			if (!AutSetCurrentState(psClientInst->hAut, QFDMI_CTRL_ATM2_ST_INACTIVE))
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}
		}



		QFDMI_CTRL_PrepareNextBeaconID(psClientInst);

		//
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFDMI_L2RunThread, (void*)psInst, 0, &dwId, "TAG_QFDMI_L2_PROTOCOL" );
        psClientInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFDMI_HTTP_CLIENT_RunThread, (void*)psClientInst, 0, &dwId );
        if ( psClientInst->hThread == NULL )
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
        (*ppsClientThrInst) = psClientInst;
        psClientInst = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psClientInst != NULL )
            QFDMI_HTTP_CLIENT_Close( psClientInst, 0 );

    }

    return dwErr;
}





void WINAPI QFDMI_HTTP_CLIENT_Close( 
        IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst,
        IN DWORD            dwTimeout )
{
    if ( psClientInst != NULL )
    {
        if ( psClientInst->hThread != NULL )
        {
            timeEndPeriod( 1 );
            SetEvent( psClientInst->hEndEvent );
            WaitForSingleObject( psClientInst->hThread, dwTimeout );
            TerminateThread( psClientInst->hThread, NO_ERROR );
            CloseHandle( psClientInst->hThread );
        }

		if (psClientInst->hAut != NULL)
			AutClose(psClientInst->hAut);

        if ( psClientInst->psClientReadQueue != NULL )
            QueueClose( psClientInst->psClientReadQueue );
        if ( psClientInst->psClientWriteQueue != NULL )
            QueueClose( psClientInst->psClientWriteQueue );
        if ( psClientInst->hEndEvent != NULL )
            CloseHandle( psClientInst->hEndEvent );
		if (psClientInst->hConnectEvent != NULL)
			WSACloseEvent(psClientInst->hConnectEvent);
		if (psClientInst->hReceiveEvent != NULL)
			WSACloseEvent(psClientInst->hReceiveEvent);

		if (psClientInst->psTimer != NULL)
			TimerClose(psClientInst->psTimer);

        HeapFree( GetProcessHeap(), 0, psClientInst );
    }
}

DWORD WINAPI QFDMI_HTTP_CLIENT_SendMessage(
		IN              QFDMI_HTTP_CLIENT_INSTANCE  * psClientInst,
		IN              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         eMsgType,
		IN              DWORD             dwData)
{
	DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psPacket = NULL;

	__try
	{
		// Prepare un "packet" contenant le message à émettre
		//
		psPacket = HeapAlloc(GetProcessHeap(), 0, sizeof(*psPacket) );
		if (psPacket == NULL)
		{
			dwErr = ERROR_NOT_ENOUGH_MEMORY;
			__leave;
		}

		//TO DO missing information on type of message
		psPacket->bMessageType = eMsgType;
		psPacket->dwData = dwData;

		// Placer le "packet" en file d'attente (attente infinie en cas de saturation
		// de la file).
		//
		dwErr = QueueWriteItem(psClientInst->psClientWriteQueue, psPacket, INFINITE);
		if (dwErr != NO_ERROR)
			__leave;

		psPacket = NULL;
		dwErr = NO_ERROR;
	}
	__finally
	{
		if (psPacket != NULL)
			HeapFree(GetProcessHeap(), 0, psPacket);

	}

	return dwErr;
}

DWORD WINAPI QFDMI_HTTP_CLIENT_ReceiveMessage(
		IN      QFDMI_HTTP_CLIENT_INSTANCE  *psClientInst,
		OUT              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         *peMsgType,
		OUT              DWORD             *pdwData,
		IN      DWORD           dwTimeout)
{
	DWORD           dwErr = NO_ERROR;
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psPacket;

	__try
	{
		//
		// Initialiser avec les valeurs par défaut (aucun message en attente).
		//
		(*peMsgType) = 0;
		(*pdwData) = 0;

		//
		// Extraire un élément de la file d'attente
		//
		dwErr = QueueReadItem(psClientInst->psClientReadQueue, &psPacket, dwTimeout);
		if (dwErr != NO_ERROR)
		{
			__leave;
		}

		//
		// Mise à jour de la valeur de retour
		//
		(*peMsgType) = psPacket->bMessageType;
		(*pdwData) = psPacket->dwData;

		// Libérer le bloc
		HeapFree(GetProcessHeap(), 0, psPacket);

		// Plus de packet en cours
		psPacket = NULL;
	}
	__finally
	{
		;
	}

	return dwErr;
}


DWORD WINAPI QFDMI_HTTP_CLIENT_RunThread(
		IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD       dwErr;
	DWORD       dwRes;
	DWORD       dwIndex;
	char szTemp[QFDMI_HTTP_IN_BUFFER_BYTES];

	SetThreadPriorityBoost(GetCurrentThread(), TRUE);
	SetThreadPriority(GetCurrentThread(), psClientInst->sParams.dwPriority);

	//
	// Animer l'automate
	//
	while (TRUE)
	{
		dwRes = WaitForMultipleObjects(QFDMI_HTTP_CLIENT_EVT_COUNT, psClientInst->thClientEvents, FALSE, INFINITE);
		dwIndex = dwRes - WAIT_OBJECT_0;

		if (dwIndex == QFDMI_HTTP_CLIENT_EVT_END_INDEX)
		{
			//
			// Signal de fin reçu, on sort de la boucle
			//
			break;
		}

		else if (dwIndex == QFDMI_HTTP_CLIENT_EVT_CMD_QUEUE_INDEX)
		{
			QFDMI_HTTP_CLIENT_HandleCmdQueue(psClientInst);
		}
		else if (dwIndex == QFDMI_HTTP_CLIENT_EVT_SOCK_CONNECT_INDEX)
		{
			WSAResetEvent(psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_SOCK_CONNECT_INDEX]);
			dwErr = QFDMI_HTTP_CLIENT_HandleNewConnection(psClientInst);
			if(dwErr != NO_ERROR && dwErr !=ERROR_IO_PENDING)
				dwErr = QFDMI_HTTP_CLIENT_InitializeConnectSock(psClientInst);
		}
		else if (dwIndex == QFDMI_HTTP_CLIENT_EVT_SOCK_RECEIVE_INDEX)
		{
			WSAResetEvent(psClientInst->thClientEvents[QFDMI_HTTP_CLIENT_EVT_SOCK_RECEIVE_INDEX]);
			QFDMI_HTTP_CLIENT_HandleClientSock(psClientInst);
		}
		else if (dwIndex == QFDMI_HTTP_CLIENT_EVT_TIMER_INDEX)
		{
			// Timer event is trigered by the windows system 
			// if set to be triggered using CSR_TIMER.h function
			// named TimerActivate
			TimerAcknowledge(psClientInst->psTimer); //Acknoledge the timer so the event will not be detected as signaled again

			//QFDMI_HTTP_CLIENT_HandleComTimer(psClientInst);
		}
		else
		{
			sprintf_s(szTemp, sizeof(szTemp), "QFDMI_HTTP_CLIENT_RunThread: WaitForMultipleObjects returned %d \n!", dwRes);
			QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, szTemp));

			//
			// Ignorer
			//
		}
	}

	//
	// Avant la fin du thread, vider la file d'attente en emission
	//
	QFDMI_HTTP_CLIENT_ClearQueue(psClientInst->psClientWriteQueue);
	QFDMI_HTTP_CLIENT_ClearQueue(psClientInst->psClientReadQueue);

	ExitThread(NO_ERROR);

	return NO_ERROR;
}



DWORD WINAPI QFDMI_HTTP_CLIENT_ClearQueue(QUEUE_INSTANCE * psQueue)
{
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psPacket = NULL;
	DWORD       dwErr;

	if (psPacket == NULL)
	{
		//
		// Si aucun packet en cours, récuperer le suivant si présent
		//
		dwErr = QueueReadItem(psQueue, &psPacket, 0);
	}

	//
	// Tant qu'il reste des packets pour l'envoi de messages
	//
	while (psPacket != NULL && dwErr == NO_ERROR)
	{
		// Libérer le bloc
		HeapFree(GetProcessHeap(), 0, psPacket);

		// Plus de packet en cours
		psPacket = NULL;

		//
		// Passer à l'élément suivant de la file
		//
		dwErr = QueueReadItem(psQueue, &psPacket, 0);
	}

	return 	dwErr;
}



PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_InitializeConnectSock(
		IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD dwErr = NO_ERROR;
	unsigned long	ul = 1;
	int				nClientLength;
	char FAR	*   pucRcvBufSize;
	char FAR	*   pucSndBufSize;
	char FAR	*	pbNoDelayOpt;
	struct sockaddr_in	sQFRServerAddress;
	DWORD dwBytesCount;

	__try
	{
		// Setup the listening socket for connections
		if ((psClientInst->socClientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		{
			dwErr = WSAGetLastError();
//			QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - Failed to get a socket!");
			QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
			__leave;
		}

		psClientInst->fClientSocketInitialized = TRUE;

		/* ConnectEx requires the socket to be initially bound. */
		{
			int rc;
			struct sockaddr_in addr;
			ZeroMemory(&addr, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = 0;
			rc = bind(psClientInst->socClientSocket, (SOCKADDR*)&addr, sizeof(addr));
			if (rc != 0) {
				//printf("bind failed: %d\n", WSAGetLastError());
				dwErr = WSAGetLastError();
				QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
				__leave;
			}
		}


		ZeroMemory(&psClientInst->ovrConnectionOverlapped, sizeof(OVERLAPPED));
		WSAResetEvent(psClientInst->hConnectEvent);
		psClientInst->ovrConnectionOverlapped.hEvent = psClientInst->hConnectEvent;

		
		sQFRServerAddress.sin_family = AF_INET;
		sQFRServerAddress.sin_port = htons((USHORT)psClientInst->sParams.dwHTTPServer_Port);
		sQFRServerAddress.sin_addr.s_addr = inet_addr(psClientInst->sParams.szHTTPServerIP);

		if (mswsock.ConnectEx(psClientInst->socClientSocket, (struct sockaddr *)&sQFRServerAddress,  sizeof(struct sockaddr_in), NULL,
			0, &dwBytesCount, &psClientInst->ovrConnectionOverlapped) == FALSE)
		{
			dwErr = WSAGetLastError();

			if (dwErr != ERROR_IO_PENDING)
			{
				//QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - AcceptEx() failed with error!");
				QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
				__leave;
			}
			else
				dwErr = NO_ERROR;
		}
		else
		{
			dwErr = WSAGetLastError();
			//QFDMI_L2MakeError(psInst, dwErr, "QFDMI_L2Initialize_AcceptSocket() - AcceptEx() returned false!");
			QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
			__leave;
		}


	}
	__finally
	{


	}

	return dwErr;
}


PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_HandleNewConnection(
		IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD        dwBytesTransferred = 0;
	DWORD dwFlags = 0;
	DWORD dwErr = NO_ERROR;
	BOOL bNewConnection = FALSE;


	__try
	{

		if (WSAGetOverlappedResult(psClientInst->socClientSocket, &(psClientInst->ovrConnectionOverlapped), &dwBytesTransferred, FALSE, &dwFlags) == FALSE)
		{
			dwErr = WSAGetLastError();

			__leave;
		}
		else
		{
			if (psClientInst->fClientSocketConnected == FALSE)
			{
				bNewConnection = TRUE;

				psClientInst->fClientSocketConnected = TRUE;
				dwErr = QFDMI_HTTP_CLIENT_InitiateWSARecv(psClientInst);
			}
		}
	}
	__finally
	{
		if (dwErr != NO_ERROR && dwErr !=ERROR_IO_PENDING)
		{
			 QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
		}
		else
		{
			if(bNewConnection)
				QFDMI_HTTP_CLIENT_AutSendEvent(psClientInst, psClientInst->hAut, QFDMI_CTRL_EVT_SOCK_CONNECTED, NULL);
		}
	}

	return dwErr;
}

PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ResetSocket(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	
	if (psClientInst->fClientSocketConnected)
	{
		int rc;
		/* Make the socket more well-behaved. */
		rc = setsockopt(psClientInst->socClientSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		if (rc == 0) {
			// This will fail if SO_UPDATE_CONNECT_CONTEXT was not performed. 
			rc = shutdown(psClientInst->socClientSocket, SD_BOTH);
			if (rc != 0) {
				//Error shutting down socket
			}
		}
	}

	if (psClientInst->fClientSocketInitialized)
		closesocket(psClientInst->socClientSocket);

	psClientInst->fClientSocketConnected = FALSE;
	psClientInst->fClientSocketInitialized = FALSE;

	psClientInst->fClientSocketReceiveInitiated = FALSE;
	
	return NO_ERROR;
}


PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_InitiateWSARecv(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD        dwBytesTransferred = 0;
	DWORD		 dwFlags = 0;
	DWORD			dwErr = NO_ERROR;

	__try
	{
		if (!psClientInst->fClientSocketReceiveInitiated)
		{
			ZeroMemory(&(psClientInst->ovrReceptionOverlapped), sizeof(WSAOVERLAPPED));
			psClientInst->ovrReceptionOverlapped.hEvent = psClientInst->hReceiveEvent;
			WSAResetEvent(psClientInst->hReceiveEvent);
			psClientInst->sWsaDataBuf.len = sizeof(psClientInst->bufrec);
			psClientInst->sWsaDataBuf.buf = psClientInst->bufrec;

			// Post a WSARecv request to to begin receiving data on the socket
			if (WSARecv(psClientInst->socClientSocket, &psClientInst->sWsaDataBuf, 1, &dwBytesTransferred, &dwFlags,
				&(psClientInst->ovrReceptionOverlapped), NULL) == SOCKET_ERROR)
			{
				dwErr = WSAGetLastError();
				//if (dwErr != ERROR_IO_PENDING && dwErr != NO_ERROR)
				//	QFDMI_L2MakeError(psClientInst, dwErr, "QFDMI_L2HandleNewConnection() - WSARecv() failed with error!");

				__leave;
			}

			psClientInst->fClientSocketReceiveInitiated = TRUE;
		}
	}
	__finally
	{
		if (dwErr != ERROR_IO_PENDING && dwErr != NO_ERROR)
		{
//			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2InitiateWSARecv:QFDMI_L2Force_CloseConnection- Connection closed..."));
			QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
		}
	}

	return dwErr;
}





PRIVATE void WINAPI QFDMI_HTTP_CLIENT_HandleClientSock(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	int				iErr = NO_ERROR;
	DWORD			dwTransfer;
	DWORD			dwFlags = 0;
	//char szTemp[QFDMI_HTTP_CLIENT_HOOK_BUFFER_BYTES] = { 0 };

	__try
	{

		//  Determine the status of the overlapped
		//  request
		if (WSAGetOverlappedResult(psClientInst->socClientSocket, &(psClientInst->ovrReceptionOverlapped), &dwTransfer, FALSE, &dwFlags) == TRUE)
		{

			// First check to see whether the peer has closed
			// the connection, and if so, close the
			// socket
			if (dwTransfer == 0)
			{
				iErr = WSAGetLastError();

				//if (iErr != NO_ERROR)
				//{
				//	sprintf_s(szTemp, sizeof(szTemp), "FDMI_L2HandleAcceptedSock: dwTransfer = 0 => Error:[%d]!", iErr);
				//	QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
				//}

			}
			else
			{
				if (dwTransfer >= sizeof(psClientInst->bufrec) - psClientInst->dwBufRecLen)
				{
					iErr = ERROR_NOT_ENOUGH_MEMORY;
					//QFDMI_L2MakeError(psInst, iErr, "QFDMI_L2HandleAcceptedSock() - dwTransfer > sizeof(PPT_QFR[inst_id].bufrec) - PPT_QFR[inst_id].dwBufRecLen  error!");
					__leave;
				}

				memcpy(&psClientInst->bufrec[psClientInst->dwBufRecLen], psClientInst->sWsaDataBuf.buf, dwTransfer);
				psClientInst->dwBufRecLen += dwTransfer;

				psClientInst->fClientSocketReceiveInitiated = FALSE;

				//try recognize another package
				if (QFDMI_HTTP_CLIENT_RecognizeWholeMessage(psClientInst))
				{
					//Process new message
					//sprintf_s(szTemp, sizeof(szTemp), "IN: [%s]!", psInst->bufWholeMessage);
					//QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));

					//QFDMI_L2_CALL_HOOK(QFDMI_L2_HOOK_INPUT | QFDMI_L2_HOOK_DATA | QFDMI_L2_HOOK_FLUSH,
					//	psInst->dwBufWholeMessageLen,
					//	psInst->bufWholeMessage);

					QFDMI_HTTP_CLIENT_ProcessWholeMessage(psClientInst);
					//if (iErr != NO_ERROR)
					//{
					//	QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleAcceptedSock: QFDMI_L2ProcessWholeMessage failed:\n"));
					//}
				}

			}


		}
		else
		{
			iErr = WSAGetLastError();

			//if (iErr != WSA_IO_PENDING)
			//{
			//	sprintf_s(szTemp, sizeof(szTemp), "QFDMI_L2HandleAcceptedSock: WSAGetOverlappedResult failed! Error:[%d]!", iErr);
			//	QFDMI_CALL_HOOK_COMMENT((psInst, szTemp));
			//}
		}

	}
	__finally
	{
		if (iErr != NO_ERROR && iErr != WSA_IO_PENDING)
		{
//			QFDMI_CALL_HOOK_COMMENT((psInst, "QFDMI_L2HandleAcceptedSock:QFDMI_L2Force_CloseConnection- Connection closed..."));
			QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
		}
		else if (!psClientInst->fClientSocketReceiveInitiated)
		{
			QFDMI_HTTP_CLIENT_InitiateWSARecv(psClientInst);
		}

	}
}


PRIVATE DWORD WINAPI QFDMI_HTTP_CLIENT_ForceResetConnection(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD dwErr = NO_ERROR;

	//QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_ForceResetConnection... "));

	dwErr = QFDMI_HTTP_CLIENT_ResetSocket(psClientInst);
	dwErr = QFDMI_HTTP_CLIENT_InitializeConnectSock(psClientInst);

	return dwErr;
}


PRIVATE void WINAPI prepareStringForSprintf(char *szTemp, DWORD dwLen)
{
	int i;
	for (i = 0; i < dwLen; i++)
	{
		if (szTemp[i] == '%') szTemp[i] = '#';
	}

	szTemp[dwLen] = '\0';
}

PRIVATE BOOL QFDMI_HTTP_CLIENT_LanTransmit(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst, char *buffer, int length)
{
	struct timeval	sDelay;
	fd_set			fdwrite;
	int				iRet, nBytesSent, nSumBytesSent = 0, iLenLeft;
	char szTemp[QFDMI_HTTP_IN_BUFFER_BYTES] = { 0 };
	char szTemp2[QFDMI_HTTP_IN_BUFFER_BYTES] = { 0 };


	//initialize delay 
	sDelay.tv_sec = 0; //seconds
	sDelay.tv_usec = 20; //miliseconds

	// Always clear the read set before calling 
	// select()
	FD_ZERO(&fdwrite);

	// Add socket s to the read set
	FD_SET(psClientInst->socClientSocket, &fdwrite);
	if ((iRet = select(0, NULL, &fdwrite, NULL, &sDelay)) == SOCKET_ERROR)
	{
		// Error condition
		QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_LanTransmit:select: FAILED!"));

		return FALSE;
	}

	if (iRet > 0)
	{

		if (FD_ISSET(psClientInst->socClientSocket, &fdwrite))
		{
			iLenLeft = length;

			while (nSumBytesSent < length)
			{
				nBytesSent = send(psClientInst->socClientSocket, buffer + nSumBytesSent, iLenLeft, 0);

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

			//memcpy(szTemp, buffer, length);
			//prepareStringForSprintf(szTemp, length);
			//sprintf_s(szTemp2, sizeof(szTemp2), "QFDMI_HTTP_CLIENT_LanTransmit:OUT:[%s]!", szTemp);
			//QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, szTemp2));

			return TRUE;
		}
	}

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_HTTP_CLIENT_LanTransmit:FAILED!"));
	return FALSE;
}





PRIVATE BOOL WINAPI QFDMI_HTTP_CLIENT_RecognizeWholeMessage(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD		i;
	BOOL bResultOK = FALSE;
	char szTemp[QFDMI_HTTP_IN_BUFFER_BYTES] = { 0 };
	char szTemp2[QFDMI_HTTP_IN_BUFFER_BYTES] = { 0 };

	for (i = 0; i <= psClientInst->dwBufRecLen-4; i++)
	{
		if (psClientInst->bufrec[i] == 0x0D && 
			psClientInst->bufrec[i+1] == 0x0A && 
			psClientInst->bufrec[i+2] == 0x0D && 
			psClientInst->bufrec[i+3] == 0x0A) // end of http rsp = 0x0D 0x0A 0x0D 0x0A
		{
			memcpy(psClientInst->bufWholeMessage, psClientInst->bufrec, psClientInst->dwBufRecLen);
			psClientInst->dwBufWholeMessageLen =  psClientInst->dwBufRecLen;

			psClientInst->dwBufRecLen = 0;
			bResultOK = TRUE;

			//memcpy(szTemp, psClientInst->bufWholeMessage, psClientInst->dwBufWholeMessageLen);
			//prepareStringForSprintf(szTemp, psClientInst->dwBufWholeMessageLen);
			//sprintf_s(szTemp2, sizeof(szTemp2), "QFDMI_HTTP_CLIENT_RecognizeWholeMessage:IN:[%s]!", szTemp);
			//QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, szTemp2));

			break;
		}
	}

	return bResultOK;
}



PRIVATE void WINAPI QFDMI_HTTP_CLIENT_ProcessWholeMessage(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD		i;

	for (i = 0; i <= psClientInst->dwBufWholeMessageLen-6; i++)
	{
		//Search for 200 OK
		if(psClientInst->bufWholeMessage[i]=='2' && 
		   psClientInst->bufWholeMessage[i + 1] == '0' &&
		   psClientInst->bufWholeMessage[i + 2] == '0' &&
		   psClientInst->bufWholeMessage[i + 3] == ' ' &&
		   psClientInst->bufWholeMessage[i + 4] == 'O' &&
		   psClientInst->bufWholeMessage[i + 5] == 'K' )
		{

			QFDMI_HTTP_CLIENT_AutSendEvent(psClientInst, psClientInst->hAut, QFDMI_CTRL_EVT_HTTP_RSP_OK, NULL);
			psClientInst->dwBufWholeMessageLen = 0;
			break;
		}

	}

	QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
	return;

}


PRIVATE void WINAPI QFDMI_HTTP_CLIENT_HandleComTimer(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD			dwErr = NO_ERROR;

	__try
	{
		QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
	}
	__finally
	{

	}
}

PRIVATE void WINAPI QFDMI_HTTP_CLIENT_HandleCmdQueue(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	DWORD        dwErr, dwPos, dwLen;
	BOOL		 fResult;
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psCurrentPacket = NULL;
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psTempPacket = NULL;

	__try
	{

		//
		// Si le message n'a pas encore été construit, on l'extrait de la file d'attente
		//
		dwErr = QueueReadItem(psClientInst->psClientWriteQueue, &psCurrentPacket, 0);
		if (dwErr != NO_ERROR)
		{
			//QFDMI_L2MakeError(psInst, dwErr, "QFDMI_HTTP_CLIENT_HandleCmdQueue : QueueReadItem");
			__leave;
		}

		//
		// Dans le mode où les buffers sont vidés automatiquement,
		// on vide la file de réception chaque fois qu'on émet un
		// nouveau message (mode question/réponse).
		//
		if (psClientInst->sParams.dwAutoClearBuffers != 0)
		{
			QFDMI_HTTP_CLIENT_ClearQueue(psClientInst->psClientReadQueue);
		}


		switch (psCurrentPacket->bMessageType)
		{
		case eMsg_QFDMI_ACTIVATE:
				QFDMI_HTTP_CLIENT_AutSendEvent(psClientInst, psClientInst->hAut, QFDMI_CTRL_EVT_ACTIVATE_REQ, NULL);
			break;
		case eMsg_QFDMI_DEACTIVATE:
				QFDMI_HTTP_CLIENT_AutSendEvent(psClientInst, psClientInst->hAut, QFDMI_CTRL_EVT_DEACTIVATE_REQ, NULL);
			break;
		case eMsg_QFDMI_VST_RECEIVED:
				QFDMI_HTTP_CLIENT_AutSendEvent(psClientInst, psClientInst->hAut, QFDMI_CTRL_EVT_VST_RECEIVED, NULL);
			break;

		default:
			//	//QFDMI_L2MakeError(psInst, dwErr, "QFDMI_HTTP_CLIENT_HandleCmdQueue : Unknown internal message!");
			break;

		}


	}
	__finally
	{
		//if (dwErr != NO_ERROR)
		//{
		//	//QFDMI_L2MakeError(psInst, dwErr, "QFDMI_HTTP_CLIENT_HandleCmdQueue : Error processing internal message!");
		//}

		// Libérer le bloc
		HeapFree(GetProcessHeap(), 0, psCurrentPacket);

		// Plus de packet en cours
		psCurrentPacket = NULL;

	}

}



PRIVATE void WINAPI QFDMI_HTTP_CLIENT_ReplyOnCmd(
		IN              QFDMI_HTTP_CLIENT_INSTANCE  * psClientInst,
		IN              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         eMsgType,
		IN              DWORD             dwData)
{
	DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
	QFDMI_HTTP_CLIENT_INTERNAL_MSG    * psPacket = NULL;

	__try
	{
		// Prepare un "packet" contenant le message à émettre
		//
		psPacket = HeapAlloc(GetProcessHeap(), 0, sizeof(*psPacket));
		if (psPacket == NULL)
		{
			dwErr = ERROR_NOT_ENOUGH_MEMORY;
			__leave;
		}

		psPacket->bMessageType = eMsgType;
		psPacket->dwData = dwData;

		dwErr = QueueWriteItem(psClientInst->psClientReadQueue, psPacket, INFINITE);
		if (dwErr != NO_ERROR)
			__leave;

		psPacket = NULL;
		dwErr = NO_ERROR;
	}
	__finally
	{
		if (psPacket != NULL)
			HeapFree(GetProcessHeap(), 0, psPacket);

	}

	return;
}






PRIVATE void WINAPI QFDMI_CTRL_PrepareNextBeaconID(IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst)
{
	psClientInst->dwCurrentBeaconId = psClientInst->dwNextBeaconId;
	psClientInst->dwNextBeaconId++;

	if (psClientInst->dwNextBeaconId > psClientInst->sParams.dwHighBeaconId ||
	   psClientInst->dwNextBeaconId < psClientInst->sParams.dwLowBeaconId)
		psClientInst->dwNextBeaconId = psClientInst->sParams.dwLowBeaconId;

}

//-------------------------------------------------------------------------------------

PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeBeaconID(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);

	char				szHttpRequest[MAX_PATH] = { 0 };

	if (!psClientInst->fClientSocketConnected)
		return;

	sprintf(szHttpRequest, "%s%d%s", psClientInst->sParams.szChangeBeaconID_ParamUrl, psClientInst->dwNextBeaconId, HTTP_1_1);


	if (QFDMI_HTTP_CLIENT_LanTransmit(psClientInst, szHttpRequest, strlen(szHttpRequest)) == FALSE)
	{
		QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_ChangeBeaconID... failed! "));
		QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
	}
	else
	{
		QFDMI_CTRL_PrepareNextBeaconID(psClientInst);
		QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_ChangeBeaconID... dwCurrentBeaconId:%d ", psClientInst->dwCurrentBeaconId));

	}

}

PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeMDR_ON(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);
	char				szHttpRequest[MAX_PATH] = { 0 };

	if (!psClientInst->fClientSocketConnected)
		return;

	sprintf(szHttpRequest, "%s%d%s", psClientInst->sParams.szMdrEnable_ParamUrl, 1, HTTP_1_1);
	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_ChangeMDR_ON... "));

	if (QFDMI_HTTP_CLIENT_LanTransmit(psClientInst, szHttpRequest, strlen(szHttpRequest)) == FALSE)
	{
		QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
	}


}

PRIVATE void WINAPI QFDMI_CTRL_Trans_ChangeMDR_OFF(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);
	char				szHttpRequest[MAX_PATH] = { 0 };

	if (!psClientInst->fClientSocketConnected)
		return;

	sprintf(szHttpRequest, "%s%d%s", psClientInst->sParams.szMdrEnable_ParamUrl, 0, HTTP_1_1);

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_ChangeMDR_OFF... "));

	if (QFDMI_HTTP_CLIENT_LanTransmit(psClientInst, szHttpRequest, strlen(szHttpRequest)) == FALSE)
	{
		QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);
	}


}

PRIVATE void WINAPI QFDMI_CTRL_Trans_ResetConn(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_ResetConn... "));
	QFDMI_HTTP_CLIENT_ForceResetConnection(psClientInst);

}


PRIVATE void WINAPI QFDMI_CTRL_Trans_Activated_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_Activated_OK... "));
	QFDMI_HTTP_CLIENT_ReplyOnCmd(psClientInst, eMsg_QFDMI_ACTIVATE_RSP, 1);
}

PRIVATE void WINAPI QFDMI_CTRL_Trans_Dectivated_OK(H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam)
{
	DWORD dwErr;
	QFDMI_HTTP_CLIENT_INSTANCE * psClientInst = AutGetContext(hAut);

	QFDMI_CALL_HOOK_COMMENT((psClientInst->psInst, "QFDMI_CTRL_Trans_Dectivated_OK... "));

	QFDMI_HTTP_CLIENT_ReplyOnCmd(psClientInst, eMsg_QFDMI_DEACTIVATE_RSP, 1);
}

