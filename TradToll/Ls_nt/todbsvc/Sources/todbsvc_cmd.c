/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_cmd.c
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
#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <col.h>
#include <csr_list.h>
#include <trc.h>

#include <todbsvc_glob.h>
#include <todbsvc_text.h>
#include <todbsvc_lane.h>
#include <todbsvc_db.h>

#define LOC_DEF
#include <todbsvc_cmd.h>
#undef LOC_DEF

#include <memclass.h>


#define CMD_MAX_SIZE 1024
#define CMD_MAX_PARAMS 4


#define CMD_STATE_HEADER       0
#define CMD_STATE_NO_QUOTE     1
#define CMD_STATE_SINGLE_QUOTE 2
#define CMD_STATE_DOUBLE_QUOTE 3

//#pragma warning (disable : 4996)

//
// PROTOTYPES DES FONCTIONS PRIVEES
//


PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... );
PRIVATE void CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecutePrompt( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteStop( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteReload( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteParamList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteSetParam( ACOM_CNX_HANDLE hCnx, char * pcName, char * pcValue );
PRIVATE void CmdExecuteSaveParams( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteLaneList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteReinitPool( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteSuspend( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane );
PRIVATE void CmdExecuteResume( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane );
PRIVATE void CmdExecuteSetLastTreated( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane, char * pcLast );
PRIVATE void CmdExecuteStatus( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteIncorporate( ACOM_CNX_HANDLE hCnx, char * pcFilePath );
PRIVATE void CmdExecuteKill( ACOM_CNX_HANDLE hCnx );
PRIVATE BOOL CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams );
PRIVATE char * CmdTrim( char * pcStr );




//
// CODE DES FONCTIONS PROTEGEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
 * PARAMETERS: hCnx       : Handle de la connexion
 *             dwMsgSize  : Taille du message de commande
 *             pcMsg      : Message de commande
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite une commande reçue
 * --------------------------------------------------------------------
 */
PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
{
    DWORD dwIndex;
    char * tpcParams[CMD_MAX_PARAMS];
    char tszParams[CMD_MAX_PARAMS][CMD_MAX_SIZE];

    for ( dwIndex = 0 ; dwIndex < CMD_MAX_PARAMS ; dwIndex ++ )
        tpcParams[dwIndex] = tszParams[dwIndex];

    NTSVCInfo( "CmdReceive(), réception commande" );

    if      ( CmdCheck( "COMMANDLIST", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteCommandList( hCnx );

    else if ( CmdCheck( "PROMPT", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecutePrompt( hCnx );

    else if ( CmdCheck( "STOP", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteStop( hCnx );

    else if ( CmdCheck( "RELOAD", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteReload( hCnx );

    else if ( CmdCheck( "PARAMLIST", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteParamList( hCnx );

    else if ( CmdCheck( "SETPARAM", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteSetParam( hCnx, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "SAVEPARAMS", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteSaveParams( hCnx );

    else if ( CmdCheck( "LANELIST", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteLaneList( hCnx );

    else if ( CmdCheck( "REINITPOOL", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteReinitPool( hCnx );

    else if ( CmdCheck( "SUSPEND", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteSuspend( hCnx, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "RESUME", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteResume( hCnx, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "SETLASTTREATED", pcMsg, dwMsgSize, 3, tpcParams ) )
        CmdExecuteSetLastTreated( hCnx, tszParams[0], tszParams[1], tszParams[2] );

    else if ( CmdCheck( "STATUS", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteStatus( hCnx );

    else if ( CmdCheck( "INCORPORATE", pcMsg, dwMsgSize, 1, tpcParams ) )
        CmdExecuteIncorporate( hCnx, tszParams[0] );

    else if ( CmdCheck( "KILL", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteKill( hCnx );

    else 
        CmdResponse( 
                hCnx, 
                "COMMANDERROR %u,Unknown command", 
                ERROR_INVALID_DATA );
}




//
// CODE DES FONCTIONS PRIVEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... )
 * PARAMETERS: hCnx       : Handle de la connexion
 *             pcRsp      : Chaine format de la réponse (façon printf)
 *             ...        : Paramètre du format (façon printf)
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie un message de réponse sur la connexion de commande
 * --------------------------------------------------------------------
 */
PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... )
{
    int iLen;
    va_list lParm;      // Pour les paramètres du format
	char szMsg[1000];   // Buffer du message

	// Placer le curseur des paramètres au premier
	va_start( lParm, pcRsp );

    // Remplir le buffer du message avec la chaine formatée
    iLen = _vsnprintf_s( szMsg, sizeof(szMsg), sizeof(szMsg), pcRsp, lParm );
    if ( iLen < 0 ) iLen = sizeof(szMsg);

    // Envoyer la chaine sur la connexion
    AComSendMessage( 
            gsSvcWork.hWks, 
            hCnx, 
            0, 
            (DWORD)iLen, 
            szMsg );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "COMMANDLISTBEGIN NAME[,PARAMS]" );
    CmdResponse( hCnx, "COMMANDLISTITEM COMMANDLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM INCORPORATE,FILEPATH" );
    CmdResponse( hCnx, "COMMANDLISTITEM KILL" );
    CmdResponse( hCnx, "COMMANDLISTITEM LANELIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM PARAMLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM PROMPT" );
    CmdResponse( hCnx, "COMMANDLISTITEM REINITPOOL" );
    CmdResponse( hCnx, "COMMANDLISTITEM RELOAD" );
    CmdResponse( hCnx, "COMMANDLISTITEM RESUME,PLAZA|LANE" );
    CmdResponse( hCnx, "COMMANDLISTITEM SAVEPARAMS" );
    CmdResponse( hCnx, "COMMANDLISTITEM SETLASTTREATED,PLAZA|LANE|LASTTREATED" );
    CmdResponse( hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE" );
    CmdResponse( hCnx, "COMMANDLISTITEM STATUS" );
    CmdResponse( hCnx, "COMMANDLISTITEM STOP" );
    CmdResponse( hCnx, "COMMANDLISTITEM SUSPEND,PLAZA|LANE" );
    CmdResponse( hCnx, "COMMANDLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecutePrompt( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "PROMPTBEGIN" );
    CmdResponse( hCnx, "PROMPTITEM TITLE    : %s", TODBSVC_PROMPT_TITLE );
    CmdResponse( hCnx, "PROMPTITEM VERSION  : %s", TODBSVC_PROMPT_VERSION );
    CmdResponse( hCnx, "PROMPTITEM DATE     : %s", TODBSVC_PROMPT_DATE );
    CmdResponse( hCnx, "PROMPTITEM COMMENTS : %s", TODBSVC_PROMPT_COMMENTS );
    CmdResponse( hCnx, "PROMPTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteStop( ACOM_CNX_HANDLE hCnx )
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdResponse( hCnx, "STOPOK" );

    // Demander la fin
    NTSVCSignalEnd();
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteReload( ACOM_CNX_HANDLE hCnx )
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdResponse( hCnx, "RELOADOK" );

    gsSvcWork.bReload = TRUE;

    // Demander la fin
    NTSVCSignalEnd();
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteParamList( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwScan;
    DWORD dwCount = NTSVCGetParametersCount( gsSvcWork.psParams );
    NTSVC_PARAMETER_DEF * psParam;

    CmdResponse( hCnx, "PARAMLISTBEGIN NAME,VALUE" );
    for ( dwScan = 0 ; dwScan < dwCount ; dwScan ++ )
    {
        psParam = &gsSvcWork.psParams[dwScan];
        if ( psParam->dwType == REG_SZ )
            CmdResponse( hCnx, "PARAMLISTITEM %s,%s", psParam->szName, psParam->pvValue );
        else if ( psParam->dwType == REG_DWORD )
            CmdResponse( hCnx, "PARAMLISTITEM %s,%u", psParam->szName, *(DWORD*)psParam->pvValue );
        else
            CmdResponse( hCnx, "PARAMLISTITEM %s,-", psParam->szName );
    }
    CmdResponse( hCnx, "PARAMLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSetParam( ACOM_CNX_HANDLE hCnx, char * pcName, char * pcValue )
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
                strncpy_s( psParam->pvValue, psParam->dwSize, pcValue, psParam->dwSize );
                ((char*)psParam->pvValue)[psParam->dwSize-1] = '\0';
                CmdResponse( hCnx, "SETPARAMOK" );
            }
            else if ( psParam->dwType == REG_DWORD )
            {
                *(DWORD*)psParam->pvValue = atol( pcValue );
                CmdResponse( hCnx, "SETPARAMOK" );
            }
            else
                CmdResponse( hCnx, "SETPARAMERROR %u,Unknown parameter type %u for [%s]", ERROR_INVALID_PARAMETER, psParam->dwType, pcName );
            break;
        }
    }
    if ( dwItem >= dwCount )
        CmdResponse( hCnx, "SETPARAMERROR %u,Unknown parameter [%s]", ERROR_INVALID_PARAMETER, pcName );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSaveParams( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwErr;
    DWORD dwPos;

    dwErr = NTSVCSaveParameters( gsSvcWork.psParams, &dwPos );
    if ( dwErr == NO_ERROR )
        CmdResponse( hCnx, "SAVEPARAMSOK" );
    else
        CmdResponse( hCnx, "SAVEPARAMSERROR %u,Cannot save parameter %u", dwErr, dwPos );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteLaneList( ACOM_CNX_HANDLE hCnx )
{
    HCOLLECTIONITEM hItem;
    TODBSVC_LANE * psLane;

    ColLock( gsSvcWork.hLanes );

    CmdResponse( hCnx, "LANELISTBEGIN PLAZA,LANE,MODE,LASTTREATED,STATE" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hLanes, &hItem ) )
    {
        psLane = ColItemData( hItem );
        CmdResponse( hCnx, "LANELISTITEM %u,%u,%s/%s,%08u,%s", 
                psLane->sId.dwPlaza, 
                psLane->sId.dwLane, 
                psLane->bRequestToSend ? "RECOVERY" : "STANDARD",
                psLane->bUsedByPool ? "UPLOADING" : ( psLane->bSuspended ? "SUSPENDED" : "WAITING" ),
                psLane->dwLastTreated,
                psLane->bConnected ? "CONNECTED" : "DISCONNECTED" );
    }

    CmdResponse( hCnx, "LANELISTEND" );

    ColUnlock( gsSvcWork.hLanes );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteReinitPool( ACOM_CNX_HANDLE hCnx )
{
    gsSvcWork.bTerminatePool = TRUE;
    CmdResponse( hCnx, "REINITPOOLOK" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSuspend( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane )
{
    TODBSVC_LANE_ID sId;
    TODBSVC_LANE * psLane;
    HCOLLECTIONITEM hItem;

    sId.dwPlaza = atol(pcPlaza);
    sId.dwLane = atol(pcLane);

    ColLock( gsSvcWork.hLanes );
    hItem = ColItemFind( gsSvcWork.hLanes, &sId );
    if ( hItem != NULL )
    {
        psLane = ColItemData( hItem );
        psLane->bSuspended = TRUE;
        CmdResponse( hCnx, "SUSPENDOK" );
    }
    else
        CmdResponse( hCnx, "SUSPENDERROR %u,The lane does not exist", ERROR_FILE_NOT_FOUND);
    ColUnlock( gsSvcWork.hLanes );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteResume( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane )
{
    TODBSVC_LANE_ID sId;
    TODBSVC_LANE * psLane;
    HCOLLECTIONITEM hItem;

    sId.dwPlaza = atol(pcPlaza);
    sId.dwLane = atol(pcLane);

    ColLock( gsSvcWork.hLanes );
    hItem = ColItemFind( gsSvcWork.hLanes, &sId );
    if ( hItem != NULL )
    {
        psLane = ColItemData( hItem );
        psLane->bSuspended = FALSE;
        psLane->llLastDone = 0;
        psLane->bSaveLastTreated = TRUE;
        CmdResponse( hCnx, "RESUMEOK" );
    }
    else
        CmdResponse( hCnx, "RESUMEERROR %u,The lane does not exist", ERROR_FILE_NOT_FOUND);
    ColUnlock( gsSvcWork.hLanes );
}


// --------------------------------------------------------------------
PRIVATE void CmdExecuteSetLastTreated( ACOM_CNX_HANDLE hCnx, char * pcPlaza, char * pcLane, char * pcLast )
{
    DWORD dwLast;
    TODBSVC_LANE_ID sId;
    TODBSVC_LANE * psLane;
    HCOLLECTIONITEM hItem;

    sId.dwPlaza = atol(pcPlaza);
    sId.dwLane = atol(pcLane);
    dwLast = atol( pcLast );

    ColLock( gsSvcWork.hLanes );
    hItem = ColItemFind( gsSvcWork.hLanes, &sId );
    if ( hItem != NULL )
    {
        psLane = ColItemData( hItem );
        if ( psLane->bSuspended )
            if ( ! psLane->bUsedByPool )
            {
                psLane->dwLastTreated = dwLast;
                CmdResponse( hCnx, "SETLASTTREATEDOK" );
            }
            else
                CmdResponse( hCnx, "SETLASTTREATEDERROR %u,The lane has been supended but is still beeing used by the pool", ERROR_INVALID_DATA );
        else
            CmdResponse( hCnx, "SETLASTTREATEDERROR %u,The lane has not been suspended", ERROR_INVALID_DATA );
    }
    else
        CmdResponse( hCnx, "SETLASTTREATEDERROR %u,The lane does not exist", ERROR_FILE_NOT_FOUND);
    ColUnlock( gsSvcWork.hLanes );
}

// --------------------------------------------------------------------


PRIVATE void CmdExecuteStatus( ACOM_CNX_HANDLE hCnx )
{
    BOOL bDbConnect = gsSvcWork.bDatabaseMissing;
    BOOL bRouteConnect = ( gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0 );

    DWORD dwThreads;
    DWORD dwErr;

    DWORD dwHours;
    DWORD dwMinutes;
    DWORD dwSeconds;
    DWORD dwMilli;

    ULONGLONG ullIdle;
    ULONGLONG ullUser;
    ULONGLONG ullKernel;

#define SPLIT_TIME(x) \
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

    CmdResponse( hCnx, "STATUSBEGIN FLAG,STATE" );
    CmdResponse( hCnx, "STATUSITEM DATABASE,%s (%s)", 
                        gsSvcWork.sParmWork.dwLanesFileOnly ? "Use file" : ( bDbConnect ? "Offline" : "Online" ),
                        gsSvcWork.sParmWork.dwLanesFileOnly ? gsSvcWork.sParmWork.szLanesFile : ( bDbConnect ? gsSvcWork.sParmWork.szLanesFile : gsSvcWork.sParmWork.szDbInst ) );
    CmdResponse( hCnx, "STATUSITEM ROUTESVC,%s", 
                        bRouteConnect ? "Online" : "Offline" );
    CmdResponse( hCnx, "STATUSITEM RUNNINGMODE,%s", 
                        gsSvcWork.bIsDebug ? "Debug" : "Standard" );
    
    dwErr = AComGetWorkersStatistics( gsSvcWork.hWks, &dwThreads, &ullIdle, &ullUser, &ullKernel );
    if ( dwErr == NO_ERROR )
    {
        CmdResponse( hCnx, "STATUSITEM COMMAXCONCURRENTWORKERS,%u",  dwThreads );

        SPLIT_TIME(ullIdle)
        CmdResponse( hCnx, "STATUSITEM COMIDLETIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullUser)
        CmdResponse( hCnx, "STATUSITEM COMUSERTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullKernel)
        CmdResponse( hCnx, "STATUSITEM COMKERNELTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );
    }
    else
    {
        CmdResponse( hCnx, "STATUSITEM MAXCONCURRENTWORKERS,#ERR#" );
        CmdResponse( hCnx, "STATUSITEM IDLETIME,#ERR#" );
        CmdResponse( hCnx, "STATUSITEM USERTIME,#ERR#" );
        CmdResponse( hCnx, "STATUSITEM KERNELTIME,#ERR#" );
    }

    CmdResponse( hCnx, "STATUSEND" );

#undef SPLIT_TIME

}


// --------------------------------------------------------------------

PRIVATE void CmdExecuteIncorporate( ACOM_CNX_HANDLE hCnx, char * pcFilePath )
{
    DWORD dwErr;            // Code d'erreur avec interface db
    DB_CNX * hDbCnx = NULL; // Handle de la connexion avec la base

    hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
    gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
    if ( hDbCnx != NULL )
    {
        // Alimenter la base avec les données du fichier
        dwErr = DBIncorporate( &hDbCnx, pcFilePath, NULL, NULL, DB_FILE_BACKUP );
        if ( dwErr != NO_ERROR )
            CmdResponse( hCnx, "INCORPORATEERROR %u,The file has been refused", dwErr );
        else
            CmdResponse( hCnx, "INCORPORATEOK" );
        DBDisconnect( hDbCnx );
    }
    else
        CmdResponse( hCnx, "INCORPORATEERROR %u,Cannot connect to the database", ERROR_INVALID_DATA );
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteKill( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "KILLOK" );

    SVC_ERR( ERROR_INVALID_DATA, "ERR_MANUAL_KILL" );

    Sleep( 50 );

    ExitProcess( 0xDEADDEAD );
}

// --------------------------------------------------------------------

PRIVATE BOOL CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams )
{
    DWORD dwMsgSize = dwCmdSize;
    DWORD dwLen = (DWORD)strlen( pcCmdName );
    DWORD dwParamIndex = 0;
    DWORD dwState;
    BOOL bAfterQuote;
    char cCur;
    char szMsg[CMD_MAX_SIZE+1];
    char szParam[CMD_MAX_SIZE+1];
    DWORD dwMark = 0xFFFFFFFF;

    if ( dwMsgSize > CMD_MAX_SIZE )
        dwMsgSize = CMD_MAX_SIZE;
    memcpy( szMsg, pcCmdLine, dwMsgSize );
    szMsg[dwMsgSize] = '\0';
    pcCmdLine = szMsg;

    // Supprimer les espaces de tête et de fin
    pcCmdLine = CmdTrim( szMsg );

    // Comparer le début du message avec la commande à tester
    if ( _strnicmp( pcCmdName, pcCmdLine, dwLen ) != 0 )
        return FALSE;

    // Si ce n'est pas une fin de chaine ou un espace qui suit la commande, ce n'est pas
    // la command qu'on cherche
    pcCmdLine += dwLen;
    cCur = *pcCmdLine;
    if ( ( cCur != '\0' ) && ( cCur != ' ' ) && ( cCur != '\t' ) )
        return FALSE;

    // Si commande sans paramètre et reste chaine vide, c'est OK
    if ( cCur == '\0' )
        return ( dwParams == 0 );

    // Passer le premier séparateur
    pcCmdLine ++;

    do
    {
        dwLen = 0;
        dwState = CMD_STATE_HEADER;
        bAfterQuote = FALSE;
        while ( ( cCur = *pcCmdLine ) != '\0' )
        {
            if ( ( cCur == ',' ) && ( ( dwState == CMD_STATE_NO_QUOTE ) || ( dwState == CMD_STATE_HEADER ) ) )
                break;

            else if ( ( cCur == '\"' ) && ( ( dwState == CMD_STATE_NO_QUOTE ) || ( dwState == CMD_STATE_HEADER ) ) )
            {
                if ( ! bAfterQuote && ( dwMark != 0xFFFFFFFF ) )
                    dwLen = dwMark;
                dwMark = 0xFFFFFFFF;
                bAfterQuote = FALSE;
                dwState = CMD_STATE_DOUBLE_QUOTE;
            }

            else if ( ( cCur == '\"' ) && ( dwState == CMD_STATE_DOUBLE_QUOTE ) )
            {
                dwMark = 0xFFFFFFFF;
                bAfterQuote = TRUE;
                dwState = CMD_STATE_NO_QUOTE;
            }

            else if ( ( cCur == '\'' ) && ( ( dwState == CMD_STATE_NO_QUOTE ) || ( dwState == CMD_STATE_HEADER ) ) )
            {
                if ( ! bAfterQuote && ( dwMark != 0xFFFFFFFF ) )
                    dwLen = dwMark;
                dwMark = 0xFFFFFFFF;
                bAfterQuote = TRUE;
                dwState = CMD_STATE_SINGLE_QUOTE;
            }

            else if ( ( cCur == '\'' ) && ( dwState == CMD_STATE_SINGLE_QUOTE ) )
            {
                dwMark = 0xFFFFFFFF;
                bAfterQuote = FALSE;
                dwState = CMD_STATE_NO_QUOTE;
            }

            else if ( ( cCur == ' ' ) || ( cCur == '\t' ) )
            {
                if ( dwState != CMD_STATE_HEADER )
                {
                    if ( dwState != CMD_STATE_NO_QUOTE )
                        dwMark = 0xFFFFFFFF;
                    else if ( dwMark == 0xFFFFFFFF )
                        dwMark = dwLen;

                    if ( ! bAfterQuote )
                        szParam[dwLen++] = cCur;
                }
            }
            else
            {
                dwMark = 0xFFFFFFFF;

                if ( cCur == '\\' )
                {
                    pcCmdLine ++;
                    if ( ( cCur = *pcCmdLine ) == '\0' )
                        break;
                }

                if ( dwState == CMD_STATE_HEADER )
                    dwState = CMD_STATE_NO_QUOTE;
                if ( ( dwState == CMD_STATE_NO_QUOTE ) || ( dwState == CMD_STATE_HEADER ) );
                    bAfterQuote = FALSE;
                szParam[dwLen++] = cCur;
            }

            pcCmdLine ++;
        }
        if ( dwMark != 0xFFFFFFFF )
            dwLen = dwMark;

        szParam[dwLen] = '\0';
        strcpy_s( ppcParams[dwParamIndex], CMD_MAX_SIZE, szParam );

        dwParams --;                    
        szParam[dwLen++] = cCur;

        if ( cCur == '\0' )
            break;
        else
            if ( dwParams == 0 )
                return FALSE;

        dwParamIndex ++;
        pcCmdLine ++;
    }
    while ( TRUE );

    return ( dwParams == 0 );
}



PRIVATE char * CmdTrim( char * pcStr )
{
    unsigned char * pcSrc = pcStr;
    unsigned char * pcDst = pcStr;
    unsigned char * pcStop = NULL;

    while ( ( *pcSrc != '\0' ) && ( *pcSrc <= ( unsigned char)' ' ) ) pcSrc ++;
    while ( *pcSrc != '\0' )
    {
        *pcDst = *pcSrc;
        if ( ( pcStop == NULL ) && ( *pcDst <=(unsigned char)' ' ) )
            pcStop = pcDst;
        else if ( *pcDst > (unsigned char)' ' )
            pcStop = NULL;
        pcDst ++;
        pcSrc ++;
    }
    if ( pcStop != NULL ) 
        *pcStop = '\0';
    else
        *pcDst = '\0';
    return pcStr;
}


