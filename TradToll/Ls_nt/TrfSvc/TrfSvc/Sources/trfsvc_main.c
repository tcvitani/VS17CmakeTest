/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
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
#include <col.h>
#include <reg.h>

#include <trfsvc_res.h>

#include <csr_rfa_svc.h>
#include <csr_rfa.h>

#define LOC_DEF
#include <trfsvc_glob.h>
#undef LOC_DEF

#include <trfsvc_text.h>
#include <trfsvc_cmd_callback.h>
#include <trfsvc_clt_callback.h>
#include <trfsvc_srv_callback.h>
#include <trfsvc_file_callback.h>
#include <trfsvc_cfg.h>

#define LOC_DEF
#include <trfsvc_main.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)


// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE DWORD WINAPI MainCommand( char * pcParams );
PRIVATE void CALLBACK MainCallbackShutdown(
		DWORD64 dwWksUsrKey,
        DWORD dwError );
PRIVATE void WINAPI MainLoadMenu();
PRIVATE void WINAPI MainUnloadMenu();
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId );

// TMA
PROTECTED DWORD WINAPI WM_Dispatch (PVOID param);
PROTECTED DWORD WINAPI WM_Received (PVOID param);
// end of TMA

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
    DWORD dwTick;
    DWORD dwNewTick;
    BOOL bState = TRUE;
    BOOL bDisconnect;
    TRFSVC_FILE * psFile;
    TRFSVC_SERVER * psServer;
    TRFSVC_CLIENT * psClient;
    HANDLE hFile;
    HCOLLECTIONITEM hItem;
    HCOLLECTIONITEM hItem2;
    ULONGLONG ullDirectLastTime;
    ULONGLONG ullStoredLastTime;
    ULONGLONG ullCompare;
    ULONGLONG ullTolerance;
    char szFaultyExt[MAX_PATH];

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" TRFSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Ouverture des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
        TRFSVC_REG_VAL_CMDSVC         , REG_DWORD,        4,             1, &gsSvcWork.sParmCopy.dwCmdSvc,
        TRFSVC_REG_VAL_MAXCMDCNX      , REG_DWORD,        4,             5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
        TRFSVC_REG_VAL_MAXCLTCNX      , REG_DWORD,        4,            16, &gsSvcWork.sParmCopy.dwMaxCltCnx,
        TRFSVC_REG_VAL_WORKERS        , REG_DWORD,        4,             4, &gsSvcWork.sParmCopy.dwWorkers,
        TRFSVC_REG_VAL_RECONNECTTIME  , REG_DWORD,        4,         30000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
        TRFSVC_REG_VAL_QUEUESIZE      , REG_DWORD,        4,           256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
        TRFSVC_REG_VAL_MAXMSGSIZE     , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
        TRFSVC_REG_VAL_BUFFERSIZE     , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
        TRFSVC_REG_VAL_MAXLIFE        , REG_DWORD,        4,             3, &gsSvcWork.sParmCopy.dwMaxLife,
        TRFSVC_REG_VAL_LIFETIME       , REG_DWORD,        4,          5000, &gsSvcWork.sParmCopy.dwLifeTime,
        TRFSVC_REG_VAL_CHECKPERIOD    , REG_DWORD,        4,          5000, &gsSvcWork.sParmCopy.dwCheckPeriod,
        TRFSVC_REG_VAL_TIMETOLERANCE  , REG_DWORD,        4,          1500, &gsSvcWork.sParmCopy.dwTimeTolerance,
        TRFSVC_REG_VAL_TCPCLIENT      , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTcpClient,
        TRFSVC_REG_VAL_TCPSERVER      , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTcpServer,
        NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TRFSVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TRFSVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TRFSVC_OK_RUN ) );


	gsSvcWork.WM_ACOM = RegisterWindowMessage (ACOM_WM_EVENT_NAME);
	NTSVCInfo( "Creating new thread -> WM_Dispatch()" );
	gsSvcWork.hTread = NTSVCCreateThread (NULL, 0, WM_Dispatch, NULL, 0, &gsSvcWork.dwThreadId, "ServerSideManagement");
	NTSVCInfo( "Creating new thread -> WM_Received()" );
	gsSvcWork.hReceived = NTSVCCreateThread (NULL, 0, WM_Received, NULL, 0, &gsSvcWork.dwThreadReceivedId, "ACOM_WM_RECEIVED");
	// end of TMA

    do 
    {
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INITIALIZING" );

        if ( dwLoadCount == 0 )
        {
            // Passer dans l'état "en cours de démarrage"
            NTSVCSetCurrentState( SERVICE_START_PENDING, 10000, NO_ERROR );
        }

        // Lire les paramètres d'init du service
        dwErr = MainInitService();
        if ( dwErr != NO_ERROR )
            break;

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
            NTSVCError( TextFind( gsSvcWork.hText, "SVC_STARTED", "SVC_STARTED" ) );
        }

        MainLoadMenu();

        // Les traitements ont été lancés avec les workers

        // Attendre la demande d'arrêt tout en gérant les timers de vie du protocole
        dwTick = GetTickCount();
        while ( TRUE )
        {
            bState = ! bState;

            if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT ) 
                break;

            NTSVCRefreshTrayIcon( bState ? gsSvcWork.hIconOk : gsSvcWork.hIconOkRun, "SVC_RUNNING" );
            
            if ( ! CfgExtensionTestStatus( szFaultyExt, sizeof(szFaultyExt) ) )
            {
                NTSVCError( "NTSVCExternalMain(), l'extension %s a reporte une erreur, le service va tenter de se recharger", szFaultyExt );
                SVC_ERR( dwErr, "ERR_EXT_STATUS" );
                gsSvcWork.bReload = TRUE;
                NTSVCSignalEnd();
            }

            dwNewTick = GetTickCount();

			if ( (dwNewTick > dwTick && ((dwNewTick - dwTick) > gsSvcWork.sParmWork.dwCheckPeriod)) || 
				(dwTick > dwNewTick))
            {
                dwTick = dwNewTick;

                hItem = COL_SCAN_BEGIN;
                while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
                {
                    // Utiliser le vérou de la collection comme exclusion pour
                    // serialiser les comparaisons et mises à jour des dates
                    ColLock( gsSvcWork.hColFiles );

                    psFile = ColItemData( hItem );

					if (psFile->bIsReceivingFileInProgress) 
					{
						if ((GetSystemULLTime() - psClient->psFile->ullLastTimeFullFileChunkReceived) > (ULONGLONG)gsSvcWork.sParmWork.dwCheckPeriod * 10000ULL)
						{
							NTSVCInfo("NTSVCExternalMain()::Check period for File:[%s] - ReceivingFileInProgress - to old. Reset flag and check the file time.", psFile->szFileKey);

							//mark the flag to FALSE so if no package is received 
							//it will initiate the file check next time the check period elapses
							psFile->bIsReceivingFileInProgress = FALSE;
						}
						else //recheck in the next check period
						{
							NTSVCInfo("NTSVCExternalMain()::Check period for File:[%s] - ReceivingFileInProgress -  retry latter", psFile->szFileKey);
							ColUnlock(gsSvcWork.hColFiles);
							continue; //continue to the next file if any
						}
					}

					ullStoredLastTime = psFile->ullLastTime;

                    hFile = RFACreateFile( psFile->szFileKey, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                    if ( hFile == INVALID_HANDLE_VALUE )
                    {
						NTSVCInfo("NTSVCExternalMain()::Error RFACreateFile:[%s]", psFile->szFileKey);
                        ullDirectLastTime = 0;
                    }
                    else 
                    {
						FILETIME sftLastWriteTime;

                        RFAEnter( hFile );
						if (!RFAGetFileTime(hFile, NULL, NULL, (LPFILETIME)&sftLastWriteTime))
						{
							NTSVCInfo("NTSVCExternalMain()::Error RFAGetFileTime:[%s]", psFile->szFileKey);
							ullDirectLastTime = 0;
						}
						else
							ullDirectLastTime = convertFILETIME(&sftLastWriteTime);

                        RFALeave( hFile );
                        RFACloseHandle( hFile );
                    }

                    if ( ullStoredLastTime >= ullDirectLastTime )
                        ullCompare = ( ullStoredLastTime - ullDirectLastTime );
                    else
                        ullCompare = ( ullDirectLastTime - ullStoredLastTime );

                    if ( ullCompare != 0 )
                    {
                        ullTolerance = ((ULONGLONG)(gsSvcWork.sParmWork.dwTimeTolerance)) * (ULONGLONG)10000;

                        if ( ullCompare > ullTolerance )
                        {
							NTSVCInfo("NTSVCExternalMain()::Time Diff NOK: Diff: 0x%016X .100ns (tolerance de 0x%016X .100ns)  ullStoredLastTime:[0x%016X]  ullDirectLastTime[0x%016X] :Will force disconnect...", ullCompare, ullTolerance, ullStoredLastTime, ullDirectLastTime);
                            bDisconnect = TRUE;
                        }
                        else
                        {
							NTSVCInfo("NTSVCExternalMain()::Time Diff OK:  0x%016X .100ns (tolerance de 0x%016X .100ns)", ullCompare, ullTolerance);
                            bDisconnect = FALSE;
                        }
                    }
                    else
                        bDisconnect = FALSE;

                    ColUnlock( gsSvcWork.hColFiles );

                    if ( bDisconnect )
                    {
						NTSVCInfo("NTSVCExternalMain(), Disconnecting..");
						SVC_ERR_S(dwErr, "ERR_EXTERNAL_UPDATE", psFile->szFileKey);

                        ColLock( gsSvcWork.hColServers );
                        hItem2 = COL_SCAN_BEGIN;
                        while ( ColItemScan( gsSvcWork.hColServers, &hItem2 ) )
                        {
                            psServer = ColItemData( hItem2 );
                            if ( psServer->psFile != NULL )
                                if ( _stricmp( psFile->szFileKey, psServer->psFile->szFileKey ) != 0 ) continue;
                            AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
                        }
                        ColUnlock( gsSvcWork.hColServers );

                        ColLock( gsSvcWork.hColClients );
                        hItem2 = COL_SCAN_BEGIN;
                        while ( ColItemScan( gsSvcWork.hColClients, &hItem2 ) )
                        {
                            psClient = ColItemData( hItem2 );
                            if ( psClient->psFile != NULL )
                                if ( _stricmp( psFile->szFileKey, psClient->psFile->szFileKey ) != 0 ) continue;
                            AComDisconnectPeer( gsSvcWork.hWks, psClient->hCnx, FALSE );
                        }
                        ColUnlock( gsSvcWork.hColClients );
                    }
                }
            }

        }

        MainUnloadMenu();

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

        if ( ! gsSvcWork.bReload )
        {
            // Passer dans l'état "en cours d'arrêt"
            NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );
        }
        else
            NTSVCResetEnd();

        NTSVCInfo( "NTSVCExternalMain(), destruction des objets" );
        MainTerminatePipes();

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

    __try
    {

        // Charger les paramètres standard du service
        dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwFoo );
        if ( dwErr != NO_ERROR ) __leave;

        gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

        gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
        gsSvcWork.bReload = FALSE;

        // Créer la collection des extensions
        gsSvcWork.hColExtensions = ColCreate( 
            COL_INDEX_ZSTRING,  // Index type
            MAX_PATH,           // Index max size
            TRUE,               // Copy item
            TRUE );             // Protected
        if ( gsSvcWork.hColExtensions == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            SVC_ERR( dwErr, "ERR_EXT_COL_CREATE" );
            __leave; 
        }

        // Créer la collection des fichiers à gérer
        gsSvcWork.hColFiles = ColCreate( 
            COL_INDEX_ZSTRING,  // Index type
            MAX_PATH,           // Index max size
            TRUE,               // Copy item
            TRUE );             // Protected
        if ( gsSvcWork.hColFiles == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            SVC_ERR( dwErr, "ERR_FILES_COL_CREATE" );
            __leave; 
        }

        // Créer la collection des client connectés
        gsSvcWork.hColClients = ColCreate( 
            COL_INDEX_BINARY,   // Index type
            sizeof(ACOM_CNX_HANDLE), // Index max size
            FALSE,              // Copy item
            TRUE );             // Protected
        if ( gsSvcWork.hColClients == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            SVC_ERR( dwErr, "ERR_CLT_COL_CREATE" );
            __leave; 
        }

        // Créer la collection des serveurs
        gsSvcWork.hColServers = ColCreate( 
            COL_INDEX_BINARY,   // Index type
            sizeof(ACOM_CNX_HANDLE), // Index max size
            FALSE,              // Copy item
            TRUE );             // Protected
        if ( gsSvcWork.hColServers == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            SVC_ERR( dwErr, "ERR_SRV_COL_CREATE" );
            __leave; 
        }

        // Charger les paramètres de configuration des fichier à répliquer
        dwErr = CfgLoad();
        if ( dwErr != NO_ERROR ) 
        {
            SVC_ERR( dwErr, "ERR_REPL_CFG_LOAD" );
            __leave;
        }

        // Vérifier qu'au moins un fichier est défini
        if ( ColCount( gsSvcWork.hColFiles ) == 0 )
        { 
            dwErr = ERROR_INVALID_DATA; 
            SVC_ERR( dwErr, "ERR_FILES_COL_EMPTY" );
            __leave; 
        }

        RFASetHook( FileCallback );
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( gsSvcWork.hColServers != NULL )
                ColDestroy( gsSvcWork.hColServers );

            if ( gsSvcWork.hColClients != NULL )
                ColDestroy( gsSvcWork.hColClients );

            if ( gsSvcWork.hColFiles != NULL )
            {
                CfgUnload();
                ColDestroy( gsSvcWork.hColFiles );
            }

            if ( gsSvcWork.hColExtensions != NULL )
                ColDestroy( gsSvcWork.hColExtensions );
        }

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
    HCOLLECTIONITEM hItem;
    TRFSVC_CLIENT * psClient;
    TRFSVC_SERVER * psServer;

    RFASetHook( NULL );

    CfgUnload();

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColClients, &hItem ) )
    {
        psClient = ColItemData( hItem );
        ColItemRemove( hItem );
        HeapFree( GetProcessHeap(), 0, psClient );
        hItem = COL_SCAN_BEGIN;
    }
    ColDestroy( gsSvcWork.hColClients );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColServers, &hItem ) )
    {
        psServer = ColItemData( hItem );
        ColItemRemove( hItem );
        HeapFree( GetProcessHeap(), 0, psServer );
        hItem = COL_SCAN_BEGIN;
    }
    ColDestroy( gsSvcWork.hColServers );

    ColDestroy( gsSvcWork.hColFiles );
    ColDestroy( gsSvcWork.hColExtensions );

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
    DWORD dwErr = NO_ERROR;
    HCOLLECTIONITEM hItem;
    TRFSVC_FILE * psFile;
    DWORD dwImportCount;
    DWORD dwMaxCnx;
    DWORD dwMaxInst;
	char szName[MAX_PATH];
	char szKey[MAX_PATH];
	DWORD dwRes, dwSize;
	DWORD dwACOMTraceResult = NO_ERROR;

    __try
    {
        gsSvcWork.hWks = NULL;
        gsSvcWork.hCmdInst = NULL;

        hItem = COL_SCAN_BEGIN;
        dwImportCount = 0;
        while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
        {
            psFile = ColItemData( hItem );
            if ( psFile->bImport )
                dwImportCount ++;
        }

        dwMaxCnx = gsSvcWork.sParmWork.dwMaxCmdCnx + gsSvcWork.sParmWork.dwMaxCltCnx + dwImportCount;
        dwMaxInst = 2 + dwImportCount;


		sprintf_s(szKey, MAX_PATH, "%s\\%s\\%s", NTSVC_REG_KEY_SERVICE, TRFSVC_SERVICE_NAME, NTSVC_REG_KEY_PARAM);
		dwSize = sizeof(gsSvcWork.sParmWork.szACOMTraceFile);

		dwRes = REG_Defaut_Chaine(HKEY_LOCAL_MACHINE, szKey, TRFSVC_REG_VAL_ACOM_TRACE_FILE, gsSvcWork.sParmWork.szACOMTraceFile, &dwSize, "");
		if (dwRes == NO_ERROR && dwSize>1)
		{
			dwACOMTraceResult = AComOpenTrace(TRUE, TRUE, gsSvcWork.sParmWork.szACOMTraceFile);
			NTSVCInfo("MainInitPipes(), Initalizing ACOM trace to file:[%s] returned %d", gsSvcWork.sParmWork.szACOMTraceFile, dwACOMTraceResult);
		}
		else
		{
			NTSVCInfo("MainInitPipes(), ACOM trace disabled:parameter [%s] not set or invalid!", TRFSVC_REG_VAL_ACOM_TRACE_FILE);
		}


        // Ajouter les pipes dans la liste des pipes NULL session
        // Attention, si c'est la première fois, ca ne marchera pas.
        AComSetNullSessionPipe( TRFSVC_PIPE_TRF, TRUE );
        AComSetNullSessionPipe( TRFSVC_PIPE_CMD, TRUE );

        // MISE EN PLACE DU WORKING SET

        NTSVCInfo( "MainInitPipes(), creation du working set" );
        gsSvcWork.hWks = AComOpenWorkingSet(
            0,                              // dwWksUsrKey
            dwMaxCnx,                       // dwMaxConnections
            dwMaxInst,                      // dwMaxInstances
            THREAD_PRIORITY_ABOVE_NORMAL,   // dwPriority
            1000,                           // dwConnectLoopDelay
            gsSvcWork.sParmWork.dwWorkers,            // dwWorkers
			ACOM_PROTOCOL_LIFE,             // dwProtocol | ACOM_FLAG_NO_GET_PEER_NAME ???
            gsSvcWork.sParmWork.dwLifeTime,           // dwLifeTime
            gsSvcWork.sParmWork.dwMaxLife,            // dwMaxLife
            0,                              // dwAllowedTimeShift
            MainCallbackShutdown );             // pfShut


        if ( gsSvcWork.hWks == NULL )
        {
            dwErr = ERROR_PIPE_NOT_CONNECTED;
            SVC_ERR( dwErr, "ERR_INIT_WKS" );
            __leave;
        }

        NTSVCInfo( "MainInitPipes(), création du serveur interface de commande" );
        if ( gsSvcWork.sParmWork.dwCmdSvc != 0 )
        {
            if ( gsSvcWork.sParmWork.dwCmdSvc == 1 )
                gsSvcWork.hCmdInst = AComOpenPipeServerInstance(
                        gsSvcWork.hWks,                 // hWks
                        TRFSVC_PIPE_CMD,                // pcPipeName
                        TRFSVC_PIPE_CMD_TYPE,           // dwInstUsrKey
                        gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
                        gsSvcWork.sParmWork.dwMaxCmdCnx,          // dwMaxConnections
                        gsSvcWork.sParmWork.dwPipeBufferSize,     // dwOutBufferSize
                        gsSvcWork.sParmWork.dwPipeBufferSize,     // dwInBufferSize
                        gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
                        gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
                        CmdCallbackConnection,
                        CmdCallbackDisconnection,
                        CmdCallbackReceived,
                        CmdCallbackSent,
                        NULL );
            else
                gsSvcWork.hCmdInst = AComOpenTcpServerInstance(
                        gsSvcWork.hWks,                             // hWks
                        "",                                         // pcAddress
                        gsSvcWork.sParmWork.dwCmdSvc,               // dwPort
                        TRFSVC_PIPE_CMD_TYPE,                       // dwInstUsrKey
                        gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
                        gsSvcWork.sParmWork.dwMaxCmdCnx,            // dwMaxConnections
                        gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
                        gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
                        gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
                        gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
                        CmdCallbackConnection,
                        CmdCallbackDisconnection,
                        CmdCallbackReceived,
                        CmdCallbackSent,
                        NULL );
            if ( gsSvcWork.hCmdInst == NULL )
            {
                dwErr = ERROR_PIPE_NOT_CONNECTED;
                SVC_ERR( dwErr, "ERR_INIT_CMD_SRV" );
                __leave;
            }
        }

        NTSVCInfo( "MainInitPipes(), création des instances de clients pour la gestion des fichiers importés" );
        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
        {
            psFile = ColItemData( hItem );
            if ( psFile->bImport )
            {
				_snprintf_s(szName, sizeof(szName), sizeof(szName), TRFSVC_PIPE_TRF_CLT, psFile->szServer);
                szName[sizeof(szName)-1] = '\0';

                if ( gsSvcWork.sParmWork.dwTcpClient == 0 )
                    psFile->hInst = AComOpenPipeClientInstance(
                        gsSvcWork.hWks,                           // hWks
                        szName,                                   // pcPipeName
						(DWORD64)psFile,                            // dwInstUsrKey
                        gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
                        gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
                        gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
                        CltCallbackConnection,
                        CltCallbackDisconnection,
                        CltCallbackReceived,
                        CltCallbackSent,
                        NULL );
                else
                    psFile->hInst = AComOpenTcpClientInstance(
                        gsSvcWork.hWks,                           // hWks
                        psFile->szServer,                         // pcAddress
                        gsSvcWork.sParmWork.dwTcpClient,          // dwPort
						(DWORD64)psFile,                            // dwInstUsrKey
                        gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
                        gsSvcWork.sParmWork.dwPipeBufferSize,     // dwOutBufferSize
                        gsSvcWork.sParmWork.dwPipeBufferSize,     // dwInBufferSize
                        gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
                        gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
                        CltCallbackConnection,
                        CltCallbackDisconnection,
                        CltCallbackReceived,
                        CltCallbackSent,
                        NULL );

                if ( psFile->hInst == NULL )
                {
                    dwErr = ERROR_PIPE_NOT_CONNECTED;
                    SVC_ERR( dwErr, "ERR_INIT_FILE_CLT" );
                    __leave;
                }
            }
            else
                psFile->hInst = NULL;
        }

        if ( gsSvcWork.sParmWork.dwTcpServer == 0 )
				// TMA
				/*gsSvcWork.hSrvInst = AComOpenPipeServerInstance(
					gsSvcWork.hWks,                 // hWks
					TRFSVC_PIPE_TRF,                // pcPipeName
					TRFSVC_PIPE_TRF_TYPE,           // dwInstUsrKey
					gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
					gsSvcWork.sParmWork.dwMaxCltCnx,          // dwMaxConnections
					gsSvcWork.sParmWork.dwPipeBufferSize,     // dwOutBufferSize
					gsSvcWork.sParmWork.dwPipeBufferSize,     // dwInBufferSize
					gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
					gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
					SrvCallbackConnection,
					SrvCallbackDisconnection,
					SrvCallbackReceived,
					SrvCallbackSent,
					NULL );*/
				// end of TMA
				// TMA
				gsSvcWork.hSrvInst = AComOpenPipeServerInstanceTQ(
					gsSvcWork.hWks,                 // hWks
					TRFSVC_PIPE_TRF,                // pcPipeName
					TRFSVC_PIPE_TRF_TYPE,           // dwInstUsrKey
					gsSvcWork.sParmWork.dwTimeToReconnect,    // dwTimeToReconnect
					gsSvcWork.sParmWork.dwMaxCltCnx,          // dwMaxConnections
					gsSvcWork.sParmWork.dwPipeBufferSize,     // dwOutBufferSize
					gsSvcWork.sParmWork.dwPipeBufferSize,     // dwInBufferSize
					gsSvcWork.sParmWork.dwMaxQueuedMessages,  // dwQueueSize
					gsSvcWork.sParmWork.dwMaxMsgSize,         // dwMaxMessageSize
					gsSvcWork.dwThreadId);
				// end of TMA
			else
				// TMA
				/*gsSvcWork.hSrvInst = AComOpenTcpServerInstance(
					gsSvcWork.hWks,                             // hWks
					"",                                         // pcAddress
					gsSvcWork.sParmWork.dwTcpServer,            // dwPort
					TRFSVC_PIPE_TRF_TYPE,                       // dwInstUsrKey
					gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
					gsSvcWork.sParmWork.dwMaxCltCnx,            // dwMaxConnections
					gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
					gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
					gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
					gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
					SrvCallbackConnection,
					SrvCallbackDisconnection,
					SrvCallbackReceived,
					SrvCallbackSent,
					NULL );*/
				// end of TMA
				// TMA
				gsSvcWork.hSrvInst = AComOpenTcpServerInstanceTQ(
					gsSvcWork.hWks,                             // hWks
					"",                                         // pcAddress
					gsSvcWork.sParmWork.dwTcpServer,            // dwPort
					TRFSVC_PIPE_TRF_TYPE,                       // dwInstUsrKey
					gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
					gsSvcWork.sParmWork.dwMaxCltCnx,            // dwMaxConnections
					gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
					gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
					gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
					gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
					gsSvcWork.dwThreadId);
				// end of TMA
        if ( gsSvcWork.hSrvInst == NULL )
        {
            dwErr = ERROR_PIPE_NOT_CONNECTED;
            SVC_ERR( dwErr, "ERR_INIT_FILE_SRV" );
            __leave;
        }

        dwErr = CfgExtensionsStart();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_START_EXT" );
            __leave;
        }

        NTSVCInfo( "MainInitPipes(), toutes les instance sont créés" );
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            CfgExtensionsStop();
            if ( gsSvcWork.hWks != NULL )
                AComCloseWorkingSet( gsSvcWork.hWks, TRFSVC_WORKINGSET_TIMEMOUT );
        }
    }

	return dwErr;
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

    CfgExtensionsStop();

    AComCloseWorkingSet( gsSvcWork.hWks, TRFSVC_WORKINGSET_TIMEMOUT );

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
        printf( "TrfSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( TRFSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "TrfSvc - Error %u\n", dwErr );
        else
        {
            dwErr = AComSetNullSessionPipe( TRFSVC_PIPE_TRF, TRUE );
            if ( dwErr != NO_ERROR )
                printf( "TrfSvc - Error %u\n", dwErr );
            else
                printf( "TrfSvc - Null session pipe setup done\n" );
        }
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK MainCallbackShutdown(
 *                     DWORD64 dwWksUsrKey,
 *                     DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Appelé lorsque la com va mal. Cela provoque un arrêt du service.
 * --------------------------------------------------------------------
 */
PRIVATE void CALLBACK MainCallbackShutdown(
        DWORD64 dwWksUsrKey,
        DWORD dwError )
{
    SVC_ERR( ERROR_INVALID_DATA, "ERR_WKS_SHUTDOWN" );
    RaiseException( 0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL );
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
		sprintf_s(szKey, 
				sizeof(szKey),
				NTSVC_REG_KEY_SERVICE "\\"
                TRFSVC_SERVICE_NAME "\\" 
                NTSVC_REG_KEY_PARAM "\\MenuItems\\%u",
				gsSvcWork.dwMenuCommands );

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


PROTECTED unsigned long long convertFILETIME(const FILETIME *pFileTime)
{
	ULARGE_INTEGER    lv_Large;

	lv_Large.LowPart = pFileTime->dwLowDateTime;
	lv_Large.HighPart = pFileTime->dwHighDateTime;

	return lv_Large.QuadPart;
}




PROTECTED unsigned long long GetSystemULLTime() //UTC time
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return convertFILETIME(&ft);

}