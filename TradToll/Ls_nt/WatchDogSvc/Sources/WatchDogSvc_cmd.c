/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WATCHDOGsvc
 * FILE       : WATCHDOGsvc_cmd.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : commande, controle, service
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des commandes de contrôle du service
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

#include <WATCHDOGsvc_glob.h>
#include <WATCHDOGsvc_text.h>
#include <WATCHDOGsvc_cmdgen.h>

#define LOC_DEF
#include <WATCHDOGsvc_cmd.h>
#undef LOC_DEF

#include <memclass.h>


PRIVATE void WINAPI CmdExecuteStatus( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteSaveParams( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteSetParam( ACOM_CNX_HANDLE hCnx, char * pcName, char * pcValue );
PRIVATE void WINAPI CmdExecuteParamList( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteReload( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteStop( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecutePrompt( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteServiceList( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecutePause( ACOM_CNX_HANDLE hCnx );
PRIVATE void WINAPI CmdExecuteResume( ACOM_CNX_HANDLE hCnx );


// --------------- CODE ----------------------




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
 * PARAMETERS: hCnx       : Index de la connexion de commande
 *             dwMsgSize  : Taille du message de commande
 *             pcMsg      : Message de commande
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite une commande reçue
 * --------------------------------------------------------------------
 */
PROTECTED void WINAPI CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
{
    DWORD dwIndex;
    char * tpcParams[CMD_MAX_PARAMS];
    char tszParams[CMD_MAX_PARAMS][CMD_MAX_SIZE];

    for ( dwIndex = 0 ; dwIndex < CMD_MAX_PARAMS ; dwIndex ++ )
        tpcParams[dwIndex] = tszParams[dwIndex];

    NTSVCInfo( "CmdReceive(), réception commande sur dwIndex=%u", hCnx );
 
    if      ( CmdCheck( "COMMANDLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteCommandList( hCnx );

    else if ( CmdCheck( "PROMPT", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecutePrompt( hCnx );

    else if ( CmdCheck( "PAUSE", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecutePause( hCnx );

    else if ( CmdCheck( "RESUME", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteResume( hCnx );

    else if ( CmdCheck( "STOP", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteStop( hCnx );

    else if ( CmdCheck( "RELOAD", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteReload( hCnx );

    else if ( CmdCheck( "PARAMLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteParamList( hCnx );

    else if ( CmdCheck( "SETPARAM", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteSetParam( hCnx, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "SAVEPARAMS", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteSaveParams( hCnx );

    else if ( CmdCheck( "STATUS", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteStatus( hCnx );

    else if ( CmdCheck( "SERVICELIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteServiceList( hCnx );

    else 
        CmdMessage( 
                hCnx, 
                "COMMANDERROR %u,Unknown command", 
                ERROR_INVALID_DATA );
}



PRIVATE void WINAPI CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx )
{
    CmdMessage( hCnx, "COMMANDLISTBEGIN NAME[,PARAMS]" );
    CmdMessage( hCnx, "COMMANDLISTITEM COMMANDLIST" );
    CmdMessage( hCnx, "COMMANDLISTITEM PARAMLIST" );
    CmdMessage( hCnx, "COMMANDLISTITEM PAUSE" );
    CmdMessage( hCnx, "COMMANDLISTITEM PROMPT" );
    CmdMessage( hCnx, "COMMANDLISTITEM RELOAD" );
    CmdMessage( hCnx, "COMMANDLISTITEM RESUME" );
    CmdMessage( hCnx, "COMMANDLISTITEM SAVEPARAMS" );
    CmdMessage( hCnx, "COMMANDLISTITEM SERVICELIST" );
    CmdMessage( hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE" );
    CmdMessage( hCnx, "COMMANDLISTITEM STATUS" );
    CmdMessage( hCnx, "COMMANDLISTITEM STOP" );
    CmdMessage( hCnx, "COMMANDLISTEND" );
}




PRIVATE void WINAPI CmdExecutePrompt( ACOM_CNX_HANDLE hCnx )
{
    CmdMessage( hCnx, "PROMPTBEGIN" );
    CmdMessage( hCnx, "PROMPTITEM TITLE    : %s", WATCHDOGSVC_PROMPT_TITLE );
    CmdMessage( hCnx, "PROMPTITEM VERSION  : %s", WATCHDOGSVC_PROMPT_VERSION );
    CmdMessage( hCnx, "PROMPTITEM DATE     : %s", WATCHDOGSVC_PROMPT_DATE );
    CmdMessage( hCnx, "PROMPTITEM COMMENTS : %s", WATCHDOGSVC_PROMPT_COMMENTS );
    CmdMessage( hCnx, "PROMPTEND" );
}




PRIVATE void WINAPI CmdExecuteStop( ACOM_CNX_HANDLE hCnx )
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdMessage( hCnx, "STOPOK" );

    // Demander la fin
    NTSVCSignalEnd();
}




PRIVATE void WINAPI CmdExecuteReload( ACOM_CNX_HANDLE hCnx )
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdMessage( hCnx, "RELOADOK" );

    gsSvcWork.bReload = TRUE;

    // Demander la fin
    NTSVCSignalEnd();
}




PRIVATE void WINAPI CmdExecuteParamList( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwScan;
    DWORD dwCount = NTSVCGetParametersCount( gsSvcWork.psParams );
    NTSVC_PARAMETER_DEF * psParam;

    CmdMessage( hCnx, "PARAMLISTBEGIN NAME,VALUE" );
    for ( dwScan = 0 ; dwScan < dwCount ; dwScan ++ )
    {
        psParam = &gsSvcWork.psParams[dwScan];
        if ( psParam->dwType == REG_SZ )
            CmdMessage( hCnx, "PARAMLISTITEM %s,\"%s\"", psParam->szName, psParam->pvValue );
        else if ( psParam->dwType == REG_DWORD )
            CmdMessage( hCnx, "PARAMLISTITEM %s,%u", psParam->szName, *(DWORD*)psParam->pvValue );
        else
            CmdMessage( hCnx, "PARAMLISTITEM %s,-", psParam->szName );
    }
    CmdMessage( hCnx, "PARAMLISTEND" );
}




PRIVATE void WINAPI CmdExecuteSetParam( ACOM_CNX_HANDLE hCnx, char * pcName, char * pcValue )
{
    DWORD dwCount = NTSVCGetParametersCount( gsSvcWork.psParams );
    DWORD dwItem;
    NTSVC_PARAMETER_DEF * psParam;

    for ( dwItem = 0 ; dwItem < dwCount ; dwItem ++ )
    {
        psParam = &gsSvcWork.psParams[dwItem];
        if ( _stricmp( pcName, psParam->szName ) == 0 )
        {
            if ( psParam->dwType == REG_SZ )
            {
                strncpy_s( psParam->pvValue, psParam->dwSize, pcValue, _TRUNCATE );
                ((char*)psParam->pvValue)[psParam->dwSize-1] = '\0';
                CmdMessage( hCnx, "SETPARAMOK" );
            }
            else if ( psParam->dwType == REG_DWORD )
            {
                *(DWORD*)psParam->pvValue = atol( pcValue );
                CmdMessage( hCnx, "SETPARAMOK" );
            }
            else
                CmdMessage( hCnx, "SETPARAMERROR %u,Unknown parameter type %u for [%s]", ERROR_INVALID_PARAMETER, psParam->dwType, pcName );
            break;
        }
    }
    if ( dwItem >= dwCount )
        CmdMessage( hCnx, "SETPARAMERROR %u,Unknown parameter [%s]", ERROR_INVALID_PARAMETER, pcName );
}




PRIVATE void WINAPI CmdExecuteSaveParams( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwErr;
    DWORD dwPos;

    dwErr = NTSVCSaveParameters( gsSvcWork.psParams, &dwPos );
    if ( dwErr == NO_ERROR )
        CmdMessage( hCnx, "SAVEPARAMSOK" );
    else
        CmdMessage( hCnx, "SAVEPARAMSERROR %u,Cannot save parameter %u", dwErr, dwPos );
}






PRIVATE void WINAPI CmdExecuteStatus( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwThreads;
    DWORD dwErr;

    DWORD dwHours;
    DWORD dwMinutes;
    DWORD dwSeconds;
    DWORD dwMilli;

    ULONGLONG ullIdle;
    ULONGLONG ullUser;
    ULONGLONG ullKernel;

#   define SPLIT_TIME(x) \
        {\
            ULONGLONG ullWork = ( (x) / ((ULONGLONG)10000) );\
            dwHours = (DWORD)( ullWork / ((ULONGLONG)3600000) );\
            ullWork = ullWork % 3600000;\
            dwMinutes = (DWORD)( ullWork / ((ULONGLONG)60000) );\
            ullWork = ullWork % 60000;\
            dwSeconds = (DWORD)( ullWork / ((ULONGLONG)1000) );\
            ullWork = ullWork % 1000;\
            dwMilli = (DWORD)ullWork;\
        }

    CmdMessage( hCnx, "STATUSBEGIN FLAG,STATE" );

    CmdMessage( hCnx, "STATUSITEM RUNNINGSTATUS,%s", 
                        gsSvcWork.fPaused ? "Paused" : "Running" );

    CmdMessage( hCnx, "STATUSITEM RUNNINGMODE,%s", 
                        gsSvcWork.bIsDebug ? "Debug" : "Standard" );
    
    dwErr = AComGetWorkersStatistics( gsSvcWork.hWks, &dwThreads, &ullIdle, &ullUser, &ullKernel );
    if ( dwErr == NO_ERROR )
    {
        CmdMessage( hCnx, "STATUSITEM COMMAXCONCURRENTWORKERS,%u",  dwThreads );

        SPLIT_TIME(ullIdle)
        CmdMessage( hCnx, "STATUSITEM COMIDLETIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullUser)
        CmdMessage( hCnx, "STATUSITEM COMUSERTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullKernel)
        CmdMessage( hCnx, "STATUSITEM COMKERNELTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );
    }
    else
    {
        CmdMessage( hCnx, "STATUSITEM MAXCONCURRENTWORKERS,#ERR#" );
        CmdMessage( hCnx, "STATUSITEM IDLETIME,#ERR#" );
        CmdMessage( hCnx, "STATUSITEM USERTIME,#ERR#" );
        CmdMessage( hCnx, "STATUSITEM KERNELTIME,#ERR#" );
    }

    CmdMessage( hCnx, "STATUSEND" );

#   undef SPLIT_TIME
}




PRIVATE void WINAPI CmdExecuteServiceList( ACOM_CNX_HANDLE hCnx )
{
    DWORD               dwIndex;
    WATCHDOGSVC_SVC   * psSvc; 

    EnterCriticalSection( &gsSvcWork.sProtectSvc );

    CmdMessage( hCnx, "SERVICELISTBEGIN NAME,WAITRESTART,STATUS" );

    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwServices ; dwIndex ++ )
    {
        psSvc = &(gsSvcWork.psServices[dwIndex]);
        CmdMessage( hCnx, "SERVICELISTITEM %s,%u,%s",
                    psSvc->szName,
                    psSvc->dwWaitToRestart,
                    ( psSvc->dwLastState == SERVICE_CONTINUE_PENDING ? "SERVICE_CONTINUE_PENDING" :
                      psSvc->dwLastState == SERVICE_PAUSE_PENDING    ? "SERVICE_PAUSE_PENDING"    :
                      psSvc->dwLastState == SERVICE_PAUSED           ? "SERVICE_PAUSED"           :
                      psSvc->dwLastState == SERVICE_RUNNING          ? "SERVICE_RUNNING"          :
                      psSvc->dwLastState == SERVICE_START_PENDING    ? "SERVICE_START_PENDING"    :
                      psSvc->dwLastState == SERVICE_STOP_PENDING     ? "SERVICE_STOP_PENDING"     :
                      psSvc->dwLastState == SERVICE_STOPPED          ? "SERVICE_STOPPED"          :
                      "SERVICE_UNKNOWN" ) );
    }

    CmdMessage( hCnx, "SERVICELISTEND" );

    LeaveCriticalSection( &gsSvcWork.sProtectSvc );
}



PRIVATE void WINAPI CmdExecutePause( ACOM_CNX_HANDLE hCnx )
{
    gsSvcWork.fPaused = TRUE;
    NTSVCDefineTrayIconMenu( NTSVC_MENU_REMOVE, 1, "Pause" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 2, "Resume" );
    CmdMessage( hCnx, "PAUSEOK" );
}


PRIVATE void WINAPI CmdExecuteResume( ACOM_CNX_HANDLE hCnx )
{
    gsSvcWork.fPaused = FALSE;
    NTSVCDefineTrayIconMenu( NTSVC_MENU_REMOVE, 2, "Resume" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 1, "Pause" );
    CmdMessage( hCnx, "RESUMEOK" );
}


