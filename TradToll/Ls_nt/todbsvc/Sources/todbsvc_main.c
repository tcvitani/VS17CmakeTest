/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_main.c
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
#include <trc.h>
#include <col.h>
#include <csr_list.h>
#include <csr_msg.h>
#include <msg_sv_con_rep.h>
#include <msg_lc_perm_rq_send_file.h>
#include <reg.h>

#include <resource.h>

#define LOC_DEF
#include <todbsvc_glob.h>
#undef LOC_DEF

#include <todbsvc_text.h>
#include <todbsvc_files.h>
#include <todbsvc_callback.h>
#include <todbsvc_lane.h>
#include <todbsvc_db.h>
#include <todbsvc_bak.h>

#define LOC_DEF
#include <todbsvc_main.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE void MainTreatment();
PRIVATE DWORD MainContextual( TODBSVC_LANE * psLane, DWORD dwValue );
PRIVATE DWORD MainPoolThread( void * pvFoo );
PRIVATE DWORD MainNalThread( void * pvFoo );
PRIVATE DWORD MainInitMessages();
PRIVATE void MainTerminateMessages();
PRIVATE void MainPurgeRejectDirectory();
PRIVATE void WINAPI MainLoadMenu();
PRIVATE void WINAPI MainUnloadMenu();
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId );


PRIVATE BOOL gbBlink = FALSE;


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
    (*ppfCommand) = (NTSVCCommandMain*)BakMain;
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

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" TODBSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Définition des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            TODBSVC_REG_VAL_CMDSVC        , REG_DWORD,        4,                  1, &gsSvcWork.sParmCopy.dwCmdSvc,
            TODBSVC_REG_VAL_MAXLANES      , REG_DWORD,        4,                 64, &gsSvcWork.sParmCopy.dwMaxLanes,
            TODBSVC_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,                  5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            TODBSVC_REG_VAL_WORKERS       , REG_DWORD,        4,                  2, &gsSvcWork.sParmCopy.dwWorkers,
            TODBSVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4,               1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            TODBSVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4,                256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            TODBSVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,              16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            TODBSVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,              16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            TODBSVC_REG_VAL_MAXLIFE       , REG_DWORD,        4,                  5, &gsSvcWork.sParmCopy.dwMaxLife,
            TODBSVC_REG_VAL_LIFETIME      , REG_DWORD,        4,               5000, &gsSvcWork.sParmCopy.dwLifeTime,
            TODBSVC_REG_VAL_MAINPOLLING   , REG_DWORD,        4,            3600000, &gsSvcWork.sParmCopy.dwMainPolling,
            TODBSVC_REG_VAL_ERRORSLEEP    , REG_DWORD,        4,              10000, &gsSvcWork.sParmCopy.dwErrorSleep,
            TODBSVC_REG_VAL_SENDAGAIN     , REG_DWORD,        4,              60000, &gsSvcWork.sParmCopy.dwSendAgain,
            TODBSVC_REG_VAL_POOLSCANDELAY , REG_DWORD,        4,              10000, &gsSvcWork.sParmCopy.dwPoolScanDelay,
            TODBSVC_REG_VAL_MAXFILES      , REG_DWORD,        4,                 16, &gsSvcWork.sParmCopy.dwMaxFiles,
            TODBSVC_REG_VAL_MAXFILEMSGSIZE, REG_DWORD,        4,              16384, &gsSvcWork.sParmCopy.dwMaxFileMsgSize,
            TODBSVC_REG_VAL_MAXFILERETRY  , REG_DWORD,        4,                 32, &gsSvcWork.sParmCopy.dwMaxFileRetry,
            TODBSVC_REG_VAL_LANESFILEONLY , REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwLanesFileOnly,
            TODBSVC_REG_VAL_CYCLELOW      , REG_DWORD,        4,              99999, &gsSvcWork.sParmCopy.dwCycleLow,
            TODBSVC_REG_VAL_CYCLEHIGH     , REG_DWORD,        4,           99900000, &gsSvcWork.sParmCopy.dwCycleHigh,
            TODBSVC_REG_VAL_ONLYBACKUP    , REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwOnlyBackup,
            TODBSVC_REG_VAL_ACCEPTNALFILES, REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwAcceptNALFiles,
            TODBSVC_REG_VAL_MAXNALERRORS  , REG_DWORD,        4,                 32, &gsSvcWork.sParmCopy.dwMaxNALErrors,
            TODBSVC_REG_VAL_BASELANEDIR   , REG_SZ   , MAX_PATH,     "C:\\MSGFILES", &gsSvcWork.sParmCopy.szBaseLaneDir,
            TODBSVC_REG_VAL_REJECTLANEDIR , REG_SZ   , MAX_PATH,"C:\\MSGFILES\\ERR", &gsSvcWork.sParmCopy.szRejectLaneDir,
            TODBSVC_REG_VAL_REJECTKBQUOTA , REG_DWORD,        4,              10000, &gsSvcWork.sParmCopy.dwRejectKBQuota,
            TODBSVC_REG_VAL_TCPROUTE      , REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwTCPRoute,
            TODBSVC_REG_VAL_DBPOOLCOUNT   , REG_DWORD,        4,                  1, &gsSvcWork.sParmCopy.dwDbPoolCount,
            TODBSVC_REG_VAL_APP_MSG_ID    , REG_DWORD,        4,                110, &gsSvcWork.sParmCopy.dwAppMsgId,
            TODBSVC_REG_VAL_DBUSR         , REG_SZ   , MAX_PATH,          "comuser", &gsSvcWork.sParmCopy.szDbUsr,
            TODBSVC_REG_VAL_DBPWD         , REG_SZ   , MAX_PATH,           "compwd", &gsSvcWork.sParmCopy.szDbPwd,
            TODBSVC_REG_VAL_DBINST        , REG_SZ   , MAX_PATH,              "pcs", &gsSvcWork.sParmCopy.szDbInst,
            TODBSVC_REG_VAL_ROUTESERVER   , REG_SZ   , MAX_PATH,                ".", &gsSvcWork.sParmCopy.szRouteServer,
            TODBSVC_REG_VAL_LANESFILE     , REG_SZ   , MAX_PATH,   "C:\\TODBLN.TXT", &gsSvcWork.sParmCopy.szLanesFile,
            TODBSVC_REG_VAL_MSGFILESTRACE , REG_SZ   , MAX_PATH,"C:\\TODBFILES.LOG", &gsSvcWork.sParmCopy.szMsgFilesTrace,
            TODBSVC_REG_VAL_MSGFILESTRACESIZE, REG_DWORD,     4,           10000000, &gsSvcWork.sParmCopy.dwMsgFilesTraceSize,
            TODBSVC_REG_VAL_CONTEXT_ID    , REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwContextId,
            TODBSVC_REG_VAL_AUTOCOPYDIR   , REG_SZ   , MAX_PATH,                 "", &gsSvcWork.sParmCopy.szAutoCopyDir,
            TODBSVC_REG_VAL_DEADLOCKDELAY , REG_DWORD,        4,                  0, &gsSvcWork.sParmCopy.dwDeadlockDelay,
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TODBSVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TODBSVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TODBSVC_OK_RUN ) );

    // Boucler sur démarré / en pause -> rechargement des paramètre à chaque boucle
    do 
    {
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

        gsSvcWork.fLicenceIsValid = TRUE;

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

        MainLoadMenu();

        // Les traitements ont été lancés avec les workers

        NTSVCInfo( "NTSVCExternalMain(), démarrage de la boucle principale" );
        // Attendre la demande d'arrêt tout en gerant l'interface avec la
        // base de données
        do
        {
            gbBlink = ! gbBlink;
            if ( gsSvcWork.fLicenceIsValid )
                NTSVCRefreshTrayIcon( gbBlink ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );
            else
                NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INVALID_LICENCE" );

            dwWait = NTSVCWaitForEnd( 1000 );
            if ( dwWait == WAIT_OBJECT_0 ) break;

            // Déclencher le traitement périodique
            MainTreatment();

            // Attendre la demande de fin.
            // L'écoulement du délai de polling de la base de données a déjà eu lieu dans MainTreatment()
            dwWait = NTSVCWaitForEndOrMultipleObjects(
                        1, 
                        &gsSvcWork.hWakeupEvent, 
                        0 );
        }
        while ( ( dwWait == ( WAIT_OBJECT_0 + 1 ) ) || ( dwWait == WAIT_TIMEOUT ) );

        MainUnloadMenu();

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

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
    gsSvcWork.llSendAgain = ((LONGLONG)10000)*((LONGLONG)gsSvcWork.sParmWork.dwSendAgain);
    gsSvcWork.llPoolScanDelay = ((LONGLONG)10000)*((LONGLONG)gsSvcWork.sParmWork.dwPoolScanDelay);

    // Initialiser l'évènement chargé de réveiller la boucle principale
    gsSvcWork.hWakeupEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( gsSvcWork.hWakeupEvent == NULL )
    {
        dwErr = GetLastError();
        SVC_ERR( dwErr, "ERR_INIT_WAKEUP_EVENT" );
        return dwErr;
    }

    // Initialiser la trace des fichiers traites
    TRC_Initialise_Trace( 
        "TODBFILES", 
        gsSvcWork.sParmWork.szMsgFilesTrace, 
        TRC_OPT_FICHIER | TRC_OPT_IMMEDIAT | TRC_OPT_CREER_FICHIER,
        &gsSvcWork.hTrc );
    TRC_Taille_Max_Fichier(
        gsSvcWork.hTrc,
        gsSvcWork.sParmWork.dwMsgFilesTraceSize );

    // Préparation de la tables des voies
    gsSvcWork.hLanes = ColCreate( COL_INDEX_BINARY, sizeof( TODBSVC_LANE_ID ), TRUE, TRUE );
    if ( gsSvcWork.hLanes == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        SVC_ERR( dwErr, "ERR_INIT_LC_COL" );
        CloseHandle( gsSvcWork.hWakeupEvent );
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

    TRC_Termine_Trace( gsSvcWork.hTrc ); 

    // Fermeture de la tables des voies
    ColDestroy( gsSvcWork.hLanes );

    CloseHandle( gsSvcWork.hWakeupEvent );

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
    // Ajout du pipe dans la liste des null session
    AComSetNullSessionPipe( TODBSVC_PIPE_CMD, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.sParmWork.dwMaxCmdCnx + ( gsSvcWork.sParmWork.dwOnlyBackup ? 0 : 1 ),
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
                    TODBSVC_PIPE_CMD,
                    TODBSVC_PIPE_CMD_TYPE,
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
                    TODBSVC_PIPE_CMD_TYPE,                      // dwInstUsrKey
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
            AComCloseWorkingSet( gsSvcWork.hWks, TODBSVC_WORKINGSET_TIMEOUT );
            return ERROR_INVALID_DATA;
        }
    }

    // MISE EN PLACE DE LA CONNEXION AVEC LA COM

    if ( ! gsSvcWork.sParmWork.dwOnlyBackup )
    {
        NTSVCInfo( "MainInitPipes(), création du client" );
        sprintf_s( szPipe, sizeof(szPipe), TODBSVC_PIPE_COM, gsSvcWork.sParmWork.szRouteServer );
        if ( gsSvcWork.sParmWork.dwTCPRoute == 0 )
            gsSvcWork.hComInst = AComOpenPipeClientInstance(
                    gsSvcWork.hWks,
                    szPipe,
                    TODBSVC_PIPE_COM_TYPE,
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
                    TODBSVC_PIPE_COM_TYPE,                    // dwInstUsrKey
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
            AComCloseWorkingSet( gsSvcWork.hWks, TODBSVC_WORKINGSET_TIMEOUT );
            return ERROR_INVALID_DATA;
        }
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

    AComCloseWorkingSet( gsSvcWork.hWks, TODBSVC_WORKINGSET_TIMEOUT );

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
    DWORD dwErr = NO_ERROR; // Récupération des codes d'erreur
    HANDLE * phThreads;     // Tableau des handles de threads constituant le pool
    DWORD dwThreadCount;
    DWORD dwIndex;          // Pour parcourir le tableau des threads
    DWORD dwStat;           // Status du code de retour des threads
    DWORD dwRes;            // Code de retour d'appel de fonction
    DWORD dwId;             // Id de thread
    DWORD dwTickStart;
    DWORD dwTick;
    DWORD dwWait;

    // La boucle do while est utilisée uniquement par commodité, la condition de
    // bouclage étant FALSE, elle n'est parcourue qu'une fois. Par contre, un
    // break permet de faire un saut à la fin.
    do
    {
        dwThreadCount = gsSvcWork.sParmWork.dwDbPoolCount + 
                        ( gsSvcWork.sParmWork.dwAcceptNALFiles ? 1 : 0 );

        // Allouer un tableau capable de récupérer la totalité des handles de
        // threads de mise en base (pool)
        phThreads = HeapAlloc( 
                GetProcessHeap(), 
                HEAP_ZERO_MEMORY, 
                sizeof(*phThreads) * dwThreadCount );
        if ( phThreads == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            NTSVCInfo( "MainTreatment(), erreur %u, d'allouer la table des threads", ERROR_NOT_ENOUGH_MEMORY );
            break;
        }

        // Mettre à jour la liste des voies
        if ( ( dwErr = DBGetLanes() ) != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_GET_LANES" );
            break;
        }

        // Déconnexion (si connecté) du service de com pour provoquer une reconnexion et 
        // une interrogation sur l'état de connexion des voies
        AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.hComCnx, FALSE );

        // RAZ du flag d'arret des threads du pool
        gsSvcWork.bTerminatePool = FALSE;

        // Pour chaque élément du pool
        for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwDbPoolCount ; dwIndex ++ )
        {
            char szTName[48];

            sprintf_s( szTName, sizeof(szTName), "MainPoolThread[%u]", dwIndex );
            // Créer le thread suspendu. Ils seront réactivés lorsqu'ils
            // auront tous été créés
            phThreads[dwIndex] = NTSVCCreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE)MainPoolThread,
                    NULL,
                    CREATE_SUSPENDED,
                    &dwId,
                    szTName );
            if ( phThreads[dwIndex] == NULL )
            {
                dwErr = GetLastError();
                break;
            }
        }
        // En cas d'erreur, on ne va pas plus loin
        if ( dwErr != NO_ERROR )
            break;

        if ( gsSvcWork.sParmWork.dwAcceptNALFiles )
        {
            phThreads[gsSvcWork.sParmWork.dwDbPoolCount] = NTSVCCreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE)MainNalThread,
                    NULL,
                    CREATE_SUSPENDED,
                    &dwId,
                    "MainNalThread" );
            if ( phThreads[gsSvcWork.sParmWork.dwDbPoolCount] == NULL )
            {
                dwErr = GetLastError();
                break;
            }
        }

        // Réactiver les threads du pool qui viennent d'être créés
        for ( dwIndex = 0 ; dwIndex < dwThreadCount ; dwIndex ++ )
            ResumeThread( phThreads[dwIndex] );

        MainPurgeRejectDirectory();

        dwTickStart = GetTickCount();
        while ( TRUE )
        {
            dwTick = GetTickCount();
            dwWait = dwTick - dwTickStart;
            if ( dwWait >= gsSvcWork.sParmWork.dwMainPolling ) { dwRes = WAIT_TIMEOUT; break; }

            gbBlink = !gbBlink;
            if ( gsSvcWork.fLicenceIsValid )
                NTSVCRefreshTrayIcon( gbBlink ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );
            else
                NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INVALID_LICENCE" );

            // Attendre la fin d'un des threads ou une demande d'arret du service
            dwRes = NTSVCWaitForEndOrMultipleObjects( 
                    dwThreadCount, 
                    phThreads, 
                    1000 );
            if ( dwRes != WAIT_TIMEOUT ) break;
        }

        // S'il s'agit de l'arrêt d'un thread
        if ( ( dwRes > WAIT_OBJECT_0 ) && 
             ( dwRes <= ( WAIT_OBJECT_0 + dwThreadCount ) ) && 
             ! gsSvcWork.bTerminatePool )
        {
            dwIndex = dwRes - WAIT_OBJECT_0 - 1;
            dwRes = GetExitCodeThread( phThreads[dwIndex], &dwStat );
            SVC_ERR( dwStat, "ERR_POOL_UNEXPECTED" );
            dwErr = ERROR_INVALID_DATA;
            break;
        }

        NTSVCInfo( "MainTreatment(), fin du tour, attente de l'arrêt des pool db" );

        // Activer le flag d'arret du pool si pas déjà activé
        gsSvcWork.bTerminatePool = TRUE;

        // Attendre que tout soit arrété
        dwRes = WaitForMultipleObjects( dwThreadCount, 
                                        phThreads, 
                                        TRUE, 
                                        gsSvcWork.sParmWork.dwDeadlockDelay == 0 ? 
                                            INFINITE : 
                                            gsSvcWork.sParmWork.dwDeadlockDelay );
        if ( ( dwRes == WAIT_TIMEOUT ) || ( dwRes == WAIT_FAILED ) )
        {
            SVC_ERR( dwRes, "ERR_POTENTIAL_DB_DEADLOCK" );
            RaiseException( 0xE0000002, EXCEPTION_NONCONTINUABLE, 0, NULL );
        }

        NTSVCInfo( "MainTreatment(), fin du tour, pool db arrétés" );
    }
    while ( FALSE );

    // Si le tableau des threads a été créé
    if ( phThreads != NULL )
    {
        // Pour chaque élément du tableau, si un thread a été créé,
        // forcer la fin du thread et fermer le handle lié
        for ( dwIndex = 0 ; dwIndex < dwThreadCount ; dwIndex ++ )
            if ( phThreads[dwIndex] != NULL )
            {
                TerminateThread( phThreads[dwIndex], 0 );
                CloseHandle( phThreads[dwIndex] );
            }

        // Libérer la mémoire allouée pour le tableau de handles
        HeapFree( GetProcessHeap(), 0, phThreads );
    }

    // En cas d'erreur, on marque une pause
    if ( dwErr != NO_ERROR ) 
    {
        NTSVCWaitForEnd( gsSvcWork.sParmWork.dwErrorSleep > 1000 ? gsSvcWork.sParmWork.dwErrorSleep - 1000 : 0 );
        gbBlink = !gbBlink;
        NTSVCRefreshTrayIcon( gbBlink ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );
        NTSVCWaitForEnd( 1000 );
    }
}


#define NEXT_SEQ(x) (((x)%99999999)+1)
#define PREV_SEQ(x) (((x)==1)?99999999:(x)-1)
#define HIGH_CONTEXT(x) ((x)>gsSvcWork.sParmWork.dwCycleHigh)
#define LOW_CONTEXT(x) ((x)<=gsSvcWork.sParmWork.dwCycleLow)

PRIVATE DWORD MainContextual( TODBSVC_LANE * psLane, DWORD dwValue )
{
    dwValue += 100000000;

    if ( HIGH_CONTEXT( psLane->dwLastTreated ) && ( LOW_CONTEXT( dwValue ) ) )
        dwValue = dwValue + 99999999;

    else if ( LOW_CONTEXT( psLane->dwLastTreated ) && ( HIGH_CONTEXT( dwValue ) ) )
        dwValue = dwValue - 99999999;

    return dwValue;
}

PRIVATE DWORD MainPoolThread( void * pvFoo )
{
    HCOLLECTIONITEM hItem;          // Pour scanner la collection des voies
    TODBSVC_LANE * psLane;          // Pointe sur la voie connecté
    TODBSVC_LANE * psScan;          // En complément de hItem pour scanner les voies
    DWORD dwCount;                  // Nombre de fichiers dans la collection
    DWORD dwIndex;                  // Index du fichier courant dans la liste des fichiers
    char szMask[MAX_PATH];          // Mask sur les nombre de fichiers à énumérer
    char szFullFile[MAX_PATH];      // Chemin complet du fichier courant
    char szCopyFile[MAX_PATH];      // Chemin complet du fichier pour la copie automatique
    char szText[32];
    FILE_LIST_ITEM * psList;        // Liste de fichiers vérifiant le masque
    DWORD dwFile;                   // Numéro de séquence du fichier courant
    DWORD dwNextPresent;            // Numéro de séquence de la borne supérieur du trou
    DWORD dwErr;                    // Code d'erreur niveau 1
    DWORD dwErr2;                   // Code d'erreur niveau 2
    DB_CNX * hDbCnx = NULL;         // Handle de connexion à la base de données
    DWORD dwTreatCount;             // Nombre de fichiers traités par la boucle
    LONGLONG llNow;                 // Date courante
    BOOL bRemain;                   // Indique s'il reste encore des fichiers à traiter
    BOOL bResult;                   // Resultat d'appel
 
    // La boucle ne se termine que par commande du flag d'arret des pools
    while ( ! gsSvcWork.bTerminatePool )
    {
        // CONNEXION A LA BASE DE DONNEES
        if ( hDbCnx == NULL )
        {
            NTSVCInfo( "MainPoolThread(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
            hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
            gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
            if ( hDbCnx == NULL )
            {
                NTSVCInfo( "MainPoolThread(), connexion avec la base impossible" );
                Sleep( gsSvcWork.sParmWork.dwErrorSleep );
                continue;
            }
        }

        // RECHERCHE D'UNE VOIE A TRAITER
        NTSVCInfo( "MainPoolThread(), recherche d'une voie à traiter" );

        // Rechercher la voie pour laquelle les traitements sont les plus anciens
        // et non utilisée par un pool
        // On accède à "bUsedByPool" qui peut être utilisé par un autre thread
        // du pool de traitement, on utiliser donc le vérou.
        ColLock( gsSvcWork.hLanes );
        hItem = COL_SCAN_BEGIN;
        psLane = NULL;
        GetSystemTimeAsFileTime( (LPFILETIME)&llNow );
        while ( ColItemScan( gsSvcWork.hLanes, &hItem ) )
        {
            psScan = ColItemData( hItem );

            // Si on a eu un retour arrière de plus de 2 secondes au niveau de l'heure, 
            // on fait en sorte que le traitement soit déclenché.
            if ( ( llNow - psScan->llLastDone ) < (LONGLONG)( -2 * 10000000 ) )
            {
                sprintf_s( szText, sizeof(szText), "PZ%04u-LN%04u", psScan->sId.dwPlaza, psScan->sId.dwLane  );
                SVC_ERR_S( dwErr, "ERR_BACK_TIME", szText );
                psScan->llLastBackup = 0;
                psScan->llLastDone = 0;
            }

            if ( ( ! psScan->bUsedByPool                                      ) && 
                 ( ! psScan->bSuspended                                       ) && 
                 ( ( llNow - psScan->llLastDone ) > gsSvcWork.llPoolScanDelay ) )
            {
                if ( psLane == NULL )
                    psLane = psScan;
                else if ( psScan->llLastDone < psLane->llLastDone )
                    psLane = psScan;
            }
        }
        if ( psLane != NULL )
        {
            psLane->bUsedByPool = TRUE;
            psLane->llLastDone = llNow;
        }
        ColUnlock( gsSvcWork.hLanes );
        
        // Pas de voie trouvée, on attend un peu et on réessaie
        if ( psLane == NULL )
        {
            NTSVCInfo( "MainPoolThread(), aucun traitement nécessaire" );
            Sleep( 1000 );
            continue;
        }

        NTSVCInfo( "MainPoolThread(), voie sélectionnées PZ%04u-LN%04u", psLane->sId.dwPlaza, psLane->sId.dwLane );

        // SAUVERGARDE DE DERNIER NUMERO TRAITE SI DEMANDE
        if ( psLane->bSaveLastTreated )
        {
            NTSVCInfo( "MainPoolThread(), mise à jour du dernier fichier traité demandé (%u) pour PZ%04u-LN%04u", psLane->dwLastTreated, psLane->sId.dwPlaza, psLane->sId.dwLane );
            psLane->bSaveLastTreated = FALSE;
            if ( ( dwErr = DBSetLastTreatedNumber( &hDbCnx, psLane ) ) != NO_ERROR )
            {
                // On accède à "bUsedByPool" qui peut être utilisé par un autre thread
                // du pool de traitement, on utiliser donc le vérou.
                ColLock( gsSvcWork.hLanes );
                psLane->bUsedByPool = FALSE;
                ColUnlock( gsSvcWork.hLanes );
                SVC_ERR( dwErr, "ERR_UPDATE_SEQ" );
                continue;
            }
        }

        // RECUPERATION LISTE DES FICHIERS CONSERNANT LA VOIE SELECTIONNEE

        // Préparer le masque pour les fichiers backup
        if ( gsSvcWork.sParmWork.dwOnlyBackup )
        {
            sprintf_s( szMask, sizeof(szMask), "???.PCS%04u.L%04u.S????????", psLane->sId.dwPlaza, psLane->sId.dwLane );
        }
        else
        {
            sprintf_s( szMask, sizeof(szMask), "BAK.PCS%04u.L%04u.S????????", psLane->sId.dwPlaza, psLane->sId.dwLane );
        }
        // Récupérer les fichiers classés par ordre croissant
        NTSVCInfo( "MainPoolThread(), scan des fichiers dans [%s] correspondant à [%s]", gsSvcWork.sParmWork.szBaseLaneDir, szMask );
        psList = FileListOpen( gsSvcWork.sParmWork.szBaseLaneDir, szMask, &dwCount );
        for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
        {
            // Récupérer le nom du fichier et composer son chemin d'accès complet
            _snprintf_s( szFullFile, _countof(szFullFile), sizeof(szFullFile), "%s\\%s", gsSvcWork.sParmWork.szBaseLaneDir, psList[dwIndex] );
            szFullFile[sizeof(szFullFile)-1] = '\0';

            // Si la fonction autocopy est active (chaine non vide pour le répertoire de
            // destination de l'autocopy).
            if ( gsSvcWork.sParmWork.szAutoCopyDir[0] != 0 )
            {
                // Composer le nom complet de la destination pour l'autocopie
                _snprintf_s( szCopyFile, _countof(szCopyFile), sizeof(szCopyFile), "%s\\%s", gsSvcWork.sParmWork.szAutoCopyDir, psList[dwIndex] );
                szCopyFile[sizeof(szCopyFile)-1] = '\0';

                CopyFile( szFullFile, szCopyFile, TRUE );
            }

            if ( gsSvcWork.sParmWork.dwOnlyBackup )
            {
                if ( ( _stricmp( psList[dwIndex], "PCS" ) != 0 ) &&
                     ( _stricmp( psList[dwIndex], "BAK" ) != 0 )
                   )
                continue;
            }

            NTSVCInfo( "MainPoolThread(), Intégration fichier backup [%s]", szFullFile );
            dwErr = DBIncorporate( &hDbCnx, szFullFile, psLane, NULL, DB_FILE_BACKUP );
            if ( dwErr != NO_ERROR )
            {
                SVC_ERR( dwErr, "ERR_INCORPORATE_FILE" );
            }
            else
            {
                NTSVCError( "AVERTISSEMENT : Fichier de backup [%s] intégré !", szFullFile );

                // Effacer le fichier
                DeleteFile( szFullFile );
            }
        }

        // On a plus besoin de la liste des fichiers
        if ( psList != NULL )
            FileListClose( psList );

        if ( gsSvcWork.sParmWork.dwOnlyBackup )
            continue;

        // RECUPERATION DES INFOS COURANTES CONSERNANT LA VOIE SELECTIONNEE

        // Préparer le masque pour les fichiers courants
        sprintf_s( szMask, sizeof(szMask), "MSG.PCS%04u.L%04u.S????????", psLane->sId.dwPlaza, psLane->sId.dwLane );
        // Récupérer les fichiers classés par ordre croissant
        NTSVCInfo( "MainPoolThread(), scanning files in [%s] matching [%s]", gsSvcWork.sParmWork.szBaseLaneDir, szMask );
        psList = FileListOpen( gsSvcWork.sParmWork.szBaseLaneDir, szMask, &dwCount );

        // Rappatrier le dernier numéro de séquence traité si on ne le connait pas
        if ( psLane->dwLastTreated == 0 )
        {
            dwErr = DBGetLastTreatedNumber( &hDbCnx, psLane );
            if ( dwErr != NO_ERROR )
            {
                // On accède à "bUsedByPool" qui peut être utilisé par un autre thread
                // du pool de traitement, on utiliser donc le vérou.
                ColLock( gsSvcWork.hLanes );
                psLane->bUsedByPool = FALSE;
                ColUnlock( gsSvcWork.hLanes );
                SVC_ERR( dwErr, "ERR_GET_SEQ" );
                continue;
            }
        }

        // TRAITEMENT DES FICHIERS

        // Initialiser les indicateurs de la boucle
        dwNextPresent = 0xFFFFFFFF;     // Limite supérieur du trou détecté
        dwTreatCount = 0;               // Nombre des fichier effectivement insérés en base
        bRemain = FALSE;                // Par défaut, aucun fichier ne reste à traiter

        // Scanner la liste des fichiers
        // Les conditions d'arret de la boucle sont : fin de liste ou nombre maximum de fichiers
        // traités en un coup atteint.
        for ( dwIndex = 0 ; ( dwIndex < dwCount ) && ( dwTreatCount < gsSvcWork.sParmWork.dwMaxFiles ) && ( ! gsSvcWork.bTerminatePool ) ; dwIndex ++ )
        {
            NTSVCInfo( "MainPoolThread(), scan file : Filename         = %s", psList[dwIndex] );

            // Récupérer le nom du fichier et composer son chemin d'accès complet
            _snprintf_s( szFullFile, _countof(szFullFile), sizeof(szFullFile), "%s\\%s", gsSvcWork.sParmWork.szBaseLaneDir, psList[dwIndex] );
            szFullFile[sizeof(szFullFile)-1] = '\0';

            // Si la fonction autocopy est active (chaine non vide pour le répertoire de
            // destination de l'autocopy).
            if ( gsSvcWork.sParmWork.szAutoCopyDir[0] != 0 )
            {
                // Composer le nom complet de la destination pour l'autocopie
                _snprintf_s( szCopyFile, _countof(szCopyFile), sizeof(szCopyFile), "%s\\%s", gsSvcWork.sParmWork.szAutoCopyDir, psList[dwIndex] );
                szCopyFile[sizeof(szCopyFile)-1] = '\0';

                CopyFile( szFullFile, szCopyFile, TRUE );
            }

			_strupr_s( psList[dwIndex], MAX_PATH );
            // Décomposer le nom de manière à obtenir le numéro de séquence
            // On le converti en majuscules pour faciliter l'extraction du numéro
			bResult = FileSplitName( psList[dwIndex], &dwFile );

            // CAS 1 : Le fichier ne vérifier pas la syntaxe
            //         Le fichier doit être purgé
            if ( ! bResult )
            {
                DeleteFile( szFullFile );
                SVC_ERR( ERROR_INVALID_DATA, "ERR_INVALID_FILE_NAME" );
                // On ne sort pas de la boucle for(), on passe aux fichiers suivants
            }

            // CAS 2 : Le numéro de séquence du fichier est un numéro déjà traité
            //         Le fichier doit être purgé
            // Si on est pas en contexte de bouclage, le fichier est considéré comme déja traité si
            // le numero du fichier est inférieur ou égale au dernier traité.
            // Si on est en contexte de bouclage, le fichier est considéré comme déja traité si le numéro
            // du fichier est inférieur ou égale au dernier traité ET s'il est supérieur à la limite basse
            // du bouclage.
            else if ( MainContextual( psLane, dwFile ) <= MainContextual( psLane, psLane->dwLastTreated ) )
            {
                NTSVCInfo( "MainPoolThread(), le fichier [%s] a déjà été incorporé, il va être purgé", szFullFile );
                DeleteFile( szFullFile );
                // On ne sort pas de la boucle for(), on passe aux fichiers suivants
            }

            // CAS 3 : Le numéro de séquence du fichier est celui attendu ou on est en cold start
            //         Les données du fichier doivent être intégrées à la base
            else if ( ( NEXT_SEQ( psLane->dwLastTreated ) == dwFile ) || 
                      psLane->bRequestToSend 
                    )
            {
                NTSVCInfo( "MainPoolThread(), le fichier [%s] est prèt à être incorporé%s", szFullFile, psLane->bRequestToSend ? " en mode COLDSTART" : "" );

                // Il reste des fichiers à traiter
                bRemain = TRUE;
                
                // On vérifie que le fichier est accessible en exclusif
                if ( ! FileAccessCheck( szFullFile ) )
                {
                    NTSVCInfo( "MainPoolThread(), le fichier [%s] est vérouillé, il sera traité plus tard", szFullFile );
                    // Inutile de continuer
                    break;
                }

                // Incorporer les données du fichier dans la base de données
                
                dwErr = DBIncorporate( 
                    &hDbCnx, 
                    szFullFile, 
                    psLane, 
                    NULL, 
                    psLane->dwRetry >= gsSvcWork.sParmWork.dwMaxFileRetry ? DB_FILE_LAST_TRY : 0 );
                
                // Compter une tentative supplémentaire
                psLane->dwRetry ++;

                if ( dwErr != NO_ERROR )
                {
                    // Si l'erreur provient de la base, annuler l'incrément de la tentative
                    if ( hDbCnx == NULL )
                        psLane->dwRetry --;

                    TRC_Direct_Trace_Texte( gsSvcWork.hTrc, TRC_OPT_MASK, "ERR ; PZ%04u-LN%04u ; %08u ; %s", psLane->sId.dwPlaza, psLane->sId.dwLane, dwFile, szFullFile );
                    SVC_ERR( dwErr, "ERR_INCORPORATE_FILE" );
                    // Inutile de continuer
                    break;
                }


                TRC_Direct_Trace_Texte( gsSvcWork.hTrc, TRC_OPT_MASK, "OK  ; PZ%04u-LN%04u ; %08u ; %s", psLane->sId.dwPlaza, psLane->sId.dwLane, dwFile, szFullFile );

                // Mettre à jour le numéro de séquence de la voie en base
                psLane->dwLastTreated = dwFile;
                if ( ( dwErr = DBSetLastTreatedNumber( &hDbCnx, psLane ) ) != NO_ERROR )
                {
                    SVC_ERR( dwErr, "ERR_UPDATE_SEQ" );
                    // Inutile de continuer
                    break;
                }

                // Effacer le fichier
                DeleteFile( szFullFile );

                // Réinitialiser le nombre de tentatives
                psLane->dwRetry = 0;

                // Un fichier traité de plus
                dwTreatCount++;

                // On ne sort pas de la boucle for(), on passe aux fichiers suivants
            }

            // CAS 4 : Le numéro de séquence du fichier est après celui attendu
            //         Il s'agit d'un trou
            //         On n'effectue aucun traitement en attendant que ce soit son tour
            else
            {
                // Il est inutile d'aller plus loin
                NTSVCInfo( "MainPoolThread(), détection d'un trou avant le fichier [%s]", szFullFile );

                // Il reste des fichiers à traiter
                bRemain = TRUE; 

                // Mémoriser la borne supérieure du trou :
                // On ne mémorise que si aucune borne de trou n'est définie
                // ou la nouvelle borne de trou est inférieure à celle mémorisée.
                if ( ( dwNextPresent == 0xFFFFFFFF ) || ( MainContextual( psLane, dwFile ) < MainContextual( psLane, dwNextPresent ) ) )
                    dwNextPresent = dwFile;

                break;
            }
        }

        // On a plus besoin de la liste des fichiers
        if ( psList != NULL )
            FileListClose( psList );

        // Si le nombre de tentatives effectuées sur le fichier courant est trop grand
        if (  psLane->dwRetry > gsSvcWork.sParmWork.dwMaxFileRetry )
        {
            SVC_ERR( ERROR_INVALID_DATA, "ERR_FILE_MAX_ATTEMPTS" );
            // Passer au numéro suivant dans la séquence et enregistrer le nouveau numéro
            psLane->dwLastTreated = NEXT_SEQ( psLane->dwLastTreated );
            if ( ( dwErr2 = DBSetLastTreatedNumber( &hDbCnx, psLane ) ) != NO_ERROR )
            {
                SVC_ERR( dwErr2, "ERR_UPDATE_SEQ" );
                // Tant pis, on réessaiera au prochain tour
                dwErr = ( dwErr == NO_ERROR ? dwErr2 : dwErr );
            }
        }

        // FICHIERS TRAITES, EFFECTUER LA DEMANDE DE BACKUP SI NECESSAIRE

        GetSystemTimeAsFileTime( (LPFILETIME)&llNow );

        // Si une borne de trou existe et si le trou est réel
        if ( ( dwNextPresent != 0xFFFFFFFF ) && ( dwNextPresent != NEXT_SEQ( psLane->dwLastTreated ) ) )
        {

            // En cas de retour arrière, on force un dépassement de temps
            if ( ( llNow - psLane->llLastBackup ) < (LONGLONG)( -2 * 10000000 ) )
            {
                sprintf_s( szText, sizeof(szText), "PZ%04u-LN%04u", psScan->sId.dwPlaza, psScan->sId.dwLane  );
                SVC_ERR_S( dwErr, "ERR_BACK_TIME", szText );
                psLane->llLastBackup = 0;
                psLane->llLastDone = 0;
            }

            // Si temps dépassé ou changement borne supérieure du trou
            if ( ( ( llNow - psLane->llLastBackup ) > gsSvcWork.llSendAgain ) || 
                 ( dwNextPresent != psLane->dwNextPresent ) )
            {
                // On accède à "bConnected" qui peut potentiellement etre modifié par les
                // workers de la com. On effectue un accès vérouillé.
                ColLock( gsSvcWork.hLanes );
                if ( psLane->bConnected )
                {
                    NTSVCInfo( "MainPoolThread(), envoi demande backup à la voie PZ%04u-LN%04u de %u à %u", psLane->sId.dwPlaza, psLane->sId.dwLane, psLane->dwLastTreated + 1, dwNextPresent - 1 );
                    // Envoyer la demande de backup et mémoriser l'heure d'envoi
                    LaneSendBackup( psLane, NEXT_SEQ( psLane->dwLastTreated ), PREV_SEQ( dwNextPresent ) );
                    psLane->llLastBackup = llNow;
                }
                else
                {
                    NTSVCInfo( "MainPoolThread(), lors de sa reconnexion, envoi demande backup à la voie PZ%04u-LN%04u de %u à %u", psLane->sId.dwPlaza, psLane->sId.dwLane, psLane->dwLastTreated + 1, dwNextPresent - 1 );
                    // On force le backup pour la prochaine reconnection
                    psLane->llLastBackup = 0;
                }
                ColUnlock( gsSvcWork.hLanes );
            }
            else
            {
                NTSVCInfo( "MainPoolThread(), trou ignoré pour la voie PZ%04u-LN%04u de %u à %u", psLane->sId.dwPlaza, psLane->sId.dwLane, psLane->dwLastTreated + 1, dwNextPresent - 1 );
            }

            psLane->dwNextPresent = dwNextPresent;
        }

        psLane->llLastDone = llNow;

        // On accède à "bUsedByPool" qui peut être utilisé par un autre thread
        // du pool de traitement, on utiliser donc le vérou.
        ColLock( gsSvcWork.hLanes );
        // Si plus aucun fichier à traiter et demande de permission d'émettre
        if ( ( ! bRemain ) && psLane->bRequestToSend && psLane->bConnected )
        {
            // Passer au numéro attendu = 0
            psLane->dwLastTreated = 0;
            
            ColUnlock( gsSvcWork.hLanes );

            if ( ( dwErr = DBSetLastTreatedNumber( &hDbCnx, psLane ) ) != NO_ERROR )
            {
                SVC_ERR( dwErr, "ERR_UPDATE_SEQ" );
                // Tant pis, on réessaiera au prochain tour
            }
            else
            {
                NTSVCInfo( "MainPoolThread(), autorisation d'émettre envoyée à la voie PZ%04u-LN%04u", psLane->sId.dwPlaza, psLane->sId.dwLane );
                NTSVCError( "AVERTISSEMENT : La voie PZ%04u-LN%04u est désormais autorisée à émettre (fin de démarrage à froid)", psLane->sId.dwPlaza, psLane->sId.dwLane );
                LaneSendOkToSend( psLane );

                ColLock( gsSvcWork.hLanes );
                // Reseter la demande d'émission
                psLane->bRequestToSend = FALSE;
                ColUnlock( gsSvcWork.hLanes );
            }
        }
        else
            ColUnlock( gsSvcWork.hLanes );

        ColLock( gsSvcWork.hLanes );
        psLane->bUsedByPool = FALSE;
        ColUnlock( gsSvcWork.hLanes );

        Sleep( 1000 );

    }

    NTSVCInfo( "MainPoolThread(), arrêt du thread demandé" );

    // Le travail est terminé, on peut se déconnecter de la base
    if ( hDbCnx != NULL ) 
    {
        NTSVCInfo( "MainPoolThread(), déconnexion base de données" );
        DBDisconnect( hDbCnx );
    }

    NTSVCInfo( "MainPoolThread(), fin du thread" );
    ExitThread( 0 );
    return 0;
}





PRIVATE DWORD MainNalThread( void * pvFoo )
{
    DWORD dwIndex;                  // Index du fichier courant dans la liste des fichiers
    DWORD dwErrCount;               // Compte du nombre de fichiers consécutifs en erreur
    DWORD dwCount;                  // Nombre de fichiers dans la collection
    char szMask[MAX_PATH];          // Mask sur les nombre de fichiers à énumérer
    char szFullFile[MAX_PATH];      // Chemin complet du fichier courant
    char szCopyFile[MAX_PATH];      // Chemin complet du fichier pour la copie automatique
    FILE_LIST_ITEM * psList;        // Liste de fichiers vérifiant le masque
    DWORD dwErr;                    // Code d'erreur niveau 1
    DB_CNX * hDbCnx = NULL;         // Handle de connexion à la base de données
 
    // La boucle ne se termine que par commande du flag d'arret des pools
    while ( ! gsSvcWork.bTerminatePool )
    {
        // CONNEXION A LA BASE DE DONNEES
        if ( hDbCnx == NULL )
        {
            NTSVCInfo( "MainNalThread(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
            hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
            gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
            if ( hDbCnx == NULL )
            {
                NTSVCInfo( "MainNalThread(), connexion avec la base impossible" );
                Sleep( gsSvcWork.sParmWork.dwErrorSleep );
                continue;
            }
        }

        // RECUPERATION LISTE DES FICHIERS NAL
        NTSVCInfo( "MainNalThread(), Récupération de la liste des fichiers NAL" );

        // En début de boucle, aucune erreur
        dwErrCount = 0;

        // Préparer le masque pour les fichiers backup
        strcpy_s( szMask, sizeof(szMask), "NAL.PCS????.C????.S????????" );

        // Récupérer les fichiers classés par ordre croissant
        NTSVCInfo( "MainNalThread(), scan des fichiers dans [%s] correspondant à [%s]", gsSvcWork.sParmWork.szBaseLaneDir, szMask );
        psList = FileListOpen( gsSvcWork.sParmWork.szBaseLaneDir, szMask, &dwCount );
        for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
        {
            // Récupérer le nom du fichier et composer son chemin d'accès complet
            _snprintf_s( szFullFile, _countof(szFullFile), sizeof(szFullFile), "%s\\%s", gsSvcWork.sParmWork.szBaseLaneDir, psList[dwIndex] );
            szFullFile[sizeof(szFullFile)-1] = '\0';

            // Si la fonction autocopy est active (chaine non vide pour le répertoire de
            // destination de l'autocopy).
            if ( gsSvcWork.sParmWork.szAutoCopyDir[0] != 0 )
            {
                // Composer le nom complet de la destination pour l'autocopie
                _snprintf_s( szCopyFile, _countof(szCopyFile), sizeof(szCopyFile), "%s\\%s", gsSvcWork.sParmWork.szAutoCopyDir, psList[dwIndex] );
                szCopyFile[sizeof(szCopyFile)-1] = '\0';

                CopyFile( szFullFile, szCopyFile, TRUE );
            }

            NTSVCInfo( "MainPoolThread(), Intégration fichier NAL [%s]", szFullFile );
            dwErr = DBIncorporate( &hDbCnx, szFullFile, DB_NAL, NULL, 0 );
            if ( dwErr != NO_ERROR )
            {
                SVC_ERR( dwErr, "ERR_INCORPORATE_FILE_KEEP" );
                dwErrCount ++;
            }
            else
            {
                dwErrCount = 0;
                NTSVCError( "AVERTISSEMENT : Fichier NAL [%s] intégré !", szFullFile );

                // Effacer le fichier
                DeleteFile( szFullFile );
            }

            // Si trop d'erreurs consécutives, on interromp la boucle.
            // Les fichiers restant à traiter le seront au prochain cycle
            if ( dwErrCount > gsSvcWork.sParmWork.dwMaxNALErrors )
                break;

            // Si la connexion à la base a été rompue, on arrete la boucle
            if ( hDbCnx == NULL )
                break;
        }

        // On a plus besoin de la liste des fichiers
        if ( psList != NULL )
            FileListClose( psList );
        
        for ( dwIndex = 0 ; dwIndex < 10 ; dwIndex ++ )
        {
            Sleep( 500 );
            if ( gsSvcWork.bTerminatePool ) break;
        }

    }

    NTSVCInfo( "MainNalThread(), arrêt du thread demandé" );

    // Le travail est terminé, on peut se déconnecter de la base
    if ( hDbCnx != NULL ) 
    {
        NTSVCInfo( "MainNalThread(), déconnexion base de données" );
        DBDisconnect( hDbCnx );
    }

    NTSVCInfo( "MainNalThread(), fin du thread" );
    ExitThread( 0 );
    return 0;
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
    if ( ! MSG_PERM_RQ_SEND_FILE_New_Record( &gsSvcWork.hInMsgList ) )
    {
        MSG_Delete_All_Records( &gsSvcWork.hInMsgList );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void MainTerminateMessages()
 * PARAMETERS: Aucun
 * RETURN    : Rien
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
 * SYNTAX    : PRIVATE void MainPurgeRejectDirectory()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Limite la taille utilisée du répertoire de rejet en
 *             effacant les plus vieux fichiers jusqu'à être repassé en
 *             dessous de la taille limite.
 * --------------------------------------------------------------------
 */
PRIVATE void MainPurgeRejectDirectory()
{
    HANDLE hFind;
    ULONGLONG ullLimit = 1024 * (ULONGLONG)gsSvcWork.sParmWork.dwRejectKBQuota;
    ULONGLONG ullSize;
    ULONGLONG ullFileSize;
    ULONGLONG ullFileDate;
    ULONGLONG ullStoredDate;
    ULONGLONG ullStoredSize;
    WIN32_FIND_DATA sFind;
    WIN32_FIND_DATA sFindMemo;
    char szMask[MAX_PATH*2];
    char szFile[MAX_PATH*2];

    NTSVCInfo( "MainPurgeRejectDirectory(), scan des fichiers dans [%s]", gsSvcWork.sParmWork.szRejectLaneDir );

    _snprintf_s( szMask, _countof(szMask), sizeof(szMask), "%s\\*", gsSvcWork.sParmWork.szRejectLaneDir );
    szMask[sizeof(szMask)-1] = '\0';

    do
    {
        sFindMemo.cFileName[0] = 0;
        ullSize = 0;
        ullStoredDate = 0;
        ullStoredSize = 0;

        hFind = FindFirstFile( szMask, &sFind );
        if ( hFind == INVALID_HANDLE_VALUE )
            break;

        while ( hFind != INVALID_HANDLE_VALUE )
        {
            if ( ((sFind.dwFileAttributes) & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY )) == 0 )
            {
                ullFileSize = ((ULONGLONG)sFind.nFileSizeHigh) * ((ULONGLONG)0x100000000) + ((ULONGLONG)sFind.nFileSizeLow);
                ullSize += ullFileSize;
                ullFileDate = *((ULONGLONG*)&sFind.ftLastWriteTime);

                if ( ( ullStoredDate == 0 ) || ( ullFileDate < ullStoredDate ) )
                {
                    ullStoredSize = ullFileSize;
                    ullStoredDate = ullFileDate;
                    sFindMemo = sFind;
                }
            }

            if ( ! FindNextFile( hFind, &sFind ) )
            {
                FindClose( hFind );
                hFind = INVALID_HANDLE_VALUE;
            }
        }

        NTSVCInfo( "MainPurgeRejectDirectory(), le répertoire utilise %I64u octets", ullSize );

        if ( ( ullSize > ullLimit ) && ( sFindMemo.cFileName[0] != 0 ) )
        {
            NTSVCInfo( "MainPurgeRejectDirectory(), effacement de %s (%I64u octets)", sFindMemo.cFileName, ullStoredSize );

            _snprintf_s( szFile, _countof(szFile), sizeof(szFile), "%s\\%s", gsSvcWork.sParmWork.szRejectLaneDir, sFindMemo.cFileName );
            szFile[sizeof(szFile)-1] = '\0';

            if ( ! DeleteFile( szFile ) )
            {
                NTSVCInfo( "MainPurgeRejectDirectory(), échec effacement de %s", sFindMemo.cFileName );
                break;
            }
            
            ullSize -= ullStoredSize;
        }
    }
    while ( ullSize > ullLimit );
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
                        TODBSVC_SERVICE_NAME "\\" 
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





