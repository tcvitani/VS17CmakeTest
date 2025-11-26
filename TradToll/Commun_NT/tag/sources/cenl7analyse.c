/* --------------------------------------------------------------------
 * (C) 2001 CS  - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CENL7
 * FILE       : CENL7ANALYSE.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Analyse des messages couche 7 DSRC (norme CEN)
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




#define GET_BITS(s,v)   { (v) = 0; if ( ! BITBLGetNumber( (DWORD*)&(v), s, pbBuf, pdwBufBitPos, pdwBufBitLen ) ) __leave; }

#define CPY_BITS(s,v)   if ( ! BITBLCopyFromBuffer( v, 0, s, pbBuf, pdwBufBitPos, pdwBufBitLen ) ) __leave; else;

#define CHK_BITS(s,v)   {   DWORD dwVal = 0;\
                            if ( ! BITBLGetNumber( &dwVal, s, pbBuf, pdwBufBitPos, pdwBufBitLen ) ) __leave;\
                            if ( dwVal != (DWORD)(v) )\
                                __leave;\
                        }

#define FOO_BITS(s)     {   DWORD dwVal = 0;\
                            if ( ! BITBLGetNumber( &dwVal, s, pbBuf, pdwBufBitPos, pdwBufBitLen ) ) __leave;\
                        }





PRIVATE BOOL WINAPI _CENL7AnalyseHeader( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     CENL7_API         * psDef );




PRIVATE BOOL WINAPI _CENL7AnalyseIniRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_INI_RSP     * psDef );




PRIVATE BOOL WINAPI _CENL7AnalyseSetRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_SET_RSP     * psDef );




PRIVATE BOOL WINAPI _CENL7AnalyseGetRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_GET_RSP     * psDef );




PRIVATE BOOL WINAPI _CENL7AnalyseActRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_ACT_RSP     * psDef );




PRIVATE BOOL WINAPI _CENL7AnalyseEvtRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_EVT_RSP     * psDef );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7AnalyseApdu( 
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      OUT     CENL7_API         * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer contenant des données
 *             pdwBufBytePos : En entrée, position du premier octet,
 *                             En sortie, position de l'octet immmédiatement après le dernier utilisé
 *             pdwBufByteLen : En entrée, taille du buffer en octets, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             psDef         : Récupère la définition du bloc analysé.
 * RETURN    : TRUE si le bloc a été analysé. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Analyse le premier bloc CEN d'un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7AnalyseApdu( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     CENL7_API         * psDef )
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

        if ( ! _CENL7AnalyseHeader( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, psDef ) )
            __leave;

        switch ( psDef->bBlockType )
        {
        case CENL7_APDU_INI_RSP :
            if ( ! _CENL7AnalyseIniRsp( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sIniRsp ) )
                __leave;
            break;

        case CENL7_APDU_SET_RSP :
            if ( ! _CENL7AnalyseSetRsp( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sSetRsp ) )
                __leave;
            break;

        case CENL7_APDU_GET_RSP :
            if ( ! _CENL7AnalyseGetRsp( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sGetRsp ) )
                __leave;
            break;

        case CENL7_APDU_ACT_RSP :
            if ( ! _CENL7AnalyseActRsp( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sActRsp ) )
                __leave;
            break;

        case CENL7_APDU_EVT_RSP :
            if ( ! _CENL7AnalyseEvtRsp( psInst, pbBuf, &dwBufBitPos, &dwBufBitLen, &psDef->sEvtRsp ) )
                __leave;
            break;

        default:
            __leave;
        }
        
        fReturn = TRUE;
    }
    __finally
    {
        if ( fReturn )
        {
            (*pdwBufBytePos) = ( dwBufBitPos + 7 ) / 8;
            (*pdwBufByteLen) = dwBufBitLen / 8;
        }
        
    }

	return fReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7AnalyseActionParams(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN      char              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      BYTE                bActionType,
 *                      OUT     CENL7_ACT_PARAMS  * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer contenant des données
 *             pdwBufBytePos : En entrée, position du premier octet,
 *                             En sortie, position de l'octet immmédiatement après le dernier utilisé
 *             pdwBufByteLen : En entrée, taille du buffer en octets, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             bActionType   : Type d'action à traiter.
 *             psDef         : Récupère la définition du bloc analysé.
 * RETURN    : TRUE si le bloc a été analysé. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Analyse un bloc de paramètre d'action dans un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7AnalyseActionParams(
        IN      CENL7_INSTANCE    * psInst,
        IN      char              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      BYTE                bActionType,
        OUT     CENL7_ACT_PARAMS  * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
//    BYTE            bCont;
//    BYTE            bDef;
    DWORD           dwLen;
    DWORD           dwBitLen;
    DWORD           dwBufBitPos;
    DWORD           dwBufBitLen;
    DWORD           dwFullBitLen;
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

            break;

        case CENL7_ACTCODE_GET_NONCE :

            if ( ( psInst->dwFlags & CENL7_FLAG_QF_PRIVATE ) != 0 )
            {
                /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING               )

                /* RND LENGTH            */ GET_BITS (     8, psDef->sQFGNoRsp.bRndLen                )
                if ( psDef->sQFGNoRsp.bRndLen > CENL7_QF_MAX_RND_BYTES ) 
                    __leave;

                dwLen = (DWORD)psDef->sQFGNoRsp.bRndLen * 8;
                /* [ATTR DATA(n)]        */ CPY_BITS ( dwLen, psDef->sQFGNoRsp.tbRnd                  )
            }
            else
            {
                /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING               )

                /* RND LENGTH            */ GET_BITS (     8, psDef->sGNoRsp.bRndLen                  )
                if ( psDef->sGNoRsp.bRndLen > CENL7_MAX_RND_BYTES ) 
                    __leave;

                dwLen = (DWORD)psDef->sGNoRsp.bRndLen * 8;
                /* [ATTR DATA(n)]        */ CPY_BITS ( dwLen, psDef->sGNoRsp.tbRnd                    )
            }

            break;

        case CENL7_ACTCODE_GET_STAMPED :

            /* TYPE GET STAMPED RS   */ CHK_BITS (     8, CENL7_CHOICE_GET_STAMPED_RS                 )

            /* ATTR COUNT            */ GET_BITS (     8, psDef->sGStRsp.bAttrCount                   )
            if ( psDef->sGStRsp.bAttrCount > CENL7_MAX_ATTRS ) 
                __leave;

            for ( bIndex = 0 ; bIndex < psDef->sGStRsp.bAttrCount ; bIndex ++ )
            {
            /* ATTR ID(n)            */ GET_BITS (     8, psDef->sGStRsp.tsAttrs[bIndex].bAttrId      )
                if ( ( psDef->sGStRsp.tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                    __leave;

                dwFullBitLen = CENL7GetBitRecordSize( psInst, pbBuf, *pdwBufBitPos, *pdwBufBitLen, &dwBitLen );
                if ( dwFullBitLen == CENL7_INVALID )
                    __leave;

                psDef->sGStRsp.tsAttrs[bIndex].bAttrLen = (byte)( ( dwBitLen + 7 ) / 8 );
                                    FOO_BITS(  dwFullBitLen - dwBitLen );
        /* [ATTR DATA(n)]        */ CPY_BITS ( dwBitLen, psDef->sGStRsp.tsAttrs[bIndex].tbAttrData                   )


//                bCont = psInst->tbAttributes[psDef->sGStRsp.tsAttrs[bIndex].bAttrId];
//            /* CONTAINER ASN1        */ CHK_BITS (     8, bCont                                       )
//                bDef = psInst->tbContainers[bCont];
//                if ( ( bDef & CONTAINER_CODED_SIZE ) != 0 )
//                {
//            /* ATTR LEN             */ GET_BITS (     8, psDef->sGStRsp.tsAttrs[bIndex].bAttrLen      )
//                }
//                else
//                    psDef->sGStRsp.tsAttrs[bIndex].bAttrLen = bDef & CONTAINER_SIZE_MASK;
//
//                dwLen = psDef->sGStRsp.tsAttrs[bIndex].bAttrLen;
//                dwBitLen = dwLen * 8;
//                CHK_VAL( dwBitLen )
//                if ( dwLen > sizeof(psDef->sGStRsp.tsAttrs[bIndex].tbAttrData) )
//                     __leave;
//
//            /* [ATTR DATA(n)]        */ CPY_BITS ( dwBitLen, psDef->sGStRsp.tsAttrs[bIndex].tbAttrData)
            }

            /* ATTR COUNT            */ GET_BITS (     8, psDef->sGStRsp.bMacLen                      )
            dwLen = (DWORD)psDef->sGStRsp.bMacLen * 8;
            /* [ATTR DATA(n)]        */ CPY_BITS ( dwLen, psDef->sGStRsp.tbMac                        )

            break;

        case CENL7_ACTCODE_ECHO :

            /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
            /* NB ITERATION          */ GET_BITS (     8, psDef->sEchRsp.bEchoLen                     )
            dwLen = (DWORD)psDef->sEchRsp.bEchoLen * 8;
            /* DONNE ECHO            */ CPY_BITS ( dwLen, psDef->sEchRsp.tbEcho                       )

            break;

        case CENL7_ACTCODE_TRANSFER_CHANNEL :
        case CENL7_ACTCODE_PRIVATE_TRF_CHN :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) != 0 )
                __leave;

            if ( ( psInst->dwFlags & CENL7_FLAG_STRING_CHANNEL ) != 0 )
            {
            /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                if ( ( psInst->dwFlags & CENL7_FLAG_SUB_STRING_CHANNEL ) != 0 )
                {
            /* NB ITERATION          */ GET_BITS (     8, bIndex                                      )
                    if ( bIndex == 1 )
                    {
            /* CHANNEL ID            */ GET_BITS (     8, psDef->sChnRsp.bChannelId                   )
                        psDef->sChnRsp.bChannelLen = 0;
                    }
                    else if ( bIndex >= 3 )
                    {
            /* CHANNEL ID            */ GET_BITS (     8, psDef->sChnRsp.bChannelId                   )
            /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
            /* NB ITERATION          */ GET_BITS (     8, psDef->sChnRsp.bChannelLen                  )
                        if ( ( psDef->sChnRsp.bChannelLen + 3 ) != bIndex )
                            __leave;
                    }
                    else
                        __leave;
                }
                else
                {
            /* NB ITERATION          */ GET_BITS (     8, bIndex                                      )
                    if ( bIndex < 1 )
                        __leave;
            /* CHANNEL ID            */ GET_BITS (     8, psDef->sChnRsp.bChannelId                   )
                    psDef->sChnRsp.bChannelLen = bIndex - 1;
                }
            }
            else
            {
            /* TYPE OCTET CHANNELRS  */ CHK_BITS (     8, CENL7_CHOICE_CHANNEL_RS                     )
                if ( ( psInst->dwFlags & CENL7_FLAG_SUB_STRING_CHANNEL ) != 0 )
                {
            /* CHANNEL ID            */ GET_BITS (     8, psDef->sChnRsp.bChannelId                   )
            /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
                }
                else
                {
            /* CHANNEL ID            */ GET_BITS (     8, psDef->sChnRsp.bChannelId                   )
                }
            /* NB ITERATION          */ GET_BITS (     8, psDef->sChnRsp.bChannelLen                  )
            }

            dwLen = (DWORD)psDef->sChnRsp.bChannelLen * 8;
            /* CHANNEL DATA          */ CPY_BITS ( dwLen, psDef->sChnRsp.tbChannelData                )

            break;

        case CENL7_ACTCODE_SUBTRACT :
        case CENL7_ACTCODE_ADD :

            break;

        case CENL7_ACTCODE_GET_SECURE :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) != 0 )
                __leave;

            /* TYPE GET STAMPED RS   */ CHK_BITS (     8, CENL7_CHOICE_ATTRIBUTE_LIST                 )

            /* ATTR COUNT            */ GET_BITS (     8, psDef->sGScRsp.bAttrCount                   )
            if ( psDef->sGScRsp.bAttrCount > CENL7_MAX_ATTRS ) 
                __leave;

            for ( bIndex = 0 ; bIndex < psDef->sGScRsp.bAttrCount ; bIndex ++ )
            {
            /* ATTR ID(n)            */ GET_BITS (     8, psDef->sGScRsp.tsAttrs[bIndex].bAttrId      )
                if ( ( psDef->sGScRsp.tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                    __leave;
            /* CONTAINER ASN1        */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )

            /* ATTR LEN              */ GET_BITS (     8, psDef->sGScRsp.tsAttrs[bIndex].bAttrLen     )

                dwLen = psDef->sGScRsp.tsAttrs[bIndex].bAttrLen;
                dwBitLen = dwLen * 8;
                CHK_VAL( dwBitLen )
                if ( dwLen > sizeof(psDef->sGScRsp.tsAttrs[bIndex].tbAttrData) )
                    __leave;

            /* [ATTR DATA(n)]        */ CPY_BITS ( dwBitLen, psDef->sGScRsp.tsAttrs[bIndex].tbAttrData)
            }

            break;

        case CENL7_ACTCODE_SET_SECURE :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) == 0 )
                break; // This is not an error
            
            /* TYPE OCTET STRING     */ CHK_BITS (     8, CENL7_CHOICE_OCTET_STRING                   )
            /* NB BYTES              */ GET_BITS (     8, bIndex                                      )

            /* NB ITERATION          */ GET_BITS (     8, psDef->sTGSScRsp.bKeyCount                  )
            if ( ( ( psDef->sTGSScRsp.bKeyCount * 5 ) + 1 ) != bIndex )
                __leave;

            for ( bIndex = 0 ; bIndex < psDef->sTGSScRsp.bKeyCount ; bIndex ++ )
            {
            /* KEY REF               */ GET_BITS (     8, psDef->sTGSScRsp.tsKvcs[bIndex].bKeyRef     )
            /* CHANNEL DATA          */ CPY_BITS (    32, psDef->sTGSScRsp.tsKvcs[bIndex].tbKvc       )
            }

            break;

        case CENL7_ACTCODE_TG_PERSO_APP :

            if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) == 0 )
                __leave;
            
            /* TYPE PERSO_APP RSP    */ CHK_BITS (     8, CENL7_CHOICE_TG_PERSO_APP_RS                )
            /* BLOCK NUMBER          */ GET_BITS (     8, psDef->sTGPApRsp.bBlockNumber               )
            /* STATUS                */ GET_BITS (     8, psDef->sTGPApRsp.bBlockStatus               )

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
		;
    }

	return fReturn;
}





PRIVATE BOOL WINAPI _CENL7AnalyseHeader( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_API         * psDef )
{
    BOOL            fReturn         = FALSE;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* SINGLE_FRAG           */ CHK_BITS (     1, 1                                                      )
        /* PDU NBR               */ GET_BITS (     4, psDef->bPDU                                            )
        /* FRAG NBR              */ GET_BITS (     2, psDef->bFrag                                           )
        /* FILL                  */ FOO_BITS (     1                                                         )

        /* T-APDU                */ GET_BITS (     4, psDef->bBlockType                                      )

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7AnalyseIniRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_INI_RSP     * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* FILL                  */ FOO_BITS (     4                                                         )
        /* PROFILE               */ GET_BITS (     8, psDef->bProf                                           )

        /* NBR OF APP            */ GET_BITS (     8, psDef->bAppCount                                       )
        if ( psDef->bAppCount > CENL7_MAX_APPS ) 
            __leave;
        for ( bIndex = 0 ; bIndex < psDef->bAppCount ; bIndex ++ ) 
        {
        /* EDI PRESENT           */ GET_BITS (     1, psDef->tsApps[bIndex].fEIDPresent                      )
        /* PARM PRESENT          */ GET_BITS (     1, psDef->tsApps[bIndex].fParmPresent                     )
        /* APP ID                */ GET_BITS (     6, psDef->tsApps[bIndex].bAppId                           )

            if ( psDef->tsApps[bIndex].fEIDPresent )
            {
        /* [EID]                 */ GET_BITS (     8, psDef->tsApps[bIndex].bEID                             )
            }
        
            if ( psDef->tsApps[bIndex].fParmPresent )
            {
                dwLen = CENL7GetBitRecordSize( psInst, pbBuf, *pdwBufBitPos, *pdwBufBitLen, NULL );
                CHK_VAL( dwLen )
                if ( dwLen > ( sizeof(psDef->tsApps[bIndex].tbParmsData) * 8 ) )
                    __leave;
                psDef->tsApps[bIndex].bParmsLen = (BYTE)( ( dwLen + 7 ) / 8 );
        /* [APP PARM]            */ CPY_BITS ( dwLen, psDef->tsApps[bIndex].tbParmsData                      )
            }
        }

        /* OBE STATUS PRESENT    */ GET_BITS (     1, psDef->fOBEStatusPresent                               )
        /* EQUIPMENT CLASS       */ GET_BITS (    15, psDef->wEqtClass                                       )
        /* NANUFACTURER ID       */ GET_BITS (    16, psDef->wManufacturerId                                 )
        if ( psDef->fOBEStatusPresent )
        {
        /* OBE STATUS            */ GET_BITS (     8, psDef->bOBEStatus                                      )
        }
        if ( ( ( psInst->dwFlags & CENL7_FLAG_LONG_OBE_STATUS ) == 0 ) ||
             ( psDef->fOBEStatusPresent                              ) )
        {
        /* PRIVATE STATUS CODE   */ GET_BITS (     8, psDef->bPrivate                                        )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7AnalyseSetRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_SET_RSP     * psDef )
{
    BOOL            fReturn         = FALSE;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* IID PRESENT           */ GET_BITS (     1, psDef->fIIDPresent                                     )
        /* STATUS PRESENT        */ GET_BITS (     1, psDef->fStatusPresent                                  )

        /* FILL                  */ FOO_BITS (     2                                                         )

        /* EID                   */ GET_BITS (     8, psDef->bEID                                            )

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ GET_BITS (     8, psDef->bIID                                            )
        }

        if ( psDef->fStatusPresent )
        {
        /* [STATUS]              */ GET_BITS (     8, psDef->bStatus                                         )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7AnalyseGetRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_GET_RSP     * psDef )
{
    BOOL            fReturn         = FALSE;
    BYTE            bIndex;
//    BYTE            bCont;
//    BYTE            bDef;
//    DWORD           dwLen;
    DWORD           dwBitLen;
    DWORD           dwFullBitLens;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* IID PRESENT           */ GET_BITS (     1, psDef->fIIDPresent                                     )
        /* ATTR LIST PRESENT     */ GET_BITS (     1, psDef->fAttrListPresent                                )
        /* STATUS PRESENT        */ GET_BITS (     1, psDef->fStatusPresent                                  )

        /* FILL                  */ FOO_BITS (     1                                                         )

        /* EID                   */ GET_BITS (     8, psDef->bEID                                            )

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ GET_BITS (     8, psDef->bIID                                            )
        }

        if ( psDef->fAttrListPresent )
        {
        /* [ATTR COUNT]          */ GET_BITS (     8, psDef->bAttrCount                                      )
            if ( psDef->bAttrCount > CENL7_MAX_ATTRS ) 
                __leave;
            for ( bIndex = 0 ; bIndex < psDef->bAttrCount ; bIndex ++ )
            {


        /* [ATTR ID(n)]          */ GET_BITS (     8, psDef->tsAttrs[bIndex].bAttrId                         )
                if ( ( psDef->tsAttrs[bIndex].bAttrId & 0x80 ) != 0 )
                    __leave;
                dwFullBitLens = CENL7GetBitRecordSize( psInst, pbBuf, *pdwBufBitPos, *pdwBufBitLen, &dwBitLen );
                if ( dwFullBitLens == CENL7_INVALID )
                    __leave;

                psDef->tsAttrs[bIndex].bAttrLen = (byte)( ( dwBitLen + 7 ) / 8 );
                                    FOO_BITS(  dwFullBitLens - dwBitLen );
        /* [ATTR DATA(n)]        */ CPY_BITS ( dwBitLen, psDef->tsAttrs[bIndex].tbAttrData                   )

//                bCont = psInst->tbAttributes[psDef->tsAttrs[bIndex].bAttrId];
//        /* CONTAINER ASN1        */ CHK_BITS (     8, bCont                                                  )
//                bDef = psInst->tbContainers[bCont];
//
//                if ( ( bDef & CONTAINER_CODED_SIZE ) != 0 )
//                {
//        /* ATTR LEN             */ GET_BITS (     8, psDef->tsAttrs[bIndex].bAttrLen                         )
//                }
//                else
//                    psDef->tsAttrs[bIndex].bAttrLen = bDef & CONTAINER_SIZE_MASK;
//                
//                dwLen = psDef->tsAttrs[bIndex].bAttrLen;
//                dwBitLen = dwLen * 8;
//                CHK_VAL( dwBitLen )
//                if ( dwLen > sizeof(psDef->tsAttrs[bIndex].tbAttrData) )
//                    __leave;
//                
//        /* [ATTR DATA(n)]        */ CPY_BITS ( dwBitLen, psDef->tsAttrs[bIndex].tbAttrData                   )
            }
        }

        if ( psDef->fStatusPresent )
        {
        /* [STATUS]              */ GET_BITS (     8, psDef->bStatus                                         )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7AnalyseActRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_ACT_RSP     * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwLen;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* IID PRESENT           */ GET_BITS (     1, psDef->fIIDPresent                                     )
        /* PARM PRESENT          */ GET_BITS (     1, psDef->fParmsPresent                                   )
        /* STATUS PRESENT        */ GET_BITS (     1, psDef->fStatusPresent                                  )

        /* FILL                  */ FOO_BITS (     1                                                         )

        /* EID                   */ GET_BITS (     8, psDef->bEID                                            )

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ GET_BITS (     8, psDef->bIID                                            )
        }

        if ( psDef->fParmsPresent )
        {
            dwLen = CENL7GetBitRecordSize( psInst, pbBuf, *pdwBufBitPos, *pdwBufBitLen, NULL );
            CHK_VAL( dwLen )
            if ( dwLen > ( sizeof(psDef->tbParmsData) * 8 ) )
                __leave;
        /* [PARAM]               */ CPY_BITS ( dwLen, psDef->tbParmsData                                     )
			psDef->bParmsLen = (BYTE)((dwLen+7)/8);
        }

        if ( psDef->fStatusPresent )
        {
        /* [STATUS]              */ GET_BITS (     8, psDef->bStatus                                         )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE BOOL WINAPI _CENL7AnalyseEvtRsp( 
        IN      CENL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBitPos,
        IN OUT  DWORD             * pdwBufBitLen,
        OUT     CENL7_EVT_RSP     * psDef )
{
    BOOL            fReturn         = FALSE;

    __try
    {
        CHK_PTR( pdwBufBitPos )
        CHK_PTR( pdwBufBitLen )

        /* IID PRESENT           */ GET_BITS (     1, psDef->fIIDPresent                                     )
        /* STATUS PRESENT        */ GET_BITS (     1, psDef->fStatusPresent                                  )

        /* FILL                  */ FOO_BITS (     2                                                         )

        /* EID                   */ GET_BITS (     8, psDef->bEID                                            )

        if ( psDef->fIIDPresent )
        {
        /* [IID]                 */ GET_BITS (     8, psDef->bIID                                            )
        }

        if ( psDef->fStatusPresent )
        {
        /* [STATUS]              */ GET_BITS (     8, psDef->bStatus                                         )
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}






