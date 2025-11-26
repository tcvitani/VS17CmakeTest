/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : BAC
 * FILE       : BAC.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Couche 2 ISO (LLC) - Liaison communicateur.
 * --------------------------------------------------------------------
 * DESCRIPTION: Couche protocolaire de niveau 2 ISO pour la liaison
 *              entre hôte et communicateur en utilisant une liaison
 *              série en niveau 1 ISO.
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

#include "csr_tag_bac.h"

#include <memclass.h>


/*
PRIVATE void PrintTrace( char * szText )
{
    DWORD dwWritten;
    char szBuffer[1024];

    LONGLONG llTime;
    LONGLONG llFreq;


            QueryPerformanceFrequency( (LARGE_INTEGER*)&llFreq );
            QueryPerformanceCounter( (LARGE_INTEGER*)&llTime );
            llTime = ( llTime * (LONGLONG)1000 ) / llFreq;

    dwWritten = sprintf( szBuffer, "\r\n%I64u ### %s ###", llTime, szText );

    WriteFile( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, dwWritten, &dwWritten, NULL );
}
*/




//
// Caracteres de controle du protocole
//
#define BAC_CHAR_ENQ        '\x05'
#define BAC_CHAR_ACK        '\x06'
#define BAC_CHAR_NAK        '\x15'
#define BAC_CHAR_EOT        '\x04'
#define BAC_CHAR_DLE        '\x10'
#define BAC_CHAR_STX        '\x02'
#define BAC_CHAR_ETX        '\x03'


//
// Tailles de blocs
//

// Taille de l'entête protocolaire (DLE+STX)
#define BAC_HEADER_BYTES    2

// Taille de la fin de message protocolaire (DLE+ETX sans CRC)
#define BAC_FOOTER_BYTES    2

// Taille du CRC16
#define BAC_CRC_BYTES       2

// Taille des données protocolaire dans un message
#define BAC_PROTOCOL_BYTES  (BAC_HEADER_BYTES+BAC_FOOTER_BYTES+BAC_CRC_BYTES)


//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects qui serviront
// à provoquer des événement sur l'automate protocolaire
//
#define BAC_END_INDEX            0
#define BAC_COM_INDEX            1
#define BAC_TMR_INDEX            2
#define BAC_IRP_INDEX            3
#define BAC_EVT_COUNT            4


//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et CRC et caractères
// dédoublés si nécessaire).
//
#define BAC_IN_BUFFER_BYTES      2048
#define BAC_OUT_BUFFER_BYTES     2048
#define BAC_HOOK_BUFFER_BYTES    2048



//
// Hook utilisé pour l'espionnage de la liaison
//
#define BAC_CALL_HOOK(ctx,size,ptr)  if ( psInst->sParams.pfHook != NULL ) BACTrack( psInst, ctx, size, ptr ); else



//
// Etats d'analyse lors de la réception des caractères d'un message
//
typedef enum
{
    // Etat initial : le prochain caractère attendu est un DLE d'échappement précédent un STX
    BAC_MSG_WAIT_DLE_STX = 0,

    // Le prochain caractère attendu est un STX
    BAC_MSG_WAIT_STX,

    // Le prochain caractère attendu est un DLE d'échappement précédent un ETX, un 
    // DLE d'échappement précédent un DLE, ou un caractère des données utiles.
    BAC_MSG_WAIT_DLE_ETX,

    // Le prochain caractère attendu est un ETX
    BAC_MSG_WAIT_ETX,

    // Le prochain caractère attendu est le premier octet du CRC16
    BAC_MSG_WAIT_CRC1,

    // Etat final : le prochain caractère attendu est le second octet du CRC16
    BAC_MSG_WAIT_CRC2,
}
    BAC_MSG_WAIT_STATES;



//
// Définition d'un packet pour la file des messages en sortie
// et en entrée
//
typedef struct _BAC_PACKET
{
    void              * pvContext;
    DWORD             * pdwErrCode;
    DWORD               dwBufferBytes;
    BYTE                tbBuffer[];
}
    BAC_PACKET;

    
//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _BAC_INSTANCE
{
    // Paramètres de l'instance
    BAC_PARAMS_EX       sParams;

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
    HANDLE              thEvents[BAC_EVT_COUNT];

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // Comptes de tentatives de réémission
    DWORD               dwEnqRetry;
    DWORD               dwMsgRetry;

    // En phase d'émission, packet courant à émettre
    BAC_PACKET        * psCurrentPacket;

    // Analyse structurelle du message (définition du prochain caractère
    // attendu dans le message).
    BAC_MSG_WAIT_STATES eCurrentMsgState;

    // Dernier octet lu
    BYTE                bRead;

    // Taille occupée dans le buffer de réception
    DWORD               dwInBytes;

    // Buffer de réception
    BYTE                tbIn[ BAC_IN_BUFFER_BYTES ];

    // CRC courant calculé au fur et à mesure de la réception des octets
    BYTE                tbCrcComp[2];

    // CRC transmis avec le message recu
    BYTE                tbCrcRecv[2];

    // Taille occupée dans le buffer d'émission
    DWORD               dwOutBytes;

    // Buffer d'émission
    BYTE                tbOut[ BAC_OUT_BUFFER_BYTES ];

    // Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ BAC_HOOK_BUFFER_BYTES ];
}
    BAC_INSTANCE;



  
//
// Fonctions de transition de l'automate protocolaire
//
PRIVATE void WINAPI BAC_ACT_SEND_ENQ( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_RECV_ENQ( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_RECV_ACK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_RECV_ACK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_RECV_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_RECV_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_ACK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_ACK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_ERROR( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_CONTENSION( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_NAK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_SEND_NAK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );
PRIVATE void WINAPI BAC_ACT_IGNORE_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );


//
// Autres fonctions privées
//
PRIVATE void WINAPI BACMakeAutError( BAC_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE DWORD WINAPI BACRunThread( IN BAC_INSTANCE * psInst );
PRIVATE void WINAPI BACTerminateCurrentPacket( BAC_INSTANCE * psInst, DWORD dwErr );
PRIVATE DWORD WINAPI BACBuildMessage( BAC_INSTANCE * psInst );
PRIVATE void WINAPI BACCrcAddByte( BYTE bByte, BYTE * pbCrc );
PRIVATE void WINAPI BACCrcCompute( BYTE * pbBuffer, DWORD dwBufferBytes, BYTE * pbCrc );
PRIVATE void WINAPI BACTrack( BAC_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );



//
// Définitions des événements de l'automate protocolaire
//
typedef enum
{
    BAC_EVT_QUEUED_MSG = 0,
    BAC_EVT_RECV_CHAR     ,
    BAC_EVT_CTRL_ENQ      ,
    BAC_EVT_CTRL_ACK      ,
    BAC_EVT_CTRL_NAK      ,
    BAC_EVT_CTRL_EOT      ,
    BAC_EVT_TIMER         ,
    BAC_EVT_MSG_BAD       ,
    BAC_EVT_MSG_OK        ,
    BAC_EVT_ERROR         ,
    BAC_MAX_AUT_EVENTS
}
    BAC_AUT_EVENTS;


//
// Définitions des états de l'automate protocolaire
//
typedef enum
{
    BAC_ST_SLEEPING = 0,
    BAC_ST_WAIT_ACK1   ,
    BAC_ST_WAIT_ACK2   ,
    BAC_ST_WAIT_MSG    ,
    BAC_ST_WAIT_EOT    ,

    BAC_MAX_AUT_STATES
}
    BAC_AUT_STATES;




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI BACOpen( 
 *                      OUT BAC_INSTANCE  ** ppsInst,
 *                      IN  BAC_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication BAC
 *             (utilisé pour la communication HOTE<->Communicateur)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI BACOpen( 
        OUT BAC_INSTANCE  ** ppsInst,
        IN  BAC_PARAMS     * psParams )
{
    DWORD               dwErr       = ERROR_NOT_ENOUGH_MEMORY;
    BAC_INSTANCE      * psInst      = NULL;
    DWORD               dwId;

    __try
    {
        // The default retuned value is NULL
        (*ppsInst) = NULL;

        //
        // Allouer la place pour la structure d'instance et la remplir de 0
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BAC_INSTANCE) );
        if ( psInst == NULL ) 
            __leave;

        //
        // Reporter les valeurs des paramètres une fois qu'on s'est assuré que la
        // taille de la structure est compatible.
        //
        if ( ( psParams->dwStructBytes != sizeof(BAC_PARAMS   ) ) &&
             ( psParams->dwStructBytes != sizeof(BAC_PARAMS_EX) ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        memcpy( (void*)&psInst->sParams, (void*)psParams, psParams->dwStructBytes );

        //
        // Déterminer le type de timer à utiliser
        //
        if ( ( psInst->sParams.dwFlags & BAC_USE_WAITABLE_TIMER ) != 0 )
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
        psInst->thEvents[BAC_END_INDEX] = psInst->hEndEvent;
        psInst->thEvents[BAC_COM_INDEX] = SERGetWaitableHandle( psInst->psPort );
        psInst->thEvents[BAC_IRP_INDEX] = QueueGetWaitableHandle( psInst->psWriteQueue );
        psInst->thEvents[BAC_TMR_INDEX] = TimerGetWaitableHandle( psInst->psTimer );

        //
        // Création de l'automate protocolaire
        //
        psInst->hAut = AutOpen( BAC_MAX_AUT_STATES, psInst );
        if ( psInst->hAut == NULL )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Définition des états de l'automate protocolaire
        //
        if ( ( ! AutAddState( psInst->hAut, BAC_ST_SLEEPING  ) ) ||
             ( ! AutAddState( psInst->hAut, BAC_ST_WAIT_ACK1 ) ) ||
             ( ! AutAddState( psInst->hAut, BAC_ST_WAIT_ACK2 ) ) ||
             ( ! AutAddState( psInst->hAut, BAC_ST_WAIT_MSG  ) ) ||
             ( ! AutAddState( psInst->hAut, BAC_ST_WAIT_EOT  ) ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Définition des transitions de l'automate protocolaire
        //
        if ( ( ! AutAddTransition( psInst->hAut, BAC_ST_SLEEPING , BAC_EVT_QUEUED_MSG, BAC_ST_WAIT_ACK1, BAC_ACT_SEND_ENQ  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_SLEEPING , BAC_EVT_RECV_CHAR , BAC_ST_SLEEPING , BAC_ACT_RECV_ENQ  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_SLEEPING , BAC_EVT_CTRL_ENQ  , BAC_ST_WAIT_MSG , BAC_ACT_SEND_ACK1 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_SLEEPING , BAC_EVT_ERROR     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_SLEEPING , AUT_EVENT_ANY     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||

             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_RECV_CHAR , BAC_ST_WAIT_ACK1, BAC_ACT_RECV_ACK1 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_CTRL_ACK  , BAC_ST_WAIT_ACK2, BAC_ACT_SEND_MSG  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_CTRL_NAK  , BAC_ST_WAIT_ACK1, BAC_ACT_SEND_ENQ  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_CTRL_ENQ  , BAC_ST_SLEEPING , BAC_ACT_CONTENSION) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_TIMER     , BAC_ST_WAIT_ACK1, BAC_ACT_SEND_ENQ  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, BAC_EVT_ERROR     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK1, AUT_EVENT_ANY     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
     
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, BAC_EVT_RECV_CHAR , BAC_ST_WAIT_ACK2, BAC_ACT_RECV_ACK2 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, BAC_EVT_CTRL_ACK  , BAC_ST_SLEEPING , BAC_ACT_SEND_EOT  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, BAC_EVT_CTRL_NAK  , BAC_ST_WAIT_ACK2, BAC_ACT_SEND_MSG  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, BAC_EVT_TIMER     , BAC_ST_WAIT_ACK2, BAC_ACT_SEND_MSG  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, BAC_EVT_ERROR     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_ACK2, AUT_EVENT_ANY     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||

             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_RECV_CHAR , BAC_ST_WAIT_MSG , BAC_ACT_RECV_MSG  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_CTRL_ENQ  , BAC_ST_WAIT_MSG , BAC_ACT_SEND_ACK1 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_MSG_OK    , BAC_ST_WAIT_EOT , BAC_ACT_SEND_ACK2 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_MSG_BAD   , BAC_ST_WAIT_MSG , BAC_ACT_SEND_NAK2 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_TIMER     , BAC_ST_SLEEPING , BAC_ACT_SEND_NAK1 ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , BAC_EVT_ERROR     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_MSG , AUT_EVENT_ANY     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||

             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_EOT , BAC_EVT_RECV_CHAR , BAC_ST_WAIT_EOT , BAC_ACT_RECV_EOT  ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_EOT , BAC_EVT_CTRL_EOT  , BAC_ST_SLEEPING , BAC_ACT_EOT       ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_EOT , BAC_EVT_TIMER     , BAC_ST_SLEEPING , BAC_ACT_IGNORE_EOT) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_EOT , BAC_EVT_ERROR     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) ||
             ( ! AutAddTransition( psInst->hAut, BAC_ST_WAIT_EOT , AUT_EVENT_ANY     , BAC_ST_SLEEPING , BAC_ACT_ERROR     ) ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Placer l'automate dans son état initial
        //
        if ( ! AutSetCurrentState( psInst->hAut, BAC_ST_SLEEPING ) )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        //
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)BACRunThread, (void*)psInst, 0, &dwId, "TAG_BAC_PROTOCOL" );
        psInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)BACRunThread, (void*)psInst, 0, &dwId );
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
            BACClose( psInst, 0 );

    }

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI BACClose( 
 *                      IN BAC_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par BACOpen
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             BACOpen(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI BACClose( 
        IN BAC_INSTANCE   * psInst,
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
 * SYNTAX    : EXPORT DWORD WINAPI BACSendMessage( 
 *                      IN              BAC_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par BACOpen
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
EXPORT DWORD WINAPI BACSendMessage( 
        IN              BAC_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    BAC_PACKET    * psPacket = NULL;

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
        psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + dwMsgBytes );
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
 * SYNTAX    : EXPORT HANDLE WINAPI BACGetWaitableHandle(
 *                      IN BAC_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par BACOpen
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
EXPORT HANDLE WINAPI BACGetWaitableHandle(
        IN BAC_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI BACGetCommInstance(
 *                      IN BAC_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par BACOpen
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI BACGetCommInstance(
        IN BAC_INSTANCE * psInst )
{
    return psInst->psPort;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI BACReceiveMessage(
 *                      IN      BAC_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par BACOpen
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
 *             à BACFreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI BACReceiveMessage(
        IN      BAC_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    BAC_PACKET    * psPacket;

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
 * SYNTAX    : EXPORT void WINAPI BACFreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par BACReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à BACReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI BACFreeMessage(
        IN BYTE * pbMsg )
{
    BAC_PACKET * psPacket = NULL;

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
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI BACSetOption(
 *                      IN BAC_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par BACOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour BAC)
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
EXPORT DWORD WINAPI WINAPI BACSetOption(
        IN BAC_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI BACGetOption(
 *                      IN BAC_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par BACOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour BAC)
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
EXPORT DWORD WINAPI WINAPI BACGetOption(
        IN BAC_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}




PRIVATE void WINAPI BAC_ACT_SEND_ENQ( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr = NO_ERROR;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    static BYTE tbEnq[] = { BAC_CHAR_ENQ };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Si le nombre max de tentative n'est pas atteint
    if ( psInst->dwEnqRetry <= psInst->sParams.dwMaxEnqRetry )
    {
        // Incrémenter le nombre de tentative
        psInst->dwEnqRetry ++;

        // Envoyer le caractère ENQ
        if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
        dwErr = SERWrite( psInst->psPort, tbEnq, sizeof(tbEnq) );
        if ( dwErr == NO_ERROR )
        {
            BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbEnq), tbEnq );

            // Le caractère a été envoyé, on active le timer
            dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer1 );
            if ( dwErr != NO_ERROR )
            {
                BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ENQ : TimerActivate" );
                return;
            }
        }
        else
        {
            // Erreur grave de comm, on dépile le packet pour pouvoir l'annuler
            QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
            // L'envoi a échoué, on provoque une erreur (l'IRP en cours est annulé)
            BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ENQ : SERWrite" );
            return;
        }
    }
    else
    {
        // Nombre max de tentatives effectué, on annule l'IRP en cours
        QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
        BACMakeAutError( psInst, ERROR_ACCESS_DENIED, "BAC_ACT_SEND_ENQ : MaxRetry" );
        return;
    }
}




PRIVATE void WINAPI BAC_ACT_CONTENSION( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    BAC_INSTANCE * psInst = AutGetContext( hAut );

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    if ( ( psInst->sParams.dwFlags & BAC_CONTENSION_CANCEL ) != 0 )
    {
        // Nombre max de tentatives effectué, on enlève de la fifo l'IRP en cours
        QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );

        // Annuler le packet en cours
        BACTerminateCurrentPacket( psInst, ERROR_CANCELLED );

        psInst->dwLastError = ERROR_CANCELLED;
        strcpy( psInst->szLastError, "BAC_ACT_CONTENSION : MessageCancel" );

        // Reseter les compteurs
        psInst->dwMsgRetry = 0;
        psInst->dwEnqRetry = 0;
        psInst->dwOutBytes = 0;
        psInst->dwInBytes = 0;
        psInst->dwOutBytes = 0;
        psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
        psInst->tbCrcComp[0] = 0;
        psInst->tbCrcComp[1] = 0;
    }

    // Propager l'ENQ qui a provoqué la contension pour passer
    // directement en phase de réception
    AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ENQ, NULL );
}




PRIVATE void WINAPI BAC_ACT_SEND_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Si le nombre max de tentative n'est pas atteint
    if ( psInst->dwMsgRetry <= psInst->sParams.dwMaxMsgRetry )
    {
        // Incrémenter le nombre de tentative
        psInst->dwMsgRetry ++;

        // Si le message n'a pas encore été construit
        if ( psInst->psCurrentPacket == NULL )
        {
            // Extraire le message de la file d'attente
            dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
            if ( dwErr != NO_ERROR )
            {
                BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_MSG : QueueReadItem" );
                return;
            }

            // Construire le message
            dwErr = BACBuildMessage( psInst );
            if ( dwErr != NO_ERROR )
            {
                BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_MSG : BACBuildMessage" );
                return;
            }
        }

        // Envoyer le message sur le port série
        if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
        dwErr = SERWrite( psInst->psPort, psInst->tbOut, psInst->dwOutBytes );
        if ( dwErr == NO_ERROR )
        {
            BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH,
                BAC_HEADER_BYTES, 
                psInst->tbOut );

            BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_DATA | BAC_HOOK_FLUSH,
                psInst->dwOutBytes - BAC_PROTOCOL_BYTES, 
                psInst->tbOut + BAC_HEADER_BYTES );

            BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH,
                BAC_FOOTER_BYTES + BAC_CRC_BYTES, 
                psInst->tbOut + psInst->dwOutBytes - BAC_FOOTER_BYTES - BAC_CRC_BYTES );

            // Le message a été envoyé, on active le timer
            dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer1 );
            if ( dwErr != NO_ERROR )
            {
                BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_MSG : TimerActivate" );
                return;
            }
        }
        else
        {
            // L'envoi a échoué, on provoque une erreur, on annule l'IRP en cours
            BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_MSG : SERWrite" );
            return;
        }
    }
    else
    {
        // Si pas d'IRP extrait, en extraire 1
        if ( psInst->psCurrentPacket == NULL )
            QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
        BACMakeAutError( psInst, ERROR_INVALID_DATA, "BAC_ACT_SEND_MSG : MaxRetry" );
        return;
    }
}




PRIVATE void WINAPI BAC_ACT_SEND_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr = NO_ERROR;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    static BYTE tbEot[] = { BAC_CHAR_EOT };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Le packet a bien été traité
    BACTerminateCurrentPacket( psInst, NO_ERROR );

    // Envoyer le caractère EOT
    // On ignore les erreurs sur envoi EOT
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, tbEot, sizeof(tbEot) );

    if ( dwErr == NO_ERROR )
        BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbEot), tbEot );

    // Reinitialiser les compteurs
    psInst->dwEnqRetry = 0;
    psInst->dwMsgRetry = 0;
    psInst->psCurrentPacket = NULL;
}




PRIVATE void WINAPI BAC_ACT_SEND_ACK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr = NO_ERROR;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    static BYTE tbAck[] = { BAC_CHAR_ACK };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Envoyer le caractère ACK
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, tbAck, sizeof(tbAck) );
    if ( dwErr != NO_ERROR )
    {
         // L'envoi a échoué, on provoque une erreur
         BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ACK : SERWrite" );
         return;
    }
    else
    {
        BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbAck), tbAck );

        psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
        // Le message a été envoyé, on active le timer
        dwErr = TimerActivate( psInst->psTimer, (DWORD)psInst->sParams.dwTimer2 );
        if ( dwErr != NO_ERROR )
        {
             BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ACK : TimerActivate" );
             return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_SEND_ACK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr = NO_ERROR;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    BAC_PACKET * psPacket;
    static BYTE tbAck[] = { BAC_CHAR_ACK };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Envoyer le caractère ACK
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, tbAck, sizeof(tbAck) );
    if ( dwErr != NO_ERROR )
    {
         // L'envoi a échoué, on provoque une erreur
         BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ACK2 : SERWrite" );
         return;
    }
    else
    {
        BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbAck), tbAck );

        // Le message a été envoyé, on active le timer
        dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer1 + psInst->sParams.dwTimer2 );
        if ( dwErr != NO_ERROR )
        {
             BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ACK : TimerActivate" );
             return;
        }
    }

    // Préparer le packet de lecture
    psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + psInst->dwInBytes );
    if ( psPacket != NULL )
    {
        // Mettre à zéro l'entête du packet
        ZeroMemory( psPacket, sizeof( *psPacket ) );

        psPacket->dwBufferBytes = psInst->dwInBytes;
        CopyMemory( psPacket->tbBuffer, psInst->tbIn, psPacket->dwBufferBytes );

        // Repartir à zéro avec le buffer et l'état de reception du message
        psInst->dwInBytes = 0;
        psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
        psInst->tbCrcComp[0] = 0;
        psInst->tbCrcComp[1] = 0;

        // Ajouter l'item dans la file d'attente
        dwErr = QueueWriteItem( psInst->psReadQueue, psPacket, INFINITE );
        if ( dwErr != NO_ERROR )
        {
            // Echec dans la file d'attente : On libère le buffer alloué
            HeapFree( GetProcessHeap(), 0, psPacket );
           
            // La reception a échouée, on provoque une erreur
            BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_ACK2 : QueueWriteItem" );
            return;
        }
    }
    else
    {
        // L'allocation a échouée, on provoque une erreur
        BACMakeAutError( psInst, ERROR_NOT_ENOUGH_MEMORY, "BAC_ACT_SEND_ACK2 : HeapAlloc" );
        return;
    }
}




PRIVATE void WINAPI BAC_ACT_SEND_NAK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    static BYTE tbNak[] = { BAC_CHAR_NAK };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Envoyer le caractère NAK
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, tbNak, sizeof(tbNak) );
    if ( dwErr != NO_ERROR )
    {
        // L'envoi a échoué, on provoque une erreur
        BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_NAK : SERWrite" );
        return;
    }
    else
    {
        BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbNak), tbNak );
    }
}




PRIVATE void WINAPI BAC_ACT_SEND_NAK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    static BYTE tbNak[] = { BAC_CHAR_NAK };

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Reseter le contenu du buffer
    psInst->dwInBytes = 0;
    psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
    psInst->tbCrcComp[0] = 0;
    psInst->tbCrcComp[1] = 0;

    // Envoyer le caractère NAK
    if ( psInst->sParams.dwAutoClearBuffers != 0 ) SEREmptyBuffer( psInst->psPort );
    dwErr = SERWrite( psInst->psPort, tbNak, sizeof(tbNak) );
    if ( dwErr != NO_ERROR )
    {
        // L'envoi a échoué, on provoque une erreur
        BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_NAK : SERWrite" );
        return;
    }
    else
    {
        BAC_CALL_HOOK( BAC_HOOK_OUTPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, sizeof(tbNak), tbNak );

        // Le message a été envoyé, on active le timer
        dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer2 );
        if ( dwErr != NO_ERROR )
        {
            BACMakeAutError( psInst, dwErr, "BAC_ACT_SEND_NAK : TimerActivate" );
            return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    BAC_INSTANCE * psInst = AutGetContext( hAut );

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );
}




PRIVATE void WINAPI BAC_ACT_IGNORE_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    BAC_INSTANCE * psInst = AutGetContext( hAut );

    // Annuler tout timer en cours
    TimerAcknowledge( psInst->psTimer );
}




PRIVATE void WINAPI BAC_ACT_RECV_ENQ( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;

    // Lire le caractère suivant
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( ( dwErr == NO_ERROR ) && ( dwCount == 1 ) )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

        // En fonction du caractère
        if      ( psInst->bRead == BAC_CHAR_ENQ )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ENQ, NULL );
        else
        {
            // Rien à faire, le caractère est ignoré
        }
    }
    else
    {
        // Ne pas provoquer d'erreur si buffer reception comm vide
        if ( dwErr != NO_ERROR )
        {
            // Erreur de lecture, on provoque une erreur
            BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_ENQ : SERRead" );
            return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_RECV_ACK1( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;

    // Lire le caractère suivant
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( ( dwErr == NO_ERROR ) && ( dwCount == 1 ) )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

        // En fonction du caractère
        if      ( psInst->bRead == BAC_CHAR_ENQ )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ENQ, NULL );

        else if ( psInst->bRead == BAC_CHAR_ACK )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ACK, NULL );

        else if ( psInst->bRead == BAC_CHAR_NAK )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_NAK, NULL );

        else
        {
            // Rien à faire, le caractère est ignoré
        }
    }
    else
    {
        // Ne pas provoquer d'erreur si buffer reception comm vide
        if ( dwErr != NO_ERROR )
        {
            // Erreur de lecture, on provoque une erreur
            BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_ACK1 : SERRead" );
            return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_RECV_ACK2( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;

    // Lire le caractère suivant
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( ( dwErr == NO_ERROR ) && ( dwCount == 1 ) )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

        // En fonction du caractère
        if      ( psInst->bRead == BAC_CHAR_ACK )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ACK, NULL );

        else if ( psInst->bRead == BAC_CHAR_NAK )
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_NAK, NULL );

        else
        {
            // Rien à faire, le caractère est ignoré
        }
    }
    else
    {
        // Ne pas provoquer d'erreur si buffer reception comm vide
        if ( dwErr != NO_ERROR )
        {
            // Erreur de lecture, on provoque une erreur
            BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_ACK2 : SERRead" );
            return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_RECV_EOT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;

    // Lire le caractère suivant
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    if ( ( dwErr == NO_ERROR ) && ( dwCount == 1 ) )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

        // Qu'il s'agisse d'un EOT ou d'un autre caractère, on fait comme si
        // c'était un EOT dans tous les cas
        AutSendEvent( psInst->hAut, BAC_EVT_CTRL_EOT, NULL );
    }
    else
    {
        // Ne pas provoquer d'erreur si buffer reception comm vide
        if ( dwErr != NO_ERROR )
        {
            // Erreur de lecture, on provoque une erreur
            BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_EOT : SERRead" );
            return;
        }
    }
}




PRIVATE void WINAPI BAC_ACT_RECV_MSG( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    DWORD dwErr;
    BAC_INSTANCE * psInst = AutGetContext( hAut );
    DWORD dwCount = 1;
    static BYTE tbDleEtx[] = { BAC_CHAR_DLE, BAC_CHAR_ETX };
    static BYTE tbDleDle[] = { BAC_CHAR_DLE, BAC_CHAR_DLE };
    static BYTE tbDle[]    = { BAC_CHAR_DLE };
    
    TimerCancel( psInst->psTimer );

    // Lire le caractère suivant
    dwErr = SERRead( psInst->psPort, &psInst->bRead, &dwCount );
    // Ne pas provoquer d'erreur si buffer reception comm vide
    if ( dwErr != NO_ERROR )
    {
        // Erreur de lecture, on provoque une erreur
        BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_CHAR : SERRead" );
        return;
    }

    // Réarmer le timer
    dwErr = TimerActivate( psInst->psTimer, psInst->sParams.dwTimer2 );
    if ( dwErr != NO_ERROR )
    {
        BACMakeAutError( psInst, dwErr, "BAC_ACT_RECV_MSG : TimerActivate" );
        return;
    }

    if ( dwCount != 1 )
        return;

    // Interpréter le caractère en fonction de celui attendu

    // ON ATTENT LE DLE PRECEDENT LE PREMIER STX
    if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_DLE_STX )
    {
        if ( psInst->bRead == BAC_CHAR_DLE )
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL, 1, &psInst->bRead );

            // On attent désormais le caractère STX
            psInst->eCurrentMsgState = BAC_MSG_WAIT_STX;
        }

        else if ( psInst->bRead == BAC_CHAR_ENQ )
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

            // Le ACK qu'on a renvoyé n'a pas été reçu, l'autre coté renvoie un ENQ
            // On reboucle
            AutSendEvent( psInst->hAut, BAC_EVT_CTRL_ENQ, NULL );
        }

        else
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

            // On ignore et on repart à zéro
            psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
        }
    }

    // ON ATTEND LE PREMIER STX (LE DLE VIENT D'ETRE RECU)
    else if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_STX )
    {
        if ( psInst->bRead == BAC_CHAR_STX )
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL, 1, &psInst->bRead );

            // On attent désormais les caractères du message ou un DLE précedant un ETX
            psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_ETX;
        }
        else
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

            // On ignore et on repart à zéro
            psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
        }
    }

    // ON ATTEND LES CARACTERES DU MESSAGE OU LE DLE QUI PRECEDE LE ETX
    else if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_DLE_ETX )
    {
        if ( psInst->bRead == BAC_CHAR_DLE )
        {
            // Qu'il s'agisse d'un double DLE ou DLE/ETX, le caractère compte pour le CRC
            BACCrcAddByte( psInst->bRead, psInst->tbCrcComp );
            // On attent soit l'EXT soit un autre DLE
            psInst->eCurrentMsgState = BAC_MSG_WAIT_ETX;
        }
        else 
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_DATA, 1, &psInst->bRead );

            // Si saturation buffer
            if ( psInst->dwInBytes >= sizeof( psInst->tbIn ) )
            {
                BACMakeAutError( psInst, ERROR_NOT_ENOUGH_MEMORY, "BAC_ACT_RECV_MSG : Buffer saturation" );
                return;
            }
            // Le prendre en compte dans le calcul du CRC
            BACCrcAddByte( psInst->bRead, psInst->tbCrcComp );
            // Ajouter caractère au buffer
            psInst->tbIn[psInst->dwInBytes] = psInst->bRead;
            psInst->dwInBytes ++;
            // On attent toujours les caractères du message ou un DLE précedant un ETX
            psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_ETX;
        }
    }

    // ON ATTENT LE ETX OU LE DLE DOUBLE (UN DLE VIENT D'ETRE RECU)
    else if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_ETX )
    {
        if ( psInst->bRead == BAC_CHAR_ETX )
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL, sizeof(tbDleEtx), tbDleEtx );

            // Le prendre en compte dans le calcul du CRC
            BACCrcAddByte( psInst->bRead, psInst->tbCrcComp );
            // Le message est terminé, on attend le premier caractère du CRC
            psInst->eCurrentMsgState = BAC_MSG_WAIT_CRC1;
        }
        else if ( psInst->bRead == BAC_CHAR_DLE )
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_DATA, sizeof(tbDleDle), tbDleDle );

            // Doublage de DLE, il fait partie des caractères du message
            // Si saturation buffer
            if ( psInst->dwInBytes >= sizeof( psInst->tbIn ) )
            {
                BACMakeAutError( psInst, ERROR_NOT_ENOUGH_MEMORY, "BAC_ACT_RECV_MSG : Buffer saturation" );
                return;
            }
            // Le prendre en compte dans le calcul du CRC
            BACCrcAddByte( psInst->bRead, psInst->tbCrcComp );
            // Ajouter caractère au buffer
            psInst->tbIn[psInst->dwInBytes] = psInst->bRead;
            psInst->dwInBytes ++;
            // On attent toujours les caractères du message ou un DLE précedant un ETX
            psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_ETX;
        }
        else
        {
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_DATA, sizeof(tbDle), tbDle );
            BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_DATA | BAC_HOOK_FLUSH, 1, &psInst->bRead );
            AutSendEvent( psInst->hAut, BAC_EVT_MSG_BAD, NULL );
        }
    }

    // ON ATTENT LE PREMIER OCTET DU CRC
    else if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_CRC1 )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL, 1, &psInst->bRead );
        psInst->tbCrcRecv[0] = psInst->bRead;
        psInst->eCurrentMsgState = BAC_MSG_WAIT_CRC2;
    }

    // ON ATTENT LE SECOND OCTET DU CRC
    else if ( psInst->eCurrentMsgState == BAC_MSG_WAIT_CRC2 )
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );
        psInst->tbCrcRecv[1] = psInst->bRead;

        // Changer l'état de l'automate en fonction de la validité du CRC
        if ( ( psInst->tbCrcRecv[0] == psInst->tbCrcComp[0] ) && ( psInst->tbCrcRecv[1] == psInst->tbCrcComp[1] ) )
            AutSendEvent( psInst->hAut, BAC_EVT_MSG_OK, NULL );
        else
            AutSendEvent( psInst->hAut, BAC_EVT_MSG_BAD, NULL );
    }

    else
    {
        BAC_CALL_HOOK( BAC_HOOK_INPUT | BAC_HOOK_PROTOCOL | BAC_HOOK_FLUSH, 1, &psInst->bRead );

        BACMakeAutError( psInst, ERROR_INVALID_DATA, "BAC_ACT_RECV_MSG : Internal error" );
        return;
    }
}




PRIVATE void WINAPI BAC_ACT_ERROR( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam )
{
    BAC_INSTANCE * psInst = AutGetContext( hAut );

    // Annuler tout timer en cours
    TimerCancel( psInst->psTimer );

    // Reseter les compteurs
    psInst->dwMsgRetry = 0;
    psInst->dwEnqRetry = 0;
    psInst->dwOutBytes = 0;
    psInst->dwInBytes = 0;
    psInst->dwOutBytes = 0;
    psInst->eCurrentMsgState = BAC_MSG_WAIT_DLE_STX;
    psInst->tbCrcComp[0] = 0;
    psInst->tbCrcComp[1] = 0;
}




PRIVATE void WINAPI BACMakeAutError( BAC_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    BACTerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
    strcpy( psInst->szLastError, pcText );
    AutSendEvent( psInst->hAut, BAC_EVT_ERROR, NULL );
}




PRIVATE DWORD WINAPI BACRunThread( 
        IN BAC_INSTANCE * psInst )
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
        if ( AutGetCurrentState( psInst->hAut ) == BAC_ST_SLEEPING )
        {
            //
            // Prendre compte les messages à émettre
            //
            dwCount = BAC_EVT_COUNT;
        }
        else
        {
            //
            // Ne pas prendre en compte les messages à émettre
            //
            dwCount = BAC_EVT_COUNT - 1;
        }

        if ( SERGetInputBufferStatus( psInst->psPort ) == 0 )
        {
            dwRes =  WaitForMultipleObjects( dwCount, psInst->thEvents, FALSE, INFINITE );
            dwIndex = dwRes - WAIT_OBJECT_0;
        }
        else
            dwIndex = BAC_COM_INDEX;


        if ( dwIndex == BAC_END_INDEX )
        {
            //
            // Signal de fin reçu, on sort de la boucle
            //
            break;
        }

        else if ( dwIndex == BAC_TMR_INDEX )
        {
            //
            // Si événement du à timer
            //
            TimerAcknowledge( psInst->psTimer );
            AutSendEvent( psInst->hAut, BAC_EVT_TIMER, NULL );
        }

        else if ( dwIndex == BAC_COM_INDEX )
        {
            //
            // Si événement du à comm entrante
            //
            AutSendEvent( psInst->hAut, BAC_EVT_RECV_CHAR, NULL );
        }

        else if ( dwIndex == BAC_IRP_INDEX )
        {
            //
            // Si événement du à la présence d'un request packet en émission
            //
            AutSendEvent( psInst->hAut, BAC_EVT_QUEUED_MSG, NULL );
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
        BACTerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

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




PRIVATE void WINAPI BACTerminateCurrentPacket( BAC_INSTANCE * psInst, DWORD dwErr )
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




PRIVATE DWORD WINAPI BACBuildMessage( BAC_INSTANCE * psInst )
{
    DWORD dwIndex;
    static BYTE tbHeader[] = { BAC_CHAR_DLE, BAC_CHAR_STX };
    static BYTE tbFooter[] = { BAC_CHAR_DLE, BAC_CHAR_ETX };

    //
    // Position courant dans le buffer, au début, 0
    //
    psInst->dwOutBytes = 0;

    //
    // Si aucun message à envoyer, ce n'est pas normal
    //
    if ( psInst->psCurrentPacket == NULL )
        return ERROR_INVALID_DATA;

    //
    // Placer l'entête du message
    //
    CopyMemory( 
        &psInst->tbOut[psInst->dwOutBytes], 
        tbHeader, 
        sizeof(tbHeader) );

    //
    // Incrémenter la position d'autant
    //
    psInst->dwOutBytes += BAC_HEADER_BYTES;

    //
    // Pour chaque octet du message à envoyer
    //
    for ( dwIndex = 0 ; dwIndex < psInst->psCurrentPacket->dwBufferBytes ; dwIndex ++ )
    {
        //
        // Vérifier que l'octet ne sort pas du buffer
        //
        if ( psInst->dwOutBytes >= BAC_OUT_BUFFER_BYTES - BAC_FOOTER_BYTES - BAC_CRC_BYTES )
        {
            //
            // Si c'est le cas, on retourne une erreur
            //
            psInst->dwOutBytes = 0;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        //
        // Recopier l'octet
        //
        psInst->tbOut[psInst->dwOutBytes] = psInst->psCurrentPacket->tbBuffer[dwIndex];

        //
        // Incrémenter la position
        //
        psInst->dwOutBytes ++;

        //
        // Si le caractère qu'on vient de recopier est DLE (caractère déchapement),
        // il faut le doubler
        //
        if ( psInst->psCurrentPacket->tbBuffer[dwIndex] == BAC_CHAR_DLE )
        {
            //
            // Vérifier que le nouvel octet ne sort pas du buffer
            //
            if ( psInst->dwOutBytes >= BAC_OUT_BUFFER_BYTES - BAC_FOOTER_BYTES - BAC_CRC_BYTES )
            {
                //
                // Si c'est le cas, on retourne une erreur
                //
                psInst->dwOutBytes = 0;
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            //
            // Recopier l'octet
            //
            psInst->tbOut[psInst->dwOutBytes] = psInst->psCurrentPacket->tbBuffer[dwIndex];

            //
            // Incrémenter la position
            //
            psInst->dwOutBytes ++;
        }
    }

    //
    // Placer la fin du message
    //
    CopyMemory( 
        &psInst->tbOut[psInst->dwOutBytes], 
        tbFooter,
        sizeof(tbFooter) );

    //
    // Incrémenter la position d'autant
    //
    psInst->dwOutBytes += BAC_FOOTER_BYTES;

    //
    // Placer enfin le CRC à la fin
    //
    BACCrcCompute( 
        &psInst->tbOut[BAC_HEADER_BYTES], 
        psInst->dwOutBytes - BAC_HEADER_BYTES, 
        &psInst->tbOut[psInst->dwOutBytes] );

    //
    // Incrémenter la position de la taille du CRC
    //
    psInst->dwOutBytes += BAC_CRC_BYTES;

    return NO_ERROR;
}




PRIVATE void WINAPI BACCrcAddByte( BYTE bByte, BYTE * pbCrc )
{
	DWORD dwCrc;
    DWORD dwBitIndex;
	DWORD dwCurrent;

    //
    // Prendre en compte un octet supplémentaire dans un calcul de CRC
    // Le polynome de calcul est X16+X15+X2+X0
    //
    dwCrc = ((DWORD)pbCrc[0]) | ( ((DWORD)pbCrc[1]) << 8 );
    dwCurrent = bByte;
	for( dwBitIndex = 0 ; dwBitIndex < 8 ; dwBitIndex ++, dwCurrent >>= 1 )
        dwCrc = ( dwCrc >> 1 ) ^ ( ( ( dwCurrent ^ dwCrc ) & 0x00000001 ) != 0 ? 0x0000A001 : 0x00000000 );

	pbCrc[0] = (unsigned char)(dwCrc & 0x000000ff);
	pbCrc[1] = (unsigned char)((dwCrc & 0x0000ff00) >> 8);
}




PRIVATE void WINAPI BACCrcCompute( BYTE * pbBuffer, DWORD dwBufferBytes, BYTE * pbCrc )
{
	DWORD dwByteIndex;

    pbCrc[0] = 0;
    pbCrc[1] = 0;

	for( dwByteIndex = 0 ; dwByteIndex < dwBufferBytes ; dwByteIndex ++ )
        BACCrcAddByte( pbBuffer[dwByteIndex], pbCrc );
}



PRIVATE void WINAPI BACTrack( BAC_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes )
{
    BOOL    fFlush;
    DWORD   dwRemain;

    fFlush = ( ( dwContext & BAC_HOOK_FLUSH ) != 0 );
    dwContext = ( dwContext & ( ~ BAC_HOOK_FLUSH ) );

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




