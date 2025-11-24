/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_worker.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des threads worker
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
#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#include <acom_timer.h>
#include <acom_queue.h>
#include <acom_block.h>
#include <acom_io.h>
#include <acom_mail.h>
#include <acom_pipe.h>
#include <acom_tcp.h>
#include <acom_serial.h>
#include <acom_stats.h>
#include <acom_clbk.h>
#define LOC_DEF
#include <acom_worker.h>
#undef LOC_DEF

#include <memclass.h>


// PROTOTYPES PRIVES
PRIVATE DWORD AComWorkerQueueMsg( ACOM_CONNECTION * psCnx, ACOM_BLOCK * psBlock );
PRIVATE DWORD AComWorkerConnect( ACOM_CONNECTION * psCnx );
PRIVATE DWORD AComWorkerIoCompletion(ACOM_OVERLAPPED * psOver, DWORD dwCompletionBytes, ULONG_PTR pulCompletionKey, DWORD dwCompletionError);
PRIVATE DWORD AComWorkerCommand( ACOM_BLOCK * psBlock );
PRIVATE DWORD AComWorkerRefreshConnections( ACOM_WORKING_SET * psWks, DWORD * pdwNextDelay );

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD AComWorkerQueueMsg ( ACOM_CONNECTION * psCnx, ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 *             ACOM_BLOCK * psBlock    : Bloc de commande d'émission à mettre en file.
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Place un bloc d'émission en file d'attente d'émission et tente de lancer
 *             une écriture (n'a pas lieu si une écriture est déjà en cours).
 * --------------------------------------------------------------------
 */
PRIVATE DWORD AComWorkerQueueMsg( ACOM_CONNECTION * psCnx, ACOM_BLOCK * psBlock )
{
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerQueueMsg(0x%016X,0x%016X)", psCnx, psBlock );

    if ( ! AComQueueAddItem( psCnx->psWriteQueue, psBlock ) )
    {
        AComDbgError( 0, __FILE__, __LINE__, "Saturation FIFO interne" );
        dwErr = AComWorkerDisconnectOnError( psCnx, ERROR_NOT_ENOUGH_MEMORY );
    }
    else
        dwErr = AComWorkerSendNext( psCnx );

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerQueueMsg return %d", dwErr );
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD AComWorkerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Oriente la gestion de connexion vers la bonne routine
 *             de connexion (client/server/pipe/mailslot...).
 * --------------------------------------------------------------------
 */
PRIVATE DWORD AComWorkerConnect( ACOM_CONNECTION * psCnx )
{
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerConnect(0x%016X)", psCnx );

    psCnx->dwLastConnectionTry = GetTickCount();

    switch ( psCnx->psInst->dwType )
    {

    case ACOM_INSTANCE_PIPE_SERVER :

        // ------------------------------------------
        // SERVEUR DE PIPE
        // ------------------------------------------
        // Le server de pipe est le seul cas qui passe par l'état
        // intermédiaire ACOM_CONNECTION_STATE_CONNECTING. C'est le
        // seul qui passe par l'i/o completion de connexion.
        // Le pipe est créé, il est associé au completion port,
        // puis mis en attente sur une connexion de client
        dwErr = AComPipeServerConnect( psCnx );
        break;

    case ACOM_INSTANCE_PIPE_CLIENT :

        // ------------------------------------------
        // CLIENT DE PIPE
        // ------------------------------------------
        // Le pipe est créé, si la création réussi, il est connecté
        // au serveur. Immédiatement après, il est passé en mode
        // message. Il est ensuite associé au completion port.
        // Une fois fait, on lance la première lecture.
        dwErr = AComPipeClientConnect( psCnx );
        break;

    case ACOM_INSTANCE_MAIL_SERVER :

        // ------------------------------------------
        // SERVEUR DE MAILSLOT
        // ------------------------------------------
        // Le mailslot est créé, si la création réussi, il est "connecté".
        // Il est ensuite associé au completion port.
        // Une fois fait, on lance la première lecture.
        dwErr = AComMailServerConnect( psCnx );
        break;

    case ACOM_INSTANCE_MAIL_CLIENT :

        // ------------------------------------------
        // CLIENT DE MAILSLOT
        // ------------------------------------------
        // Le client de mailslot est créé, si la création réussi, il est "connecté".
        // Il est ensuite associé au completion port.
        // Pas de premiere lecture à faire, le client ne fonctionne qu'en émission.
        dwErr = AComMailClientConnect( psCnx );
        break;

    case ACOM_INSTANCE_TCP_SERVER :

        // ------------------------------------------
        // SERVEUR TCP
        // ------------------------------------------
        // A la création de l'instance, un socket a été mis en écoute.
        // On ne fait donc que tester si au moins un client est en
        // attente d'acceptation de connexion. Celui-ci donne lieu
        // à la création d'un socket qui est est ensuite associé au
        // completion port.
        // Une fois fait, on lance la première lecture.
        dwErr = AComTcpServerConnect( psCnx );
        break;

    case ACOM_INSTANCE_TCP_CLIENT :

        // ------------------------------------------
        // CLIENT TCP
        // ------------------------------------------
        // Le lien TCP est connecté au serveur.
        // Il est ensuite associé au completion port.
        // Une fois fait, on lance la première lecture.
        dwErr = AComTcpClientConnect( psCnx );
        break;

    case ACOM_INSTANCE_SERIAL :

        // ------------------------------------------
        // LIAISON SERIE
        // ------------------------------------------
        // Le port série a été ouvert.
        // Il est ensuite associé au completion port.
        // Une fois fait, on lance la première lecture.
        dwErr = AComSerialConnect( psCnx );
        break;

    default :

        // Type non connu

        dwErr = ERROR_INVALID_DATA;
        psCnx->dwState = ACOM_CONNECTION_STATE_SHUTDOWN;
        psCnx->dwError = dwErr;

        AComDbgError( dwErr, __FILE__, __LINE__, "Type de communication inconnu : %d", psCnx->psInst->dwType );
        AComClbkShutdown( psCnx->psWks, dwErr );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerConnect return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD AComWorkerIoCompletion ( 
 *                      ACOM_OVERLAPPED * psOver,
 *                      DWORD dwCompletionBytes,
 *                      ULONG_PTR pulCompletionKey,
 *                      DWORD dwCompletionError )
 * PARAMETERS: ACOM_OVERLAPPED * psOver : Pointeur overlapped de l'i/o completion casté dans
 *                                        la sur-structure overlapped de la librairie.
 *             DWORD dwCompletionBytes  : Octets concernés par l'i/o completion.
 *             ULONG_PTR pulCompletionKey    : Clé concernée par l'i/o completion.
 *             DWORD dwCompletionError  : Code d'erreur concerné par l'i/o completion.
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un i/o completion packet recu dans un i/o completion port
 * --------------------------------------------------------------------
 */

PRIVATE DWORD AComWorkerIoCompletion(ACOM_OVERLAPPED * psOver, DWORD dwCompletionBytes, ULONG_PTR pulCompletionKey, DWORD dwCompletionError)
{
    DWORD dwErr = NO_ERROR;
    ACOM_CONNECTION   * psCnx   = psOver->psCnx;

	AComDbgInfo(__FILE__, __LINE__, "AComWorkerIoCompletion(0x%016X)", pulCompletionKey);

    EnterCriticalSection( &psCnx->sCnxCritical );

    LeaveCriticalSection( &psCnx->psWks->sIoCritical );

    // Verifier que la connexion n'a pas changée ou qu'elle n'est pas en erreur
	if ((psCnx->hCnxHandle == pulCompletionKey) &&
         ( psCnx->dwState     != ACOM_CONNECTION_STATE_FREE ) )
    {
		// -------------------------------------------
        // S'IL S'AGIT D'UNE ERREUR
        // -------------------------------------------
        if ( dwCompletionError != NO_ERROR )
        {
            AComDbgError( dwCompletionError, __FILE__, __LINE__, "Erreur d'io completion" );

            // SI ON ÉTAIT CONNECTÉ
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
            {
                dwErr = AComWorkerDisconnectOnError( psCnx, dwCompletionError );
            }

            // SI ON ÉTAIT EN TRAIN DE SE CONNECTER
            else if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTING )
            {
                ACOM_CLOSE_HANDLE( psCnx );
                psCnx->dwState = ACOM_CONNECTION_STATE_DISCONNECTED;
                psCnx->szIdentity[0] = 0;
            }

            // SI ON ÉTAIT EN ERREUR OU DECONNECTE
            else if ( ( psCnx->dwState == ACOM_CONNECTION_STATE_ERROR ) || 
                      ( psCnx->dwState == ACOM_CONNECTION_STATE_DISCONNECTED ) )
            {
                // On ignore
                AComDbgInfo( __FILE__, __LINE__, "L'erreur a eu lieu par dessus une autre" );
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
                AComDbgError( 0, __FILE__, __LINE__, "Etat incoherent : %08X", psCnx->dwState );
            }

        }

        // -------------------------------------------
        // S'IL S'AGIT D'UNE RÉCEPTION RÉUSSIE
        // -------------------------------------------
        else if ( psOver->dwType == ACOM_IO_TYPE_INPUT )
        {
            // SI ON ÉTAIT CONNECTÉ
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
            {
                ACOM_STATS_RECEIVED( psCnx, dwCompletionBytes );
                dwErr = AComClbkReceived( psCnx, dwCompletionBytes + psCnx->dwReadPos );
                if ( dwErr == NO_ERROR )
                    dwErr = AComWorkerReceiveNext( psCnx );
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
                AComDbgError( 0, __FILE__, __LINE__, "Etat incoherent : %08X", psCnx->dwState );
            }
        }

        // -------------------------------------------
        // S'IL S'AGIT D'UNE ÉMISSION RÉUSSIE
        // -------------------------------------------
        else if ( psOver->dwType == ACOM_IO_TYPE_OUTPUT )
        {
            // SI ON ÉTAIT CONNECTÉ
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
            {
                ACOM_STATS_SENT( psCnx, dwCompletionBytes );

                dwErr = AComClbkSent( psCnx, NO_ERROR );
                DMEM_FREE( psCnx->psWriteCurrent );
                if ( dwErr == NO_ERROR )
                    dwErr = AComWorkerSendNext( psCnx );
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
                AComDbgError( 0, __FILE__, __LINE__, "Etat incoherent : %08X", psCnx->dwState );
            }
        }

        // -------------------------------------------
        // S'IL S'AGIT D'UNE CONNEXION REUSSIE
        // -------------------------------------------
        else if ( psOver->dwType == ACOM_IO_TYPE_CONNECTION )
        {
            // SI ON ÉTAIT EN COURS DE CONNEXION
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTING )
            {
                psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
                AComPipeGetIdentity( psCnx );
                AComStatsReset( psCnx );
                dwErr = AComClbkConnection( psCnx );
                if ( ( dwErr == NO_ERROR ) && ( psCnx->psInst->bHasInput ) )
                    dwErr = AComWorkerReceiveNext( psCnx );
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
                AComDbgError( 0, __FILE__, __LINE__, "Etat incoherent : %08X", psCnx->dwState );
            }

        }

        // RETOUR D'IO COMPLETION ERRONNE
        else
        {
            psCnx->dwState = ACOM_CONNECTION_STATE_SHUTDOWN;
            psCnx->dwError = ERROR_INVALID_DATA;
            AComDbgError( psCnx->dwError, __FILE__, __LINE__, "I/O completion inattendue" );
            AComClbkShutdown( psCnx->psWks, psCnx->dwError );
        }
    }
    else
    {
        // Ignorer
        AComDbgInfo( __FILE__, __LINE__, "I/O completion sur connexion fermée" );
    }

    LeaveCriticalSection( &psCnx->sCnxCritical );

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerIoCompletion return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD AComWorkerCommand ( ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_BLOCK * psBlock : Structure bloc de commande
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un bloc de commande recu dans un i/o completion port
 * --------------------------------------------------------------------
 */
PRIVATE DWORD AComWorkerCommand( ACOM_BLOCK * psBlock )
{
    BOOL bFree = TRUE;
    DWORD dwErr = NO_ERROR;
    ACOM_CONNECTION   * psCnx   = psBlock->psCnx;
    ACOM_LIFE_HEADER sHeader;

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerCommand(0x%016X)", psBlock );

    EnterCriticalSection( &psCnx->sCnxCritical );

    LeaveCriticalSection( &psCnx->psWks->sIoCritical );

    // Verifier que la connexion n'a pas changée ou qu'elle n'est pas en erreur
    if ( ( psCnx->hCnxHandle == psBlock->hCnxHandle       ) &&
         ( psCnx->dwState    != ACOM_CONNECTION_STATE_FREE ) )
    {

		AComDbgInfo(__FILE__, __LINE__, "AComWorkerCommand(0x%016X)-block(0x%016X) -Check connection- dwBlockType:%08X-%s connectionState:%08X-%s", psCnx,
			psBlock, psBlock->dwBlockType, GetNameForBlockType(psBlock->dwBlockType), psCnx->dwState, GetNameForConnectionState(psCnx->dwState));
		// -------------------------------------------
        // DEMANDE DE DECONNEXION
        // -------------------------------------------
        if ( psBlock->dwBlockType == ACOM_COMMAND_DISCONNECT )
        {
            if ( psBlock->bDisable )
                psCnx->bEnabled = FALSE;

            // SI ON EST CONNECTÉ OU EN ERREUR
            if ( ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED ) ||
                 ( psCnx->dwState == ACOM_CONNECTION_STATE_ERROR ) )
            {
				if ( psCnx->psWriteCurrent != NULL )
                {
                    // Pas de stat à mettre à jours
                    dwErr = AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                    DMEM_FREE( psCnx->psWriteCurrent );
                }
                while ( ( psCnx->psWriteCurrent = AComQueueRemoveItem( psCnx->psWriteQueue ) ) != NULL )
                {
                    // Pas de stat à mettre à jours
                    if ( dwErr == NO_ERROR )
                        dwErr = AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                    DMEM_FREE( psCnx->psWriteCurrent );
                }

                if ( dwErr == NO_ERROR )
                    dwErr = AComClbkDisconnection( psCnx );
				AComTimerRemoveTimer(psCnx->dwTimerId);
                CancelIo( psCnx->hFile );
                ACOM_CLOSE_HANDLE( psCnx );
                psCnx->dwState = ACOM_CONNECTION_STATE_DISCONNECTED;
                psCnx->szIdentity[0] = 0;
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
				AComDbgError(0, __FILE__, __LINE__, "AComWorkerCommand(0x%016X)-block(0x%016X) Etat incoherent connectionState:%08X-%s ", psCnx,
					psBlock, psCnx->dwState, GetNameForConnectionState(psCnx->dwState));
            }

        }

        // -------------------------------------------
        // DEMANDE D'ENVOI
        // -------------------------------------------
        else if ( psBlock->dwBlockType == ACOM_COMMAND_SEND )
        {
            // SI ON EST CONNECTÉ
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
            {
                dwErr = AComWorkerQueueMsg( psCnx, psBlock );

                // Indiquer qu'il ne faut pas désallouer
                bFree = FALSE;
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
				AComDbgError(0, __FILE__, __LINE__, "AComWorkerCommand(0x%016X)-block(0x%016X) Etat incoherent connectionState:%08X-%s ", psCnx,
					psBlock, psCnx->dwState, GetNameForConnectionState(psCnx->dwState));
			}

        }

        // -------------------------------------------
        // DEMANDE DE TEST DE VIE
        // -------------------------------------------
        else if ( ( psBlock->dwBlockType                        == ACOM_COMMAND_LIFE  ) && 
                  ( ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE ) )
        {
            // SI ON EST CONNECTÉ
            if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
            {

				psCnx->dwLifeCount++;
                if ( psCnx->dwLifeCount > psCnx->psWks->dwMaxLife )
                {
					AComDbgError(0, __FILE__, __LINE__, "AComWorkerCommand(0x%016X), ACOM_COMMAND_LIFE dwLifeCount[%d]>dwMaxLife[%d] ...", psCnx, psCnx->dwLifeCount, psCnx->psWks->dwMaxLife);

					if ( psCnx->psWriteCurrent != NULL )
                    {
                        // Pas de stat à mettre à jours
                        dwErr = AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                        DMEM_FREE( psCnx->psWriteCurrent );
                    }
                    while ( ( psCnx->psWriteCurrent = AComQueueRemoveItem( psCnx->psWriteQueue ) ) != NULL )
                    {
                        // Pas de stat à mettre à jours
                        if ( dwErr == NO_ERROR )
                            dwErr = AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                        DMEM_FREE( psCnx->psWriteCurrent );
                    }

					if (dwErr == NO_ERROR)
                        dwErr = AComClbkDisconnection( psCnx );

					AComDbgInfo(__FILE__, __LINE__, "AComWorkerCommand(0x%016X), ACOM_CLOSE_HANDLE...", psBlock);

					CancelIo( psCnx->hFile );
                    ACOM_CLOSE_HANDLE( psCnx );
                    psCnx->dwState = ACOM_CONNECTION_STATE_DISCONNECTED;
                }
                else
                {
					if (psCnx->dwLifeCount>1)
						AComDbgError(0, __FILE__, __LINE__, "AComWorkerCommand(0x%016X), ACOM_COMMAND_LIFE dwLifeCount = %u", psCnx, psCnx->dwLifeCount);

					sHeader.dwType = ACOM_LIFE_REQ;
                    sHeader.dwDataSize = 0;

                    // ENVOYER DEMANDE DE VIE
                    dwErr = AComBlockPost(
                        psCnx->psWks->hCompletion,
                        ACOM_COMMAND_SEND,
                        psCnx,
                        psCnx->hCnxHandle,
                        0,
                        0,
                        FALSE,
                        sizeof( sHeader ),
                        &sHeader,
                        0,
                        NULL );

					AComTimerRemoveTimer(psCnx->dwTimerId);
                    psCnx->dwTimerId = AComTimerAddTimer(
                            psCnx->psWks->dwLifeTime,
                            psCnx->psWks->hCompletion,
                            psCnx,
                            psCnx->hCnxHandle,
                            ACOM_COMMAND_LIFE );
                }
            }

            else
            {
                // SI ÉTAT INCOHÉRENT
                // On ignore
				AComDbgError(0, __FILE__, __LINE__, "AComWorkerCommand(0x%016X)-block(0x%016X) Etat incoherent connectionState:%08X-%s ", psCnx,
					psBlock, psCnx->dwState, GetNameForConnectionState(psCnx->dwState));
			}
            
        }

        // -------------------------------------------
        // COMMANDE INCONNUE
        // -------------------------------------------
        else
        {
            psCnx->dwState = ACOM_CONNECTION_STATE_SHUTDOWN;
            psCnx->dwError = ERROR_INVALID_DATA;
            AComDbgError( psCnx->dwError, __FILE__, __LINE__, "Commande inconnue" );
            AComClbkShutdown( psCnx->psWks, psCnx->dwError );
        }
    }
    else
    {
        // Ignorer le packet
        AComDbgError( 0, __FILE__, __LINE__, "Commande sur connexion fermée" );
    }

    LeaveCriticalSection( &psCnx->sCnxCritical );

    if ( bFree )
        DMEM_FREE( psBlock );

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerCommand return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD AComWorkerRefreshConnections ( ACOM_WORKING_SET * psWks, DWORD * pdwNextDelay )
 * PARAMETERS: ACOM_WORKING_SET * psWks : Structure de working set
 *             DWORD * pdwNextDelay     : Récupère le prochaine délai d'attente dans la boucle de polling
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Rafraichissement de l'état des connexions si
 *             leur délai de reconnexion est expiré.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD AComWorkerRefreshConnections( ACOM_WORKING_SET * psWks, DWORD * pdwNextDelay )
{
    DWORD dwIndex;
    DWORD dwErr = NO_ERROR;
    ACOM_CONNECTION * psCnx;
    DWORD dwDelay;
    DWORD dwTick = GetTickCount();
    BOOL  fConnect = FALSE;
    BOOL  fTry;
    SOCKET hLastListener = (SOCKET)NULL;

    //AComDbgInfo( __FILE__, __LINE__, "AComWorkerRefreshConnections(0x%016X)", psWks );

    for ( dwIndex = 0 ; ( dwIndex < psWks->dwMaxConnections ) && ( dwErr == NO_ERROR ) ; dwIndex++ )
    {
        psCnx = &psWks->psCnx[dwIndex];
        EnterCriticalSection( &psCnx->sCnxCritical );
        if ( ( psCnx->dwState == ACOM_CONNECTION_STATE_DISCONNECTED ) && psCnx->bEnabled )
        {
            //
            // Dans le cas des coms tcp serveur, on ne teste que pour la première connexion
            // utilisant un socket listener donné.
            //
            if ( psCnx->psInst->dwType == ACOM_INSTANCE_TCP_SERVER ) 
            {
                fTry = ( hLastListener != psCnx->psInst->hListener );
                hLastListener = psCnx->psInst->hListener;
            }
            else
                fTry = TRUE;

            if ( fTry )
            {
                dwDelay = dwTick - psCnx->dwLastConnectionTry;
                if ( dwDelay >= psCnx->psInst->dwTimeToReconnect )
                {
                    dwErr = AComWorkerConnect( psCnx );
                    if ( ( dwErr == NO_ERROR ) &
                         ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED ) )
                    {
                        fConnect = TRUE;
                    }
                }    
            }
        }
        
        LeaveCriticalSection( &psCnx->sCnxCritical );
        Sleep(0); // Laisser les autres worker travailler
    }

    // On ajuste le délai de rebouclage du thread gérant les connexion pour avoir
    // un deuxième cycle immédiatement aprés une connexion réussie (permet de prendre
    // immédiatement en compte les connexions en file d'attente en TCP).
    if ( pdwNextDelay != NULL )
        (*pdwNextDelay) = ( fConnect ? 0 : psWks->dwConnectLoopDelay );
        
    //AComDbgInfo( __FILE__, __LINE__, "AComWorkerRefreshConnections return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerThread ( ACOM_WORKING_SET * psWks )
 * PARAMETERS: ACOM_WORKING_SET * psWks : Structure de workings set.
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée d'un thread worker.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerThread( ACOM_WORKING_SET * psWks )
{
    BOOLEAN             bSucces;
    BOOLEAN             bCnxWorker;
    DWORD               dwErr = NO_ERROR;
    DWORD               dwCompletionBytes;
    void              * pCompletionKey;
	DWORD               dwCompletionDelay;
    DWORD               dwTick;
    BOOL                fInCriticalSection;
    ACOM_OVERLAPPED   * psOver;
    ACOM_BLOCK        * psBlock;

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerThread(0x%016X)", psWks );

    SetThreadPriority( GetCurrentThread(), psWks->dwPriority );

    // Le premier thread lancé sera le worker qui, en plus des i/o gèrera lés
    // connexions
    bCnxWorker = ( InterlockedIncrement( &psWks->dwCnxWorker ) == 1 );
    dwCompletionDelay = ( bCnxWorker ? psWks->dwConnectLoopDelay : INFINITE );
    dwTick = GetTickCount();

    while ( TRUE )
    {
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
        
        if ( ! bCnxWorker )
        {
            EnterCriticalSection( &psWks->sIoCritical );
            fInCriticalSection = TRUE;
        }
        else
        {
            if ( dwCompletionDelay != 0 )
                fInCriticalSection = TryEnterCriticalSection( &psWks->sIoCritical );
            else
                fInCriticalSection = FALSE;
        }

        if ( fInCriticalSection )
        {
            // Récupérer le prochaine packet de completion
            bSucces = GetQueuedCompletionStatus(
                psWks->hCompletion,
                &dwCompletionBytes,
                (PULONG_PTR)&pCompletionKey,
                &((OVERLAPPED*)psOver),
                dwCompletionDelay );
            dwErr = ( bSucces ? NO_ERROR : GetLastError() );
            SetThreadPriority( GetCurrentThread(), psWks->dwPriority );

            EnterCriticalSection( &psWks->sWksCritical );

            psWks->dwWorkerConcurrency ++;
            if ( psWks->dwWorkerConcurrency > psWks->dwMaxWorkerConcurrency )
                psWks->dwMaxWorkerConcurrency = psWks->dwWorkerConcurrency;
            LeaveCriticalSection( &psWks->sWksCritical );

            // S'il s'agit d'une erreur autre que d'i/o ou timeout, on shutdown
            if ( ( dwErr != NO_ERROR ) && ( dwErr != WAIT_TIMEOUT ) && ( psOver == NULL ) )
            {
                AComDbgError( dwErr, __FILE__, __LINE__, "Erreur GetQueuedCompletionStatus" );
                AComClbkShutdown( psWks, dwErr );
                break;
            }

            // Le dépilement a réussi, ce n'est pas un timeout
            // (sur un timeout psOver = NULL ).
            if ( dwErr != WAIT_TIMEOUT )
            {
                // Si la clé est 0, 
                if ( pCompletionKey == NULL )
                {
                    // IL S'AGIT D'UNE COMMANDE QU'ON A FAIT PASSÉE PAR LES I/O COMPLETION
                    // Récupérer le bloc de commande
                    psBlock = (ACOM_BLOCK*)psOver;
                    // Si le block est NULL, il s'agit d'une demande d'arret du thread
                    if ( psBlock == NULL )
                    {
                        AComDbgInfo( __FILE__, __LINE__, "demande d'arret" );
                        break;
                    }
                    else
                    {
                        dwErr = AComWorkerCommand( psBlock );
                        fInCriticalSection = FALSE;
                        if ( dwErr != NO_ERROR )
                            break; // Shutdown callback déjà appelé
                    }
                }
                else
                {
                    // IL S'AGIT D'UN RETOUR D'I/O
                    dwErr = AComWorkerIoCompletion( psOver, dwCompletionBytes, (ULONG_PTR)pCompletionKey, dwErr );
                    fInCriticalSection = FALSE;
                    if ( dwErr != NO_ERROR )
                    {
                        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur AComWorkerIoCompletion" );
                        AComClbkShutdown( psWks, dwErr );
                        break;
                    }
                }
            }
        }
        else
        {
            Sleep( 10 );
            EnterCriticalSection( &psWks->sWksCritical );
            psWks->dwWorkerConcurrency ++;
            if ( psWks->dwWorkerConcurrency > psWks->dwMaxWorkerConcurrency )
                psWks->dwMaxWorkerConcurrency = psWks->dwWorkerConcurrency;
            LeaveCriticalSection( &psWks->sWksCritical );
            dwErr = WAIT_TIMEOUT;
        }

        if ( fInCriticalSection )
        {
            LeaveCriticalSection( &psWks->sIoCritical );
            fInCriticalSection = FALSE;
        }

        // Tester si c'est le moment de vérifier les connexions
        // REMARQUE 1 : La vérification des connexions ne doit-être effectuée qu'après
        // le traitement de l'i/o completion de manière à donner la priorités aux i/o
        // REMARQUE 2 : Pour la vérification du temps écoulé avec le tick 32 bits, il
        // est nécessaire de faire une comparaison de différence pour les problème
        // de cyclage du tick ( faire b - a >= delai et pas b >= a + delai ).
        if ( bCnxWorker )
            if ( ( dwErr                   == WAIT_TIMEOUT              ) ||
                 ( GetTickCount() - dwTick >= psWks->dwConnectLoopDelay ) ||
                 ( dwCompletionDelay       == 0                         ) 
               )
        {
            // Pour prochaine vérif.
            dwTick = GetTickCount();

            // Véfier l'état des connexions, reprendre celles qui sont déconnectées
            dwErr = AComWorkerRefreshConnections( psWks, &dwCompletionDelay );
            // dwErr = AComWorkerRefreshConnections( psWks, NULL );
            if ( dwErr != NO_ERROR )
            {
                AComDbgError( dwErr, __FILE__, __LINE__, "Erreur AComWorkerRefreshConnections" );
                AComClbkShutdown( psWks, dwErr );
                break;
            }

        }

        EnterCriticalSection( &psWks->sWksCritical );
        psWks->dwWorkerConcurrency --;
        LeaveCriticalSection( &psWks->sWksCritical );
    }

    if ( fInCriticalSection )
    {
        LeaveCriticalSection( &psWks->sIoCritical );
        fInCriticalSection = FALSE;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerThread return %d", dwErr );
    ExitThread( dwErr );
    return dwErr;
}


/* -------------  FIN DU FICHIER : acom_worker.c ------------- */ 
