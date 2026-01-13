/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_main.c
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
#include <csr_list.h>
#include <csr_msg.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <msg_sv_header.h>
#include <msg_lc_header.h>
#include <reg.h>

#include <resource.h>

#define LOC_DEF
#include <routesvc_glob.h>
#undef LOC_DEF

#include <routesvc_text.h>
#include <routesvc_callback.h>
#include <routesvc_struct.h>

#define LOC_DEF
#include <routesvc_main.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

// --------------- FONCTION PRIVEES ----------------------

PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE DWORD MainInitPipes();
PRIVATE DWORD MainTerminatePipes();
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

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

    TextLoadDefinitions( NTSVC_REG_KEY_SERVICE "\\" ROUTESVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\Text", &gsSvcWork.hText );

    // Ouverture des paramètres
    gsSvcWork.psParams = NTSVCOpenParameters( 
            ROUTESVC_REG_VAL_CMDSVC        , REG_DWORD,        4,             1, &gsSvcWork.sParmCopy.dwCmdSvc,
            ROUTESVC_REG_VAL_MAXCMDCNX     , REG_DWORD,        4,             5, &gsSvcWork.sParmCopy.dwMaxCmdCnx,
            ROUTESVC_REG_VAL_MAXLSCNX      , REG_DWORD,        4,             4, &gsSvcWork.sParmCopy.dwMaxLSCnx,
            ROUTESVC_REG_VAL_MAXLCCNX      , REG_DWORD,        4,            64, &gsSvcWork.sParmCopy.dwMaxLCCnx,
            ROUTESVC_REG_VAL_TCPLS         , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTCPLS,
            ROUTESVC_REG_VAL_TCPLC         , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTCPLC,
            ROUTESVC_REG_VAL_WORKERS       , REG_DWORD,        4,             4, &gsSvcWork.sParmCopy.dwWorkers,
            ROUTESVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4,          1000, &gsSvcWork.sParmCopy.dwTimeToReconnect,
            ROUTESVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4,           256, &gsSvcWork.sParmCopy.dwMaxQueuedMessages,
            ROUTESVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwMaxMsgSize,
            ROUTESVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwPipeBufferSize,
            ROUTESVC_REG_VAL_MAXLIFE       , REG_DWORD,        4,             3, &gsSvcWork.sParmCopy.dwMaxLife,
            ROUTESVC_REG_VAL_LIFETIME      , REG_DWORD,        4,          5000, &gsSvcWork.sParmCopy.dwLifeTime,
            ROUTESVC_REG_VAL_APP_MSG_ID    , REG_DWORD,        4,           100, &gsSvcWork.sParmCopy.dwAppMsgId,
            ROUTESVC_REG_VAL_ACOM_TRC_ON   , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwAComTraceOn,
            ROUTESVC_REG_VAL_ACOM_ERR_ON   , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwAComErrorOn,
            ROUTESVC_REG_VAL_TRACE_LC_MSG  , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTraceLCMsg,
            ROUTESVC_REG_VAL_TRACE_LS_MSG  , REG_DWORD,        4,             0, &gsSvcWork.sParmCopy.dwTraceLSMsg,
            ROUTESVC_REG_VAL_ACOM_FILE     , REG_SZ   , MAX_PATH,"C:\\ACOM.LOG", gsSvcWork.sParmCopy.szAComTraceFile,
            NULL );

    if ( gsSvcWork.psParams == NULL )
        return;

    gsSvcWork.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ROUTESVC_OK ) );
    gsSvcWork.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ROUTESVC_KO ) );
    gsSvcWork.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ROUTESVC_OK_RUN ) );
    gsSvcWork.hIconKoRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ROUTESVC_KO_RUN ) );

    do {

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

        gsSvcWork.fLicenceIsValid = TRUE;

        AComOpenTrace( gsSvcWork.sParmWork.dwAComTraceOn, 
                       gsSvcWork.sParmWork.dwAComErrorOn, 
                       gsSvcWork.sParmWork.szAComTraceFile );

        // Initialiser la messagerie
        dwErr = MainInitMessages();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_MESSAGES" );
            StructCloseList();
            AComCloseTrace();
            MainTerminateService();
            break;
        }


        // Initialiser les pipes
        dwErr = MainInitPipes();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_PIPES" );
            MainTerminateMessages();
            StructCloseList();
            AComCloseTrace();
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
        while ( TRUE )
        {
            if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT ) break;

            if ( gsSvcWork.fLicenceIsValid )
                NTSVCRefreshTrayIcon( gsSvcWork.hIconOk, "SVC_RUNNING" );
            else
                NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_INVALID_LICENCE" );

            if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT ) break;

            if ( gsSvcWork.fLicenceIsValid )
                NTSVCRefreshTrayIcon( gsSvcWork.hIconOkRun, "SVC_RUNNING" );
            else
                NTSVCRefreshTrayIcon( gsSvcWork.hIconKoRun, "SVC_INVALID_LICENCE" );
        }

        MainUnloadMenu();

        NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "SVC_STOPPING" );

        if ( ! gsSvcWork.bReload )
        {
            // Passer dans l'état "en cours d'arrêt"
            NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );
        }
        else
            NTSVCResetEnd();

        NTSVCInfo( "NTSVCExternalMain(), destruction des objets" );
        MainTerminatePipes();
        StructCloseList();
        MainTerminateMessages();

        AComCloseTrace();
        NTSVCError( TextFind( gsSvcWork.hText, "SVC_STOPPED", "SVC_STOPPED" ) );
        MainTerminateService();

        NTSVCRefreshTrayIcon( NULL, NULL );

        dwLoadCount ++;
    }
    while ( gsSvcWork.bReload );

    DestroyIcon( gsSvcWork.hIconOk );
    DestroyIcon( gsSvcWork.hIconKo );
    DestroyIcon( gsSvcWork.hIconOkRun );
    DestroyIcon( gsSvcWork.hIconKoRun );

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
        return dwErr;

    gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.dwMaxTotalCnx = gsSvcWork.sParmWork.dwMaxCmdCnx + gsSvcWork.sParmWork.dwMaxLSCnx + gsSvcWork.sParmWork.dwMaxLCCnx;
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        
    gsSvcWork.bReload = FALSE;

    InitializeCriticalSection( &gsSvcWork.sCritical );

    // Préparer la liste  des connexions
    dwErr = StructOpenList(
        gsSvcWork.sParmWork.dwMaxLCCnx,
        gsSvcWork.sParmWork.dwMaxLSCnx,
        gsSvcWork.sParmWork.dwMaxCmdCnx );
    if ( dwErr != NO_ERROR )
    {
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
    // Ajout des pipes dans la liste null session
    AComSetNullSessionPipe( ROUTESVC_PIPE_CMD, TRUE );
    AComSetNullSessionPipe( ROUTESVC_PIPE_LS, TRUE );
    AComSetNullSessionPipe( ROUTESVC_PIPE_LC, TRUE );

    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "MainInitPipes(), creation du working set" );
    gsSvcWork.hWks = AComOpenWorkingSet(
            0,
            gsSvcWork.dwMaxTotalCnx,
            3,
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
                    ROUTESVC_PIPE_CMD,
                    ROUTESVC_PIPE_CMD_TYPE,
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
                    ROUTESVC_PIPE_CMD_TYPE,                     // dwInstUsrKey
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
            AComCloseWorkingSet( gsSvcWork.hWks, ROUTESVC_WORKINGSET_TIMEMOUT );
            return ERROR_INVALID_DATA;
        }
    }

    // MISE EN PLACE DE L'INSTANCE DE LS

    NTSVCInfo( "MainInitPipes(), création du serveur LS" );
    if ( gsSvcWork.sParmWork.dwTCPLS == 0 )
        gsSvcWork.hLSInst = AComOpenPipeServerInstance(
                gsSvcWork.hWks,
                ROUTESVC_PIPE_LS,
                ROUTESVC_PIPE_LS_TYPE,
                gsSvcWork.sParmWork.dwTimeToReconnect,
                gsSvcWork.sParmWork.dwMaxLSCnx,
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
        gsSvcWork.hLSInst = AComOpenTcpServerInstance(
                gsSvcWork.hWks,                             // hWks
                "",                                         // pcAddress
                gsSvcWork.sParmWork.dwTCPLS,                // dwPort
                ROUTESVC_PIPE_LS_TYPE,                      // dwInstUsrKey
                gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
                gsSvcWork.sParmWork.dwMaxLSCnx,             // dwMaxConnections
                gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
                gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
                gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
                gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
    if ( gsSvcWork.hLSInst == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_LS_SRV" );
        AComCloseWorkingSet( gsSvcWork.hWks, ROUTESVC_WORKINGSET_TIMEMOUT );
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE L'INSTANCE DE LC

    NTSVCInfo( "MainInitPipes(), création du serveur voie" );
    if ( gsSvcWork.sParmWork.dwTCPLC == 0 )
        gsSvcWork.hLCInst = AComOpenPipeServerInstance(
                gsSvcWork.hWks,
                ROUTESVC_PIPE_LC,
                ROUTESVC_PIPE_LC_TYPE,
                gsSvcWork.sParmWork.dwTimeToReconnect,
                gsSvcWork.sParmWork.dwMaxLCCnx,
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
        gsSvcWork.hLCInst = AComOpenTcpServerInstance(
                gsSvcWork.hWks,                             // hWks
                "",                                         // pcAddress
                gsSvcWork.sParmWork.dwTCPLC,                // dwPort
                ROUTESVC_PIPE_LC_TYPE,                      // dwInstUsrKey
                gsSvcWork.sParmWork.dwTimeToReconnect,      // dwTimeToReconnect
                gsSvcWork.sParmWork.dwMaxLCCnx,             // dwMaxConnections
                gsSvcWork.sParmWork.dwPipeBufferSize,       // dwOutBufferSize
                gsSvcWork.sParmWork.dwPipeBufferSize,       // dwInBufferSize
                gsSvcWork.sParmWork.dwMaxQueuedMessages,    // dwQueueSize
                gsSvcWork.sParmWork.dwMaxMsgSize,           // dwMaxMessageSize
                CallbackConnection,
                CallbackDisconnection,
                CallbackReceived,
                CallbackSent,
                NULL );
    if ( gsSvcWork.hLCInst == NULL )
    {
        SVC_ERR( ERROR_PIPE_NOT_CONNECTED, "ERR_INIT_LC_SRV" );
        AComCloseWorkingSet( gsSvcWork.hWks, ROUTESVC_WORKINGSET_TIMEMOUT );
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

    AComCloseWorkingSet( gsSvcWork.hWks, ROUTESVC_WORKINGSET_TIMEMOUT );

    return NO_ERROR;
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
    if ( ! MSG_SV_FILT_DEC_New_Record( &gsSvcWork.hInMsgList ) )
        return ERROR_NOT_ENOUGH_MEMORY;
    if ( ! MSG_SV_CON_REQ_New_Record( &gsSvcWork.hInMsgList ) )
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
        printf( "RouteSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( ROUTESVC_PIPE_CMD, TRUE );
        if ( dwErr == NO_ERROR ) dwErr = AComSetNullSessionPipe( ROUTESVC_PIPE_LS, TRUE );
        if ( dwErr == NO_ERROR ) dwErr = AComSetNullSessionPipe( ROUTESVC_PIPE_LC, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "RouteSvc - Error %u\n", dwErr );
        else
            printf( "RouteSvc - Null session pipe setup done\n" );
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
                        ROUTESVC_SERVICE_NAME "\\" 
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




