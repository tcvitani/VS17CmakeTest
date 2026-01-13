/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_cmd.c
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
#include <col.h>
#include <csr_list.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>
#include <routesvc_struct.h>
#include <routesvc_spy.h>
#include <routesvc_lc.h>
#include <routesvc_ls.h>

#define LOC_DEF
#include <routesvc_cmd.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

#define CMD_MAX_SIZE 1024
#define CMD_MAX_PARAMS 4

#define CMD_STATE_HEADER       0
#define CMD_STATE_NO_QUOTE     1
#define CMD_STATE_SINGLE_QUOTE 2
#define CMD_STATE_DOUBLE_QUOTE 3

PRIVATE BOOL CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams );
PRIVATE void CmdResponse(DWORD64 hCnx, char * pcRsp, ...);
PRIVATE char * CmdTrim( char * pcStr );

// --------------- CODE ----------------------




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void CmdResponse( DWORD64 dwCmdIndex, char * pcRsp, ... )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande
 *             pcRsp      : Chaine format de la réponse (façon printf)
 *             ...        : Paramètre du format (façon printf)
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie un message de réponse sur la connexion de commande
 * --------------------------------------------------------------------
 */
PRIVATE void CmdResponse(DWORD64 dwCmdIndex, char * pcRsp, ...)
{
    int iLen;
    va_list lParm;      // Pour les paramètres du format
	char szMsg[1000];   // Buffer du message

	// Placer le curseur des paramètres au premier
	va_start( lParm, pcRsp );

    // Remplir le buffer du message avec la chaine formatée
    iLen = _vsnprintf_s( szMsg, _countof(szMsg), sizeof(szMsg), pcRsp, lParm );
    if ( iLen < 0 ) iLen = sizeof(szMsg);

    // Envoyer la chaine sur la connexion
    AComSendMessage( 
            gsSvcWork.hWks, 
            gsSvcWork.psList[dwCmdIndex].hCnx, 
            0, 
            (DWORD)iLen, 
            szMsg );

    SpySendMessage( dwCmdIndex, "SPYOUT", (DWORD)iLen, szMsg );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteCommandList(DWORD64 hCnx)
{
    CmdResponse( hCnx, "COMMANDLISTBEGIN NAME[,PARAMS]" );
    CmdResponse( hCnx, "COMMANDLISTITEM CLOSESPY,INDEX" );
    CmdResponse( hCnx, "COMMANDLISTITEM COMMANDLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM CONNECTIONLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM DISCONNECT,INDEX" );
    CmdResponse( hCnx, "COMMANDLISTITEM OPENSPY,INDEX" );
    CmdResponse( hCnx, "COMMANDLISTITEM PARAMLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM PROMPT" );
    CmdResponse( hCnx, "COMMANDLISTITEM RELOAD" );
    CmdResponse( hCnx, "COMMANDLISTITEM SAVEPARAMS" );
    CmdResponse( hCnx, "COMMANDLISTITEM SEND,INDEX|MESSAGE" );
    CmdResponse( hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE" );
    CmdResponse( hCnx, "COMMANDLISTITEM SPYCNXOFF" );
    CmdResponse( hCnx, "COMMANDLISTITEM SPYCNXON" );
    CmdResponse( hCnx, "COMMANDLISTITEM SPYLIST" );
    CmdResponse( hCnx, "COMMANDLISTITEM STATUS" );
    CmdResponse( hCnx, "COMMANDLISTITEM STOP" );
    CmdResponse( hCnx, "COMMANDLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecutePrompt(DWORD64 hCnx)
{
    CmdResponse( hCnx, "PROMPTBEGIN" );
    CmdResponse( hCnx, "PROMPTITEM TITLE    : %s", ROUTESVC_PROMPT_TITLE );
    CmdResponse( hCnx, "PROMPTITEM VERSION  : %s", ROUTESVC_PROMPT_VERSION );
    CmdResponse( hCnx, "PROMPTITEM DATE     : %s", ROUTESVC_PROMPT_DATE );
    CmdResponse( hCnx, "PROMPTITEM COMMENTS : %s", ROUTESVC_PROMPT_COMMENTS );
    CmdResponse( hCnx, "PROMPTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteStop(DWORD64 dwCmdIndex)
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdResponse( dwCmdIndex, "STOPOK" );

    // Demander la fin
    NTSVCSignalEnd();
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteReload(DWORD64 dwCmdIndex)
{
    // Renvoyer la réponse.
    // Du fait de l'assynchronisme, il y a des chances que l'application
    // soit arrétée avant l'émission du message.
    CmdResponse( dwCmdIndex, "RELOADOK" );

    gsSvcWork.bReload = TRUE;

    // Demander la fin
    NTSVCSignalEnd();
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteParamList(DWORD64 dwCmdIndex)
{
    DWORD dwScan;
    DWORD dwCount = NTSVCGetParametersCount( gsSvcWork.psParams );
    NTSVC_PARAMETER_DEF * psParam;

    CmdResponse( dwCmdIndex, "PARAMLISTBEGIN NAME,VALUE" );
    for ( dwScan = 0 ; dwScan < dwCount ; dwScan ++ )
    {
        psParam = &gsSvcWork.psParams[dwScan];
        if ( psParam->dwType == REG_SZ )
            CmdResponse( dwCmdIndex, "PARAMLISTITEM %s,\"%s\"", psParam->szName, psParam->pvValue );
        else if ( psParam->dwType == REG_DWORD )
            CmdResponse( dwCmdIndex, "PARAMLISTITEM %s,%u", psParam->szName, *(DWORD*)psParam->pvValue );
        else
            CmdResponse( dwCmdIndex, "PARAMLISTITEM %s,-", psParam->szName );
    }
    CmdResponse( dwCmdIndex, "PARAMLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSetParam(DWORD64 hCnx, char * pcName, char * pcValue)
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
PRIVATE void CmdExecuteSaveParams(DWORD64 dwCmdIndex)
{
    DWORD dwErr;
    DWORD dwPos;

    dwErr = NTSVCSaveParameters( gsSvcWork.psParams, &dwPos );
    if ( dwErr == NO_ERROR )
        CmdResponse( dwCmdIndex, "SAVEPARAMSOK" );
    else
        CmdResponse( dwCmdIndex, "SAVEPARAMSERROR %u,Cannot save parameter %u", dwErr, dwPos );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteConnectionList(DWORD64 dwCmdIndex)
{
    DWORD dwIndex;               // Pour scanner la liste des connexions
    DWORD dwSubIndex;            // Pour scanner la liste des sous-connexion
    DWORD dwCount = 0;           // Compteur de message
    DWORD dwFilter;              // Pour scanner les filtres
    DWORD dwPos;                 // Position caractère courant dans les chaines à générer
    ROUTESVC_CONNECTION * psCnx; // Connexion courante
    char szInfo[256];            // Chaine informative à propos de la connexion
    ROUTESVC_CONNECTION sCnx;    // Copie de la structure connexion courante.

    // Début de la réponse
    CmdResponse( dwCmdIndex, "CONNECTIONLISTBEGIN INDEX,TYPE,INFO" );

    // Scanner l'intégralité des connexions
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwListCount ; dwIndex ++ )
    {
        psCnx = &gsSvcWork.psList[dwIndex];

        // Passer en section critique pour éviter toute modification pendant la consultation
        EnterCriticalSection( &gsSvcWork.sCritical );
        sCnx = *psCnx;
        LeaveCriticalSection( &gsSvcWork.sCritical );

        // Ne traiter que les connexions établies
        if ( sCnx.bBusy )
        {
            // Il s'agit d'une voie
            if ( ( sCnx.dwType == ROUTESVC_PIPE_LC_TYPE ) && ! sCnx.uCnx.sLC.fQualified )
            {
                CmdResponse( 
                        dwCmdIndex, 
                        "CONNECTIONLISTITEM %u,XLC,-",
                        dwIndex );
            }
            else if ( sCnx.dwType == ROUTESVC_PIPE_LC_TYPE )
            {
                for ( dwSubIndex = 0 ; dwSubIndex < sCnx.uCnx.sLC.dwCount ; dwSubIndex ++ )
                {
                    sprintf_s( szInfo, sizeof(szInfo), "Plaza=%u Lane=%u", sCnx.uCnx.sLC.tdwPlaza[dwSubIndex], sCnx.uCnx.sLC.tdwLane[dwSubIndex] );
                    if ( sCnx.uCnx.sLC.fMultiple )
                        CmdResponse( 
                                dwCmdIndex, 
                                "CONNECTIONLISTITEM %u/%u,LC,%s",
                                dwIndex, dwSubIndex,
                                szInfo );
                    else
                        CmdResponse( 
                                dwCmdIndex, 
                                "CONNECTIONLISTITEM %u,LC,%s",
                                dwIndex,
                                szInfo );
                }
            }

            // Il s'agit d'un client LS
            else if ( sCnx.dwType == ROUTESVC_PIPE_LS_TYPE )
            {
                dwPos = sprintf_s( 
                        szInfo, 
						sizeof(szInfo),
                        "%u FILTERS%s", 
                        sCnx.uCnx.sLS.dwFilterCount,
                        ( sCnx.uCnx.sLS.dwFilterCount != 0 ) ? " : " : "" );
                for ( dwFilter = 0 ; dwFilter < sCnx.uCnx.sLS.dwFilterCount ; dwFilter ++ )
                {
                    if ( dwFilter != 0 )
                        dwPos += sprintf_s( 
                                szInfo + dwPos,
								sizeof(szInfo) - dwPos,
                                " / " );
                    dwPos += sprintf_s( 
                                szInfo + dwPos,
								sizeof(szInfo) - dwPos,
                                "ID%u-CD%u",
                                sCnx.uCnx.sLS.psFilterList[dwFilter].dwID,
                                sCnx.uCnx.sLS.psFilterList[dwFilter].dwCD );
                    if ( dwPos > ( sizeof( szInfo ) - 35 ) )
                    {
                        dwPos += sprintf_s( 
                                    szInfo + dwPos, sizeof(szInfo) - dwPos, "..." );
                        break;
                    }
                }

                CmdResponse( 
                        dwCmdIndex, 
                        "CONNECTIONLISTITEM %u,LS,%s",
                        dwIndex,
                        szInfo );
            }

            // Il s'agit d'une client de contrôle
            else
            {
                if ( dwCmdIndex == dwIndex )
                    strcpy_s( szInfo, sizeof(szInfo), "Current connexion" );
                else
                    strcpy_s( szInfo, sizeof(szInfo), "-" );
                CmdResponse( 
                        dwCmdIndex, 
                        "CONNECTIONLISTITEM %u,CMD,%s",
                        dwIndex,
                        szInfo );
            }
        }
    }

    // Fin de la réponse
    CmdResponse( dwCmdIndex, "CONNECTIONLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteDisconnect(DWORD64 dwCmdIndex, char * pcIndex)
{
    DWORD dwIndex = atol( pcIndex ); // Index de la connexion à déconnecter
    ROUTESVC_CONNECTION * psCnx;     // Connexion courante
    BOOL bSend;                     
    ACOM_CNX_HANDLE hCnx;

    // Vérifier la cohérence de l'index
    if ( dwIndex < gsSvcWork.dwListCount )
    {
        psCnx = &gsSvcWork.psList[dwIndex];

        // Passer en section critique pour éviter toute modification pendant la consultation
        EnterCriticalSection( &gsSvcWork.sCritical );
        
        // Tester l'état de la connexion
        bSend = psCnx->bBusy;
        hCnx = gsSvcWork.psList[dwIndex].hCnx;

        // Sortir de section critique, fin de la protection de l'enregistrement
        LeaveCriticalSection( &gsSvcWork.sCritical );

        if ( bSend )
        {
            // On est bien connecté
            // Demander la déconnexion
            AComDisconnectPeer( 
                    gsSvcWork.hWks, 
                    hCnx, 
                    FALSE );
            CmdResponse( dwCmdIndex, "DISCONNECTOK" );
        }
        else
        {
            CmdResponse( 
                    dwCmdIndex, 
                    "DISCONNECTERROR %u,This connection is not opened", 
                    ERROR_PIPE_NOT_CONNECTED );
        }
    }
    else
        CmdResponse( 
                dwCmdIndex, 
                "DISCONNECTERROR %u,Invalid connection number", 
                ERROR_PIPE_NOT_CONNECTED );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteOpenSpy(DWORD64 dwCmdIndex, char * pcIndex)
{
    DWORD dwIndex;     // Index de la connexion à déconnecter
    DWORD dwErr;       // Récupération du code d'erreur

    // Vérifier la cohérence de l'index
    
    dwIndex = atol( pcIndex );
    if ( dwIndex < gsSvcWork.dwListCount )
    {
        dwErr = SpyAdd( dwCmdIndex, dwIndex );

        if ( dwErr == NO_ERROR )
        {
            CmdResponse( dwCmdIndex, "OPENSPYOK" );
        }
        else
        {
            CmdResponse( 
                    dwCmdIndex, 
                    "OPENSPYERROR %u,An error occured while updating the spy list", 
                    ERROR_PIPE_NOT_CONNECTED );
        }
    }
    else
        CmdResponse( 
                dwCmdIndex, 
                "OPENSPYERROR %u,Invalid connection number", 
                ERROR_PIPE_NOT_CONNECTED );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteCloseSpy(DWORD64 dwCmdIndex, char * pcIndex)
{
    DWORD dwIndex = atol( pcIndex ); // Index de la connexion à déconnecter
    DWORD dwErr;                     // Récupération du code d'erreur

    // Vérifier la cohérence de l'index

    if ( dwIndex < gsSvcWork.dwListCount )
    {
        dwErr = SpyRemove( dwCmdIndex, dwIndex );

        if ( dwErr == NO_ERROR )
        {
            CmdResponse( dwCmdIndex, "CLOSESPYOK" );
        }
        else
        {
            CmdResponse( 
                    dwCmdIndex, 
                    "CLOSESPYERROR %u,An error occured while updating the spy list", 
                    ERROR_PIPE_NOT_CONNECTED );
        }
    }
    else
        CmdResponse( 
                dwCmdIndex, 
                "CLOSESPYERROR %u,Invalid connection number", 
                ERROR_PIPE_NOT_CONNECTED );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSpyList(DWORD64 dwCmdIndex)
{
    DWORD dwScan;
    CmdResponse( dwCmdIndex, "SPYLISTBEGIN INDEX" );
    for ( dwScan = 0 ; dwScan < gsSvcWork.psList[dwCmdIndex].uCnx.sCmd.dwSpyCount ; dwScan ++ )
        CmdResponse( dwCmdIndex, "SPYLISTITEM %u", gsSvcWork.psList[dwCmdIndex].uCnx.sCmd.pdwSpyList[dwScan] );
    CmdResponse( dwCmdIndex, "SPYLISTEND" );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSpyCnxOn(DWORD64 dwCmdIndex)
{
    DWORD dwErr = SpyCnxAdd( dwCmdIndex, TRUE );
    if ( dwErr == NO_ERROR )
        CmdResponse( dwCmdIndex, "SPYCNXONOK" );
    else
        CmdResponse( dwCmdIndex, "SPYCNXONERROR %u,Cannot activate connection spy", dwErr );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSpyCnxOff(DWORD64 dwCmdIndex)
{
    DWORD dwErr = SpyCnxAdd( dwCmdIndex, FALSE );
    if ( dwErr == NO_ERROR )
        CmdResponse( dwCmdIndex, "SPYCNXOFFOK" );
    else
        CmdResponse( dwCmdIndex, "SPYCNXOFFERROR %u,Cannot desactivate connection spy", dwErr );
}

// --------------------------------------------------------------------
PRIVATE void CmdExecuteSend(DWORD64 dwCmdIndex, char * pcIndex, char * pcMsg)
{
    DWORD dwIndex = atol( pcIndex );
    BOOL bSend;
    ACOM_CNX_HANDLE hCnx;
    DWORD dwLen;

    if ( dwIndex < gsSvcWork.dwListCount )
    {
        EnterCriticalSection( &gsSvcWork.sCritical );
        bSend = gsSvcWork.psList[dwIndex].bBusy;
        hCnx =  gsSvcWork.psList[dwIndex].hCnx;
        LeaveCriticalSection( &gsSvcWork.sCritical );

        if ( bSend )
        {
            dwLen = (DWORD)strlen( pcMsg );
            
            AComSendMessage( gsSvcWork.hWks, hCnx, 0, dwLen, pcMsg );

            if ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LS_TYPE )
                LSTraceMsg( FALSE, dwIndex, dwLen, pcMsg );
            else if ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LC_TYPE )
                LCTraceMsg( FALSE, dwIndex, dwLen, pcMsg );
            
            SpySendMessage( dwIndex, "SPYOUT", dwLen, pcMsg );
            CmdResponse( dwCmdIndex, "SENDOK" );
        }
        else
            CmdResponse( dwCmdIndex, "SENDERROR %u,Not connected",  ERROR_PIPE_NOT_CONNECTED );
    }
    else
        CmdResponse( 
                dwCmdIndex, "SENDERROR %u,Invalid connection number", ERROR_PIPE_NOT_CONNECTED );
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


PRIVATE void CmdExecuteStatus(DWORD64 dwCmdIndex)
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

    CmdResponse( dwCmdIndex, "STATUSBEGIN FLAG,STATE" );

    CmdResponse( dwCmdIndex, "STATUSITEM RUNNINGMODE,%s", 
                        gsSvcWork.bIsDebug ? "Debug" : "Standard" );
    
    dwErr = AComGetWorkersStatistics( gsSvcWork.hWks, &dwThreads, &ullIdle, &ullUser, &ullKernel );
    if ( dwErr == NO_ERROR )
    {
        CmdResponse( dwCmdIndex, "STATUSITEM COMMAXCONCURRENTWORKERS,%u",  dwThreads );

        SPLIT_TIME(ullIdle)
        CmdResponse( dwCmdIndex, "STATUSITEM COMIDLETIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullUser)
        CmdResponse( dwCmdIndex, "STATUSITEM COMUSERTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );

        SPLIT_TIME(ullKernel)
        CmdResponse( dwCmdIndex, "STATUSITEM COMKERNELTIME,%02u:%02u:%02u.%03u", dwHours, dwMinutes, dwSeconds, dwMilli );
    }
    else
    {
        CmdResponse( dwCmdIndex, "STATUSITEM MAXCONCURRENTWORKERS,#ERR#" );
        CmdResponse( dwCmdIndex, "STATUSITEM IDLETIME,#ERR#" );
        CmdResponse( dwCmdIndex, "STATUSITEM USERTIME,#ERR#" );
        CmdResponse( dwCmdIndex, "STATUSITEM KERNELTIME,#ERR#" );
    }

    CmdResponse( dwCmdIndex, "STATUSITEM LICENCE,%s", gsSvcWork.fLicenceIsValid ? "VALID" : "INVALID" );

    CmdResponse( dwCmdIndex, "STATUSEND" );
}

#undef SPLIT_TIME
// --------------------------------------------------------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CmdReceive( DWORD64 dwCmdIndex, DWORD dwMsgSize, char * pcMsg )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande
 *             dwMsgSize  : Taille du message de commande
 *             pcMsg      : Message de commande
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite une commande reçue
 * --------------------------------------------------------------------
 */
PROTECTED void CmdReceive(DWORD64 dwCmdIndex, DWORD dwMsgSize, char * pcMsg)
{
    DWORD dwIndex;
    char * tpcParams[CMD_MAX_PARAMS];
    char tszParams[CMD_MAX_PARAMS][CMD_MAX_SIZE];

    for ( dwIndex = 0 ; dwIndex < CMD_MAX_PARAMS ; dwIndex ++ )
        tpcParams[dwIndex] = tszParams[dwIndex];

    NTSVCInfo( "CmdReceive(), réception commande sur dwIndex=%u", dwCmdIndex );
 
    if      ( CmdCheck( "COMMANDLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteCommandList( dwCmdIndex );

    else if ( CmdCheck( "PROMPT", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecutePrompt( dwCmdIndex );

    else if ( CmdCheck( "STOP", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteStop( dwCmdIndex );

    else if ( CmdCheck( "RELOAD", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteReload( dwCmdIndex );

    else if ( CmdCheck( "PARAMLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteParamList( dwCmdIndex );

    else if ( CmdCheck( "SETPARAM", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteSetParam( dwCmdIndex, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "SAVEPARAMS", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteSaveParams( dwCmdIndex );

    else if ( CmdCheck( "CONNECTIONLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteConnectionList( dwCmdIndex );

    else if ( CmdCheck( "DISCONNECT", pcMsg , dwMsgSize, 1, tpcParams ) )
        CmdExecuteDisconnect( dwCmdIndex, tszParams[0] );

    else if ( CmdCheck( "OPENSPY", pcMsg , dwMsgSize, 1, tpcParams ) )
        CmdExecuteOpenSpy( dwCmdIndex, tszParams[0] );

    else if ( CmdCheck( "CLOSESPY", pcMsg , dwMsgSize, 1, tpcParams ) )
        CmdExecuteCloseSpy( dwCmdIndex, tszParams[0] );

    else if ( CmdCheck( "SPYLIST", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteSpyList( dwCmdIndex );

    else if ( CmdCheck( "SPYCNXON", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteSpyCnxOn( dwCmdIndex );

    else if ( CmdCheck( "SPYCNXOFF", pcMsg , dwMsgSize, 0, tpcParams ) )
        CmdExecuteSpyCnxOff( dwCmdIndex );

    else if ( CmdCheck( "SEND", pcMsg, dwMsgSize, 2, tpcParams ) )
        CmdExecuteSend( dwCmdIndex, tszParams[0], tszParams[1] );

    else if ( CmdCheck( "STATUS", pcMsg, dwMsgSize, 0, tpcParams ) )
        CmdExecuteStatus( dwCmdIndex );

    else 
        CmdResponse( 
                dwCmdIndex, 
                "COMMANDERROR %u,Unknown command", 
                ERROR_INVALID_DATA );
}





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


