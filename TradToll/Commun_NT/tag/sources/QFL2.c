/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFL2
 * FILE       : QFL2.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Couche 2 QFL2
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

#include "csr_tag_qfl2.h"

#include <memclass.h>



#define QFL2_PIPE_TIMEOUT         5000

//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects et qui
// serviront à provoquer des événements sur l'automate protocolaire
//
#define QFL2_END_INDEX            0
#define QFL2_IRP_INDEX            1
#define QFL2_EVT_COUNT            2


//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et LRC).
//
#define QFL2_IN_BUFFER_BYTES      256
#define QFL2_OUT_BUFFER_BYTES     256
#define QFL2_HOOK_BUFFER_BYTES    256


//
// Hook utilisé pour l'espionnage de la liaison
//
#define QFL2_CALL_HOOK(ctx,size,ptr)  if ( psInst->sParams.pfHook != NULL ) QFL2Track( psInst, ctx, size, ptr ); else



//
// Définition d'un packet pour la file des messages en sortie
// et en entrée
//
typedef struct _QFL2_PACKET
{
    void              * pvContext;
    DWORD             * pdwErrCode;
    DWORD               dwBufferBytes;
    BYTE                tbBuffer[];
}
    QFL2_PACKET;

    
//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _QFL2_INSTANCE
{
    // Paramètres de l'instance
    QFREE_PARAMS        sParams;

    CRITICAL_SECTION    sSerialize;

    // Handle du thread de gestion du protocole
    HANDLE              hThread;

    // Flag indiquant que le port a été ouvert
    BOOLEAN             fPortOpen;

    // Handle of the pipe
    HANDLE              hPipe;

    // Handle of the event for async i/o
    HANDLE              hAsyncEvent;

    // Structure for the async i/o
    OVERLAPPED          sAsync;

    // File d'attente pour les messages sortant
    QUEUE_INSTANCE    * psWriteQueue;

    // File d'attente pour les messages entrant
    QUEUE_INSTANCE    * psReadQueue;

    // Evénement signalé si la file psReadQueue a au moins un élément
    // et non signalé si la psReadQueue est vide
    HANDLE              hEndEvent;

    // Tableau des handles signalables servant à animer l'automate
    HANDLE              thEvents[QFL2_EVT_COUNT];

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // En phase d'émission, packet courant à émettre
    QFL2_PACKET        * psCurrentPacket;

    // Taille occupée dans le buffer de réception
    DWORD               dwInBytes;

    // Buffer de réception
    BYTE                tbIn[ QFL2_IN_BUFFER_BYTES ];

    // Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ QFL2_HOOK_BUFFER_BYTES ];


}
    QFL2_INSTANCE;


//
// Autres fonctions privées
//
PRIVATE void WINAPI QFL2MakeError( QFL2_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE void WINAPI QFL2HandleIRP( QFL2_INSTANCE * psInst );
PRIVATE DWORD WINAPI QFL2RunThread( IN QFL2_INSTANCE * psInst );
PRIVATE void WINAPI QFL2TerminateCurrentPacket( QFL2_INSTANCE * psInst, DWORD dwErr );
PRIVATE void WINAPI QFL2Track( QFL2_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );
PRIVATE DWORD WINAPI QFL2VerifyOpenPort( QFL2_INSTANCE * psInst );
PRIVATE DWORD WINAPI QFL2WriteRead( QFL2_INSTANCE * psInst, void * pvOut, DWORD dwOut, void * pvIn, DWORD * pdwIn );
PRIVATE DWORD WINAPI QFL2ClearPipe( QFL2_INSTANCE * psInst );
PRIVATE WINAPI WINAPI QFL2DirectStop( QFL2_INSTANCE * psInst );
PRIVATE WINAPI WINAPI QFL2DirectPortParams( IN QFL2_INSTANCE * psInst );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI QFL2Open( 
 *                      OUT QFL2_INSTANCE  ** ppsInst,
 *                      IN  QFREE_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication QFL2
 *             (utilisé pour la communication HOTE<->BADGE par liaison filaire)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI QFL2Open( 
        OUT QFL2_INSTANCE  ** ppsInst,
        IN  QFREE_PARAMS     * psParams )
{
    DWORD               dwErr       = NO_ERROR;
    QFL2_INSTANCE      * psInst      = NULL;
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
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(QFL2_INSTANCE) );
        if ( psInst == NULL ) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        InitializeCriticalSection( &psInst->sSerialize );

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
        // Créer l'événement utilisé pour les i/o asynchrones
        //
        psInst->hAsyncEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psInst->hAsyncEvent == NULL )
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
        // Initialisation de la liaison
        //
        dwErr = QFL2VerifyOpenPort( psInst );
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        //
        psInst->thEvents[QFL2_END_INDEX] = psInst->hEndEvent;
        psInst->thEvents[QFL2_IRP_INDEX] = QueueGetWaitableHandle( psInst->psWriteQueue );

        //
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFL2RunThread, (void*)psInst, 0, &dwId, "TAG_QFL2_PROTOCOL" );
        psInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)QFL2RunThread, (void*)psInst, 0, &dwId );
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
            QFL2Close( psInst, 0 );

    }

    return dwErr;
}




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
        IN DWORD            dwTimeout )
{
    if ( psInst != NULL )
    {
        EnterCriticalSection( &psInst->sSerialize );

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
        if ( psInst->hAsyncEvent != NULL )
            CloseHandle( psInst->hAsyncEvent );
        if ( psInst->fPortOpen )
            CloseHandle( psInst->hPipe );

        LeaveCriticalSection( &psInst->sSerialize );
        DeleteCriticalSection( &psInst->sSerialize );

        HeapFree( GetProcessHeap(), 0, psInst );
    }
}




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
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    QFL2_PACKET    * psPacket = NULL;

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
        IN QFL2_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




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
        IN QFL2_INSTANCE * psInst )
{
    return NULL;
}




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
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    QFL2_PACKET    * psPacket;

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
 * SYNTAX    : EXPORT void WINAPI QFL2FreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par QFL2ReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à QFL2ReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI QFL2FreeMessage(
        IN BYTE * pbMsg )
{
    QFL2_PACKET * psPacket = NULL;

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
        IN void         * pvValue )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwParam;
    DWORD dwReq;
    DWORD dwRsp;
    char  tbReq[QFL2_OUT_BUFFER_BYTES+1];
    char  tbRsp[QFL2_IN_BUFFER_BYTES+1];

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( pvValue != NULL )
        {
            if      ( dwValue == 1 )
                dwParam = (DWORD)(*(BYTE*)pvValue);
            else if ( dwValue == 2 )
                dwParam = (DWORD)(*(WORD*)pvValue);
            else if ( dwValue == 4 )
                dwParam =        (*(WORD*)pvValue);
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }
        }
        else
            dwParam = dwValue;

        if ( dwOptionId == QFL2_OPTION_DW_BEACONID )
            sprintf( tbReq, "C_BEACID %u", dwParam );

        else if ( dwOptionId == QFL2_OPTION_DW_KEEPALIVE )
            sprintf( tbReq, "C_KEEPALIVE %u", dwParam );

        else if ( dwOptionId == QFL2_OPTION_DW_N11 )
            sprintf( tbReq, "C_N11 %u", dwParam );

        else if ( dwOptionId == QFL2_OPTION_DW_RETRYINTOVERHEAD )
            sprintf( tbReq, "C_RetryIntOverhead %u", dwParam );

        else if ( dwOptionId == QFL2_OPTION_F_DISABLEBEFOREBST )
            sprintf( tbReq, "C_DisableBeforeBst %u", dwParam );

        else if ( dwOptionId == QFL2_OPTION_DW_INSERT_BEACONID )
            sprintf( tbReq, "C_InsertBeaconId %u", dwParam );

        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        dwReq = (DWORD)strlen(tbReq);
        tbReq[1] = (BYTE)dwReq;
        dwRsp = sizeof(tbRsp);
        dwErr = QFL2WriteRead( psInst, tbReq, dwReq, tbRsp, &dwRsp );
        if ( dwErr != NO_ERROR )
            __leave;

        tbRsp[dwRsp] = 0;
        if ( strcmp( tbRsp, "C\008NOT OK" ) == 0 )
            dwErr = ERROR_INVALID_PARAMETER;
        else if ( strcmp( tbRsp, "C\4OK" ) != 0 )
            dwErr = ERROR_INVALID_DATA;
        else
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
        IN void         * pvValue )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}




PRIVATE void WINAPI QFL2MakeError( QFL2_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    QFL2TerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
    strcpy( psInst->szLastError, pcText );
}




PRIVATE void WINAPI QFL2HandleIRP( QFL2_INSTANCE * psInst )
{
    DWORD        dwErr;
    QFL2_PACKET * psPacket;

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( psInst->psCurrentPacket == NULL )
        {
            //
            // Si le message n'a pas encore été construit, on l'extrait de la file d'attente
            //
            dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
            if ( dwErr != NO_ERROR )
            {
                QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : QueueReadItem" );
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
                dwErr = QueueReadItem( psInst->psReadQueue, &psPacket, 0 );
                if ( dwErr == NO_ERROR )
                    HeapFree( GetProcessHeap(), 0, psPacket );
            }
        }

        dwErr = QFL2VerifyOpenPort( psInst );
        if ( dwErr != NO_ERROR )
        {
            QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : QFL2VerifyOpenPort" );
            __leave;
        }
        if ( ! psInst->fPortOpen )
        {
            QFL2MakeError( psInst, ERROR_DEVICE_NOT_CONNECTED, "QFL2HandleIRP : QFL2VerifyOpenPort" );
            __leave;
        }

        // 
        // Clear the pipe in order to resync if sync lost.
        //
        dwErr = QFL2ClearPipe( psInst );
        if ( dwErr != NO_ERROR )
        {
            QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : QFL2ClearPipe" );
            __leave;
        }

        if ( psInst->psCurrentPacket->tbBuffer[0] == 'I' )
        {
            dwErr = QFL2DirectStop( psInst );
            if ( dwErr != NO_ERROR )
            {
                QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : QFL2DirectStop" );
                __leave;
            }
        }

        //
        // Envoyer le message sur le port
        //
        psInst->dwInBytes = sizeof( psInst->tbIn );
        dwErr = QFL2WriteRead( psInst,
                               psInst->psCurrentPacket->tbBuffer,
                               psInst->psCurrentPacket->dwBufferBytes,
                               psInst->tbIn,
                               &psInst->dwInBytes );
        if ( ( dwErr == NO_ERROR ) && ( psInst->dwInBytes != 0 ) )
        {
            // Le message a été émis et une réponse a été reçue
            //
            QFL2TerminateCurrentPacket( psInst, NO_ERROR );

            //
            // Préparer le packet de lecture
            //
            psInst->dwInBytes = sizeof( psInst->tbIn );
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
                    QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : QueueWriteItem" );
                }
            }
        }
        else if ( ( dwErr == NO_ERROR ) && ( psInst->dwInBytes == 0 ) )
        {
            //
            // Le message a été émis mais aucune réponse n'a été renvoyée
            //
            QFL2_CALL_HOOK( QFL2_HOOK_OUTPUT | QFL2_HOOK_DATA | QFL2_HOOK_FLUSH,
                psInst->psCurrentPacket->dwBufferBytes, 
                psInst->psCurrentPacket->tbBuffer );
            QFL2TerminateCurrentPacket( psInst, NO_ERROR );
        }
        else
        {
            CloseHandle( psInst->hPipe );
            psInst->fPortOpen = FALSE;

            //
            // L'envoi a échoué, on provoque une erreur, on annule l'IRP en cours
            //
            QFL2MakeError( psInst, dwErr, "QFL2HandleIRP : WriteReadData" );
        }
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }
}







PRIVATE DWORD WINAPI QFL2RunThread( 
        IN QFL2_INSTANCE * psInst )
{
    DWORD       dwErr;
    DWORD       dwRes;
    DWORD       dwIndex;

    SetThreadPriorityBoost( GetCurrentThread(), TRUE );
    SetThreadPriority( GetCurrentThread(), psInst->sParams.dwPriority );

    //
    // Animer l'automate
    //
    while ( TRUE )
    {
        dwRes =  WaitForMultipleObjects( 2, psInst->thEvents, FALSE, INFINITE );
        dwIndex = dwRes - WAIT_OBJECT_0;

        if ( dwIndex == QFL2_END_INDEX )
        {
            //
            // Signal de fin reçu, on sort de la boucle
            //
            break;
        }

        else if ( dwIndex == QFL2_IRP_INDEX )
        {
            QFL2HandleIRP( psInst );
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
        QFL2TerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

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




PRIVATE void WINAPI QFL2TerminateCurrentPacket( QFL2_INSTANCE * psInst, DWORD dwErr )
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



PRIVATE void WINAPI QFL2Track( QFL2_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes )
{
    BOOL    fFlush;
    DWORD   dwRemain;

    fFlush = ( ( dwContext & QFL2_HOOK_FLUSH ) != 0 );
    dwContext = ( dwContext & ( ~ QFL2_HOOK_FLUSH ) );

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



PRIVATE DWORD WINAPI QFL2VerifyOpenPort( QFL2_INSTANCE * psInst )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwPipeMode;

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( ! psInst->fPortOpen )
        {
        	psInst->hPipe = CreateFile( psInst->sParams.szL7DataPipeName ,
		                                GENERIC_READ | GENERIC_WRITE,
		                                0,
		                                NULL, 
		                                OPEN_EXISTING,
		                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		                                0);
	
	        if ( psInst->hPipe == INVALID_HANDLE_VALUE )
	        {
		        dwErr = GetLastError();
		        __leave;
            }

            //
            // Ensure that message mode is activated
            //
            dwPipeMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
            SetNamedPipeHandleState( psInst->hPipe, &dwPipeMode, NULL, NULL );

            psInst->fPortOpen = TRUE;

            dwErr = QFL2DirectPortParams( psInst );
            if ( dwErr != NO_ERROR )
            {
                CloseHandle( psInst->hPipe );
                psInst->fPortOpen = FALSE;
                __leave;
            }
        }
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }

    return dwErr;
}










PRIVATE DWORD WINAPI QFL2WriteRead( QFL2_INSTANCE * psInst,
                                    void          * pvOut,
                                    DWORD           dwOut,
                                    void          * pvIn,
                                    DWORD         * pdwIn )
{
    DWORD dwErr = NO_ERROR;
    BOOL  fResult;
    DWORD dwWritten;

    __try
    {
        if ( ! psInst->fPortOpen )
        {
            dwErr = ERROR_PIPE_NOT_CONNECTED;
            __leave;
        }

        QFL2_CALL_HOOK( QFL2_HOOK_OUTPUT | QFL2_HOOK_DATA | QFL2_HOOK_FLUSH,
            dwOut, 
            pvOut );

        //
        // WRITE
        //

        //
        // Prepare the overlapped structure in the instance
        //
        memset( &psInst->sAsync, 0, sizeof(psInst->sAsync) );
        psInst->sAsync.hEvent = psInst->hAsyncEvent;

        //
        // Ensure the event is not signaled
        //
        ResetEvent( psInst->hAsyncEvent );

        //
        // Initiate Question
        //
        fResult = WriteFile( psInst->hPipe,
                             pvOut,
                             dwOut,
                             &dwWritten,
                             &psInst->sAsync );
        if ( ! fResult )
        {
            //
            // An error occured, we must verify that this error is not related to
            // the pending read operation
            //
            dwErr = GetLastError();

            if ( dwErr != ERROR_IO_PENDING )
            {
                //
                // This is a real error, exit immediatly
                //
                CancelIo( psInst->hPipe );
                __leave;
            }

            //
            // Wait until the pending operation completes or until the
            // timeout delay is ellapsed.
            //
            if ( WaitForSingleObject( psInst->hAsyncEvent, QFL2_PIPE_TIMEOUT ) != WAIT_OBJECT_0 )
            {
                //
                // The timeout delay is ellapsed, we return the corresponding
                // error code. The pending write operation must be canceled.
                //
                CancelIo( psInst->hPipe );
                dwErr = WAIT_TIMEOUT;
                __leave;
            }

            //
            // The pending i/o is now finished, so we just have get the
            // result of the overlapped write
            //
            if ( ! GetOverlappedResult( psInst->hPipe, &psInst->sAsync, &dwWritten, FALSE ) )
            {
                dwErr = GetLastError();

                //
                // For security, make sure no there is no remaining pending i/o
                //
                CancelIo( psInst->hPipe );
                __leave;
            }
        }

        if ( dwWritten != dwOut )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            __leave;
        }

        //
        // READ
        //

        //
        // Prepare the overlapped structure in the instance
        //
        memset( &psInst->sAsync, 0, sizeof(psInst->sAsync) );
        psInst->sAsync.hEvent = psInst->hAsyncEvent;

        //
        // Ensure the event is not signaled
        //
        ResetEvent( psInst->hAsyncEvent );

        //
        // Initiate response
        //
        fResult = ReadFile( psInst->hPipe,
                            pvIn,
                            *pdwIn,
                            pdwIn,
                            &psInst->sAsync );
        if ( fResult )
            __leave;

        //
        // An error occured, we must verify that this error is not related to
        // the pending read operation
        //
        dwErr = GetLastError();

        if ( dwErr != ERROR_IO_PENDING )
        {
            //
            // This is a real error, exit immediatly
            //
             CancelIo( psInst->hPipe );
            __leave;
        }

        //
        // At this point, a write operation is pending
        //

        //
        // Wait until the pending operation completes or until the
        // timeout delay is ellapsed.
        //
        if ( WaitForSingleObject( psInst->hAsyncEvent, QFL2_PIPE_TIMEOUT ) != WAIT_OBJECT_0 )
        {
            //
            // The timeout delay is ellapsed, we return the corresponding
            // error code. The pending write operation must be canceled.
            //
            CancelIo( psInst->hPipe );
            dwErr = WAIT_TIMEOUT;
            __leave;
        }

        //
        // The pending i/o is now finished, so we just have get the
        // result of the overlapped write
        //
        if ( ! GetOverlappedResult( psInst->hPipe, &psInst->sAsync, pdwIn, FALSE ) )
        {
            dwErr = GetLastError();

            //
            // For security, make sure no there is no remaining pending i/o
            //
            CancelIo( psInst->hPipe );
            __leave;
        }

        //
        // Data written successfully
        //
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( ( dwErr == NO_ERROR ) && ( *pdwIn > 0 ) )
        {
            QFL2_CALL_HOOK( QFL2_HOOK_INPUT | QFL2_HOOK_DATA | QFL2_HOOK_FLUSH,
                            *pdwIn, 
                            pvIn );
        }

    }

    return dwErr;
}


PRIVATE DWORD WINAPI QFL2ClearPipe( QFL2_INSTANCE * psInst )
{
    DWORD dwRead;
    DWORD dwErr = NO_ERROR;
    BYTE  tbIn[QFL2_IN_BUFFER_BYTES];

    while ( dwErr == NO_ERROR )
    {
        if ( ! psInst->fPortOpen )
        {
            dwErr = ERROR_PIPE_NOT_CONNECTED;
            break;
        }

        //
        // Prepare the overlapped structure in the instance
        //
        memset( &psInst->sAsync, 0, sizeof(psInst->sAsync) );
        psInst->sAsync.hEvent = psInst->hAsyncEvent;

        //
        // Ensure the event is not signaled
        //
        ResetEvent( psInst->hAsyncEvent );

        //
        // Initiate reading.
        //
        if ( ReadFile( psInst->hPipe, tbIn, sizeof(tbIn), &dwRead, &psInst->sAsync ) )
        {
            QFL2_CALL_HOOK( QFL2_HOOK_INPUT | QFL2_HOOK_PROTOCOL | QFL2_HOOK_FLUSH,
                            dwRead, 
                            tbIn );

            //
            // There was already something in the input buffer so we just have
            // to return
            //
            dwErr = NO_ERROR;
            continue;
        }

        //
        // An error occured, we must verify that this error is not related to
        // the pending read operation
        //
        dwErr = GetLastError();
        if ( dwErr != ERROR_IO_PENDING )
        {
            //
            // This is a real error, exit immediatly
            //
            break;
        }

        //
        // Wait until the pending read operation completes or until the
        // timeout delay is ellapsed. The timeout is minimum because this is
        // for clearing.
        //
        if ( WaitForSingleObject( psInst->hAsyncEvent, 1 ) != WAIT_OBJECT_0 )
        {
            //
            // The timeout delay is ellapsed, we return the corresponding
            // error code. The pending read operation canceled.
            //
            dwErr = NO_ERROR;  // In this case, a timeout is not an error
            CancelIo( psInst->hPipe );
            break;
        }

        //
        // The pending i/o is now finished, so we just have get the
        // result of the overlapped read
        //
        if ( ! GetOverlappedResult( psInst->hPipe, &psInst->sAsync, &dwRead, FALSE ) )
        {
            dwErr = GetLastError();
            //
            // For security, make sure no there is no remaining pending i/o
            //
            CancelIo( psInst->hPipe );
            break;
        }

        QFL2_CALL_HOOK( QFL2_HOOK_INPUT | QFL2_HOOK_PROTOCOL | QFL2_HOOK_FLUSH,
                        dwRead, 
                        tbIn );

        dwErr = NO_ERROR;
    }

    return dwErr;
}



PRIVATE WINAPI WINAPI QFL2DirectStop( IN QFL2_INSTANCE * psInst )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwReq;
    DWORD dwRsp;
    char  tbReq[QFL2_OUT_BUFFER_BYTES+1];
    char  tbRsp[QFL2_IN_BUFFER_BYTES+1];

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        strcpy( tbReq, "S_" );

		dwReq = (DWORD)strlen(tbReq);
        tbReq[1] = (BYTE)dwReq;
        dwRsp = sizeof(tbRsp);
        dwErr = QFL2WriteRead( psInst, tbReq, dwReq, tbRsp, &dwRsp );
        if ( dwErr != NO_ERROR )
            __leave;

        tbRsp[dwRsp] = 0;
        if ( strcmp( tbRsp, "S\008NOT OK" ) == 0 )
            dwErr = ERROR_INVALID_PARAMETER;
        else if ( strcmp( tbRsp, "S\4OK" ) != 0 )
            dwErr = ERROR_INVALID_DATA;
        else
            dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }

    return dwErr;
}


PRIVATE WINAPI WINAPI QFL2DirectPortParams( IN QFL2_INSTANCE * psInst )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwReq;
    DWORD dwRsp;
    char  tbReq[QFL2_OUT_BUFFER_BYTES+1];
    char  tbRsp[QFL2_IN_BUFFER_BYTES+1];

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        _snprintf( tbReq, sizeof(tbReq), "C_SERIAL %s", psInst->sParams.szPortSettings );

        dwReq = strlen(tbReq);
        tbReq[1] = (BYTE)dwReq;
        dwRsp = sizeof(tbRsp);
        dwErr = QFL2WriteRead( psInst, tbReq, dwReq, tbRsp, &dwRsp );
        if ( dwErr != NO_ERROR )
            __leave;

        tbRsp[dwRsp] = 0;
        if ( strcmp( tbRsp, "C\008NOT OK" ) == 0 )
            dwErr = ERROR_INVALID_PARAMETER;
        else if ( strcmp( tbRsp, "C\4OK" ) != 0 )
            dwErr = ERROR_INVALID_DATA;
        else
            dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }

    return dwErr;
}

