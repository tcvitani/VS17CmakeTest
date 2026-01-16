/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_glob.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Définitions globales au projet
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_GLOB_H
#define CM_GLOB_H

#include <protect.h>

#include "..\resources\version.h"
#include "commoc.h"




//
// DEFINITION DES MACROS
//

#define CM_SERVICE_NAME       RESINFO_PRODUCT

#define CM_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define CM_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define CM_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define CM_PROMPT_COMMENTS    RESINFO_FILEDESC

#define CM_WORKINGSET_TIEMOUT 5000

#define CM_PIPE_CMD           "\\\\.\\pipe\\" CM_SERVICE_NAME "\\CMD"

#define CM_REG_ROOT           HKEY_LOCAL_MACHINE
#define CM_REG_KEY            NTSVC_REG_KEY_SERVICE "\\" CM_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM
#define CM_REG_KEY_HOSTS      NTSVC_REG_KEY_SERVICE "\\" CM_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\HOSTS"
#define CM_REG_KEY_LINKS      NTSVC_REG_KEY_SERVICE "\\" CM_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\LINKS"
#define CM_REG_KEY_WORKERS    NTSVC_REG_KEY_SERVICE "\\" CM_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM "\\WORKERS"

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


#define CM_MAX_HOSTS           64
#define CM_MAX_LINKS           64
#define CM_MAX_WORKERS         64

#define CM_LIST_INCREMENT      32

#define CM_INVALID             0xFFFFFFFF
#define CM_EXISTS              1
#define CM_NOT_FOUND           0

#define CM_STATE_ERROR         0
#define CM_STATE_SYNCHRONIZED  1

#define CM_STRNCPY(x,y,n)      (strncpy(x,y,n),(x)[n-1]=0,x)
#define CM_STRNCAT(x,y,n)      (strncat(x,y,n),(x)[n-1]=0,x)


// Definition des constantes associées à la gestion du menu
#define MAX_CUSTOM_MENU_ITEMS   8
#define CUSTOM_ITEM_BASE_ID     0x8000

// Définition des critères de tri des fichiers

#define CM_SORT_CRITERIA    0x0000FFFF
#define CM_SORT_NAME        0x00000000
#define CM_SORT_DATE        0x00000001
#define CM_SORT_NUMBER      0x00000002

#define CM_SORT_ORDER       0xFFFF0000
#define CM_SORT_INCREASING  0x00000000
#define CM_SORT_DECREASING  0x00010000


#define SORT_MIN_FILE_NAME  ((char*)(0))
#define SORT_MAX_FILE_NAME  ((char*)(-1))


//
// DEFINITION DES STRUCTURES ET AUTRES TYPES DE DONNEES
//


typedef struct CM_SORT
{
    DWORD     dwMethod;

    BOOL      fLoop;

    ULONGLONG ullNbrFirstQuarter;
    ULONGLONG ullNbrLastQuarter;
    ULONGLONG ullNbrMax;
    char      szNbrMask[CM_MAX_STR];

    // Bornes de tri évaluées à chaque rafraissement
    BOOL      fFirstQuarter;
    BOOL      fMiddle;
    BOOL      fLastQuarter;

}
    CM_SORT;



//
// Paramètres de fonctionnement courant du service
//
typedef struct CM_PARAMS
{
    // Flag indiquant que le service dispose d'une interface de commande
#define CM_REG_VAL_CMDSVC           "CmdSvcEnabled"
    DWORD                           dwCmdSvc;

    // Nombre maximum de connexions sur l'interface de commande
#define CM_REG_VAL_MAXCMDCNX        "MaxCmdCnx"
    DWORD                           dwMaxCmdCnx;

    // Workers ACOM
#define CM_REG_VAL_WORKERS          "Workers"
    DWORD                           dwWorkers;

    // Délai de reconnexion pour les communications
#define CM_REG_VAL_RECONNECTTIME    "ReconnectTime"
    DWORD                           dwTimeToReconnect;

    // Taille de la file d'attente interne des cannaux de communication
#define CM_REG_VAL_QUEUESIZE        "QueueSize"
    DWORD                           dwMaxQueuedMessages;

    // Taille maximum d'un message dans un canal de communication
#define CM_REG_VAL_MAXMSGSIZE       "MaxMsgSize"
    DWORD                           dwMaxMsgSize;

    // Taille du buffer alloué un canal de communication
#define CM_REG_VAL_BUFFERSIZE       "BufferSize"
    DWORD                           dwPipeBufferSize;

    // Nombre maximum de réémission du message de vie sur un canal de communication
#define CM_REG_VAL_MAXLIFE          "MaxLife"
    DWORD                           dwMaxLife;

    // Délai d'émission du message de vie sur un canal de communication
#define CM_REG_VAL_LIFETIME         "LifeTime"
    DWORD                           dwLifeTime;
}
    CM_PARAMS;




//
// Part of a file list used for sorting
//
typedef struct CM_FILE
{
    char                szName[CM_MAX_STR];
    ULONGLONG           ullSize;
    ULONGLONG           ullLastWrite;
    ULONGLONG           ullScore;
    DWORD               dwAttr;
}
    CM_FILE;





//
// Structure de gestion d'un host (connexion distante).
//
typedef struct CM_HOST
{
    // Nom du host
    char                szName[CM_MAX_STR];

    // Nom de la DLL de gestion de cet host. Chaine vide,
    // pour utiliser les fonctions standard de copie sans
    // connexion / déconnexionun (cas de l'host local et
    // des échanges par LAN Manager sur un LAN).
    char                szDll[CM_MAX_STR];

    // Indique si l'host est local ou nom
    BOOL                bLocal;
    
    // Handle de la dll
    HINSTANCE           hDll;

    // Handle de la connexion retourné par l'appel à HostOpen (pfOpen)
    HANDLE              hCnx;

    // Pointeur sur les fonctions de la DLL
    CM_FCNX_OPEN      * pfOpen;
    CM_FCNX_STATUS    * pfStatus;
    CM_FCNX_CLOSE     * pfClose;
    CM_FCNX_PUT       * pfPut;
    CM_FCNX_GET       * pfGet;
    CM_FCNX_MOVE      * pfMove;
    CM_FCNX_ENUM      * pfEnum;
    CM_FCNX_FREE      * pfFree;

    // Section critique de protection contre les accés concurents
    CRITICAL_SECTION    sProtect;
}
    CM_HOST;




//
// Structure de gestion d'un répertoire chargée de maintenir
// l'image du contenu d'un répertoire en mémoire pour minimiser
// les i/o d'énumération.
//
typedef struct CM_DIR
{
    // Chemin du répertoire exprimé dans la syntaxe de l'host
    // par l'intermédiaire duquel le répertoire est joignable. La seule
    // contrainte sur la syntaxe est l'utilisation du caractère
    // "\" comme séparateur de répertoire.
    char                szPath[CM_MAX_STR];

    // Chemin complet du masque à appliquer pour énumérer les fichier
    // du répertoire dans la syntaxe de l'host par l'intermédiaire duquel
    // le répertoire est joignable. 
    char                szMask[CM_MAX_STR];

    // Ordre de tri des fichiers
    CM_SORT             sSort;

    // Etat courant du répertoire.
    //  - CM_STATE_ERROR : La structure est offline par rapport au répertoire
    //  - CM_STATE_SYNCHRONIZED : La structure possède une image synchonisée du répertoire
    //  au delta prés des modifications externes au services.
    DWORD               dwState;

    // Compte des changements subits par le repertoire depuis la dernière synchronisation?
    DWORD               dwChanges;

    // Nombre maximum de fichiers pouvant être stocké dans le tableau actuellement alloué
    DWORD               dwMaxFiles;

    // Nombre de fichiers actuellement stockés dans le tableau.
    DWORD               dwFiles;

    // Tableau des fichiers
    CM_FILE           * psFiles;

    // Host permettant d'accéder à ce repertoire
    CM_HOST           * psHost;

    // Section critique de protection contre les accés concurents
    CRITICAL_SECTION    sProtect;
}
    CM_DIR;




//
// Structure de gestion d'un lien commoc. Un lien étant l'ensemble
// des resources permettant de mettre en oeuvre des transferts de fichier
// d'un point vers un autre dans un sens et/ou dans l'autre.
//
typedef struct CM_LINK
{
    // Nom du lien commoc
    char                szName[CM_MAX_STR];

    // Périodicité de scrutation
    DWORD               dwPollingPeriod;

    // Defini le comportement sur non traitement de fichier
    DWORD               dwEnsureOrder;

    // Masque de sélection des fichiers
    char                szNameMask[CM_MAX_STR];

    // Host gérant la partie distante
    CM_HOST           * psHost;

    // Ce flag indique qu'on effectue le dispatching en flux montant
    // ainsi que les transferts montant (upload).
    // Il est incompatible avec le flag bDispatch.
    BOOL                bExport;

    // Flag d'usage général
    BOOL                fFlag;

    // Ce flag indique qu'on effectue uniquement le dispatching en flux
    // montant (upload).
    // Il est incompatible avec le flag bExport.
    BOOL                bDispatch;

    // Répertoires mis en jeux pour le flux montant
    CM_DIR              sDirEAB;
    CM_DIR              sDirEABS;
    CM_DIR              sDirRBA;
    CM_DIR              sDirRBAT;
    CM_DIR              sDirRBAS;

    // Ce flag indique qu'on effectue les transfert en flux déscendant (download)
    BOOL                bImport;

    // Ce flag indique qu'on effectue les transfert en flux déscendant (download) en mode acquité
    BOOL                bImportAck;

    // Répertoires mis en jeux pour le flux déscendant
    CM_DIR              sDirRAB;
    CM_DIR              sDirRABS;
    CM_DIR              sDirRABT;
    CM_DIR              sDirEBA;
    CM_DIR              sDirEBAS;
}
    CM_LINK;





//
// Structure de gestion d'un worker.
//
typedef struct CM_WORKER
{
    // Nom du worker
    char                szName[CM_MAX_STR];

    // Nom de la DLL de gestion de ce worker. Chaine vide,
    // pour utiliser les fonctions standard traitement
    // (lancement d'une commande)
    char                szDll[CM_MAX_STR];

    // Périodicité de scrutation
    DWORD               dwPollingPeriod;

    // Comportement sur présence dans répertoire d'acquittement
    // et dans répertoire mail-in
    DWORD               dwInAndAckDelete;

    // Comportement sur présence dans répertoire d'erreur
    // et dans répertoire mail-in
    DWORD               dwInAndErrDelete;

    // Max acceptée pour les répertoire d'acquittement et d'erreurs
    DWORD               dwMaxAckFiles;
    DWORD               dwMaxAckKB;
    DWORD               dwMaxErrFiles;
    DWORD               dwMaxErrKB;

    // Defini le comportement sur non traitement de fichier
    DWORD               dwEnsureOrder;

    // Handle de la dll
    HINSTANCE           hDll;

    // Handle du worker retourné par l'appel à WorkerOpen (pfOpen)
    HANDLE              hWrk;

    // Pointeur sur les fonctions de la DLL
    CM_WORKER_OPEN    * pfOpen;
    CM_WORKER_CLOSE   * pfClose;
    CM_WORKER_PROCESS * pfProcess;
    CM_WORKER_PROCESS_EX * pfProcessEx;

    // Masque de sélection des fichiers
    char                szNameMask[CM_MAX_STR];

    // Répertoires de travail du worker
    CM_DIR              sDirRAB;
    CM_DIR              sDirRABS;
    CM_DIR              sDirRABX;
    BOOL                fSXIdentical;

    // Section critique de protection contre les accés concurents
    CRITICAL_SECTION    sProtect;
}
    CM_WORKER;




//
// Définition de la structure de travail du service
//
typedef struct CM_WORK
{
    // Flag indiquant que le programme est lancé en mode debug et non
    // comme un service.
    BOOL                    bIsDebug;

    // Flag indiquant que le service, lors d'une demande d'arret
    // doit être rechargé et non arrété
    BOOL                    bReload;

    // Flag de pause des traitements
    BOOL                    fPaused;

    // TLS Slot
    DWORD                   dwTLS;

    // Icones utilisés pour le bureau
    HICON                   hIconOk[15];
    HICON                   hIconPaused[2];
    HICON                   hIconKo;

    // Text localisé des traces d'erreur
    HCOLLECTION             hText;

    // Handle du working set utilisé pour les canaux de communication
    ACOM_WKS_HANDLE         hWks;
    
    // Instance des canaux de communication de l'interface de commande
    ACOM_INST_HANDLE        hCmdInst;

    // Définition des paramètres de fonctionnement courant du service
    NTSVC_PARAMETER_DEF   * psParams;

    // Tableau des hosts
    DWORD                   dwHosts;
    CM_HOST                 tsHosts[ CM_MAX_HOSTS ];
    CM_HOST               * psLocalHost;

    // Tableau des liens commoc
    DWORD                   dwLinks;
    CM_LINK                 tsLinks[ CM_MAX_LINKS ];

    // Tableau des workers
    DWORD                   dwWorkers;
    CM_WORKER               tsWorkers[ CM_MAX_WORKERS ];

    // Gestion du répertoire de dispatching
    DWORD                   dwDispatchPeriod;
    char                    szNameMask[CM_MAX_STR];
    CM_DIR                  sDirEA;

    // Tableau des handles des thread de traitement
    DWORD                   dwThreads;
    HANDLE                  thThreads[ CM_MAX_LINKS + 1 + CM_MAX_WORKERS ];

    // Valeurs des paramètres courant de travail
    CM_PARAMS               sParmWork;

    // Copie des Valeurs des paramètres courant de travail
    // utilisées pour les commande ParamList, SetParam
    // et SaveParams
    CM_PARAMS               sParmCopy;

    DWORD   dwMenuCommands;
    char    tszMenuCommands[MAX_CUSTOM_MENU_ITEMS][1024];
    DWORD   tdwMenuFlags[MAX_CUSTOM_MENU_ITEMS];
}
    CM_WORK;



//
// VARIABLES GLOBALES
//

//
// Instanciation de la structure de gestion du service
//
PROTECTED CM_WORK   gsSvcWork INIT(0);




//
// Nom du service
//
PROTECTED char gszSvcName[MAX_PATH]
#ifdef LOC_DEF
 = CM_SERVICE_NAME
#endif
;



#endif
