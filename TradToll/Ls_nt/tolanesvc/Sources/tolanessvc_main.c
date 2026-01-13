/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : tolanessvc
 * FILE       : tolanessvc_main.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module principal du service
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

#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <csr_list.h>
#include <csr_msg.h>
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>
#include <col.h>
#include <reg.h>

#include <resource.h>

#define LOC_DEF
#include <tolanessvc_glob.h>
#undef LOC_DEF

#include <tolanessvc_text.h>
#include <tolanessvc_callback.h>
#include <tolanessvc_lane.h>
#include <tolanessvc_db.h>

#define LOC_DEF
#include <tolanessvc_main.h>
#undef LOC_DEF

#include <memclass.h>

// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE void MainTreatment();
PRIVATE DWORD MainInitMessages();
PRIVATE void MainTerminateMessages();
PRIVATE DWORD WINAPI MainCommand( char * pcParams );
PRIVATE void WINAPI MainLoadMenu();
PRIVATE void WINAPI MainUnloadMenu();
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId );


// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
 *                        OUT char ** ppcServiceName,
 *                        OUT NTSVCCommandMain ** ppfCommand )
 * PARAMETERS: Cf doc NTSVC
 * RETURN    : Cf doc NTSVC
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Cf doc NTSVC
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
        OUT     char ** ppcServiceName,
        OUT     NTSVCCommandMain ** ppfCommand )
{
    (*ppcServiceName) = gszSvcName;
    (*ppfCommand) = MainCommand;
    return TRUE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
 *                          IN      DWORD    dwArgc,
 *                          IN      char  ** ppcArgv )
 * PARAMETERS: dwArgc  : nombre de params.
 *             ppcArgv : liste des arguments
 * RETURN    : paramètre, ligne de commande, initialisation
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré pour le service
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
        IN      DWORD    dwArgc,
        IN      char  ** ppcArgv )
{
    DWORD dwErr;
    DWORD dwLoadCount = 0;
    DWORD dwWait;
    DWORD dwDelay;
    BOOL bOnOff = TRUE;

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" TOLANESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Définition des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            TOLANESSVC_REG_VAL_CMDSVC        , REG_DWORD,        4,                1, &gsSvcWork.sParmCopy.dwCmdSvc,
            TOLANESSVC_REG_VAL_MAXLANES      , REG_DWORD,        4,               64, &gsSvcWork.sParmCopy.dwMaxLanes,
            TOLANESSVC_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,                5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            TOLANESSVC_REG_VAL_WORKERS       , REG_DWORD,        4,                2, &gsSvcWork.sParmCopy.dwWorkers,
            TOLANESSVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4,             1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            TOLANESSVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4,              256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            TOLANESSVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,            16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            TOLANESSVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,            16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            TOLANESSVC_REG_VAL_MAXLIFE       , REG_DWORD,        4,                5, &gsSvcWork.sParmCopy.dwMaxLife,
            TOLANESSVC_REG_VAL_LIFETIME      , REG_DWORD,        4,             5000, &gsSvcWork.sParmCopy.dwLifeTime,
            TOLANESSVC_REG_VAL_MAINPOLLING   , REG_DWORD,        4,            30000, &gsSvcWork.sParmCopy.dwMainPolling,
            TOLANESSVC_REG_VAL_ERRORSLEEP    , REG_DWORD,        4,            10000, &gsSvcWork.sParmCopy.dwErrorSleep,
            TOLANESSVC_REG_VAL_SENDAGAIN     , REG_DWORD,        4,           300000, &gsSvcWork.sParmCopy.dwSendAgain,
            TOLANESSVC_REG_VAL_APP_MSG_ID    , REG_DWORD,        4,              111, &gsSvcWork.sParmCopy.dwAppMsgId,
            TOLANESSVC_REG_VAL_LANESFILEONLY , REG_DWORD,        4,                0, &gsSvcWork.sParmCopy.dwLanesFileOnly,
            TOLANESSVC_REG_VAL_TCPROUTE      , REG_DWORD,        4,                0, &gsSvcWork.sParmCopy.dwTCPRoute,
            TOLANESSVC_REG_VAL_DBUSR         , REG_SZ   , MAX_PATH,        "comuser", &gsSvcWork.sParmCopy.szDbUsr,
            TOLANESSVC_REG_VAL_DBPWD         , REG_SZ   , MAX_PATH,         "compwd", &gsSvcWork.sParmCopy.szDbPwd,
            TOLANESSVC_REG_VAL_DBINST        , REG_SZ   , MAX_PATH,            "pcs", &gsSvcWork.sParmCopy.szDbInst,
            TOLANESSVC_REG_VAL_ROUTESERVER   , REG_SZ   , MAX_PATH,              ".", &gsSvcWork.sParmCopy.szRouteServer,
            TOLANESSVC_REG_VAL_LANESFILE     , REG_SZ   , MAX_PATH, "C:\\TOLNLN.TXT", &gsSvcWork.sParmCopy.szLanesFile,
            TOLANESSVC_REG_VAL_REFERENCEDIR  , REG_SZ   , MAX_PATH,  "C:\\REFERENCE", &gsSvcWork.sParmCopy.szReferenceDir,
            TOLANESSVC_REG_VAL_CONTEXT_ID    , REG_DWORD,        4,                0, &gsSvcWork.sParmCopy.dwContextId,
            TOLANESSVC_REG_VAL_PERMANENTDB   , REG_DWORD,        4,                0, &gsSvcWork.sParmCopy.dwPermanentDB,
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TOLANESSVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TOLANESSVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TOLANESSVC_OK_RUN ) );

    // Boucler sur démarré / en pause -> rechargement des paramètre à chaque boucle
    do {

        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INITIALIZING" );

        // Dans le cas de la premiere boucle, on commence dans l'état "en cours de démarrage"
        if ( dwLoadCount == 0 )
        {
            // Passer dans l'état "en cours de démarrage"
            NTSVCSetCurrentState( SERVICE_START_PENDING, 10000, NO_ERROR );
        }

        // Lire les paramètres d'init du service
        dwErr = MainInitService();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_SERVICE" );
            break;
        }

        // Initialiser les requètes base
        DBInitRequests();

        // Initialiser la messagerie
        dwErr = MainInitMessages();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_LC_MSG_INIT" );
            MainTerminateService();
            break;
        }

        // Initialiser la com
        dwErr = MainInitPipes();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_PIPES" );
            MainTerminateMessages();
            MainTerminateService();
            break;
        }

        // Dans le cas de la premiere boucle, on passe a l'état "démarré"
        if ( dwLoadCount == 0 )
        {
            // Passer dans l'état "running"
            NTSVCSetCurrentState( SERVICE_RUNNING, 0, NO_ERROR );
            NTSVCError( TextFind( gsSvcWork.hText, "SVC_STARTED", "SVC_STARTED" ) );
        }

        // Les traitements ont été lancés avec les workers

        MainLoadMenu();

        NTSVCInfo( "NTSVCExternalMain(), démarrage de la boucle principale" );
        // Attendre la demande d'arrêt tout en gerant l'interface avec la
        // base de données
        do
        {
            NTSVCRefreshTrayIcon( gsSvcWork.hIconOk, "SVC_RUNNING" );

            // Déclencher le traitement périodique ( interrogation de
            // la base pour connaitre les changements ).
            MainTreatment();

            dwDelay = 0;
            while ( TRUE )
            {
                bOnOff = ! bOnOff;
                NTSVCRefreshTrayIcon( bOnOff ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );
                // Attendre la demande de fin ou l'écoulement du délai de
                // polling de la base de données
                dwWait = NTSVCWaitForEndOrMultipleObjects(
                            1, 
                            &gsSvcWork.hWakeupEvent, 
                            1000 );
                if ( dwWait != WAIT_TIMEOUT ) break;
                dwDelay = dwDelay + 1000;
                if ( dwDelay >= gsSvcWork.sParmWork.dwMainPolling ) { dwWait = WAIT_TIMEOUT; break; }
            }
        }
        while ( ( dwWait == ( WAIT_OBJECT_0 + 1 ) ) || ( dwWait == WAIT_TIMEOUT ) );

        MainUnloadMenu();

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

        DBCleanup();

        // Il s'agit d'une vraie demande d'arrêt, mais de rechargement
        if ( ! gsSvcWork.bReload )
        {
            // Passer dans l'état "en cours d'arrêt"
            NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );
        }
        // Il s'agit simplement d'une demande de rechargement
        else
            NTSVCResetEnd();

        NTSVCInfo( "NTSVCExternalMain(), destruction des objets" );
        MainTerminatePipes();
        MainTerminateMessages();

        NTSVCError( TextFind( gsSvcWork.hText, "SVC_STOPPED", "SVC_STOPPED" ) );
        MainTerminateService();

        NTSVCRefreshTrayIcon( NULL, NULL );

        dwLoadCount ++;
    }
    while ( gsSvcWork.bReload );

    DestroyIcon( gsSvcWork.hIconOk );
    DestroyIcon( gsSvcWork.hIconKo );
    DestroyIcon( gsSvcWork.hIconOkRun );

    // Fermeture des paramètres
    NTSVCCloseParameters( gsSvcWork.psParams );

    TextUnloadDefinitions( gsSvcWork.hText );

    NTSVCSetCurrentState( SERVICE_STOPPED, 0, dwErr );

    // Tout est terminé
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainInitService()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Charge les données de conf. et initialise le service
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainInitService()
{
    DWORD dwErr = NO_ERROR;   // Code d'erreur à renvoyer
    DWORD dwFoo;

    // Charger les paramètres
    dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwFoo );
    if ( dwErr != NO_ERROR )
    {
        SVC_ERR( dwErr, "ERR_LOAD_PARAMS" );
        return dwErr;
    }

    gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
    gsSvcWork.bReload = FALSE;
    gsSvcWork.llSendAgain = ((LONGLONG)gsSvcWork.sParmWork.dwSendAgain) * 10000;

    // Initialisation de la section critique de protection des listes
    InitializeCriticalSection( &gsSvcWork.sCritical );

    // Initialiser l'évènement chargé de réveiller la boucle principale
    gsSvcWork.hWakeupEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( gsSvcWork.hWakeupEvent == NULL )
    {
        dwErr = GetLastError();
        SVC_ERR( dwErr, "ERR_INIT_WAKEUP_EVENT" );
        DeleteCriticalSection( &gsSvcWork.sCritical );
        return dwErr;
    }

    // Préparation de la tables des voies
    gsSvcWork.psList = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, gsSvcWork.sParmWork.dwMaxLanes * sizeof(TOLANESSVC_LANE) );
    if ( gsSvcWork.psList == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        SVC_ERR( dwErr , "ERR_INIT_LC_LIST" );
        CloseHandle( gsSvcWork.hWakeupEvent );
        DeleteCriticalSection( &gsSvcWork.sCritical );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    gsSvcWork.psListNew = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, gsSvcWork.sParmWork.dwMaxLanes * sizeof(TOLANESSVC_LANE) );
    if ( gsSvcWork.psListNew == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        SVC_ERR( dwErr , "ERR_INIT_LC_LIST" );
        HeapFree( GetProcessHeap(), 0, gsSvcWork.psList );
        CloseHandle( gsSvcWork.hWakeupEvent );
        DeleteCriticalSection( &gsSvcWork.sCritical );
        return dwErr;
    }

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainTerminateService()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Termine le service et décharge les données de conf.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainTerminateService()
{
    DWORD dwErr = NO_ERROR;

    // Fermeture de la tables des voies
    HeapFree( GetProcessHeap(), 0, gsSvcWork.psListNew );
    HeapFree( GetProcessHeap(), 0, gsSvcWork.psList );

    CloseHandle( gsSvcWork.hWakeupEvent );

    DeleteCriticalSection( &gsSvcWork.sCritical );

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainInitPipes()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Met en place le serveur de pipe.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainInitPipes()
{
    char szPipe[MAX_PATH*2];

    // Ajout du pipe dans la liste null session
    AComSetNullSessionPipe( TOLANESSVC_PIPE_CMD, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.sParmWork.dwMaxCmdCnx + 1,
            2,
            THREAD_PRIORITY_ABOVE_NORMAL,
            1000,
            gsSvcWork.sParmWork.dwWorkers,
            ACOM_PROTOCOL_LIFE,
            gsSvcWork.sParmWork.dwLifeTime,
            gsSvcWork.sParmWork.dwMaxLife,
            0,
            CallbackShutdown );
    if ( gsSvcWork.hWks == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_WKS" );
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE L'INSTANCE DE COMMANDE

    NTSVCInfo( "MainInitPipes(), création du serveur interface de commande" );
    if ( gsSvcWork.sParmWork.dwCmdSvc != 0 )
    {
        if ( gsSvcWork.sParmWork.dwCmdSvc == 1 )
            gsSvcWork.hCmdInst = AComOpenPipeServerInstance(
                    gsSvcWork.hWks,
                    TOLANESSVC_PIPE_CMD,
                    TOLANESSVC_PIPE_CMD_TYPE,
                    gsSvcWork.sParmWork.dwTimeToReconnect,
                    gsSvcWork.sParmWork.dwMaxCmdCnx,
                    gsSvcWork.sParmWork.dwPipeBufferSize,
                    gsSvcWork.sParmWork.dwPipeBufferSize,
                    gsSvcWork.sParmWork.dwMaxQueuedMessages,
                    gsSvcWork.sParmWork.dwMaxMsgSize,
                    CallbackConnection,
                    CallbackDisconnection,
                    CallbackReceived,
                    CallbackSent,
                    NULL );
        else
            gsSvcWork.hCmdInst = AComOpenTcpServerInstance(
                    gsSvcWork.hWks,                             // hWks
                    "",                                         // pcAddress
                    gsSvcWork.sParmWork.dwCmdSvc,               // dwPort
                    TOLANESSVC_PIPE_CMD_TYPE,                   // dwInstUsrKey
                    gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
                    gsSvcWork.sParmWork.dwMaxCmdCnx,            // dwMaxConnections
                    gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
                    gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
                    gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
                    gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
                    CallbackConnection,
                    CallbackDisconnection,
                    CallbackReceived,
                    CallbackSent,
                    NULL );
        if ( gsSvcWork.hCmdInst == NULL )
        {
            SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_CMD_SRV" );
            AComCloseWorkingSet( gsSvcWork.hWks, TOLANESSVC_WORKINGSET_TIEMOUT );
            return ERROR_INVALID_DATA;
        }
    }

    // MISE EN PLACE DE LA CONNEXION AVEC LA COM

    NTSVCInfo( "MainInitPipes(), création du client" );
    sprintf_s( szPipe, sizeof(szPipe), TOLANESSVC_PIPE_COM, gsSvcWork.sParmWork.szRouteServer );
    if ( gsSvcWork.sParmWork.dwTCPRoute == 0 )
        gsSvcWork.hComInst = AComOpenPipeClientInstance(
                gsSvcWork.hWks,
                szPipe,
                TOLANESSVC_PIPE_COM_TYPE,
                gsSvcWork.sParmWork.dwTimeToReconnect,
                gsSvcWork.sParmWork.dwMaxQueuedMessages,
                gsSvcWork.sParmWork.dwMaxMsgSize,
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
    else
        gsSvcWork.hComInst = AComOpenTcpClientInstance(
                gsSvcWork.hWks,                           // hWks
                gsSvcWork.sParmWork.szRouteServer,        // pcAddress
                gsSvcWork.sParmWork.dwTCPRoute,           // dwPort
                TOLANESSVC_PIPE_COM_TYPE,                 // dwInstUsrKey
                gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
                gsSvcWork.sParmWork.dwPipeBufferSize,     // dwOutBufferSize
                gsSvcWork.sParmWork.dwPipeBufferSize,     // dwInBufferSize
                gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
                gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
    if ( gsSvcWork.hComInst == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_LS_CLT" );
        AComCloseWorkingSet( gsSvcWork.hWks, TOLANESSVC_WORKINGSET_TIEMOUT );
        return ERROR_INVALID_DATA;
    }

    NTSVCInfo( "MainInitPipes(), toutes les instance sont créés" );

    return NO_ERROR;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainTerminatePipes()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Termine le working set
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainTerminatePipes()
{
    NTSVCInfo( "MainTerminatePipes(), destruction des objets de com" );

    AComCloseWorkingSet( gsSvcWork.hWks, TOLANESSVC_WORKINGSET_TIEMOUT );

    return NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void MainTreatment()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traitement de la boucle principal :
 *             Etablir la liste des voies attendues et leur
 *             fichier de réference. Sur connexion, envoyer
 *             message de référence. Sur changement de référence,
 *             envoyer message de référence. Sur expiration
 *             de délai, envoyer message de référence.
 * --------------------------------------------------------------------
 */
PRIVATE void MainTreatment()
{
    LONGLONG llTime;           // Date courante au format FILETIME
    DWORD dwIndex;             // Indice de boucle, index de voie
    TOLANESSVC_LANE * psLane;  // Pointe sur une voie en particulier
    DWORD dwErr;               // Code d'erreur
    char szText[32];

    // Rapatrier les dernières infos de la base
    dwErr = DBGetLanes();
    if ( dwErr != NO_ERROR )
    {
        // Les données de la base n'ont pas pu être lues
        SVC_ERR( dwErr, "ERR_GET_LANES" );

        // On continue quand même, en utilisant les données mémorisées
    }
    
    // Quelle est l'heure courante ?
    GetSystemTimeAsFileTime( (LPFILETIME)&llTime );

    // On utilise la liste des voie, on passe donc en section critique
    EnterCriticalSection( &gsSvcWork.sCritical );

    // Pour chacun des éléments de la liste
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
    {
        psLane = &gsSvcWork.psList[dwIndex];

        // L'élement est-il une voie, et cette voie est-elle connectée ?
        if ( psLane->bBusy && psLane->bConnected )
        {
            // Si on a eu un retour arrière de plus de 2 secondes au niveau de l'heure,
            // on fait en sorte que le traitement soit déclenché.
            if ( ( llTime - psLane->llLastSent ) < (LONGLONG)( -2 * 10000000 ) )
            {
                sprintf_s( szText, sizeof(szText), "PZ%04u-LN%04u", psLane->dwPlaza, psLane->dwLane  );
                SVC_ERR_S( dwErr, "ERR_BACK_TIME", szText );
                psLane->llLastSent = 0;
            }

            // Il s'agit bien d'une voie connectée
            // La valeur psLane->bMustSend qui a déjà été mise à jour pour les
            // voies nouvellement connectées indique pour chaque voie si un
            // message de référence doit être envoyé
            // Si le temps de réémission est dépassé, il faut également
            // envoyer un message de référence.
            // Si le temps de réémission est 0, on n'effectue aucune réémission
            if ( psLane->bMustSend || 
                 ( ( (llTime-psLane->llLastSent) > gsSvcWork.llSendAgain )  && ( gsSvcWork.llSendAgain != (LONGLONG)0) ) )
            {
                // Envoyer le message
                LaneSendRefMsg( dwIndex );

                // Remise à zéro des indicateurs
                psLane->bMustSend = FALSE;
                psLane->llLastSent = llTime;
            }
        }
    }

    LeaveCriticalSection( &gsSvcWork.sCritical );

    // En cas d'erreur, on marque une pause
    if ( dwErr != NO_ERROR ) 
        NTSVCWaitForEnd( gsSvcWork.sParmWork.dwErrorSleep );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainInitMessages()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Met en place la définition des messages à recevoir.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainInitMessages()
{
    if ( ! MSG_SV_CON_REP_New_Record( &gsSvcWork.hInMsgList ) )
        return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void MainTerminateMessages()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Libère définition des messages à recevoir.
 * --------------------------------------------------------------------
 */
PRIVATE void MainTerminateMessages()
{
    MSG_Delete_All_Records( &gsSvcWork.hInMsgList );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD WINAPI MainCommand( char * pcParams )
 * PARAMETERS: Cf doc NTSVC
 * RETURN    : Cf doc NTSVC
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Cf doc NTSVC
 * --------------------------------------------------------------------
 */
PRIVATE DWORD WINAPI MainCommand( char * pcParams )
{
    DWORD dwErr;

    // Traitement ligne de commande
    if ( _stricmp( pcParams, "NULLPIPE" ) == 0 )
    {
        printf( "ToLanesSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( TOLANESSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "ToLanesSvc - Error %u\n", dwErr );
        else
            printf( "ToLanesSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI MainLoadMenu()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Chargement des menus
 * --------------------------------------------------------------------
 */
PRIVATE void WINAPI MainLoadMenu()
{
    DWORD   dwErr;
    DWORD   dwIndex = 0;
    DWORD   dwSize;
    DWORD   dwVal;
    char    szKey[1024];
    char    szText[256];

    //
    // Par défaut, pas d'item de menu customisé.
    //
    gsSvcWork.dwMenuCommands = 0;

    //
    // Avant d'ajouter les éléments du menu, on défini
    // le handler.
    //
    NTSVCSetTrayIconHandler( &MainTrayIconHandler );

    //
    // Il faut maintenant aller voir dans le registre
    // items customisés du menu.
    //
    while ( gsSvcWork.dwMenuCommands < MAX_CUSTOM_MENU_ITEMS )
    {
        //
        // Composer le chemin d'accès à la clé du registre
        //
        sprintf_s( szKey, sizeof(szKey), NTSVC_REG_KEY_SERVICE "\\" 
                        TOLANESSVC_SERVICE_NAME "\\" 
                        NTSVC_REG_KEY_PARAM "\\MenuItems\\%u", gsSvcWork.dwMenuCommands );

        //
        // Lire le texte associé à l'item.
        //
        dwSize = sizeof(szText);
        dwErr = REG_Lire_Chaine( HKEY_LOCAL_MACHINE,
                                 szKey,
                                 "Text",
                                 szText,
                                 &dwSize );
        if ( dwErr != NO_ERROR )
        {
            //
            // La clé n'est pas présente, on s'arrète ici
            //
            break;
        }

        if ( szText[0] != 0 )
        {
            //
            // Le texte n'est pas vide, il s'agit d'un item associé à une
            // commande à exécuter. Il faut donc lire cette commande.
            //
            dwSize = sizeof(gsSvcWork.tszMenuCommands[gsSvcWork.dwMenuCommands]);
            dwErr = REG_Lire_Chaine( HKEY_LOCAL_MACHINE,
                                     szKey,
                                     "Command",
                                     gsSvcWork.tszMenuCommands[gsSvcWork.dwMenuCommands],
                                     &dwSize );
            if ( dwErr != NO_ERROR )
            {
                //
                // La commande n'est pas présente, on s'arrète ici
                //
                break;
            }

            //
            // Ensuite, on prépare les flag de lancement à utiliser avec
            // commande associé à l'item.
            //
            gsSvcWork.tdwMenuFlags[gsSvcWork.dwMenuCommands] = NORMAL_PRIORITY_CLASS;
            dwErr = REG_Defaut_Entier( HKEY_LOCAL_MACHINE,
                                     szKey,
                                     "NoConsole",
                                     &dwVal,
                                     0 );
            if ( ( dwErr == NO_ERROR ) && ( dwVal != 0 ) )
                gsSvcWork.tdwMenuFlags[gsSvcWork.dwMenuCommands] |= CREATE_NO_WINDOW;
            else
                gsSvcWork.tdwMenuFlags[gsSvcWork.dwMenuCommands] |= CREATE_NEW_CONSOLE;
                                   
            //
            // Enfin, on ajout l'élément dans le menu.
            //
            NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 
                                     gsSvcWork.dwMenuCommands + CUSTOM_ITEM_BASE_ID, 
                                     szText );
        }
        else
        {
            //
            // Le texte est vide, il s'agit donc d'un séparateur.
            //
            gsSvcWork.tszMenuCommands[gsSvcWork.dwMenuCommands][0] = 0;
            NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 
                                     gsSvcWork.dwMenuCommands + CUSTOM_ITEM_BASE_ID, 
                                     NULL );
        }

        //
        // Et un élément de plus, un !
        //
        gsSvcWork.dwMenuCommands ++;
    }

    //
    // Ajouter ici les menus spécifiques au service (non customisables)
    //
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI MainUnloadMenu()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Déchargement des menus
 * --------------------------------------------------------------------
 */
PRIVATE void WINAPI MainUnloadMenu()
{
    //
    // Vider la liste
    //
    NTSVCDefineTrayIconMenu( NTSVC_MENU_CLEAR_ALL, 
                             0, 
                             NULL );

    //
    // Désactiver le handler.
    //
    NTSVCSetTrayIconHandler( NULL );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI MainTrayIconHandler( UINT uiId )
 * PARAMETERS: uiId : Identifiant de l'événement de notification.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Handler des événements de notification.
 * --------------------------------------------------------------------
 */
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId )
{
    STARTUPINFO sInfo;
    PROCESS_INFORMATION sProcess;
    BOOL fResult;
    DWORD dwIndex;

    //
    // Tester si l'id de l'item correspond à un id customisé.
    //
    if ( ( uiId >= CUSTOM_ITEM_BASE_ID ) && 
         ( uiId < ( CUSTOM_ITEM_BASE_ID + gsSvcWork.dwMenuCommands ) ) )
    {
        //
        // C'est un item customisé, on en détermine son index.
        //
        dwIndex = uiId-CUSTOM_ITEM_BASE_ID;

        //
        // On n'a plus qu'à lancer la commande sélectionnée par le menu
        //
        ZeroMemory( &sInfo, sizeof(sInfo) );
        sInfo.cb = sizeof( sInfo );
        fResult = CreateProcess( NULL,
                                 gsSvcWork.tszMenuCommands[dwIndex],
                                 NULL,
                                 NULL,
                                 FALSE,
                                 gsSvcWork.tdwMenuFlags[dwIndex],
                                 NULL,
                                 NULL,
                                 &sInfo,
                                 &sProcess );
        if ( fResult )
        {
            //
            // Pour éviter une consommation de handles, on 
            // les ferme immédiatement.
            //
            CloseHandle( sProcess.hThread );
            CloseHandle( sProcess.hProcess );
        }
    }

    else
    {
        //
        // Tout menu spécifique (non customisable) au service doit être géré ici
        //
    }
}





