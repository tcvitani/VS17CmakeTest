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
#include <trc.h>
#include <ntsvc.h>

#define LOC_DEF
	#include <ntsvc_main.h>
#undef LOC_DEF

#include <ntsvc_install.h>
#include <ntsvc_mgmt.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE int WINAPI NTSVCMainEntryPoint( 
        IN      int iArgc,
        IN      char ** ppcArgv );

PRIVATE DWORD WINAPI NTSVCMakeError( 
        IN      DWORD dwErr, 
        IN      char * pcText );

PRIVATE DWORD NTSVCRestartService();

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED int __cdecl main( 
        IN      int     iArgc,
        IN      char ** ppcArgv )
{
    gpsMain = &gsMain;
    ZeroMemory( gpsMain, sizeof( *gpsMain ) );
    gpsMain->sFlags.bConsole = TRUE;
    return (int)NTSVCMainEntryPoint( iArgc, ppcArgv );
}


PROTECTED int WINAPI WinMain( 
        IN      HINSTANCE hInst,
        IN      HINSTANCE hPrev,
        IN      LPSTR lpCmdLine,
        IN      int nCmdShow )
{
    int iArgc;
    int iRet;
    char ** ppcArgv;

    gpsMain = &gsMain;
    ZeroMemory( gpsMain, sizeof( *gpsMain ) );
    gpsMain->sFlags.bConsole = FALSE;
    ppcArgv = CMDLOpenArgv( GetCommandLine(), &(DWORD)iArgc );
    iRet = (int)NTSVCMainEntryPoint( iArgc, ppcArgv );
    CMDLCloseArgv( ppcArgv );
    return iRet;
}



PRIVATE int WINAPI NTSVCMainEntryPoint( 
        IN      int iArgc,
        IN      char ** ppcArgv )
{
    DWORD dwIndex;
    DWORD dwErr = NO_ERROR;
    BOOL bHelp = FALSE;
    BOOL bDebugMode = FALSE;
    BOOL bCommandMode = FALSE;
    BOOL bInstall = FALSE;
    BOOL bRemove = FALSE;
    BOOL bExcptRestart = FALSE;
    DWORD dwStartType = SERVICE_DEMAND_START;
    BOOL bAsAccount = FALSE;
    DWORD dwDepIndex = 0;
    NTSVCCommandMain * pfCommand;
    char szUser[CMDL_MAX_PARAM_SIZE] = "";
    char szPassword[CMDL_MAX_PARAM_SIZE] = "";
    char szDepList[CMDL_MAX_PARAM_SIZE] = "\0\0";
    char szFilePath[CMDL_MAX_PARAM_SIZE] = "";
    char szParams[CMDL_MAX_PARAM_SIZE] = "";

    pfCommand = NULL;
    gpsMain->sFlags.pcServiceName = NULL;
    if ( ! NTSVCExternalQueryInfo( &gpsMain->sFlags.pcServiceName, &pfCommand ) )
        return NTSVCMakeError( ERROR_INVALID_DATA, "Service definition" );

    if ( gpsMain->sFlags.pcServiceName == NULL )
        return NTSVCMakeError( ERROR_INVALID_DATA, "Bad service implementation" );

    if ( iArgc > 1 )
    {
        for ( dwIndex = 1 ; dwIndex < (DWORD)iArgc ; dwIndex ++ )
        {
            if ( CMDLParamAsFlag( "/HELP", ppcArgv[dwIndex] ) ||
                 CMDLParamAsFlag( "/?"   , ppcArgv[dwIndex] ) )
            {
                bHelp = TRUE;
                continue;
            }

            if ( CMDLParamAsFlag( "/INSTALL", ppcArgv[dwIndex] ) )
            {
                bInstall = TRUE;
                continue;
            }

            if ( CMDLParamAsFlag( "/REMOVE", ppcArgv[dwIndex] ) )
            {
                bRemove = TRUE;
                continue;
            }
        
            if ( CMDLParamAsEnum( "/START=", ppcArgv[dwIndex], &dwStartType, 3,
                                 "AUTO", SERVICE_AUTO_START,
                                 "DEMAND", SERVICE_DEMAND_START,
                                 "DISABLED", SERVICE_DISABLED ) )
            {
                continue;
            }

            if ( CMDLParamAsString( "/USER=", ppcArgv[dwIndex], sizeof(szUser) , szUser ) )
            {
                bAsAccount = TRUE;
                continue;
            }


            if ( CMDLParamAsString( "/PASSWORD=", ppcArgv[dwIndex], sizeof(szPassword) , szPassword ) )
            {
                bAsAccount = TRUE;
                continue;
            }
        
            if ( CMDLParamAsString( "/DEP=", ppcArgv[dwIndex], sizeof(szDepList) - dwDepIndex - 1 , szDepList + dwDepIndex ) )
            {
                dwDepIndex += ( (DWORD)strlen( szDepList + dwDepIndex ) + 1 );
                szDepList[dwDepIndex] = '\0';
                continue;
            }

            if ( CMDLParamAsFlag( "/DEBUG", ppcArgv[dwIndex] ) )
            {
                bDebugMode = TRUE;
                continue;
            }

            if ( CMDLParamAsFlag( "/EXCPTRESTART", ppcArgv[dwIndex] ) )
            {
                bExcptRestart = TRUE;
                continue;
            }

            if ( CMDLParamAsFlag( "/COMMAND", ppcArgv[dwIndex] ) )
            {
                bCommandMode = TRUE;
                continue;
            }

            if ( CMDLParamAsString( "/PARAMS=", ppcArgv[dwIndex], sizeof(szParams) , szParams ) )
            {
                continue;
            }

            dwErr = ERROR_INVALID_PARAMETER;
            return NTSVCMakeError( dwErr, "Unknown command" );
        }

        if ( bHelp )
        {
            printf( "\n%s service command line help.", gpsMain->sFlags.pcServiceName );
            printf( "\n" );
            printf( "\n Command line parameters :" );
            printf( "\n   /HELP  or  /?         - Show this help text." );
            printf( "\n   /INSTALL [InstParms]  - Registers the service." );
            printf( "\n   /REMOVE               - Unregisters the service." );
            printf( "\n   /DEBUG                - Run the service using the debug mode." );
            if ( pfCommand != NULL )
                printf( "\n   /COMMAND [CmdParams] - Use the command line interface services." );
            printf( "\n" );
            printf( "\n With InstParams, a combination of :" );
            printf( "\n   [/START=AUTO]         - Automatic startup." );
            printf( "\n   [/START=DEMAND]       - Manual startup." );
            printf( "\n   [/START=DISABLED]     - Service disabled." );
            printf( "\n   [/USER=[Domain\\]User] - Starts under a particular user account." );
            printf( "\n   [/PASSWORD=Pass]      - Password for the account." );
            printf( "\n   [/DEP=Dependency]     - Dependent service or group (repeated as needed)." );
            printf( "\n" );
            printf( "\n With CmdParams :" );
            printf( "\n   [/PARAMS=CmdParams]   - Command line interface parameters." );
            printf( "\n\n" );
            return NTSVCMakeError( NO_ERROR, "Service help." );
        }

        if ( bInstall )
        {
            if ( GetModuleFileName( NULL, szFilePath, sizeof( szFilePath ) ) == 0 )
                dwErr = GetLastError();
            else
            {
                dwErr = NTSVCInstall(
                    szFilePath,
                    gpsMain->sFlags.pcServiceName,
                    dwStartType,
                    szDepList,
                    bAsAccount ? szUser : NULL,
                    bAsAccount ? szPassword : NULL,
                    ! bAsAccount );
                return NTSVCMakeError( dwErr, "Service installation" );
            }
        }

        else if ( bRemove )
        {
            dwErr = NTSVCRemove( gpsMain->sFlags.pcServiceName );
            return NTSVCMakeError( dwErr, "Service deletion" );
        }

        else if ( bCommandMode )
        {
            if ( pfCommand == NULL )
                return NTSVCMakeError( ERROR_INVALID_PARAMETER, "Command mode not implemented" );
            else
            {
                dwErr = pfCommand( szParams );
                return NTSVCMakeError( dwErr, "Command mode" );
            }
        }

        else if ( bExcptRestart )
        {
            dwErr = NTSVCRestartService();
            return NTSVCMakeError( dwErr, "Exception restart" );
        }

        else if ( ! bDebugMode )
            return NTSVCMakeError( ERROR_INVALID_PARAMETER, "Command line" );
    }

    dwErr = NTSVCStart(
        gpsMain->sFlags.pcServiceName,
        NTSVCExternalMain,
        NTSVC_START_ACKNOWLEDGE_DELAY,
        NTSVC_STOP_ACKNOWLEDGE_DELAY,
        bDebugMode ? szParams : NULL );
    
    return NTSVCMakeError( dwErr, gpsMain->sFlags.pcServiceName );
}


PRIVATE DWORD WINAPI NTSVCMakeError( 
        IN      DWORD dwErr, 
        IN      char * pcText )
{
    DWORD dwResult;
    DWORD dwIcon;
    char * pcScan;
    char szCaption[100] = "";
    char szMsgText[1000] = "";
    char szErrText[1000] = "";

    if ( dwErr != NO_ERROR )
    {
        dwResult = FormatMessage( 
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErr,
                0,
                szErrText,
                sizeof(szErrText),
                NULL );
        if ( dwResult == 0 )
            strcpy_s( szErrText, 1000, "Unknown error" );
        else
            for ( pcScan = szErrText ; *pcScan != '\0' ; pcScan ++ )
                if ( *pcScan < ' ' ) *pcScan = '.';

        _snprintf_s( szCaption, 100, sizeof(szCaption)-1, "ERR %010u / 0x%08X", dwErr, dwErr );
        _snprintf_s( szMsgText, 1000, sizeof(szMsgText)-1, "%s : %s", pcText, szErrText );

        dwIcon = MB_ICONERROR;
    }
    else
    {
        _snprintf_s( szCaption, 100, sizeof(szCaption)-1, "SUCCESS" );
        _snprintf_s( szMsgText, 1000, sizeof(szMsgText)-1, "%s", pcText );

        dwIcon = MB_ICONINFORMATION;
    }
    szCaption[sizeof(szCaption)-1] = '\0';
    szMsgText[sizeof(szMsgText)-1] = '\0';

    if ( gpsMain->sFlags.bConsole )
        printf( "%s : %s\n", szCaption, szMsgText );
    else
        if ( gpsMain->sFlags.bSimulated )
            MessageBox( NULL, szMsgText, szCaption, MB_OK | dwIcon );

    return dwErr;
}



 PRIVATE DWORD NTSVCRestartService()
{
	DWORD dwErr = NO_ERROR;
	SC_HANDLE hSvc = NULL;
	SC_HANDLE hMng = NULL;
    SERVICE_STATUS sStatus;

    hMng = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( hMng == NULL ) goto ErrorHandler;

	hSvc = OpenService(
		hMng,
		gpsMain->sFlags.pcServiceName,
		SERVICE_ALL_ACCESS );
	if ( hSvc == NULL ) goto ErrorHandler;

    ControlService( hSvc, SERVICE_CONTROL_STOP, &sStatus );

    do
    {
        Sleep( 1000 );
        if ( ! QueryServiceStatus( hSvc, &sStatus ) ) goto ErrorHandler;
    }
    while ( sStatus.dwCurrentState != SERVICE_STOPPED );

    if ( ! StartService( hSvc, 0, NULL ) ) goto ErrorHandler;

    SetLastError( NO_ERROR );

ErrorHandler:
    dwErr = GetLastError();

    if ( hSvc != NULL )
        CloseServiceHandle( hSvc );
	if ( hMng != NULL )
		CloseServiceHandle( hMng );

	return dwErr;
}

 /*-------------------------------- END OF FILE ------------------------------*/