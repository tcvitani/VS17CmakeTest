/* --------------------------------------------------------------------
 * (C) 2001 CS  - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CENL7
 * FILE       : CENL7SECURITY.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion de la securite couche 7 DSRC (norme CEN)
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
#include <csr_des.h>

#include "csr_tag_cenl7.h"

#include <memclass.h>



PRIVATE WORD WINAPI CENL7Algo2Crc(
        IN      BYTE      * pbData, 
        IN      DWORD       dwLen );

PRIVATE BOOL WINAPI CENL7Algo2Cipher(
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      BYTE      * pbKey );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7SetMasterSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwSecurityType,
 *                      IN      BYTE                  * pbSecurityData,
 *                      IN      DWORD                   dwSecurityLen )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             dwSecurityType: Type de sécurité utilisé (CENL7_SECURITY_NONE, CENL7_SECURITY_DES ou CENL7_SECURITY_ALGO2)
 *             pbSecurityData: Pointe sur un bloc contenant les données maitresses de sécurité.
 *                              - Dans le cas de la sécurité DES, il s'agit d'une succession
 *                               de 9 clés sur 16 octets chacunes.La première clé est la clé
 *                               d'accés par défaut. Les clés non connues doivent être initialisée à 0.
 *                              - Dans le cas de la sécurité ALGO2, il s'agit d'une clé sur 6 octets.
 *             dwSecurityLen : Taille du bloc contenant les données de sécurité maitre (6, 144).
 * RETURN    : TRUE si les données sont acceptables, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les données maitresses de la sécurité.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7SetMasterSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwSecurityType,
        IN      BYTE                  * pbSecurityData,
        IN      DWORD                   dwSecurityLen )
{
    BOOL    fReturn = FALSE;

    __try
    {

        if ( dwSecurityLen > sizeof( psInst->tbMasterSecurity ) )
            __leave;

        if ( ( ( dwSecurityType == CENL7_SECURITY_DES   ) &&
               ( dwSecurityLen  == 144                  ) ) ||
             ( ( dwSecurityType == CENL7_SECURITY_ALGO2 ) &&
               ( dwSecurityLen  == 6                    ) ) ||
			   /*TIS CARDME*/
			 ( ( dwSecurityType == CENL7_SECURITY_DES  ) &&
               ( dwSecurityLen  == 16                  ) ))
        {
            CopyMemory( psInst->tbMasterSecurity, pbSecurityData, dwSecurityLen );
            psInst->dwMasterSecurityLen = dwSecurityLen;
            psInst->dwSecurityType = dwSecurityType;
        }
        else if ( dwSecurityType == CENL7_SECURITY_NONE )
        {
            psInst->dwMasterSecurityLen = 0;
            psInst->dwSecurityType = dwSecurityType;
        }
        else
            __leave;


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
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7SetDerivedSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwSecurityType,
 *                      IN      BYTE                  * pbSecurityData,
 *                      IN      DWORD                   dwSecurityLen )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             dwSecurityType: Type de sécurité utilisé (CENL7_SECURITY_NONE, CENL7_SECURITY_DES ou CENL7_SECURITY_ALGO2)
 *             pbSecurityData: Pointe sur un bloc contenant les données dérivées de sécurité.
 *                              - Dans le cas de la sécurité DES, il s'agit d'une succession
 *                               de 9 clés sur 8 octets chacunes. La première clé est la clé
 *                               d'accés par défaut. Les clés non connues doivent être initialisée à 0.
 *                               Si la référence à la clé n'est pas fournie, la valeur utilisée est 0.
 *                              - Dans le cas de la sécurité ALGO2, il s'agit d'une clé sur 6 octets.
 *             dwSecurityLen : Taille du bloc contenant les données de sécurité dérivées (6, 72).
 * RETURN    : TRUE si les données sont acceptables, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les données dérivées de la sécurité. Permet de passer outre
 *             la phase de dérivation.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7SetDerivedSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwSecurityType,
        IN      BYTE                  * pbSecurityData,
        IN      DWORD                   dwSecurityLen )
{
    BOOL    fReturn = FALSE;

    __try
    {
        if ( dwSecurityLen > sizeof( psInst->tbDerivedSecurity ) )
            __leave;

        if ( ( ( dwSecurityType == CENL7_SECURITY_DES   ) &&    // 1+8 clés
               ( dwSecurityLen  == 72                   ) )  ||
             ( ( dwSecurityType == CENL7_SECURITY_ALGO2 ) &&
               ( dwSecurityLen  == 6                    ) )  || // 1 clé ALGO2
			 ( ( dwSecurityType == CENL7_SECURITY_DES   ) &&
               ( dwSecurityLen  == 8                    ) )  || // 1 clé
			 ( ( dwSecurityType == CENL7_SECURITY_DES   ) &&
               ( dwSecurityLen  == 16                   ) ))    // 1+1 clé : bug, conservé pour compatibilité ascendante
        {
            CopyMemory( psInst->tbDerivedSecurity, pbSecurityData, dwSecurityLen );
            psInst->dwDerivedSecurityLen = dwSecurityLen;
            psInst->dwSecurityType = dwSecurityType;
        }
        else if ( dwSecurityType == CENL7_SECURITY_NONE )
        {
            psInst->dwDerivedSecurityLen = 0;
            psInst->dwSecurityType = dwSecurityType;
        }
        else
            __leave;

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
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7DeriveSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbDerivationData,
 *                      IN      DWORD                   dwDerivationLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbDerivationData : Pointe sur les données à dériver pour obtenir les données
 *                                de sécurité de session.
 *             dwDerivationLen  : Taille des données de derivation.
 * RETURN    : TRUE si la dérivation a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue une dérivation des données de sécurité initialisées
 *             par un appel à CENL7SetMasterSecurity().
 *             En mode DES A1, les données à dériver sont constituées soit d'un
 *             bloc constitué des deux octets OBEGroupId (fourni en paramètre d'application
 *             dans la VST), soit d'un bloc de 7 octets constitué de la concaténation
 *             des 4 octets du ContractSerialNumber, et des 3 octets du ContractProvider.
 *             Dans le premier cas, seule la clé d'accés est dérivée, dans le second cas,
 *             seules les clés d'authentification sont dérivées.
 *             En mode ALGO2, les données à dériver sont constituées d'un
 *             bloc de 6 octets dont les deux premiers valent 0, les 4 suivant
 *             sont le contenu du champ "RSETime" de la BST.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7DeriveSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbDerivationData,
        IN      DWORD                   dwDerivationLen )
{
    BOOL    fReturn = FALSE;
    DWORD   dwIndex;
    DWORD   dwFoo;
    DWORD   dwStart;
    DWORD   dwCount;
    BYTE  * pbKey;
    BYTE  * pbDst;
    BYTE    tbData[8];
    BYTE    tbTmp[8];
    HDESKEY hDesKey1 = NULL;
    HDESKEY hDesKey2 = NULL;

    __try
    {
        pbKey = psInst->tbMasterSecurity;
        pbDst = psInst->tbDerivedSecurity;

        if ( ( ( psInst->dwMasterSecurityLen == 144                 ) &&
               ( psInst->dwSecurityType      == CENL7_SECURITY_DES  ) ) ||
             ( ( psInst->dwMasterSecurityLen == 16                  ) &&
			   ( psInst->dwSecurityType      == CENL7_SECURITY_DES  ) ) )
        {
            if ( dwDerivationLen == 2 ) //OBEGroupId,OBEGroupId,OBEGroupId,OBEGroupId
            {
                tbData[0] = pbDerivationData[0]; 
                tbData[1] = pbDerivationData[1];
                tbData[2] = pbDerivationData[0];
                tbData[3] = pbDerivationData[1];
                tbData[4] = pbDerivationData[0];
                tbData[5] = pbDerivationData[1];
                tbData[6] = pbDerivationData[0];
                tbData[7] = pbDerivationData[1];
                dwStart = 0;
                dwCount = 1;
            }
            else if ( dwDerivationLen == 7 )  //ContractSerialNumber, ContractProvider
            {
                tbData[0] = pbDerivationData[0];
                tbData[1] = pbDerivationData[1];
                tbData[2] = pbDerivationData[2];
                tbData[3] = pbDerivationData[3];
                tbData[4] = pbDerivationData[4];
                tbData[5] = pbDerivationData[5];
                tbData[6] = pbDerivationData[6];
                tbData[7] = 0;
                dwStart = 1;
                dwCount = 8;
            }
            else
                __leave;

            pbKey += ( dwStart * 16 );
            pbDst += ( dwStart * 8 );

            for ( dwIndex = dwStart ; dwIndex < ( dwStart + dwCount ) ; dwIndex ++ )
            {
                hDesKey1 = DESOpenKey( pbKey, FALSE );
                hDesKey2 = DESOpenKey( pbKey + 8, FALSE );

                if ( ( hDesKey1 == NULL ) || ( hDesKey2 == NULL ) )
                    __leave;

                //
                // Encryption en 3 passe, en utilisant le buffer de destination comme buffer
                // intermédiaire numéro 1, ainsi qu'un buffer dédié pour le numéro 2 :
                //    Encryption 1 : BufferSource vers BufferDestination
                //    Encryption 2 : BufferDestination vers BufferTemp
                //    Encryption 3 : BufferTemp vers BufferDestination
                //
                dwFoo = 8;
                if ( ! DESEncryptEcb( FALSE, TRUE, tbData, 8, pbDst, &dwFoo, hDesKey1 ) )
                    __leave;
                dwFoo = 8;
                if ( ! DESEncryptEcb( FALSE, FALSE, pbDst, 8, tbTmp, &dwFoo, hDesKey2 ) )
                    __leave;
                dwFoo = 8;
                if ( ! DESEncryptEcb( FALSE, TRUE, tbTmp, 8, pbDst, &dwFoo, hDesKey1 ) )
                    __leave;

                //
                // Passer à la clé maitre suivante ainsi qu'à la clé dérivée suivante
                //
                pbKey += 16;
                pbDst +=  8;

                DESCloseKey( hDesKey1 );
                DESCloseKey( hDesKey2 );
                hDesKey1 = NULL;
                hDesKey2 = NULL;
            }

            psInst->dwDerivedSecurityLen = 72;
        }
        else if ( ( psInst->dwMasterSecurityLen == 6                    ) &&
                  ( psInst->dwSecurityType      == CENL7_SECURITY_ALGO2 ) &&
                  ( dwDerivationLen             == 6                    ) )
        {
            dwFoo = 6;
            if ( ! CENL7Algo2Cipher( pbDerivationData, dwDerivationLen, pbDst, &dwFoo, pbKey ) )
                __leave;

            psInst->dwDerivedSecurityLen = 6;
        }
        else
            __leave;

        fReturn = TRUE;
    }
    __finally
    {
        if ( hDesKey1 != NULL )
            DESCloseKey( hDesKey1 );
        if ( hDesKey2 != NULL )
            DESCloseKey( hDesKey2 );

        
    }

	return fReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7GetAccessCredentials(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbNonce,
 *                      IN      DWORD                   dwNonceLen,
 *                      IN      DWORD                   dwKeyRef,
 *                      OUT     BYTE                  * pbAC,
 *                      IN OUT  DWORD                 * pdwACLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbNonce          : Pointe sur les données d'entrée pour le calcul des
 *                                access credentials.
 *             dwNonceLen       : Taille des données d'entrée
 *             dwKeyRef         : Référence à la clé de calcul.
 *                                 - Avec la sécurité DES : de 0 à 8, ou CENL7_DEFAULT.
 *                                 - Avec la sécurité ALGO2 : Non utilisé
 *             pbAC             : Pointe sur le buffer qui récupéra le bloc des access
 *                                credentials.
 *             pdwACLen         : En entrée, taille max du buffer pointé par pbAC.
 *                                En sortie, taille effectivement utilisée par le bloc
 *                                des access credentials.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le calcul des access credentials basé sur les données
 *             de sécurité initialisées par un appel à CENL7SetMasterSecurity() et
 *             dérivées par un appel à CENL7DeriveSecurity().
 *             En mode DES A1, les données Nonce sont constituées d'un
 *             bloc de 4 octets constitué de la dernière valeur Nonce obtenue d'un OBE
 *             (par les paramètres d'une application dans la VST ou par un message
 *             GET_NONCE.RESPONSE).
 *             En mode ALGO2, les données à dériver sont constituées d'un
 *             bloc de 6 octets constitué de l'attribut ContextMark.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7GetAccessCredentials(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbNonce,
        IN      DWORD                   dwNonceLen,
        IN      DWORD                   dwKeyRef,
        OUT     BYTE                  * pbAC,
        IN OUT  DWORD                 * pdwACLen )
{
    BOOL    fReturn = FALSE;
    DWORD   dwUsedKeyRef;
    DWORD   dwFoo;
    BYTE    tbAC[8];
    HDESKEY hDesKey = NULL;

    __try
    {
        if ( ( ( psInst->dwDerivedSecurityLen == 72 ) ||   // 1+8 clé
			   ( psInst->dwDerivedSecurityLen == 8  ) ||   // 1 clé
			   ( psInst->dwDerivedSecurityLen == 16 ) ) && // 1+1 clés -> Bug préservé pour compatibilité
             ( psInst->dwSecurityType       == CENL7_SECURITY_DES ) &&
             ( dwNonceLen                   == 4                  ) )
        {
            if ( *pdwACLen < dwNonceLen )
                __leave;

            if ( dwKeyRef != CENL7_DEFAULT )
                dwUsedKeyRef = ( dwKeyRef % 10 ) % 9;
            else
                dwUsedKeyRef = 0;
            
            hDesKey = DESOpenKey( &psInst->tbDerivedSecurity[8*dwUsedKeyRef], FALSE );
            if ( hDesKey == NULL )
                __leave;

            if ( ! DESEncryptEcb( FALSE, TRUE, pbNonce, 4, tbAC, &dwFoo, hDesKey ) )
                __leave;

            CopyMemory( pbAC, tbAC, 4 );            
            if ( dwKeyRef != CENL7_DEFAULT )
                pbAC[4] = (BYTE)dwUsedKeyRef;
            *pdwACLen = dwNonceLen;
        }
        else if ( ( psInst->dwDerivedSecurityLen == 6                    ) &&
                  ( psInst->dwSecurityType       == CENL7_SECURITY_ALGO2 ) )
        {
            if ( dwNonceLen != 6 )
                __leave;
            if ( *pdwACLen < 6 )
                __leave;

            dwFoo = 6;
            if ( ! CENL7Algo2Cipher( pbNonce, 6, pbAC, &dwFoo, psInst->tbDerivedSecurity ) )
                __leave;

            *pdwACLen = 6;
        }
        else
            __leave;

        fReturn = TRUE;
    }
    __finally
    {
        if ( hDesKey != NULL )
            DESCloseKey( hDesKey );
        
    }

	return fReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7GetAuthenticator(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbData,
 *                      IN      DWORD                   dwDataLen,
 *                      IN      DWORD                   dwKeyRef,
 *                      OUT     BYTE                  * pbMAC,
 *                      IN OUT  DWORD                 * pdwMACLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbData           : Pointe sur les données pour lesquelles un authentificateur
 *                                doit être généré.
 *             dwDataLen        : Taille des données à authentifier.
 *             dwKeyRef         : Référence à la clé de calcul.
 *                                 - Avec la sécurité DES : de 0 à 8, ou CENL7_DEFAULT.
 *                                 - Avec la sécurité ALGO2 : Non utilisé
 *             pbMAC            : Pointe sur le buffer qui récupéra le bloc d'authentification.
 *                                credentials.
 *             pdwMACLen        : En entrée, taille max du buffer pointé par pbMAC.
 *                                En sortie, taille effectivement utilisée par le bloc
 *                                de l'authentificateur.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le calcul d'un authentificateur basé sur les données
 *             de sécurité initialisées par un appel à CENL7SetMasterSecurity() et
 *             dérivées par un appel à CENL7DeriveSecurity().
 *             ATTENTION : Utilisable uniquement en mode DES.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7GetAuthenticator(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbData,
        IN      DWORD                   dwDataLen,
        IN      DWORD                   dwKeyRef,
        OUT     BYTE                  * pbMAC,
        IN OUT  DWORD                 * pdwMACLen )
{
    BOOL    fReturn = FALSE;
    DWORD   dwUsedKeyRef;
    DWORD   dwFoo;
    BYTE    tbSrc[8];
    BYTE    tbDst[8];
    HDESKEY hDesKey = NULL;

    __try
    {
        if ( ( psInst->dwDerivedSecurityLen == 72                 ) &&
             ( psInst->dwSecurityType       == CENL7_SECURITY_DES ) &&
             ( *pdwMACLen                   >= 4                  ) )
        {
            ZeroMemory( tbSrc, sizeof(tbSrc) );
            ZeroMemory( tbDst, sizeof(tbDst) );

            if ( dwKeyRef != CENL7_DEFAULT )
                dwUsedKeyRef = ( dwKeyRef % 10 ) % 9;
            else
                dwUsedKeyRef = 0;

            hDesKey = DESOpenKey( &psInst->tbDerivedSecurity[8*dwUsedKeyRef], FALSE );
            if ( hDesKey == NULL )
                __leave;

            while ( dwDataLen > 0 )
            {
                if ( dwDataLen >= 8 )
                {
                    CopyMemory( tbSrc, pbData, 8 );
                    pbData += 8;
                    dwDataLen -= 8;
                }
                else
                {
                    ZeroMemory( tbSrc, 8 );
                    CopyMemory( tbSrc, pbData, dwDataLen );
                    pbData += dwDataLen;
                    dwDataLen = 0;
                }

                *(ULONGLONG*)tbSrc ^= *(ULONGLONG*)tbDst;
                dwFoo = 8;
                if ( ! DESEncryptEcb( FALSE, TRUE, tbSrc, 8, tbDst, &dwFoo, hDesKey ) )
                    __leave;
            }    

            memcpy( pbMAC, tbDst, 4 );            
            *pdwMACLen = 4;
        }
        else
            __leave;

        fReturn = TRUE;
    }
    __finally
    {
        if ( hDesKey != NULL )
            DESCloseKey( hDesKey );
        
    }

	return fReturn;
}






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7Algo2Cipher(
 *                      IN      BYTE      * pbSrcBlock, 
 *                      IN      DWORD       dwSrcBlockBytes,
 *                      OUT     BYTE      * pbDstBlock,
 *                      IN OUT  DWORD     * pdwDstBlockBytes,
 *                      IN      BYTE      * pbKey )
 * PARAMETERS: pbSrcBlock      : Pointe sur le bloc de données source
 *             dwSrcBlockBytes : Taille du bloc de données source
 *             pbDstBlock      : Pointe sur le bloc de données récupérant le résultat
 *             pdwDstBlockBytes: En entrée, taille max du bloc de destination,
 *                               En sortie, taille effective des données calculées.
 *             pbKey           : Pointeur sur un bloc de 6 octets contenant la clé de chiffrement.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le chiffrement ALGO2 d'un bloc de données. Attention, ce n'est pas
 *             un encryption dans la mesure ou ce chiffrement n'est pas réversible.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7Algo2Cipher(
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      BYTE      * pbKey )
{
    BOOL        fReturn = FALSE;
    BYTE        tbSrc[6];
    BYTE        tbPrm[12];
    BYTE      * pbDst = pbDstBlock;
    DWORD       dwDstLen = *pdwDstBlockBytes;

    __try
    {
        while ( dwSrcBlockBytes > 0 )
        {
            if ( dwDstLen < 6 )
                __leave;

            if ( dwSrcBlockBytes >= 6 )
            {
                CopyMemory( tbSrc, pbSrcBlock, 6 );
                dwSrcBlockBytes -= 6;
            }
            else
            {
                ZeroMemory( tbSrc, 6 );
                CopyMemory( tbSrc, pbSrcBlock, dwSrcBlockBytes );
                dwSrcBlockBytes = 0;
            }
        
            tbPrm[ 0] = tbSrc[3];
            tbPrm[ 1] = tbSrc[4];
            tbPrm[ 2] = pbKey[0];
            tbPrm[ 3] = pbKey[1];
            tbPrm[ 4] = tbSrc[0];
            tbPrm[ 5] = tbSrc[5];
            tbPrm[ 6] = pbKey[2];
            tbPrm[ 7] = pbKey[3];
            tbPrm[ 8] = tbSrc[1];
            tbPrm[ 9] = tbSrc[2];
            tbPrm[10] = pbKey[4];
            tbPrm[11] = pbKey[5];


            *(WORD*)(&tbSrc[0]) = CENL7Algo2Crc( &tbPrm[0], 4 );
            *(WORD*)(&tbSrc[2]) = CENL7Algo2Crc( &tbPrm[4], 4 );
            *(WORD*)(&tbSrc[4]) = CENL7Algo2Crc( &tbPrm[8], 4 );

            pbDst[0] = tbSrc[3];
            pbDst[1] = tbSrc[4];
            pbDst[2] = tbSrc[0];
            pbDst[3] = tbSrc[5];
            pbDst[4] = tbSrc[1];
            pbDst[5] = tbSrc[2];

            pbSrcBlock += 6;
            pbDst += 6;
            dwDstLen -= 6;
        }

        *pdwDstBlockBytes = (DWORD)pbDst - (DWORD)pbDstBlock;

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
 * SYNTAX    : PRIVATE WORD WINAPI CENL7Algo2Crc(
 *                      IN      BYTE      * pbData, 
 *                      IN      DWORD       dwLen )
 * PARAMETERS: pbData : Bloc des données source
 *             dwLen  : Taille de bloc des données source
 * RETURN    : Code de redondance cyclique 16 bits (CRC16) basé sur
 *             le polynome X16+X12+X5+1 avec FFFF comme valeur initiale.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le calcule d'un CRC16.
 * --------------------------------------------------------------------
 */
PRIVATE WORD WINAPI CENL7Algo2Crc(
        IN      BYTE      * pbData, 
        IN      DWORD       dwLen )
{
    DWORD dwByteIdx;
    DWORD dwBitIdx;
    WORD  wCrc;
    WORD  wData;
    WORD  wBit;

    wCrc = 0xFFFF;

    for( dwByteIdx = 0; dwByteIdx < dwLen; dwByteIdx++ )
    {
        wData = (WORD)pbData[dwByteIdx];

        for ( dwBitIdx = 0 ; dwBitIdx < 8 ; dwBitIdx++ )
        {
            wBit = ( wData ^ wCrc ) & 0x0001;
            wCrc = ( wCrc >> 1);
            if ( wBit )
                wCrc = wCrc ^ 0x8408;
            wData = ( wData >> 1 );
        }
    }
    
    wCrc = ~wCrc;

    return wCrc;
}
