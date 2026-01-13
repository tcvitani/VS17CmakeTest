#ifndef TOLANESSVC_GLOB_H
#define TOLANESSVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"


// ----------------- MACROS ----------------------

#define TOLANESSVC_SERVICE_NAME       RESINFO_PRODUCT

#define TOLANESSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define TOLANESSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define TOLANESSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define TOLANESSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

#define TOLANESSVC_WORKINGSET_TIEMOUT 5000

#define TOLANESSVC_MAX_PLAZA_NAME     256
#define TOLANESSVC_MAX_LANE_NAME      256
#define TOLANESSVC_MAX_LANES          100

#define TOLANESSVC_PIPE_CMD_TYPE 0
#define TOLANESSVC_PIPE_COM_TYPE 1

#define TOLANESSVC_PIPE_COM           "\\\\%s\\pipe\\RouteSvc\\LS"
#define TOLANESSVC_PIPE_CMD           "\\\\.\\pipe\\" TOLANESSVC_SERVICE_NAME "\\CMD"

#define TOLANESSVC_REG_ROOT           HKEY_LOCAL_MACHINE
#define TOLANESSVC_REG_KEY            NTSVC_REG_KEY_SERVICE "\\" TOLANESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM
#define TOLANESSVC_REG_KEY_REQ        NTSVC_REG_KEY_SERVICE "\\" TOLANESSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\DBRequests"



// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000



typedef struct TOLANESSVC_PARAMS
{
#define TOLANESSVC_REG_VAL_CMDSVC          "CmdSvcEnabled"
    DWORD                                   dwCmdSvc;

#define TOLANESSVC_REG_VAL_MAXLANES        "MaxLanes"
    DWORD                                   dwMaxLanes;

#define TOLANESSVC_REG_VAL_MAXCMDCNX       "MaxCmdCnx"
    DWORD                                   dwMaxCmdCnx;

#define TOLANESSVC_REG_VAL_WORKERS         "Workers"
    DWORD                                  dwWorkers;

#define TOLANESSVC_REG_VAL_RECONNECTTIME   "ReconnectTime"
    DWORD                                   dwTimeToReconnect;

#define TOLANESSVC_REG_VAL_QUEUESIZE       "QueueSize"
    DWORD                                   dwMaxQueuedMessages;

#define TOLANESSVC_REG_VAL_MAXMSGSIZE      "MaxMsgSize"
    DWORD                                   dwMaxMsgSize;

#define TOLANESSVC_REG_VAL_BUFFERSIZE      "BufferSize"
    DWORD                                   dwPipeBufferSize;

#define TOLANESSVC_REG_VAL_MAXLIFE         "MaxLife"
    DWORD                                   dwMaxLife;

#define TOLANESSVC_REG_VAL_LIFETIME        "LifeTime"
    DWORD                                   dwLifeTime;

#define TOLANESSVC_REG_VAL_MAINPOLLING     "MainPolling"
    DWORD                                   dwMainPolling;

#define TOLANESSVC_REG_VAL_ERRORSLEEP      "ErrorSleep"
    DWORD                                   dwErrorSleep;

#define TOLANESSVC_REG_VAL_SENDAGAIN       "SendAgain"
    DWORD                                   dwSendAgain;

#define TOLANESSVC_REG_VAL_APP_MSG_ID      "AppMsgId"
    DWORD                                   dwAppMsgId;

#define TOLANESSVC_REG_VAL_LANESFILEONLY   "LanesFileOnly"
    DWORD                                   dwLanesFileOnly;

#define TOLANESSVC_REG_VAL_TCPROUTE        "TCPRoute"
    DWORD                                   dwTCPRoute;

#define TOLANESSVC_REG_VAL_DBUSR           "DbUser"
    char                                    szDbUsr[MAX_PATH];

#define TOLANESSVC_REG_VAL_DBPWD           "DbPassword"
    char                                    szDbPwd[MAX_PATH];

#define TOLANESSVC_REG_VAL_DBINST          "DbInstance"
    char                                    szDbInst[MAX_PATH];

#define TOLANESSVC_REG_VAL_ROUTESERVER     "RouteServer"
    char                                    szRouteServer[MAX_PATH];

#define TOLANESSVC_REG_VAL_LANESFILE       "LanesFile"
    char                                    szLanesFile[MAX_PATH];

#define TOLANESSVC_REG_VAL_REFERENCEDIR    "ReferenceDir"
    char                                    szReferenceDir[MAX_PATH];

#define TOLANESSVC_REG_VAL_CONTEXT_ID      "ContextId"
    DWORD                                   dwContextId;

#define TOLANESSVC_REG_VAL_PERMANENTDB     "PermanentDB"
    DWORD                                   dwPermanentDB;

}
    TOLANESSVC_PARAMS;


#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }

#define SVC_ERR_S(err,txt,s1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR3( "#ERR%u# : %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1 );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }

// ----------------- TYPES ----------------------


typedef struct TOLANESSVC_LANE
{
    BOOL       bBusy;
    DWORD      dwPlaza;
    DWORD      dwLane;
    BOOL       bConnected;
    BOOL       bMustSend;
    LONGLONG   llLastSent;
    char       szRefFile[MAX_PATH];
}
TOLANESSVC_LANE;


typedef struct TOLANESSVC_WORK
{
    BOOL bIsDebug;
    BOOL bReload;
    BOOL bDatabaseMissing;

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    HCOLLECTION hText;

    CRITICAL_SECTION sCritical;
    HANDLE hWakeupEvent;

    ACOM_WKS_HANDLE hWks;
    ACOM_INST_HANDLE hCmdInst;
    ACOM_INST_HANDLE hComInst;
    ACOM_CNX_HANDLE  hComCnx;

    HLIST hInMsgList;

    TOLANESSVC_LANE * psList;
    TOLANESSVC_LANE * psListNew;

    LONGLONG llSendAgain;
    DB_CNX * hDbCnx;

    NTSVC_PARAMETER_DEF * psParams;

    TOLANESSVC_PARAMS sParmWork;
    TOLANESSVC_PARAMS sParmCopy;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
TOLANESSVC_WORK;




// ----------------- VARIABLES GLOBALES ----------------------


PROTECTED TOLANESSVC_WORK   gsSvcWork INIT(0);

PROTECTED char gszSvcName[MAX_PATH]
#ifdef LOC_DEF
 = TOLANESSVC_SERVICE_NAME
#endif
;


#endif
