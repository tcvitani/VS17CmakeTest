/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_main.c
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
#include <col.h>
#include <reg.h>
#include <csr_excpt.h>

#include <cm_res.h>

#include <cmhost.h>
#include <cmwork.h>

#define LOC_DEF
#include <cm_glob.h>
#undef LOC_DEF

#include <cm_text.h>
#include <cm_callback.h>
#include <cm_hosts.h>
#include <cm_dirs.h>
#include <cm_workers.h>
#include <cm_threads.h>
#include <cm_utl.h>

#define LOC_DEF
#include <cm_main.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
PRIVATE DWORD MainLoadConfig();
PRIVATE void MainUnloadConfig();
PRIVATE DWORD MainStartThreads();
PRIVATE void MainStopThreads();
PRIVATE void WINAPI MainLoadMenu();
PRIVATE void WINAPI MainUnloadMenu();
PRIVATE void WINAPI MainTrayIconHandler( UINT uiId );



PRIVATE DWORD gdwBlink = 0;


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
    (*ppfCommand) = (NTSVCCommandMain*)NULL;
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
    DWORD dwIndex;
    DWORD dwRes;

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    gsSvcWork.dwTLS = TlsAlloc();

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" CM_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Définition des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            CM_REG_VAL_CMDSVC        , REG_DWORD,        4,                  1, &gsSvcWork.sParmCopy.dwCmdSvc,
            CM_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,                  5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            CM_REG_VAL_WORKERS       , REG_DWORD,        4,                  2, &gsSvcWork.sParmCopy.dwWorkers,
            CM_REG_VAL_RECONNECTTIME , REG_DWORD,        4,               1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            CM_REG_VAL_QUEUESIZE     , REG_DWORD,        4,                256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            CM_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,              16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            CM_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,              16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            CM_REG_VAL_MAXLIFE       , REG_DWORD,        4,                  5, &gsSvcWork.sParmCopy.dwMaxLife,
            CM_REG_VAL_LIFETIME      , REG_DWORD,        4,               5000, &gsSvcWork.sParmCopy.dwLifeTime,
            NULL );
    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk[ 0]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK00 ) );
    gsSvcWork.hIconOk[ 1]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK01 ) );
    gsSvcWork.hIconOk[ 2]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK02 ) );
    gsSvcWork.hIconOk[ 3]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK03 ) );
    gsSvcWork.hIconOk[ 4]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK04 ) );
    gsSvcWork.hIconOk[ 5]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK05 ) );
    gsSvcWork.hIconOk[ 6]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK06 ) );
    gsSvcWork.hIconOk[ 7]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK07 ) );
    gsSvcWork.hIconOk[ 8]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK08 ) );
    gsSvcWork.hIconOk[ 9]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK09 ) );
    gsSvcWork.hIconOk[10]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK10 ) );
    gsSvcWork.hIconOk[11]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK11 ) );
    gsSvcWork.hIconOk[12]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK12 ) );
    gsSvcWork.hIconOk[13]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK13 ) );
    gsSvcWork.hIconOk[14]    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_OK14 ) );
    gsSvcWork.hIconPaused[0] = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_PS00 ) );
    gsSvcWork.hIconPaused[1] = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_PS01 ) );
    gsSvcWork.hIconKo        = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CM_KO ) );

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
            SVC_ERR( dwErr, "ERR_SERVICE_INIT" );
            break;
        }

        // Charger la config des hosts et links et les initialiser
        dwErr = MainLoadConfig();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_CONFIG_INIT" );
            MainTerminateService();
            break;
        }

        // Initialiser la com
        dwErr = MainInitPipes();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_PIPES_INIT" );
            MainUnloadConfig();
            MainTerminateService();
            break;
        }

        gsSvcWork.fPaused = FALSE;

        // Lancer les threads
        dwErr = MainStartThreads();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_THREADS_INIT" );
            MainTerminatePipes();
            MainUnloadConfig();
            MainTerminateService();
            break;
        }

        // Dans le cas de la premiere boucle, on passe a l'état "démarré"
        if ( dwLoadCount == 0 )
        {
            // Passer dans l'état "running"
            NTSVCSetCurrentState( SERVICE_RUNNING, 0, NO_ERROR );
            NTSVCInfo( TextFind( gsSvcWork.hText, "SVC_STARTED", "SVC_STARTED" ) );
        }

        MainLoadMenu();

        // Les traitements ont été lancés avec les workers

        NTSVCInfo( "NTSVCExternalMain(), démarrage de la boucle principale" );
        
        // Attendre la demande d'arrêt ou la fin d'un des threads
        while ( TRUE )
        {
            dwRes = NTSVCWaitForEndOrMultipleObjects( 
                gsSvcWork.dwThreads,
                gsSvcWork.thThreads,
                300 );
            if ( dwRes != WAIT_TIMEOUT )
                break;

            gdwBlink = ( gdwBlink + 1 ) % 15;
            if ( ! gsSvcWork.fPaused )
                NTSVCRefreshTrayIcon( gsSvcWork.hIconOk[gdwBlink], "SVC_RUNNING" );
            else
                NTSVCRefreshTrayIcon( gsSvcWork.hIconPaused[(gdwBlink/4)%2], "SVC_PAUSED" );
        }

        MainUnloadMenu();

        if ( dwRes != WAIT_OBJECT_0 )
        {
            // L'arret n'est pas sur demande.
            SVC_ERR( dwErr, "ERR_THREAD_END" );
        }

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

        // S'assurer que tout le monde est arrété
        NTSVCInfo( "NTSVCExternalMain(), arret des threads" );
        NTSVCSignalEnd();
        MainStopThreads();

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

        NTSVCInfo( "NTSVCExternalMain(), destruction des hosts" );
        MainUnloadConfig();

        NTSVCError( TextFind( gsSvcWork.hText, "SVC_STOPPED", "SVC_STOPPED" ) );
        MainTerminateService();

        NTSVCRefreshTrayIcon( NULL, NULL );

        dwLoadCount ++;
    }
    while ( gsSvcWork.bReload );

    for ( dwIndex = 0 ; dwIndex < 15 ; dwIndex ++ )
        DestroyIcon( gsSvcWork.hIconOk[dwIndex] );
    DestroyIcon( gsSvcWork.hIconKo );

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
        SVC_ERR( dwErr, "ERR_SERVICE_PARAM" );
        return dwErr;
    }
    gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
    gsSvcWork.bReload = FALSE;

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
    // Ajout du pipe dans la liste des null session
    AComSetNullSessionPipe( CM_PIPE_CMD, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.sParmWork.dwMaxCmdCnx,
            1,
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
        SVC_ERR_S( ERROR_PIPE_NOT_CONNECTED, "ERR_COMM_INIT", "WKS" );
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE L'INSTANCE DE COMMANDE

    NTSVCInfo( "MainInitPipes(), création du serveur interface de commande" );
    if ( gsSvcWork.sParmWork.dwCmdSvc != 0 )
    {
        if ( gsSvcWork.sParmWork.dwCmdSvc == 1 )
            gsSvcWork.hCmdInst = AComOpenPipeServerInstance(
                    gsSvcWork.hWks,
                    CM_PIPE_CMD,
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
                    0,                                        // dwInstUsrKey
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
            SVC_ERR_S( ERROR_PIPE_NOT_CONNECTED, "ERR_COMM_INIT", "CMDINST" );
            AComCloseWorkingSet( gsSvcWork.hWks, CM_WORKINGSET_TIEMOUT );
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

    AComCloseWorkingSet( gsSvcWork.hWks, CM_WORKINGSET_TIEMOUT );

    return NO_ERROR;
}




PRIVATE DWORD MainLoadConfig()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    DWORD dwIndex;
    DWORD dwScan;
    BOOL bStop;
    HKEY hKeyConfig = NULL;
    CM_HOST * psHost;
    CM_LINK * psLink;
    CM_WORKER * psWorker;
    CM_SORT sSortParams;
    char szKey [CM_MAX_STR];
    char szName[CM_MAX_STR];
    char szDll [CM_MAX_STR];
    char szHost[CM_MAX_STR];
    char szPath[CM_MAX_STR];
    char szSort[CM_MAX_STR];

#define LIRE_ENTIER(var,chemin,nom) \
    dwErr = REG_Lire_Entier( CM_REG_ROOT, (chemin), (nom), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", (chemin), (nom) );\
        __leave;\
    }\
    (var) = dwSize

#define DEF_ENTIER(var,chemin,nom,def) \
    dwErr = REG_Defaut_Entier( CM_REG_ROOT, (chemin), (nom), &dwSize, (def) );\
    if ( dwErr != NO_ERROR )\
        (var) = (def);\
    else\
        (var) = dwSize

#define LIRE_CHAINE(var,chemin,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( CM_REG_ROOT, (chemin), (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", (chemin), (nom) );\
        __leave;\
    }\
    else

#define DEF_CHAINE(var,chemin,nom,def) \
    dwSize = sizeof(var);\
    dwErr = REG_Defaut_Chaine( CM_REG_ROOT, (chemin), (nom), (var), &dwSize, (def) );\
    if ( dwErr != NO_ERROR )\
    {\
        strcpy_s( (var), dwSize, (def) ); \
        dwErr = NO_ERROR;\
    }\
    else

    __try
    {
        NTSVCInfo( "MainLoadConfig(), Début" );

        gsSvcWork.dwHosts = 0;
        ZeroMemory( gsSvcWork.tsHosts, sizeof(gsSvcWork.tsHosts) );
        gsSvcWork.psLocalHost = NULL;

        gsSvcWork.dwLinks = 0;
        ZeroMemory( gsSvcWork.tsLinks, sizeof(gsSvcWork.tsLinks) );

        ZeroMemory( &gsSvcWork.sDirEA, sizeof( gsSvcWork.sDirEA ) );

        gsSvcWork.dwWorkers = 0;
        ZeroMemory( gsSvcWork.tsWorkers, sizeof(gsSvcWork.tsWorkers) );

        //
        // -------------------------------------------------------
        // CHARGEMENT DES HOSTS
        // -------------------------------------------------------
        //

        NTSVCInfo( "MainLoadConfig(), Chargement des hosts" );

        bStop = FALSE;
        for ( dwIndex = 0 ; dwIndex < CM_MAX_HOSTS ; dwIndex ++ )
        {
            //
            // Pointer sur le prochain host
            //
            psHost = &gsSvcWork.tsHosts[dwIndex];

            //
            // Récupérer le nom d'host suivant.
            //
            dwSize = sizeof(szName);
            dwErr = REG_Enum_Cles( CM_REG_ROOT, CM_REG_KEY_HOSTS, dwIndex, szName, &dwSize );
            if ( dwErr != NO_ERROR )
            {
                //
                // On a atteint la fin de l'énumération, on peut ajouter l'host local
                // à la liste des hosts.
                //
				strcpy_s(szName, sizeof(szName), "LocalHost");
				strcpy_s(szKey, sizeof(szKey), "");
				strcpy_s(szDll, sizeof(szDll), "");
                bStop = TRUE;
            }
            else
            {
                //
                // On s'assure que le nom utilisé n'est pas celui qui
                // sera pris pour l'host local.
                //
                if ( _stricmp( szName, "LocalHost" ) == 0 )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", szKey, "LocalHost" );
                    __leave;
                }

                //
                // Construire le chemin complet de la clé.
                //
                CM_STRNCPY( szKey, CM_REG_KEY_HOSTS, sizeof(szKey) );
                CM_STRNCAT( szKey, "\\"            , sizeof(szKey) );
                CM_STRNCAT( szKey, szName          , sizeof(szKey) );
            }

            //
            // Ouvrir l'accés au registre pour transmettre le handle
            // à l'init du host.
            //
            if ( szKey[0] != 0 )
            {
                dwErr = REG_Ouvrir_Access( NULL, CM_REG_ROOT, szKey, &hKeyConfig, KEY_ALL_ACCESS );
                if ( dwErr != NO_ERROR )
                {
                    SVC_ERR_S( dwErr, "ERR_CONFIG_PARAM", szKey );
                    __leave;
                }
            }
            else
                hKeyConfig = NULL;

            //
            // Initialiser le host correspondant dans le tableau
            //
            if ( ! CMHostInit( psHost, szName, hKeyConfig ) )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_CONFIG_HOST", szName );
                __leave;
            }

            //
            // Fermer le handle d'accés au registre fourni
            // à l'init du host.
            //
            if ( hKeyConfig != NULL )
            {   
                REG_Fermer( hKeyConfig );
                hKeyConfig = NULL;
            }

            //
            // Un élement de plus !
            //
            gsSvcWork.dwHosts = dwIndex + 1;

            //
            // S'il s'agit du dernier host (le host local)
            //
            if ( bStop )
            {
                //
                // Mémoriser le pointeur du host local (utilisé plus loin)
                //
                gsSvcWork.psLocalHost = psHost;
                break;
            }
        }

        //
        // Aucun host local n'a pu être défini, il y a donc
        // trop de hosts distants.
        //
        if ( gsSvcWork.psLocalHost == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            SVC_ERR_S( dwErr, "ERR_CONFIG_PARAM", "+++" );
            __leave;
        }

        //
        // -------------------------------------------------------
        // CHARGEMENT DES LINK ET INIT DES REPERTOIRES ASSOCIES
        // -------------------------------------------------------
        // 

        NTSVCInfo( "MainLoadConfig(), Chargement des links" );

        for ( dwIndex = 0 ; dwIndex < CM_MAX_LINKS ; dwIndex ++ )
        {
            //
            // Pointer sur le prochain link
            //
            psLink = &gsSvcWork.tsLinks[dwIndex];

            //
            // Récupérer le nom et construire le chemin complet
            // de la clé
            //
            dwSize = sizeof(szName);
            dwErr = REG_Enum_Cles( CM_REG_ROOT, CM_REG_KEY_LINKS, dwIndex, szName, &dwSize );
            if ( dwErr != NO_ERROR )
                break;

            CM_STRNCPY( szKey, CM_REG_KEY_LINKS, sizeof(szKey) );
            CM_STRNCAT( szKey, "\\"            , sizeof(szKey) );
            CM_STRNCAT( szKey, szName          , sizeof(szKey) );

            //
            // Recherche le host dont dépend le link
            //
            LIRE_CHAINE( szHost , szKey, "Host" );
            CMTrim( szHost );
            if ( szHost[0] == 0 )
				strcpy_s(szHost, sizeof(szHost), "LocalHost");

            for ( psHost = NULL, dwScan = 0 ; ( psHost == NULL ) && ( dwScan < gsSvcWork.dwHosts ) ; dwScan ++ )
            {
                if ( _stricmp( szHost, gsSvcWork.tsHosts[dwScan].szName ) == 0 )
                    psHost = &gsSvcWork.tsHosts[dwScan];
            }
            if ( psHost == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", szKey, "Host" );
                __leave;
            }

            //
            // Un item supplementaire peut être créé dans la liste
            // des links.
            //
            gsSvcWork.dwLinks = dwIndex + 1;

            //
            // Récupérer les valeurs des paramètres du link
            //
            CM_STRNCPY( psLink->szName, szName, sizeof(psLink->szName) );
            psLink->psHost = psHost;
            LIRE_ENTIER( psLink->dwPollingPeriod, szKey, "PollingPeriod" );
            LIRE_ENTIER( psLink->bImport, szKey, "Import" );
            LIRE_ENTIER( psLink->bImportAck, szKey, "ImportAck" );
            LIRE_ENTIER( psLink->bExport, szKey, "Export" );
            LIRE_ENTIER( psLink->bDispatch, szKey, "Dispatch" );
            if ( ( ( ! psLink->bImport ) && ( ! psLink->bExport    ) && ( ! psLink->bDispatch ) ) ||
                 ( (   psLink->bExport ) && (   psLink->bDispatch  ) ) ||
                 ( ( ! psLink->bImport ) && (   psLink->bImportAck ) )
               )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", szKey, "Import / Export / Dispatch" );
                __leave;
            }

            DEF_CHAINE( psLink->szNameMask   , szKey, "NameMask", "" );
            DEF_ENTIER( psLink->dwEnsureOrder, szKey, "EnsureOrder", 0 );

            DEF_CHAINE( szSort            , szKey, "SortBy"  , "INC:NAME" );
            CMTrim( szSort );
            if ( ! CMAnalyseSortParams( szSort, &sSortParams ) )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", szKey, "SortBy" );
                __leave;
            }

            //
            // Récupérer les infos spécifiques à chaque répertoires
            // d'export et les initialiser.
            //
            LIRE_CHAINE( szPath, szKey, "LocalMailOut" );
            CMDirInit( &psLink->sDirEAB, gsSvcWork.psLocalHost, szPath, psLink->szNameMask );
            psLink->sDirEAB.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "LocalMailOutAck" );
            CMDirInit( &psLink->sDirEABS, gsSvcWork.psLocalHost, szPath, psLink->szNameMask );
            psLink->sDirEABS.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "RemoteMailIn" );
            CMDirInit( &psLink->sDirRBA, psHost, szPath, psLink->szNameMask );
            psLink->sDirRBA.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "RemoteMailInAck" );
            CMDirInit( &psLink->sDirRBAS, psHost, szPath, psLink->szNameMask );
            psLink->sDirRBAS.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "RemoteMailInTemp" );
            CMDirInit( &psLink->sDirRBAT, psHost, szPath, psLink->szNameMask );
            psLink->sDirRBAT.sSort = sSortParams;
    
            //
            // Récupérer les infos spécifiques à chaque répertoires
            // d'import et les initialiser.
            //
            LIRE_CHAINE( szPath, szKey, "LocalMailIn" );
            CMDirInit( &psLink->sDirRAB, gsSvcWork.psLocalHost, szPath, psLink->szNameMask );
            psLink->sDirRAB.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "LocalMailInAck" );
            CMDirInit( &psLink->sDirRABS, gsSvcWork.psLocalHost, szPath, psLink->szNameMask );
            psLink->sDirRABS.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "LocalMailInTemp" );
            CMDirInit( &psLink->sDirRABT, gsSvcWork.psLocalHost, szPath, psLink->szNameMask );
            psLink->sDirRABT.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "RemoteMailOut" );
            CMDirInit( &psLink->sDirEBA, psHost, szPath, psLink->szNameMask );
            psLink->sDirEBA.sSort = sSortParams;

            LIRE_CHAINE( szPath, szKey, "RemoteMailOutAck" );
            CMDirInit( &psLink->sDirEBAS, psHost, szPath, psLink->szNameMask );
            psLink->sDirEBAS.sSort = sSortParams;
        }

        //
        // -------------------------------------------------------
        // INIT DU REPERTOIRE DE DISPATCHING
        // -------------------------------------------------------
        // 

        NTSVCInfo( "MainLoadConfig(), Init répertoire de dispatching" );

        //
        // Récupérer les infos spécifiques au répertoire de
        // dispatching et l'initialiser.
        //
        DEF_CHAINE( gsSvcWork.szNameMask, CM_REG_KEY_LINKS, "NameMask", "" );

        DEF_CHAINE( szSort              , CM_REG_KEY_LINKS, "SortBy"  , "INC:NAME" );
        CMTrim( szSort );
        if ( ! CMAnalyseSortParams( szSort, &sSortParams ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            SVC_ERR_SS( dwErr, "ERR_CONFIG_PARAM", CM_REG_KEY_LINKS, "SortBy" );
            __leave;
        }

        LIRE_CHAINE( szPath, CM_REG_KEY_LINKS, "LocalMailDispatcher" );
        CMDirInit( &gsSvcWork.sDirEA, gsSvcWork.psLocalHost, szPath, gsSvcWork.szNameMask );
        gsSvcWork.sDirEA.sSort = sSortParams;
        LIRE_ENTIER( gsSvcWork.dwDispatchPeriod, CM_REG_KEY_LINKS, "DispatchPeriod" );

        //
        // -------------------------------------------------------
        // CHARGEMENT DES WORKERS ET INIT DES REPERTOIRES ASSOCIES
        // -------------------------------------------------------
        // 

        NTSVCInfo( "MainLoadConfig(), Chargement des workers" );

        for ( dwIndex = 0 ; dwIndex < CM_MAX_WORKERS ; dwIndex ++ )
        {
            //
            // Pointer sur le prochain worker
            //
            psWorker = &gsSvcWork.tsWorkers[dwIndex];

            //
            // Récupérer le nom et construire le chemin complet
            // de la clé
            //
            dwSize = sizeof(szName);
            dwErr = REG_Enum_Cles( CM_REG_ROOT, CM_REG_KEY_WORKERS, dwIndex, szName, &dwSize );
            if ( dwErr != NO_ERROR )
                break;

            CM_STRNCPY( szKey, CM_REG_KEY_WORKERS, sizeof(szKey) );
            CM_STRNCAT( szKey, "\\"              , sizeof(szKey) );
            CM_STRNCAT( szKey, szName            , sizeof(szKey) );

            //
            // Ouvrir le handle d'accés au registre à fournir
            // à l'init du worker.
            //
            dwErr = REG_Ouvrir_Access( NULL, CM_REG_ROOT, szKey, &hKeyConfig, KEY_ALL_ACCESS );
            if ( dwErr != NO_ERROR )
            {
                SVC_ERR_S( dwErr, "ERR_CONFIG_PARAM", szKey );
                __leave;
            }

            if ( ! CMWorkerInit( psWorker, szName, hKeyConfig ) )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_CONFIG_WORKER", szName );
                __leave;
            }

            //
            // Un item supplementaire a été créé dans la liste
            // des workers.
            //
            gsSvcWork.dwWorkers = dwIndex + 1;

            //
            // Fermer le handle d'accés au registre fourni
            // à l'init du worker.
            //
            REG_Fermer( hKeyConfig );
            hKeyConfig = NULL;
        }

        NTSVCInfo( "MainLoadConfig(), Tout chargé sans erreur" );

        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( hKeyConfig != NULL ) REG_Fermer( hKeyConfig );

        if ( dwErr != NO_ERROR )
        {
            //
            // En cas d'erreur, on termine tout ce qui avait été commencé
            //

            //
            // Liste des workers
            //
            for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwWorkers ; dwIndex ++ )
                CMWorkerTerminate( &gsSvcWork.tsWorkers[dwIndex] );
            gsSvcWork.dwWorkers = 0;
            ZeroMemory( gsSvcWork.tsWorkers, sizeof(gsSvcWork.tsWorkers) );

            //
            // Répertoire de dispatching
            //
            if ( gsSvcWork.sDirEA.psHost != NULL ) CMDirTerminate( &gsSvcWork.sDirEA );
            ZeroMemory( &gsSvcWork.sDirEA, sizeof(gsSvcWork.sDirEA) );

            //
            // Liste des links
            //
            for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwLinks ; dwIndex ++ )
            {
                psLink = &gsSvcWork.tsLinks[dwIndex];

                if ( psLink->bExport || psLink->bDispatch )
                {
                    if ( psLink->sDirEAB.psHost  != NULL ) CMDirTerminate( &psLink->sDirEAB  );
                    if ( psLink->sDirEABS.psHost != NULL ) CMDirTerminate( &psLink->sDirEABS );
                }

                if ( psLink->bExport )
                {
                    if ( psLink->sDirRBA.psHost  != NULL ) CMDirTerminate( &psLink->sDirRBA  );
                    if ( psLink->sDirRBAS.psHost != NULL ) CMDirTerminate( &psLink->sDirRBAS );
                    if ( psLink->sDirRBAT.psHost != NULL ) CMDirTerminate( &psLink->sDirRBAT );
                }

                if ( psLink->bImport )
                {
                    if ( psLink->sDirRAB.psHost  != NULL ) CMDirTerminate( &psLink->sDirRAB  );
                    if ( psLink->sDirRABS.psHost != NULL ) CMDirTerminate( &psLink->sDirRABS );
                    if ( psLink->sDirRABT.psHost != NULL ) CMDirTerminate( &psLink->sDirRABT );
                    if ( psLink->sDirEBA.psHost  != NULL ) CMDirTerminate( &psLink->sDirEBA  );
                }
            }
            gsSvcWork.dwLinks = 0;
            ZeroMemory( gsSvcWork.tsLinks, sizeof(gsSvcWork.tsLinks) );

            //
            // Liste des hosts
            //
            for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwHosts ; dwIndex ++ )
                CMHostTerminate( &gsSvcWork.tsHosts[dwIndex] );
            gsSvcWork.dwHosts = 0;
            ZeroMemory( gsSvcWork.tsHosts, sizeof(gsSvcWork.tsHosts) );
        }
    }

#undef LIRE_ENTIER
#undef LIRE_CHAINE

	return dwErr;
}




PRIVATE void MainUnloadConfig()
{
    DWORD dwIndex;
    CM_LINK * psLink;

    NTSVCInfo( "MainUnloadConfig(), Début" );

    //
    // Liste des workers
    //
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwWorkers ; dwIndex ++ )
        CMWorkerTerminate( &gsSvcWork.tsWorkers[dwIndex] );
    gsSvcWork.dwWorkers = 0;
    ZeroMemory( gsSvcWork.tsWorkers, sizeof(gsSvcWork.tsWorkers) );

    //
    // Répertoire de dispatching
    //
    if ( gsSvcWork.sDirEA.psHost != NULL ) CMDirTerminate( &gsSvcWork.sDirEA );
    ZeroMemory( &gsSvcWork.sDirEA, sizeof(gsSvcWork.sDirEA) );

    //
    // Liste des links
    //
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwLinks ; dwIndex ++ )
    {
        psLink = &gsSvcWork.tsLinks[dwIndex];

        if ( psLink->bExport || psLink->bDispatch )
        {
            CMDirTerminate( &psLink->sDirEAB  );
            CMDirTerminate( &psLink->sDirEABS );
        }

        if ( psLink->bExport )
        {
            CMDirTerminate( &psLink->sDirRBA  );
            CMDirTerminate( &psLink->sDirRBAS );
            CMDirTerminate( &psLink->sDirRBAT );
        }

        if ( psLink->bImport )
        {
            CMDirTerminate( &psLink->sDirRAB  );
            CMDirTerminate( &psLink->sDirRABS );
            CMDirTerminate( &psLink->sDirRABT );
            CMDirTerminate( &psLink->sDirEBA  );
        }
    }
    gsSvcWork.dwLinks = 0;
    ZeroMemory( gsSvcWork.tsLinks, sizeof(gsSvcWork.tsLinks) );

    //
    // Liste des hosts
    //
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwHosts ; dwIndex ++ )
        CMHostTerminate( &gsSvcWork.tsHosts[dwIndex] );
    gsSvcWork.dwHosts = 0;
    ZeroMemory( gsSvcWork.tsHosts, sizeof(gsSvcWork.tsHosts) );

}




PRIVATE DWORD MainStartThreads()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwId;

    __try
    {
        NTSVCInfo( "MainStartThreads(), Début" );

        gsSvcWork.dwThreads = 0;

        NTSVCInfo( "MainStartThreads(), Lancement des threads de gestion des links" );

        // Lancer les threads de travail sur les links
        for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwLinks ; dwIndex ++ )
        {
            gsSvcWork.thThreads[gsSvcWork.dwThreads] = ExcptCreateThread(
                NULL,
                0, 
                (LPTHREAD_START_ROUTINE)&CMThreadLink, 
                (LPVOID)dwIndex, 
                0, 
                &dwId,
                "LinkWorker" );
            if ( gsSvcWork.thThreads[gsSvcWork.dwThreads] == NULL )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_THREAD_INIT", "LINKS" );
                __leave;
            }
            gsSvcWork.dwThreads++;
        }

        if ( gsSvcWork.dwLinks > 0 )
        {
            NTSVCInfo( "MainStartThreads(), Lancement du thread de gestion du dispatching" );

            // Lancer le thread de travail sur le dispatching
            gsSvcWork.thThreads[gsSvcWork.dwThreads] = ExcptCreateThread( 
                NULL, 
                0, 
                (LPTHREAD_START_ROUTINE)&CMThreadDispatch, 
                (LPVOID)0, 
                0, 
                &dwId,
                "ExcptDispatcher" );
            if ( gsSvcWork.thThreads[gsSvcWork.dwThreads] == NULL )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_THREAD_INIT", "DISPATCHER" );
                __leave;
            }
            gsSvcWork.dwThreads ++;
        }

        NTSVCInfo( "MainStartThreads(), Lancement des threads de gestion des workers" );

        // Lancer les threads de travail sur les workers
        for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwWorkers ; dwIndex ++ )
        {
            gsSvcWork.thThreads[gsSvcWork.dwThreads] = ExcptCreateThread(
                NULL,
                0, 
                (LPTHREAD_START_ROUTINE)&CMThreadWorker, 
                (LPVOID)dwIndex, 
                0, 
                &dwId,
                "WorkerWorker" );
            if ( gsSvcWork.thThreads[gsSvcWork.dwThreads] == NULL )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_THREAD_INIT", "WORKER" );
                __leave;
            }
            gsSvcWork.dwThreads++;
        }

        NTSVCInfo( "MainStartThreads(), Tous les threads ont été lancés" );
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            NTSVCSignalEnd();
            WaitForMultipleObjects(
                gsSvcWork.dwThreads,
                gsSvcWork.thThreads,
                TRUE,
                10000 );
            for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwThreads ; dwIndex ++ )
            {
                // Forcer la fin des thread (s'ils ne sont pas terminés)
                TerminateThread( gsSvcWork.thThreads[dwIndex], 0xDEADBEEF );
                CloseHandle( gsSvcWork.thThreads[dwIndex] );
            }
        }
    }

	return dwErr;
}



PRIVATE void MainStopThreads()
{
    DWORD dwIndex;

    NTSVCSignalEnd();
    WaitForMultipleObjects(
        gsSvcWork.dwThreads,
        gsSvcWork.thThreads,
        TRUE,
        10000 );
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwThreads ; dwIndex ++ )
    {
        // Forcer la fin des thread (s'ils ne sont pas terminés
        TerminateThread( gsSvcWork.thThreads[dwIndex], 0xDEADBEEF );
        CloseHandle( gsSvcWork.thThreads[dwIndex] );
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
        sprintf( szKey, NTSVC_REG_KEY_SERVICE "\\" 
                        CM_SERVICE_NAME "\\" 
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






