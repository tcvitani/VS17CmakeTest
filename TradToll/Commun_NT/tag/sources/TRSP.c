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
 * DESCRIPTION: Implémentation de la couche 7 de premier niveau
 *              pour l'interface Hôte<->Communicateur.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include "csr_tag_trsp.h"

#include <memclass.h>




//
// Vérification des pointeurs et de leurs valeurs
//
#define CHK_PTR(x)      if ( (x) == NULL ) __leave; else if ( ! TRSP_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)      if ( ! TRSP_IS_VALID(x) ) __leave;




//
// Manipulation des buffers
//
#define GET_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { (v) = *(BYTE*)&pbBuf[dwPos] ; dwPos += sizeof(BYTE); }
#define GET_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( (v), pbBuf+dwPos, (s) ) ; dwPos += (s); }
#define ADD_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { *(BYTE*)&pbBuf[dwPos]  = (BYTE)(v); dwPos += sizeof(BYTE); }
#define ADD_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( pbBuf+dwPos, (v), (s) ); dwPos += (s); }




typedef struct _TRSP_INSTANCE
{
    DWORD       dwDummy;
}
    TRSP_INSTANCE;




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
EXPORT TRSP_INSTANCE * WINAPI TRSPOpen()
{
    TRSP_INSTANCE * psInst;

    psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );

    return psInst;
}




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
        IN OUT  TRSP_INSTANCE         * psInst )
{
    HeapFree( GetProcessHeap(), 0, psInst );
}




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
        IN      TRSP_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      TRSP_API         *  psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // En fonction de l'action demander, se brancher sur le
        // bon traitement.
        //
        switch( psDef->bActionType )
        {

        case TRSP_ACTION_SET_MODE :

            ADD_BYTE( psDef->bActionType );
            ADD_BYTE( psDef->sSetModeReq.bMode );
            break;

        case TRSP_ACTION_GET_STATUS :

            ADD_BYTE( psDef->bActionType );
            break;

        case TRSP_ACTION_GET_COUNTERS :

            ADD_BYTE( psDef->bActionType );
            break;

        case TRSP_ACTION_UPLOAD :

            ADD_BYTE( psDef->bActionType );
            ADD_BYTES( sizeof(psDef->sUploadReq.tbIdentifier), psDef->sUploadReq.tbIdentifier );
            break;

        case TRSP_ACTION_SET_CNF :

            ADD_BYTE( psDef->bActionType );
            ADD_BYTE( psDef->sSetCnfReq.bMode );
            ADD_BYTE( psDef->sSetCnfReq.bFrequency );
            ADD_BYTE( psDef->sSetCnfReq.bBaudRate );
            break;

        case TRSP_ACTION_GET_CNF :

            ADD_BYTE( psDef->bActionType );
            break;

        case TRSP_ACTION_TRSP_INI :
        case TRSP_ACTION_TRSP_CMD :
        case TRSP_ACTION_TRSP_END :

            ADD_BYTE( psDef->bActionType );
            ADD_BYTES( psDef->sTrspReq.dwReqSize, psDef->sTrspReq.tbReq );
            break;

        case TRSP_ACTION_BEACON_OFF :

            ADD_BYTE( psDef->bActionType );
            break;

        default:
            __leave;
        }

        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwPos;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




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
        OUT     TRSP_API       * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;
    DWORD           dwIndex;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // Le premier octet donne le type de l'action
        //
        GET_BYTE( psDef->bActionType );

        //
        // En fonction de l'action à analyser, se brancher sur le
        // bon traitement.
        //
        switch ( psDef->bActionType )
        {

        case TRSP_ACTION_SET_MODE :
            
            GET_BYTE( psDef->sSetModeRsp.bError );
            break;

        case TRSP_ACTION_GET_STATUS :

            GET_BYTE( psDef->sGetStatusRsp.bError );
            GET_BYTE( psDef->sGetStatusRsp.bMode );
            GET_BYTE( psDef->sGetStatusRsp.bTransacting );
            break;

        case TRSP_ACTION_GET_COUNTERS :

            GET_BYTES( sizeof(psDef->sGetCountersRsp.tbCounters), psDef->sGetCountersRsp.tbCounters );
            break;

        case TRSP_ACTION_UPLOAD :

            GET_BYTE( psDef->sUploadRsp.bError );
            break;

        case TRSP_ACTION_SET_CNF :

            GET_BYTE( psDef->sSetCnfRsp.bError );
            break;

        case TRSP_ACTION_GET_CNF :

            GET_BYTE( psDef->sGetCnfRsp.bError );

            for ( dwIndex = 0 ; dwIndex < sizeof(psDef->sGetCnfRsp.szVersion) ; dwIndex ++ )
            {
                GET_BYTE( psDef->sGetCnfRsp.szVersion[dwIndex] );
                if ( psDef->sGetCnfRsp.szVersion[dwIndex] == 0 )
                    break;
            }
            if ( dwIndex >= sizeof(psDef->sGetCnfRsp.szVersion) )
                psDef->sGetCnfRsp.szVersion[sizeof(psDef->sGetCnfRsp.szVersion)-1] = 0;

            GET_BYTES( sizeof(psDef->sGetCnfRsp.tbParams) , psDef->sGetCnfRsp.tbParams );
            break;

        case TRSP_ACTION_TRSP_INI :
        case TRSP_ACTION_TRSP_CMD :
        case TRSP_ACTION_TRSP_END :

            GET_BYTE( psDef->sTrspRsp.bError );
            psDef->sTrspRsp.dwRspSize = dwBufLen - dwPos;
            GET_BYTES( psDef->sTrspRsp.dwRspSize, psDef->sTrspRsp.tbRsp );
            break;

        case TRSP_ACTION_BEACON_OFF :

            GET_BYTE( psDef->sBeaconOffRsp.bError );
            break;

        default:
            __leave;
        }
        
        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwBufLen;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}

