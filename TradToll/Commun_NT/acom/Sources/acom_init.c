/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_init.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions internes d'initialisation et de terminaison
 *              des working-set, des instances et des connexions.
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

#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_timer.h>
#include <acom_dbg.h>
#include <acom_queue.h>
#include <acom_block.h>
#include <acom_worker.h>
#include <acom_sec.h>
#include <acom_clbk.h>
#define LOC_DEF
#include <acom_init.h>
#undef LOC_DEF
#include <acom_tcp.h>

#include <memclass.h>




PROTECTED BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
    switch ( dwWhy )
    {
        
    // Lorsque le process fait le premier LoadLibrary consernant cette DLL
    case DLL_PROCESS_ATTACH :

        // Récupération d'un id de window message
        gdwWmAComEvent = RegisterWindowMessage( ACOM_WM_EVENT_NAME );
        InitializeCriticalSection( &gsGlobalProtect );
        AComDbgInit();

        break;
        
    // Lorsque le process fait le dernier FreeLibrary consernant cett DLL
    case DLL_PROCESS_DETACH :
        DeleteCriticalSection( &gsGlobalProtect );
        break;
    }
    
    return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_WORKING_SET * AComInitOpenWorkingSet ( 
 *                      DWORD64 dwWksUsrKey,
 *                      DWORD dwMaxConnections, 
 *                      DWORD dwMaxInstances, 
 *                      DWORD dwPriority, 
 *                      DWORD dwConnectLoopDelay, 
 *                      DWORD dwWorkers, 
 *                      DWORD dwProtocol,
 *                      DWORD dwLifeTime,
 *                      DWORD dwMaxLife,
 *                      DWORD dwAllowedTimeShift,
 *                      DWORD dwUseMode,
 *                      ACOM_CALLBACK_SHUTDOWN * pfShut,
 *                      HANDLE hObjEvent )
 * PARAMETERS: DWORD64 dwWksUsrKey               : Clé utilisateur associé au working set (pour les
 *                                               callback)
 *             DWORD dwMaxConnections          : Nombre total maximum de connexions pour le working set
 *             DWORD dwMaxInstances            : Nombre total maximum d'instances pour le working set
 *             DWORD dwPriority                : Priorité de traitement des thread workers
 *             DWORD dwConnectLoopDelay        : Fréquence de test de l'état des connexions
 *             DWORD dwWorkers                 : Nombre de thread worker
 *             DWORD dwProtocol                : Type du protocol à mettre en place (ACOM_PROTOCOL_DEFAULT
 *                                               ou ACOM_PROTOCOL_LIFE).
 *                                               ATTENTION : Ce type de protocole ne fonctionne qu'avec
 *                                               les connexions bidirectionnelles (ne marche pas avec
 *                                               les mailslots).
 *             DWORD dwLifeTime                : Délai de vie pour le protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwMaxLife                 : Nombre max de demande de vies infructueuse pour le
 *                                               protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwAllowedTimeShift        : Décalage d'heure GMT permis entre les deux extrémités.
 *                                               Sur dépassement de ce décallage, provoque un callback.
 *                                               Si 0, aucun callback. Valeur en ms.
 *                                               N'est utilisé qu'avec le protocole ACOM_PROTOCOL_LIFE.
 *             DWORD dwUseMode                 : Mode d'utilisation du working set (ACOM_MODE_XXXX)
 *             ACOM_CALLBACK_SHUTDOWN * pfShut : Callback d'erreur critique sur le working set si non null
 *             HANDLE hObjEvent                : En fonction de dwUseMode, handle de la fenêtre ou id du
 *                                               thread recevant les window messages de shutdown.
 * RETURN    : Un pointeur sur une structure de working set ou NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création et l'initialisation d'un working set.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_WORKING_SET * AComInitOpenWorkingSet(
                DWORD64 dwWksUsrKey,
                DWORD dwMaxConnections,
                DWORD dwMaxInstances,
                DWORD dwPriority,
                DWORD dwConnectLoopDelay,
                DWORD dwWorkers,
                DWORD dwProtocol,
                DWORD dwLifeTime,
                DWORD dwMaxLife,
                DWORD dwAllowedTimeShift,
                DWORD dwUseMode,
                ACOM_CALLBACK_SHUTDOWN * pfShut,
                HANDLE hObjEvent )
{
    BOOL bCreated;
    DWORD dwThreadId;
    DWORD dwIndex;
    ACOM_WORKING_SET * psWks = NULL;
    ACOM_CONNECTION * psCnx;
    ACOM_INSTANCE * psInst;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComInitOpenWorkingSet()" );


    if ( ! AComTimerStartThread() )
    {
        AComDbgError( 0, __FILE__, __LINE__, "timerthread" );
    }
    // Vérifier les paramètres
    else if ( ( dwMaxConnections              <= ACOM_MAX_WKS_CONNECTIONS ) && 
              ( dwMaxConnections              >= ACOM_MIN_WKS_CONNECTIONS ) &&
              ( dwMaxInstances                <= ACOM_MAX_WKS_INSTANCES   ) && 
              ( dwMaxInstances                >= ACOM_MIN_WKS_INSTANCES   ) &&
              ( dwWorkers                     <= ACOM_MAX_WORKERS         ) &&
              ( dwWorkers                     >= ACOM_MIN_WORKERS         ) &&
              ( dwUseMode                     <= ACOM_MODE_MAX            ) &&
              ( dwUseMode                     >= ACOM_MODE_MIN            ) &&
              ( ACOM_GET_PROTOCOL(dwProtocol) >= ACOM_PROTOCOL_MIN        ) &&
              ( ACOM_GET_PROTOCOL(dwProtocol) <= ACOM_PROTOCOL_MAX        ) )
    {
        if (  DMEM_ZALLOC( psWks ) != NULL )
        {
            // Initialisation des données de la structure working set
            InitializeCriticalSection( &psWks->sWksCritical );
            InitializeCriticalSection( &psWks->sIoCritical );
            psWks->bShutDown = FALSE;
            psWks->dwWksUsrKey = dwWksUsrKey;
            psWks->dwMaxConnections = dwMaxConnections;
            psWks->dwMaxInstances = dwMaxInstances;
            psWks->dwPriority = dwPriority;
            psWks->dwConnectLoopDelay = dwConnectLoopDelay;
            psWks->dwWorkers = dwWorkers;
            psWks->pfShut = pfShut;
            psWks->dwUseMode = dwUseMode;
            psWks->hObjEvent = hObjEvent;
            psWks->dwProtocol = dwProtocol;
            psWks->dwLifeTime = dwLifeTime;
            psWks->dwMaxLife = dwMaxLife;
            psWks->llAllowedTimeShift = ((LONGLONG)dwAllowedTimeShift)*((LONGLONG)10000);
            psWks->dwWmAComEvent = gdwWmAComEvent;

            // OUVERTURE DES OBJETS DYNAMIQUES DE LA STRUCTURE WORKING SET

            // Sécurité
            psWks->psSA = AComSecOpen();
            if ( psWks->psSA == NULL )
            {
                AComInitCloseWorkingSet( psWks, 0 );
                psWks = NULL;
            }

            // I/O completion port
            if ( psWks != NULL )
            {
                psWks->hCompletion = CreateIoCompletionPort(
                    INVALID_HANDLE_VALUE,
                    NULL,
                    0,
                    dwWorkers );
                if ( psWks->hCompletion == NULL )
                {
                    dwErr = GetLastError();
                    AComInitCloseWorkingSet( psWks, 0 );
                    psWks = NULL;
                    AComDbgError( dwErr, __FILE__, __LINE__, "CreateIoCompletionPort" );
                }
            }

            // Table des connexions
            if ( psWks != NULL )
            {
                if ( DMEM_ZALLOC_TABLE( psWks->psCnx, dwMaxConnections ) == NULL )
                {
                    AComInitCloseWorkingSet( psWks, 0 );
                    psWks = NULL;
                    AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC_TABLE" );
                }
                else
                    for ( dwIndex = 0 ; dwIndex < dwMaxConnections ; dwIndex ++ )
                    {
                        psCnx = &psWks->psCnx[dwIndex];

                        InitializeCriticalSection( &psCnx->sCnxCritical );
                        psCnx->dwState = ACOM_CONNECTION_STATE_FREE;
                        psCnx->dwIndex = dwIndex;
                        psCnx->psWks = psWks;
                        psCnx->sOverIn.dwType  = ACOM_IO_TYPE_INPUT;
                        psCnx->sOverOut.dwType = ACOM_IO_TYPE_OUTPUT;
                        psCnx->sOverCnx.dwType = ACOM_IO_TYPE_CONNECTION;
                    }
            }

            // Table des instances
            if ( psWks != NULL )
            {
                if ( DMEM_ZALLOC_TABLE( psWks->psInst, dwMaxInstances ) == NULL )
                {
                    AComInitCloseWorkingSet( psWks, 0 );
                    psWks = NULL;
                    AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC_TABLE" );
                }
                else
                    for ( dwIndex = 0 ; dwIndex < dwMaxInstances ; dwIndex ++ )
                    {

                        psInst = &psWks->psInst[dwIndex];

                        InitializeCriticalSection( &psInst->sInstCritical );
                        psInst->dwType = ACOM_INSTANCE_FREE;
                        psInst->dwIndex = dwIndex;
                        psInst->psWks = psWks;
                    }
            }

            // Table des threads workers
            if ( psWks != NULL )
            {
                if ( DMEM_ZALLOC_TABLE( psWks->phWorkerThreads, dwWorkers ) == NULL )
                {
                    AComInitCloseWorkingSet( psWks, 0 );
                    psWks = NULL;
                    AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC_TABLE" );
                }
                else
                {
                    bCreated = TRUE;
                    dwErr = NO_ERROR;
                    for ( dwIndex = 0 ; dwIndex < dwWorkers ; dwIndex ++ )
                    {
                        psWks->phWorkerThreads[dwIndex] = ExcptCreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)AComWorkerThread,
                            psWks,
                            CREATE_SUSPENDED,
                            &dwThreadId,
                            "ACOM_WORKER_THREAD"
                            );
                        
                        if ( psWks->phWorkerThreads[dwIndex] == NULL )
                            dwErr = GetLastError();
                        bCreated = ( bCreated && ( psWks->phWorkerThreads[dwIndex] != NULL ) );
                        if ( ! bCreated )
                            break;
                    }

                    if ( ! bCreated )
                    {
                        AComInitCloseWorkingSet( psWks, 0 );
                        psWks = NULL;
                        AComDbgError( dwErr, __FILE__, __LINE__, "CreateThread" );
                    }
                }
            }

            // DEMARRAGE EN BLOC DES WORKERS
            if ( psWks != NULL )
            {
                for ( dwIndex = 0 ; dwIndex < dwWorkers ; dwIndex ++ )
                    ResumeThread( psWks->phWorkerThreads[dwIndex] );

                psWks->bRunning = TRUE;
            }
        }
        else
            AComDbgError( ERROR_INVALID_PARAMETER, __FILE__, __LINE__, "DMEM_ZALLOC psWks" );
    }
    else
        AComDbgError( 0, __FILE__, __LINE__, "connections/instances/workers" );

    if ( psWks == NULL )
        AComTimerStopThread();

    AComDbgInfo( __FILE__, __LINE__, "AComInitOpenWorkingSet return 0x%016X", psWks );
    return psWks;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_INSTANCE * AComInitOpenInstance ( 
 *                      ACOM_WORKING_SET * psWks, 
 *                      DWORD              dwInstType, 
 *                      DWORD64              dwInstUsrKey, 
 *                      DWORD              dwTimeToReconnect, 
 *                      ACOM_PARAM       * psParams, 
 *                      char             * pcFileName )
 * PARAMETERS: ACOM_WORKING_SET * psWks             : Structure de working set ouvert
 *             DWORD              dwInstType        : Type de l'instance (ACOM_INSTANCE_XXXX)
 *             DWORD64              dwInstUsrKey      : Clé utilisateur d'instance (pour callback)
 *             DWORD              dwTimeToReconnect : Délai de reconnexion en cas de déconnexion.
 *             ACOM_PARAM       * psParams          : Structure des paramètres d'instance
 *             char             * pcFileName        : Nom de "fichier" de l'instance dans le
 *                                                    format : \\Machine\Type\Nom
 * RETURN    : Un pointeur sur une structure d'instance, NULL si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création et l'initialisation d'une instance dans un working set.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_INSTANCE * AComInitOpenInstance( 
                ACOM_WORKING_SET * psWks,
                DWORD              dwInstType,
				DWORD64              dwInstUsrKey,
                DWORD              dwTimeToReconnect,
                ACOM_PARAM       * psParams,
                char             * pcFileName )
{
    BOOL bError;
    BOOL bCancel;
    BOOL bIsOverlapped = TRUE;
    DWORD dwIndex;
    DWORD dwConnections;
    DWORD dwMaxMessageSize;
    DWORD dwQueueSize;
	DWORD dwErr = NO_ERROR;
    ACOM_CONNECTION * psCnx;
    ACOM_INSTANCE * psInst = NULL;

    AComDbgInfo( __FILE__, __LINE__, "AComInitOpenInstance(%s)", pcFileName );

    EnterCriticalSection( &psWks->sWksCritical );

    // Vérifier qu'il y a encore de la place
    if ( psWks->dwInstances < psWks->dwMaxInstances )
    {

        // Rechercher un emplacement d'instance libre
        for ( psInst = NULL, dwIndex = 0 ; ( dwIndex < psWks->dwMaxInstances ) && ( psInst == NULL ) ; dwIndex ++ )
        {
            EnterCriticalSection( &psWks->psInst[dwIndex].sInstCritical );
            // Si libre
            if ( psWks->psInst[dwIndex].dwType == ACOM_INSTANCE_FREE )
            {
                // La marquer tout de suite utilisée pour qu'on puisse sortir
                // de la section critique
                psInst = &psWks->psInst[dwIndex];
                psInst->dwType = dwInstType;
            }
            LeaveCriticalSection( &psWks->psInst[dwIndex].sInstCritical );
        }
        if ( psInst != NULL )
        {
            bCancel = FALSE;

            // Entrer dans la section critique de l'instance
            EnterCriticalSection( &psInst->sInstCritical );

            psInst->sParams = *psParams;
            strncpy_s( psInst->szFileName, MAX_PATH, pcFileName, sizeof(psInst->szFileName) );
            psInst->szFileName[sizeof(psInst->szFileName)-1] = '\0';
            psInst->dwInstUsrKey = dwInstUsrKey;
            psInst->dwTimeToReconnect = dwTimeToReconnect;
            psInst->pfCnx  = psInst->sParams.pfCnx;
            psInst->pfDcnx = psInst->sParams.pfDcnx;
            psInst->pfRecv = psInst->sParams.pfRecv;
            psInst->pfSent = psInst->sParams.pfSent;
            psInst->pfTime = psInst->sParams.pfTime;
            psInst->dwUseMode = psInst->sParams.dwUseMode;
            psInst->hObjEvent = psInst->sParams.hObjEvent;

            // En fonction du type de l'instance, récupérer les
            // paramètres utils et vérifier ceux qui ne peuvent fonctionner.
            switch ( dwInstType )
            {
            case ACOM_INSTANCE_PIPE_SERVER :

                psInst->bHasInput = TRUE;
                psInst->bHasOutput = TRUE;
                dwConnections = psParams->sPipeServer.dwMaxConnections;
                dwMaxMessageSize = psParams->sPipeServer.dwMaxMessageSize;
                dwQueueSize = psParams->sPipeServer.dwQueueSize;
                break;

            case ACOM_INSTANCE_PIPE_CLIENT :

                psInst->bHasInput = TRUE;
                psInst->bHasOutput = TRUE;
                dwConnections = 1;
                dwMaxMessageSize = psParams->sPipeClient.dwMaxMessageSize;
                dwQueueSize = psParams->sPipeClient.dwQueueSize;
                break;

            case ACOM_INSTANCE_MAIL_SERVER :

                if ( ACOM_GET_PROTOCOL(psWks->dwProtocol) != ACOM_PROTOCOL_DEFAULT )
                {
                    AComInitCloseInstance( psInst );
                    bCancel = TRUE;
                    AComDbgError( 0, __FILE__, __LINE__, "protocole non valide pour ce type d'instance" );
                    break;
                }
                psInst->bHasInput = TRUE;
                psInst->bHasOutput = FALSE;
                psInst->pfSent = NULL;
                dwConnections = 1;
                dwMaxMessageSize = psParams->sMailServer.dwMaxMessageSize;
                dwQueueSize = 0;
                break;

            case ACOM_INSTANCE_MAIL_CLIENT :

                if ( ACOM_GET_PROTOCOL(psWks->dwProtocol) != ACOM_PROTOCOL_DEFAULT )
                {
                    AComInitCloseInstance( psInst );
                    bCancel = TRUE;
                    AComDbgError( 0, __FILE__, __LINE__, "protocole non valide pour ce type d'instance" );
                    break;
                }
                bIsOverlapped = FALSE;
                psInst->bHasInput = FALSE;
                psInst->bHasOutput = TRUE;
                psInst->pfRecv = NULL;
                dwConnections = 1;
                dwMaxMessageSize = 0;
                dwQueueSize = psParams->sMailClient.dwQueueSize;
                break;

            case ACOM_INSTANCE_TCP_SERVER :

                psInst->bHasInput = TRUE;
                psInst->bHasOutput = TRUE;
                dwConnections = psParams->sTcpServer.dwMaxConnections;
                dwMaxMessageSize = psParams->sTcpServer.dwMaxMessageSize;
                dwQueueSize = psParams->sTcpServer.dwQueueSize;
                break;

            case ACOM_INSTANCE_TCP_CLIENT :

                psInst->bHasInput = TRUE;
                psInst->bHasOutput = TRUE;
                dwConnections = 1;
                dwMaxMessageSize = psParams->sTcpClient.dwMaxMessageSize;
                dwQueueSize = psParams->sTcpClient.dwQueueSize;
                break;

            case ACOM_INSTANCE_SERIAL :

                if ( ACOM_GET_PROTOCOL(psWks->dwProtocol) != ACOM_PROTOCOL_DEFAULT )
                {
                    AComInitCloseInstance( psInst );
                    bCancel = TRUE;
                    AComDbgError( 0, __FILE__, __LINE__, "protocole non valide pour ce type d'instance" );
                    break;
                }
                psInst->bHasInput = TRUE;
                psInst->bHasOutput = TRUE;
                dwConnections = 1;
                dwMaxMessageSize = psParams->sSerial.dwMaxMessageSize;
                dwQueueSize = psParams->sSerial.dwQueueSize;
                break;

            default :
            
                AComInitCloseInstance( psInst );
                bCancel = TRUE;
                AComDbgError( 0, __FILE__, __LINE__, "type non valide" );
            }

            if ( ! bCancel )
            {
                if ( ( dwConnections + psWks->dwConnections ) > psWks->dwMaxConnections )
                {
                    AComInitCloseInstance( psInst );
                    bCancel = TRUE;
                    AComDbgError( 0, __FILE__, __LINE__, "max connexions" );
                }
            }

            if ( ! bCancel )
            {
                // L'init des paramètres est OK

                bError = FALSE;

                // Préparer les connexions de l'instances
                for ( dwIndex = 0 ; 
                      ( ! bError ) && ( dwIndex < psWks->dwMaxConnections ) && ( dwConnections > 0 ) ;
                      dwIndex ++ 
                    )
                {
                    psCnx = &psWks->psCnx[dwIndex];
                    EnterCriticalSection( &psCnx->sCnxCritical );

                    if ( psCnx->dwState == ACOM_CONNECTION_STATE_FREE )
                    {
                        psCnx->dwState = ACOM_CONNECTION_STATE_INIT;
                        psCnx->dwLifeCount = 0;
                        psCnx->bIsOverlapped = bIsOverlapped;
                        psCnx->hCnxHandle = 0;
                        psCnx->dwCnxUsrKey = 0;
                        psCnx->dwReadBufferSize = dwMaxMessageSize;
                        psCnx->psWriteCurrent = NULL;
                        psCnx->psInst = psInst;
                        psCnx->dwLastConnectionTry = 0;
                        psCnx->sOverCnx.psCnx = psCnx;
                        psCnx->sOverIn.psCnx = psCnx;
                        psCnx->sOverOut.psCnx = psCnx;
                        psCnx->bEnabled = TRUE;
                        psCnx->dwReadPos = 0;
                        psCnx->hFile = NULL;

                        if ( dwMaxMessageSize != 0 )
                        {
                            if ( DMEM_ALLOC_BUFFER( psCnx->pbReadBuffer,dwMaxMessageSize ) == NULL )
                                bError = TRUE;
                        }
                        else 
                            psCnx->pbReadBuffer = NULL;

                        if ( ! bError && dwQueueSize > 0 )
                        {
                            if ( ( psCnx->psWriteQueue = AComQueueOpen( dwQueueSize ) ) == NULL )
                            {
                                if ( psCnx->pbReadBuffer != NULL ) DMEM_FREE( psCnx->pbReadBuffer );
                                bError = TRUE;
                            }
                        }
                        else
                            psCnx->psWriteQueue = NULL;
                
                        if ( ! bError )
                        {
                            psCnx->dwState = ACOM_CONNECTION_STATE_DISCONNECTED;
                            psCnx->szIdentity[0] = 0;
                            dwConnections --;
                            psWks->dwConnections ++;
                        }
                    }
                    LeaveCriticalSection( &psCnx->sCnxCritical );
                }

                if ( ( ! bCancel ) && ( ! bError ) && ( dwConnections == 0 ) )
                {
                    if ( dwInstType == ACOM_INSTANCE_TCP_SERVER )
                    {
                        dwErr = AComTcpServerPrepare( psInst );
                        if ( dwErr != NO_ERROR )
                        {
                            bError = TRUE;
                            AComDbgError( dwErr, __FILE__, __LINE__, "Erreur lors de la préparation de l'écoute" );
                        }
                    }
                }

                if ( ( ! bCancel ) & ( bError ) || ( dwConnections > 0 ) )
                {
                    AComInitCloseInstance( psInst );
                    bCancel = TRUE;
                    AComDbgError( 0, __FILE__, __LINE__, "creation connexion" );
                }

            }

            LeaveCriticalSection( &psInst->sInstCritical );

            if ( bCancel )
                psInst = NULL;
        }
        else
            AComDbgError( 0, __FILE__, __LINE__, "emplacement libre non trouvé" );

    }
    else
        AComDbgError( 0, __FILE__, __LINE__, "max instances" );

    LeaveCriticalSection( &psWks->sWksCritical );

    AComDbgInfo( __FILE__, __LINE__, "AComInitOpenInstance return 0x%016X", psInst );
    return psInst;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComInitCloseInstance ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la fermetue d'une instance ouverte ou partiellement ouverte
 * --------------------------------------------------------------------
 */
PROTECTED void AComInitCloseInstance( ACOM_INSTANCE * psInst )
{
    DWORD dwIndex;
    ACOM_WORKING_SET * psWks = psInst->psWks;
    ACOM_CONNECTION * psCnx;

    AComDbgInfo( __FILE__, __LINE__, "AComInitCloseInstance(0x%016X)", psInst );

    EnterCriticalSection( &psWks->sWksCritical );
    EnterCriticalSection( &psInst->sInstCritical );

    if ( psInst->dwType == ACOM_INSTANCE_TCP_SERVER )
        AComTcpServerFinish( psInst );

    for ( dwIndex = 0 ; dwIndex < psWks->dwMaxConnections ; dwIndex ++ )
    {
        psCnx = &psWks->psCnx[dwIndex];
        EnterCriticalSection( &psCnx->sCnxCritical );
        if ( ( psCnx->dwState != ACOM_CONNECTION_STATE_FREE ) && ( psCnx->psInst == psInst ) )
        {
            // Desactiver la connexion pour empecher toute tentative de reconnexion
            psCnx->bEnabled = FALSE;

            // SI ON EST CONNECTÉ OU EN ERREUR
            if ( ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED ) ||
                 ( psCnx->dwState == ACOM_CONNECTION_STATE_ERROR ) )
            {
                if ( psCnx->psWriteCurrent != NULL )
                {
                    // Pas de stat à mettre à jours
                    AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                    DMEM_FREE( psCnx->psWriteCurrent );
                }
                while ( ( psCnx->psWriteCurrent = AComQueueRemoveItem( psCnx->psWriteQueue ) ) != NULL )
                {
                    // Pas de stat à mettre à jours
                    AComClbkSent( psCnx, ERROR_BROKEN_PIPE );
                    DMEM_FREE( psCnx->psWriteCurrent );
                }

                AComClbkDisconnection( psCnx );
            }

            // if ( ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTING ) ||
            //      ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED  ) ||
            //      ( psCnx->dwState == ACOM_CONNECTION_STATE_ERROR      ) )
            if ( psCnx->hFile != NULL )
            {
                CancelIo( psCnx->hFile );
                ACOM_CLOSE_HANDLE( psCnx );
            }

            if ( psCnx->pbReadBuffer != NULL ) DMEM_FREE( psCnx->pbReadBuffer );
            if ( psCnx->psWriteQueue != NULL ) AComQueueClose( psCnx->psWriteQueue );
            psCnx->psInst = NULL;
            psCnx->dwState = ACOM_CONNECTION_STATE_FREE;
            psWks->dwConnections --;
        }
        LeaveCriticalSection( &psCnx->sCnxCritical );
    }

    psInst->dwType = ACOM_INSTANCE_FREE;

    LeaveCriticalSection( &psInst->sInstCritical );
    LeaveCriticalSection( &psWks->sWksCritical );

    AComDbgInfo( __FILE__, __LINE__, "AComInitCloseInstance return", psInst );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComInitCloseWorkingSet ( ACOM_WORKING_SET * psWks, DWORD dwTimeout )
 * PARAMETERS: ACOM_WORKING_SET * psWks : Structure de working set
 *             DWORD dwTimeout          : Délai imparti à la fermeture
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la fermeture d'un working set ouvert ou ouvert
 *             partiellement ainsi que les instances qu'il contient.
 *             Si au bout du temp imparti, les thread workers ne sont
 *             pas arrétés, ceux-ci sont tués.
 * --------------------------------------------------------------------
 */
PROTECTED void AComInitCloseWorkingSet( ACOM_WORKING_SET * psWks, DWORD dwTimeout )
{
    DWORD dwIndex;

    AComDbgInfo( __FILE__, __LINE__, "AComInitCloseWorkingSet(0x%016X)", psWks );

    if ( psWks->bRunning == TRUE )
    {
        for ( dwIndex = 0 ; dwIndex < psWks->dwWorkers ; dwIndex ++ )
            PostQueuedCompletionStatus( psWks->hCompletion, 0, 0, NULL );
        WaitForMultipleObjects(
            psWks->dwWorkers,
            psWks->phWorkerThreads,
            TRUE,
            dwTimeout );
        for ( dwIndex = 0 ; dwIndex < psWks->dwWorkers ; dwIndex ++ )
        {
            TerminateThread( psWks->phWorkerThreads[dwIndex], 0xFFFFFFFF );
            CloseHandle( psWks->phWorkerThreads[dwIndex] );
        }
    }

    EnterCriticalSection( &psWks->sWksCritical );

    if ( psWks->psInst != NULL )
    {
        for ( dwIndex = 0 ; dwIndex < psWks->dwMaxInstances ; dwIndex ++ )
        {
            EnterCriticalSection( &psWks->psInst[dwIndex].sInstCritical );
            if ( psWks->psInst[dwIndex].dwType != ACOM_INSTANCE_FREE )
                AComInitCloseInstance( &psWks->psInst[dwIndex] );
            LeaveCriticalSection( &psWks->psInst[dwIndex].sInstCritical );
            DeleteCriticalSection( &psWks->psInst[dwIndex].sInstCritical );
        }
        DMEM_FREE( psWks->psInst );
        if ( psWks->dwConnections != 0 )
        {
            // Y A UN PB
        }
    }

    if ( psWks->psCnx != NULL )
    {
        // Libérer les sections critiques associées aux connexions
        for ( dwIndex = 0 ; dwIndex < psWks->dwMaxConnections ; dwIndex ++ )
            DeleteCriticalSection( &psWks->psCnx[dwIndex].sCnxCritical );
        DMEM_FREE( psWks->psCnx );
    }

    if ( psWks->hCompletion != NULL )
        CloseHandle( psWks->hCompletion );

    if ( psWks->phWorkerThreads != NULL )
        DMEM_FREE( psWks->phWorkerThreads );

    if ( psWks->psSA != NULL )
        AComSecClose( psWks->psSA );

    DeleteCriticalSection( &psWks->sWksCritical );
    DeleteCriticalSection( &psWks->sIoCritical );

    DMEM_FREE( psWks );

    AComTimerStopThread();
}





/* -------------  FIN DU FICHIER : acom_init.c ------------- */ 
