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

#ifndef TRFSVC_GLOB_H
#define TRFSVC_GLOB_H

#include <protect.h>

#include "../resources/version.h"

// ----------------- MACROS ----------------------

// Espace à prévoir pour le protocol LIFE dans les buffer ACOM
#define TRFSVC_PROTOCOL_BYTES     32

// Nom du service
#define TRFSVC_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define TRFSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define TRFSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define TRFSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define TRFSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define TRFSVC_WORKINGSET_TIMEMOUT 5000

// Nom des pipes pour les différents points d'entrées du service 
// C'est à dire :
//   - Interface time,
//   - Interface de commande,
#define TRFSVC_PIPE_TRF            "\\\\.\\pipe\\" TRFSVC_SERVICE_NAME "\\TRF"
#define TRFSVC_PIPE_CMD            "\\\\.\\pipe\\" TRFSVC_SERVICE_NAME "\\CMD"
#define TRFSVC_PIPE_TRF_CLT        "\\\\%s\\pipe\\" TRFSVC_SERVICE_NAME "\\TRF"


// Définition des codes de types des clients correspondant
#define TRFSVC_PIPE_CMD_TYPE          0x00000000
#define TRFSVC_PIPE_TRF_TYPE          0xFFFFFFFF


// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000


// Définition des variables de configuration du service
typedef struct TRFSVC_PARAMS
{
#define TRFSVC_REG_VAL_CMDSVC          "CmdSvcEnabled"
    DWORD                               dwCmdSvc;

#define TRFSVC_REG_VAL_MAXCMDCNX       "MaxCmdCnx"
    DWORD                               dwMaxCmdCnx;

#define TRFSVC_REG_VAL_MAXCLTCNX       "MaxCltCnx"
    DWORD                               dwMaxCltCnx;

#define TRFSVC_REG_VAL_WORKERS         "Workers"
    DWORD                               dwWorkers;

#define TRFSVC_REG_VAL_RECONNECTTIME   "ReconnectTime"
    DWORD                               dwTimeToReconnect;

#define TRFSVC_REG_VAL_QUEUESIZE       "QueueSize"
    DWORD                               dwMaxQueuedMessages;

#define TRFSVC_REG_VAL_MAXMSGSIZE      "MaxMsgSize"
    DWORD                               dwMaxMsgSize;

#define TRFSVC_REG_VAL_BUFFERSIZE      "BufferSize"
    DWORD                               dwPipeBufferSize;

#define TRFSVC_REG_VAL_MAXLIFE         "MaxLife"
    DWORD                               dwMaxLife;

#define TRFSVC_REG_VAL_LIFETIME        "LifeTime"
    DWORD                               dwLifeTime;

#define TRFSVC_REG_VAL_CHECKPERIOD      "CheckPeriod"
    DWORD                               dwCheckPeriod;

#define TRFSVC_REG_VAL_TIMETOLERANCE    "TimeTolerance"
    DWORD                               dwTimeTolerance;

#define TRFSVC_REG_VAL_TCPCLIENT        "TcpClient"
    DWORD                               dwTcpClient;

#define TRFSVC_REG_VAL_TCPSERVER        "TcpServer"
    DWORD                               dwTcpServer;

#define TRFSVC_REG_VAL_ACOM_TRACE_FILE  "ACOMTraceFile"
	char                               szACOMTraceFile[MAX_PATH];
}
    TRFSVC_PARAMS;


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

#define SVC_ERR_SS(err,txt,s1,s2) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR4( "#ERR%u# : %s | %s | %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1, s2 );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }

#define SVC_ERR_I(err,txt,i1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR3( "#ERR%u# : %s | %u", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), i1 );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }

#define SVC_ERR_SI(err,txt,s1,i1) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR4( "#ERR%u# : %s | %s | %u (0x%08X)", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr), s1, i1 );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }



// ----------------- TYPES ----------------------

typedef struct TRFSVC_FILE
{
    char                    szFileKey[MAX_PATH];

    ULONGLONG               ullLastTime;

    BOOL                    bImport;
    BOOL                    bExport;
    ACOM_INST_HANDLE        hInst;

    char                    szFile [MAX_PATH];
    char                    szServer[MAX_PATH];
    char                    szTemp[MAX_PATH+4];

	BOOL bIsReceivingFileInProgress;
	ULONGLONG               ullLastTimeFullFileChunkReceived;

}
TRFSVC_FILE;

typedef struct TRFSVC_CLIENT
{
    ACOM_CNX_HANDLE         hCnx;

    TRFSVC_FILE           * psFile;

    DWORD                   dwMaxBufferBytes;
    BYTE                    tbBuffer[];
}
    TRFSVC_CLIENT;

typedef struct TRFSVC_SERVER
{
    ACOM_CNX_HANDLE         hCnx;

	// TMA
	DWORD					dwQueueCounter;
	DWORD					dwCounterSnapshot;
	// end of TMA

    TRFSVC_FILE           * psFile;

    char                    szClientId[MAX_PATH];
    char                    szFileKey[MAX_PATH];

    DWORD                   dwMaxBufferBytes;
    BYTE                    tbBuffer[];
}
    TRFSVC_SERVER;


typedef struct TRFSVC_EXTENSION
{
    char szName[MAX_PATH];

    HINSTANCE hInst;
    void * pfInitialize;
    void * pfTeminate;
    void * pfGetStatus;
    void * pvContext;
    char szFileDll[MAX_PATH];
}
    TRFSVC_EXTENSION;


// Définition de la structure de travail du service, regroupant toutes les données
// de configuration et de fonctionnement nécessaire au service
typedef struct TRFSVC_WORK
{
    BOOL bIsDebug;                      // Si TRUE, le service est en mode debug
    BOOL bReload;                       // Si TRUE, à la prochaine demande d'arret, le service ne s'arrète pas mais se réiinitialise

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    HCOLLECTION hText;

    ACOM_WKS_HANDLE hWks;               // Handle du working set gérant la com
    ACOM_INST_HANDLE hCmdInst;
    ACOM_INST_HANDLE hSrvInst;

    HCOLLECTION hColExtensions;
    HCOLLECTION hColFiles;
    HCOLLECTION hColClients;
    HCOLLECTION hColServers;

    NTSVC_PARAMETER_DEF  * psParams;    // Liste des paramètres du service
    TRFSVC_PARAMS sParmWork;
    TRFSVC_PARAMS sParmCopy;

	// TMA
	HANDLE hTread;
	HANDLE hReceived;
	DWORD dwThreadId;
	DWORD dwThreadReceivedId;
	DWORD WM_ACOM;
	// end of TMA

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
TRFSVC_WORK;

// TMA
DWORD WINAPI AComSendMessageCb(ACOM_WKS_HANDLE hWks,
							   ACOM_CNX_HANDLE hCnxHandle,
							   DWORD64 dwMsgUsrKey,
							   DWORD dwDataSize,
							   void * pvData,
							   TRFSVC_SERVER * psServer);
// end of TMA

// ----------------- VARIABLES GLOBALES ----------------------

// Instantiation des données de travail du service
PROTECTED TRFSVC_WORK  gsSvcWork INIT(0);

// Nom du service, utilisé par NTSVCExternalQueryInfo
char           gszServiceName[MAX_PATH]
#ifdef LOC_DEF
 = TRFSVC_SERVICE_NAME
#endif
;

#endif
