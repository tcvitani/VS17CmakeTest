/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : ISA
 * FILE       : ISA.C
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

#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>
#include <csr_queue.h>
#include <csr_timer.h>

#include "csr_tag_ser.h"
#include "auto.h"
#include "csr_tag_isa.h"

#include <memclass.h>



//
// Caracteres de controle du protocole
//
#define ISA_CHAR_STX        '\x02'
#define ISA_CHAR_ETX        '\x03'

//
// Tailles de blocs
//

// Taille maximum des données utiles d'un message
#define ISA_MAX_MSG_BYTES   123

// Taille de l'entête protocolaire (STX+TAILLE)
#define ISA_HEADER_BYTES    2

// Taille de l'entête sans la taille (STX)
#define ISA_SHORT_HEADER_BYTES    1

// Taille du CRC16
#define ISA_CRC_BYTES       2

// Taille de la fin de message protocolaire (ETX sans CRC)
#define ISA_FOOTER_BYTES    1

// Taille des données protocolaire dans un message
#define ISA_PROTOCOL_BYTES  5


//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects et qui
// serviront à provoquer des événements sur l'automate protocolaire
//
#define ISA_END_INDEX            0
#define ISA_COM_INDEX            1
#define ISA_TMR_INDEX            2
#define ISA_IRP_INDEX            3
#define ISA_EVT_COUNT            4


//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et CRC).
//
#define ISA_IN_BUFFER_BYTES      256
#define ISA_OUT_BUFFER_BYTES     256
#define ISA_HOOK_BUFFER_BYTES    256


//
// Hook utilisé pour l'espionnage de la liaison
//
#define ISA_CALL_HOOK(ctx,size,ptr)  if ( psInst->sParams.pfHook != NULL ) ISATrack( psInst, ctx, size, ptr ); else



//
// Etats d'analyse lors de la réception des caractères d'un message
//
typedef enum
{
    // Etat initial : le prochain caractère attendu est un STX
    ISA_MSG_WAIT_STX = 0,

    // Le prochain caractère attendu est la taille des données utiles
    ISA_MSG_WAIT_SIZE,

    // Le prochain caractère attendu fait partie des données utiles
    ISA_MSG_WAIT_DATA,

    // Le prochain caractère attendu est le premier octet du CRC16
    ISA_MSG_WAIT_CRC1,

    // Le prochain caractère attendu est le second octet du CRC16
    ISA_MSG_WAIT_CRC2,

    // Etat final : le prochain caractère attendu est un ETX
    ISA_MSG_WAIT_ETX,
}
    ISA_MSG_WAIT_STATES;


//
// Définition d'un packet pour la file des messages en sortie
// et en entrée
//
typedef struct _ISA_PACKET
{
    void              * pvContext;
    DWORD             * pdwErrCode;
    DWORD               dwBufferBytes;
    BYTE                tbBuffer[];
}
    ISA_PACKET;

    
//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _ISA_INSTANCE
{
    // Paramètres de l'instance
    ISA_PARAMS_EX       sParams;

    // Flag indiquant le type de timer utilisé
    BOOL                dwTimerFlags;

    // Handle du thread de gestion du protocole
    HANDLE              hThread;

    // Objet "port" pour la gestion du port comm
    SER_INSTANCE      * psPort;

    // Objet "timer" pour la gestion des délais dans le protocole
    TIMER_INSTANCE    * psTimer;

    // Objet "automate" pour la gestion de l'automate
    H_AUT               hAut;

    // File d'attente pour les messages sortant
    QUEUE_INSTANCE    * psWriteQueue;

    // File d'attente pour les messages entrant
    QUEUE_INSTANCE    * psReadQueue;

    // Evénement signalé si la file psReadQueue a au moins un élément
    // et non signalé si la psReadQueue est vide
    HANDLE              hEndEvent;

    // Tableau des handles signalables servant à animer l'automate
    HANDLE              thEvents[ISA_EVT_COUNT];

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // Comptes de tentatives de réémission
    DWORD               dwEnqRetry;
    DWORD               dwMsgRetry;

    // En phase d'émission, packet courant à émettre
    ISA_PACKET        * psCurrentPacket;

    // Analyse structurelle du message (définition du prochain caractère
    // attendu dans le message).
    ISA_MSG_WAIT_STATES eCurrentMsgState;

    // Dernier octet lu
    BYTE                bRead;

    // Taille de message attendue
    DWORD               dwExpectedSize;

    // Taille occupée dans le buffer de réception
    DWORD               dwInBytes;

    // Buffer de réception
    BYTE                tbIn[ ISA_IN_BUFFER_BYTES ];

    // CRC courant calculé au fur et à mesure de la réception des octets
    BYTE                tbCrcComp[2];

    // CRC transmis avec le message recu
    BYTE                tbCrcRecv[2];

    // Taille occupée dans le buffer d'émission
    DWORD               dwOutBytes;

    // Buffer d'émission
    BYTE                tbOut[ ISA_OUT_BUFFER_BYTES ];

    // Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ ISA_HOOK_BUFFER_BYTES ];
}
    ISA_INSTANCE;


//
// Fonctions de transition de l'automate protocolaire
//
PRIVATE void WINAPI ISA_ACT_SEND_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_CHAR_SLEEPING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_GO_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_CHAR_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_MSG_RECEIVED( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_ERROR_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI ISA_ACT_ERROR( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );


//
// Autres fonctions privées
//
PRIVATE void WINAPI ISAMakeAutError( ISA_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE DWORD WINAPI ISARunThread( IN ISA_INSTANCE * psInst );
PRIVATE void WINAPI ISATerminateCurrentPacket( ISA_INSTANCE * psInst, DWORD dwErr );
PRIVATE DWORD WINAPI ISABuildMessage( ISA_INSTANCE * psInst );
PRIVATE void WINAPI ISACrcAddByte( BYTE bByte, BYTE * pbCrc );
PRIVATE void WINAPI ISACrcCompute( BYTE * pbBuffer, DWORD dwBufferBytes, BYTE * pbCrc );
PRIVATE void WINAPI ISATrack( ISA_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );



//
// Définitions des événements de l'automate protocolaire
//
typedef enum
{
    ISA_EVT_QUEUED_MSG = 0,
    ISA_EVT_RECV_CHAR     ,
    ISA_EVT_CTRL_STX      ,
    ISA_EVT_TIMER         ,
    ISA_EVT_MSG_BAD       ,
    ISA_EVT_MSG_OK        ,
    ISA_EVT_ERROR         ,
    ISA_MAX_AUT_EVENTS
}
    ISA_AUT_EVENTS;


//
// Définitions des états de l'automate protocolaire
//
typedef enum
{
    ISA_ST_SLEEPING,
    ISA_ST_RECEIVING,

    ISA_MAX_AUT_STATES
}
    ISA_AUT_STATES;

    





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
        IN  ISA_PARAMS     * psParams )
{
    DWORD               dwErr       = ERROR_NOT_ENOUGH_MEMORY;
    ISA_INSTANCE      * psInst      = NULL;
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
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ISA_INSTANCE) );
        if ( psInst == NULL ) 
            __leave;

        //
        // Reporter les valeurs des paramètres une fois qu'on s'est assuré que la
        // taille de la structure est compatible.
        //
        if ( ( psParams->dwStructBytes != sizeof(ISA_PARAMS   ) ) &&
             ( psParams->dwStructBytes != sizeof(ISA_PARAMS_EX) ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        memcpy( (void*)&psInst->sParams, (void*)psParams, psParams->dwStructBytes );

        //
        // Déterminer le type de timer à utiliser
        //
        if ( ( psInst->sParams.dwFlags & ISA_USE_WAITABLE_TIMER ) != 0 )
            psInst->dwTimerFlags = TIMER_TYPE_WAITABLE | TIMER_MAKE_RESOLUTION(4);
        else
            psInst->dwTimerFlags = TIMER_TYPE_SPECIFIC_THREAD | TIMER_MAKE_RESOLUTION(4);

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
        dwErr = TimerOpenEx( psInst->dwTimerFlags, &psInst->psTimer );
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Initialisation de la liaison série
        //
        dwErr = SEROpen( &psInst->psPort, psInst->sParams.dwPort, psInst->sParams.szPortSettings );
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        //
        psInst->thEvents[ISA_END_INDEX] = psInst->hEndEvent;
        psInst->thEvents[ISA_COM_INDEX] = SERGetWaitableHandle( psInst->psPort );
        psInst->thEvents[ISA_IRP_INDEX] = QueueGetWaitableHandle( psInst->psWriteQueue );
        psInst->thEvents[ISA_TMR_INDEX] = TimerGetWaitableHandle( psInst->psTimer );

        //
        // Création de l'automate protocolaire
        //
        psInst->hAut = AutOpen( ISA_MAX_AUT_STATES, psInst );
        if ( psInst->hAut == NULL )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Définition des états de l'automate protocolaire
        //
        if ( ( ! AutAddState( psInst->hAut, ISA_ST_SLEEPING  ) ) ||
             ( ! AutAddState( psInst->hAut, ISA_ST_RECEIVING ) ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Définition des transitions de l'automate protocolaire
        //
        if ( ( ! AutAddTransition( psInst->hAut, ISA_ST_SLEEPING , ISA_EVT_QUEUED_MSG, ISA_ST_SLEEPING , ISA_ACT_SEND_MSG       ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_SLEEPING , ISA_EVT_RECV_CHAR , ISA_ST_SLEEPING , ISA_ACT_CHAR_SLEEPING  ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_SLEEPING , ISA_EVT_CTRL_STX  , ISA_ST_RECEIVING, ISA_ACT_GO_RECEIVING   ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_SLEEPING , ISA_EVT_ERROR     , ISA_ST_SLEEPING , ISA_ACT_ERROR          ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_SLEEPING , AUT_EVENT_ANY     , ISA_ST_SLEEPING , ISA_ACT_ERROR          ) ) ||

             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, ISA_EVT_RECV_CHAR , ISA_ST_RECEIVING, ISA_ACT_CHAR_RECEIVING ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, ISA_EVT_MSG_OK    , ISA_ST_SLEEPING , ISA_ACT_MSG_RECEIVED   ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, ISA_EVT_TIMER     , ISA_ST_SLEEPING , ISA_ACT_ERROR_RECEIVING) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, ISA_EVT_MSG_BAD   , ISA_ST_SLEEPING , ISA_ACT_ERROR_RECEIVING) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, ISA_EVT_ERROR     , ISA_ST_SLEEPING , ISA_ACT_ERROR          ) ) ||
             ( ! AutAddTransition( psInst->hAut, ISA_ST_RECEIVING, AUT_EVENT_ANY     , ISA_ST_SLEEPING , ISA_ACT_ERROR          ) ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Placer l'automate dans son état initial
        //
        if ( ! AutSetCurrentState( psInst->hAut, ISA_ST_SLEEPING ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ISARunThread, (void*)psInst, 0, &dwId, "TAG_ISA_PROTOCOL" );
        psInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ISARunThread, (void*)psInst, 0, &dwId );
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
            ISAClose( psInst, 0 );

        
    }

	return dwErr;
}




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
        IN DWORD            dwTimeout )
{
    if ( psInst != NULL )
    {
        if ( psInst->hThread != NULL )
        {
            timeEndPeriod( 1 );
            SetEvent( psInst->hEndEvent );
            WaitForSingleObject( psInst->hThread, dwTimeout );
            TerminateThread( psInst->hThread, NO_ERROR );
            CloseHandle( psInst->hThread );
        }
        if ( psInst->hAut != NULL )
            AutClose( psInst->hAut );
        if ( psInst->psReadQueue != NULL )
            QueueClose( psInst->psReadQueue );
        if ( psInst->psWriteQueue != NULL )
            QueueClose( psInst->psWriteQueue );
        if ( psInst->psTimer != NULL )
            TimerClose( psInst->psTimer );
        if ( psInst->psPort != NULL )
            SERClose( psInst->psPort );
        if ( psInst->hEndEvent != NULL )
            CloseHandle( psInst->hEndEvent );

        HeapFree( GetProcessHeap(), 0, psInst );
    }
}




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
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    ISA_PACKET    * psPacket = NULL;

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
        IN ISA_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




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
        IN ISA_INSTANCE * psInst )
{
    return psInst->psPort;
}




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
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    ISA_PACKET    * psPacket;

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
    }

    return dwErr;
}




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
        IN BYTE * pbMsg )
{
    ISA_PACKET * psPacket = NULL;

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
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



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
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



PRIVATE void WINAPI ISA_ACT_SEND_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    ISA_INSTANCE * psInst = AutGetContext( hAut );

    //
    // Annuler tout timer en cours
    //
    TimerCancel( psInst->psTimer );

    if ( psInst->psCurrentPacket == NULL )
    {
        //
        // Si le message n'a pas encore été construit, on l'extrait de la file d'attente
        //
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
        if ( dwErr != NO_ERROR )
        {
            ISAMakeAutError( psInst, dwErr, "ISA_ACT_SEND_MSG : QueueReadItem" );
            return;
        }

        //
        // Construire le message
        //
        dwErr = ISABuildMessage( psInst );
        if ( dwErr != NO_ERROR )
        {
            ISAMakeAutError( psInst, dwErr, "ISA_ACT_SEND_MSG : ISABuildMessage" );
            return;
        }
    }

    //
    // Envoyer le message sur le port série
    //
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, psInst->tbOut, psInst->dwOutBytes );
    if ( dwErr == NO_ERROR )
    {
        ISA_CALL_HOOK( ISA_HOOK_OUTPUT | ISA_HOOK_PROTOCOL | ISA_HOOK_FLUSH,
            ISA_HEADER_BYTES, 
            psInst->tbOut );

        ISA_CALL_HOOK( ISA_HOOK_OUTPUT | ISA_HOOK_DATA | ISA_HOOK_FLUSH,
            psInst->dwOutBytes - ISA_PROTOCOL_BYTES, 
            psInst->tbOut + ISA_HEADER_BYTES );

        ISA_CALL_HOOK( ISA_HOOK_OUTPUT | ISA_HOOK_PROTOCOL | ISA_HOOK_FLUSH,
            ISA_CRC_BYTES + ISA_FOOTER_BYTES, 
            psInst->tbOut + psInst->dwOutBytes - ISA_CRC_BYTES - ISA_FOOTER_BYTES );

        ISATerminateCurrentPacket( psInst, NO_ERROR );
    }
    else
    {
        //
        // L'envoi a échoué, on provoque une erreur, on annule l'IRP en cours
        //
        ISAMakeAutError( psInst, dwErr, "ISA_ACT_SEND_MSG : SERWrite" );
        return;
    }
}





PRIVATE void WINAPI ISA_ACT_CHAR_SLEEPING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    ISA_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;
    
    TimerCancel( psInst->psTimer );

    //
    // Lire le caractère suivant
    //
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( dwErr != NO_ERROR )
    {
        //
        // Erreur de lecture, on provoque une erreur
        //
        ISAMakeAutError( psInst, dwErr, "ISA_ACT_CHAR_SLEEPING : SERRead" );
        return;
    }

    if ( dwCount == 0 )
    {
        //
        // Ne pas provoquer d'erreur si buffer reception comm vide
        //
        return;
    }

    //
    // Si le caractère n'est pas STX, il est ignoré.
    //
    if ( psInst->bRead == ISA_CHAR_STX )
    {
        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL, 1, &psInst->bRead );

        //
        // Passer en phase de réception.
        //
        AutSendEvent( psInst->hAut, ISA_EVT_CTRL_STX, NULL );
    }
}




PRIVATE void WINAPI ISA_ACT_GO_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    ISA_INSTANCE * psInst = AutGetContext( hAut );

    //
    // Armer le timer inter-caractère
    //
    dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer );
    if ( dwErr != NO_ERROR )
    {
        ISAMakeAutError( psInst, dwErr, "ISA_ACT_GO_RECEIVING : TimerActivate" );
        return;
    }

    //
    // Initialiser la réception
    //
    psInst->dwInBytes = 0;
    psInst->eCurrentMsgState = ISA_MSG_WAIT_SIZE;
    psInst->dwExpectedSize = 0;
    psInst->tbCrcComp[0] = 0xFF;
    psInst->tbCrcComp[1] = 0xFF;
}


PRIVATE void WINAPI ISA_ACT_CHAR_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    ISA_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;
    
    TimerCancel( psInst->psTimer );

    //
    // Lire le caractère suivant
    //
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( dwErr != NO_ERROR )
    {
        //
        // Erreur de lecture, on provoque une erreur
        //
        ISAMakeAutError( psInst, dwErr, "ISA_ACT_CHAR_RECEIVING : SERRead" );
        return;
    }

    //
    // Réarmer le timer
    //
    dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer );
    if ( dwErr != NO_ERROR )
    {
        ISAMakeAutError( psInst, dwErr, "ISA_ACT_CHAR_RECEIVING : TimerActivate" );
        return;
    }

    //
    // Ne pas provoquer d'erreur si buffer reception comm vide
    //
    if ( dwCount != 1 )
        return;

    //
    // Interpréter le caractère en fonction de celui attendu
    //

    if ( psInst->eCurrentMsgState == ISA_MSG_WAIT_SIZE )
    {
        //
        // ON ATTENT LA TAILLE DU MESSAGE
        //

        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL | ISA_HOOK_FLUSH, 1, &psInst->bRead );

        psInst->dwExpectedSize = (DWORD)psInst->bRead;
        if ( ( psInst->dwExpectedSize < 1 ) || ( psInst->dwExpectedSize > ISA_MAX_MSG_BYTES ) )
        {
            //
            // La taille n'est pas valide, le message est refusé
            //
            AutSendEvent( psInst->hAut, ISA_EVT_MSG_BAD, NULL );
        }
        else
        {
            //
            // La taille fait partie du calcul de CRC
            //
            ISACrcAddByte( psInst->bRead, psInst->tbCrcComp );

            psInst->eCurrentMsgState = ISA_MSG_WAIT_DATA;
        }
    }

    else if ( psInst->eCurrentMsgState == ISA_MSG_WAIT_DATA )
    {
        //
        // ON ATTEND LES DONNEES
        //

        //
        // Si aucune donnée n'est attendu (cas anormal)
        //
        if ( psInst->dwExpectedSize < 1 )
        {
            ISAMakeAutError( psInst, ERROR_INVALID_DATA, "ISA_ACT_CHAR_RECEIVING : Internal error" );
            return;
        }

        //
        // Ajouter l'octet recu
        //
        psInst->tbIn[psInst->dwInBytes] = psInst->bRead;
        psInst->dwInBytes ++;
        ISACrcAddByte( psInst->bRead, psInst->tbCrcComp );

        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_DATA, 1, &psInst->bRead );

        psInst->dwExpectedSize --;

        if ( psInst->dwExpectedSize == 0 )
        {
            //
            // On fini le calcul du CRC en en prenant son inverse
            //
            psInst->tbCrcComp[0] = ~psInst->tbCrcComp[0];
            psInst->tbCrcComp[1] = ~psInst->tbCrcComp[1];

            //
            // Lorsqu'on arrive à la fin des données, on attend un CRC
            //
            psInst->eCurrentMsgState = ISA_MSG_WAIT_CRC1;
        }
    }

    else if ( psInst->eCurrentMsgState == ISA_MSG_WAIT_CRC1 )
    {
        //
        // ON ATTEND LE PREMIER OCTET DU CRC
        //

        psInst->tbCrcRecv[0] = psInst->bRead;
        psInst->eCurrentMsgState = ISA_MSG_WAIT_CRC2;

        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL, 1, &psInst->bRead );
    }

    else if ( psInst->eCurrentMsgState == ISA_MSG_WAIT_CRC2 )
    {
        //
        // ON ATTENT LE SECOND OCTET DU CRC
        //

        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL, 1, &psInst->bRead );
        psInst->tbCrcRecv[1] = psInst->bRead;

        if ( ( psInst->tbCrcRecv[0] == psInst->tbCrcComp[0] ) && ( psInst->tbCrcRecv[1] == psInst->tbCrcComp[1] ) )
        {
            psInst->eCurrentMsgState = ISA_MSG_WAIT_ETX;
        }
        else
            AutSendEvent( psInst->hAut, ISA_EVT_MSG_BAD, NULL );
    }

    else if ( psInst->eCurrentMsgState == ISA_MSG_WAIT_ETX )
    {
        //
        // ON ATTENT LE ETX
        //

        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL | ISA_HOOK_FLUSH, 1, &psInst->bRead );
        if ( psInst->bRead == ISA_CHAR_ETX )
            AutSendEvent( psInst->hAut, ISA_EVT_MSG_OK, NULL );
        else
            AutSendEvent( psInst->hAut, ISA_EVT_MSG_BAD, NULL );
    }

    else
    {
        ISA_CALL_HOOK( ISA_HOOK_INPUT | ISA_HOOK_PROTOCOL | ISA_HOOK_FLUSH, 1, &psInst->bRead );

        ISAMakeAutError( psInst, ERROR_INVALID_DATA, "ISA_ACT_CHAR_RECEIVING : Internal error" );
        return;
    }
}




PRIVATE void WINAPI ISA_ACT_MSG_RECEIVED( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr = NO_ERROR;
    ISA_INSTANCE * psInst = AutGetContext( hAut );
    ISA_PACKET * psPacket;

    //
    // Annuler tout timer en cours
    //
    TimerCancel( psInst->psTimer );

    //
    // Préparer le packet de lecture
    //
    psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + psInst->dwInBytes );
    if ( psPacket != NULL )
    {
        //
        // Mettre à zéro l'entête du packet
        //
        ZeroMemory( psPacket, sizeof( *psPacket ) );

        psPacket->dwBufferBytes = psInst->dwInBytes;
        CopyMemory( psPacket->tbBuffer, psInst->tbIn, psPacket->dwBufferBytes );

        //
        // Repartir à zéro avec le buffer et l'état de reception du message
        //
        psInst->dwInBytes = 0;
        psInst->eCurrentMsgState = ISA_MSG_WAIT_STX;
        psInst->tbCrcComp[0] = 0xFF;
        psInst->tbCrcComp[1] = 0xFF;

        //
        // Ajouter l'item dans la file d'attente
        //
        dwErr = QueueWriteItem( psInst->psReadQueue, psPacket, INFINITE );
        if ( dwErr != NO_ERROR )
        {
            //
            // Echec dans la file d'attente : On libère le buffer alloué
            // La reception a échouée, on provoque une erreur
            //
            HeapFree( GetProcessHeap(), 0, psPacket );
            ISAMakeAutError( psInst, dwErr, "ISA_ACT_MSG_RECEIVED : QueueWriteItem" );
            return;
        }
    }
}


PRIVATE void WINAPI ISA_ACT_ERROR_RECEIVING( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    ISA_INSTANCE * psInst = AutGetContext( hAut );

    //
    // Annuler tout timer en cours
    //
    TimerCancel( psInst->psTimer );

    //
    // Repartir à zéro avec le buffer et l'état de reception du message
    //
    psInst->dwInBytes = 0;
    psInst->eCurrentMsgState = ISA_MSG_WAIT_STX;
    psInst->tbCrcComp[0] = 0xFF;
    psInst->tbCrcComp[1] = 0xFF;
}





PRIVATE void WINAPI ISA_ACT_ERROR( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    ISA_INSTANCE * psInst = AutGetContext( hAut );

    //
    // Annuler tout timer en cours
    //
    TimerCancel( psInst->psTimer );

    //
    // Reseter les compteurs
    //
    psInst->dwOutBytes = 0;
    psInst->dwInBytes = 0;
    psInst->dwOutBytes = 0;
    psInst->eCurrentMsgState = ISA_MSG_WAIT_STX;
    psInst->tbCrcComp[0] = 0xFF;
    psInst->tbCrcComp[1] = 0xFF;
}




PRIVATE void WINAPI ISAMakeAutError( ISA_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    ISATerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
    strcpy( psInst->szLastError, pcText );
    AutSendEvent( psInst->hAut, ISA_EVT_ERROR, NULL );
}




PRIVATE DWORD WINAPI ISARunThread( 
        IN ISA_INSTANCE * psInst )
{
    DWORD       dwErr;
    DWORD       dwRes;
    DWORD       dwIndex;
    DWORD       dwCount;

    SetThreadPriorityBoost( GetCurrentThread(), TRUE );
    SetThreadPriority( GetCurrentThread(), psInst->sParams.dwPriority );

    //
    // Animer l'automate
    //
    while ( TRUE )
    {
        //
        // Lorsque l'état courant est au repos, on peut traiter les message à émettre.
        // Si on n'est pas au repos, c'est qu'on est en train d'effectuer un traitement
        // ne permettant pas de commencer l'émission d'un nouveau message. On ignore
        // donc temporairement les événements liés à la file d'attent d'émission.
        //
        if ( AutGetCurrentState( psInst->hAut ) == ISA_ST_SLEEPING )
        {
            //
            // Prendre compte les messages à émettre
            //
            dwCount = ISA_EVT_COUNT; 
        }
        else
        {
            //
            // Ne pas prendre en compte les messages à émettre
            //
            dwCount = ISA_EVT_COUNT - 1; 
        }

        dwRes =  WaitForMultipleObjects( dwCount, psInst->thEvents, FALSE, INFINITE );
        dwIndex = dwRes - WAIT_OBJECT_0;

        if ( dwIndex == ISA_END_INDEX )
        {
            //
            // Signal de fin reçu, on sort de la boucle
            // 
			// Signal is signaled from ISAClose function and 
			// it exits the L2 layer thread loop
            break;
        }

        else if ( dwIndex == ISA_TMR_INDEX )
        {
            //
            // Si événement du à timer
			//
            // Timer event is trigered by the windows system 
			// if set to be triggered using CSR_TIMER.h function
			// named TimerActivate
            TimerAcknowledge( psInst->psTimer ); //Acknoledge the timer so the event will not be detected as signaled again
            
			AutSendEvent( psInst->hAut, ISA_EVT_TIMER, NULL ); //process the event
        }

        else if ( dwIndex == ISA_COM_INDEX )
        {
            //
            // Si événement du à comm entrante
            //
			// A char receive detected on port - process this event
			// (Depends on what is sent and what is expected 
			// - the authomate solves this type of problems)
            AutSendEvent( psInst->hAut, ISA_EVT_RECV_CHAR, NULL );
        }

        else if ( dwIndex == ISA_IRP_INDEX )
        {
            //
            // Si événement du à la présence d'un request packet en émission
            //
			// Request package from the parent protcol detected in the 
			// WriteQueue - The message was sent from parent protocol 
			// using  ISASendMessage.  
            AutSendEvent( psInst->hAut, ISA_EVT_QUEUED_MSG, NULL );
        }

        else
        {
            //
            // Ignorer
            //
        }
    }

    //
    // Avant la fin du thread, vider la file d'attente en emission
    //
	// After the tread loop is finished do the cleanup
	// Empty the write and read queue package one at a time
	// - Signalise the associated events for the outgoing packages and deallocate 
	//		its memory
	// - Dealocate memory for incomming packages

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
        ISATerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

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




PRIVATE void WINAPI ISATerminateCurrentPacket( ISA_INSTANCE * psInst, DWORD dwErr )
{
    //
    // Si un packet est en cours d'émission
    //
    if ( psInst->psCurrentPacket != NULL )
    {
        //
        // Signaler le code de résultat associé
        //
        if ( psInst->psCurrentPacket->pdwErrCode != NULL )
            (*(psInst->psCurrentPacket->pdwErrCode)) = dwErr;

        //
        // Signaler l'événement associé
        //
        if ( psInst->psCurrentPacket->pvContext != NULL )
            SetEvent( psInst->psCurrentPacket->pvContext );

        //
        // Libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentPacket );

        //
        // Plus de packet en cours
        //
        psInst->psCurrentPacket = NULL;
    }
}


PRIVATE DWORD WINAPI ISABuildMessage( ISA_INSTANCE * psInst )
{
    //
    // Position courant dans le buffer, au début, 0
    //
    psInst->dwOutBytes = 0;

    //
    // Si aucun message à envoyer, ce n'est pas normal
    //
    if ( psInst->psCurrentPacket == NULL )
        return ERROR_INVALID_DATA;

    if ( psInst->psCurrentPacket->dwBufferBytes > ISA_MAX_MSG_BYTES )
        return ERROR_BUFFER_OVERFLOW;

    if ( ( psInst->psCurrentPacket->dwBufferBytes + ISA_PROTOCOL_BYTES ) > ISA_OUT_BUFFER_BYTES )
        return ERROR_BUFFER_OVERFLOW;

    //
    // Placer l'entête du message
    //
    psInst->tbOut[0] = ISA_CHAR_STX; //first byte is STX 
    psInst->dwOutBytes ++;
    psInst->tbOut[psInst->dwOutBytes] = (BYTE)psInst->psCurrentPacket->dwBufferBytes;
    psInst->dwOutBytes ++;

    //
    // Placer ensuite les données du message
    //
    CopyMemory( 
        &psInst->tbOut[psInst->dwOutBytes], 
        psInst->psCurrentPacket->tbBuffer,
        psInst->psCurrentPacket->dwBufferBytes );
    psInst->dwOutBytes += psInst->psCurrentPacket->dwBufferBytes;

    //
    // Placer le CRC
    //
    ISACrcCompute( 
        &psInst->tbOut[ISA_SHORT_HEADER_BYTES], 
        psInst->dwOutBytes - ISA_SHORT_HEADER_BYTES, 
        &psInst->tbOut[psInst->dwOutBytes] );
    psInst->dwOutBytes += ISA_CRC_BYTES;

    //
    // Placer la fin du message
    //
    psInst->tbOut[psInst->dwOutBytes] = ISA_CHAR_ETX;
    psInst->dwOutBytes ++;

    return NO_ERROR;
}




PRIVATE void WINAPI ISACrcAddByte( BYTE bByte, BYTE * pbCrc )
{
	DWORD dwCrc;
    DWORD dwBitIndex;
	DWORD dwCurrent;

    //
    // Prendre en compte un octet supplémentaire dans un calcul de CRC
    // Le polynome de calcul est X16+X12+X5+X0
    //

    dwCrc = ((DWORD)pbCrc[0]) | ( ((DWORD)pbCrc[1]) << 8 );
    dwCurrent = bByte;
	for( dwBitIndex = 0 ; dwBitIndex < 8 ; dwBitIndex ++, dwCurrent >>= 1 )
        dwCrc = ( dwCrc >> 1 ) ^ ( ( ( dwCurrent ^ dwCrc ) & 0x00000001 ) != 0 ? 0x00008408 : 0x00000000 );

	pbCrc[0] = (unsigned char)(dwCrc & 0x000000ff);
	pbCrc[1] = (unsigned char)((dwCrc & 0x0000ff00) >> 8);
}




PRIVATE void WINAPI ISACrcCompute( BYTE * pbBuffer, DWORD dwBufferBytes, BYTE * pbCrc )
{
	DWORD dwByteIndex;

    pbCrc[0] = 0xFF;
    pbCrc[1] = 0xFF;

	for( dwByteIndex = 0 ; dwByteIndex < dwBufferBytes ; dwByteIndex ++ )
        ISACrcAddByte( pbBuffer[dwByteIndex], pbCrc );

    pbCrc[0] = ~pbCrc[0];
    pbCrc[1] = ~pbCrc[1];
}



PRIVATE void WINAPI ISATrack( ISA_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes )
{
    BOOL    fFlush;
    DWORD   dwRemain;

    fFlush = ( ( dwContext & ISA_HOOK_FLUSH ) != 0 );
    dwContext = ( dwContext & ( ~ ISA_HOOK_FLUSH ) );

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


