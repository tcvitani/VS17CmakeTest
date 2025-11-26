#ifndef QFDMI_HTTP_CLIENT_H
#define QFDMI_HTTP_CLIENT_H

#define QFDMI_HTTP_IN_BUFFER_BYTES 8096

typedef enum
{
	eMsg_QFDMI_ACTIVATE,
	eMsg_QFDMI_ACTIVATE_RSP,
	eMsg_QFDMI_DEACTIVATE,
	eMsg_QFDMI_DEACTIVATE_RSP,
	eMsg_QFDMI_VST_RECEIVED,

} enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE;


typedef struct
{
	BYTE              bMessageType;
	DWORD             dwData;

}
QFDMI_HTTP_CLIENT_INTERNAL_MSG;



typedef enum _enumQFDMI_HTTP_CLIENT_EVT {
	QFDMI_HTTP_CLIENT_EVT_END_INDEX = 0,
	QFDMI_HTTP_CLIENT_EVT_SOCK_CONNECT_INDEX,
	QFDMI_HTTP_CLIENT_EVT_SOCK_RECEIVE_INDEX,
	QFDMI_HTTP_CLIENT_EVT_TIMER_INDEX,
	QFDMI_HTTP_CLIENT_EVT_CMD_QUEUE_INDEX,
	QFDMI_HTTP_CLIENT_EVT_COUNT
}enumQFDMI_HTTP_CLIENT_EVT;


typedef struct _QFDMI_HTTP_CLIENT_INSTANCE
{
	struct _QFDMI_L2_INSTANCE * psInst;

	// Paramètres de l'instance
	QFR_DMI_PARAMS        sParams;

	// Handle du thread de gestion du protocole
	HANDLE              hThread;

	// Objet "timer" pour la gestion des délais dans le protocole
	TIMER_INSTANCE    * psTimer;

	// Objet "automate" pour la gestion de l'automate
	H_AUT               hAut;

	// File d'attente pour les messages sortant
	QUEUE_INSTANCE    * psClientWriteQueue;

	// File d'attente pour les messages entrant
	QUEUE_INSTANCE    * psClientReadQueue;

	// Evénement signalé si la file psReadQueue a au moins un élément
	// et non signalé si la psReadQueue est vide
	HANDLE              hEndEvent;

	// Handle of the event for client connect
	HANDLE              hConnectEvent;
	// Handle of the event for async i/o
	HANDLE              hReceiveEvent;

	// Tableau des handles signalables servant à animer l'automate
	HANDLE              thClientEvents[QFDMI_HTTP_CLIENT_EVT_COUNT];

	BOOLEAN             fClientSocketInitialized;
	BOOLEAN             fClientSocketConnected;
	BOOLEAN             fClientSocketReceiveInitiated;

	SOCKET	socClientSocket;
	WSAOVERLAPPED ovrConnectionOverlapped;
	WSAOVERLAPPED ovrReceptionOverlapped;
	WSABUF		  sWsaDataBuf;

	// Codes d'erreur courant du protocole
	DWORD               dwLastError;
	char                szLastError[256];

	// En phase d'émission, packet courant à émettre
	QFDMI_INTERNAL_MSG	*psCurrentPacket;

	// Message from application level that possibly contains the L2 level packet to be emitted to the beacon 
	QFDMI_INTERNAL_MSG	sCurrentDecodedMessage;

	// Taille occupée dans le buffer de réception
	BYTE				bufrec[QFDMI_HTTP_IN_BUFFER_BYTES]; // Reception buffer
	// Buffer de réception
	DWORD				dwBufRecLen;

	BYTE				bufWholeMessage[QFDMI_HTTP_IN_BUFFER_BYTES]; //Whole msg Reception buffer
	DWORD				dwBufWholeMessageLen;

	DWORD dwCurrentBeaconId;
	DWORD dwNextBeaconId;


}
QFDMI_HTTP_CLIENT_INSTANCE;

//-------------------------------------------

DWORD WINAPI QFDMI_HTTP_CLIENT_Open(
		IN QFDMI_L2_INSTANCE * psInst,
		IN QFR_DMI_PARAMS * psParams,
		OUT QFDMI_HTTP_CLIENT_INSTANCE ** ppsClientThrInst);

void WINAPI QFDMI_HTTP_CLIENT_Close( 
        IN QFDMI_HTTP_CLIENT_INSTANCE * psClientInst,
        IN DWORD            dwTimeout );


DWORD WINAPI QFDMI_HTTP_CLIENT_SendMessage(
		IN              QFDMI_HTTP_CLIENT_INSTANCE  * psClientInst,
		IN              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         eMsgType,
		IN              DWORD             dwData);

DWORD WINAPI QFDMI_HTTP_CLIENT_ReceiveMessage(
		IN      QFDMI_HTTP_CLIENT_INSTANCE  *psClientInst,
		OUT              enuQFDMI_HTTP_CLIENT_INTERNAL_MSGS_TYPE         *peMsgType,
		OUT              DWORD             *pdwData,
		IN      DWORD           dwTimeout);



#endif