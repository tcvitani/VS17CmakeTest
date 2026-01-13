/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : authsvc_vt
 * FILE       : authsvc_vt_main.c
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
#include <dbif.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>
#include <reg.h>
#include <csr_msg.h>

#include <msg_lc_header.h>
#include <msg_lc_auth_vt_enl_rep.h>
#include <msg_lc_auth_vt_enl_req.h>
#include <msg_lc_auth_vt_exl_rep.h>
#include <msg_lc_auth_vt_exl_req.h>
#include <msg_lc_auth_vt_enl_rep_v2.h>
#include <msg_lc_auth_vt_enl_req_v2.h>
#include <msg_lc_auth_vt_exl_rep_v2.h>
#include <msg_lc_auth_vt_exl_req_v2.h>
#include <msg_lc_auth_vt_enl_rep_v3.h>
#include <msg_lc_auth_vt_enl_req_v3.h>
#include <msg_lc_auth_vt_exl_rep_v3.h>
#include <msg_lc_auth_vt_exl_req_v3.h>


#include <resource.h>

#define LOC_DEF
#include <authsvc_vt_glob.h>
#undef LOC_DEF

#include <authsvc_vt_text.h>
#include <authsvc_vt_callback.h>
#include <authsvc_vt_comcallback.h>
#include <authsvc_vt_commsg.h>
#include <authsvc_vt_db.h>

#define LOC_DEF
#include <authsvc_vt_main.h>
#undef LOC_DEF

#include <memclass.h>

// --------------- MACROS           ----------------------

// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE void MainTreatment();
PRIVATE DWORD WINAPI MainCommand( char * pcParams );

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

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" AUTHSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Définition des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            AUTHSVC_REG_VAL_CMDSVC               , REG_DWORD,        4,                       1, &gsSvcWork.sParmCopy.dwCmdSvc,
            AUTHSVC_REG_VAL_MAXCMDCNX            , REG_DWORD,        4,                       5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
			AUTHSVC_REG_VAL_MAXAUTHCLTCNX        , REG_DWORD,        4,                       3, &gsSvcWork.sParmCopy.dwAuthCltCnx,
            AUTHSVC_REG_VAL_RECONNECTTIME        , REG_DWORD,        4,                    1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            AUTHSVC_REG_VAL_QUEUESIZE            , REG_DWORD,        4,                     256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            AUTHSVC_REG_VAL_MAXMSGSIZE           , REG_DWORD,        4,                   16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            AUTHSVC_REG_VAL_BUFFERSIZE           , REG_DWORD,        4,                   16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            AUTHSVC_REG_VAL_MAXLIFE              , REG_DWORD,        4,                       3, &gsSvcWork.sParmCopy.dwMaxLife,
            AUTHSVC_REG_VAL_LIFETIME             , REG_DWORD,        4,                    5000, &gsSvcWork.sParmCopy.dwLifeTime,
            AUTHSVC_REG_VAL_MAINPOLLING          , REG_DWORD,        4,                   60000, &gsSvcWork.sParmCopy.dwMainPolling,
			AUTHSVC_REG_VAL_SYNCHRODELAY         , REG_DWORD,        4,                   30000, &gsSvcWork.sParmCopy.dwSynchroDelay,
            AUTHSVC_REG_VAL_ISAUTHSERVER         , REG_DWORD,        4,                       0, &gsSvcWork.sParmCopy.dwIsAuthServer,
            AUTHSVC_REG_VAL_ISAUTHCLIENT         , REG_DWORD,        4,                       0, &gsSvcWork.sParmCopy.dwIsAuthClient,
            AUTHSVC_REG_VAL_AUTHSERVERNAME       , REG_SZ   , MAX_PATH,                      "", &gsSvcWork.sParmCopy.szAuthServer,
            AUTHSVC_REG_VAL_ROUTESERVER          , REG_SZ   , MAX_PATH,                      "", &gsSvcWork.sParmCopy.szRouteServer,
			AUTHSVC_REG_VAL_APP_MSG_ID           , REG_DWORD,        4,                     130, &gsSvcWork.sParmCopy.dwAppMsgId,
										         
			AUTHSVC_REG_VAL_DBUSR                , REG_SZ   , MAX_PATH,                      "", &gsSvcWork.sParmCopy.szDbUsr,
            AUTHSVC_REG_VAL_DBPWD                , REG_SZ   , MAX_PATH,                      "", &gsSvcWork.sParmCopy.szDbPwd,
            AUTHSVC_REG_VAL_DBINST               , REG_SZ   , MAX_PATH,                      "", &gsSvcWork.sParmCopy.szDbInst,
            AUTHSVC_REG_VAL_DBKEEPCONN           , REG_DWORD,        4,                       1, &gsSvcWork.sParmCopy.dwDbKeepConnection,
            AUTHSVC_REG_VAL_DBKEEPCONNPERIOD_MIN , REG_DWORD,        4,                       20, &gsSvcWork.sParmCopy.dwDbKeepConnPeriodMin,
										         
			AUTHSVC_REG_VAL_TRFCHG               , REG_DWORD,        4,                       0, &gsSvcWork.sParmCopy.dwTrfChanges,
			AUTHSVC_REG_VAL_MAXTRFROWS           , REG_DWORD,        4,                     100, &gsSvcWork.sParmCopy.dwMaxTrfRows,
            AUTHSVC_REG_VAL_DBRECCONECT_IF_ERROR, REG_SZ    , MAX_PATH,           "2396,12571", &gsSvcWork.sParmCopy.szDbReconnectIfErr,
			
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_IMAGEEXTSVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_IMAGEEXTSVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_IMAGEEXTSVC_OK_RUN ) );

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

		// Initialize Route messages
		ROUTE_Start ();

        // Initialiser la com
        dwErr = MainInitPipes();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_PIPES" );
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
                            &gsSvcWork.hEvent, 
                            1000 );
                if ( dwWait != WAIT_TIMEOUT ) break;
                dwDelay = dwDelay + 1000;
                if ( dwDelay >= gsSvcWork.sParmWork.dwMainPolling ) { dwWait = WAIT_TIMEOUT; break; }
            }
        }
        while ( ( dwWait == ( WAIT_OBJECT_0 + 1 ) ) || ( dwWait == WAIT_TIMEOUT ) );

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

		ROUTE_Stop();

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
    DWORD dwLen;

    // Charger les paramètres
    dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwLen );
    if ( dwErr != NO_ERROR )
        return dwErr;

    gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
    gsSvcWork.bReload = FALSE;
	gsSvcWork.hDbCnx = NULL;

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		InitializeCriticalSection(&gsSvcWork.csDBConnectionProtect);

		EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			DBOpen();
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}

    gsSvcWork.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( gsSvcWork.hEvent == NULL )
        return GetLastError();

	if (dwErr != NO_ERROR)
    {
        CloseHandle( gsSvcWork.hEvent );
        return dwErr;
    }

    return NO_ERROR;
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

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			DBCleanup();
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

		DeleteCriticalSection(&gsSvcWork.csDBConnectionProtect);
	}

    CloseHandle( gsSvcWork.hEvent );

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
    char szRemote[MAX_PATH*2], szPipe[MAX_PATH*2];

//	AComOpenTrace(TRUE, TRUE, "C:\\Acom.log" );

    // Ajout du pipe dans la liste des null sessions
    AComSetNullSessionPipe( AUTHSVC_PIPE_CMD, TRUE );

    // INIT WORKING SET

    NTSVCInfo( "MainInitPipes(), creation of working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.sParmWork.dwMaxCmdCnx + 1,
            2,
            THREAD_PRIORITY_ABOVE_NORMAL,
            1000,
            2,
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

	// INIT AUTHORIZATION WORKING SET
	if ( gsSvcWork.sParmWork.dwIsAuthServer || gsSvcWork.sParmWork.dwIsAuthClient )
	{
		NTSVCInfo( "MainInitPipes(), creation of auhtorization working set" );
        gsSvcWork.hAuthWks = AComOpenWorkingSet(
                0,
                gsSvcWork.sParmWork.dwAuthCltCnx + 1,
                2,
                THREAD_PRIORITY_ABOVE_NORMAL,
                1000,
                2,
                ACOM_PROTOCOL_LIFE,
                gsSvcWork.sParmWork.dwLifeTime,
                gsSvcWork.sParmWork.dwMaxLife,
                0,
                CallbackShutdown );
        if ( gsSvcWork.hAuthWks == NULL )
        {
            SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_AUTH_WKS" );
            return ERROR_INVALID_DATA;
        }
	}

    // INIT COMMAND INSTANCE

    NTSVCInfo( "MainInitPipes(), creation of command interface" );
    gsSvcWork.hCmdInst = AComOpenPipeServerInstance(
            gsSvcWork.hWks,
            AUTHSVC_PIPE_CMD,
            AUTHSVC_PIPE_CMD_TYPE,
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
    if ( gsSvcWork.hCmdInst == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_CMD_SRV" );
        AComCloseWorkingSet( gsSvcWork.hWks, AUTHSVC_WORKINGSET_TIMEOUT );
        return ERROR_INVALID_DATA;
    }

	if ( gsSvcWork.sParmWork.dwIsAuthServer )
    {
        // INIT SERVER INSTANCE
        NTSVCInfo( "MainInitPipes(), creation of authentication serveur interface, pipe: %s", AUTHSVC_PIPE_AUTH );
        gsSvcWork.hSrvInst = AComOpenPipeServerInstance(
                gsSvcWork.hAuthWks,                          // hWks
                AUTHSVC_PIPE_AUTH,                           // pcPipeName
                AUTHSVC_PIPE_SERVER_TYPE,                    // dwInstUsrKey
                gsSvcWork.sParmWork.dwTimeToReconnect,       // dwTimeToReconnect
                gsSvcWork.sParmWork.dwAuthCltCnx,            // dwMaxConnections
                gsSvcWork.sParmWork.dwPipeBufferSize,        // dwOutBufferSize
                gsSvcWork.sParmWork.dwPipeBufferSize,        // dwInBufferSize
                gsSvcWork.sParmWork.dwMaxQueuedMessages,     // dwQueueSize
                gsSvcWork.sParmWork.dwMaxMsgSize,            // dwMaxMessageSize
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
        if ( gsSvcWork.hSrvInst == NULL )
        {
            SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_AUTH_SRV" );
            AComCloseWorkingSet( gsSvcWork.hWks, AUTHSVC_WORKINGSET_TIMEOUT );
            return ERROR_INVALID_DATA;
        }
    }

    if ( gsSvcWork.sParmWork.dwIsAuthClient )
    {
        _snprintf_s( szRemote, MAX_PATH*2, sizeof(szRemote), AUTHSVC_PIPE_AUTH_CLT, gsSvcWork.sParmWork.szAuthServer );
        szRemote[sizeof(szRemote)-1] = '\0';

        // INIT CLIENT INSTANCE
        NTSVCInfo( "MainInitPipes(), creation of authentication client interface, pipe: %s", szRemote );
        gsSvcWork.hCltInst = AComOpenPipeClientInstance(
                gsSvcWork.hAuthWks,                          // hWks
                szRemote,                                    // pcPipeName
                AUTHSVC_PIPE_CLIENT_TYPE,                    // dwInstUsrKey
                gsSvcWork.sParmWork.dwSynchroDelay,          // dwTimeToReconnect
                gsSvcWork.sParmWork.dwMaxQueuedMessages,     // dwQueueSize
                gsSvcWork.sParmWork.dwMaxMsgSize,            // dwMaxMessageSize
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
        if ( gsSvcWork.hCltInst == NULL )
        {
            SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_AUTH_CLT" );
            AComCloseWorkingSet( gsSvcWork.hWks, AUTHSVC_WORKINGSET_TIMEOUT );
            return ERROR_INVALID_DATA;
        }
    }

	if( strlen( gsSvcWork.sParmCopy.szRouteServer) != 0 )
	{
		sprintf_s( szPipe, sizeof(szPipe), AUTHSVC_PIPE_COM, gsSvcWork.sParmWork.szRouteServer );

		// INIT CONNECTION TO ROUTE SERVICE
        NTSVCInfo( "MainInitPipes(), creation of route client interface PIPE %s",  szPipe);
        gsSvcWork.hComInst = AComOpenPipeClientInstance(
                gsSvcWork.hWks,
                szPipe,
                AUTHSVC_PIPE_COM_TYPE,
                gsSvcWork.sParmWork.dwTimeToReconnect,
                gsSvcWork.sParmWork.dwMaxQueuedMessages,
                gsSvcWork.sParmWork.dwMaxMsgSize,
                ComCallbackConnection,
                ComCallbackDisconnection,
                ComCallbackReceived,
                ComCallbackSent,
                NULL );
		if ( gsSvcWork.hComInst == NULL )
        {
            SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_LS_CLT" );
            AComCloseWorkingSet( gsSvcWork.hWks, AUTHSVC_WORKINGSET_TIMEOUT );
//			AComCloseTrace();
            return ERROR_INVALID_DATA;
        }
	}

    NTSVCInfo( "MainInitPipes(), all instances are created" );

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
    NTSVCInfo( "MainTerminatePipes(), close communication objects" );

	if ( gsSvcWork.sParmWork.dwIsAuthServer )
        AComCloseInstance( gsSvcWork.hSrvInst );
    if ( gsSvcWork.sParmWork.dwIsAuthClient )
        AComCloseInstance( gsSvcWork.hCltInst );
	if ( strlen( gsSvcWork.sParmCopy.szRouteServer) != 0 )
		AComCloseInstance( gsSvcWork.hComInst );
    AComCloseInstance( gsSvcWork.hCmdInst );

    AComCloseWorkingSet( gsSvcWork.hWks, AUTHSVC_WORKINGSET_TIMEOUT );
	if ( gsSvcWork.sParmWork.dwIsAuthServer || gsSvcWork.sParmWork.dwIsAuthClient )
	{
		AComCloseWorkingSet( gsSvcWork.hAuthWks, AUTHSVC_WORKINGSET_TIMEOUT );
	}

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
 * --------------------------------------------------------------------
 */
PRIVATE void MainTreatment()
{

//    NTSVCInfo( "MainTreatment(), periodical treatment" );

	// Put your procedure here
	// for periodical processing
	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE && gsSvcWork.sParmWork.dwDbKeepConnPeriodMin>0)
	{
		//!!!TO DO protect the  gsSvcWork.hDbCnx; from race thread !!!

		//GetLast DBExecuteStatement time
		unsigned long long ullNow = GetSystemULLTime();
		unsigned long long ullLastDbCnxUse = DBGetLastTimeDbCnxUsed();
		//Check DB connection if no request for more than dwDbKeepConnPeriodMin
		unsigned long long ullDiff = (ullNow > ullLastDbCnxUse) ? ullNow - ullLastDbCnxUse : 0;

		if (ullDiff > (unsigned long long)gsSvcWork.sParmWork.dwDbKeepConnPeriodMin * 60000 * 10000ULL)
		{
			if (DBIfShouldRetry(DBDoDummyDBReq()) == TRUE)
				DBDoDummyDBReq();
		}

	}


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
        printf( "AuthSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( AUTHSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "AuthSvc - Error %u\n", dwErr );
        else
            printf( "AuthSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}


PROTECTED unsigned long long GetSystemULLTime() //UTC time
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return *((unsigned long long*)(&ft));

}