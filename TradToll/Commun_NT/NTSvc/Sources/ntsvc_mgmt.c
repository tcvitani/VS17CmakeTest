/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV.C                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>

#include <cmdl.h>
#include <ntsvc.h>
#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>

#define LOC_DEF
	#include <ntsvc_mgmt.h>
#undef LOC_DEF

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE void WINAPI NTSVCMain( 
        IN      DWORD    dwArgc,
        IN      char  ** ppcArgv );


PRIVATE BOOL NTSCVExcptCallMain( 
        IN      DWORD    dwArgc, 
        IN      char * * ppcArgv );

PRIVATE void WINAPI NTSVCHandler(
        IN      DWORD dwCtrlCode );

PRIVATE BOOL WINAPI NTSVCConsoleHandler( 
        IN      DWORD dwCtrlType );

PRIVATE void NTSVCRestart();

PRIVATE WINAPI NTSVCWindowManagementThread( 
        IN      void * pvFoo );

PRIVATE LRESULT CALLBACK NTSVCWindowManagementCallback( 
        IN      HWND hWnd,
        IN      UINT uMsg,
        IN      WPARAM wParam,
        IN      LPARAM lParam );

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI NTSVCStart( 
 *                         IN      char * pcServiceName, 
 *                         IN      LPSERVICE_MAIN_FUNCTION pfServiceProc, 
 *                         IN      DWORD dwStartTimeout,
 *                         IN      DWORD dwStopTimeout,
 *                         IN      char * pcSimulateParams )
 * PARAMETERS: IN      char * pcServiceName                  : Nom du service
 *             IN      LPSERVICE_MAIN_FUNCTION pfServiceProc : Fonction main du service (fournie au link avec
 *                                                             la librairie.
 *             IN      DWORD dwStartTimeout                  : Temps imparti pour démarrage
 *             IN      DWORD dwStopTimeout                   : Temps imparti pour arrêt
 *             IN      char * pcSimulateParams               : Si NULL, il s'agit d'un démarrage de service
 *                                                             normal. Si non NULL, il s'agit d'un démarrage en
 *                                                             ligne de commande, et ce paramètre contient
 *                                                             les paramètre de démarrage du service.
 * RETURN    : Code d'erreur WIN32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lance de service
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI NTSVCStart( 
        IN      char * pcServiceName, 
        IN      LPSERVICE_MAIN_FUNCTION pfServiceProc, 
        IN      DWORD dwStartTimeout,
        IN      DWORD dwStopTimeout,
        IN      char * pcSimulateParams )
{
    int iArgc;
    char ** ppcArgv;
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    DWORD dwSize;
    HANDLE hMutex;
    char szMutex[MAX_PATH];
    char szKey[MAX_PATH];
    char szFile[MAX_PATH];
    SERVICE_TABLE_ENTRY tsDispatchTable[2];

    if ( gpsMain->sWork.bInitialized )
	    return ERROR_ALREADY_EXISTS;

    _snprintf_s( szMutex, MAX_PATH, sizeof( szMutex )-1, "MTX_%s", pcServiceName );

    szMutex[sizeof(szMutex)-1] = '\0';

    // Mutex d'exclusion mutuelle évitant au service de 
    // se retrouver lancé plusieurs fois (en mode debug par exemple)
    hMutex = CreateMutex( NULL, FALSE, szMutex );
    if ( hMutex == NULL )
    {
        dwErr = GetLastError();
        return dwErr;
    }

    if ( WaitForSingleObject( hMutex, 0 ) != WAIT_OBJECT_0 )
    {
        dwErr = GetLastError();
        CloseHandle( hMutex );
        return dwErr;
    }

    ZeroMemory( &gpsMain->sWork, sizeof(gpsMain->sWork) );
    ZeroMemory( tsDispatchTable, sizeof( tsDispatchTable ) );

    if ( strlen( pcServiceName ) >= sizeof( gpsMain->sWork.szServiceName ) )
    {
        CloseHandle( hMutex );
        return ERROR_INVALID_PARAMETER;
    }
    strcpy_s( gpsMain->sWork.szServiceName, 256, pcServiceName );

    InitializeCriticalSection( &gpsMain->sWork.sIconSerializer );
    gpsMain->sWork.dwStartTimeout = dwStartTimeout;
    gpsMain->sWork.dwStopTimeout = dwStopTimeout;
    gpsMain->sWork.pfServiceProc = pfServiceProc;
    gpsMain->sWork.bInitialized = TRUE;
    gpsMain->sFlags.bSimulated = ( pcSimulateParams != NULL );
    /*
    if ( ( ! gpsMain->sFlags.bSimulated ) && ( gpsMain->sFlags.bConsole ) )
    {
        gpsMain->sFlags.bConsole = FALSE;
        FreeConsole();
    }*/

    sprintf_s( szKey, MAX_PATH, "%s\\%s\\%s", NTSVC_REG_KEY_SERVICE, pcServiceName, NTSVC_REG_KEY_PARAM );

    dwSize = sizeof(gpsMain->sWork.szInfoFile);

    sprintf_s( szFile, MAX_PATH, "\\%s.TRC", pcServiceName );

    dwRes = REG_Defaut_Chaine( HKEY_LOCAL_MACHINE, szKey, NTSVC_REG_VAL_INFOFILE, gpsMain->sWork.szInfoFile, &dwSize, szFile );
    if ( dwRes != NO_ERROR ) strcpy_s( gpsMain->sWork.szInfoFile, MAX_PATH, szFile );

    dwSize = sizeof(gpsMain->sWork.szErrorFile);

    sprintf_s( szFile, MAX_PATH, "\\%s.ERR", pcServiceName );

    dwRes = REG_Defaut_Chaine( HKEY_LOCAL_MACHINE, szKey, NTSVC_REG_VAL_ERRORFILE, gpsMain->sWork.szErrorFile, &dwSize, szFile );
    if ( dwRes != NO_ERROR ) strcpy_s( gpsMain->sWork.szErrorFile, MAX_PATH, szFile );


	dwRes = REG_Defaut_Entier(HKEY_LOCAL_MACHINE, szKey, NTSVC_REG_VAL_TRACEFILE_MAX_SIZE, &gpsMain->sWork.dwTraceMaxSizeMb, 2);
	if (dwRes != NO_ERROR) gpsMain->sWork.dwTraceMaxSizeMb = 2;
	

    dwRes = REG_Defaut_Entier( HKEY_LOCAL_MACHINE, szKey, NTSVC_REG_VAL_EXCPTRESTART, &gpsMain->sWork.dwExcptRestart, 1 );
    if ( dwRes != NO_ERROR ) gpsMain->sWork.dwExcptRestart = 1;
    


    dwRes = TRC_Initialise_Trace( 
            pcServiceName, 
            gpsMain->sWork.szInfoFile,
            TRC_OPT_FICHIER | ( gpsMain->sFlags.bSimulated ? TRC_OPT_CONSOLE : 0 ),
            &gpsMain->sWork.hInfo );
    if ( dwRes != NO_ERROR ) 
		gpsMain->sWork.hInfo = NULL;
	else
		TRC_Taille_Max_Fichier(gpsMain->sWork.hInfo, gpsMain->sWork.dwTraceMaxSizeMb * 1000000);


    dwRes = TRC_Initialise_Trace( 
            pcServiceName, 
            gpsMain->sWork.szErrorFile,
            TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | ( gpsMain->sFlags.bSimulated ? TRC_OPT_CONSOLE : 0 ) | TRC_OPT_NUMEROTATION,
            &gpsMain->sWork.hError );
    if ( dwRes != NO_ERROR ) 
		gpsMain->sWork.hError = NULL;
	else
		TRC_Taille_Max_Fichier(gpsMain->sWork.hError, gpsMain->sWork.dwTraceMaxSizeMb * 1000000);

    gpsMain->sWork.hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( ! gpsMain->sFlags.bSimulated )
    {
	    tsDispatchTable[0].lpServiceName = gpsMain->sWork.szServiceName;
	    tsDispatchTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)NTSVCMain;
	    if ( ! StartServiceCtrlDispatcher( tsDispatchTable ) )
	    {
		    dwErr = GetLastError();
        }
    }
    else
    {
        if ( gpsMain->sFlags.bConsole )
            SetConsoleCtrlHandler( NTSVCConsoleHandler, TRUE );

        iArgc = 0;
        ppcArgv = CMDLOpenArgv( pcSimulateParams, &(DWORD)iArgc );
        NTSCVExcptCallMain( iArgc, ppcArgv );
        CMDLCloseArgv( ppcArgv );
    }

    CloseHandle( gpsMain->sWork.hEndEvent );
    TRC_Vider_Traces( INFINITE );
    TRC_Termine_Tout();
    CloseHandle( hMutex );

    if ( gpsMain->sWork.bRestart && ( gpsMain->sWork.dwExcptRestart != 0 ) )
        NTSVCRestart();

    gpsMain->sWork.bInitialized = FALSE;
    DeleteCriticalSection( &gpsMain->sWork.sIconSerializer );

	return dwErr;
}


PRIVATE void WINAPI NTSVCMain( 
        IN      DWORD    dwArgc,
        IN      char  ** ppcArgv )
{
	DWORD dwErr;

    gpsMain->sWork.hStatus = RegisterServiceCtrlHandler( gpsMain->sWork.szServiceName, (LPHANDLER_FUNCTION)NTSVCHandler );
    if ( gpsMain->sWork.hStatus == 0 )
		return;

	SetLastError(NO_ERROR);
	if ( ( dwErr = NTSVCSetCurrentState( SERVICE_START_PENDING, gpsMain->sWork.dwStartTimeout, NO_ERROR ) ) != NO_ERROR )
	{
		NTSVCSetCurrentState( SERVICE_STOPPED, 0, dwErr );
        gpsMain->sWork.hStatus = 0;
		return;
	}

    gpsMain->sWork.bIsRunning = TRUE;

    NTSCVExcptCallMain( dwArgc, ppcArgv );

    gpsMain->sWork.bIsRunning = FALSE;

	NTSVCSetCurrentState( SERVICE_STOPPED, 0, NO_ERROR );
    gpsMain->sWork.hStatus = 0;
}



PRIVATE BOOL NTSCVExcptCallMain( 
        IN      DWORD    dwArgc, 
        IN      char * * ppcArgv )
{
    DWORD dwCount;
    BOOL bHasException = FALSE;

    gpsMain->sWork.bRestart = FALSE;
    gpsMain->sWork.uiWndNotify = RegisterWindowMessage( "NTSVCNotify" );
    gpsMain->sWork.dwWndItems = 0;
    gpsMain->sWork.pfWndHandler = NULL;
    gpsMain->sWork.fWndClose = FALSE;

    gpsMain->sWork.hWndThread = NTSVCCreateThread( NULL,
                                                   0,
                                                   (LPTHREAD_START_ROUTINE)&NTSVCWindowManagementThread,
                                                   NULL,
                                                   0,
                                                   &gpsMain->sWork.dwWndhread,
                                                   "NTSVCWindowManagementThread" );

    ExcptSetDefaultThreadMode( EXPT_MODE_EXECUTE( NTSVCThreadExceptionHandler ) );

    __try
    {
        gpsMain->sWork.pfServiceProc( dwArgc, ppcArgv );
    }
    __except( ( ExcptLog( 
                      (EXCEPTION_POINTERS*)_exception_info(), 
                      "SERVICEMAIN" ) 
              ),
              ( EXCEPTION_EXECUTE_HANDLER )
            )
    {
        bHasException = TRUE;
        dwCount = 0;

        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "*******************************" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "******      EXCEPTION    ******" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "****** CF LOG EXCEPTION  ******" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "*******************************" );
    }

    NTSVCRefreshTrayIcon( NULL, NULL );

    if ( gpsMain->sWork.hWndThread != NULL )
    {
        gpsMain->sWork.fWndClose = TRUE;
        PostThreadMessage( gpsMain->sWork.dwWndhread, WM_QUIT, 0, 0 );

        if ( bHasException )
            WaitForSingleObject( gpsMain->sWork.hWndThread, 10000 );
        else
            WaitForSingleObject( gpsMain->sWork.hWndThread, INFINITE );

        TerminateThread( gpsMain->sWork.hWndThread, 0 );
        CloseHandle( gpsMain->sWork.hWndThread );
    }

    gpsMain->sWork.bRestart = bHasException;

    return bHasException;
}




PRIVATE void WINAPI NTSVCHandler(
        IN      DWORD dwCtrlCode )
{
	switch( dwCtrlCode )
    {
        case SERVICE_CONTROL_STOP :
		case SERVICE_CONTROL_SHUTDOWN :
			SetEvent( gpsMain->sWork.hEndEvent );
            NTSVCSetCurrentState( SERVICE_STOP_PENDING, gpsMain->sWork.dwStopTimeout, NO_ERROR );
            break;

        case SERVICE_CONTROL_PAUSE :
            break;

        case SERVICE_CONTROL_CONTINUE :
            break;

        case SERVICE_CONTROL_INTERROGATE :
            break;

        default:
            break;

    }
	
	SetServiceStatus( gpsMain->sWork.hStatus, &gpsMain->sWork.sStatus );
}



PRIVATE BOOL WINAPI NTSVCConsoleHandler( 
        IN      DWORD dwCtrlType )
{
    SetEvent( gpsMain->sWork.hEndEvent );
    return TRUE;
}



PRIVATE void NTSVCRestart()
{
    STARTUPINFO sStart;
    PROCESS_INFORMATION sProcess;
    char szFullPath[MAX_PATH+64];

    if ( ! gpsMain->sFlags.bSimulated )
    {
        if ( GetModuleFileName( NULL, szFullPath, MAX_PATH ) == 0 )
            return;

        strcat_s( szFullPath, MAX_PATH+64, " /EXCPTRESTART" );

        ZeroMemory( &sStart, sizeof(sStart) );
        CreateProcess( NULL, szFullPath, NULL, NULL, FALSE, 0, NULL, NULL, &sStart, &sProcess );
    }
}




PROTECTED void WINAPI NTSVCThreadExceptionHandler( DWORD dwExcpt, char * szThreadName )
{
    __try
    {
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "*******************************" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "******      EXCEPTION    ******" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "****** CF LOG EXCEPTION  ******" );
        TRC_Direct_Trace_Texte( gpsMain->sWork.hError, TRC_OPT_MASK, "*******************************" );
        NTSVCSetCurrentState( SERVICE_STOP_PENDING, 1000, NO_ERROR );
        NTSVCSetCurrentState( SERVICE_STOPPED, 0, NO_ERROR );
        if ( gpsMain->sWork.dwExcptRestart != 0 )
            NTSVCRestart();
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
    }
    ExitProcess( dwExcpt );
}





PRIVATE WINAPI NTSVCWindowManagementThread( 
        IN      void * pvFoo )
{
    HDESK           hSavedDesktop = NULL;
    HWINSTA         hSavedStation = NULL;
    HDESK           hNewDesktop;
    HWINSTA         hNewStation;
    BOOL            fSuccess;
    WNDCLASSEX      sClass;
    ATOM            aClass;
    MSG             sMsg;
    HWND            hWndSave;

    //
    // Récupérer les informations sur la window station et
    // le bureau courant. On va les modifier, il faut donc
    // pouvoir les restaurer.
    //
    hSavedStation = GetProcessWindowStation();
    hSavedDesktop = GetThreadDesktop( GetCurrentThreadId() );

    //
    // On entre dans la boucle infinie de reconstruction de
    // la fenètre. Cette fenètre servira essentiellement à la
    // récupération des notifications provenant des icônes de
    // la barre de tache.
    //
    while ( ! gpsMain->sWork.fWndClose )
    {
        //
        // Pour éviter une boucle un peu trop rapide qui monopoliserait
        // trop de CPU, on met une attente en début de boucle.
        //
        if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT )
        {
            if ( ! gpsMain->sWork.fWndClose )
            {
                Sleep( 100 );
                continue;
            }
            else
                break;
        }

        //
        // Remise à zéro des handles manipulés dans la boucle.
        // En fin de boucle, c'est plus facile pour repérer ceux
        // qui doivent être désalloués/détruits/fermés.
        //
        gpsMain->sWork.hWnd = NULL;
        aClass = 0;
        hNewDesktop = NULL;
        hNewStation = NULL;

        //
        // Liaison avec window station et desktop par défaut.
        // Suivi de la création de la classe associée à la fenètre,
        // et enfin création et ouverture de la fenètre.
        // Par défaut, ces actions échouent. Elles ne réussissent
        // que si on arrive à la fin de la séquence sans erreur.
        //
        fSuccess = FALSE;
        __try
        {
            //
            // Ouverture de la window station primaire. Sur un terminal server, il s'agit de
            // la station locale.
            //
            if ( ( hNewStation = OpenWindowStation( "WinSta0", FALSE, MAXIMUM_ALLOWED ) ) == NULL )
                __leave;

            //
            // On affecte le processus à cette station.
            //
            if ( ! SetProcessWindowStation( hNewStation ) )
                __leave;
    
            //
            // La stations pouvant gérer plusieurs bureaux, on ouvre le bureau par défaut.
            //
            if ( ( hNewDesktop = OpenDesktop( "Default", 0, FALSE, MAXIMUM_ALLOWED ) ) == NULL )
                __leave;
        
            //
            // Et enfin, on associe le thread courant au bureau par défaut. C'est donc ce
            // thread qui contient la message loop de la fenètre.
            //
            if ( ! SetThreadDesktop( hNewDesktop ) )
                __leave;
            
            //
            // Définition et enregistrement de la classe de la nouvelle fenètre.
            //
            sClass.cbSize = sizeof(sClass);
            sClass.style = 0; // CS_OWNDC | CS_NOCLOSE;
            sClass.lpfnWndProc = &NTSVCWindowManagementCallback;
            sClass.cbClsExtra = 0;
            sClass.cbWndExtra = 0;
            sClass.hInstance = GetModuleHandle( NULL );
            sClass.hIcon = LoadIcon( NULL, IDI_WINLOGO );;
            sClass.hCursor = LoadCursor( NULL, IDC_ARROW );
            sClass.hbrBackground = GetSysColorBrush( COLOR_WINDOW );
            sClass.lpszMenuName = NULL;
            sClass.lpszClassName = "NTSvcWindowClass";
            sClass.hIconSm = NULL;
            if ( ( aClass = RegisterClassEx( &sClass ) ) == 0 )
                __leave;

            //
            // Création d'une instance de la classe qu'on vient de définir
            //
            gpsMain->sWork.hWnd = CreateWindowEx( 0, //WS_EX_APPWINDOW,
                                                  (LPCTSTR)aClass,
                                                  "NTSvcWindow",
                                                  WS_BORDER, //WS_CAPTION|WS_POPUPWINDOW,
                                                  0, 0, 10, 10,
                                                  NULL, NULL, NULL, NULL );
            if ( gpsMain->sWork.hWnd == NULL )
                __leave;

            //
            // Commander l'affichage de la fenètre et sa mise à jour.
            //
            ShowWindow( gpsMain->sWork.hWnd, SW_HIDE );
            UpdateWindow( gpsMain->sWork.hWnd );

            //
            // Message loop de la fenètre.
            //
            while ( GetMessage( &sMsg, NULL, 0, 0 ) )
            {
                TranslateMessage( &sMsg );
                DispatchMessage( &sMsg );
            }
        }
        __finally { }

        hWndSave = gpsMain->sWork.hWnd;
        gpsMain->sWork.hWnd = NULL;

        //
        // Destruction/fermeture/désallocation des objets.
        //

        if ( hWndSave != NULL )
            DestroyWindow( hWndSave );

        if ( aClass != 0 )
            UnregisterClass( (LPCSTR)aClass, GetModuleHandle( NULL ) );

        if ( hNewDesktop != NULL )
        {
            SetThreadDesktop( hSavedDesktop);
            CloseDesktop( hNewDesktop );
        }

        if ( hNewStation != NULL )
        {
            SetProcessWindowStation( hSavedStation );
            CloseWindowStation( hNewStation );
        }
    }

    ExitThread( 0 );
    return 0;
}




PRIVATE LRESULT CALLBACK NTSVCWindowManagementCallback( 
        IN      HWND hWnd,
        IN      UINT uMsg,
        IN      WPARAM wParam,
        IN      LPARAM lParam )
{
    HMENU       hMenu;
    POINT       sPos;
    UINT        uiResult;
    DWORD       dwIndex;

    if ( uMsg == gpsMain->sWork.uiWndNotify )
    {
        EnterCriticalSection( &gpsMain->sWork.sIconSerializer );

        if ( ( lParam == WM_RBUTTONDOWN       ) && 
             ( gpsMain->sWork.dwWndItems != 0 ) )
        {
            hMenu = CreatePopupMenu();

            for ( dwIndex = 0 ; dwIndex < gpsMain->sWork.dwWndItems ; dwIndex ++ )
                AppendMenu( hMenu, 
                            MF_STRING | ( gpsMain->sWork.tsWndItem[dwIndex].szText[0] == 0 ? MF_SEPARATOR : 0 ), 
                            gpsMain->sWork.tsWndItem[dwIndex].uiId, 
                            gpsMain->sWork.tsWndItem[dwIndex].szText );

            GetCursorPos( &sPos );
            SetForegroundWindow( hWnd );

            uiResult = TrackPopupMenu( hMenu,
                                       TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                       sPos.x,
                                       sPos.y,
                                       0,
                                       hWnd,
                                       NULL );

            PostMessage( hWnd, WM_NULL, 0, 0);
            DestroyMenu( hMenu );

            if ( ( uiResult > 0 ) && ( gpsMain->sWork.pfWndHandler != NULL ) )
                gpsMain->sWork.pfWndHandler( uiResult );
        }
        else if ( lParam == WM_LBUTTONDBLCLK )
        {
            if ( gpsMain->sWork.pfWndHandler != NULL )
                gpsMain->sWork.pfWndHandler( 0 );
        }

        LeaveCriticalSection( &gpsMain->sWork.sIconSerializer );
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/*-------------------------------- END OF FILE ------------------------------*/