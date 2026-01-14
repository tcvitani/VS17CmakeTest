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
#include <col.h>
#include <ntsvc.h>

#include <csr_rfa_svc.h>
#include <csr_rfa.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>
#include <trfsvc_protocol.h>

#define LOC_DEF
#include <trfsvc_cmd.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

#define CMD_MAX_SIZE 1024
#define CMD_MAX_PARAMS 4


#define CMD_STATE_HEADER       0
#define CMD_STATE_NO_QUOTE     1
#define CMD_STATE_SINGLE_QUOTE 2
#define CMD_STATE_DOUBLE_QUOTE 3



PRIVATE void CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecutePrompt( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteStop( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteReload( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteParamList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteSetParam( ACOM_CNX_HANDLE hCnx, char * pcName, char * pcValue );
PRIVATE void CmdExecuteSaveParams( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteGetCopy( ACOM_CNX_HANDLE hCnx, char * pcFileKey, char * pcCopyFile, char * pcOverwrite );
PRIVATE void CmdExecutePutCopy( ACOM_CNX_HANDLE hCnx, char * pcFileKey, char * pcSourceFile, char * pcOverwrite );
PRIVATE void CmdExecuteFileList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecuteFileStatus( ACOM_CNX_HANDLE hCnx, char * pcFileKey );
PRIVATE void CmdExecuteClientList( ACOM_CNX_HANDLE hCnx );
PRIVATE void CmdExecutePingClient( ACOM_CNX_HANDLE hCnx, char * szName );
PRIVATE void CmdExecutePingServer( ACOM_CNX_HANDLE hCnx, char * szFileKey );
PRIVATE void CmdExecuteStatus( ACOM_CNX_HANDLE hCnx );


PRIVATE BOOL CmdCheck( char * pcCmdName, 
					char * pcCmdLine, 
					DWORD dwCmdSize, 
					DWORD dwParams, 
					char ** ppcParams, 
					DWORD dwParameElmentSize);

PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... );
PRIVATE char * CmdTrim( char * pcStr );


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
PROTECTED void CmdReceive(  ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
{
    DWORD dwIndex;
    char * tpcParams[CMD_MAX_PARAMS];
    char tszParams[CMD_MAX_PARAMS][CMD_MAX_SIZE];

    for ( dwIndex = 0 ; dwIndex < CMD_MAX_PARAMS ; dwIndex ++ )
        tpcParams[dwIndex] = tszParams[dwIndex];

    NTSVCInfo( "CmdReceive(), réception commande sur dwIndex=%u", hCnx );
 
    if      ( CmdCheck( "COMMANDLIST", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteCommandList( hCnx );

    else if ( CmdCheck( "PROMPT", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecutePrompt( hCnx );

    else if ( CmdCheck( "STOP", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteStop( hCnx );

    else if ( CmdCheck( "RELOAD", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteReload( hCnx );

    else if ( CmdCheck( "PARAMLIST", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteParamList( hCnx );

    else if ( CmdCheck( "SETPARAM", pcMsg, dwMsgSize, 2, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteSetParam( hCnx, tpcParams[0], tpcParams[1] );

    else if ( CmdCheck( "SAVEPARAMS", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteSaveParams( hCnx );

    else if ( CmdCheck( "GETCOPY", pcMsg , dwMsgSize, 3, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteGetCopy( hCnx, tpcParams[0], tpcParams[1], tpcParams[2] );

    else if ( CmdCheck( "PUTCOPY", pcMsg , dwMsgSize, 3, tpcParams, CMD_MAX_SIZE ) )
        CmdExecutePutCopy( hCnx, tpcParams[0], tpcParams[1], tpcParams[2] );

    else if ( CmdCheck( "FILELIST", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteFileList( hCnx );

    else if ( CmdCheck( "FILESTATUS", pcMsg , dwMsgSize, 1, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteFileStatus( hCnx, tpcParams[0] );

    else if ( CmdCheck( "CLIENTLIST", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteClientList( hCnx );
    
    else if ( CmdCheck( "PINGCLIENT", pcMsg , dwMsgSize, 1, tpcParams, CMD_MAX_SIZE ) )
        CmdExecutePingClient( hCnx, tpcParams[0] );
    
    else if ( CmdCheck( "PINGSERVER", pcMsg , dwMsgSize, 1, tpcParams, CMD_MAX_SIZE ) )
        CmdExecutePingServer( hCnx, tpcParams[0] );
    
    else if ( CmdCheck( "STATUS", pcMsg , dwMsgSize, 0, tpcParams, CMD_MAX_SIZE ) )
        CmdExecuteStatus( hCnx );
    
    else 
        CmdResponse( 
                hCnx, 
                "COMMANDERROR %u,Unknown command", 
                ERROR_INVALID_DATA );
}


// --------------------------------------------------------------------
PRIVATE void CmdExecuteCommandList( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "COMMANDLISTBEGIN NAME[,PARAMS]" );
    CmdResponse( hCnx, "COMMANDLISTITEM CLIENTLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM COMMANDLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM FILELIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM FILESTATUS,KEY" );
    CmdResponse( hCnx, "COMMANDLISTITEM GETCOPY,KEY|PATH|OVERWRITE" );
    CmdResponse( hCnx, "COMMANDLISTITEM PARAMLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM PINGCLIENT,CLIENTID" );
    CmdResponse( hCnx, "COMMANDLISTITEM PINGSERVER,KEY" );
    CmdResponse( hCnx, "COMMANDLISTITEM PROMPT" );
    CmdResponse( hCnx, "COMMANDLISTITEM PUTCOPY,KEY|PATH|OVERWRITE" );
    CmdResponse( hCnx, "COMMANDLISTITEM RELOAD" );
    CmdResponse( hCnx, "COMMANDLISTITEM SAVEPARAMS" );
    CmdResponse( hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE" );
    CmdResponse( hCnx, "COMMANDLISTITEM STATUS" );
    CmdResponse( hCnx, "COMMANDLISTITEM STOP" );
    CmdResponse( hCnx, "COMMANDLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecutePrompt( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "PROMPTBEGIN" );
    CmdResponse( hCnx, "PROMPTITEM TITLE    : %s", TRFSVC_PROMPT_TITLE );
    CmdResponse( hCnx, "PROMPTITEM VERSION  : %s", TRFSVC_PROMPT_VERSION );
    CmdResponse( hCnx, "PROMPTITEM DATE     : %s", TRFSVC_PROMPT_DATE );
    CmdResponse( hCnx, "PROMPTITEM COMMENTS : %s", TRFSVC_PROMPT_COMMENTS );
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
            CmdResponse( hCnx, "PARAMLISTITEM %s,\"%s\"", psParam->szName, psParam->pvValue );
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
				strncpy_s(psParam->pvValue, sizeof(psParam->pvValue), pcValue, psParam->dwSize);
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


PRIVATE void CmdExecuteGetCopy( ACOM_CNX_HANDLE hCnx, char * pcFileKey, char * pcCopyFile, char * pcOverwrite )
{
    BOOL bResult;
    DWORD dwErr;

    bResult = RFACopyFileKey( pcFileKey, pcCopyFile, atol(pcOverwrite) == 0 );
    if ( ! bResult )
    {
        dwErr = GetLastError();
        CmdResponse( hCnx, "GETCOPYERROR %u,Cannot copy key [%s] to file [%s]", dwErr, pcFileKey, pcCopyFile );
    }
    else
        CmdResponse( hCnx, "GETCOPYOK" );
}

// --------------------------------------------------------------------

PRIVATE void CmdExecutePutCopy( ACOM_CNX_HANDLE hCnx, char * pcFileKey, char * pcSourceFile, char * pcOverwrite )
{
    BOOL bResult;
    DWORD dwErr;

    bResult = RFACopyFile( pcSourceFile, pcFileKey, atol(pcOverwrite) == 0 );
    if ( ! bResult )
    {
        dwErr = GetLastError();
        CmdResponse( hCnx, "PUTCOPYERROR %u,Cannot copy file [%s] to key [%s]", dwErr, pcFileKey, pcSourceFile );
    }
    else
        CmdResponse( hCnx, "PUTCOPYOK" );
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteFileList( ACOM_CNX_HANDLE hCnx )
{
    HCOLLECTIONITEM hItem;
    TRFSVC_FILE * psFile = NULL;

    CmdResponse( hCnx, "FILELISTBEGIN FILEKEY , MODE, PATH" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
    {
        psFile = ColItemData( hItem );

        CmdResponse( hCnx, "FILELISTITEM %s,%s%s,%s",
            psFile->szFileKey,
            psFile->bExport ? ( psFile->bImport ? "EXPORT/IMPORT:" : "EXPORT:" ) : ( psFile->bImport ? "IMPORT:" : "#ERR#" ),
            psFile->bImport ? psFile->szServer : "",
            psFile->szFile );
    }

    CmdResponse( hCnx, "FILELISTEND" );
}


PRIVATE void CmdExecuteFileStatus( ACOM_CNX_HANDLE hCnx, char * pcFileKey )
{
    DWORD dwIndex;
    HCOLLECTIONITEM hItem;
    TRFSVC_FILE * psFile = NULL;
    TRFSVC_SERVER * psServer = NULL;
    TRFSVC_CLIENT * psClient = NULL;

    CmdResponse( hCnx, "FILESTATUSBEGIN RECORD , VALUE" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
    {
        psFile = ColItemData( hItem );
        if ( _stricmp( pcFileKey, psFile->szFileKey ) == 0 )
            break;
        psFile = NULL;
    }

    if ( psFile != NULL )
    {
        CmdResponse( hCnx, "FILESTATUSITEM FILEKEY,%s", psFile->szFileKey );
        CmdResponse( hCnx, "FILESTATUSITEM FILEPATH,%s", psFile->szFile );
        CmdResponse( hCnx, "FILESTATUSITEM FILEIMPORT,%s%s", psFile->bImport ? "YES:" : "NO", psFile->bImport ? psFile->szServer : "" );
        CmdResponse( hCnx, "FILESTATUSITEM FILEEXPORT,%s", psFile->bExport ? "YES:" : "NO" );
        if ( psFile->bExport )
        {
            dwIndex = 0;
            ColLock( gsSvcWork.hColServers );
            hItem = COL_SCAN_BEGIN;
            while ( ColItemScan( gsSvcWork.hColServers, &hItem ) )
            {
                psServer = ColItemData( hItem );
                if ( psServer->psFile == psFile )
                {
                    CmdResponse( hCnx, "FILESTATUSITEM CLIENT[%u],%s", dwIndex, psServer->szClientId );
                    dwIndex ++;
                }
            }
            ColUnlock( gsSvcWork.hColServers );
        }

        if ( psFile->bImport )
        {
            ColLock( gsSvcWork.hColClients );
            hItem = COL_SCAN_BEGIN;
            while ( ColItemScan( gsSvcWork.hColClients, &hItem ) )
            {
                psClient = ColItemData( hItem );
            }
            ColUnlock( gsSvcWork.hColClients );
            CmdResponse( hCnx, "FILESTATUSITEM SERVERCONNECTION,%s", psClient == NULL ? "OFF" : "ON" );
        }
    }

    CmdResponse( hCnx, "FILESTATUSEND" );
}


// --------------------------------------------------------------------

PRIVATE void CmdExecuteClientList( ACOM_CNX_HANDLE hCnx )
{
    HCOLLECTIONITEM hItem;
    TRFSVC_SERVER * psServer = NULL;

    CmdResponse( hCnx, "CLIENTLISTBEGIN FILEKEY,CLIENTNAME" );

    ColLock( gsSvcWork.hColServers );
    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColServers, &hItem ) )
    {
        psServer = ColItemData( hItem );
        CmdResponse( hCnx, "CLIENTLISTITEM %s,%s", psServer->psFile->szFileKey, psServer->szClientId );
    }
    ColUnlock( gsSvcWork.hColServers );

    CmdResponse( hCnx, "CLIENTLISTEND" );
}

// --------------------------------------------------------------------

PRIVATE void CmdExecutePingClient( ACOM_CNX_HANDLE hCnx, char * szName )
{
    DWORD                   dwErr;
    BOOL                    fFound = FALSE;
    HCOLLECTIONITEM         hItem;
    TRFSVC_SERVER         * psServer = NULL;
    FSYNC_MSG_PING_REQ      sPing;
    FSYNC_MSG_PING_REQ    * psPing = &sPing;

    ColLock( gsSvcWork.hColServers );
    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColServers, &hItem ) )
    {
        psServer = ColItemData( hItem );
        if ( _stricmp( psServer->szClientId, szName ) == 0 )
        {
            psPing->dwMessageCode = FSYNC_CODE_PING_REQ;
            psPing->dwCnxId = hCnx;

            dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_PING_REQ( psPing ), psPing );
            if ( dwErr != NO_ERROR )
                CmdResponse( hCnx, "PINGCLIENTERROR %u,Cannot send the ping message to the client", dwErr );
            else
                CmdResponse( hCnx, "PINGCLIENTOK" );

            fFound = TRUE;
        }
    }
    ColUnlock( gsSvcWork.hColServers );

    if ( ! fFound )
        CmdResponse( hCnx, "PINGCLIENTERROR 2,Client not found" );
}


// --------------------------------------------------------------------

PRIVATE void CmdExecutePingServer( ACOM_CNX_HANDLE hCnx, char * szFileKey )
{
    DWORD                   dwErr;
    BOOL                    fFound = FALSE;
    HCOLLECTIONITEM         hItem;
    TRFSVC_CLIENT         * psClient = NULL;
    FSYNC_MSG_PING_REQ      sPing;
    FSYNC_MSG_PING_REQ    * psPing = &sPing;

    ColLock( gsSvcWork.hColClients );
    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColClients, &hItem ) )
    {
        psClient = ColItemData( hItem );
        if ( _stricmp( psClient->psFile->szFileKey, szFileKey ) == 0 )
        {
            psPing->dwMessageCode = FSYNC_CODE_PING_REQ;
            psPing->dwCnxId = hCnx;

            dwErr = AComSendMessage( gsSvcWork.hWks, psClient->hCnx, 0, FSYNC_SIZEOF_PING_REQ( psPing ), psPing );
            if ( dwErr != NO_ERROR )
                CmdResponse( hCnx, "PINGSERVERERROR %u,Cannot send the ping message to the server", dwErr );
            else
                CmdResponse( hCnx, "PINGSERVEROK" );

            fFound = TRUE;
        }
    }
    ColUnlock( gsSvcWork.hColClients );

    if ( ! fFound )
        CmdResponse( hCnx, "PINGCLIENTERROR 2,Server not found" );
}

// --------------------------------------------------------------------

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


PRIVATE void CmdExecuteStatus( ACOM_CNX_HANDLE hCnx )
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

    CmdResponse( hCnx, "STATUSBEGIN FLAG,STATE" );

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
}

#undef SPLIT_TIME
// --------------------------------------------------------------------

// --------------------------------------------------------------------

PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... )
{
    int iLen;
    va_list lParm;      // Pour les paramètres du format
	char szMsg[1000];   // Buffer du message

	// Placer le curseur des paramètres au premier
	va_start( lParm, pcRsp );

    // Remplir le buffer du message avec la chaine formatée
	iLen = _vsnprintf_s(szMsg, sizeof(szMsg), sizeof(szMsg), pcRsp, lParm);
    szMsg[sizeof(szMsg)-1] = '\0';
    iLen = (int)strlen(szMsg);

    AComSendMessage( 
            gsSvcWork.hWks, 
            hCnx, 
            0, 
            (DWORD)iLen, 
            szMsg );
}



// --------------------------------------------------------------------

PRIVATE BOOL CmdCheck(char * pcCmdName, 
					char * pcCmdLine, 
					DWORD dwCmdSize, 
					DWORD dwParams, 
					char ** ppcParams, 
					DWORD dwParameElmentSize)
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
		strcpy_s(ppcParams[dwParamIndex], dwParameElmentSize, szParam);

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


