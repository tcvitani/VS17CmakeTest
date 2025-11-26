/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CENL7
 * FILE       : CENL7.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Implémentation de la couche 7 DSRC (norme CEN)
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


//
// The file MEMCLASS.H must be included at the last position
//
#include <memclass.h>





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT CENL7_INSTANCE * WINAPI CENL7Open()
 * PARAMETERS: Aucun
 * RETURN    : Un handle de la nouvelle instance créée ou NULL en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Mise en place d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un bagde.
 * --------------------------------------------------------------------
 */
EXPORT CENL7_INSTANCE * WINAPI CENL7Open()
{
    CENL7_INSTANCE * psInst;

    psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );

    return psInst;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7SetMode( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwModeFlags )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             dwModeFlags : Options de l'instance
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Définit le mode de fonctionnement de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7SetMode( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwModeFlags )
{
    if ( ( dwModeFlags & ~CENL7_FLAG_MASK ) == 0 )
    {
        psInst->dwFlags = dwModeFlags;
        return TRUE;
    }
    else
        return FALSE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7RegisterDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId,
 *                      IN      CENL7_TYPE_DECODER    * pfDecoder,
 *                      IN      DWORD                   dwUserData )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 *             pfDecoder   : Pointeur sur la fonction de décodage.
 *             dwUserData  : User data à passer en parametre des callback à pfDecoder
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un décodeur spécial pour un type de container
 *             particulier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7RegisterDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId,
        IN      CENL7_TYPE_DECODER    * pfDecoder,
        IN      DWORD                   dwUserData )
{
    if ( ( ( bContId & 0x80 ) == 0 ) && ( pfDecoder != NULL ) )
    {
        if ( psInst->tpfDecoders[bContId] == NULL )
        {
            psInst->tpfDecoders[bContId] = pfDecoder;
            psInst->tdwDecoders[bContId] = dwUserData;
            return TRUE;
        }
        else
            return FALSE;
    }
    else
        return FALSE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7UnregisterDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Supprimer un décodeur spécial pour un type de container
 *             particulier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7UnregisterDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId )
{
    if ( ( bContId & 0x80 ) == 0 )
    {
        if ( psInst->tpfDecoders[bContId] != NULL )
        {
            psInst->tpfDecoders[bContId] = NULL;
            psInst->tdwDecoders[bContId] = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    else
        return FALSE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7GetDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId )
 *                         OUT  CENL7_TYPE_DECODER   ** ppfDecoder,
 *                         OUT  DWORD                 * pdwUserData )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 *             ppfDecoder   : retourne un pointeur sur la fonction de décodage. Use NULL to unregister.
 *             pdwUserData  : retourne les user data associée
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir les information sur un décodeur enregistré
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7GetDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId,
           OUT  CENL7_TYPE_DECODER   ** ppfDecoder,
           OUT  DWORD                 * pdwUserData )
{
    if ( ( bContId & 0x80 ) == 0 )
    {
        *ppfDecoder = psInst->tpfDecoders[bContId];
        *pdwUserData = psInst->tdwDecoders[bContId];
        return TRUE;
    }
    else
        return FALSE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7Close(
 *                      IN OUT  CENL7_INSTANCE         * psInst )
 * PARAMETERS: psInst : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un badge.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7Close(
        IN OUT  CENL7_INSTANCE         * psInst )
{
    HeapFree( GetProcessHeap(), 0, psInst );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7GetRecordSize(
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE               * pbBuf,
 *                      IN      DWORD                dwBufBytes,
 *                      OUT     DWORD              * pdwRecBytesWithoutContainer
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             pbBuf      : Buffer contenant des données
 *             dwBufBytes : Taille totale des données
 *             pdwRecBytesWithoutContainer : Usefull data size without
 *                          container related data. NULL si non utilisé.
 * RETURN    : Taille du premier bloc de données dans le buffer.
 * --------------------------------------------------------------------
 * ROLE      : Calcul la taille du premier bloc de données dans le buffer,
 *             le premier octet étant utilisé comme un type ASN1.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI CENL7GetRecordSize(
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE               * pbBuf,
        IN      DWORD                dwBufBytes,
        OUT     DWORD              * pdwRecBytesWithoutContainer )
{
    BOOL            fResult = FALSE;
    BYTE            bType;
    BYTE            bCont;
    DWORD           dwSize;
    DWORD           dwNb;
    DWORD           dwSubSize;
    DWORD           dwIndex;
    DWORD           dwNoContainer;

    __try
    {

        if ( ! BITBL_IS_VALID( dwBufBytes ) )
            __leave;

        if ( dwBufBytes <= 0 )
            __leave;

        bType = pbBuf[0];
        if ( ( bType & 0x80 ) != 0 )
            __leave;

        //
        // Si c'est un type spécial nécessitant un décodage particulier
        //
        if ( psInst->tpfDecoders[bType] != NULL )
        {
            dwSize = (*(psInst->tpfDecoders[bType]))( psInst->tdwDecoders[bType], pbBuf, dwBufBytes, &dwNoContainer );
            if ( dwSize != CENL7_INVALID )
            {
                if ( pdwRecBytesWithoutContainer != NULL )
                    *pdwRecBytesWithoutContainer = dwNoContainer;
                fResult = TRUE;
                __leave;
            }
        }

        dwSize = 1;

        bCont = psInst->tbContainers[bType];
        if ( bCont == 0 )
        {
            switch ( bType )
            {

            case CENL7_CHOICE_INTEGER :
                if ( ( psInst->dwFlags & CENL7_FLAG_INTEGER_ITERATION ) != 0 )
                {
                    //
                    // Codage (comme une octet string) :
                    //    + NbIter
                    //    + ( Octet * NbIter )
                    //
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // NbIter
                    dwNoContainer = (DWORD)pbBuf[dwSize];
                    dwSize ++;
                }
                else
                {
                    //
                    // Codage :
                    //    + Octet
                    //
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // Octet
                    dwNoContainer = 1;
                }
                dwSize += dwNoContainer;
                break;

            case CENL7_CHOICE_OCTET_STRING :
            case CENL7_CHOICE_ATTRIBUTE_ID_LIST :
                //
                // Codage :
                //    + NbIter                    ou + NbIter
                //    + ( Octet * NbIter )           + ( AttrId * NbIder )
                //
                // REMARQUE : Dans le cas de la liste d'attribut, un identifiant d'un
                // attribut est codé sur 1 octet, on est donc équivalent à une octet string.
                //
                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter
                dwNoContainer = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // Octet * NbIter
                dwSize += dwNoContainer;

                break;

            case CENL7_CHOICE_ATTRIBUTE_LIST :
                //
                // Codage :
                //    + NbIter
                //    + ( ( AttrId + AttrContainer ) * NbIter )
                //
                // ATTENTION : les attributs sont de taille variable, on doit donc parcourir
                // la liste et appeler la fonction récursivement.
                //
                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                for ( dwIndex = 0 ; dwIndex < dwNb ; dwIndex ++ )
                {
                    // AttrId
                    dwSize ++;
                
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // AttrContainer (appel récursif)
                    dwSubSize = CENL7GetRecordSize( psInst, pbBuf + dwSize, dwBufBytes - dwSize, NULL );
                    if ( dwSubSize == CENL7_INVALID )
                        __leave;

                    dwSize += dwSubSize;
                }

                dwNoContainer = dwSize - 1;
                break;

            case CENL7_CHOICE_GET_STAMPED_RQ :
                //
                // Codage (commence comme une liste d'id d'attributs, suivie d'une octet
                // string et d'un octet) :
                //    + NbIter1
                //    + ( AttrId * NbIter1 )
                //    + NbIter2
                //    + ( Octet * NbIter2 )
                //    + Octet
                //
                // ATTENTION : les attributs sont de taille variable, on doit donc parcourir
                // la liste et appeler la fonction récursivement.
                //
                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter1
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // AttrId * NbIter1
                dwSize += dwNb;

                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter2
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // Octet * NbIter2
                dwSize += dwNb;

                // Octet
                dwSize ++;

                dwNoContainer = dwSize - 1;
                break;

            case CENL7_CHOICE_GET_STAMPED_RS :
                //
                // Codage (commence comme une liste d'attributs, suivie d'une octet string):
                //    + NbIter1
                //    + ( ( AttrId + AttrContainer ) * NbIter1 )
                //    + NbIter2
                //    + ( Octet * NbIter2 )
                //
                // ATTENTION : les attributs sont de taille variable, on doit donc parcourir
                // la liste et appeler la fonction récursivement.
                //
                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter1
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                for ( dwIndex = 0 ; dwIndex < dwNb ; dwIndex ++ )
                {
                    // AttrId
                    dwSize ++;
                
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // AttrContainer (appel récursif)
                    dwSubSize = CENL7GetRecordSize( psInst, pbBuf + dwSize, dwBufBytes - dwSize, NULL );
                    if ( dwSubSize == CENL7_INVALID )
                        __leave;

                    dwSize += dwSubSize;
                }


                if ( dwSize >= dwBufBytes )
                    __leave;

                // NbIter2
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // Octet * NbIter2
                dwSize += dwNb;

                dwNoContainer = dwSize - 1;
                break;

            case CENL7_CHOICE_CHANNEL_RQ :
            case CENL7_CHOICE_CHANNEL_RS :
                if ( ( psInst->dwFlags & CENL7_FLAG_SUB_STRING_CHANNEL ) != 0 )
                {
                    //
                    // Codage :
                    //    + ChannelId
                    //    + DataContainer
                    //
                    // ATTENTION : Le container est de taille variable, on doit donc appeler
                    // la fonction récursivement.
                    //
                
                    // ChannelId
                    dwSize ++;
            
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // DataContainer
                    dwSubSize = CENL7GetRecordSize( psInst, pbBuf + dwSize, dwBufBytes - dwSize, NULL );
                    if ( dwSubSize == CENL7_INVALID )
                        __leave;

                    dwSize += dwSubSize;
                }
                else
                {
                    //
                    // Codage :
                    //    + ChannelId
                    //    + NbIter
                    //    + ( Octet * NbIter )
                    //
                
                    // ChannelId
                    dwSize ++;
            
                    if ( dwBufBytes <= dwSize )
                        __leave;

                    // NbIter
                    dwNb = (DWORD)pbBuf[dwSize];
                    dwSize ++;

                    // Octet * NbIter
                    dwSize += dwNb;
                }

                dwNoContainer = dwSize - 1;
                break;

            case CENL7_CHOICE_TG_PERSO_APP_RQ :
                if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) == 0 )
                    __leave;

                //
                // Codage :
                //    + Mode
                //    + Total number
                //    + Block number
                //    + Block bytes
                //    + Block
                //
                // ATTENTION : Le container est de taille variable, on doit donc appeler
                // la fonction récursivement.
                //
                
                // On passe mode, total number, block number
                dwSize += 3;
                if ( dwBufBytes <= dwSize )
                    __leave;

                // Block bytes
                dwNb = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // Block
                dwSize += dwNb;

                dwNoContainer = dwSize - 1;
				break;

            case CENL7_CHOICE_TG_PERSO_APP_RS :
                if ( ( psInst->dwFlags & CENL7_FLAG_TG_PRIVATE ) == 0 )
                    __leave;

                //
                // Codage :
                //    + Block number
                //    + Block status
                //
                // ATTENTION : Le container est de taille variable, on doit donc appeler
                // la fonction récursivement.
                //
                dwSize += 2;
                dwNoContainer = dwSize - 1;
				break;

			case CENL7_CHOICE_VLPN:
				//
				// Codage :
				//    + ContryCode + Alphabet (10+6 bits = 2 bytes 
				//    + Size
				//    + Data
				//
				dwSize += 2; // ( country code + alphabet )
				if (dwBufBytes <= dwSize)
					__leave;

				//
				// Size
				//
				dwNb = (DWORD)pbBuf[dwSize];
				dwSize++;

				// Data
				dwSize += dwNb;

				dwNoContainer = dwSize - 1;
				break;


            default :
                __leave;
            }
        }
        else
        {
            if ( ( bCont & CONTAINER_CODED_SIZE ) != 0 )
            {
                if ( dwBufBytes <= dwSize )
                    __leave;

                // NbIter
                dwNoContainer = (DWORD)pbBuf[dwSize];
                dwSize ++;

                // Octet * NbIter
                dwSize += dwNoContainer;
            }
            else
            {
                // NbIter
                dwNoContainer = (DWORD)bCont & CONTAINER_SIZE_MASK;

                // Octet * NbIter
                dwSize += dwNoContainer;
            }
        }

        if ( dwBufBytes < dwSize )
            __leave;

        if ( pdwRecBytesWithoutContainer != NULL )
            (*pdwRecBytesWithoutContainer) = dwNoContainer;

        fResult = TRUE;
    }
    __finally
    {
		;
    }

    return ( fResult ? dwSize : CENL7_INVALID );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7GetBitRecordSize(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN      BYTE              * pbBuf,
 *                      IN      DWORD               dwBitPos,
 *                      IN      DWORD               dwBufBits,
 *                      OUT     DWORD             * pdwRecBitsWithoutCountainer )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             pbBuf      : Buffer contenant des données
 *             dwBitPos   : Position du premier bit
 *             dwBufBytes : Taille du buffer en bits, à partir du premier bit.
 *             pdwRecBitsWithoutCountainer : Usefull data size without
 *                          container related data. NULL si non utilisé.
 * RETURN    : Taille du premier bloc de données dans le buffer, en bits
 * --------------------------------------------------------------------
 * ROLE      : Calcul la taille du premier bloc de données dans le buffer,
 *             le premier octet étant utilisé comme un type ASN1.
 *             Si le type n'est pas connu de la librairie, le bloc est soumis
 *             aux fonctions GBS définies avec CENL7AddGBSFunction.
 *             Le premier octet est choisi en fonction de dwBitPos.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI CENL7GetBitRecordSize(
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE               * pbBuf,
        IN      DWORD                dwBitPos,
        IN      DWORD                dwBufBits,
        OUT     DWORD              * pdwRecBitsWithoutCountainer )
{
    BOOL        fResult = FALSE;
    DWORD       dwSize;
    DWORD       dwNoContainer;

    __try
    {
        if ( ! BITBL_IS_VALID( dwBitPos ) )
            __leave;

        if ( ( dwBitPos % 8 ) != 0 )
            __leave;

        dwSize = CENL7GetRecordSize( psInst, pbBuf + ( dwBitPos / 8 ), dwBufBits / 8, &dwNoContainer);
        if ( dwSize == CENL7_INVALID )
            __leave;

        if ( pdwRecBitsWithoutCountainer != NULL )
            (*pdwRecBitsWithoutCountainer) = ( dwNoContainer * 8 );
        fResult = TRUE;
    }
    __finally
    {
		;
    }

	return (fResult ? dwSize * 8 : CENL7_INVALID);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7ClearAttributDefinitions(
 *                      IN      CENL7_INSTANCE     * psInst )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Remet à zéro la table de définition des attributs
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7ClearAttributDefinitions(
        IN      CENL7_INSTANCE     * psInst )
{
    ZeroMemory( psInst->tbAttributes, sizeof(psInst->tbAttributes) );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7ClearContainerDefinitions(
 *                      IN      CENL7_INSTANCE     * psInst )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Remet à zéro la table de définition des container ASN1
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7ClearContainerDefinitions(
        IN      CENL7_INSTANCE     * psInst )
{
    ZeroMemory( psInst->tbContainers, sizeof(psInst->tbContainers) );
    ZeroMemory( psInst->tpfDecoders, sizeof(psInst->tpfDecoders) );
    ZeroMemory( psInst->tdwDecoders, sizeof(psInst->tdwDecoders) );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7DefineAttribute( 
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE                 bAttrId,
 *                      IN      BYTE                 bContId )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             bAttrId    : Identifiant de l'attribut à définir (0 à 127)
 *             bContId    : Identifiant de container ASN1.
 * RETURN    : TRUE  : OK
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini le container ASN1 associé à un attribut.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7DefineAttribute( 
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE                 bAttrId,
        IN      BYTE                 bContId )
{
    BOOL fResult = FALSE;

    __try
    {
        if ( ( ( bAttrId & 0x80 ) != 0 ) || 
             ( ( bContId & 0x80 ) != 0 ) )
            __leave;

        psInst->tbAttributes[bAttrId] = bContId;
        fResult = TRUE;
    }
    __finally
    {
		;
    }

	return fResult;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7DefineContainer( 
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE                 bContId,
 *                      IN      BOOL                 fCodedSize,
 *                      IN      BYTE                 bBytesLen )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             bContId    : Identifiant de container ASN1.
 *             bContId    : Identifiant de container ASN1.
 *             fCodedSize : TRUE : La taille des données est codée systématiquement
 *                                 avec les données elle-mêmes.
 *                          FALSE: La taille des données est implicite.
 *             bBytesLen  : Taille implicite des données (n'est exploité que si 
 *                          fCodedSize vaut FALSE.
 * RETURN    : TRUE  : OK
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les codage d'un container ASN1.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7DefineContainer( 
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE                 bContId,
        IN      BOOL                 fCodedSize,
        IN      BYTE                 bBytesLen )
{
    BOOL fResult = FALSE;

    __try
    {
        if ( ( ( bContId   & 0x80 ) != 0 ) ||
             ( ( bBytesLen & 0x80 ) != 0 ) )
            __leave;

        psInst->tbContainers[bContId] = bBytesLen | ( fCodedSize ? 0x80 : 0x00 );
        fResult = TRUE;
    }
    __finally
    {
		;
    }

	return fResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7Decoder4VehicleLicensePlateNumber(
 *                      IN      BYTE               * pbBufWithContainer,
 *                      IN      DWORD                dwBufBytes,
 *             OPTIONAL OUT     DWORD              * pdwRecBytesWithoutContainer )
 * PARAMETERS: cf. CENL7_TYPE_DECODER
 * RETURN    : cf. CENL7_TYPE_DECODER
 * --------------------------------------------------------------------
 * ROLE      : CENL7_TYPE_DECODER - compliant function that can be used
 *             to decode variable-sized VehicleLicensePlateNumber in
 *             compliance with ISO14906. To do used in a call to
 *             CENL7RegisterDecoder.
 * --------------------------------------------------------------------
 */
/*
   Not to be defined here, shall be defined in the assembly that uses
   the library
 */
/*
EXPORT DWORD WINAPI CENL7Decoder4VehicleLicensePlateNumber(
                 IN      DWORD                dwUserData,
                 IN      BYTE               * pbBufWithContainer,
                 IN      DWORD                dwBufBytes,
        OPTIONAL OUT     DWORD              * pdwRecBytesWithoutContainer )
{
    DWORD dwLen = CENL7_INVALID;

    __try
    {
        //
        // 4 is the minimum size require for the container and its associated data
        //
        if ( dwBufBytes < 4 )
            __leave;
    
        //
        // Byte 0 is the container code, we just verify this is the one the
        // function is supposed to decode.
        //
        if ( pbBufWithContainer[0] != 0x2F ) // VehicleLicensePlateNumber container
            __leave;

        //
        // Bytes 1 and 2 are coding a county (10bits) and an alphabet indicator (6 bits)
        // so we just skip them
        //

        //
        // Byte 3 is coding the size of a string (we just have to verify it fits in the
        // buffer passed in parameters. Other size verifications will have to be made
        // elsewhere if necessary.
        //
        if ( ((DWORD)(pbBufWithContainer[3]) + 4 ) > dwBufBytes )
            __leave;

        dwLen = ((DWORD)(pbBufWithContainer[3]) + 4 );
    }
    __finally
    {
        if ( dwLen != CENL7_INVALID )
            if ( pdwRecBytesWithoutContainer != NULL )
                *pdwRecBytesWithoutContainer = dwLen - 1;
    }

    return ((DWORD)(pbBufWithContainer[3]) + 4 );
}

 */


