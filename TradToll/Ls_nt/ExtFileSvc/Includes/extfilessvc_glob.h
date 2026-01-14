#ifndef EXTFILESSVC_GLOB_H
#define EXTFILESSVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"


// ----------------- MACROS ----------------------

// Nom du service
#define EXTFILESSVC_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define EXTFILESSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define EXTFILESSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define EXTFILESSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define EXTFILESSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define EXTFILESSVC_WORKINGSET_TIMEOUT 5000


#define EXTFILESSVC_PIPE_CMD           "\\\\.\\pipe\\" EXTFILESSVC_SERVICE_NAME "\\CMD"

#define EXTFILESSVC_REG_ROOT           HKEY_LOCAL_MACHINE
#define EXTFILESSVC_REG_KEY            NTSVC_REG_KEY_SERVICE "\\" EXTFILESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM

#define EXTFILESSVC_REG_KEY_COPYFILES      "CopyFiles"
#define EXTFILESSVC_REG_KEY_IMPORTFILES    "ImportFiles"
#define EXTFILESSVC_REG_KEY_REQ            NTSVC_REG_KEY_SERVICE "\\" EXTFILESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\DBRequests"



// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000


#define EXTFILESSVC_MAXPATH     1024


typedef struct EXTFILESSVC_PARAMS
{
#define EXTFILESSVC_REG_VAL_CMDSVC         "CmdSvcEnabled"
    DWORD                                   dwCmdSvc;

#define EXTFILESSVC_REG_VAL_MAXCMDCNX      "MaxCmdCnx"
    DWORD                                   dwMaxCmdCnx;

#define EXTFILESSVC_REG_VAL_WORKERS        "Workers"
    DWORD                                  dwWorkers;

#define EXTFILESSVC_REG_VAL_RECONNECTTIME  "ReconnectTime"
    DWORD                                   dwTimeToReconnect;

#define EXTFILESSVC_REG_VAL_QUEUESIZE      "QueueSize"
    DWORD                                   dwMaxQueuedMessages;

#define EXTFILESSVC_REG_VAL_MAXMSGSIZE     "MaxMsgSize"
    DWORD                                   dwMaxMsgSize;

#define EXTFILESSVC_REG_VAL_BUFFERSIZE     "BufferSize"
    DWORD                                   dwPipeBufferSize;

#define EXTFILESSVC_REG_VAL_MAXLIFE        "MaxLife"
    DWORD                                   dwMaxLife;

#define EXTFILESSVC_REG_VAL_LIFETIME       "LifeTime"
    DWORD                                   dwLifeTime;

#define EXTFILESSVC_REG_VAL_MAINPOLLING    "MainPolling"
    DWORD                                   dwMainPolling;

#define EXTFILESSVC_REG_VAL_MAXBAKTRIES    "MaxBakTries"
    DWORD                                   dwMaxBakTries;

#define EXTFILESSVC_REG_VAL_MAXBAKFILES    "MaxBakFiles"
    DWORD                                   dwMaxBakFiles;

#define EXTFILESSVC_REG_VAL_DBUSR          "DbUser"
    char                                    szDbUsr[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_DBPWD          "DbPassword"
    char                                    szDbPwd[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_DBINST         "DbInstance"
    char                                    szDbInst[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_INDIRECTORY    "InDirectory"
    char                                    szInDir[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_OUTDIRECTORY   "OutDirectory"
    char                                    szOutDir[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_BAKDIRECTORY   "BakDirectory"
    char                                    szBakDir[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_ERRDIRECTORY   "ErrDirectory"
    char                                    szErrDir[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_REJECTDIRECTORY "RejectDirectory"
    char                                    szRejectDir[EXTFILESSVC_MAXPATH];

#define EXTFILESSVC_REG_VAL_EVTLOGERRORS   "EvtLogErrors"
    DWORD                                   dwEvtLogErrors;

#define EXTFILESSVC_REG_VAL_PARSEERRORCODE "ParseErrorCode"
    DWORD                                   dwParseErrorCode;

#define EXTFILESSVC_REG_VAL_ERRORMAXHEAD   "ErrorMaxHead"
    DWORD                                   dwErrorMaxHead;

#define EXTFILESSVC_REG_VAL_ERRORMAXLINE   "ErrorMaxLine"
    DWORD                                   dwErrorMaxLine;

#define EXTFILESSVC_REG_VAL_SORTBYNAME     "SortByName"
    DWORD                                   dwSortByName;
}
    EXTFILESSVC_PARAMS;



#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        if ( gsSvcWork.sParmWork.dwEvtLogErrors != 0 ) EvtLogReportText( gsSvcWork.hLog, EVTLOG_ERROR, 0, (err), "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) ); \
        Sleep( 500 );\
    }

#define SVC_ERR1(err,txt,a) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR3( "#ERR%u# : %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), a );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        if ( gsSvcWork.sParmWork.dwEvtLogErrors != 0 ) EvtLogReportText( gsSvcWork.hLog, EVTLOG_ERROR, 0, (err), "#ERR%u# : %s\r\n -> %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), a ); \
        Sleep( 500 );\
    }

#define SVC_ERR2(err,txt,a,b) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR4( "#ERR%u# : %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), a, b );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        if ( gsSvcWork.sParmWork.dwEvtLogErrors != 0 ) EvtLogReportText( gsSvcWork.hLog, EVTLOG_ERROR, 0, (err), "#ERR%u# : %s\r\n -> %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), a, b ); \
        Sleep( 500 );\
    }

// ----------------- TYPES ----------------------


#define EXTFILESSVC_DLL_FUNCTION_NAME "ExtFilesProcess"
typedef DWORD WINAPI EXTFILESSVC_PROCESS_FILE( char * pcOldPath, char * pcNewPath, char * pcProcessParam );

typedef struct EXTFILESSVC_FILE
{
    char    szType[EXTFILESSVC_MAXPATH];
    char    szTypeDB[EXTFILESSVC_MAXPATH];
    char    szInMask[EXTFILESSVC_MAXPATH];
    char    szProcessLib[EXTFILESSVC_MAXPATH];
    char    szProcessParam[EXTFILESSVC_MAXPATH];
    BOOL    bImportData;
    HANDLE  hLog;
    char    szBlockedFile[EXTFILESSVC_MAXPATH];
}
EXTFILESSVC_FILE;



typedef struct EXTFILESSVC_LOGSTRINGS
{
    char    szServiceStarted[1024];
    char    szServiceStopped[1024];
    char    szProcessSuccess[1024];
    char    szProcessParse[1024];
    char    szProcessErrorInt[1024];
    char    szProcessErrorExt[1024];
}
EXTFILESSVC_LOGSTRINGS;



typedef struct EXTFILESSVC_WORK
{
    BOOL bIsDebug;
    BOOL bReload;
    BOOL bDatabaseMissing;

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    HCOLLECTION hText;

    ACOM_WKS_HANDLE hWks;
    ACOM_INST_HANDLE hCmdInst;
    
    HANDLE hEvent;
    HANDLE hLog;
    HANDLE hSysLog;

    HCOLLECTION hFiles;

    NTSVC_PARAMETER_DEF * psParams;

    EXTFILESSVC_PARAMS sParmWork;
    EXTFILESSVC_PARAMS sParmCopy;

    EXTFILESSVC_LOGSTRINGS sLogStr;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
EXTFILESSVC_WORK;




// ----------------- VARIABLES GLOBALES ----------------------


PROTECTED EXTFILESSVC_WORK   gsSvcWork INIT(0);

PROTECTED char gszSvcName[EXTFILESSVC_MAXPATH]
#ifdef LOC_DEF
 = EXTFILESSVC_SERVICE_NAME
#endif
;




#endif
