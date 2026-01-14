/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : EVTLOG
 * FILE       : EVTLOG.H
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

#ifndef EVTLOG_H
#define EVTLOG_H


#ifdef EVTLOG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#define EVTLOG_OPTION_LOCALTEXTDEF  0x00000000
#define EVTLOG_OPTION_REMOTETEXTDEF 0x00000001


#define EVTLOG_FROM_OLDEST          0x00000001
#define EVTLOG_FROM_ANY             0x00000002



#define EVTLOG_RECORD_NBR_PRESENT   0x00000001
#define EVTLOG_TIME_PRESENT         0x00000002
#define EVTLOG_ID_PRESENT           0x00000004
#define EVTLOG_TYPE_PRESENT         0x00000008
#define EVTLOG_CATEGORY_PRESENT     0x00000010
#define EVTLOG_SOURCE_PRESENT       0x00000020
#define EVTLOG_TEXT_PRESENT         0x00000040
#define EVTLOG_DATA_PRESENT         0x00000080
#define EVTLOG_TEXT_CUT             0x00010000
#define EVTLOG_DATA_CUT             0x00020000
#define EVTLOG_NO_LABEL_FOUND       0x00040000



typedef enum
{
    EVTLOG_TYPE_FIRST = 0,

    EVTLOG_SUCCESS = EVTLOG_TYPE_FIRST,
    EVTLOG_INFO,
    EVTLOG_WARNING,
    EVTLOG_ERROR,

    EVTLOG_TYPE_COUNT
}
    EVTLOG_TYPE;




typedef struct _EVTLOG_VIEWER EVTLOG_VIEWER;
typedef struct _EVTLOG_ENTRY  EVTLOG_ENTRY;

typedef struct _EVTLOG_ENTRY
{
    EVTLOG_ENTRY   * psNext;
    EVTLOG_VIEWER  * psViewer;
    EVENTLOGRECORD   sEvt;
}
    EVTLOG_ENTRY;


typedef struct _EVTLOG_VIEWER
{
    char    szComputer[MAX_PATH];
    char    szLogFile[MAX_PATH];
    HANDLE  hLog;
    HANDLE  hTextDefs;
}
    EVTLOG_VIEWER;



EXPORT DWORD WINAPI EvtLogCreateSource( IN const char * szSourceName,
                                        OUT HANDLE    * phLog );

EXPORT DWORD WINAPI EvtLogCreateRemoteSource( IN const char * szServerPath,
                                              IN const char * szSourceName,
                                              OUT HANDLE    * phLog );

EXPORT void WINAPI EvtLogCloseSource( IN HANDLE hLog );

EXPORT DWORD WINAPI EvtLogReportText( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN const char * szFormat,
                                      IN ... );

EXPORT DWORD WINAPI EvtLogReportData( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN DWORD        dwDataLen,
                                      IN const BYTE * pbData );

EXPORT DWORD WINAPI EvtLogReportBase( IN HANDLE       hLog,
                                      IN EVTLOG_TYPE  eType,
                                      IN WORD         wCategory,
                                      IN DWORD        dwId,
                                      IN DWORD        dwDataLen,
                                      IN const BYTE * pbData,
                                      IN const char * szText );

EXPORT DWORD WINAPI EvtLogRemoteLink( IN const char * szComputer,
                                      OPTIONAL IN const char * szDomain,
                                      IN const char * szUser,
                                      IN const char * szPassword );

EXPORT DWORD WINAPI EvtLogCreateView( OPTIONAL IN const char * szComputer,
                                      OPTIONAL IN const char * szLogFile,
                                      OUT HANDLE    * phLog);

EXPORT void WINAPI EvtLogCloseView( IN HANDLE hLog );

EXPORT DWORD WINAPI EvtLogReadItems( IN  HANDLE          hLog,
                                     IN  DWORD           dwFlags,
                                     IN  DWORD           dwRecord,
                                     OUT EVTLOG_ENTRY ** ppsEntries );

EXPORT EVTLOG_ENTRY * WINAPI EvtLogGetNextItem( IN EVTLOG_ENTRY * psEntry );

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
                                         OUT DWORD       * pdwFlags );

EXPORT void WINAPI EvtLogFreeItems( IN EVTLOG_ENTRY * psEntries );

EXPORT DWORD WINAPI EvtLogGetStatus( IN HANDLE   hLog,
                                     OUT DWORD * pdwOldest,
                                     OUT DWORD * pdwMostRecent,
                                     OUT DWORD * pdwCount );

EXPORT DWORD WINAPI EvtLogGetMostRecent( IN HANDLE   hLog,
                                         OUT DWORD * pdwMostRecent );

EXPORT DWORD WINAPI EvtLogGetOldest( IN HANDLE   hLog,
                                     OUT DWORD * pdwOldest );

EXPORT DWORD WINAPI EvtLogGetCount( IN HANDLE   hLog,
                                    OUT DWORD * pdwCount );

EXPORT DWORD WINAPI EvtLogGetTime( IN DWORD dwTime,
                                   OUT WORD * pwYe,
                                   OUT WORD * pwMo,
                                   OUT WORD * pwDa,
                                   OUT WORD * pwHo,
                                   OUT WORD * pwMi,
                                   OUT WORD * pwSe );

EXPORT void WINAPI EvtLogSetOptions( IN DWORD dwOptions );

#endif
