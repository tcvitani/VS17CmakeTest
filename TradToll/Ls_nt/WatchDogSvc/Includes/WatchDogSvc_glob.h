/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WATCHDOGsvc
 * FILE       : WATCHDOGsvc_glob.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : voie
 * --------------------------------------------------------------------
 * SUMMARY    : Définitions globales du service
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef WATCHDOGSVC_GLOB_H
#define WATCHDOGSVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"

// ----------------- MACROS ----------------------

// Nom du service
#define WATCHDOGSVC_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define WATCHDOGSVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define WATCHDOGSVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define WATCHDOGSVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define WATCHDOGSVC_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define WATCHDOGSVC_WORKINGSET_TIMEMOUT 5000

// Nom des pipes pour les différents points d'entrées du service 
// C'est à dire :
//   - Interface de commande,
#define WATCHDOGSVC_PIPE_CMD            "\\\\.\\pipe\\" WATCHDOGSVC_SERVICE_NAME "\\CMD"


// Définition des codes de types des clients correspondant
#define WATCHDOGSVC_CMD_TYPE              3


// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000



typedef struct WATCHDOGSVC_PARAMS
{
// Définition des variables de configuration du service
#define WATCHDOGSVC_REG_VAL_CMDSVC         "CmdSvcEnabled"
    DWORD                               dwCmdSvc;

#define WATCHDOGSVC_REG_VAL_MAXCMDCNX      "MaxCmdCnx"
    DWORD                               dwMaxCmdCnx;

#define WATCHDOGSVC_REG_VAL_WORKERS         "Workers"
    DWORD                               dwWorkers;

#define WATCHDOGSVC_REG_VAL_RECONNECTTIME  "ReconnectTime"
    DWORD                               dwTimeToReconnect;

#define WATCHDOGSVC_REG_VAL_QUEUESIZE      "QueueSize"
    DWORD                               dwMaxQueuedMessages;

#define WATCHDOGSVC_REG_VAL_MAXMSGSIZE     "MaxMsgSize"
    DWORD                               dwMaxMsgSize;

#define WATCHDOGSVC_REG_VAL_BUFFERSIZE     "BufferSize"
    DWORD                               dwPipeBufferSize;

#define WATCHDOGSVC_REG_VAL_MAXLIFE        "MaxLife"
    DWORD                               dwMaxLife;

#define WATCHDOGSVC_REG_VAL_LIFETIME       "LifeTime"
    DWORD                               dwLifeTime;

#define WATCHDOGSVC_REG_VAL_SCMPOLLING     "ScmPolling"
    DWORD                               dwScmPolling;

#define WATCHDOGSVC_REG_VAL_CMDTIMEOUTMS   "CmdTimeoutMs"
    DWORD                               dwCmdTimeoutMs;
}
    WATCHDOGSVC_PARAMS;


#define SVC_ERR(err,txt) \
    if ( TRUE ){ \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,NULL) );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }else


// ----------------- TYPES ----------------------

// Définition des services à monitorer
typedef struct  _WATCHDOGSVC_SVC
{
    DWORD   dwLastState;
    DWORD   dwLastTick;
    DWORD   dwWaitToRestart;
    char    szName[MAX_PATH];
    char    szOnStop[MAX_PATH*4];
    char    szOnStart[MAX_PATH*4];
}
    WATCHDOGSVC_SVC;
    


// Définition de la structure de travail du service, regroupant toutes les données
// de configuration et de fonctionnement nécessaire au service
typedef struct _WATCHDOGSVC_WORK
{
    BOOL bIsDebug;                      // Si TRUE, le service est en mode debug
    BOOL bReload;                       // Si TRUE, à la prochaine demande d'arret, le service ne s'arrète pas mais se réiinitialise
    BOOL fPaused;

    HICON hIconKo;
    HICON hIconOk;
    HICON hIconOkRun;
    HICON hIconPause;
    HICON hIconPauseRun;

    HCOLLECTION hText;

    ACOM_WKS_HANDLE hWks;               // Handle du working set gérant la com
    ACOM_INST_HANDLE hCmdInst;

    NTSVC_PARAMETER_DEF  * psParams;    // Liste des paramètres du service

    WATCHDOGSVC_PARAMS  sParmWork;
    WATCHDOGSVC_PARAMS  sParmCopy;

    CRITICAL_SECTION    sProtectSvc;
    DWORD               dwServices;
    WATCHDOGSVC_SVC *   psServices;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
WATCHDOGSVC_WORK;


// ----------------- VARIABLES GLOBALES ----------------------

// Instantiation des données de travail du service
PROTECTED WATCHDOGSVC_WORK  gsSvcWork INIT(0);

// Nom du service, utilisé par NTSVCExternalQueryInfo
char           gszServiceName[MAX_PATH]
#ifdef LOC_DEF
 = WATCHDOGSVC_SERVICE_NAME
#endif
;



PROTECTED DWORD WINAPI MainNotifyIcon( HICON hIcon, char * pcText );


#endif
