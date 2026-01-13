/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : authsvc_vt
 * FILE       : authsvc_vt_cmd.c
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
#include <dbif.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>
#include <csr_msg.h>

#include <authsvc_vt_glob.h>
#include <authsvc_vt_text.h>
#include <authsvc_vt_db.h>

#define LOC_DEF
#include <authsvc_vt_cmd.h>
#undef LOC_DEF

#include <memclass.h>

#define CMD_MAX_SIZE 1024
#define CMD_MAX_PARAMS 4


#define CMD_STATE_HEADER       0
#define CMD_STATE_NO_QUOTE     1
#define CMD_STATE_SINGLE_QUOTE 2
#define CMD_STATE_DOUBLE_QUOTE 3


PRIVATE BOOL CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams );
PRIVATE void CmdResponse( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... );
PRIVATE char * CmdTrim( char * pcStr );

// --------------- CODE ----------------------




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
    iLen = _vsnprintf_s( szMsg, 1000, sizeof(szMsg), pcRsp, lParm );
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
    CmdResponse( hCnx, "COMMANDLISTITEM PROMPT" );
    CmdResponse( hCnx, "COMMANDLISTITEM STOP" );
    CmdResponse( hCnx, "COMMANDLISTITEM RELOAD" );
    CmdResponse( hCnx, "COMMANDLISTITEM PARAMLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE" );
    CmdResponse( hCnx, "COMMANDLISTITEM SAVEPARAMS" );
    CmdResponse( hCnx, "COMMANDLISTITEM RUNTREATMENT" );
    CmdResponse( hCnx, "COMMANDLISTITEM STATUS" );
    CmdResponse( hCnx, "COMMANDLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecutePrompt( ACOM_CNX_HANDLE hCnx )
{
    CmdResponse( hCnx, "PROMPTBEGIN" );
    CmdResponse( hCnx, "PROMPTITEM TITLE    : %s", AUTHSVC_PROMPT_TITLE );
    CmdResponse( hCnx, "PROMPTITEM VERSION  : %s", AUTHSVC_PROMPT_VERSION );
    CmdResponse( hCnx, "PROMPTITEM DATE     : %s", AUTHSVC_PROMPT_DATE );
    CmdResponse( hCnx, "PROMPTITEM COMMENTS : %s", AUTHSVC_PROMPT_COMMENTS );
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
PRIVATE void CmdExecuteRunTreatment( ACOM_CNX_HANDLE hCnx )
{
    DWORD dwErr;

    if ( ! SetEvent( gsSvcWork.hEvent ) )
    {
        dwErr = GetLastError();
        CmdResponse( hCnx, "RUNTREATMENTERROR %u,Cannot trigger treatment", dwErr );
    }
    else
        CmdResponse( hCnx, "RUNTREATMENTOK" );
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
	CmdResponse(hCnx, "STATUSITEM DATABASE,%s", DBGetIsDatabaseMissing() ? "Offline" : "Online");
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
 
    if      ( CmdCheck( "PROMPT", pcMsg, dwMsgSize, 0, tpcParams ) )
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

    else if ( CmdCheck( "RUNTREATMENT", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteRunTreatment( hCnx );

    else if ( CmdCheck( "COMMANDLIST", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteCommandList( hCnx );

    else if ( CmdCheck( "STATUS", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteStatus( hCnx );

    else 
        CmdResponse( 
                hCnx, 
                "COMMANDERROR %u,Unknown command", 
                ERROR_INVALID_DATA );
}





PRIVATE BOOL CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams )
{
    DWORD dwMsgSize = dwCmdSize;
    DWORD dwLen = (DWORD) strlen( pcCmdName );
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

