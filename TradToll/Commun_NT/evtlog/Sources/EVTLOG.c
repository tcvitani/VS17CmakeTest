/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : EVTLOG
 * FILE       : EVTLOG.C
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
#include <search.h>
#include <lm.h>
#include "../resources/messages.h"

//#pragma warning(disable : 4996)

#ifndef EVTLOG_EXPORTS
#   define EVTLOG_EXPORTS
#endif
#include <CSR_EVTLOG.h>


#define LOC_DEF
#include <EVTLOG_loc.h>
#undef LOC_DEF

#include <memclass.h>

#pragma comment( lib, "NETAPI32.LIB" )

#define NULLIFEMPTY(x)  ( (x)[0] == 0 ? NULL : (x) )
#define EMPTYORNULL(x)  ( (x) == NULL ? TRUE : (x)[0] == 0 )

#define MAX_EVTLOG_STRING    2048



PRIVATE HINSTANCE ghInst = NULL;
PRIVATE DWORD gdwOptions = 0;
PRIVATE WORD gtwTypeMap[] = { EVENTLOG_SUCCESS,
                              EVENTLOG_INFORMATION_TYPE,
                              EVENTLOG_WARNING_TYPE,
                              EVENTLOG_ERROR_TYPE };
PRIVATE DWORD gtdwMsgMap[] = { EVTLOG_MSG_SUCCESS,
                               EVTLOG_MSG_INFO,
                               EVTLOG_MSG_WARNING,
                               EVTLOG_MSG_ERROR };



PRIVATE int __cdecl evtLogCompareStr( const void * pvArg1,
                                      const void * pvArg2 );

PRIVATE DWORD WINAPI evtLogOpenTextDefs( OPTIONAL IN const char   * szComputer,
                                                  IN const char   * szLogFile,
                                                  OUT      HANDLE * phTextDLLs );

PRIVATE void WINAPI evtLogUnloadTextDlls( IN HANDLE hTextDLLs );

PRIVATE void WINAPI evtLogCloseTextDefs( IN HANDLE hTextDLLs );

PRIVATE HANDLE WINAPI evtLogLoadTextDll( IN HANDLE hTextDLLs,
                                         IN const char * szSourceName );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL WINAPI DllMain(
 *                      IN HINSTANCE hInstanceDll,
 *                      IN DWORD dwReason,
 *                      IN LPVOID pvReserved )
 * --------------------------------------------------------------------
 * PARAMETERS: hInstanceDll (input) : Refer to Win32 documentation
 *             dwReason     (input) : Refer to Win32 documentation
 *             pvReserved   (input) : Refer to Win32 documentation
 * --------------------------------------------------------------------
 * RETURN    : Refer to Win32 documentation
 * --------------------------------------------------------------------
 * ROLE      : Refer to Win32 documentation
 * --------------------------------------------------------------------
 */
PROTECTED BOOL WINAPI DllMain(
                        IN HINSTANCE hInstanceDll,
                        IN DWORD dwReason,
                        IN LPVOID pvReserved )
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH :
        ghInst = hInstanceDll;
        break;

    case DLL_THREAD_ATTACH  :
    case DLL_THREAD_DETACH  :
    case DLL_PROCESS_DETACH :
        break;
    }

    return TRUE;
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogCreateSource=_EvtLogCreateSource@8" )
#endif
EXPORT DWORD WINAPI EvtLogCreateSource( IN const char * szSourceName,
                                        OUT HANDLE    * phLog )
{
    return EvtLogCreateRemoteSource( NULL, szSourceName, phLog );
}




#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogCreateRemoteSource=_EvtLogCreateRemoteSource@12" )
#endif
EXPORT DWORD WINAPI EvtLogCreateRemoteSource( IN const char * szServerPath,
                                              IN const char * szSourceName,
                                              OUT HANDLE    * phLog )
{
    DWORD  dwErr;
    HKEY   hKey; 
    BOOL   fResult;
    char   szFile[MAX_PATH];
    char   szRegPath[2048] = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";

    __try
    {
        if ( strlen(szSourceName) > 1980 )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            __leave;
        }

        fResult = GetModuleFileName( ghInst, szFile, sizeof(szFile) );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        strcat_s( szRegPath, sizeof(szRegPath), szSourceName );

        dwErr = RegCreateKey( HKEY_LOCAL_MACHINE, 
                              szRegPath,
                              &hKey );
        if ( dwErr != 0 )
            __leave;

        dwErr = RegSetValueEx( hKey,
                               "EventMessageFile",
                               0,
                               REG_EXPAND_SZ,
                               (LPBYTE)szFile,
                               (DWORD)strlen(szFile) + 1 );
        if ( dwErr != 0 )
        {
            RegCloseKey( hKey ); 
            __leave;
        }

        RegCloseKey( hKey ); 

        *phLog = RegisterEventSource( szServerPath,
                                      szSourceName );
        if ( *phLog == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogCloseSource=_EvtLogCloseSource@4" )
#endif
EXPORT void WINAPI EvtLogCloseSource( IN HANDLE hLog )
{
    DeregisterEventSource( hLog );
}




EXPORT DWORD WINAPI EvtLogReportText( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN const char * szFormat,
                                      IN ... )
{
    va_list pMark;
    char    szText[4096];

    va_start( pMark, szFormat );
    _vsnprintf_s( szText, sizeof(szText), sizeof(szText), szFormat, pMark );
    szText[sizeof(szText)-1] = 0;

    return EvtLogReportBase( hLog, eType, wCategory, dwId, 0, NULL, szText );
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogReportData=_EvtLogReportData@24" )
#endif
EXPORT DWORD WINAPI EvtLogReportData( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN DWORD        dwDataLen,
                                      IN const BYTE * pbData )
{
    return EvtLogReportBase( hLog, eType, wCategory, dwId, dwDataLen, pbData, "" );
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogReportBase=_EvtLogReportBase@28" )
#endif
EXPORT DWORD WINAPI EvtLogReportBase( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN DWORD        dwDataLen,
                                      IN const BYTE * pbData,
                                      IN const char * szText )
{
    BOOL    fResult;
    DWORD   dwErr;
    DWORD   dwStrSize;
    DWORD   dwSidSize;
    char  * pszText;
    SID_NAME_USE eUse;
    char    szUser[4096];
    char    szDomain[4096];
    BYTE    tbSID[4096];

    __try
    {
        if ( eType >= EVTLOG_TYPE_COUNT )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        pszText = (char*)szText;

        dwStrSize = sizeof( szUser );
        fResult = GetUserName( szUser, &dwStrSize );
        if ( ! fResult )
            return GetLastError();
        szUser[sizeof(szUser)-1] = 0;

        dwSidSize = sizeof(tbSID);
        dwStrSize = sizeof(szDomain);
        fResult = LookupAccountName( NULL,
                                     szUser,
                                     (PSID)tbSID,
                                     &dwStrSize,
                                     szDomain,
                                     &dwSidSize,
                                     &eUse );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }
        szDomain[sizeof(szDomain)-1] = 0;

        fResult = ReportEvent( hLog,
                               gtwTypeMap[eType],
                               wCategory,
                               gtdwMsgMap[eType] | dwId,
                               (PSID)tbSID,
                               1,
                               dwDataLen,
                               &pszText,
                               (LPVOID)(pbData) );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }
        
        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogRemoteLink=_EvtLogRemoteLink@16" )
#endif
EXPORT DWORD WINAPI EvtLogRemoteLink( IN const char * szComputer,
                                      OPTIONAL IN const char * szDomain,
                                      IN const char * szUser,
                                      IN const char * szPassword )
{
    DWORD           dwErr;
    DWORD           dwIndex;
    DWORD           dwCount;
    DWORD           dwRead;
    DWORD           dwHandle = 0;
    USE_INFO_2      sUse;
    USE_INFO_2    * psUse;
    BOOL            fFound = FALSE;
    DWORD           dwOffset;
    WCHAR           wszComp[4096];
    WCHAR           wszUNC[4096];
    WCHAR           wszDom[1024];
    WCHAR           wszUsr[1024];
    WCHAR           wszPwd[1024];

    if ( ( szComputer == NULL ) || ( szUser == NULL ) || ( szPassword == NULL ) )
        dwErr = ERROR_INVALID_PARAMETER;
    else
    {
        _snwprintf_s( wszComp, sizeof(wszComp), sizeof(wszComp)/sizeof(*wszComp), L"\\\\%S\\", szComputer );
        wszComp[sizeof(wszComp)/sizeof(*wszComp)-1] = 0;

        do
        {
            dwErr = NetUseEnum( NULL,
                                2,
                                (LPBYTE*)&psUse, 
                                MAX_PREFERRED_LENGTH, 
                                &dwRead, 
                                &dwCount,
                                &dwHandle );
            if ( ( dwErr != NO_ERROR ) && ( dwErr != ERROR_MORE_DATA ) )
                break;

            for ( dwIndex = 0 ; dwIndex < dwRead ; dwIndex ++ )
            {
                if ( _wcsnicmp( wszComp, (void*)psUse[dwIndex].ui2_remote, wcslen( wszComp ) ) == 0 )
                    fFound = TRUE;
            }

            NetApiBufferFree( (LPVOID)psUse );
        }
        while ( dwErr == ERROR_MORE_DATA  );

        if ( ! fFound )
        {
            _snwprintf_s( wszUNC, sizeof(wszUNC), sizeof(wszUNC)/sizeof(*wszUNC), L"\\\\%S\\IPC$", szComputer );
            wszUNC[sizeof(wszUNC)/sizeof(*wszUNC)-1] = 0;

            if ( szDomain != NULL )
            {
                if ( szDomain[0] != 0 )
                {
                    _snwprintf_s( wszDom, sizeof(wszDom), sizeof(wszDom)/sizeof(*wszDom), L"%S", szDomain );
                    wszDom[sizeof(wszDom)/sizeof(*wszDom)-1] = 0;
                }
                else
                    szDomain = NULL;
            }

            _snwprintf_s( wszUsr, sizeof(wszUsr), sizeof(wszUsr)/sizeof(*wszUsr), L"%S", szUser );
            wszUsr[sizeof(wszUsr)/sizeof(*wszUsr)-1] = 0;
        
            _snwprintf_s( wszPwd, sizeof(wszPwd), sizeof(wszPwd)/sizeof(*wszPwd), L"%S", szPassword );
            wszPwd[sizeof(wszPwd)/sizeof(*wszPwd)-1] = 0;

            sUse.ui2_local = NULL;
            sUse.ui2_remote = (void*)wszUNC;
            sUse.ui2_password = (void*)wszPwd;
            sUse.ui2_status = 0;
            sUse.ui2_asg_type = USE_IPC;
            sUse.ui2_refcount = 0;
            sUse.ui2_usecount = 0;
            sUse.ui2_username = (void*)wszUsr;
            sUse.ui2_domainname = (void*)( szDomain == NULL ? NULL : wszDom );
    
            dwErr = NetUseAdd( NULL, 2, (BYTE*)&sUse, &dwOffset );
        }
        else
            dwErr = NO_ERROR;
    }

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogCreateView=_EvtLogCreateView@12" )
#endif
EXPORT DWORD WINAPI EvtLogCreateView( OPTIONAL IN const char * szComputer,
                                      OPTIONAL IN const char * szLogFile,
                                      OUT HANDLE    * phLog)
{
    DWORD           dwErr;
    EVTLOG_VIEWER * psViewer = NULL;

    __try
    {
        psViewer = HeapAlloc( GetProcessHeap(), 0, sizeof(*psViewer) );
        if ( psViewer == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        if ( szComputer != NULL )
        {
            strncpy_s( psViewer->szComputer, MAX_PATH, szComputer, sizeof(psViewer->szComputer) );
            psViewer->szComputer[sizeof(psViewer->szComputer)-1] = 0;
        }
        else
            psViewer->szComputer[0] = 0;

        if ( szLogFile != NULL )
        {
            strncpy_s( psViewer->szLogFile, MAX_PATH, szLogFile, sizeof(psViewer->szLogFile ) );
            psViewer->szLogFile[sizeof(psViewer->szLogFile)-1] = 0;
        }
        else
            psViewer->szLogFile[0] = 0;

        if ( evtLogOpenTextDefs( szComputer, szLogFile, &psViewer->hTextDefs ) != NO_ERROR )
            psViewer->hTextDefs = NULL;

        psViewer->hLog = OpenEventLog( NULLIFEMPTY(psViewer->szComputer), NULLIFEMPTY(psViewer->szLogFile) );
        if ( psViewer->hLog == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        *phLog = (HANDLE*)psViewer;
        psViewer = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psViewer != NULL )
            HeapFree( GetProcessHeap(), 0, psViewer );
    }

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogCloseView=_EvtLogCloseView@4" )
#endif
EXPORT void WINAPI EvtLogCloseView( IN HANDLE hLog )
{
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;

    CloseEventLog( psViewer->hLog );
    if ( psViewer->hTextDefs != NULL )
        evtLogCloseTextDefs( psViewer->hTextDefs );
    HeapFree( GetProcessHeap(), 0, psViewer );
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogReadItems=_EvtLogReadItems@16" )
#endif
EXPORT DWORD WINAPI EvtLogReadItems( IN  HANDLE          hLog,
                                     IN  DWORD           dwFlags,
                                     IN  DWORD           dwRecord,
                                     OUT EVTLOG_ENTRY ** ppsEntries )
{
    BOOL  fResult;
    DWORD dwErr = NO_ERROR;
    DWORD dwRead;
    DWORD dwNext;
    DWORD dwRecNumber;
    EVTLOG_ENTRY * psFirst = NULL;
    EVTLOG_ENTRY * psCurrent;
    EVTLOG_ENTRY * psNew;
    EVENTLOGRECORD * psRec;
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;
    BYTE  tbEventBuffer[8192];

    if ( psViewer->hTextDefs != NULL )
        evtLogUnloadTextDlls( psViewer->hTextDefs );

    if ( ( dwFlags & EVTLOG_FROM_OLDEST ) != 0 )
    {
        fResult = GetOldestEventLogRecord( psViewer->hLog, &dwRecNumber );
        if ( ! fResult )
            return GetLastError();

        fResult = ReadEventLog( psViewer->hLog,
                                EVENTLOG_FORWARDS_READ | EVENTLOG_SEEK_READ,
                                dwRecNumber,
                                (LPVOID)tbEventBuffer,
                                sizeof(tbEventBuffer),
                                &dwRead,
                                &dwNext );
    }
    else if ( ( dwFlags & EVTLOG_FROM_ANY ) != 0 )
        fResult = ReadEventLog( psViewer->hLog,
                                EVENTLOG_FORWARDS_READ | EVENTLOG_SEEK_READ,
                                dwRecord,
                                (LPVOID)tbEventBuffer,
                                sizeof(tbEventBuffer),
                                &dwRead,
                                &dwNext );

    while ( fResult )
    {
        psRec = (EVENTLOGRECORD*)tbEventBuffer;

        while ( ( dwRead > 0 ) && ( dwErr == NO_ERROR ) )
        {
            psNew = HeapAlloc( GetProcessHeap(), 0, sizeof(EVTLOG_ENTRY) - sizeof(EVENTLOGRECORD) + psRec->Length );
            if ( psNew == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            memcpy( &psNew->sEvt, psRec, psRec->Length );
            psNew->psNext = NULL;
            psNew->psViewer = psViewer;

            if ( psFirst == NULL )
            {
                psFirst = psNew;
                psCurrent = psNew;
            }
            else
            {
                psCurrent->psNext = psNew;
                psCurrent = psNew;
            }

            dwRead -= psRec->Length;
            psRec = (EVENTLOGRECORD*)( (BYTE*)psRec + psRec->Length );
        }
        
        fResult = ReadEventLog( psViewer->hLog,
                                EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                                0,
                                (LPVOID)tbEventBuffer,
                                sizeof(tbEventBuffer),
                                &dwRead,
                                &dwNext );
    }

    if ( dwErr != NO_ERROR )
    {
        EvtLogFreeItems( psFirst );
        psFirst = NULL;
    }

    *ppsEntries = psFirst;

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetNextItem=_EvtLogGetNextItem@4" )
#endif
EXPORT EVTLOG_ENTRY * WINAPI EvtLogGetNextItem( IN EVTLOG_ENTRY * psEntry )
{
    return psEntry->psNext;
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetItemDetails=_EvtLogGetItemDetails@52" )
#endif
EXPORT void WINAPI EvtLogGetItemDetails( IN EVTLOG_ENTRY * psEntry,
                                         OUT DWORD       * pdwRecordNbr,
                                         OUT DWORD       * pdwAnsiTime,
                                         OUT DWORD       * pdwEventId,
                                         OUT WORD        * pwEventType,
                                         OUT WORD        * pwEventCategory,
                                         IN  DWORD         dwSourceBytes,
                                         OUT char        * szSource,
                                         IN  DWORD         dwTextBytes,
                                         OUT char        * szText,
                                         IN OUT DWORD    * pdwDataBytes,
                                         OUT BYTE        * pbData,
                                         OUT DWORD       * pdwFlags )
{
    DWORD           dwErr = NO_ERROR;
    DWORD           dwSize;
    DWORD           dwCount;
    DWORD           dwIndex;
    DWORD           dwFlags = 0;
    char          * pcScan;
    HKEY            hKey = HKEY_LOCAL_MACHINE;
    HKEY            hKeyRoot = HKEY_LOCAL_MACHINE;
    HMODULE         hDLL = NULL;
    EVTLOG_VIEWER * psViewer = psEntry->psViewer;
    char          * tpcStrings[16];
    va_list         pvList = (va_list)tpcStrings;

    __try
    {
        if ( ( szSource != NULL ) && ( dwSourceBytes != 0 ) )
        {
            strncpy_s( szSource, dwTextBytes, (char*)(&psEntry->sEvt) + sizeof(psEntry->sEvt), dwSourceBytes );
            szSource[dwSourceBytes-1] = 0;
            dwFlags |= EVTLOG_SOURCE_PRESENT;
        }

        if ( ( szText != NULL ) && ( dwTextBytes != 0 ) )
        {
            hDLL = evtLogLoadTextDll( psViewer->hTextDefs, (char*)(&psEntry->sEvt) + sizeof(psEntry->sEvt) );

            dwCount = psEntry->sEvt.NumStrings;
            if ( dwCount > 15 )
                dwCount = 15;
            pcScan = ((char*)(&psEntry->sEvt)) + psEntry->sEvt.StringOffset;

            ZeroMemory( tpcStrings, sizeof(tpcStrings) );
            for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
            {
                tpcStrings[dwIndex] = pcScan;
                pcScan += strlen(pcScan) + 1;
            }

            if ( hDLL != NULL )
                dwSize = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                                        hDLL,
                                        psEntry->sEvt.EventID,
                                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                        szText,
                                        dwTextBytes,
                                        &pvList );
            else 
                dwSize = 0;

            if ( dwSize == 0 )
            {
                dwFlags |= EVTLOG_NO_LABEL_FOUND;
                szText[dwTextBytes-1] = 0;
                szText[0] = 0;

                _snprintf_s( szText, dwTextBytes + 1, dwTextBytes, "UNKNOWN(%u)", psEntry->sEvt.EventID );

                for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
                {
                    strncat_s( szText, dwTextBytes, " / ", dwTextBytes );
                    strncat_s( szText, dwTextBytes, tpcStrings[dwIndex], dwTextBytes );
                }

                if ( szText[dwTextBytes-1] != 0 )
                    dwFlags |= EVTLOG_TEXT_CUT;
            }                    

            szText[dwTextBytes-1] = 0;

            dwFlags |= EVTLOG_TEXT_PRESENT;
        }

        if ( ( pbData != NULL ) && ( pdwDataBytes != NULL ) && ( psEntry->sEvt.DataLength != 0 ) )
        {
            dwSize = (*pdwDataBytes);
            if ( dwSize < psEntry->sEvt.DataLength )
                dwFlags |= EVTLOG_DATA_CUT;
            else
                dwSize = psEntry->sEvt.DataLength;

            memcpy( pbData, ((BYTE*)(&psEntry->sEvt)) + psEntry->sEvt.DataOffset, dwSize );
            *pdwDataBytes = dwSize;
        }

        if ( pdwRecordNbr != NULL )
        {
            *pdwRecordNbr = psEntry->sEvt.RecordNumber;
            dwFlags |= EVTLOG_RECORD_NBR_PRESENT;
        }
        
        if ( pdwAnsiTime != NULL )
        {
            *pdwAnsiTime = psEntry->sEvt.TimeGenerated;
            dwFlags |= EVTLOG_TIME_PRESENT;
        }

        if ( pdwEventId != NULL )
        {
            *pdwEventId = psEntry->sEvt.EventID;
            dwFlags |= EVTLOG_ID_PRESENT;
        }

        if ( pwEventType != NULL )
        {
            *pwEventType = psEntry->sEvt.EventType;
            dwFlags |= EVTLOG_TYPE_PRESENT;
        }

        if ( pwEventCategory != NULL )
        {
            *pwEventCategory = psEntry->sEvt.EventCategory;
            dwFlags |= EVTLOG_CATEGORY_PRESENT;
        }
    }
    __finally
    {
        if ( hKey != HKEY_LOCAL_MACHINE )
            RegCloseKey( hKey );
        if ( hKeyRoot != HKEY_LOCAL_MACHINE )
            RegCloseKey( hKeyRoot );
        if ( pdwFlags != NULL )
            *pdwFlags = dwFlags;
    }
}
                                        



#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogFreeItems=_EvtLogFreeItems@4" )
#endif
EXPORT void WINAPI EvtLogFreeItems( IN EVTLOG_ENTRY * psEntries )
{
    EVTLOG_ENTRY * psNext;

    while ( psEntries != NULL )
    {
        psNext = psEntries->psNext;
        HeapFree( GetProcessHeap(), 0, psEntries );
        psEntries = psNext;
    }
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetStatus=_EvtLogGetStatus@16" )
#endif
EXPORT DWORD WINAPI EvtLogGetStatus( IN HANDLE   hLog,
                                     OUT DWORD * pdwOldest,
                                     OUT DWORD * pdwMostRecent,
                                     OUT DWORD * pdwCount )
{
    BOOL  fResult;
    DWORD dwErr;
    DWORD dwRecords;
    DWORD dwOldest;
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;

    __try
    {
        fResult = GetNumberOfEventLogRecords( psViewer->hLog, &dwRecords );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        fResult = GetOldestEventLogRecord( psViewer->hLog, &dwOldest );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        *pdwOldest = dwOldest;
        *pdwMostRecent = dwOldest + dwRecords - 1;
        *pdwCount = dwRecords;

        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}


#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetMostRecent=_EvtLogGetMostRecent@8" )
#endif
EXPORT DWORD WINAPI EvtLogGetMostRecent( IN HANDLE   hLog,
                                         OUT DWORD * pdwMostRecent )
{
    BOOL  fResult;
    DWORD dwErr;
    DWORD dwRecords;
    DWORD dwOldest;
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;

    __try
    {
        fResult = GetNumberOfEventLogRecords( psViewer->hLog, &dwRecords );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        fResult = GetOldestEventLogRecord( psViewer->hLog, &dwOldest );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        *pdwMostRecent = dwOldest + dwRecords - 1;
        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetOldest=_EvtLogGetOldest@8" )
#endif
EXPORT DWORD WINAPI EvtLogGetOldest( IN HANDLE   hLog,
                                     OUT DWORD * pdwOldest )
{
    BOOL  fResult;
    DWORD dwErr;
    DWORD dwOldest;
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;

    __try
    {
        fResult = GetOldestEventLogRecord( psViewer->hLog, &dwOldest );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        *pdwOldest = dwOldest;
        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetCount=_EvtLogGetCount@8" )
#endif
EXPORT DWORD WINAPI EvtLogGetCount( IN HANDLE   hLog,
                                    OUT DWORD * pdwCount )
{
    BOOL  fResult;
    DWORD dwErr;
    DWORD dwCount;
    EVTLOG_VIEWER * psViewer = (EVTLOG_VIEWER*)hLog;

    __try
    {
        fResult = GetNumberOfEventLogRecords( psViewer->hLog, &dwCount );
        if ( ! fResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        *pdwCount = dwCount;
        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}



#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogGetTime=_EvtLogGetTime@28" )
#endif
EXPORT DWORD WINAPI EvtLogGetTime( IN DWORD dwTime,
                                   OUT WORD * pwYe,
                                   OUT WORD * pwMo,
                                   OUT WORD * pwDa,
                                   OUT WORD * pwHo,
                                   OUT WORD * pwMi,
                                   OUT WORD * pwSe )
{
    DWORD dwErr;
    SYSTEMTIME sRef;
    SYSTEMTIME sLoc;
    ULONGLONG ullRef;

    __try
    {
        ZeroMemory( &sRef, sizeof(sRef) );
        sRef.wYear = 1970;
        sRef.wMonth = 1;
        sRef.wDay = 1;
        if ( ! SystemTimeToFileTime( &sRef, (LPFILETIME)&ullRef ) )
        {
            dwErr = GetLastError();
            __leave;
        }
        ullRef += (ULONGLONG)dwTime * (ULONGLONG)10000000;

        if ( ! FileTimeToSystemTime( (LPFILETIME)&ullRef, &sRef ) )
        {
            dwErr = GetLastError();
            __leave;
        }

        if ( ! SystemTimeToTzSpecificLocalTime( NULL, &sRef, &sLoc ) )
        {
            dwErr = GetLastError();
            __leave;
        }

        *pwYe = sLoc.wYear;
        *pwMo = sLoc.wMonth;
        *pwDa = sLoc.wDay;
        *pwHo = sLoc.wHour;
        *pwMi = sLoc.wMinute;
        *pwSe = sLoc.wSecond;

        dwErr = NO_ERROR;
    }
    __finally
    {
    }

    return dwErr;
}



#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:EvtLogSetOptions=_EvtLogSetOptions@4" )
#endif
EXPORT void WINAPI EvtLogSetOptions( IN DWORD dwOptions )
{
    gdwOptions = dwOptions;
}




PRIVATE int __cdecl evtLogCompareStr( const void * pvArg1,
                                      const void * pvArg2 )
{
   /* Compare all of both strings: */
   return _stricmp( pvArg1, pvArg2 );
}



PRIVATE DWORD WINAPI evtLogOpenTextDefs( OPTIONAL IN const char   * szComputer,
                                                  IN const char   * szLogFile,
                                                  OUT      HANDLE * phTextDLLs )
{
    DWORD            dwErr;
    DWORD            dwErr2;
    DWORD            dwIndex;
    DWORD            dwLen;
    DWORD            dwType;
    DWORD            dwDllMax = 0;
    DWORD            dwDllIdx = 0;
    EVTLOG_TEXTDLL * psDlls = NULL;
    EVTLOG_TEXTDLL * psCurrent;
    DWORD          * pdwTemp;
    FILETIME         sFTime;
    HKEY             hKey = HKEY_LOCAL_MACHINE;
    HKEY             hSubKey = HKEY_LOCAL_MACHINE;
    HKEY             hKeyRoot = HKEY_LOCAL_MACHINE;
    char             szSourceName[MAX_EVTLOG_STRING];
    char             szRegPath[MAX_EVTLOG_STRING];
    char             szRegDllPath[MAX_EVTLOG_STRING];

    __try
    {
        if ( ( ! EMPTYORNULL(szComputer) ) &&
             ( ( gdwOptions & EVTLOG_OPTION_REMOTETEXTDEF ) != 0 ) )
        {
            dwErr = RegConnectRegistry( szComputer,
                                        HKEY_LOCAL_MACHINE,
                                        &hKeyRoot );
            if ( dwErr != NO_ERROR )
                hKeyRoot = HKEY_LOCAL_MACHINE;
        }

        _snprintf_s( szRegPath, MAX_EVTLOG_STRING, MAX_EVTLOG_STRING - 1,
                   "SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s",
                   szLogFile );

        szRegPath[sizeof(szRegPath)-1] = 0;

        dwErr = RegOpenKeyEx( hKeyRoot,
                              szRegPath,
                              0,
                              KEY_READ,
                              &hKey );
        if ( dwErr != NO_ERROR )
        {
            if ( hKeyRoot != HKEY_LOCAL_MACHINE )
            {
                RegCloseKey( hKeyRoot );
                hKeyRoot = HKEY_LOCAL_MACHINE;
                dwErr2 = RegOpenKeyEx( hKeyRoot,
                                       szRegPath,
                                       0,
                                       KEY_READ,
                                       &hKey );
                if ( dwErr2 != NO_ERROR )
                {
                    hKey = HKEY_LOCAL_MACHINE;
                    __leave;
                }
            }
            else
            {
                hKey = HKEY_LOCAL_MACHINE;
                __leave;
            }
        }

        dwIndex = 0;

        while ( TRUE )
        {
            dwLen = sizeof(szSourceName);
            dwErr = RegEnumKeyEx( hKey, 
                                  dwIndex,
                                  szSourceName,
                                  &dwLen,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &sFTime );
            if ( dwErr != ERROR_SUCCESS )
                break;

            dwErr = RegOpenKeyEx( hKey,
                                  szSourceName,
                                  0,
                                  KEY_QUERY_VALUE | KEY_QUERY_VALUE,
                                  &hSubKey );
            if ( dwErr != NO_ERROR )
            {
                hSubKey = HKEY_LOCAL_MACHINE;
                dwIndex ++;
                continue;
            }

            dwLen = sizeof(szRegDllPath);
            dwErr = RegQueryValueEx( hSubKey, 
                                     "EventMessageFile", 
                                     NULL, 
                                     &dwType, 
                                     szRegDllPath, 
                                     &dwLen );
            if ( ( dwErr != NO_ERROR ) || ( ( dwType != REG_SZ) && ( dwType != REG_EXPAND_SZ) ) )
            {
                RegCloseKey( hSubKey );
                hSubKey = HKEY_LOCAL_MACHINE;
                dwIndex ++;
                continue;
            }
            szRegDllPath[sizeof(szRegDllPath)-1] = 0;

            if ( dwDllMax == 0 )
            {
                pdwTemp = HeapAlloc( GetProcessHeap(), 
                                     0, 
                                     ( sizeof(*psDlls) * 16 ) + sizeof(*pdwTemp) );
                if ( pdwTemp == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }

                psDlls = (void*)&pdwTemp[1];
                dwDllMax = 16;
            }
            else if ( ( dwDllIdx + 1 ) >= dwDllMax )
            {
                pdwTemp = HeapReAlloc( GetProcessHeap(), 
                                       0, 
                                       &((DWORD*)psDlls)[-1], 
                                       ( sizeof(*psDlls) * ( dwDllMax + 16 ) )  + sizeof(*pdwTemp) );
                if ( pdwTemp == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }

                psDlls = (void*)&pdwTemp[1];
                dwDllMax += 16;
            }

            psCurrent = &psDlls[dwDllIdx];
            
            if ( ExpandEnvironmentStrings( szRegDllPath, psCurrent->szDll, sizeof(psCurrent->szDll) ) == 0 )
                strncpy_s( psCurrent->szDll, 2048, szRegDllPath, sizeof(psCurrent->szDll) );
            psCurrent->szDll[sizeof(psCurrent->szDll)-1] = 0;

            strncpy_s( psCurrent->szSourceName, 2048, szSourceName, sizeof(psCurrent->szSourceName) );
            psCurrent->szSourceName[sizeof(psCurrent->szSourceName)-1] = 0;
            psCurrent->hDll = NULL;

            RegCloseKey( hSubKey );
            hSubKey = HKEY_LOCAL_MACHINE;

            dwIndex ++;
            dwDllIdx ++;
        }

        ((DWORD*)psDlls)[-1] = dwDllIdx;

        if ( dwDllIdx > 0 )
            qsort( psDlls, dwDllIdx, sizeof(*psDlls), &evtLogCompareStr );

        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( hSubKey != HKEY_LOCAL_MACHINE )
            RegCloseKey( hSubKey );
        if ( hKey != HKEY_LOCAL_MACHINE )
            RegCloseKey( hKey );
        if ( hKeyRoot != HKEY_LOCAL_MACHINE )
            RegCloseKey( hKeyRoot );
        if ( dwErr != NO_ERROR && psDlls != NULL )
        {
            HeapFree( GetProcessHeap(), 0, &((DWORD*)psDlls)[-1] );
            psDlls = NULL;
        }

        *phTextDLLs = (HANDLE)psDlls;
    }

    return dwErr;
}



PRIVATE void WINAPI evtLogUnloadTextDlls( IN HANDLE hTextDLLs )
{
    DWORD            dwIndex;
    DWORD            dwCount = ((DWORD*)hTextDLLs)[-1];
    EVTLOG_TEXTDLL * psDlls = hTextDLLs;

    for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
    {
        if ( ( psDlls[dwIndex].hDll != NULL ) &&
             ( psDlls[dwIndex].hDll != INVALID_HANDLE_VALUE ) )
        {
            FreeLibrary( psDlls[dwIndex].hDll );
            psDlls[dwIndex].hDll = NULL;
        }
    }
}


PRIVATE void WINAPI evtLogCloseTextDefs( IN HANDLE hTextDLLs )
{
    evtLogUnloadTextDlls( hTextDLLs );
    HeapFree( GetProcessHeap(), 0, &((DWORD*)hTextDLLs)[-1] );
}



PRIVATE HANDLE WINAPI evtLogLoadTextDll( IN HANDLE hTextDLLs,
                                         IN const char * szSourceName )
{
    DWORD            dwCount = ((DWORD*)hTextDLLs)[-1];
    EVTLOG_TEXTDLL * psDlls = hTextDLLs;
    EVTLOG_TEXTDLL * psFound;

    psFound = bsearch( szSourceName, psDlls, dwCount, sizeof(*psDlls), evtLogCompareStr );
    if ( psFound == NULL )
    {
        SetLastError( ERROR_FILE_NOT_FOUND );
        return NULL;
    }

    if ( psFound->hDll == NULL )
    {
        psFound->hDll = LoadLibraryEx( psFound->szDll, 
                                       NULL, 
                                       DONT_RESOLVE_DLL_REFERENCES | 
                                       LOAD_WITH_ALTERED_SEARCH_PATH | 
                                       LOAD_LIBRARY_AS_DATAFILE );
        if ( psFound->hDll == NULL )
                psFound->hDll = INVALID_HANDLE_VALUE;
    }
    
    return ( psFound->hDll == INVALID_HANDLE_VALUE ? NULL : psFound->hDll );
}



