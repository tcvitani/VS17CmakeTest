#ifndef AUTHSVC_GLOB_H
#define AUTHSVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"


// ----------------- MACROS ----------------------

// Nom du service
#define AUTHSVC_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define AUTHSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define AUTHSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define AUTHSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define AUTHSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define AUTHSVC_WORKINGSET_TIMEOUT 5000

#define AUTHSVC_PIPE_AUTH           "\\\\.\\pipe\\" AUTHSVC_SERVICE_NAME "\\AUTH"
#define AUTHSVC_PIPE_CMD            "\\\\.\\pipe\\" AUTHSVC_SERVICE_NAME "\\CMD"
#define AUTHSVC_PIPE_AUTH_CLT       "\\\\%s\\pipe\\" AUTHSVC_SERVICE_NAME "\\AUTH"
#define AUTHSVC_PIPE_COM            "\\\\%s\\pipe\\RouteSvc\\LS"


// Définition des codes de types des clients correspondant
#define AUTHSVC_PIPE_CMD_TYPE       0
#define AUTHSVC_PIPE_COM_TYPE       1
#define AUTHSVC_PIPE_SERVER_TYPE    2
#define AUTHSVC_PIPE_CLIENT_TYPE    3


#define AUTHSVC_REG_ROOT            HKEY_LOCAL_MACHINE
#define AUTHSVC_REG_KEY             NTSVC_REG_KEY_SERVICE "\\" AUTHSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM
#define AUTHSVC_REG_KEY_REQ         NTSVC_REG_KEY_SERVICE "\\" AUTHSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\DBRequests"

#define AUTHSVC_REG_KEY_COPYFILES   "CopyFiles"

#define AUTHSVC_MSG_BUFFER_SIZE     4024

typedef struct AUTHSVC_PARAMS
{
#define AUTHSVC_REG_VAL_CMDSVC              "CmdSvcEnabled"
    DWORD                                   dwCmdSvc;

#define AUTHSVC_REG_VAL_MAXCMDCNX           "MaxCmdCnx"
    DWORD                                   dwMaxCmdCnx;

#define AUTHSVC_REG_VAL_RECONNECTTIME       "ReconnectTime"
    DWORD                                   dwTimeToReconnect;

#define AUTHSVC_REG_VAL_MAXAUTHCLTCNX       "MaxAuthCltCnx"
    DWORD                                   dwAuthCltCnx;

#define AUTHSVC_REG_VAL_QUEUESIZE           "QueueSize"
    DWORD                                   dwMaxQueuedMessages;

#define AUTHSVC_REG_VAL_MAXMSGSIZE          "MaxMsgSize"
    DWORD                                   dwMaxMsgSize;

#define AUTHSVC_REG_VAL_BUFFERSIZE          "BufferSize"
    DWORD                                   dwPipeBufferSize;

#define AUTHSVC_REG_VAL_MAXLIFE             "MaxLife"
    DWORD                                   dwMaxLife;

#define AUTHSVC_REG_VAL_LIFETIME            "LifeTime"
    DWORD                                   dwLifeTime;

#define AUTHSVC_REG_VAL_MAINPOLLING         "MainPolling"
    DWORD                                   dwMainPolling;

#define AUTHSVC_REG_VAL_SYNCHRODELAY        "SynchroDelay"
    DWORD                                   dwSynchroDelay;

#define AUTHSVC_REG_VAL_ISAUTHSERVER        "IsAuthorizationServer"
    DWORD                                   dwIsAuthServer;

#define AUTHSVC_REG_VAL_ISAUTHCLIENT        "IsAuthorizationClient"
    DWORD                                   dwIsAuthClient;

#define AUTHSVC_REG_VAL_AUTHSERVERNAME      "AuthorizationServerName"
    char                                    szAuthServer[MAX_PATH];

#define AUTHSVC_REG_VAL_ROUTESERVER         "RouteServer"
    char                                    szRouteServer[MAX_PATH];

#define AUTHSVC_REG_VAL_APP_MSG_ID          "AppMsgId"
    DWORD                                   dwAppMsgId;


#define AUTHSVC_REG_VAL_DBUSR               "DbUser"
    char                                    szDbUsr[MAX_PATH];

#define AUTHSVC_REG_VAL_DBPWD               "DbPassword"
    char                                    szDbPwd[MAX_PATH];

#define AUTHSVC_REG_VAL_DBINST              "DbInstance"
    char                                    szDbInst[MAX_PATH];

#define AUTHSVC_REG_VAL_DBKEEPCONN         "DbKeepConnection"
    DWORD                                   dwDbKeepConnection;

	
#define AUTHSVC_REG_VAL_DBKEEPCONNPERIOD_MIN "DbKeepConnPeriodMin"
	DWORD                                   dwDbKeepConnPeriodMin;

#define AUTHSVC_REG_VAL_TRFCHG              "TransferfChanges"
    DWORD                                   dwTrfChanges;

#define AUTHSVC_REG_VAL_MAXTRFROWS          "MaxTrfRows"
    DWORD                                   dwMaxTrfRows;

#define AUTHSVC_REG_VAL_DBRECCONECT_IF_ERROR   "DbReconnectIfErr"
	char                                    szDbReconnectIfErr[MAX_PATH];
}
    AUTHSVC_PARAMS;



#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }


// ----------------- TYPES ----------------------

enum enumAUTH_RESULT
{
	AUTH_NOK = 0,
	AUTH_OK = 1
};

typedef struct AUTHSVC_WORK
{
    BOOL bIsDebug;
    BOOL bReload;

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    HCOLLECTION hText;

    ACOM_WKS_HANDLE hWks;
    ACOM_INST_HANDLE hCmdInst;
	ACOM_INST_HANDLE hComInst;
    ACOM_CNX_HANDLE  hComCnx;

	ACOM_WKS_HANDLE hAuthWks;
	ACOM_INST_HANDLE hSrvInst;
    ACOM_INST_HANDLE hCltInst;
	ACOM_CNX_HANDLE  hSrvCnx;

	HLIST hlRouteMsgList;

    HANDLE hEvent;

	CRITICAL_SECTION csDBConnectionProtect;
	DB_CNX * hDbCnx;
	unsigned long long ullLastDbCnxUse;

    NTSVC_PARAMETER_DEF * psParams;

    AUTHSVC_PARAMS sParmWork;
    AUTHSVC_PARAMS sParmCopy;
}
AUTHSVC_WORK;




// ----------------- VARIABLES GLOBALES ----------------------


PROTECTED AUTHSVC_WORK   gsSvcWork INIT(0);

PROTECTED char gszSvcName[MAX_PATH]
#ifdef LOC_DEF
 = AUTHSVC_SERVICE_NAME
#endif
;




#endif
