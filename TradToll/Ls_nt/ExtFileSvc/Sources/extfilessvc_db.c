/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : extfilessvc
 * FILE       : extfilessvc_db.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module d'interface avec la base de données
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

#include <extfilessvc_glob.h>
#include <extfilessvc_text.h>

#define LOC_DEF
#include <extfilessvc_db.h>
#undef LOC_DEF

#include <memclass.h>




#define DB_MAX_LINE_SIZE    4096
#define DB_MAX_ERR_SIZE     50


#define DB_DEFAULT_LIST_BEGIN  "DECLARE Result NUMBER(3); " \
                               "BEGIN   Result := IMP_FICHIERS.IMPORT_LIST_BEGIN( :BndType, :BndLine ); " \
                               "        IF Result <> 0 THEN " \
                               "            SELECT ID_LN_ERROR, LONG_LN_ERROR_MSG " \
                               "                   INTO :BndRet, :BndText " \
                               "                   FROM V_LN_ERROR_MSG " \
                               "                   WHERE ID_LN_ERROR = Result; " \
                               "        ELSE " \
                               "             :BndRet := 0; " \
                               "             :BndText := ''; " \
                               "        END IF; " \
                               "END;"

#define DB_DEFAULT_LIST_ITEM   "DECLARE Result NUMBER(3); " \
                               "BEGIN   Result := IMP_FICHIERS.IMPORT_LIST_ITEM( :BndType, :BndLine ); " \
                               "        IF Result <> 0 THEN " \
                               "            SELECT ID_LN_ERROR, LONG_LN_ERROR_MSG " \
                               "                   INTO :BndRet, :BndText " \
                               "                   FROM V_LN_ERROR_MSG " \
                               "                   WHERE ID_LN_ERROR = Result; " \
                               "        ELSE " \
                               "             :BndRet := 0; " \
                               "             :BndText := ''; " \
                               "        END IF; " \
                               "END;"

#define DB_DEFAULT_LIST_END    "DECLARE Result NUMBER(3); " \
                               "BEGIN   Result := IMP_FICHIERS.IMPORT_LIST_END( :BndType ); " \
                               "        IF Result <> 0 THEN " \
                               "            SELECT ID_LN_ERROR, LONG_LN_ERROR_MSG " \
                               "                   INTO :BndRet, :BndText " \
                               "                   FROM V_LN_ERROR_MSG " \
                               "                   WHERE ID_LN_ERROR = Result; " \
                               "        ELSE " \
                               "             :BndRet := 0; " \
                               "             :BndText := ''; " \
                               "        END IF; " \
                               "END;"

#define DB_DEFAULT_EXTLIST_NEW "BEGIN :BndRet := IMP_FICHIERS.EXTERNAL_LIST_NEW( :BndType, :BndFile ); " \
                               "END;"


#define DB_DEFAULT_EXTLIST_ACK "BEGIN :BndRet := IMP_FICHIERS.EXTERNAL_LIST_ACK( :BndType, :BndFile ); " \
                               "END;"





PRIVATE char gszDbListBegin[8000] = "";
PRIVATE char gszDbListItem[8000]  = "";
PRIVATE char gszDbListEnd[8000]   = "";
PRIVATE char gszDbExtListNew[8000]   = "";
PRIVATE char gszDbExtListAck[8000]   = "";




PROTECTED DWORD DBInitRequests()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwLen;

#define LOAD_REQ(var,def) \
        dwLen = sizeof(gszDb##var); \
        dwErr = REG_Defaut_Chaine( \
            EXTFILESSVC_REG_ROOT, \
            EXTFILESSVC_REG_KEY_REQ, \
            #var, \
            gszDb##var, \
            &dwLen, \
            def ); \
        if ( dwErr != NO_ERROR ) \
            return dwErr;

    LOAD_REQ( ListBegin  , DB_DEFAULT_LIST_BEGIN  )
    LOAD_REQ( ListItem   , DB_DEFAULT_LIST_ITEM   )
    LOAD_REQ( ListEnd    , DB_DEFAULT_LIST_END    )
    LOAD_REQ( ExtListNew , DB_DEFAULT_EXTLIST_NEW )
    LOAD_REQ( ExtListAck , DB_DEFAULT_EXTLIST_ACK )

    return dwErr;
}




PRIVATE void WINAPI DBErrFile( HANDLE * phFile, char * szFile, char * szErrFile, DWORD dwLine, DWORD dwErr, char * szText, HANDLE hLog )
{
    BOOL   fCreated = FALSE;
    DWORD  dwWritten;
    DWORD  dwLen;
    HANDLE hFile = *phFile;
    DWORD  dwErrLine;
    char * pcErrLine = NULL;
    char   szWork[2048] = "";
    char   szErrPathTmp[EXTFILESSVC_MAXPATH+2];
    SYSTEMTIME sTime;
    
    GetLocalTime( &sTime );

    if ( hFile == NULL )
    {
        if ( ! DBFormatText( gsSvcWork.sLogStr.szProcessErrorInt, szWork, sizeof(szWork), szText, szFile, NULL ) )
        {
            _snprintf_s( szWork, sizeof(szWork), sizeof(szWork), "%s\r\n  [1] : %s\r\n  [2] : %s", gsSvcWork.sLogStr.szProcessErrorInt, szText, szFile );
            szWork[sizeof(szWork)-1] = 0;
        }
        EvtLogReportText( hLog,
                          EVTLOG_ERROR,
                          0,
                          dwErr,
                          "%s", szWork );
    }

    if ( szErrFile == NULL )
    {
        *phFile = INVALID_HANDLE_VALUE;
        return;
    }
    else if ( szErrFile[0] == 0 )
    {
        *phFile = INVALID_HANDLE_VALUE;
        return;
    }

    _snprintf_s( szErrPathTmp, sizeof(szErrPathTmp), sizeof(szErrPathTmp), "%s.~", szErrFile );
    szErrPathTmp[sizeof(szErrPathTmp)-1] = 0;

    if ( hFile == NULL )
    {
        hFile = CreateFile( szErrPathTmp, 
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

        fCreated = TRUE;
    }

    if ( hFile != INVALID_HANDLE_VALUE )
    {

        if ( gsSvcWork.sParmWork.dwErrorMaxHead < 20 )
            gsSvcWork.sParmWork.dwErrorMaxHead = 20;
        if ( gsSvcWork.sParmWork.dwErrorMaxLine < 20 )
            gsSvcWork.sParmWork.dwErrorMaxLine = 20;
        if ( gsSvcWork.sParmWork.dwErrorMaxLine > gsSvcWork.sParmWork.dwErrorMaxHead )
            dwErrLine = gsSvcWork.sParmWork.dwErrorMaxLine;
        else
            dwErrLine = gsSvcWork.sParmWork.dwErrorMaxHead;

        pcErrLine = HeapAlloc( GetProcessHeap(), 0, dwErrLine + 3 );
        if ( pcErrLine == NULL )
        {
            CloseHandle( hFile );
            hFile = INVALID_HANDLE_VALUE;
        }
    }   

    if ( hFile != INVALID_HANDLE_VALUE )
    {
        if ( fCreated )
        {
            _snprintf_s( pcErrLine, gsSvcWork.sParmWork.dwErrorMaxHead, gsSvcWork.sParmWork.dwErrorMaxHead, "%02u/%02u/%04u - %02u:%02u:%02u : %s",
                       (DWORD)sTime.wDay,
                       (DWORD)sTime.wMonth,
                       (DWORD)sTime.wYear,
                       (DWORD)sTime.wHour,
                       (DWORD)sTime.wMinute,
                       (DWORD)sTime.wSecond,
                       szWork );

            DBReplaceText( pcErrLine, gsSvcWork.sParmWork.dwErrorMaxHead, "\r", " -" );
            DBReplaceText( pcErrLine, gsSvcWork.sParmWork.dwErrorMaxHead, "\n", " " );
            pcErrLine[gsSvcWork.sParmWork.dwErrorMaxHead] = 0;
            dwLen = (DWORD)strlen( pcErrLine );
            pcErrLine[dwLen++] = 0x0D;
            pcErrLine[dwLen++] = 0x0A;
            pcErrLine[dwLen] = 0;

            if ( ! WriteFile( hFile, pcErrLine, dwLen, &dwWritten, NULL ) )
            {
                CloseHandle( hFile );
                hFile = INVALID_HANDLE_VALUE;
            }
        }

        _snprintf_s( pcErrLine, gsSvcWork.sParmWork.dwErrorMaxLine, gsSvcWork.sParmWork.dwErrorMaxLine, "%07u|%03u|%s", dwLine, dwErr, szText );
        pcErrLine[gsSvcWork.sParmWork.dwErrorMaxLine] = 0;
        dwLen = (DWORD)strlen( pcErrLine );
        pcErrLine[dwLen++] = 0x0D;
        pcErrLine[dwLen++] = 0x0A;
        pcErrLine[dwLen] = 0;

        if ( ! WriteFile( hFile, pcErrLine, dwLen, &dwWritten, NULL ) )
        {
            CloseHandle( hFile );
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    if ( pcErrLine != NULL )
        HeapFree( GetProcessHeap(), 0, pcErrLine );

    *phFile = hFile;
}


PRIVATE void WINAPI DBErrFileEnd( HANDLE * phFile, char * szErrFile )
{
    char   szErrPathTmp[EXTFILESSVC_MAXPATH+2];

    if ( ( (*phFile) != NULL ) && ( (*phFile) != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( *phFile );

        _snprintf_s( szErrPathTmp, sizeof(szErrPathTmp), sizeof(szErrPathTmp), "%s.~", szErrFile );
        szErrPathTmp[sizeof(szErrPathTmp)-1] = 0;

        MoveFileEx( szErrPathTmp, szErrFile, MOVEFILE_REPLACE_EXISTING );
    }
}



PRIVATE DWORD DBParseLine( char * pcSrc, DWORD dwSrcStart, DWORD dwSrcSize, char * pcDst, DWORD dwDstSize )
{
    char cCurrent;
    DWORD dwSrcPos;
    DWORD dwDstPos;

    for ( dwDstPos = 0, dwSrcPos = dwSrcStart ; dwSrcPos < dwSrcSize ; dwSrcPos ++ )
    {
        cCurrent = pcSrc[dwSrcPos];

        // Si caractère fin de chaine, le remplacer par un espace
        if ( cCurrent == '\0' ) 
            cCurrent = ' ';

        // Si caractère fin de fichier, aller directement à la fin et terminer
        else if ( cCurrent == (char)26 ) 
        {
            dwSrcPos = dwSrcSize;
            break;
        }

        // Si caractère retour chariot ou saut de ligne, aller en début de ligne suivante
        else if ( ( cCurrent == '\n' ) || ( cCurrent == '\r' ) )
        {
            dwSrcPos ++;
            while ( dwSrcPos < dwSrcSize )
            {
                cCurrent = pcSrc[dwSrcPos];
                if ( ( cCurrent != '\n' ) && ( cCurrent != '\r' ) )
                    break;
                dwSrcPos ++;
            }
            break;
        }

        // Ignorer les caractères qui dépassent la taille du buffer de sortie
        if ( dwDstPos < ( dwDstSize - 1 ) )
        {
            pcDst[dwDstPos] = cCurrent;
            dwDstPos ++;
        }
    }

    // Ajouter la marque de fin de chaine
    pcDst[dwDstPos] = '\0';
    
    return dwSrcPos;
}


PRIVATE BOOL DBImportListBegin( DB_CNX ** phCnx, char * pcType, char * pcLine, DWORD * pdwErr, char * pcErr )
{
    DWORD dwErr;
    DB_STMT * hStmt;
    DB_VAR * hBndRet = NULL;    DWORD dwBndRet;
    DB_VAR * hBndType = NULL;
    DB_VAR * hBndLine = NULL;
    DB_VAR * hBndText = NULL;
    char szErr[1024];

    NTSVCInfo( "DBImportListBegin(), début import fichier" );

    hStmt = DBOpenStatement( *phCnx, gszDbListBegin );
    if ( hStmt == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_OPEN", "LISTBEGIN", szErr );
        return FALSE;
    }
        
    /* Pour l'alignement */ hBndRet  = DBBindPlaceHolderVariable( hStmt, ":BndRet" , DB_TYPE_INT, sizeof( dwBndRet ) , 0 );
    if ( hBndRet  != NULL ) hBndType = DBBindPlaceHolderVariable( hStmt, ":BndType", DB_TYPE_STR, (DWORD)strlen( pcType ) + 1, 0 );
    if ( hBndType != NULL ) hBndLine = DBBindPlaceHolderVariable( hStmt, ":BndLine", DB_TYPE_STR, (DWORD)strlen( pcLine ) + 1, 0 );
    if ( hBndLine != NULL ) hBndText = DBBindPlaceHolderVariable( hStmt, ":BndText", DB_TYPE_STR, DB_MAX_ERR_SIZE + 1, 0 );
    if ( hBndText == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_BIND", "LISTBEGIN", szErr );
        return FALSE;
    }

    if ( ( DBSetVariableItemValue( hBndType, 0, pcType, (DWORD)strlen(pcType) + 1 ) != pcType ) ||
         ( DBSetVariableItemValue( hBndLine, 0, pcLine, (DWORD)strlen(pcLine) + 1 ) != pcLine ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_SET", "LISTBEGIN", szErr );
        return FALSE;
    }

    if ( ! DBExecuteStatement( hStmt ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_EXEC", "LISTBEGIN", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndRet, 0, &dwBndRet, NULL ) != &dwBndRet )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTBEGIN", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndText, 0, pcErr, NULL ) != pcErr )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTBEGIN", szErr );
        return FALSE;
    }
    pcErr[DB_MAX_ERR_SIZE] = 0;

    DBCloseStatement( hStmt );

    if ( ( dwBndRet != 0 ) && ( gsSvcWork.sParmWork.szErrDir[0] == 0 ) )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR1( dwErr, "ERR_DB_RESULT", "LISTBEGIN" );
        return FALSE;
    }

    if ( dwBndRet != 0 )
        NTSVCInfo( "DBImportListBegin(), retour DB = %u", dwBndRet );
    *pdwErr = dwBndRet;

    return TRUE;
}


PRIVATE BOOL DBImportListItem( DB_CNX ** phCnx, char * pcType, char * pcLine, DWORD * pdwErr, char * pcErr )
{
    DWORD dwErr;
    DB_STMT * hStmt;
    DB_VAR * hBndRet = NULL;    DWORD dwBndRet;
    DB_VAR * hBndType = NULL;
    DB_VAR * hBndLine = NULL;
    DB_VAR * hBndText = NULL;
    char szErr[1024];

    hStmt = DBOpenStatement( *phCnx, gszDbListItem );
    if ( hStmt == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_OPEN", "LISTITEM", szErr );
        return FALSE;
    }
        
    /* Pour l'alignement */ hBndRet  = DBBindPlaceHolderVariable( hStmt, ":BndRet" , DB_TYPE_INT, sizeof( dwBndRet ) , 0 );
    if ( hBndRet  != NULL ) hBndType = DBBindPlaceHolderVariable( hStmt, ":BndType", DB_TYPE_STR, (DWORD)strlen( pcType ) + 1, 0 );
    if ( hBndType != NULL ) hBndLine = DBBindPlaceHolderVariable( hStmt, ":BndLine", DB_TYPE_STR, (DWORD)strlen( pcLine ) + 1, 0 );
    if ( hBndLine != NULL ) hBndText = DBBindPlaceHolderVariable( hStmt, ":BndText", DB_TYPE_STR, DB_MAX_ERR_SIZE + 1, 0 );
    if ( hBndText == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_BIND", "LISTITEM", szErr );
        return FALSE;
    }

    if ( ( DBSetVariableItemValue( hBndType, 0, pcType, (DWORD)strlen(pcType) + 1 ) != pcType ) ||
         ( DBSetVariableItemValue( hBndLine, 0, pcLine, (DWORD)strlen(pcLine) + 1 ) != pcLine ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_SET", "LISTITEM", szErr );
        return FALSE;
    }

    if ( ! DBExecuteStatement( hStmt ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_EXECUTE", "LISTITEM", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndRet, 0, &dwBndRet, NULL ) != &dwBndRet )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTITEM", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndText, 0, pcErr, NULL ) != pcErr )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTITEM", szErr );
        return FALSE;
    }
    pcErr[DB_MAX_ERR_SIZE] = 0;

    DBCloseStatement( hStmt );

    if ( ( dwBndRet != 0 ) && ( gsSvcWork.sParmWork.szErrDir[0] == 0 ) )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR1( dwErr, "ERR_DB_RESULT", "LISTITEM" );
        return FALSE;
    }

    if ( dwBndRet != 0 )
        NTSVCInfo( "DBImportListItem(), retour DB = %u", dwBndRet );

    *pdwErr = dwBndRet;

    return TRUE;
}

PRIVATE BOOL DBImportListEnd( DB_CNX ** phCnx, char * pcType, DWORD * pdwErr, char * pcErr )
{
    DWORD dwErr;
    DB_STMT * hStmt;
    DB_VAR * hBndRet = NULL;    DWORD dwBndRet;
    DB_VAR * hBndType = NULL;
    DB_VAR * hBndText = NULL;
    char szErr[1024];

    NTSVCInfo( "DBImportListEnd(), fin import fichier" );

    hStmt = DBOpenStatement( *phCnx, gszDbListEnd );
    if ( hStmt == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_OPEN", "LISTEND", szErr );
        return FALSE;
    }
        
    /* Pour l'alignement */ hBndRet  = DBBindPlaceHolderVariable( hStmt, ":BndRet" , DB_TYPE_INT, sizeof( dwBndRet ) , 0 );
    if ( hBndRet  != NULL ) hBndType = DBBindPlaceHolderVariable( hStmt, ":BndType", DB_TYPE_STR, (DWORD)strlen( pcType ) + 1, 0 );
    if ( hBndType != NULL ) hBndText = DBBindPlaceHolderVariable( hStmt, ":BndText", DB_TYPE_STR, DB_MAX_ERR_SIZE + 1, 0 );
    if ( hBndText == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_BIND", "LISTEND", szErr );
        return FALSE;
    }

    if ( DBSetVariableItemValue( hBndType, 0, pcType, (DWORD)strlen(pcType) + 1 ) != pcType )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_SET", "LISTEND", szErr );
        return FALSE;
    }

    if ( ! DBExecuteStatement( hStmt ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_EXEC", "LISTEND", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndRet, 0, &dwBndRet, NULL ) != &dwBndRet )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTEND", szErr );
        return FALSE;
    }

    if ( DBGetVariableItemValue( hBndText, 0, pcErr, NULL ) != pcErr )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        DBCloseStatement( hStmt );
        SVC_ERR2( dwErr, "ERR_DB_GET", "LISTEND", szErr );
        return FALSE;
    }
    pcErr[DB_MAX_ERR_SIZE] = 0;

    DBCloseStatement( hStmt );
    if ( ( dwBndRet != 0 ) && ( gsSvcWork.sParmWork.szErrDir[0] == 0 ) )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR1( dwErr, "ERR_DB_RESULT", "LISTEND" );
        return FALSE;
    }

    if ( dwBndRet != 0 )
        NTSVCInfo( "DBImportListEnd(), retour DB = %u", dwBndRet );

    *pdwErr = dwBndRet;

    return TRUE;
}


PROTECTED DWORD DBImportList( DB_CNX ** phCnx, char * pcType, char * pcBuffer, DWORD dwBufferSize,
                              char * szFile, char * szErrFile, HANDLE hLog )
{
    HANDLE hFile = NULL;
    DWORD dwErr;
    DWORD dwLine = 0;
    DWORD dwBufferPos;
    DWORD dwNewBufferPos;
    DWORD dwDBErr;
    BOOL  fDBErr = FALSE;
    BOOL  fLastErr = FALSE;
    char szDBErr[DB_MAX_ERR_SIZE+1];
    char szLine[4096];

    NTSVCInfo( "DBImportList(), type %s", pcType );

    if ( szErrFile[0] == 0 )
        szErrFile = NULL;

    if ( *phCnx == NULL )
    {
        NTSVCInfo( "DBImportList(), connexion à la base %s en tant que %s", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
        *phCnx = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        gsSvcWork.bDatabaseMissing = ( (*phCnx) == NULL );
        if ( *phCnx == NULL )
        {
            dwErr = ERROR_NOT_CONNECTED;
            SVC_ERR1( dwErr, "ERR_DB_CONNECT", "IMPORT" );
            return dwErr;
        }
        NTSVCInfo( "DBImportList(), connecté à la base" );
    }

    dwBufferPos = DBParseLine( pcBuffer, 0, dwBufferSize, szLine, sizeof( szLine ) );
    dwLine ++;
    if ( strlen( szLine ) == 0 )
    {
        dwErr = ERROR_INVALID_DATA;

        DBErrFile( &hFile, 
                   szFile, 
                   szErrFile, 
                   dwLine, 
                   gsSvcWork.sParmWork.dwParseErrorCode, 
                   gsSvcWork.sLogStr.szProcessParse, 
                   hLog );

        SVC_ERR( dwErr, "ERR_PARSE_FILE" );
        DBDisconnect( *phCnx );
        *phCnx = NULL;

        DBErrFileEnd( &hFile, szErrFile );
        
        return dwErr;
    }

    if ( ! DBImportListBegin( phCnx, pcType, szLine, &dwDBErr, szDBErr ) )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR( dwErr, "ERR_BEGIN_IMPORT_ORA" );
        DBDisconnect( *phCnx );
        *phCnx = NULL;

        DBErrFileEnd( &hFile, szErrFile );

        return dwErr;
    }

    if ( dwDBErr != 0 )
    {
        fDBErr = TRUE;
        DBErrFile( &hFile, szFile, szErrFile, dwLine, dwDBErr, szDBErr, hLog );
        if ( dwDBErr < 100 )
        {
            dwErr = 0xFFFFFFFF;

            SVC_ERR( dwErr, "ERR_BEGIN_IMPORT_APP" );
            DBDisconnect( *phCnx );
            *phCnx = NULL;

            DBErrFileEnd( &hFile, szErrFile );

            return dwErr;
        }

        fLastErr = TRUE;
    }

    while ( dwBufferPos < dwBufferSize )
    {
        dwNewBufferPos = DBParseLine( pcBuffer, dwBufferPos, dwBufferSize, szLine, sizeof( szLine ) );
        dwLine ++;

        // Ignorer lignes vides
        if ( strlen( szLine ) != 0 )
        {
            fLastErr = FALSE;

            if ( ! DBImportListItem( phCnx, pcType, szLine, &dwDBErr, szDBErr ) )
            {
                dwErr = ERROR_INVALID_DATA;

                SVC_ERR( dwErr, "ERR_IMPORT_ITEM_ORA" );
                DBDisconnect( *phCnx );
                *phCnx = NULL;

                DBErrFileEnd( &hFile, szErrFile );

                return dwErr;
            }

            if ( dwDBErr != 0 )
            {
                fLastErr = TRUE;
                fDBErr = TRUE;
                DBErrFile( &hFile, szFile, szErrFile, dwLine, dwDBErr, szDBErr, hLog );
                if ( dwDBErr < 100 )
                {
                    dwErr = 0xFFFFFFFF;

                    SVC_ERR( dwErr, "ERR_IMPORT_ITEM_APP" );
                    DBDisconnect( *phCnx );
                    *phCnx = NULL;

                    DBErrFileEnd( &hFile, szErrFile );

                    return dwErr;
                }
            }
        }
        dwBufferPos = dwNewBufferPos;
    }

    if ( ! DBImportListEnd( phCnx, pcType, &dwDBErr, szDBErr ) )
    {
        dwErr = ERROR_INVALID_DATA;

        SVC_ERR( dwErr, "ERR_END_IMPORT_ORA" );
        DBDisconnect( *phCnx );
        *phCnx = NULL;

        DBErrFileEnd( &hFile, szErrFile );

        return dwErr;
    }

    if ( dwDBErr != 0 )
    {
        fDBErr = TRUE;

        if ( ! fLastErr )
        {
            DBErrFile( &hFile, szFile, szErrFile, dwLine, dwDBErr, szDBErr, hLog );
            if ( dwDBErr < 100 )
            {
                dwErr = 0xFFFFFFFF;

                SVC_ERR( dwErr, "ERR_END_IMPORT_APP" );
                DBDisconnect( *phCnx );
                *phCnx = NULL;

                DBErrFileEnd( &hFile, szErrFile );

                return dwErr;
            }
        }
    }

    if ( fDBErr )
    {
        dwErr = 0xFFFFFFFF;

        SVC_ERR( dwErr, "ERR_IMPORT_FINAL" );
        DBDisconnect( *phCnx );
        *phCnx = NULL;

        DBErrFileEnd( &hFile, szErrFile );

        return dwErr;
    }

    DBErrFileEnd( &hFile, szErrFile );

    NTSVCInfo( "DBImportList(), fichier %s importé", szFile );

    return NO_ERROR;
}




PROTECTED DWORD DBGetExternalFileName( DB_CNX ** phCnx, char * pcType, char * pcFile, DWORD dwFileSize )
{
    DWORD dwErr;
    HANDLE hFile = NULL;
    DB_STMT * hStmt;
    DB_VAR * hBndRet = NULL;    DWORD dwBndRet;
    DB_VAR * hBndType = NULL;
    DB_VAR * hBndFile = NULL;
    char szErr[1024];

    if ( *phCnx == NULL )
    {
        NTSVCInfo( "DBGetExternalFileName(), connexion à la base %s en tant que %s", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
        *phCnx = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        gsSvcWork.bDatabaseMissing = ( (*phCnx) == NULL );
        if ( *phCnx == NULL )
        {
            dwErr = ERROR_NOT_CONNECTED;
            SVC_ERR1( dwErr, "ERR_DB_CONNECT", "GETEXTFILE" );
            return dwErr;
        }
        NTSVCInfo( "DBImportList(), connecté à la base" );
    }

    hStmt = DBOpenStatement( *phCnx, gszDbExtListNew );
    if ( hStmt == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_OPEN", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }
        
    /* Pour l'alignement */ hBndRet  = DBBindPlaceHolderVariable( hStmt, ":BndRet" , DB_TYPE_INT, sizeof( dwBndRet ) , 0 );
    if ( hBndRet  != NULL ) hBndType = DBBindPlaceHolderVariable( hStmt, ":BndType", DB_TYPE_STR, (DWORD)strlen( pcType ) + 1, 0 );
    if ( hBndType != NULL ) hBndFile = DBBindPlaceHolderVariable( hStmt, ":BndFile", DB_TYPE_STR, dwFileSize, 0 );
    if ( hBndFile == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_BIND", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( DBSetVariableItemValue( hBndType, 0, pcType, (DWORD)strlen(pcType) + 1 ) != pcType )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_SET", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( ! DBExecuteStatement( hStmt ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_EXEC", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( DBGetVariableItemValue( hBndFile, 0, pcFile, NULL ) != pcFile )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_GET", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( DBGetVariableItemValue( hBndRet, 0, &dwBndRet, NULL ) != &dwBndRet )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_GET", "GETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( dwBndRet == 0 )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR1( dwErr, "ERR_DB_RESULT", "GETEXTFILE" );
        return ERROR_INVALID_DATA;
    }

    return NO_ERROR;
}


PROTECTED DWORD DBSetExternalFileName( DB_CNX ** phCnx, char * pcType, char * pcFile )
{
    DWORD dwErr;
    DB_STMT * hStmt;
    DB_VAR * hBndRet = NULL;    DWORD dwBndRet;
    DB_VAR * hBndType = NULL;
    DB_VAR * hBndFile = NULL;
    char szErr[1024];

    if ( *phCnx == NULL )
    {
        NTSVCInfo( "DBSetExternalFileName(), connexion à la base %s en tant que %s", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
        *phCnx = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        gsSvcWork.bDatabaseMissing = ( (*phCnx) == NULL );
        if ( *phCnx == NULL )
        {
            dwErr = ERROR_NOT_CONNECTED;
            SVC_ERR1( dwErr, "ERR_DB_CONNECT", "SETEXTFILE" );
            return dwErr;
        }
        NTSVCInfo( "DBImportList(), connecté à la base" );
    }

    hStmt = DBOpenStatement( *phCnx, gszDbExtListAck );
    if ( hStmt == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_OPEN", "SETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }
        
    /* Pour l'alignement */ hBndRet  = DBBindPlaceHolderVariable( hStmt, ":BndRet" , DB_TYPE_INT, sizeof( dwBndRet ) , 0 );
    if ( hBndRet  != NULL ) hBndType = DBBindPlaceHolderVariable( hStmt, ":BndType", DB_TYPE_STR, (DWORD)strlen( pcType ) + 1, 0 );
    if ( hBndType != NULL ) hBndFile = DBBindPlaceHolderVariable( hStmt, ":BndFile", DB_TYPE_STR, (DWORD)strlen( pcFile ) + 1, 0 );
    if ( hBndFile == NULL )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_BIND", "SETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( ( DBSetVariableItemValue( hBndType, 0, pcType, (DWORD)strlen(pcType) + 1 ) != pcType ) ||
         ( DBSetVariableItemValue( hBndFile, 0, pcFile, (DWORD)strlen(pcFile) + 1 ) != pcFile )  )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_SET", "SETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( ! DBExecuteStatement( hStmt ) )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_EXEC", "SETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( DBGetVariableItemValue( hBndRet, 0, &dwBndRet, NULL ) != &dwBndRet )
    {
        DBGetLastError( *phCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
        SVC_ERR2( dwErr, "ERR_DB_GET", "SETEXTFILE", szErr );
        return ERROR_INVALID_DATA;
    }

    if ( dwBndRet == 0 )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR1( dwErr, "ERR_DB_RESULT", "SETEXTFILE" );
        return ERROR_INVALID_DATA;
    }

    return NO_ERROR;
}



PROTECTED BOOLEAN WINAPI DBReplaceText( char * szText, DWORD dwMaxBytes, char * szReplace, char * szBy )
{
    char * pcCurrent;
    char * pcBy;
    DWORD dwRemains;
    DWORD dwPos;
    char * pcFound;
    char * szWork;
    BOOL fReplaced = FALSE;

    if ( dwMaxBytes < 2 )
        return FALSE;

    if ( strlen( szReplace ) == 0 )
        return FALSE;

    if ( ( szWork = HeapAlloc( GetProcessHeap(), 0, dwMaxBytes ) ) == NULL )
        return FALSE;

    dwRemains = dwMaxBytes-1;
    dwPos = 0;
    pcCurrent = szText;
    
    do
    {
        pcFound = strstr( pcCurrent, szReplace );
        if ( pcFound == NULL )
        {
            while ( ( dwRemains > 0 ) && ( *pcCurrent != 0 ) )
            {
                szWork[dwPos++] = *(pcCurrent++);
                dwRemains --;
            }
        }
        else if ( pcFound == pcCurrent )
        {
            pcBy = szBy;
            while ( ( dwRemains > 0 ) && ( *pcBy != 0 ) )
            {
                szWork[dwPos++] = *(pcBy++);
                dwRemains --;
            }

            pcCurrent += strlen(szReplace);
            fReplaced = TRUE;
        }
        else
        {
            while ( ( dwRemains > 0 ) && ( pcCurrent < pcFound ) )
            {
                szWork[dwPos++] = *(pcCurrent++);
                dwRemains --;
            }
            pcBy = szBy;
            while ( ( dwRemains > 0 ) && ( *pcBy != 0 ) )
            {
                szWork[dwPos++] = *(pcBy++);
                dwRemains --;
            }

            pcCurrent += strlen(szReplace);
            fReplaced = TRUE;
        }
    }
    while ( pcFound != NULL );

    szWork[dwPos] = 0;

    strcpy_s( szText, sizeof(szText), szWork );

    HeapFree( GetProcessHeap(), 0, szWork );

    return fReplaced;
}


PROTECTED BOOL WINAPI DBFormatText( char * szFormat, char * szText, DWORD dwMaxBytes, ... )
{
    DWORD dwTag = 0;
    va_list pMark;
    char * pcCurrent;
    char szTag[16];

    va_start( pMark, dwMaxBytes ); 

    strncpy_s( szText, 2048, szFormat, dwMaxBytes );
    szText[dwMaxBytes-1] = 0;

    DBReplaceText( szText, dwMaxBytes, "\\n", "\r\n" );

    while ( ( pcCurrent = va_arg( pMark, char* ) )  != NULL )
    {
        dwTag ++;

        sprintf_s( szTag, sizeof(szTag), "[%u]", dwTag );

        DBReplaceText( szText, dwMaxBytes, szTag, pcCurrent );
    }

    return TRUE;
}





// --------------- CODE ----------------------


