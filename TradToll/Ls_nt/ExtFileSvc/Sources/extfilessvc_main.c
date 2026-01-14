/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : extfilessvc
 * FILE       : extfilessvc_main.c
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
#include <col.h>
#include <reg.h>
#include <csr_evtlog.h>

#include <resource.h>

#include <extfilessvc_files.h>

#define LOC_DEF
#include <extfilessvc_glob.h>
#undef LOC_DEF

#include <extfilessvc_text.h>
#include <extfilessvc_callback.h>
#include <extfilessvc_db.h>

#define LOC_DEF
#include <extfilessvc_main.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning(disable : 4996)

// --------------- MACROS           ----------------------
#define MAIN_CLEANUP_DIR(dir) \
{DWORD len=(DWORD)strlen(dir);if(len>0)if(dir[len-1]=='\\')dir[len-1]='\0';}

// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE void MainTreatment();
PRIVATE DWORD MainPurgeDone( char * pcFile );
PRIVATE DWORD MainPurgeRejected( char * pcFile );
PRIVATE DWORD MainPurge( char * pcFile, char * szSrc, char * szDst );
PRIVATE DWORD WINAPI MainCommand( char * pcParams );
PRIVATE DWORD MainProcessFile( char * pcProcessLib, char * pcOldPath, char * pcNewPath, char * pcProcessParam );
PRIVATE char * MainGetStringItem( char * pcList, char cSep, DWORD dwIndex, char * pcBuffer, DWORD dwSize );
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

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" EXTFILESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Définition des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            EXTFILESSVC_REG_VAL_CMDSVC        , REG_DWORD,        4,                       1, &gsSvcWork.sParmCopy.dwCmdSvc,
            EXTFILESSVC_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,                       5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            EXTFILESSVC_REG_VAL_WORKERS       , REG_DWORD,        4,                       2, &gsSvcWork.sParmCopy.dwWorkers,
            EXTFILESSVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4,                    1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            EXTFILESSVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4,                     256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            EXTFILESSVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,                   16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            EXTFILESSVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,                   16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            EXTFILESSVC_REG_VAL_MAXLIFE       , REG_DWORD,        4,                       3, &gsSvcWork.sParmCopy.dwMaxLife,
            EXTFILESSVC_REG_VAL_LIFETIME      , REG_DWORD,        4,                    5000, &gsSvcWork.sParmCopy.dwLifeTime,
            EXTFILESSVC_REG_VAL_MAINPOLLING   , REG_DWORD,        4,                  300000, &gsSvcWork.sParmCopy.dwMainPolling,
            EXTFILESSVC_REG_VAL_MAXBAKTRIES   , REG_DWORD,        4,                      10, &gsSvcWork.sParmCopy.dwMaxBakTries,
            EXTFILESSVC_REG_VAL_MAXBAKFILES   , REG_DWORD,        4,                      64, &gsSvcWork.sParmCopy.dwMaxBakFiles,
            EXTFILESSVC_REG_VAL_DBUSR         , REG_SZ   , EXTFILESSVC_MAXPATH,               "comuser", gsSvcWork.sParmCopy.szDbUsr,
            EXTFILESSVC_REG_VAL_DBPWD         , REG_SZ   , EXTFILESSVC_MAXPATH,                "compwd", gsSvcWork.sParmCopy.szDbPwd,
            EXTFILESSVC_REG_VAL_DBINST        , REG_SZ   , EXTFILESSVC_MAXPATH,                   "pcs", gsSvcWork.sParmCopy.szDbInst,
            EXTFILESSVC_REG_VAL_INDIRECTORY   , REG_SZ   , EXTFILESSVC_MAXPATH, "C:\\LISTFILES\\IMPORT", gsSvcWork.sParmCopy.szInDir,
            EXTFILESSVC_REG_VAL_OUTDIRECTORY  , REG_SZ   , EXTFILESSVC_MAXPATH,  "C:\\LISTFILES\\LANES", gsSvcWork.sParmCopy.szOutDir,
            EXTFILESSVC_REG_VAL_BAKDIRECTORY  , REG_SZ   , EXTFILESSVC_MAXPATH,"C:\\LISTFILES\\HISTORY", gsSvcWork.sParmCopy.szBakDir,
            EXTFILESSVC_REG_VAL_ERRDIRECTORY  , REG_SZ   , EXTFILESSVC_MAXPATH,                      "", gsSvcWork.sParmCopy.szErrDir,
            EXTFILESSVC_REG_VAL_REJECTDIRECTORY, REG_SZ   , EXTFILESSVC_MAXPATH,                      "", gsSvcWork.sParmCopy.szRejectDir,
            EXTFILESSVC_REG_VAL_EVTLOGERRORS  , REG_DWORD,        4,                       0, &gsSvcWork.sParmCopy.dwEvtLogErrors,
            EXTFILESSVC_REG_VAL_PARSEERRORCODE, REG_DWORD,        4,                      50, &gsSvcWork.sParmCopy.dwParseErrorCode,
            EXTFILESSVC_REG_VAL_ERRORMAXHEAD  , REG_DWORD,        4,                      80, &gsSvcWork.sParmCopy.dwErrorMaxHead,
            EXTFILESSVC_REG_VAL_ERRORMAXLINE  , REG_DWORD,        4,                      62, &gsSvcWork.sParmCopy.dwErrorMaxLine,
            EXTFILESSVC_REG_VAL_SORTBYNAME    , REG_DWORD,        4,                       0, &gsSvcWork.sParmCopy.dwSortByName,
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_EXTFILESSVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_EXTFILESSVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_EXTFILESSVC_OK_RUN ) );

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
            EvtLogReportText( gsSvcWork.hLog, EVTLOG_INFO, 0, 0, "%s", gsSvcWork.sLogStr.szServiceStarted );
        }

        MainLoadMenu();

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

        EvtLogReportText( gsSvcWork.hLog, EVTLOG_INFO, 0, 0, "%s", gsSvcWork.sLogStr.szServiceStopped );
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
    DWORD dwIndex;
    DWORD dwTypeLen;
    DWORD dwMaskLen;
    HCOLLECTIONITEM hItem;
    char szKey[EXTFILESSVC_MAXPATH];
    char szRead[EXTFILESSVC_MAXPATH*3];
    char szType[EXTFILESSVC_MAXPATH*2];
    EXTFILESSVC_FILE sFile;
    NTSVC_PARAMETER_DEF * psLogStrings;
    char szWork[1024];

    psLogStrings = NTSVCOpenParameters( 
            "LogStrServiceStarted", REG_SZ, 256 , "Service started", &gsSvcWork.sLogStr.szServiceStarted,
            "LogStrServiceStopped", REG_SZ, 256 , "Service stopped", &gsSvcWork.sLogStr.szServiceStopped,
            "LogStrProcessSuccess", REG_SZ, 256 , "SUCCESS", &gsSvcWork.sLogStr.szProcessSuccess,
            "LogStrProcessParse", REG_SZ, 256 , "WARNING", &gsSvcWork.sLogStr.szProcessParse,
            "LogStrProcessErrorInt", REG_SZ, 256 , "IMPORT DATA ERROR", &gsSvcWork.sLogStr.szProcessErrorInt,
            "LogStrProcessErrorExt", REG_SZ, 256 , "IMPORT REFERENCE ERROR", &gsSvcWork.sLogStr.szProcessErrorExt,
            NULL );
    if ( psLogStrings == NULL )
        return ERROR_INVALID_DATA;

    dwErr = NTSVCLoadParameters( psLogStrings, &dwLen );
    NTSVCCloseParameters( psLogStrings );
    if ( dwErr != NO_ERROR )
        return dwErr;

    // Initialiser les requètes base
    DBInitRequests();

    // Charger les paramètres
    dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwLen );
    if ( dwErr != NO_ERROR )
        return dwErr;

    gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
    gsSvcWork.bReload = FALSE;
    gsSvcWork.bDatabaseMissing = TRUE;
    MAIN_CLEANUP_DIR(gsSvcWork.sParmWork.szInDir);
    MAIN_CLEANUP_DIR(gsSvcWork.sParmWork.szOutDir);

    dwErr = EvtLogCreateSource( RESINFO_PRODUCT, &gsSvcWork.hLog );
    if ( dwErr != NO_ERROR )
        return dwErr;

    gsSvcWork.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( gsSvcWork.hEvent == NULL )
    {
        dwErr = GetLastError(); 
        EvtLogCloseSource( gsSvcWork.hLog );
        return dwErr;
    }

    // Creation de la collection des fichiers à traiter
    gsSvcWork.hFiles = ColCreate( COL_INDEX_ZSTRING, EXTFILESSVC_MAXPATH, TRUE, FALSE );
    if ( gsSvcWork.hFiles == NULL )
    {
        CloseHandle( gsSvcWork.hEvent );
        EvtLogCloseSource( gsSvcWork.hLog );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Charger les types de fichiers à traiter, nécéssitant une simple copie
    dwIndex = 0;
    sprintf_s( szKey, sizeof(szKey), "%s\\%s", EXTFILESSVC_REG_KEY, EXTFILESSVC_REG_KEY_COPYFILES );
    do
    {
        ZeroMemory( &sFile, sizeof(sFile) );
        dwTypeLen = sizeof(szType);
        dwMaskLen = sizeof(szRead);
        dwErr = REG_Enum_Valeurs_Chaine( EXTFILESSVC_REG_ROOT, szKey, dwIndex, szType, &dwTypeLen, szRead, &dwMaskLen );
        if ( dwErr == NO_ERROR )
        {
            MainGetStringItem( szType, '|', 0, sFile.szType, sizeof( sFile.szType ) );
            MainGetStringItem( szType, '|', 1, sFile.szTypeDB, sizeof( sFile.szTypeDB ) );

            if ( strlen( sFile.szType ) > 0 )
            {
                if ( strlen( sFile.szTypeDB ) == 0 )
                    strcpy_s( sFile.szTypeDB, sizeof(sFile.szTypeDB), sFile.szType );

                MainGetStringItem( szRead, '|', 0, sFile.szInMask, sizeof( sFile.szInMask ) );
                MainGetStringItem( szRead, '|', 1, sFile.szProcessLib, sizeof( sFile.szProcessLib ) );
                MainGetStringItem( szRead, '|', 2, sFile.szProcessParam, sizeof( sFile.szProcessParam ) );
                sFile.bImportData = FALSE;
     
                sprintf_s( szWork, sizeof(szWork), "%s_%s", RESINFO_PRODUCT, sFile.szType );

                dwErr = EvtLogCreateSource( szWork, &sFile.hLog );
                if ( dwErr != NO_ERROR )
                {
                    hItem = COL_SCAN_BEGIN;
                    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
                        EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
                    ColDestroy( gsSvcWork.hFiles );
                    CloseHandle( gsSvcWork.hEvent );
                    EvtLogCloseSource( gsSvcWork.hLog );
                    return dwErr;
                }

                hItem = ColItemAdd( gsSvcWork.hFiles, sFile.szType, &sFile, sizeof( sFile ) );
                if ( hItem == NULL )
                {
                    EvtLogCloseSource( sFile.hLog );
                    hItem = COL_SCAN_BEGIN;
                    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
                        EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
                    ColDestroy( gsSvcWork.hFiles );
                    CloseHandle( gsSvcWork.hEvent );
                    EvtLogCloseSource( gsSvcWork.hLog );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            dwIndex ++;
        }
    }
    while ( dwErr == NO_ERROR );
    if ( ( dwErr != ERROR_FILE_NOT_FOUND ) && ( dwErr != ERROR_NO_MORE_ITEMS ) )
    {
        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
            EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
        ColDestroy( gsSvcWork.hFiles );
        CloseHandle( gsSvcWork.hEvent );
        EvtLogCloseSource( gsSvcWork.hLog );
        return dwErr;
    }


    // Charger les types de fichiers à traiter, nécéssitant une intégration en base
    dwIndex = 0;
    sprintf_s( szKey, sizeof(szKey), "%s\\%s", EXTFILESSVC_REG_KEY, EXTFILESSVC_REG_KEY_IMPORTFILES );
    do
    {
        dwTypeLen = sizeof(szType);
        dwMaskLen = sizeof(sFile.szInMask);
        dwErr = REG_Enum_Valeurs_Chaine( EXTFILESSVC_REG_ROOT, szKey, dwIndex, szType, &dwTypeLen, sFile.szInMask, &dwMaskLen );
        if ( dwErr == NO_ERROR )
        {
            MainGetStringItem( szType, '|', 0, sFile.szType, sizeof( sFile.szType ) );
            MainGetStringItem( szType, '|', 1, sFile.szTypeDB, sizeof( sFile.szTypeDB ) );

            if ( strlen( sFile.szType ) > 0 )
            {
                if ( strlen( sFile.szTypeDB ) == 0 )
                    strcpy_s( sFile.szTypeDB, sizeof(sFile.szTypeDB), sFile.szType );

                sFile.bImportData = TRUE;

                sprintf_s( szWork, sizeof(szWork), "%s_%s", RESINFO_PRODUCT, sFile.szType );

                dwErr = EvtLogCreateSource( szWork, &sFile.hLog );
                if ( dwErr != NO_ERROR )
                {
                    hItem = COL_SCAN_BEGIN;
                    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
                        EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
                    ColDestroy( gsSvcWork.hFiles );
                    CloseHandle( gsSvcWork.hEvent );
                    EvtLogCloseSource( gsSvcWork.hLog );
                    return dwErr;
                }

                hItem = ColItemAdd( gsSvcWork.hFiles, sFile.szType, &sFile, sizeof( sFile ) );
                if ( hItem == NULL )
                {
                    hItem = COL_SCAN_BEGIN;
                    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
                        EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
                    ColDestroy( gsSvcWork.hFiles );
                    CloseHandle( gsSvcWork.hEvent );
                    EvtLogCloseSource( gsSvcWork.hLog );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            dwIndex ++;
        }
    }
    while ( dwErr == NO_ERROR );
    if ( ( dwErr != ERROR_FILE_NOT_FOUND ) && ( dwErr != ERROR_NO_MORE_ITEMS ) )
    {
        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
            EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
        ColDestroy( gsSvcWork.hFiles );
        CloseHandle( gsSvcWork.hEvent );
        EvtLogCloseSource( gsSvcWork.hLog );
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
    HCOLLECTIONITEM hItem;

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
        EvtLogCloseSource( ((EXTFILESSVC_FILE*)(ColItemData(hItem)))->hLog );
    ColDestroy( gsSvcWork.hFiles );
    CloseHandle( gsSvcWork.hEvent );
    EvtLogCloseSource( gsSvcWork.hLog );

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
    // Ajout du pipe dans la liste des null sessions
    AComSetNullSessionPipe( EXTFILESSVC_PIPE_CMD, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.sParmWork.dwMaxCmdCnx,
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
                    EXTFILESSVC_PIPE_CMD,
                    0,
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
                    0,                                          // dwInstUsrKey
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
            AComCloseWorkingSet( gsSvcWork.hWks, EXTFILESSVC_WORKINGSET_TIMEOUT );
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

    AComCloseWorkingSet( gsSvcWork.hWks, EXTFILESSVC_WORKINGSET_TIMEOUT );

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
    DWORD dwCount;
    DWORD dwIndex;
    DWORD dwSize;
    DWORD dwRead;
    DWORD dwErr;
    HCOLLECTIONITEM hItem;
    EXTFILESSVC_FILE * psMasks;
    WIN32_FIND_DATA * psFiles;
    DB_CNX * hCnx = NULL;
    HANDLE * hFile;
    BOOL fRejected;
    char * pcBuffer;
    char szNewFile[EXTFILESSVC_MAXPATH];
    char szNewPath[EXTFILESSVC_MAXPATH];
    char szOldPath[EXTFILESSVC_MAXPATH];
    char szErrPath[EXTFILESSVC_MAXPATH];
    char szWork[2048];

    NTSVCInfo( "MainTreatment(), traitement des fichiers détectés" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hFiles, &hItem ) )
    {
        psMasks = (EXTFILESSVC_FILE*)ColItemData(hItem);

        // Obtenir la liste classée des fichiers vérifiant le masque
        psFiles = FileListOpen( gsSvcWork.sParmWork.szInDir, psMasks->szInMask, &dwCount );

        for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
        {
            // Composer le chemin complet du fichier
            _snprintf_s( szOldPath, sizeof(szOldPath), sizeof(szOldPath), "%s\\%s", gsSvcWork.sParmWork.szInDir, psFiles[dwIndex].cFileName );
            szOldPath[sizeof(szOldPath)-1] = '\0';

            if ( psMasks->bImportData )
            {
                NTSVCInfo( "MainTreatment(), type %s (à importer): fichier %s", psMasks->szType, psFiles[dwIndex].cFileName );

                // Ouverture du fichier en lecture uniquement mais avec accès exclusif lecture/écriture
                hFile = CreateFile( 
                            szOldPath, 
                            GENERIC_READ, 
                            0, 
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL );
                if ( hFile == INVALID_HANDLE_VALUE )
                {
                    dwErr = GetLastError();
                    SVC_ERR( dwErr, "ERR_FILE_IO" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                    break;
                }

                // Déterminer la taille du fichier pour allocation
                dwSize = GetFileSize( hFile, NULL );
                if ( dwSize == 0xFFFFFFFF )
                {
                    dwErr = GetLastError();
                    CloseHandle( hFile );
                    SVC_ERR( dwErr, "ERR_FILE_IO" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                    break;
                }

                // Allouer le buffer pour lire le fichier en un seul bloc
                pcBuffer = HeapAlloc( GetProcessHeap(), 0, dwSize );
                if ( pcBuffer == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    CloseHandle( hFile );
                    SVC_ERR( dwErr, "ERR_FILE_MEMORY" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                    break;
                }

                // Lire les données du fichier en un seul bloc
                if ( ! ReadFile( hFile, pcBuffer, dwSize, &dwRead, NULL ) )
                {
                    dwErr = GetLastError();
                    CloseHandle( hFile );
                    HeapFree( GetProcessHeap(), 0, pcBuffer );
                    SVC_ERR( dwErr, "ERR_FILE_IO" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;
                    
                    break;
                }
                if ( dwRead != dwSize )
                {
                    dwErr = GetLastError();
                    CloseHandle( hFile );
                    HeapFree( GetProcessHeap(), 0, pcBuffer );
                    SVC_ERR( dwErr, "ERR_FILE_IO" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;
                    
                    break;
                }

                // On a plus besoin d'accéder au fichier
                CloseHandle( hFile );

                // Construire le nom du fichier d'erreur (utilisé uniquement si nécessaire)
                if ( gsSvcWork.sParmWork.szErrDir[0] != 0 )
                {
                    _snprintf_s( szErrPath, sizeof(szErrPath), sizeof(szErrPath), "%s\\%s.ERR", gsSvcWork.sParmWork.szErrDir, psFiles[dwIndex].cFileName );
                    szErrPath[sizeof(szErrPath)-1] = 0;
                }
                else
                    szErrPath[0] = 0;

                // Signaler à la base l'arrivée d'une nouvelle liste
                dwErr = DBImportList( &hCnx, psMasks->szTypeDB, pcBuffer, dwSize, psFiles[dwIndex].cFileName, szErrPath, psMasks->hLog );
                if ( dwErr != NO_ERROR )
                {
                    HeapFree( GetProcessHeap(), 0, pcBuffer );
                    SVC_ERR( dwErr, "ERR_FILE_IMPORT" );

                    fRejected = FALSE;

                    if ( dwErr == 0xFFFFFFFF )
                        if ( gsSvcWork.sParmWork.szRejectDir[0] != 0 )
                            fRejected = MainPurgeRejected( psFiles[dwIndex].cFileName );

                    if ( ! fRejected )
                    {
                        strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                        psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;
                    }

                    break;
                }
        
                if ( psMasks->szBlockedFile[0] != 0 )
                {
                    if ( ! DBFormatText( gsSvcWork.sLogStr.szProcessSuccess, szWork, sizeof(szWork), psMasks->szBlockedFile, szOldPath, NULL ) )
                    {
                        _snprintf_s( szWork, sizeof(szWork), sizeof(szWork), "%s\r\n  [1] : %s\r\n  [2] : %s", gsSvcWork.sLogStr.szProcessErrorInt, psMasks->szBlockedFile, szOldPath );
                        szWork[sizeof(szWork)-1] = 0;
                    }
                    EvtLogReportText( psMasks->hLog,
                                      EVTLOG_SUCCESS,
                                      0,
                                      0,
                                      "%s", szWork );

                    psMasks->szBlockedFile[0] = 0;
                }

                // Libérer la mémoire allouée
                HeapFree( GetProcessHeap(), 0, pcBuffer );

                // Purger le fichier source une fois intégré
                dwErr = MainPurgeDone( psFiles[dwIndex].cFileName );
                if ( dwErr != NO_ERROR  )
                {
                    SVC_ERR( dwErr, "ERR_FILE_DELETE" );
                    break;
                }
            }
            else
            {
                NTSVCInfo( "MainTreatment(), type %s (à déplacer): fichier %s", psMasks->szType, psFiles[dwIndex].cFileName );
                
                // Interroger la base sur le nom à lui donner
                dwErr = DBGetExternalFileName( &hCnx, psMasks->szTypeDB, szNewFile, sizeof( szNewFile ) );
                if ( dwErr != NO_ERROR )
                {
                    SVC_ERR( dwErr, "ERR_GET_EXT_FILE" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                    break;
                }

                // Composer le nouveau chemin
                _snprintf_s( szNewPath, sizeof(szNewPath), sizeof(szNewPath), "%s\\%s", gsSvcWork.sParmWork.szOutDir, szNewFile );
                szNewPath[sizeof(szNewPath)-1] = '\0';

                // Traiter le fichier
                // Si pas de DLL définie pour le traitement du fichier
                if ( strlen(psMasks->szProcessLib) == 0 )
                {
                    // On fait une copie
                    if ( ! CopyFile( szOldPath, szNewPath, TRUE ) )
                    {
                        dwErr = GetLastError();
                        SVC_ERR( dwErr, "ERR_FILE_COPY" );

                        strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                        psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                        break;
                    }
                }
                else
                {
                    // Une DLL est définie pour le traitement
                    dwErr = MainProcessFile( psMasks->szProcessLib, szOldPath, szNewPath, psMasks->szProcessParam );
                    if ( dwErr != NO_ERROR )
                    {
                        SVC_ERR( dwErr, "ERR_FILE_PROCESSING" );

                        strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                        psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                        break;
                    }
                }
                

                // Acquiter le fichier auprès de la base
                dwErr = DBSetExternalFileName( &hCnx, psMasks->szTypeDB, szNewFile );
                if ( dwErr != NO_ERROR )
                {
                    DeleteFile( szNewPath );
                    SVC_ERR( dwErr, "ERR_SET_EXT_FILE" );

                    strncpy_s( psMasks->szBlockedFile, EXTFILESSVC_MAXPATH, szOldPath, sizeof(psMasks->szBlockedFile) );
                    psMasks->szBlockedFile[sizeof(psMasks->szBlockedFile)-1] = 0;

                    break;
                }

                if ( psMasks->szBlockedFile[0] != 0 )
                {
                    if ( ! DBFormatText( gsSvcWork.sLogStr.szProcessSuccess, szWork, sizeof(szWork), psMasks->szBlockedFile, szOldPath, NULL ) )
                    {
                        _snprintf_s( szWork, sizeof(szWork), sizeof(szWork), "%s\r\n  [1] : %s\r\n  [2] : %s", gsSvcWork.sLogStr.szProcessErrorInt, psMasks->szBlockedFile, szOldPath );
                        szWork[sizeof(szWork)-1] = 0;
                    }
                    EvtLogReportText( psMasks->hLog,
                                      EVTLOG_SUCCESS,
                                      0,
                                      0,
                                      "%s", szWork );

                    psMasks->szBlockedFile[0] = 0;
                }

                // Purger le fichier source une fois copié
                dwErr = MainPurgeDone( psFiles[dwIndex].cFileName );
                if ( dwErr != NO_ERROR )
                {
                    SVC_ERR( dwErr, "ERR_FILE_DELETE" );
                    break;
                }
            }
            NTSVCError( "AVERTISSEMENT : Le fichier %s a été traité", psFiles[dwIndex].cFileName );
        }

        if ( psFiles != NULL )
            FileListClose( psFiles );
    }

    if ( hCnx != NULL )
        DBDisconnect( hCnx );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainPurgeDone( char * pcFile )
 * PARAMETERS: pcFile : Nom du fichier à purger
 * RETURN    : Erreur Win 32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Déplace un fichier dans le répertoire d'archive des fichiers
 *             traités en lui donnant un nouveau nom.
 *             Effectue une purge des fichiers les plus anciens.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainPurgeDone( char * pcFile )
{
    NTSVCInfo( "MainPurgeDone(), fichier %s", pcFile );

    return MainPurge( pcFile, gsSvcWork.sParmWork.szInDir, gsSvcWork.sParmWork.szBakDir );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainPurgeRejected( char * pcFile )
 * PARAMETERS: pcFile : Nom du fichier à purger
 * RETURN    : Erreur Win 32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Déplace un fichier dans le répertoire d'archive des
 *             fichier rejetés en lui donnant un nouveau nom.
 *             Effectue une purge des fichiers les plus anciens.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainPurgeRejected( char * pcFile )
{
    NTSVCInfo( "MainPurgeRejected(), fichier %s", pcFile );

    return MainPurge( pcFile, gsSvcWork.sParmWork.szInDir, gsSvcWork.sParmWork.szRejectDir );
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainPurge( char * pcFile, char * szSrc, char * szDst )
 * PARAMETERS: pcFile : Nom du fichier à purger
 *             szSrc  : Source dir
 *             szDst  : Destination dir
 * RETURN    : Erreur Win 32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Déplace un fichier dans le répertoire d'archive
 *             en lui donnant un nouveau nom.
 *             Effectue une purge des fichiers les plus anciens.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainPurge( char * pcFile, char * szSrc, char * szDst )
{
    DWORD dwErr;
    DWORD dwIndex;
    DWORD dwCount;
    WIN32_FIND_DATA * psFiles;
    SYSTEMTIME sTime;
    char szOldPath[EXTFILESSVC_MAXPATH];
    char szNewPath[EXTFILESSVC_MAXPATH];

    // Composer l'ancien chemin
    _snprintf_s( szOldPath, sizeof(szOldPath), sizeof(szOldPath), "%s\\%s", szSrc, pcFile );
    szOldPath[sizeof(szOldPath)-1] = '\0';

    dwIndex = gsSvcWork.sParmWork.dwMaxBakTries;
    while ( dwIndex != 0 )
    {
        GetLocalTime( &sTime );

        // Composer le nouveau chemin
        _snprintf_s( szNewPath, sizeof(szNewPath), sizeof(szNewPath), "%s\\%04u%02u%02u%02u%02u%02u.%s",
                szDst,
                (DWORD)sTime.wYear,
                (DWORD)sTime.wMonth,
                (DWORD)sTime.wDay,
                (DWORD)sTime.wHour,
                (DWORD)sTime.wMinute,
                (DWORD)sTime.wSecond,
                pcFile );
        szNewPath[sizeof(szNewPath)-1] = '\0';

        if ( MoveFileEx( szOldPath, szNewPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ) )
            break;

        // Attendre la seconde suivante
        Sleep(1000);
        dwIndex --;
    }

    if ( dwIndex == 0 )
    {
        // Les n tentative de déplacement ont échouées
        dwErr = GetLastError();
        SVC_ERR( dwErr, "ERR_FILE_MOVE" );
        return dwErr;
    }
    
    // obtenir la liste triée des fichiers archivés
    psFiles = FileListOpen( szDst, "*", &dwCount );
    if ( dwCount > gsSvcWork.sParmWork.dwMaxBakFiles )
        for ( dwIndex = 0 ; dwIndex < ( dwCount - gsSvcWork.sParmWork.dwMaxBakFiles ) ; dwIndex ++ )
        {
             // Composer le chemin
            _snprintf_s( szOldPath, sizeof(szOldPath), sizeof(szOldPath), "%s\\%s", szDst, psFiles[dwIndex].cFileName );
            szOldPath[sizeof(szOldPath)-1] = '\0';
            NTSVCError( "AVERTISSEMENT : Le fichier d'archive %s a été supprimé de %s", psFiles[dwIndex].cFileName, szDst );
            DeleteFile( szOldPath );
        }
  
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
        printf( "ExtFilesSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( EXTFILESSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "ExtFilesSvc - Error %u\n", dwErr );
        else
            printf( "ExtFilesSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * MainGetStringItem( char * pcList, char cSep, DWORD dwIndex, char * pcBuffer, DWORD dwSize )
 * PARAMETERS: pcList  : liste texte avec items séparés par cSep
 *             cSep    : caractère séparateur
 *             dwIndex : Index de l'élément dans la liste (1er = 0)
 *             pcBuffer: Buffer ou place l'item lu
 *             dwSize  : Taille du buffer. Si l'item est plus long, il est tronqué. Un marque de fin de chaine ('\0')
 *                       est systématiquement placé en fin).
 * RETURN    : pcBuffer
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Extrait les élément d'une liste texte.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainProcessFile( char * pcProcessLib, char * pcOldPath, char * pcNewPath, char * pcProcessParam )
{
    DWORD dwErr;
    HINSTANCE hLib;
    EXTFILESSVC_PROCESS_FILE * pfProcess;

    hLib = LoadLibrary( pcProcessLib );
    if ( hLib == NULL )
    {
        dwErr = GetLastError();
        SVC_ERR( dwErr, "ERR_LOAD_EXT_DLL" );
        return dwErr;
    }

    (FARPROC)pfProcess = GetProcAddress( hLib, EXTFILESSVC_DLL_FUNCTION_NAME );
    if ( pfProcess == NULL )
    {
        dwErr = GetLastError();
        SVC_ERR( dwErr, "ERR_EXT_DLL_ENTRY" );
        FreeLibrary( hLib );
        return dwErr;
    }

    dwErr = pfProcess( pcOldPath, pcNewPath, pcProcessParam );
    if ( dwErr != NO_ERROR )
    {
        SVC_ERR( dwErr, "ERR_EXT_DLL_RUN" );
    }
    FreeLibrary( hLib );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * MainGetStringItem( char * pcList, char cSep, DWORD dwIndex, char * pcBuffer, DWORD dwSize )
 * PARAMETERS: pcList  : liste texte avec items séparés par cSep
 *             cSep    : caractère séparateur
 *             dwIndex : Index de l'élément dans la liste (1er = 0)
 *             pcBuffer: Buffer ou place l'item lu
 *             dwSize  : Taille du buffer. Si l'item est plus long, il est tronqué. Un marque de fin de chaine ('\0')
 *                       est systématiquement placé en fin).
 * RETURN    : pcBuffer
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Extrait les élément d'une liste texte.
 * --------------------------------------------------------------------
 */
PRIVATE char * MainGetStringItem( char * pcList, char cSep, DWORD dwIndex, char * pcBuffer, DWORD dwSize )
{
    DWORD dwPos = 0;
    char * pcCurrent = pcList;

    while ( ( dwIndex > 0 ) && ( pcList != NULL ) )
    {
        pcList = strchr( pcList, cSep );
        if ( pcList != NULL )
        {
            dwIndex --;
            pcList ++;
        }
    }

    if ( pcList != NULL )
    {
        while ( ( dwPos < ( dwSize - 1 ) ) && ( *pcList != cSep ) && ( *pcList != '\0' ) )
            pcBuffer[dwPos++] = *(pcList++);
    }
    pcBuffer[dwPos] = '\0';

    return pcBuffer;
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
                        EXTFILESSVC_SERVICE_NAME "\\" 
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



