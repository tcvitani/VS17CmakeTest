/* --------------------------------------------------------------------
 * (C) 2001 CS  - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CENL7
 * FILE       : CENL7BUILD.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Construction des messages couche 7 DSRC (norme CEN)
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <csr_bitbl.h>

#include "csr_tag_cenl7.h"

#include <memclass.h>





#define CHK_PTR(x)              if ( (x) == NULL ) __leave; else if ( ! BITBL_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)              if ( ! BITBL_IS_VALID(x) ) __leave;




#define BUILD_BOOL(v)              ((v)?1:0)

#define ADD_BITS(s,v)   if ( ! BITBLSetNumber( v, s, pbBuf, pdwBufBitPos, pdwBufBitLen ) ) __leave; else;

#define CPY_BITS(s,v)   if ( ! BITBLCopyToBuffer( pbBuf, pdwBufBitPos, pdwBufBitLen, v, 0, s ) ) __leave; else;





PRIVATE BOOL WINAPI _CENL7BuildHeader( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_API         * psDef );




PRIVATE BOOL WINAPI _CENL7BuildIniReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_INI_REQ     * psDef );




PRIVATE BOOL WINAPI _CENL7BuildSetReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_SET_REQ     * psDef );




PRIVATE BOOL WINAPI _CENL7BuildGetReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_GET_REQ     * psDef );



        
PRIVATE BOOL WINAPI _CENL7BuildActReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_ACT_REQ     * psDef );




PRIVATE BOOL WINAPI _CENL7BuildEvtReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_EVT_REQ     * psDef );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7BuildApdu( 
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      CENL7_API         * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer destiné à recevoir les données
 *             pdwBufBytePos : En entrée, position du premier octet à écrire dans le buffer,
 *                             En sortie, position de l'octet immmédiatement après le dernier écrit
 *             pdwBufByteLen : En entrée, taille du buffer disponible, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             psDef         : Définition du bloc à générer
 * RETURN    : TRUE si le bloc a été généré. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Génération d'un bloc CEN dans buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7BuildApdu( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      CENL7_API         * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufBitPos;
    DWORD           dwBufBitLen;

    __try
    {
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufBitPos = (*pdwBufBytePos) * 8;
        dwBufBitLen = (*pdwBufByteLen) * 8;

        switch( psDef->bBlockType )
        {
        case CENL7_APDU_INI_REQ :
            if ( ! _CENL7BuildHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) )
                __leave;
            if ( ! _CENL7BuildIniReq( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sIniReq ) )
                __leave;
            break;

        case CENL7_APDU_SET_REQ :
            if ( ! _CENL7BuildHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) )
                __leave;
            if ( ! _CENL7BuildSetReq( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sSetReq ) )
                __leave;
            break;

        case CENL7_APDU_GET_REQ :
            if ( ! _CENL7BuildHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) )
                __leave;
            if ( ! _CENL7BuildGetReq( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sGetReq ) )
                __leave;
            break;

        case CENL7_APDU_ACT_REQ :
            if ( ! _CENL7BuildHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) ) 
                __leave;
            if ( ! _CENL7BuildActReq( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sActReq ) )
                __leave;
            break;

        case CENL7_APDU_EVT_REQ :
            if ( ! _CENL7BuildHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) ) 
                __leave;
            if ( ! _CENL7BuildEvtReq( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sEvtReq ) )
                __leave;
            break;

        default:
            __leave;
        }

        (*pdwBufBytePos) = ( ( dwBufBitPos + 7 ) / 8 );
        (*pdwBufByteLen) = ( dwBufBitLen / 8 );

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7BuildActionParams(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      OUT     char              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      BYTE                bActionType,
 *                      IN      CENL7_ACT_PARAMS  * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer destiné à recevoir les données
 *             pdwBufBytePos : En entrée, position du premier octet à écrire dans le buffer,
 *                             En sortie, position de l'octet immmédiatement après le dernier écrit
 *             pdwBufByteLen : En entrée, taille du buffer disponible, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             bActionType   : Type d'action à traiter.
 *             psDef         : Définition des paramètres de l'action.
 * RETURN    : TRUE si le bloc a été généré. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Génération d'un bloc de paramètre d'action dans un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7BuildActionParams(
        IN      CENL7_INSTANCE    * psInst,
        OUT     char              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      BYTE                bActionType,
        IN      CENL7_ACT_PARAMS  * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
    DWORD           dwLen;
    BYTE            bLen;
    DWORD           dwBufBitPos;
    DWORD           dwBufBitLen;
    DWORD         * pdwBufBitPos = &dwBufBitPos;
    DWORD         * pdwBufBitLen = &dwBufBitLen;

    __try
    {
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        pbBuf += *pdwBufBytePos;
        dwBufBitPos = 0;
        dwBufBitLen = 8 * (*pdwBufByteLen);

        switch ( bActionType )
        {

        case CENL7_ACTCODE_SET_MMI :

            if ( ( psInst->dwFlags & CENL7_FLAG_INTEGER_ITERATION ) != 0 )
            {
            /* TYPE INTEGER          */ ADD_BITS (     8, CENL7_CHOICE_INTEGER                        )
            /* NB ITERATION          */ ADD_BITS (     8, 1                                           )
            /* Signal MMI            */ ADD_BITS (     8, psDef->sMmiReq.bMMISignal                   )
            }
            else
            {
            /* TYPE INTEGER          */ ADD_BITS (     8, CENL7_CHOICE_INTEGER                        )
            /* Signal MMI            */ ADD_BITS (     8, psDef->sMmiReq.bMMISignal                   )
            }

            break;

        case CENL7_ACTCODE_GET_NONCE :

            break;

        case CENL7_ACTCODE_GET_STAMPED :

            /* TYPE GETSTAMPEDRQ     */ ADD_BITS (     8, CENL7_CHOICE_GET_STAMPED_RQ                 )

            if ( psDef->sGStReq.bAttrIdCount > CENL7_MAX_ATTRS ) 
                __leave;
            /* ATTR ID COUNT         */ ADD_BITS (     8, psDef->sGStReq.bAttrIdCount                 )
            
            for ( bIndex = 0 ; bIndex < psDef->sGStReq.bAttrIdCount ; bIndex ++ )
            {
            /* ATTR ID(n)            */ ADD_BITS (     8, psDef->sGStReq.tbAttrIds[bIndex]            )
            }

            dwLen = (DWORD)psDef->sGStReq.bRndLen * 8;
            /* RNDRSE LENGTH         */ ADD_BITS (     8, psDef->sGStReq.bRndLen                      )
            /* RNDRSE DATA           */ CPY_BITS ( dwLen, psDef->sGStReq.tbRnd                        )

            /* KEY REFERENCE         */ ADD_BITS (     8, psDef->sGStReq.bKeyId                       )

            break;

        case CENL7_ACTCODE_ECHO :

            /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
            /* NB ITERATION          */ ADD_BITS (     8, psDef->sEchReq.bEchoLen                     )
            dwLen = (DWORD)psDef->sEchReq.bEchoLen * 8;
            /* DONNEE ECHO           */ CPY_BITS ( dwLen, psDef->sEchReq.tbEcho                       )

            break;

        case CENL7_ACTCODE_TRANSFER_CHANNEL :
        case CENL7_ACTCODE_PRIVATE_TRF_CHN :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) != 0 )
                __leave;

//          if ( ( psInst->dwFlags & CENL7_FLAG_QF_PRIVATE ) != 0 )
//          {
//          /* CHANNEL ID            */ ADD_BITS (     8, psDef->sChnReq.bChannelId                   )
//          /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
//          }
//          else
            if ( ( psInst->dwFlags & CENL7_FLAG_STRING_CHANNEL ) != 0 )
            {
            /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                if ( ( psInst->dwFlags & CENL7_FLAG_SUB_STRING_CHANNEL ) != 0 )
                {
            /* NB ITERATION          */ ADD_BITS (     8, psDef->sChnReq.bChannelLen + 3              )
            /* CHANNEL ID            */ ADD_BITS (     8, psDef->sChnReq.bChannelId                   )
            /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                }
                else
                {
            /* NB ITERATION          */ ADD_BITS (     8, psDef->sChnReq.bChannelLen + 2              )
            /* CHANNEL ID            */ ADD_BITS (     8, psDef->sChnReq.bChannelId                   )
                }
            }
            else
            {
            /* TYPE OCTET CHANNELRQ  */ ADD_BITS (     8, CENL7_CHOICE_CHANNEL_RQ                     )
                if ( ( psInst->dwFlags & CENL7_FLAG_SUB_STRING_CHANNEL ) != 0 )
                {
            /* CHANNEL ID            */ ADD_BITS (     8, psDef->sChnReq.bChannelId                   )
            /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                }
                else
                {
            /* CHANNEL ID            */ ADD_BITS (     8, psDef->sChnReq.bChannelId                   )
                }
            }

            if ( ( psInst->dwFlags & CENL7_FLAG_CHANNEL_PSW ) != 0 )
			{
            /* NB ITERATION          */ ADD_BITS (     8, psDef->sChnReq.bChannelLen - 8              )
			}
			else
			{
            /* NB ITERATION          */ ADD_BITS (     8, psDef->sChnReq.bChannelLen                  )
			}
            dwLen = (DWORD)psDef->sChnReq.bChannelLen * 8;
            /* CHANNEL DATA          */ CPY_BITS ( dwLen, psDef->sChnReq.tbChannelData                )

            break;

        case CENL7_ACTCODE_SUBTRACT :
        case CENL7_ACTCODE_ADD :

            if ( psDef->sSubReq.bAttrCount > CENL7_MAX_ATTRS ) 
                __leave;

            bLen = 1;

            /* TYPE OCTET STRING     */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
            /* OCTETSTR SIZE (UNK)   */ ADD_BITS (     8, 0                                           )
            /* ATTRCOUNT             */ ADD_BITS (     8, psDef->sSubReq.bAttrCount                   )
            for ( bIndex = 0 ; bIndex < psDef->sSubReq.bAttrCount ; bIndex ++ )
            {
                if ( ( psDef->sSubReq.tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                    __leave;
                if ( psDef->sSubReq.tsAttrs[bIndex].bAttrLen == 0 )
                    __leave;

            /* ATTR ID(n)            */ ADD_BITS (     8, psDef->sSubReq.tsAttrs[bIndex].bAttrId      )
            
            /* ATTR CONTAINER        */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )

                dwLen = psDef->sSubReq.tsAttrs[bIndex].bAttrLen;
            /* ATTR SIZE             */ ADD_BITS (     8, (BYTE)dwLen                                 )
            /* ATTR DATA             */ CPY_BITS ( dwLen*8, psDef->sSubReq.tsAttrs[bIndex].tbAttrData )
                bLen += ( 3 + (BYTE)dwLen );
            }

            // Remplacer l'octet codant la taille avec celle obtenue.
            pbBuf[1] = bLen;

            break;

        case CENL7_ACTCODE_GET_SECURE :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) != 0 )
                __leave;

            if ( psDef->sGScReq.bAttrIdCount > CENL7_MAX_ATTRS ) 
                __leave;

            /* Container             */ ADD_BITS (     8, CENL7_CHOICE_ATTRIBUTE_ID_LIST              )
            /* ATTRCOUNT             */ ADD_BITS (     8, psDef->sGScReq.bAttrIdCount                 )
            for ( bIndex = 0 ; bIndex < psDef->sGScReq.bAttrIdCount ; bIndex ++ )
            {
                if ( ( psDef->sGScReq.tbAttrIds[bIndex] & 0x80 ) != 0 )
                    __leave;
            /* ATTR ID(n)            */ ADD_BITS (     8, psDef->sGScReq.tbAttrIds[bIndex]            )
            }

            break;

        case CENL7_ACTCODE_SET_SECURE :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) != 0 )
            {
                if ( psDef->sTGSScReq.bKeyCount > CENL7_TG_MAX_KEYS )
                    __leave;

                /* Container             */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                /* Size                  */ ADD_BITS (     8, 1 + ( psDef->sTGSScReq.bKeyCount * ( 1 + 4 + 8 ) ) + 1 + 4 )

                /* Number of keys        */ ADD_BITS (     8, psDef->sTGSScReq.bKeyCount                      )

                for ( bIndex = 0 ; bIndex < psDef->sTGSScReq.bKeyCount ; bIndex ++ )
                {
                    /* Key Ref               */ ADD_BITS (     8, psDef->sTGSScReq.tsKeys[bIndex].bKeyRef     )
                    /* Key Access            */ CPY_BITS (    32, psDef->sTGSScReq.tsKeys[bIndex].tbKeyAccess )
                    /* Crypted Key           */ CPY_BITS (    64, psDef->sTGSScReq.tsKeys[bIndex].tbCryptedKey)
                }

                dwLen = (DWORD)psDef->sTGSScReq.bRndLen * 8;
                /* RNDRSE LENGTH         */ ADD_BITS (     8, psDef->sTGSScReq.bRndLen                    )
                /* RNDRSE DATA           */ CPY_BITS ( dwLen, psDef->sTGSScReq.tbRnd                      )
            }
            else if ( ( psInst->dwFlags & CENL7_FLAG_QF_PRIVATE ) != 0 )
            {
                if ( psDef->sSScReq.bAttrCount > CENL7_MAX_ATTRS ) 
                    __leave;

                dwLen = 1;
                for ( bIndex = 0 ; bIndex < psDef->sSScReq.bAttrCount ; bIndex ++ )
                    dwLen += ( psDef->sSScReq.tsAttrs[bIndex].bAttrLen + 3 );
                /* Container             */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                /* SIZE                  */ ADD_BITS (     8, (BYTE)dwLen                                 )


                /* ATTRCOUNT             */ ADD_BITS (     8, psDef->sSScReq.bAttrCount                   )
                for ( bIndex = 0 ; bIndex < psDef->sSScReq.bAttrCount ; bIndex ++ )
                {
                    if ( ( psDef->sSScReq.tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                        __leave;
                    if ( psDef->sSScReq.tsAttrs[bIndex].bAttrLen == 0 )
                        __leave;

                /* ATTR ID(n)            */ ADD_BITS (     8, psDef->sSScReq.tsAttrs[bIndex].bAttrId      )
            
                /* ATTR CONTAINER        */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )

                    dwLen = psDef->sSScReq.tsAttrs[bIndex].bAttrLen;
                /* ATTR SIZE             */ ADD_BITS (     8, (BYTE)dwLen                                 )
                /* ATTR DATA             */ CPY_BITS ( dwLen*8, psDef->sSScReq.tsAttrs[bIndex].tbAttrData )
                }
            }
            else
            {
                if ( psDef->sSScReq.bAttrCount > CENL7_MAX_ATTRS ) 
                    __leave;

                /* Container             */ ADD_BITS (     8, CENL7_CHOICE_ATTRIBUTE_LIST                 )
                /* ATTRCOUNT             */ ADD_BITS (     8, psDef->sSScReq.bAttrCount                   )
                for ( bIndex = 0 ; bIndex < psDef->sSScReq.bAttrCount ; bIndex ++ )
                {
                    if ( ( psDef->sSScReq.tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                        __leave;
                    if ( psDef->sSScReq.tsAttrs[bIndex].bAttrLen == 0 )
                        __leave;

                /* ATTR ID(n)            */ ADD_BITS (     8, psDef->sSScReq.tsAttrs[bIndex].bAttrId      )
            
                /* ATTR CONTAINER        */ ADD_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )

                    dwLen = psDef->sSScReq.tsAttrs[bIndex].bAttrLen;
                /* ATTR SIZE             */ ADD_BITS (     8, (BYTE)dwLen                                 )
                /* ATTR DATA             */ CPY_BITS ( dwLen*8, psDef->sSScReq.tsAttrs[bIndex].tbAttrData )
                }
            }
            

            break;

        case CENL7_ACTCODE_TG_PERSO_APP :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) == 0 )
                __leave;

            if ( psDef->sTGPApReq.bBlockLen > CENL7_TG_MAX_APPBLOCK_BYTES )
                __leave;

            /* Container             */ ADD_BITS (     8, CENL7_CHOICE_TG_PERSO_APP_RQ                )
            /* Filling               */ ADD_BITS (     7, 0                                           )
            /* Mode                  */ ADD_BITS (     1, ( psDef->sTGPApReq.fAppend ? 1 : 0 )        )
            /* Total                 */ ADD_BITS (     8, psDef->sTGPApReq.bBlocksTotal               )
            /* Number (index)        */ ADD_BITS (     8, psDef->sTGPApReq.bBlockNumber               )
            dwLen = psDef->sTGPApReq.bBlockLen;
            /* SIZE                  */ ADD_BITS (     8, (BYTE)dwLen                                 )
            /* DATA                  */ CPY_BITS ( dwLen*8, psDef->sTGPApReq.tbBlock                  )

            break;

        default :
            __leave;
        }

        dwLen = ( dwBufBitPos + 7 ) / 8;
        *pdwBufBytePos += dwLen;
        *pdwBufByteLen -= dwLen;

        fReturn = TRUE;
    }
    __finally
    {
    }

    return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildHeader( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_API         * psDef )
{
    BOOL            fReturn         = FALSE;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* SINGLE_FRAG           */ ADD_BITS (     1, 1                                                      )
        /* PDU NBR               */ ADD_BITS (     4, psDef->bPDU                                            )
        /* FRAG NBR              */ ADD_BITS (     2, psDef->bFrag                                           )
        /* FILL                  */ ADD_BITS (     1, 1                                                      )

        /* T-APDU                */ ADD_BITS (     4, psDef->bBlockType                                      )

        fReturn = TRUE;
    }
    __finally
    {
    }

    return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildIniReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_INI_REQ     * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* OPTIONAL APP          */ ADD_BITS (     1, BUILD_BOOL( psDef->fOptAppPresent )                    )
        /* MANUFACTURER          */ ADD_BITS (    16, psDef->wManufacturer                                   )
        /* BEACON ID             */ ADD_BITS (    27, psDef->dwBeaconId                                      )
        /* TIME                  */ ADD_BITS (    32, psDef->dwTime                                          )
        /* PROFILE               */ ADD_BITS (     8, psDef->bProf                                           )

        if ( psDef->bAppCount > CENL7_MAX_APPS ) 
            __leave;
        /*  NBR OF APP           */ ADD_BITS (     8, psDef->bAppCount                                       )
        for ( bIndex = 0 ; bIndex < psDef->bAppCount ; bIndex ++ ) 
        {
        /* EDI PRESENT(n)        */ ADD_BITS (     1, BUILD_BOOL( psDef->tsApps[bIndex].fEIDPresent )        )
        /* PARM PRESENT(n)       */ ADD_BITS (     1, BUILD_BOOL( psDef->tsApps[bIndex].fParmPresent )       )
        /* APP ID(n)             */ ADD_BITS (     6, psDef->tsApps[bIndex].bAppId                           )

            if ( psDef->tsApps[bIndex].fEIDPresent )
        /* [EID(n)]              */ ADD_BITS (     8, psDef->tsApps[bIndex].bEID                             )
    
            if ( psDef->tsApps[bIndex].fParmPresent )
            {
                dwLen = CENL7GetBitRecordSize(
                    psInst, 
                    psDef->tsApps[bIndex].tbParmsData, 
                    0,
                    sizeof(psDef->tsApps[bIndex].tbParmsData) * 8,
                    NULL );
                CHK_VAL( dwLen )
                if ( dwLen > sizeof(psDef->tsApps[bIndex].tbParmsData) )
                    __leave;
        /* [APP PARM(n)]         */ CPY_BITS ( dwLen, psDef->tsApps[bIndex].tbParmsData                      )
            }
        }

        if ( psDef->fOptAppPresent )
        {
            if ( psDef->bOptAppCount > CENL7_MAX_APPS ) 
                __leave;
        /* [NBR OF OPT APP]      */ ADD_BITS (     8, psDef->bOptAppCount                                    )
            for ( bIndex = 0 ; bIndex < psDef->bOptAppCount ; bIndex ++ ) 
            {
        /* EDI PRESENT(n)        */ ADD_BITS (     1, BUILD_BOOL( psDef->tsOptApps[bIndex].fEIDPresent )     )
        /* PARM PRESENT(n)       */ ADD_BITS (     1, BUILD_BOOL( psDef->tsOptApps[bIndex].fParmPresent )    )
        /* APP ID(n)             */ ADD_BITS (     6, psDef->tsOptApps[bIndex].bAppId                        )

                if ( psDef->tsOptApps[bIndex].fEIDPresent )
        /* [EID(n)]              */ ADD_BITS (     8, psDef->tsOptApps[bIndex].bEID                          )
    
                if ( psDef->tsOptApps[bIndex].fParmPresent )
                {
                    dwLen = CENL7GetBitRecordSize( 
                        psInst, 
                        psDef->tsOptApps[bIndex].tbParmsData,
                        0,
                        sizeof(psDef->tsOptApps[bIndex].tbParmsData) * 8,
                        NULL );
                    CHK_VAL( dwLen )
                    if ( dwLen > sizeof(psDef->tsOptApps[bIndex].tbParmsData) )
                        __leave;
        /* [APP PARM(n)]         */ CPY_BITS ( dwLen, psDef->tsOptApps[bIndex].tbParmsData                   )
                }
            }
        }                                            

        if ( psDef->bProfCount > CENL7_MAX_PROFS ) 
            __leave;
        /* NBR OF PROFILES       */ ADD_BITS (     8, psDef->bProfCount                                      )
        for ( bIndex = 0 ; bIndex < psDef->bProfCount ; bIndex ++ ) 
        /* PROFILE(n)            */ ADD_BITS (     8, psDef->tbProfs[bIndex]                                 )

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildSetReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_SET_REQ     * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
    DWORD           dwLen;
    DWORD           dwBitLen;
    BYTE            bCont;
    BYTE            bDef;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* ACC CRED PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fAccCredPresent )                   )
        /* USE IID               */ ADD_BITS (     1, BUILD_BOOL( psDef->fIIDPresent )                       )
        /* FILL                  */ ADD_BITS (     1, 0                                                      )
        /* MODE (=CONFIRM)       */ ADD_BITS (     1, 1                                                      )

        /* EDI                   */ ADD_BITS (     8, psDef->bEID                                            )

        if ( psDef->fAccCredPresent )
        {
            dwLen = (DWORD)psDef->bAccCredLen * 8;
            if ( ( psInst->dwFlags & CENL7_FLAG_FIXED_SIZE_AC ) == 0 )
            {
        /* [ACC CRED LEN]        */ ADD_BITS (     8, psDef->bAccCredLen                                     )
            }
        /* [ACC CRED]            */ CPY_BITS ( dwLen, psDef->tbAccCred                                       )
        }

        if ( psDef->bAttrCount > CENL7_MAX_ATTRS ) 
            __leave;
        /* ATTR COUNT            */ ADD_BITS (     8, psDef->bAttrCount                                      )
        for ( bIndex = 0 ; bIndex < psDef->bAttrCount ; bIndex ++ )
        {
            if ( ( psDef->tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                __leave;
            if ( psDef->tsAttrs[bIndex].bAttrLen == 0 )
                __leave;

        /* ATTR ID(n)            */ ADD_BITS (     8, psDef->tsAttrs[bIndex].bAttrId                         )
            
            bCont = psInst->tbAttributes[psDef->tsAttrs[bIndex].bAttrId];
        /* ATTR CONTAINER        */ ADD_BITS (     8, bCont                                                  )

            bDef = psInst->tbContainers[bCont];
            dwLen = psDef->tsAttrs[bIndex].bAttrLen;
            dwBitLen = dwLen * 8;
            if ( ( bDef & CONTAINER_CODED_SIZE ) != 0 )
            {
        /* ATTR SIZE             */ ADD_BITS (     8, (BYTE)dwLen                                            )
            }
            else if ( dwLen != (DWORD)( bDef & CONTAINER_SIZE_MASK ) )
                __leave;

        /* ATTR SIZE             */ CPY_BITS ( dwBitLen, psDef->tsAttrs[bIndex].tbAttrData                   )
        }

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ ADD_BITS (     8, psDef->bIID                                            )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildGetReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_GET_REQ     * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* ACC CRED PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fAccCredPresent )                   )
        /* USE IID               */ ADD_BITS (     1, BUILD_BOOL( psDef->fIIDPresent)                        )
        /* ATTR LIST PRES        */ ADD_BITS (     1, BUILD_BOOL( psDef->fAttrIdListPresent )                )
        /* FILL                  */ ADD_BITS (     1, 0                                                      )

        /* EDI                   */ ADD_BITS (     8, psDef->bEID                                            )

        if ( psDef->fAccCredPresent )
        {
            dwLen = (DWORD)psDef->bAccCredLen * 8;
            if ( ( psInst->dwFlags & CENL7_FLAG_FIXED_SIZE_AC ) == 0 )
            {
        /* [ACC CRED LEN]        */ ADD_BITS (     8, psDef->bAccCredLen                                     )
            }
        /* [ACC CRED]            */ CPY_BITS ( dwLen, psDef->tbAccCred                                       )
        }

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ ADD_BITS (     8, psDef->bIID                                            )
        }

        if ( psDef->fAttrIdListPresent )
        {
            if ( psDef->bAttrIdCount > CENL7_MAX_ATTRS ) 
                __leave;
        /* [ATTR ID COUNT]       */ ADD_BITS (     8, psDef->bAttrIdCount                                    )
            for ( bIndex = 0 ; bIndex < psDef->bAttrIdCount ; bIndex ++ )
        /* [ATTR ID(n)]          */ ADD_BITS (     8, psDef->tbAttrIds[bIndex]                               )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildActReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_ACT_REQ     * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* ACC CRED PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fAccCredPresent )                   )
        /* ACT PARM PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fParmsPresent )                     )
        /* USE IID               */ ADD_BITS (     1, BUILD_BOOL( psDef->fIIDPresent)                        )
        /* CONFIRMED MODE        */ ADD_BITS (     1, BUILD_BOOL( psDef->fConfirmed )                        )

        /* EDI                   */ ADD_BITS (     8, psDef->bEID                                            )

        /* ACTION TYPE           */ ADD_BITS (     8, psDef->bActionType                                     )

        if ( psDef->fAccCredPresent )
        {
            dwLen = (DWORD)psDef->bAccCredLen * 8;
            if ( ( psInst->dwFlags & CENL7_FLAG_FIXED_SIZE_AC ) == 0 )
            {
        /* [ACC CRED LEN]        */ ADD_BITS (     8, psDef->bAccCredLen                                     )
            }
        /* [ACC CRED]            */ CPY_BITS ( dwLen, psDef->tbAccCred                                       )
        }

        if ( psDef->fParmsPresent )
        {
            if ( psDef->bParmsLen == 0 )
            {
                dwLen = CENL7GetBitRecordSize( 
                    psInst, 
                    psDef->tbParmsData, 
                    0,
                    sizeof(psDef->tbParmsData) * 8,
                    NULL );
                
                CHK_VAL( dwLen )
            }
            else
                dwLen = (DWORD)psDef->bParmsLen * 8;
            if ( dwLen > sizeof(psDef->tbParmsData) * 8 )
                __leave;
        /* [ACT PARMS]           */ CPY_BITS ( dwLen, psDef->tbParmsData                                     )
        }

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ ADD_BITS (     8, psDef->bIID                                            )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7BuildEvtReq( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  char              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        IN      CENL7_EVT_REQ     * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* ACC CRED PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fAccCredPresent )                   )
        /* ACT PARM PRES         */ ADD_BITS (     1, BUILD_BOOL( psDef->fParmsPresent )                     )
        /* USE IID               */ ADD_BITS (     1, BUILD_BOOL( psDef->fIIDPresent)                        )
        /* CONFIRMED MODE        */ ADD_BITS (     1, BUILD_BOOL( psDef->fConfirmed )                        )

        /* EDI                   */ ADD_BITS (     8, psDef->bEID                                            )

        if ( psDef->fAccCredPresent )
        {
            dwLen = (DWORD)psDef->bAccCredLen * 8;
            if ( ( psInst->dwFlags & CENL7_FLAG_FIXED_SIZE_AC ) == 0 )
            {
        /* [ACC CRED LEN]        */ ADD_BITS (     8, psDef->bAccCredLen                                     )
            }
        /* [ACC CRED]            */ CPY_BITS ( dwLen, psDef->tbAccCred                                       )
        }

        /* EVENT  TYPE           */ ADD_BITS (     8, psDef->bEventType                                      )
        if ( psDef->fParmsPresent )
        {
            if ( psDef->bParmsLen == 0 )
            {
                dwLen = CENL7GetBitRecordSize( 
                    psInst, 
                    psDef->tbParmsData, 
                    0,
                    sizeof(psDef->tbParmsData) * 8,
                    NULL );
                
                CHK_VAL( dwLen )
            }
            else
                dwLen = (DWORD)psDef->bParmsLen * 8;
            if ( dwLen > sizeof(psDef->tbParmsData) )
                __leave;

        /* [ACT PARMS]           */ CPY_BITS ( dwLen, psDef->tbParmsData                                     )
        }

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ ADD_BITS (     8, psDef->bIID                                            )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




