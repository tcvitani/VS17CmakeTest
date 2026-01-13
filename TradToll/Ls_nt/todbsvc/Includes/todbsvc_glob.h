#ifndef TODBSVC_GLOB_H
#define TODBSVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"

// ----------------- MACROS ----------------------

#define TODBSVC_SERVICE_NAME       RESINFO_PRODUCT

#define TODBSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define TODBSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define TODBSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define TODBSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

#define TODBSVC_WORKINGSET_TIMEOUT 5000

#define TODBSVC_MAX_PLAZA_NAME     256
#define TODBSVC_MAX_LANE_NAME      256
#define TODBSVC_MAX_LANES          100

#define TODBSVC_PIPE_CMD_TYPE 0
#define TODBSVC_PIPE_COM_TYPE 1

#define TODBSVC_PIPE_COM           "\\\\%s\\pipe\\RouteSvc\\LS"
#define TODBSVC_PIPE_CMD           "\\\\.\\pipe\\" TODBSVC_SERVICE_NAME "\\CMD"

#define TODBSVC_REG_ROOT           HKEY_LOCAL_MACHINE
#define TODBSVC_REG_KEY            NTSVC_REG_KEY_SERVICE "\\" TODBSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM
#define TODBSVC_REG_KEY_REQ        NTSVC_REG_KEY_SERVICE "\\" TODBSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\DBRequests"


// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000



typedef struct TODBSVC_PARAMS
{

#define TODBSVC_REG_VAL_CMDSVC         "CmdSvcEnabled"
    DWORD                               dwCmdSvc;

#define TODBSVC_REG_VAL_MAXLANES       "MaxLanes"
    DWORD                               dwMaxLanes;

#define TODBSVC_REG_VAL_MAXCMDCNX      "MaxCmdCnx"
    DWORD                               dwMaxCmdCnx;

#define TODBSVC_REG_VAL_WORKERS        "Workers"
    DWORD                               dwWorkers;

#define TODBSVC_REG_VAL_RECONNECTTIME  "ReconnectTime"
    DWORD                               dwTimeToReconnect;

#define TODBSVC_REG_VAL_QUEUESIZE      "QueueSize"
    DWORD                               dwMaxQueuedMessages;

#define TODBSVC_REG_VAL_MAXMSGSIZE     "MaxMsgSize"
    DWORD                               dwMaxMsgSize;

#define TODBSVC_REG_VAL_BUFFERSIZE     "BufferSize"
    DWORD                               dwPipeBufferSize;

#define TODBSVC_REG_VAL_MAXLIFE        "MaxLife"
    DWORD                               dwMaxLife;

#define TODBSVC_REG_VAL_LIFETIME       "LifeTime"
    DWORD                               dwLifeTime;

#define TODBSVC_REG_VAL_MAINPOLLING    "MainPolling"
    DWORD                               dwMainPolling;

#define TODBSVC_REG_VAL_ERRORSLEEP     "ErrorSleep"
    DWORD                               dwErrorSleep;

#define TODBSVC_REG_VAL_SENDAGAIN      "SendAgain"
    DWORD                               dwSendAgain;

#define TODBSVC_REG_VAL_POOLSCANDELAY  "PoolScanDelay"
    DWORD                               dwPoolScanDelay;

#define TODBSVC_REG_VAL_MAXFILES       "MaxFiles"
    DWORD                               dwMaxFiles;

#define TODBSVC_REG_VAL_MAXFILEMSGSIZE "MaxFileMsgSize"
    DWORD                               dwMaxFileMsgSize;

#define TODBSVC_REG_VAL_MAXFILERETRY   "MaxFileRetry"
    DWORD                               dwMaxFileRetry;

#define TODBSVC_REG_VAL_DBPOOLCOUNT    "DbPoolCount"
    DWORD                               dwDbPoolCount;

#define TODBSVC_REG_VAL_APP_MSG_ID     "AppMsgId"
    DWORD                               dwAppMsgId;

#define TODBSVC_REG_VAL_LANESFILEONLY  "LanesFileOnly"
    DWORD                               dwLanesFileOnly;

#define TODBSVC_REG_VAL_CYCLELOW       "CycleLow"
    DWORD                               dwCycleLow;

#define TODBSVC_REG_VAL_CYCLEHIGH      "CycleHigh"
    DWORD                               dwCycleHigh;

#define TODBSVC_REG_VAL_ONLYBACKUP     "OnlyBackup"
    DWORD                               dwOnlyBackup;

#define TODBSVC_REG_VAL_ACCEPTNALFILES "AcceptNALFiles"
    DWORD                               dwAcceptNALFiles;

#define TODBSVC_REG_VAL_MAXNALERRORS    "MaxNALErrors"
    DWORD                               dwMaxNALErrors;

#define TODBSVC_REG_VAL_DBUSR          "DbUser"
    char                                szDbUsr[MAX_PATH];

#define TODBSVC_REG_VAL_DBPWD          "DbPassword"
    char                                szDbPwd[MAX_PATH];

#define TODBSVC_REG_VAL_DBINST         "DbInstance"
    char                                szDbInst[MAX_PATH];

#define TODBSVC_REG_VAL_BASELANEDIR    "BaseLaneDir"
    char                                szBaseLaneDir[MAX_PATH];

#define TODBSVC_REG_VAL_REJECTLANEDIR  "RejectLaneDir"
    char                                szRejectLaneDir[MAX_PATH];

#define TODBSVC_REG_VAL_REJECTKBQUOTA  "RejectKBQuota"
    DWORD                               dwRejectKBQuota;

#define TODBSVC_REG_VAL_TCPROUTE       "TCPRoute"
    DWORD                               dwTCPRoute;

#define TODBSVC_REG_VAL_ROUTESERVER    "RouteServer"
    char                                szRouteServer[MAX_PATH];

#define TODBSVC_REG_VAL_LANESFILE      "LanesFile"
    char                                szLanesFile[MAX_PATH];

#define TODBSVC_REG_VAL_MSGFILESTRACE  "MsgFilesTrace"
    char                                szMsgFilesTrace[MAX_PATH];

#define TODBSVC_REG_VAL_MSGFILESTRACESIZE  "MsgFilesTraceSize"
    DWORD                               dwMsgFilesTraceSize;

#define TODBSVC_REG_VAL_CONTEXT_ID     "ContextId"
    DWORD                               dwContextId;

#define TODBSVC_REG_VAL_AUTOCOPYDIR    "AutoCopyDir"
    char                                szAutoCopyDir[MAX_PATH];

#define TODBSVC_REG_VAL_DEADLOCKDELAY  "DeadlockDelay"
    DWORD                               dwDeadlockDelay;
}
TODBSVC_PARAMS;

#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) );\
        if ( ! gsSvcWork.bIsCommand ) {\
            NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
            Sleep( 500 );\
        }\
    }

#define SVC_ERR_S(err,txt,s1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR3( "#ERR%u# : %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1 );\
        if ( ! gsSvcWork.bIsCommand ) {\
            NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
            Sleep( 500 );\
        }\
    }

#define SVC_ERR_SS(err,txt,s1,s2) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR4( "#ERR%u# : %s | %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1, s2 );\
        if ( ! gsSvcWork.bIsCommand ) {\
            NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
            Sleep( 500 );\
        }\
    }

#define SVC_ERR_S(err,txt,s1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR3( "#ERR%u# : %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1 );\
        if ( ! gsSvcWork.bIsCommand ) {\
            NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
            Sleep( 500 );\
        }\
    }

#define SVC_ERR_SI(err,txt,s1,i1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR4( "#ERR%u# : %s | %s | %u (0x%08X)", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1, i1 );\
        if ( ! gsSvcWork.bIsCommand ) {\
            NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
            Sleep( 500 );\
        }\
    }
// ----------------- TYPES ----------------------


typedef struct TODBSVC_LANE_ID
{
    DWORD      dwPlaza;
    DWORD      dwLane;
}
TODBSVC_LANE_ID;

typedef struct TODBSVC_LANE
{
    TODBSVC_LANE_ID sId;
    BOOL       bConnected;
    BOOL       bSuspended;
    BOOL       bUsedByPool;
    BOOL       bRequestToSend;

    DWORD      dwLastTreated;
    BOOL       bSaveLastTreated;
    DWORD      dwNextPresent;
    DWORD      dwRetry;
    LONGLONG   llLastDone;
    LONGLONG   llLastBackup;
}
TODBSVC_LANE;

typedef struct TODBSVC_WORK
{
    BOOL bIsDebug;
    BOOL bReload;
    BOOL fLicenceIsValid;               // Si TRUE, les condition de licence sont OK
    BOOL bTerminatePool;
    BOOL bIsCommand;
    BOOL bDatabaseMissing;

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    HCOLLECTION hText;

    HANDLE hWakeupEvent;
    ACOM_WKS_HANDLE hWks;
    ACOM_INST_HANDLE hCmdInst;
    ACOM_INST_HANDLE hComInst;
    ACOM_CNX_HANDLE  hComCnx;
    HCOLLECTION hLanes;
    LONGLONG llSendAgain;
    LONGLONG llPoolScanDelay;
    HLIST hInMsgList;

    TRC_EMETTEUR hTrc;

    NTSVC_PARAMETER_DEF * psParams;

    TODBSVC_PARAMS sParmWork;
    TODBSVC_PARAMS sParmCopy;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
TODBSVC_WORK;



// ----------------- VARIABLES GLOBALES ----------------------


PROTECTED TODBSVC_WORK   gsSvcWork INIT(0);

PROTECTED char gszSvcName[MAX_PATH]
#ifdef LOC_DEF
 = TODBSVC_SERVICE_NAME
#endif
;




#endif
