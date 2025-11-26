/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFL2
 * FILE       : CSR_TAG_QFL2.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Layer 2  QFL2 - MD5826 
 * --------------------------------------------------------------------
 * DESCRIPTION: Layer 2 interface -	to be used to communicate with 
 *				QFree MD5826 beacon servis driver using named pipes
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_QFL2_H
#define CSR_TAG_QFL2_H


#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_QFL2_H_PACK, 1 )


//
// Constantes pour l'espionnage de la liaison
//
#define QFL2_HOOK_DIRECTION_FILTER    0x30000000
#define QFL2_HOOK_INPUT               0x10000000
#define QFL2_HOOK_OUTPUT              0x20000000
#define QFL2_HOOK_TYPE_FILTER         0xC0000000
#define QFL2_HOOK_PROTOCOL            0x40000000
#define QFL2_HOOK_DATA                0x80000000
#define QFL2_HOOK_FLUSH               0x00000001



//
// Options for this layer
//
#define QFL2_OPTION_DW_BEACONID         0x00000001
#define QFL2_OPTION_DW_KEEPALIVE        0x00000002
#define QFL2_OPTION_DW_N11              0x00000003
#define QFL2_OPTION_DW_RETRYINTOVERHEAD 0x00000004
#define QFL2_OPTION_F_DISABLEBEFOREBST  0x00000005
#define QFL2_OPTION_DW_INSERT_BEACONID  0x00000006



//
// Définition du callback pour l'espionnage
//
typedef void WINAPI QFL2_HOOK( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes );



//
// Paramétrage de l'instance
//
typedef struct _QFL2_PARAMS
{
    // Taille de la structure
    DWORD               dwStructBytes;

    // Pointeur sur la fonction callback d'espionnage
    QFL2_HOOK          * pfHook;

    // Contexte utilisateur d'espionnage
    void              * pvHookContext;

    // Maximum number of queued messages (in output and in output buffers)
    DWORD               dwMaxPendingMsg;

    // Serial port number
    DWORD               dwPort;

    // Serial port setting EXAMPLE "COM1 9600 n 2" to configure COM1 to be used at 9600bps, no parity and 2 stop bits
    char                szPortSettings[256];
	
    // Protocol thread priority
    DWORD               dwPriority;

    // Automatically clear serial buffer
    DWORD               dwAutoClearBuffers;

    // Communication pipes to be used 
    char                szL7DataPipeName[1024]; 
    char                szSecurityDataPipeName[1024];
}
    QFREE_PARAMS;



    
//
// Structure d'instance (contenu non exporté)
//
typedef struct _QFL2_INSTANCE QFL2_INSTANCE;






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFL2Open( 
 *                      OUT QFL2_INSTANCE  ** ppsInst,
 *                      IN  QFL2_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication QFL2
 *             (utilisé pour la communication HOTE<->Communicateur)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFL2Open( 
        OUT QFL2_INSTANCE  ** ppsInst,
        IN  QFREE_PARAMS     * psParams );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI QFL2Close( 
 *                      IN QFL2_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par QFL2Open
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             QFL2Open(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI QFL2Close( 
        IN QFL2_INSTANCE   * psInst,
        IN DWORD            dwTimeout );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFL2SendMessage( 
 *                      IN              QFL2_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par QFL2Open
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
EXPORT DWORD WINAPI QFL2SendMessage( 
        IN              QFL2_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI QFL2GetWaitableHandle(
 *                      IN QFL2_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par QFL2Open
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
EXPORT HANDLE WINAPI QFL2GetWaitableHandle(
        IN QFL2_INSTANCE * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI QFL2GetCommInstance(
 *                      IN QFL2_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par QFL2Open
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI QFL2GetCommInstance(
        IN QFL2_INSTANCE * psInst );
        
        


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFL2ReceiveMessage(
 *                      IN      QFL2_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par QFL2Open
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
 *             à QFL2FreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFL2ReceiveMessage(
        IN      QFL2_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI QFL2FreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par QFL2ReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à QFL2ReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI QFL2FreeMessage(
        IN BYTE * pbMsg );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI QFL2SetOption(
 *                      IN QFL2_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par QFL2Open
 *             dwOptionId  : Identifiant d'option (aucun dispo pour QFL2)
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
EXPORT DWORD WINAPI WINAPI QFL2SetOption(
        IN QFL2_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI QFL2GetOption(
 *                      IN QFL2_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par QFL2Open
 *             dwOptionId  : Identifiant d'option (aucun dispo pour QFL2)
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
EXPORT DWORD WINAPI WINAPI QFL2GetOption(
        IN QFL2_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
        IN void         * pvValue );


#pragma pack( pop, TAG_QFL2_H_PACK )

#endif
