/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : IPGEA
 * FILE       : CSR_TAG_IPGEA.H
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

#ifndef CSR_TAG_IPGEA_H
#define CSR_TAG_IPGEA_H


#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_IPGEA_H_PACK, 1 )


//
// Constantes pour l'espionnage de la liaison
//
#define IPGEA_HOOK_DIRECTION_FILTER    0x30000000
#define IPGEA_HOOK_INPUT               0x10000000
#define IPGEA_HOOK_OUTPUT              0x20000000
#define IPGEA_HOOK_TYPE_FILTER         0xC0000000
#define IPGEA_HOOK_PROTOCOL            0x40000000
#define IPGEA_HOOK_DATA                0x80000000
#define IPGEA_HOOK_COMMENT             0x01000000
#define IPGEA_HOOK_FLUSH               0x00000001




//
// Définition du callback pour l'espionnage
//
typedef void WINAPI IPGEA_HOOK( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes );


//
// Options de l'instance
//

// R/W : Flag indiquant que le prochain message est le dernier.
//       Cette option est remise à zéro à chaque émission de message.
#define IPGEA_OPTION_F_LAST_MESSAGE      1

// R/O : Construit une chaine retournant les numéros de version
//       de tous les niveaux (L7,L2,equipement)
#define IPGEA_OPTION_SZ_VERSION          2

// R/O : Last used beacon id
#define IPGEA_OPTION_DW_BEACON_ID        3

#define IPGEA_OPTION_DW_FREQUENCY    4


typedef enum 
{
	GEAIP_INTF_REQ_ABORT,
	GEAIP_INTF_REQ_GETSTATUS,
	GEAIP_INTF_REQ_MODE_TRANSPARENT,
	GEAIP_SEND_L2_FRAME_BST,
	GEAIP_SEND_L2_FRAME,
	GEAIP_SEND_L2_FRAME_END,
	GEAIP_L2_TIMEOUT


} enuGEAIP_INTERNAL_MSGS_TYPE;


typedef enum
{
    GEAIP_ERROR_SUCCESS       = 0x00,
    GEAIP_ERROR_REFUSED       = 0x01,
    GEAIP_ERROR_TRANSACT      = 0x02,
    GEAIP_ERROR_BEACON        = 0x03,
    GEAIP_ERROR_TIMEOUT       = 0x09,
    GEAIP_ERROR_RESET         = 0x0A,
    GEAIP_ERROR_INVALID_PARAM = 0x0B,
    GEAIP_ERROR_CONFIG_FILE   = 0x0C,

}GEAIP_INTERNAL_MSGS_ERROR_CODES;


//
// Constantes diverses
//
#define IPGEA_MAX_DATAGRAM        256

typedef struct 
{
    BYTE				bMessageType;
    
    DWORD               dwNumL2Bytes;

    BYTE                tbL2Bytes[IPGEA_MAX_DATAGRAM-2];
}
    IPGEAL7_API;




//
// Paramétrage de l'instance
//
typedef struct _IPGEA_PARAMS
{
    // Taille de la structure
    DWORD               dwStructBytes;

    // Pointeur sur la fonction callback d'espionnage
    IPGEA_HOOK          * pfHook;

    // Contexte utilisateur d'espionnage
    void              * pvHookContext;

    // Maximum number of queued messages (in output and in output buffers)
    DWORD               dwMaxPendingMsg;

    // Port debit
    DWORD               dwDebit;

    // Protocol thread priority
    DWORD               dwPriority;

    // Automatically clear serial buffer
    DWORD               dwAutoClearBuffers;

	//Ip address and port to connect to
    char                szIpAdress[1024];
	DWORD				dwPort;

	//beacon pooling period
	DWORD				dwCheckPoolingPeriod;
	DWORD				dwCheckPoolingPeriodTimeout; // = 2* dwCheckPoolingPeriod
	DWORD				dwResetAfterTimeouts;

	DWORD				dwConnectionTimeout;
	
	//change beacon id
	BOOL bChangeBeaconID;
	
}
    IPGEA_PARAMS;



    
//
// Structure d'instance (contenu non exporté)
//
typedef struct _IPGEA_INSTANCE IPGEA_INSTANCE;


//-----------------------------------------------------------

EXPORT void WINAPI IPGEAInitGlobalInstanceHandling();
EXPORT void WINAPI IPGEADeinitGlobalInstanceHandling();


EXPORT BOOL WINAPI IPGEAL7_EncodeMessage( 
        IN      IPGEA_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      IPGEAL7_API       *  psDef );


EXPORT BOOL WINAPI IPGEAL7_DecodeMessage( 
        IN      IPGEA_INSTANCE   * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     IPGEAL7_API       * psDef );

//-----------------------------------------------------------

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
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI IPGEAOpen( 
        OUT IPGEA_INSTANCE  ** ppsInst,
        IN  IPGEA_PARAMS     * psParams );




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
        IN DWORD            dwTimeout );




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
        OPTIONAL OUT    DWORD         * pdwErr );




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
        IN IPGEA_INSTANCE * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI IPGEAGetCommInstance(
 *                      IN IPGEA_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par IPGEAOpen
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Always retuns NULL. No need to 
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI IPGEAGetCommInstance(
        IN IPGEA_INSTANCE * psInst );
        
        


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
        IN      DWORD           dwTimeout );


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
        IN BYTE * pbMsg );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI IPGEASetOption(
 *                      IN IPGEA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             dwOptionId  : Identifiant d'option 
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
        IN void         * pvValue );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI IPGEAGetOption(
 *                      IN IPGEA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par IPGEAOpen
 *             dwOptionId  : Identifiant d'option 
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
		IN size_t		  pvValueSize);


#pragma pack( pop, TAG_IPGEA_H_PACK )

#endif
