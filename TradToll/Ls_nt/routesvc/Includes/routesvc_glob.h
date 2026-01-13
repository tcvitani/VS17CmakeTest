/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_glob.h
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

#ifndef ROUTESVC_GLOB_H
#define ROUTESVC_GLOB_H

#include <protect.h>

#include "..\resources\version.h"


// ----------------- MACROS ----------------------

// Nom du service
#define ROUTESVC_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define ROUTESVC_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define ROUTESVC_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define ROUTESVC_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define ROUTESVC_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define ROUTESVC_WORKINGSET_TIMEMOUT 5000

// Nom des pipes pour les différents points d'entrées du service 
// C'est à dire :
//   - Interface de commande,
//   - Clients LS
//   - Clients voies
#define ROUTESVC_PIPE_CMD           "\\\\.\\pipe\\" ROUTESVC_SERVICE_NAME "\\CMD"
#define ROUTESVC_PIPE_LS            "\\\\.\\pipe\\" ROUTESVC_SERVICE_NAME "\\LS"
#define ROUTESVC_PIPE_LC            "\\\\.\\pipe\\" ROUTESVC_SERVICE_NAME "\\LC"

// Définition des codes de types des clients correspondant
#define ROUTESVC_PIPE_LC_TYPE       0
#define ROUTESVC_PIPE_LS_TYPE       1
#define ROUTESVC_PIPE_CMD_TYPE      2

// Macro pour obtenir une chaine en fonction du code de type de client
#define ROUTESVC_PIPE_TYPE_TEXT(x)  ((x==0)?"LC":((x==1)?"LS":((x==2)?"CMD":"UNKWNOWN")))


// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000
#define MAX_CONCENTRATED_LANES  100


// Définition des variables de configuration du service

typedef struct ROUTESVC_PARAMS
{
#define ROUTESVC_REG_VAL_CMDSVC        "CmdSvcEnabled"
    DWORD                               dwCmdSvc;               // Indique si le service est utilisable avec une interface de commande

#define ROUTESVC_REG_VAL_MAXCMDCNX     "MaxCmdCnx"
    DWORD                               dwMaxCmdCnx;            // Nombre de clients interface de commande connectables simultanément

#define ROUTESVC_REG_VAL_MAXLSCNX      "MaxLSCnx"
    DWORD                               dwMaxLSCnx;             // Nombre de clients LS connectables simultanément

#define ROUTESVC_REG_VAL_MAXLCCNX      "MaxLCCnx"
    DWORD                               dwMaxLCCnx;             // Nombre de clients voie connectables simultanément

#define ROUTESVC_REG_VAL_TCPLS         "TCPLS"
    DWORD                               dwTCPLS;                // Activation du serveur LS sur TCP (donne le numéro de port)

#define ROUTESVC_REG_VAL_TCPLC         "TCPLC"
    DWORD                               dwTCPLC;                // Activation du serveur voie sur TCP (donne le numéro de port)

#define ROUTESVC_REG_VAL_WORKERS       "Workers"
    DWORD                               dwWorkers;              // Nombre de threads dédiés à la com

#define ROUTESVC_REG_VAL_RECONNECTTIME "ReconnectTime"
    DWORD                               dwTimeToReconnect;      // Délai de reconnexion après déconnexion

#define ROUTESVC_REG_VAL_QUEUESIZE     "QueueSize"
    DWORD                               dwMaxQueuedMessages;    // Nombre maximum de messages sortant en attente de traitement

#define ROUTESVC_REG_VAL_MAXMSGSIZE    "MaxMsgSize"
    DWORD                               dwMaxMsgSize;           // Taille maximum des messages traités

#define ROUTESVC_REG_VAL_BUFFERSIZE    "BufferSize"
    DWORD                               dwPipeBufferSize;       // Taille des buffers alloués à chaque pipe

#define ROUTESVC_REG_VAL_MAXLIFE       "MaxLife"
    DWORD                               dwMaxLife;              // Nombre maximum de demandes de vies protocole LIFE

#define ROUTESVC_REG_VAL_LIFETIME      "LifeTime"
    DWORD                               dwLifeTime;             // Délai de demandes de vies protocole LIFE

#define ROUTESVC_REG_VAL_APP_MSG_ID    "AppMsgId"
    DWORD                               dwAppMsgId;             // Id d'application pour la messagerie

#define ROUTESVC_REG_VAL_ACOM_TRC_ON   "AComTraceOn"
    DWORD                               dwAComTraceOn;          // Flag d'activation des traces ACOM

#define ROUTESVC_REG_VAL_ACOM_ERR_ON   "AComErrorOn"
    DWORD                               dwAComErrorOn;          // Flag d'activation des traces ACOM

#define ROUTESVC_REG_VAL_TRACE_LC_MSG  "TraceLCMsg"
    DWORD                               dwTraceLCMsg;           // Trace des messages de l'interface LC

#define ROUTESVC_REG_VAL_TRACE_LS_MSG  "TraceLSMsg"
    DWORD                               dwTraceLSMsg;           // Trace des messages de l'interface LC

#define ROUTESVC_REG_VAL_ACOM_FILE     "AComFile"
    char                                szAComTraceFile[MAX_PATH];          // Flag d'activation des traces ACOM
}
ROUTESVC_PARAMS;


#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), TextFind(gsSvcWork.hText,pcTxtErr,pcTxtErr) );\
        NTSVCRefreshTrayIcon( gsSvcWork.hIconKo, "%s", pcTxtErr );\
        Sleep( 500 );\
    }


// ----------------- TYPES ----------------------


// Définition d'une entrée de filtre pour les connexions
// des clients LS (dans le sens serveurs ver client)
typedef struct ROUTESVC_FILTER
{
    DWORD dwID; 
    DWORD dwCD;
}
ROUTESVC_FILTER;

// Définition d'un filtre complet pour les connexions
// des clients LS (dans le sens serveurs ver client)
typedef struct ROUTESVC_LS
{
    DWORD             dwFilterCount;    // Nombre d'entrées de filtre dans la table pointée par psFilterList
    ROUTESVC_FILTER  * psFilterList;    // Pointe sur la table des entrées du filtre
}
ROUTESVC_LS;

// Définition des données spécifiques à une connexion de type voie
typedef struct ROUTESVC_LC
{
    BOOL              fQualified;       // Indique que les donnée suivantes sont à jour
    BOOL              fMultiple;        // Flag pour une connexion de concentration
    DWORD             dwCount;          // Nombre de voies concentrés sur cette connexion
    DWORD             tdwPlaza[MAX_CONCENTRATED_LANES];       // Id de la gare
    DWORD             tdwLane[MAX_CONCENTRATED_LANES];        // Id de la voie
}   
ROUTESVC_LC;

// Définition des données spécifiques à une connexion de type interface de commande
typedef struct ROUTESVC_CMD
{
    BOOL bSpyCnx;                       // Activation de l'espion des connexions / déconnexion
    DWORD dwSpyCount;                   // Nombre d'espions dans la liste pointée par pdwSpyList
    DWORD * pdwSpyList;                 // Liste des id des connexion espionées
}
ROUTESVC_CMD;

// Définition des données générique à une connexion quelque soit son type
typedef struct ROUTESVC_CONNECTION
{
    BOOL              bBusy;            // Si TRUE, la structure est utilisée, sinon, elle est libre
    DWORD             dwType;           // Type de la connexion ROUTESVC_PIPE_CMD, ROUTESVC_PIPE_LS ou ROUTESVC_PIPE_LC
    ACOM_CNX_HANDLE   hCnx;             // Handle de la connexion

    union                               // Données spécifiques
    {
    ROUTESVC_LC         sLC;
    ROUTESVC_LS         sLS;
    ROUTESVC_CMD        sCmd;
    } 
    uCnx;
}
ROUTESVC_CONNECTION;

// Définition de la structure de travail du service, regroupant toutes les données
// de configuration et de fonctionnement nécessaire au service
typedef struct ROUTESVC_WORK
{
    BOOL bIsDebug;                      // Si TRUE, le service est en mode debug
    BOOL bReload;                       // Si TRUE, à la prochaine demande d'arret, le service ne s'arrète pas mais se réiinitialise

    BOOL fLicenceIsValid;               // Si TRUE, les condition de licence sont OK

    HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;
    HICON hIconKoRun;

    HCOLLECTION hText;

    ACOM_WKS_HANDLE hWks;               // Handle du working set gérant la com
    DWORD dwMaxTotalCnx;                // Nombre maximum de connexions gérées par le working set
    ACOM_INST_HANDLE hLCInst;           // Handle d'instance pour le serveur des connexions voies
    ACOM_INST_HANDLE hLSInst;           // Handle d'instance pour le serveur des connexions LS
    ACOM_INST_HANDLE hCmdInst;          // Handle d'instance pour le serveur des connexions interface de commande

    HLIST hInMsgList;                   // Handle de la liste des message acceptés (coté LS)

    CRITICAL_SECTION  sCritical;        // Protection de la liste pointée par psList
    ROUTESVC_CONNECTION * psList;       // Table des connexion
    DWORD dwListCount;                  // Taille de la table
    DWORD dwLCStart;                    // Index du premier élément de la liste pour les connexions voies
    DWORD dwLSStart;                    // Index du premier élément de la liste pour les connexions LS
    DWORD dwCmdStart;                   // Index du premier élément de la liste pour les connexions interface de commande
    DWORD dwLCEnd;                      // Index du dernier élément de la liste pour les connexions voies
    DWORD dwLSEnd;                      // Index du dernier élément de la liste pour les connexions LS
    DWORD dwCmdEnd;                     // Index du dernier élément de la liste pour les connexions interface de commande

    NTSVC_PARAMETER_DEF  * psParams;    // Liste des paramètres du service

    ROUTESVC_PARAMS sParmWork;
    ROUTESVC_PARAMS sParmCopy;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
ROUTESVC_WORK;


// ----------------- VARIABLES GLOBALES ----------------------

// Instantiation des données de travail du service
PROTECTED ROUTESVC_WORK  gsSvcWork INIT(0);

// Nom du service, utilisé par NTSVCExternalQueryInfo
char           gszServiceName[MAX_PATH]
#ifdef LOC_DEF
 = ROUTESVC_SERVICE_NAME
#endif
;

#endif
