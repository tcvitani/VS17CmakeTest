/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WATCHDOGsvc
 * FILE       : WATCHDOGsvc_main.c
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
#include <col.h>
#include <acom.h>
#include <ntsvc.h>
#include <reg.h>

#include <resource.h>

#define LOC_DEF
#include <WATCHDOGsvc_glob.h>
#undef LOC_DEF

#include <WATCHDOGsvc_text.h>
#include <WATCHDOGsvc_callback.h>

#define LOC_DEF
#include <WATCHDOGsvc_main.h>
#undef LOC_DEF

#include <memclass.h>



// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE void WINAPI MainPollServices();
PRIVATE DWORD WINAPI MainCommand( char * pcParams );
PRIVATE void WINAPI MainLoadMenu();
PRIVATE void WINAPI MainUnloadMenu();
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId );
PRIVATE void WINAPI MainRunApp( char * szCmdLine, DWORD dwTimeout );

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
    (*ppcServiceName) = gszServiceName;
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
    DWORD dwResult;
    DWORD dwTime;
    DWORD dwLastTick = GetTickCount();
    BOOL bSwitch = FALSE;

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" WATCHDOGSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Ouverture des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            WATCHDOGSVC_REG_VAL_CMDSVC        , REG_DWORD,        4,             1, &gsSvcWork.sParmCopy.dwCmdSvc,
            WATCHDOGSVC_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,             5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            WATCHDOGSVC_REG_VAL_WORKERS       , REG_DWORD,        4,             3, &gsSvcWork.sParmCopy.dwWorkers,
            WATCHDOGSVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4,         10000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            WATCHDOGSVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4,           256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            WATCHDOGSVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            WATCHDOGSVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            WATCHDOGSVC_REG_VAL_MAXLIFE       , REG_DWORD,        4,             1, &gsSvcWork.sParmCopy.dwMaxLife,
            WATCHDOGSVC_REG_VAL_LIFETIME      , REG_DWORD,        4,         10000, &gsSvcWork.sParmCopy.dwLifeTime,
            WATCHDOGSVC_REG_VAL_SCMPOLLING    , REG_DWORD,        4,         10000, &gsSvcWork.sParmCopy.dwScmPolling,
            WATCHDOGSVC_REG_VAL_CMDTIMEOUTMS  , REG_DWORD,        4,          2000, &gsSvcWork.sParmCopy.dwCmdTimeoutMs,
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconKo       = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_WATCHDOGSVC_KO ) );
    gsSvcWork.hIconOk       = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_WATCHDOGSVC_OK ) );
    gsSvcWork.hIconOkRun    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_WATCHDOGSVC_OK_RUN ) );
    gsSvcWork.hIconPause    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_WATCHDOGSVC_SUSPENDED ) );
    gsSvcWork.hIconPauseRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_WATCHDOGSVC_SUSPENDED_RUN ) );

    do {
        gsSvcWork.bReload = FALSE;

        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INITIALIZING" );

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

        // Initialiser les pipes
        dwErr = MainInitPipes();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_PIPES" );
            MainTerminateService();
            break;
        }

        if ( dwLoadCount == 0 )
        {
            // Passer dans l'état "running"
            NTSVCSetCurrentState( SERVICE_RUNNING, 0, NO_ERROR );
            NTSVCError( TextFind( gsSvcWork.hText, "SVC_STARTED", NULL ) );
        }

        gsSvcWork.fPaused = FALSE;
        MainLoadMenu();

        // Attendre la demande d'arrêt tout en gérant les timers de vie du protocole
        while ( TRUE )
        {
            bSwitch = ! bSwitch;
            if ( gsSvcWork.fPaused )
                NTSVCRefreshTrayIcon( bSwitch ? gsSvcWork.hIconPause : gsSvcWork.hIconPauseRun, "SVC_PAUSED" );
            else
                NTSVCRefreshTrayIcon( bSwitch ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );

            dwResult = NTSVCWaitForEnd( 1000 );
            
            if ( dwResult != WAIT_TIMEOUT ) 
                break;

            dwTime = GetTickCount() - dwLastTick;

            if ( dwTime < gsSvcWork.sParmWork.dwScmPolling )
                continue;

            dwLastTick = GetTickCount();

            if ( ! gsSvcWork.fPaused )
                MainPollServices();
        }
        
        MainUnloadMenu();

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_STOPWATCHDOG" );

        if ( ! gsSvcWork.bReload )
        {
            // Passer dans l'état "en cours d'arrêt"
            NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );
        }
        else
            NTSVCResetEnd();

        NTSVCInfo( "NTSVCExternalMain(), destruction des objets" );
        MainTerminatePipes();

        NTSVCError( TextFind( gsSvcWork.hText, "SVC_STOPPED", NULL ) );
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
    DWORD dwFoo;
    DWORD dwErr;
    DWORD dwIndex;
    DWORD dwResult;
    DWORD dwLen;
    DWORD dwValue;
    WATCHDOGSVC_SVC * psSvc;
    char  szName[MAX_PATH];


    NTSVCInfo( "MainInitService(), debut" );

    // Charger les paramètres
    dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwFoo );
    if ( dwErr == NO_ERROR )
    {
        gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

        gsSvcWork.psServices = NULL;

        dwIndex = 0;
        while ( TRUE )
        {
            dwLen = sizeof(szName);
            dwResult = REG_Enum_Valeurs_Entier( HKEY_LOCAL_MACHINE,
                                                ( NTSVC_REG_KEY_SERVICE "\\" 
                                                  WATCHDOGSVC_SERVICE_NAME "\\" 
                                                  NTSVC_REG_KEY_PARAM
                                                  "\\Services"
                                                ),
                                                dwIndex,
                                                szName,
                                                &dwLen,
                                                &dwValue );
            if ( dwResult != NO_ERROR )
                break;

            if ( gsSvcWork.psServices == NULL )
                psSvc = HeapAlloc(GetProcessHeap(), 0, sizeof(*(gsSvcWork.psServices)) );
            else
                psSvc = HeapReAlloc( GetProcessHeap(), 0, gsSvcWork.psServices, sizeof(*(gsSvcWork.psServices)) * ( dwIndex + 1 ) );
            
            if ( psSvc == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            gsSvcWork.psServices = psSvc;
            psSvc = &gsSvcWork.psServices[dwIndex];

            ZeroMemory( psSvc, sizeof(*psSvc) );
            strcpy_s( psSvc->szName, MAX_PATH-1, szName );
            psSvc->dwWaitToRestart = dwValue;

            dwLen = sizeof(psSvc->szOnStop);
            dwResult = REG_Lire_Chaine( HKEY_LOCAL_MACHINE,
                                        ( NTSVC_REG_KEY_SERVICE "\\" 
                                          WATCHDOGSVC_SERVICE_NAME "\\" 
                                          NTSVC_REG_KEY_PARAM
                                          "\\OnStop"
                                        ),
                                        szName,
                                        psSvc->szOnStop,
                                        &dwLen );
            if ( dwResult != NO_ERROR )
                psSvc->szOnStop[0] = 0;

            dwLen = sizeof(psSvc->szOnStart);
            dwResult = REG_Lire_Chaine( HKEY_LOCAL_MACHINE,
                                        ( NTSVC_REG_KEY_SERVICE "\\" 
                                          WATCHDOGSVC_SERVICE_NAME "\\" 
                                          NTSVC_REG_KEY_PARAM
                                          "\\OnStart"
                                        ),
                                        szName,
                                        psSvc->szOnStart,
                                        &dwLen );
            if ( dwResult != NO_ERROR )
                psSvc->szOnStart[0] = 0;

            psSvc->dwLastState = SERVICE_STOPPED;

            NTSVCInfo( "MainInitService(), ajout de '%s' à la liste des services surveillés", szName );
            
            dwIndex ++;
        }

        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "SVC_INIT_ERR_MEM" );
            if ( gsSvcWork.psServices != NULL )
                HeapFree( GetProcessHeap(), 0, gsSvcWork.psServices );
        }
        else if ( dwIndex == 0 )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            SVC_ERR( dwErr, "SVC_INIT_EMPTY_LIST" );
        }
        else
        {
            NTSVCInfo( "MainInitService(), %u services doivent être surveillés", dwIndex );
            gsSvcWork.dwServices = dwIndex;

            InitializeCriticalSection( &gsSvcWork.sProtectSvc );
        }
    }
    else
        SVC_ERR( dwErr, "SVC_INIT_PARAMS" );

    NTSVCInfo( "MainInitService(), retourne %u", dwErr );

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
    if ( gsSvcWork.psServices != NULL )
    {
        DeleteCriticalSection( &gsSvcWork.sProtectSvc );
        HeapFree( GetProcessHeap(), 0, gsSvcWork.psServices );
    }

    gsSvcWork.psServices = NULL;
    gsSvcWork.dwServices = 0;

    return NO_ERROR;
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
    // Ajouter le pipe dans la liste des pipes NULL session
    // Attention, si c'est la première fois, ca ne marchera pas.
    if ( gsSvcWork.sParmWork.dwCmdSvc == 1 )
        AComSetNullSessionPipe( WATCHDOGSVC_PIPE_CMD, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,                                      // dwWksUsrKey
            gsSvcWork.sParmWork.dwMaxCmdCnx,        // dwMaxConnections
            1,                                      // dwMaxInstances
            THREAD_PRIORITY_ABOVE_NORMAL,           // dwPriority
            1000,                                   // dwConnectLoopDelay
            gsSvcWork.sParmWork.dwWorkers,          // dwWorkers
            ACOM_PROTOCOL_LIFE,                     // dwProtocol
            gsSvcWork.sParmWork.dwLifeTime,         // dwLifeTime
            gsSvcWork.sParmWork.dwMaxLife,          // dwMaxLife
            0,                                      // dwAllowedTimeShift
            CallbackShutdown );                     // pfShut
    if ( gsSvcWork.hWks == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_WKS" );
        return ERROR_INVALID_DATA;
    }

    NTSVCInfo( "MainInitPipes(), création du serveur interface de commande" );
    if ( gsSvcWork.sParmWork.dwCmdSvc != 0 )
    {
        if ( gsSvcWork.sParmWork.dwCmdSvc == 1 )
            gsSvcWork.hCmdInst = AComOpenPipeServerInstance(
                    gsSvcWork.hWks,                             // hWks
                    WATCHDOGSVC_PIPE_CMD,                       // pcPipeName
                    WATCHDOGSVC_CMD_TYPE,                       // dwInstUsrKey
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
        else if ( gsSvcWork.sParmWork.dwCmdSvc > 1 )
            gsSvcWork.hCmdInst = AComOpenTcpServerInstance(
                    gsSvcWork.hWks,                             // hWks
                    "",                                         // pcAddress
                    gsSvcWork.sParmWork.dwCmdSvc,               // dwPort
                    WATCHDOGSVC_CMD_TYPE,                       // dwInstUsrKey
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
            AComCloseWorkingSet( gsSvcWork.hWks, WATCHDOGSVC_WORKINGSET_TIMEMOUT );
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

    AComCloseWorkingSet( gsSvcWork.hWks, WATCHDOGSVC_WORKINGSET_TIMEMOUT );

    return NO_ERROR;
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
        printf( "WatchDogSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( WATCHDOGSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "WatchDogSvc - Error %u\n", dwErr );
        else
            printf( "WatchDogSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}






/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void WINAPI MainPollServices()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PRIVATE void WINAPI MainPollServices()
{
    DWORD               dwIndex;
    DWORD               dwErr;
    DWORD               dwTick;
    DWORD               dwTime;
    BOOL                fError = FALSE;
	SC_HANDLE           hManager = NULL;
	SC_HANDLE           hService = NULL;
  	SERVICE_STATUS      sStatus;
    WATCHDOGSVC_SVC   * psSvc; 


    NTSVCInfo( "MainPollServices(), début" );

    __try
    {
        hManager = OpenSCManager( NULL, NULL, GENERIC_EXECUTE|GENERIC_READ );
        if ( hManager == NULL )
        {
            fError = TRUE;
            dwErr = GetLastError();

            SVC_ERR( dwErr, "SVC_OPENSCM_ERR" );
            __leave;
        }

        for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwServices ; dwIndex ++ )
        {
            psSvc = &(gsSvcWork.psServices[dwIndex]);

            __try
            {
                hService = OpenService( hManager, psSvc->szName, GENERIC_EXECUTE|GENERIC_READ );
                if ( hService == NULL )
                {
                    dwErr = GetLastError();
                    SVC_ERR( dwErr, "SVC_OPENSVC_ERR" );
                    __leave;
                }

                if ( ! QueryServiceStatus( hService, &sStatus ) )
                {
                    dwErr = GetLastError();
                    SVC_ERR( dwErr, "SVC_QUERYSVC_ERR" );
                    __leave;
                }

                dwTick = GetTickCount();

                EnterCriticalSection( &gsSvcWork.sProtectSvc );

                if ( ( sStatus.dwCurrentState == SERVICE_STOPPED ) && 
                     ( psSvc->dwLastState     == SERVICE_STOPPED ) )
                {
                    dwTime = ( dwTick - psSvc->dwLastTick );
                    if ( dwTime > psSvc->dwWaitToRestart )
                    {
                        NTSVCInfo( "MainPollServices(), le service %s doit être redémarré", psSvc->szName );
                        
                        if ( psSvc->szOnStart[0] != 0 )
                        {
                            NTSVCInfo( "MainPollServices(), exécution de la commande %s::OnStart [%s]", psSvc->szName, psSvc->szOnStart );
                            MainRunApp( psSvc->szOnStart, gsSvcWork.sParmWork.dwCmdTimeoutMs );
                        }

               			if ( ! StartService( hService, 0, NULL ) )
                            SVC_ERR( dwErr, "SVC_STARTSVC_ERR" );
                        else
                            NTSVCError( "%s(%s)", TextFind( gsSvcWork.hText, "SVC_STARTOK", NULL ), psSvc->szName );
                        psSvc->dwLastTick = dwTick;
                        psSvc->dwLastState = SERVICE_START_PENDING;
                    }
                }
                else
                {
                    if ( ( sStatus.dwCurrentState == SERVICE_STOPPED ) && 
                         ( psSvc->dwLastState     != SERVICE_STOPPED ) )
                    {
                        if ( psSvc->szOnStop[0] != 0 )
                        {
                            NTSVCInfo( "MainPollServices(), exécution de la commande %s::OnStop [%s]", psSvc->szName, psSvc->szOnStop );
                            MainRunApp( psSvc->szOnStop, gsSvcWork.sParmWork.dwCmdTimeoutMs );
                        }
                    }
                    psSvc->dwLastState = sStatus.dwCurrentState;
                    psSvc->dwLastTick = dwTick;
                }

                LeaveCriticalSection( &gsSvcWork.sProtectSvc );
            }
            __finally
            {
                if ( hService != NULL )
                {
                    CloseServiceHandle( hService );
                    hService = NULL;
                }
            }
        }
    }
    __finally
    {
        if ( hManager != NULL )
        {
            CloseServiceHandle( hManager );
            hManager = NULL;
        }
    }

    NTSVCInfo( "MainPollServices(), fin" );
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
        sprintf_s( szKey, 
						sizeof(szKey),
						NTSVC_REG_KEY_SERVICE "\\" 
                        WATCHDOGSVC_SERVICE_NAME "\\" 
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
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 1, "Pause" );
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

        if ( uiId == 1 )
            gsSvcWork.fPaused = TRUE;
        else if ( uiId == 2 )
            gsSvcWork.fPaused = FALSE;

        if ( gsSvcWork.fPaused )
        {
            NTSVCDefineTrayIconMenu( NTSVC_MENU_REMOVE, 1, "Pause" );
            NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 2, "Resume" );
        }
        else
        {
            NTSVCDefineTrayIconMenu( NTSVC_MENU_REMOVE, 2, "Resume" );
            NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 1, "Pause" );
        }
    }
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI MainRunApp( char * szCmdLine, DWORD dwTimeout )
 * PARAMETERS: szCmdLine : Ligne de commande à exécuter
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Execute une ligne de commande quelconque en attendant sa
 *             fin avec timeout.
 * --------------------------------------------------------------------
 */
void WINAPI MainRunApp( char * szCmdLine, DWORD dwTimeout )
{
    BOOL fResult;
    STARTUPINFO sStartup;
    PROCESS_INFORMATION sProcess;

    ZeroMemory( &sStartup, sizeof(sStartup) );
    sStartup.cb = sizeof( sStartup );
    
    fResult = CreateProcess( NULL, 
                             szCmdLine, 
                             NULL,
                             NULL, 
                             FALSE, 
                             NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                             NULL,
                             NULL,
                             &sStartup,
                             &sProcess );
    if ( fResult )
    {
        WaitForSingleObject( sProcess.hProcess, dwTimeout );
        CloseHandle( sProcess.hThread );
        CloseHandle( sProcess.hProcess );
    }
}
