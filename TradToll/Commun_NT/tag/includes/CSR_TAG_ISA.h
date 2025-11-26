/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : ISA
 * FILE       : ISA.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Couche 2 ISO (LLC) - Liaison filaire badge.
 * --------------------------------------------------------------------
 * DESCRIPTION: Couche protocolaire de niveau 2 ISO pour la liaison
 *              entre hôte et badge par liaison filaire en utilisant
 *              une liaison série en niveau 1 ISO.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_ISA_H
#define CSR_TAG_ISA_H


#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif



#pragma pack( push, TAG_ISA_PACK, 1 )


//
// Constantes pour l'espionnage de la liaison
//
#define ISA_HOOK_DIRECTION_FILTER    0x30000000
#define ISA_HOOK_INPUT               0x10000000
#define ISA_HOOK_OUTPUT              0x20000000
#define ISA_HOOK_TYPE_FILTER         0xC0000000
#define ISA_HOOK_PROTOCOL            0x40000000
#define ISA_HOOK_DATA                0x80000000
#define ISA_HOOK_FLUSH               0x00000001



//
// Flags à utiliser pour spécifier le type de timer à utiliser
//
#define ISA_USE_THREADED_TIMER       0x00000000
#define ISA_USE_WAITABLE_TIMER       0x80000000



//
// Définition du callback pour l'espionnage
//
typedef void WINAPI ISA_HOOK( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes );




//
// Paramétrage de l'instance
//
typedef struct _ISA_PARAMS
{
    // Taille de la structure
    DWORD               dwStructBytes;

    // Pointeur sur la fonction callback d'espionnage
    ISA_HOOK          * pfHook;

    // Contexte utilisateur d'espionnage
    void              * pvHookContext;

    // Timer intercaractères.
    DWORD               dwTimer;

    // Maximum number of queued messages (in output and in output buffers)
    DWORD               dwMaxPendingMsg;

    // Serial port number
    DWORD               dwPort;

    // Protocol thread priority
    DWORD               dwPriority;

    // Automatically clear serial buffer
    DWORD               dwAutoClearBuffers;

    // Serial port setting (MODE style text string)
    char                szPortSettings[256];
}
    ISA_PARAMS;


typedef struct _ISA_PARAMS_EX
{
    // Taille de la structure
    DWORD               dwStructBytes;

    // Pointeur sur la fonction callback d'espionnage
    ISA_HOOK          * pfHook;

    // Contexte utilisateur d'espionnage
    void              * pvHookContext;

    // Timer intercaractères.
    DWORD               dwTimer;

    // Maximum number of queued messages (in output and in output buffers)
    DWORD               dwMaxPendingMsg;

    // Serial port number
    DWORD               dwPort;

    // Protocol thread priority
    DWORD               dwPriority;

    // Automatically clear serial buffer
    DWORD               dwAutoClearBuffers;

    // Serial port setting (MODE style text string)
    char                szPortSettings[256];

    // Ensemble de flag de configuration :
    //   ISA_USE_THREADED_TIMER : Utilisation des timers en thread dédié
    //   ISA_USE_WAITABLE_TIMER : Utilisation des waitable timers
    DWORD               dwFlags;
}
    ISA_PARAMS_EX;



//
// Structure d'instance (contenu non exporté)
//
typedef struct _ISA_INSTANCE ISA_INSTANCE;






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISAOpen( 
 *                      OUT ISA_INSTANCE  ** ppsInst,
 *                      IN  ISA_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication ISA
 *             (utilisé pour la communication HOTE<->BADGE par liaison filaire)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI ISAOpen( 
        OUT ISA_INSTANCE  ** ppsInst,
        IN  ISA_PARAMS     * psParams );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI ISAClose( 
 *                      IN ISA_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par ISAOpen
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             ISAOpen(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI ISAClose( 
        IN ISA_INSTANCE   * psInst,
        IN DWORD            dwTimeout );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISASendMessage( 
 *                      IN              ISA_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par ISAOpen
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
EXPORT DWORD WINAPI ISASendMessage( 
        IN              ISA_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr );



        
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI ISAGetWaitableHandle(
 *                      IN ISA_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par ISAOpen
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
EXPORT HANDLE WINAPI ISAGetWaitableHandle(
        IN ISA_INSTANCE * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI ISAGetCommInstance(
 *                      IN ISA_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par ISAOpen
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI ISAGetCommInstance(
        IN ISA_INSTANCE * psInst );






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISAReceiveMessage(
 *                      IN      ISA_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par ISAOpen
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
 *             à ISAFreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI ISAReceiveMessage(
        IN      ISA_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI ISAFreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par ISAReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à ISAReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI ISAFreeMessage(
        IN BYTE * pbMsg );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI ISASetOption(
 *                      IN ISA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par ISAOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour ISA)
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
EXPORT DWORD WINAPI WINAPI ISASetOption(
        IN ISA_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI ISAGetOption(
 *                      IN ISA_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par ISAOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour ISA)
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
EXPORT DWORD WINAPI WINAPI ISAGetOption(
        IN ISA_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
        IN void         * pvValue );

#pragma pack( pop, TAG_ISA_PACK )


#endif
