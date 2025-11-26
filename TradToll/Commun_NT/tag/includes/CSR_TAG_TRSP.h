/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TRSP
 * FILE       : TSRP.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion du mode transparent
 * --------------------------------------------------------------------
 * DESCRIPTION: Définitions exportées
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_TRSP_H
#define CSR_TAG_TRSP_H

#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_TRSP_PACK, 1 )


//
// Test de validité d'indexes
//
#define TRSP_IS_VALID(x)         ((x)<0x80000000)
#define TRSP_INVALID             0xFFFFFFFF



//
// Actions gérées par le communicateur
//
typedef enum
{
    TRSP_ACTION_SET_MODE     = 0x00,
    TRSP_ACTION_GET_STATUS   = 0x01,
    TRSP_ACTION_GET_COUNTERS = 0x02,
    TRSP_ACTION_UPLOAD       = 0x14,
    TRSP_ACTION_SET_CNF      = 0x15,
    TRSP_ACTION_GET_CNF      = 0x16,
    TRSP_ACTION_TRSP_INI     = 0x03,
    TRSP_ACTION_TRSP_CMD     = 0x05,
    TRSP_ACTION_TRSP_END     = 0x06,
    TRSP_ACTION_BEACON_OFF   = 0x04,
}
    TRSP_ACTIONS;




//
// Mode de fonctionnement du communicateur
//
typedef enum
{
    TRSP_MODE_STOP           = 0x00,
    TRSP_MODE_TRANSPARENT    = 0x01,
    TRSP_MODE_RESERVED       = 0x02,
    TRSP_MODE_MAINTENANCE    = 0x03,
}
    TRSP_RUNNING_MODES;




//
// Codes d'erreur renvoyés par le communicateur
//
typedef enum
{
    TRSP_ERROR_SUCCESS       = 0x00,
    TRSP_ERROR_REFUSED       = 0x01,
    TRSP_ERROR_TRANSACT      = 0x02,
    TRSP_ERROR_BEACON        = 0x03,
    TRSP_ERROR_TIMEOUT       = 0x09,
    TRSP_ERROR_RESET         = 0x0A,
    TRSP_ERROR_INVALID_PARAM = 0x0B,
    TRSP_ERROR_CONFIG_FILE   = 0x0C,
}
    TRSP_ERROR_CODES;


//
// Modes de prise en compte des changements de configuration
//
typedef enum
{
    TRSP_UPDATE_IMMEDIATE    = 0x00,
    TRSP_UPDATE_ON_RESET     = 0x01,
}
    TRSP_UPDATE_MODES;




//
// Fréquences gérées par la balise
//
typedef enum
{
    TRSP_FREQUENCY_1         = 0x01,
    TRSP_FREQUENCY_2         = 0x02,
    TRSP_FREQUENCY_3         = 0x03,
    TRSP_FREQUENCY_4         = 0x04,
}
    TSRP_BEACON_FREQUENCIES;



//
// Débits de liaison gérés par le communicateur
//
typedef enum
{
     TRSP_BAUD_1200           = 0x00,
     TRSP_BAUD_2400           = 0x01,
     TRSP_BAUD_4800           = 0x02,
     TRSP_BAUD_9600           = 0x03,
     TRSP_BAUD_19200          = 0x04,
     TRSP_BAUD_38400          = 0x05,
     TRSP_BAUD_57600          = 0x06,
     TRSP_BAUD_115200         = 0x07,
}
    TRSP_BAUD_REATES;



//
// Constantes diverses
//
#define TRSP_EVE_COUNTERS        39
#define TRSP_PARAMETERS          28
#define TRSP_MAX_VERSION         64
#define TRSP_UPLOAD_ID           2
#define TRSP_MAX_DATAGRAM        256




typedef struct 
{
    BYTE        bMode;
}
    TRSP_SET_MODE_REQ;




typedef struct 
{
    BYTE        bError;
}
    TRSP_SET_MODE_RSP;




typedef struct 
{
    BYTE        tbDummy[];
}
    TRSP_GET_STATUS_REQ;




typedef struct 
{
    BYTE        bError;
    BYTE        bMode;
    BYTE        bTransacting;
}
    TRSP_GET_STATUS_RSP;




typedef struct 
{
    BYTE        tbDummy[];
}
    TRSP_GET_COUNTERS_REQ;




typedef struct 
{
    BYTE        tbCounters[TRSP_EVE_COUNTERS];
}
    TRSP_GET_COUNTERS_RSP;




typedef struct 
{
    BYTE        tbIdentifier[TRSP_UPLOAD_ID];
}
    TRSP_UPLOAD_REQ;



    
typedef struct 
{
    BYTE        bError;
}
    TRSP_UPLOAD_RSP;




typedef struct 
{
    BYTE        bMode;
    BYTE        bFrequency;
    BYTE        bBaudRate;
}
    TRSP_SET_CNF_REQ;




typedef struct 
{
    BYTE        bError;
}
    TRSP_SET_CNF_RSP;




typedef struct 
{
    BYTE        tbDummy[];
}
    TRSP_GET_CNF_REQ;




typedef struct 
{
    BYTE        bError;
    BYTE        tbParams[TRSP_PARAMETERS];
    char        szVersion[TRSP_MAX_VERSION];
}
    TRSP_GET_CNF_RSP;




typedef struct 
{
    DWORD       dwReqSize;
    BYTE        tbReq[TRSP_MAX_DATAGRAM];
}
    TRSP_TRSP_REQ;




typedef struct 
{
    BYTE        bError;
    DWORD       dwRspSize;
    BYTE        tbRsp[TRSP_MAX_DATAGRAM];
}
    TRSP_TRSP_RSP;



typedef struct 
{
    BYTE        tbDummy[];
}
    TRSP_BEACON_OFF_REQ;




typedef struct 
{
    BYTE        bError;
}
    TRSP_BEACON_OFF_RSP;




typedef struct 
{
    BYTE                bActionType;
    
    union
    {
        TRSP_SET_MODE_REQ        sSetModeReq;
        TRSP_SET_MODE_RSP        sSetModeRsp;
        TRSP_GET_STATUS_REQ      sGetStatusReq;
        TRSP_GET_STATUS_RSP      sGetStatusRsp;
        TRSP_GET_COUNTERS_REQ    sGetCountersReq;
        TRSP_GET_COUNTERS_RSP    sGetCountersRsp;
        TRSP_UPLOAD_REQ          sUploadReq;
        TRSP_UPLOAD_RSP          sUploadRsp;
        TRSP_SET_CNF_REQ         sSetCnfReq;
        TRSP_SET_CNF_RSP         sSetCnfRsp;
        TRSP_GET_CNF_REQ         sGetCnfReq;
        TRSP_GET_CNF_RSP         sGetCnfRsp;
        TRSP_TRSP_REQ            sTrspReq;
        TRSP_TRSP_RSP            sTrspRsp;
        TRSP_BEACON_OFF_REQ      sBeaconOffReq;
        TRSP_BEACON_OFF_RSP      sBeaconOffRsp;
    };
}
    TRSP_API;




typedef struct _TRSP_INSTANCE TRSP_INSTANCE;




// ------------------ PROTOTYPES EXPORTES ---------------------


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT TRSP_INSTANCE * WINAPI TRSPOpen()
 * PARAMETRES: Aucun
 * RETOURNE  : Un handle de la nouvelle instance créée ou NULL en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Mise en place d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT TRSP_INSTANCE * WINAPI TRSPOpen();




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI TRSPClose(
 *                      IN OUT  TRSP_INSTANCE         * psInst )
 * PARAMETRES: psInst : Handle d'instance créée avec TRSPOpen().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI TRSPClose(
        IN OUT  TRSP_INSTANCE   * psInst );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT BOOL WINAPI TRSPBuildMessage( 
 *                      IN      TRSP_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      TRSP_API         * psDef )
 * PARAMETRES: psInst        : Handle d'instance créée avec TRSPOpen().
 *             pbBuf         : Pointe sur le buffer destiné à recevoir le message généré
 *             pdwBufBytePos : En entrée, position du premiere octet du buffer
 *                      à utiliser. En sortie, position dans le buffer de l'octet
 *                      immédiatement aprés le message généré.
 *             pdwBufByteLen : En entrée, nombre d'octet disponibles dans le
 *                      buffer à partir de la position donnée.
 *             psDef         : Définition du message à générer.
 * RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à la génération.
 * --------------------------------------------------------------------
 * ROLE      : Construit un message à destination d'un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI TRSPBuildMessage( 
        IN      TRSP_INSTANCE   * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      TRSP_API       * psDef );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT BOOL WINAPI TRSPAnalyseMessage( 
 *                      IN      TRSP_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      OUT     TRSP_API         * psDef )
 * PARAMETRES: psInst        : Handle d'instance créée avec TRSPOpen().
 *             pbBuf         : Pointe sur le buffer contenant le message reçu
 *             pdwBufBytePos : En entrée, position du premiere octet du buffer
 *                      à utiliser. En sortie, position dans le buffer de l'octet
 *                      immédiatement aprés le message lu.
 *             pdwBufByteLen : En entrée, nombre d'octets disponibles dans le
 *                      buffer à partir de la position donnée.
 *             psDef         : Définition du message analysé.
 * RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à l'analyse.
 * --------------------------------------------------------------------
 * ROLE      : Analyse un message à reçu d'un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI TRSPAnalyseMessage( 
        IN      TRSP_INSTANCE   * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     TRSP_API       * psDef );


#pragma pack( pop, TAG_TRSP_PACK )


#endif
