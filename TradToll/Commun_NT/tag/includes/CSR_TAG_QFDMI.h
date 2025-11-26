/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFDMI_L2
 * FILE       : CSR_TAG_QFDMI.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Layer 2  QFDMI_L2 - MD5826 
 * --------------------------------------------------------------------
 * DESCRIPTION: Layer 2 interface -	to be used to communicate with 
 *				QFree MD5850 beacon DMI protocol over TCP 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_QFDMI_L2_H
#define CSR_TAG_QFDMI_L2_H


#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_QFDMI_L2_H_PACK, 1 )


//
// Constantes pour l'espionnage de la liaison
//
#define QFDMI_L2_HOOK_DIRECTION_FILTER    0x30000000
#define QFDMI_L2_HOOK_INPUT               0x10000000
#define QFDMI_L2_HOOK_OUTPUT              0x20000000
#define QFDMI_L2_HOOK_TYPE_FILTER         0xC0000000
#define QFDMI_L2_HOOK_PROTOCOL            0x40000000
#define QFDMI_L2_HOOK_DATA                0x80000000
#define QFDMI_L2_HOOK_FLUSH               0x00000001
#define QFDMI_L2_HOOK_COMMENT             0x01000000

// R/W : Flag indiquant que le prochain message est le dernier.
//       Cette option est remise à zéro à chaque émission de message.
#define QFDMI_L2_OPTION_F_LAST_MESSAGE      1

//
// Définition du callback pour l'espionnage
//
typedef void WINAPI QFDMI_L2_HOOK( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes );

//----------------------------------------------
//				DMI PROTOCOL ATTRIBUTES	
//----------------------------------------------
#define MIN_MSG_REC_LENGTH				172 //Life message size
#define MIN_MSG_HEADER_LENGTH			140 //Life message size


// HTTP header of response msg
#define			DMI_HEADER_LINE_1			"HTTP/1.1 200 OK"
#define			DMI_HEADER_LINE_2			"Server: Q-Free/0.9.1"
#define			DMI_HEADER_LINE_3			"Keep-Alive: timeout=60, max=100"
#define			DMI_HEADER_LINE_4			"Connection: Keep-Alive"
#define			DMI_HEADER_LINE_5			"Content-Type: text/xml"
#define			DMI_HEADER_LINE_6			"Content-Length: "


#define			DMI_XML_VERSION				"<?xml version=\"1.0\"?>"
#define			DMI_STREAM_ID				"<stream id="
#define			DMI_TRACKING				"<tracking"
#define			DMI_END_TRACKING			"</tracking>"
#define			DMI_END_STREAM				"</stream>\r\n\r\n"
#define			DMI_END_TAPDU				"</tapdu>"
#define			DMI_END_ATTRIBUTE			"</attribute>"
#define			DMI_END_PARAM				"</param>"
#define			DMI_END_DSRC_FRAME			"</dsrc_frame>"
#define			DMI_DSRC_FRAME_LID			"<dsrc_frame lid="
#define			DMI_SAP_FRAME_LID			"<sap lid="
#define			DMI_DSRC_FRAME_STATUS		"status="
#define			DMI_DSRC_FRAME_TYPE			"type="
#define			DMI_DSRC_TAPDU_TYPE			"<tapdu type="
#define			DMI_VST_EQUIP_CLASS			"<equipment_class>"
#define			DMI_VST_MANUF_ID			"<manufacturer_id>"
#define			DMI_VST_STATUS				"<status>"
#define			DMI_PARAM_TYPE				"<param type="
#define			DMI_MONITOR					"<monitor>"
#define			DMI_VST_OBE_STATUS			"<obe_status>"
#define			DMI_DSCR_TRANSACTION_RESULT	"<dsrc_transaction result="

#define			STATUS_ACK_TIMEOUT			"AckTimeout"
#define			STATUS_RETRY_LIMIT			"RetryLimit"


//
// Paramétrage de l'instance
//
typedef struct _QFDMI_L2_PARAMS
{
    // Taille de la structure
    DWORD               dwStructBytes;

    // Pointeur sur la fonction callback d'espionnage
    QFDMI_L2_HOOK          * pfHook;

    // Contexte utilisateur d'espionnage
    void              * pvHookContext;

    // Maximum number of queued messages (in output and in output buffers)
    DWORD               dwMaxPendingMsg;

    // TCP Server listening port number
    DWORD               dwListenPort;

    // Protocol thread priority
    DWORD               dwPriority;

    // Automatically clear serial buffer
    DWORD               dwAutoClearBuffers;

    //HTTP Communication to be used for (change beacon ID and activate / deactivate radio if avalilable)
	char szHTTPServerIP[1024];
	DWORD				dwHTTPServer_Port;

	DWORD dwDMITimerDelayMs;
	DWORD dwTrsTimerDelayMs;

	DWORD dwHighBeaconId;
	DWORD dwLowBeaconId;
	DWORD	dwUseMDREnable;
	char    szMdrEnable_ParamUrl[1024];
	char    szChangeBeaconID_ParamUrl[1024];
}
QFR_DMI_PARAMS;



    
//
// Structure d'instance (contenu non exporté)
//
typedef struct _QFDMI_L2_INSTANCE QFDMI_L2_INSTANCE;




//
// Test de validité d'indexes
//
#define QFDMI_L7_IS_VALID(x)         ((x)<0x80000000)
#define QFDMI_L7_INVALID             0xFFFFFFFF


//
// Possible QFDMI_L7 commands
//
typedef enum
{
	QFDMI_INTF_REQ_ABORT,
	QFDMI_INTF_REQ_GETSTATUS,
	QFDMI_INTF_REQ_MODE_TRANSPARENT,
	QFDMI_SEND_L2_FRAME_BST,
	QFDMI_SEND_L2_FRAME,
	QFDMI_SEND_L2_FRAME_END,
	QFDMI_L2_TIMEOUT
} enuQFDMI_INTERNAL_MSGS_TYPE;



//
// Constantes diverses
//
#define QFDMI_INTERNAL_MSG_MAX_DATAGRAM        256





typedef struct
{
	BYTE                bMessageType;

	DWORD               dwL2BytesLen;

	BYTE                tbL2Bytes[QFDMI_INTERNAL_MSG_MAX_DATAGRAM - 2];
}
QFDMI_INTERNAL_MSG;


typedef struct
{
	BYTE        bError;
	BYTE        bMode;
	BYTE        bTransacting;
}
QFDMI_STATUS_RSP;


//-------------------------------------------
// QFDMI_tracing...
//-------------------------------------------
void WINAPI QFDMI_TraceComment(QFDMI_L2_INSTANCE * pInst, char * szStrToShow, ...);

#ifdef _WITH_TRACES
#define QFDMI_CALL_HOOK_COMMENT(x) QFDMI_TraceComment x;
#else
#define QFDMI_CALL_HOOK_COMMENT(x) ;
#endif



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFDMI_L2Open( 
 *                      OUT QFDMI_L2_INSTANCE  ** ppsInst,
 *                      IN  QFDMI_L2_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication QFDMI_L2
 *             (utilisé pour la communication HOTE<->Communicateur)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFDMI_L2Open( 
        OUT QFDMI_L2_INSTANCE  ** ppsInst,
		IN  QFR_DMI_PARAMS     * psParams);




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
        IN DWORD            dwTimeout );




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
        OPTIONAL OUT    DWORD         * pdwErr );




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
        IN QFDMI_L2_INSTANCE * psInst );




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
        IN QFDMI_L2_INSTANCE * psInst );
        
        


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
        IN      DWORD           dwTimeout );


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
        IN BYTE * pbMsg );



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
        IN void         * pvValue );




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
        IN void         * pvValue );



EXPORT BOOL WINAPI QFDMI_BuildInternalMessage(
	IN		QFDMI_L2_INSTANCE * psInst,
	IN OUT  BYTE              * pbBuf,
	IN OUT  DWORD             * pdwBufBytePos,
	IN OUT  DWORD             * pdwBufByteLen,
	IN      QFDMI_INTERNAL_MSG         * psDef);

EXPORT BOOL WINAPI QFDMI_DecodeInternalMessage(
	IN		QFDMI_L2_INSTANCE * psInst,
	IN OUT  BYTE              * pbBuf,
	IN OUT  DWORD             * pdwBufBytePos,
	IN OUT  DWORD             * pdwBufByteLen,
	OUT     QFDMI_INTERNAL_MSG         * psDef);

#pragma pack( pop, TAG_QFDMI_L2_H_PACK )

#endif
