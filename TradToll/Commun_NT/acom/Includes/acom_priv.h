/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_priv.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Définition des types et macros globaux privés, non exportés
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_PRIV_H
#define ACOM_PRIV_H

#include <protect.h>

// MACROS DE DEFINITION DES CONSTANTES

// Intervalle autorisé pour le nombre d'instances dans un working set
#define ACOM_MIN_WKS_INSTANCES              (1)
#define ACOM_MAX_WKS_INSTANCES              (128)

// Intervalle autorisé pour le nombre de workers dans un working set
#define ACOM_MIN_WORKERS                    (1)
#define ACOM_MAX_WORKERS                    (128)

// Etats des structures ACOM_CONNECTION
#define ACOM_CONNECTION_STATE_FREE          (0x00010001)
#define ACOM_CONNECTION_STATE_INIT          (0x00010002)
#define ACOM_CONNECTION_STATE_CONNECTING    (0x00010003)
#define ACOM_CONNECTION_STATE_CONNECTED     (0x00010004)
#define ACOM_CONNECTION_STATE_DISCONNECTED  (0x00010005)
#define ACOM_CONNECTION_STATE_ERROR         (0x00010006)
#define ACOM_CONNECTION_STATE_SHUTDOWN      (0x00010007)

// Type de la sur-structure ACOM_OVERLAPPED
#define ACOM_IO_TYPE_INPUT                  (0x00020001)
#define ACOM_IO_TYPE_OUTPUT                 (0x00020002)
#define ACOM_IO_TYPE_CONNECTION             (0x00020003)

// Type des blocks de commande
#define ACOM_COMMAND_SEND                   (0x00030001)
#define ACOM_COMMAND_DISCONNECT             (0x00030002)
#define ACOM_COMMAND_LIFE                   (0x00030003)

// Etats des structures ACOM_INSTANCE
#define ACOM_INSTANCE_FREE                  (0x00040000)

#define ACOM_INSTANCE_PIPE_SERVER           (0x00040101)
#define ACOM_INSTANCE_PIPE_CLIENT           (0x00040102)
#define ACOM_INSTANCE_PIPE_GROUP            (0x00040100)

#define ACOM_INSTANCE_MAIL_SERVER           (0x00040204)
#define ACOM_INSTANCE_MAIL_CLIENT           (0x00040205)
#define ACOM_INSTANCE_MAIL_GROUP            (0x00040200)

#define ACOM_INSTANCE_TCP_SERVER            (0x00040401)
#define ACOM_INSTANCE_TCP_CLIENT            (0x00040402)
#define ACOM_INSTANCE_TCP_GROUP             (0x00040400)

#define ACOM_INSTANCE_SERIAL                (0x00040801)
#define ACOM_INSTANCE_SERIAL_GROUP          (0x00040800)

// Mode d'utilisation
#define ACOM_MODE_MIN                       (0x00050001)
#define ACOM_MODE_CALLBACK                  (0x00050001)
#define ACOM_MODE_WM                        (0x00050002)
#define ACOM_MODE_TQ                        (0x00050003)
#define ACOM_MODE_MAX                       (0x00050003)

// Manipulation des handles
#define ACOM_CLOSE_HANDLE(pcnx) ( ( ( ( (pcnx)->psInst->dwType & ACOM_INSTANCE_TCP_GROUP ) == ACOM_INSTANCE_TCP_GROUP ) ? closesocket( (SOCKET)pcnx->hFile ) : CloseHandle( pcnx->hFile ) ), ( pcnx->hFile = NULL ) )

// TYPES DE DONNEES / STRUCTURES

// Prédéfinition des types à cause des cross-references
typedef struct ACOM_PARAM_PIPE_SERVER;
typedef struct ACOM_PARAM_PIPE_CLIENT;
typedef struct ACOM_PARAM_MAIL_SERVER;
typedef struct ACOM_PARAM_MAIL_CLIENT;
typedef struct ACOM_PARAM_TCP_SERVER;
typedef struct ACOM_PARAM_TCP_CLIENT;
typedef struct ACOM_PARAM;
typedef struct ACOM_BLOCK;
typedef struct ACOM_QUEUE;
typedef struct ACOM_OVERLAPPED;
typedef struct ACOM_TIMER_DATA;
typedef struct _ACOM_CONNECTION;
typedef struct ACOM_INSTANCE;
typedef struct ACOM_WORKING_SET;

// Paramètres de création d'un instance de serveur de pipe
typedef struct ACOM_PARAM_PIPE_SERVER
{
    DWORD                           dwMaxConnections;
    DWORD                           dwOutBufferSize;
    DWORD                           dwInBufferSize;
    DWORD                           dwQueueSize;
    DWORD                           dwMaxMessageSize;
}
ACOM_PARAM_PIPE_SERVER;

// Paramètres de création d'un instance de client de pipe
typedef struct ACOM_PARAM_PIPE_CLIENT
{
    DWORD dwQueueSize;
    DWORD dwMaxMessageSize;
}
ACOM_PARAM_PIPE_CLIENT;

// Paramètres de création d'un instance de serveur de mailslot
typedef struct ACOM_PARAM_MAIL_SERVER
{
    DWORD                           dwMaxMessageSize;
}
ACOM_PARAM_MAIL_SERVER;

// Paramètres de création d'un instance de client de mailslot
typedef struct ACOM_PARAM_MAIL_CLIENT
{
    DWORD                           dwQueueSize;
}
ACOM_PARAM_MAIL_CLIENT;

// Paramètres de création d'un instance de serveur TCP
typedef struct ACOM_PARAM_TCP_SERVER
{
    DWORD                           dwMaxConnections;
    DWORD                           dwOutBufferSize;
    DWORD                           dwInBufferSize;
    DWORD                           dwQueueSize;
    DWORD                           dwMaxMessageSize;
    WORD                            wPort;
}
ACOM_PARAM_TCP_SERVER;

// Paramètres de création d'un instance de client TCP
typedef struct ACOM_PARAM_TCP_CLIENT
{
    DWORD                           dwOutBufferSize;
    DWORD                           dwInBufferSize;
    DWORD                           dwQueueSize;
    DWORD                           dwMaxMessageSize;
    WORD                            wPort;
}
ACOM_PARAM_TCP_CLIENT;

// Paramètres de création d'un instance de liaison série
typedef struct ACOM_PARAM_SERIAL
{
    DWORD                           dwOutBufferSize;
    DWORD                           dwInBufferSize;
    DWORD                           dwQueueSize;
    DWORD                           dwMaxMessageSize;
    DWORD                           dwPort;
    DWORD                           dwReadInterval;
}
ACOM_PARAM_SERIAL;

// Union des paramètres de définition d'une instance
typedef struct ACOM_PARAM
{
    DWORD                           dwUseMode;
    HANDLE                          hObjEvent;

    ACOM_CALLBACK_CONNECTION      * pfCnx;
    ACOM_CALLBACK_DISCONNECTION   * pfDcnx;
    ACOM_CALLBACK_RECEIVED        * pfRecv;
    ACOM_CALLBACK_SENT            * pfSent;
    ACOM_CALLBACK_TIME_SHIFT      * pfTime;

    union
    {
    struct ACOM_PARAM_PIPE_SERVER   sPipeServer;
    struct ACOM_PARAM_PIPE_CLIENT   sPipeClient;
    struct ACOM_PARAM_MAIL_SERVER   sMailServer;
    struct ACOM_PARAM_MAIL_CLIENT   sMailClient;
    struct ACOM_PARAM_TCP_SERVER    sTcpServer;
    struct ACOM_PARAM_TCP_CLIENT    sTcpClient;
    struct ACOM_PARAM_SERIAL        sSerial;
    };
}
ACOM_PARAM;

// Bloc de commande (passant par l'i/o completion queue et
// dans la file des messages sortants
typedef struct ACOM_BLOCK
{
    DWORD                           dwBlockType;

    struct _ACOM_CONNECTION        * psCnx;
    ACOM_CNX_HANDLE                 hCnxHandle;

	DWORD64                         dwMsgUsrKey;
    BOOL                            bDisable;
	DWORD64                           dwCnxUsrKey;

    union
    {
        DWORD                       dwDataSize;
        DWORD                       tdwDataBlock[1];
    };
}
ACOM_BLOCK;

// Structure de gestion d'une fifo en liste circulaire. Plutot
// que de faire pointer tsQueue sur un tableau alloué, le tableau
// est directement intégré en fin de structure, il suffit alors 
// d'ajuster la taille allouée pour la structure pour ajuster la
// taille du tableau
// Attention : tsQueue doit forcément être en fin de structure
typedef struct ACOM_QUEUE
{
    DWORD                           dwQueueItems;
    DWORD                           dwFirst;
    DWORD                           dwLast;
    ACOM_BLOCK                    * tsQueue[1];
}
ACOM_QUEUE;

// Surtypage de la structure overlapped pour savoir à quoi on
// a affaire lors de l'arrivée d'un i/o completion packet.
// Attention : sOver doit forcément être en début de structure
typedef struct ACOM_OVERLAPPED
{
    OVERLAPPED                      sOver;
    DWORD                           dwType;
    struct _ACOM_CONNECTION        * psCnx;
}
ACOM_OVERLAPPED;


typedef struct ACOM_TIMER_DATA
{
    struct ACOM_TIMER_DATA    * psNext;
    DWORD                       dwId;
    HANDLE                      hCompletion;
    struct _ACOM_CONNECTION    * psCnx;
    ACOM_CNX_HANDLE             hCnx;
    DWORD                       dwType;
    HANDLE                      hTimer;
    LARGE_INTEGER               sDueTime;
}
ACOM_TIMER_DATA;

typedef DWORD ACOM_TIMER_ID;


// Structure de définition d'une connexion.
// Ces structure sont préallouée en fonction du nombre
// maxi de connexion que peut supporter un working set.
typedef struct _ACOM_CONNECTION
{
    DWORD                           dwIndex;
    struct ACOM_INSTANCE          * psInst;
    struct ACOM_WORKING_SET       * psWks;

    DWORD                           dwState;
    DWORD                           dwError;
    BOOL                            bEnabled;

    DWORD                           dwLifeCount;
    ACOM_TIMER_ID                   dwTimerId;

    DWORD                           dwReadBufferSize;
    DWORD                           dwLastConnectionTry;
    ACOM_CNX_HANDLE                 hCnxHandle;
	DWORD64                         dwCnxUsrKey;
	HANDLE                          hFile;
    BOOL                            bIsOverlapped;

    LONGLONG                        llLastRemoteDate;
    LONGLONG                        llLastLocalDate;
    BOOL                            bTimeShiftSignaled;

    struct ACOM_QUEUE             * psWriteQueue;
    struct ACOM_BLOCK             * psWriteCurrent;
    BYTE                          * pbReadBuffer;
    DWORD                           dwReadPos;

    struct ACOM_OVERLAPPED          sOverIn;
    struct ACOM_OVERLAPPED          sOverOut;
    struct ACOM_OVERLAPPED          sOverCnx;

    ULONGLONG                       ullCnxDate;
    ULONGLONG                       ullInBytes;
    ULONGLONG                       ullOutBytes;
    ULONGLONG                       ullInMsg;
    ULONGLONG                       ullOutMsg;

    char                            szIdentity[256];
 
    CRITICAL_SECTION                sCnxCritical;
}
ACOM_CONNECTION;

// Structure de définition d'une instance.
// Ces structure sont préallouée en fonction du nombre
// maxi d'instance que peut supporter un working set.
typedef struct ACOM_INSTANCE
{
    DWORD                           dwIndex;
    struct ACOM_WORKING_SET       * psWks;

    DWORD                           dwType;
    BOOL                            bHasInput;
    BOOL                            bHasOutput;
    char                            szFileName[MAX_PATH];
    ACOM_PARAM                      sParams;
    DWORD                           dwTimeToReconnect;

	DWORD64                         dwInstUsrKey;

    DWORD                           dwUseMode;
    HANDLE                          hObjEvent;
    SOCKET                          hListener;
    ACOM_CALLBACK_CONNECTION      * pfCnx;
    ACOM_CALLBACK_RECEIVED        * pfRecv;
    ACOM_CALLBACK_SENT            * pfSent;
    ACOM_CALLBACK_DISCONNECTION   * pfDcnx;
    ACOM_CALLBACK_TIME_SHIFT      * pfTime;
  
    CRITICAL_SECTION                sInstCritical;
}
ACOM_INSTANCE;

// Structure de définition d'un working set.
// Il fait référence à la fois au tableau des instances
// et au tableau des connexions
typedef struct ACOM_WORKING_SET
{
    BOOL                            bRunning;
    BOOL                            bShutDown;

    DWORD                           dwWmAComEvent;
	DWORD64                         dwWksUsrKey;

    DWORD                           dwProtocol;
    DWORD                           dwLifeTime;
    DWORD                           dwMaxLife;
    DWORD                           dwMaxInstances;
    DWORD                           dwMaxConnections;
    DWORD                           dwWorkers;
    DWORD                           dwPriority;
    DWORD                           dwConnectLoopDelay;
    LONGLONG                        llAllowedTimeShift;

    SECURITY_ATTRIBUTES *           psSA;

    DWORD                           dwWorkerConcurrency;
    DWORD                           dwMaxWorkerConcurrency;
    DWORD                           dwCnxWorker;
    DWORD                           dwInstances;
    DWORD                           dwConnections;
    HANDLE                          hCompletion;
    DWORD                           dwNextKey;

    HANDLE                        * phWorkerThreads;
    struct _ACOM_CONNECTION        * psCnx;
    struct ACOM_INSTANCE          * psInst;

    DWORD                           dwUseMode;
    HANDLE                          hObjEvent;
    ACOM_CALLBACK_SHUTDOWN        * pfShut;

    CRITICAL_SECTION                sWksCritical;
    CRITICAL_SECTION                sIoCritical;
}
ACOM_WORKING_SET;

typedef struct ACOM_EVT_CONNECTION
{
	DWORD64                         dwInstUsrKey;
    ACOM_CNX_HANDLE                 hCnxHandle;
}
ACOM_EVT_CONNECTION;

typedef struct ACOM_EVT_DISCONNECTION
{
	DWORD64                         dwInstUsrKey;
	DWORD64                         dwCnxUsrKey;
}
ACOM_EVT_DISCONNECTION;


typedef struct ACOM_EVT_RECEIVED
{
	DWORD64                         dwInstUsrKey;
	DWORD64                         dwCnxUsrKey;
    DWORD                           tdwData[1];
}
ACOM_EVT_RECEIVED;

typedef struct ACOM_EVT_SENT
{
	DWORD64                         dwInstUsrKey;
	DWORD64                         dwCnxUsrKey;
	DWORD64                         dwMsgUsrKey;
    DWORD                           dwError;
}
ACOM_EVT_SENT;

typedef struct ACOM_EVT_TIME_SHIFT
{
	DWORD64                           dwInstUsrKey;
	DWORD64                           dwCnxUsrKey;
}
ACOM_EVT_TIME_SHIFT;

typedef struct ACOM_EVT_SHUTDOWN
{
	DWORD64                           dwWksUsrKey;
    DWORD                           dwError;
}
ACOM_EVT_SHUTDOWN;


// SPECIFIQUE AU PROTOCOLE LIFE

#define ACOM_LIFE_DATA 0
#define ACOM_LIFE_REQ  1
#define ACOM_LIFE_RSP  2

#pragma pack( push, ACOM_LIFE_HEADER_PACK )
#pragma pack( 1 )
typedef struct ACOM_LIFE_HEADER
{
    DWORD    dwType;
    LONGLONG llDate;
    DWORD    dwDataSize;
}
ACOM_LIFE_HEADER;
#pragma pack( pop, ACOM_LIFE_HEADER_PACK )

PROTECTED DWORD             gdwWmAComEvent      INIT(0);
PROTECTED CRITICAL_SECTION  gsGlobalProtect;


#endif


/* -------------  FIN DU FICHIER : acom_priv.h ------------- */ 
