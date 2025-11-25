/*------ (v) 2000 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : CRYPT 
 * FICHIER    : CRYPT.C
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Cryptage
 * -----------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE :	28.09.2000. make Dll from lib file %NPL
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */
/*--------------- INCLUDES: ---------------*/
#include <windows.h>
#include <crypt.h>
/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- DEFINE: ---------------*/
#define CRYPT_KEY_FIRST 521
#define CRYPT_KEY_RANGE 1001
/*--------------- TYPEDEFS: ---------------*/
typedef struct CRYPT_SET
{
    unsigned int iFirst;
    unsigned int iLast;
}
CRYPT_SET;
/*--------------- FUNCTIONS: --------------*/
/*------------- VARIABLES: ------------*/
CRYPT_SET gtsSets[] =
{
    {   '0' , '9'    },
    {   'A' , 'Z'    },
    {   'a' , 'z'    },
    {   '\0', '\0'   }
};
/*--------------- CODE: ---------------*/
/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * WINAPI CryptProcessTextString( char * pcString, BOOL bEncrypt )
 * PARAMETERS: pcString : Pointeur sur la chaine à crypter
 * RETURN    : pcString
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Encryption d'un chaine de texte.
 *             Attention, seuls les caractères alphabetiques et numériques
 *             sont touchés.
 * --------------------------------------------------------------------
 */
PRIVATE char * WINAPI CryptProcessTextString( char * pcString, BOOL bEncrypt )
{
    unsigned int iKey = 3;
    int iSet;
    unsigned char * pcScan;

    for ( pcScan = (unsigned char*)pcString ; *pcScan != (unsigned char)0 ; pcScan ++ )
        for ( iSet = 0 ; gtsSets[iSet].iFirst != 0 ; iSet ++ )
        {
            unsigned int iCur = (unsigned int)*pcScan;
            unsigned int iCod;
            if ( ( iCur >= gtsSets[iSet].iFirst ) && ( iCur <= gtsSets[iSet].iLast ) )
            {
                unsigned int iRange = gtsSets[iSet].iLast - gtsSets[iSet].iFirst + (unsigned char )1;
                unsigned int iRKey = ( iKey % ( iRange - 1 ) ) + 1;
                if ( ! bEncrypt )
                    iRKey = iRange - iRKey;
                iCod = ( ( ( iCur - gtsSets[iSet].iFirst ) + iRKey ) % iRange ) + gtsSets[iSet].iFirst;
                iKey = ( ( iKey + CRYPT_KEY_FIRST + ( bEncrypt ? iCur : iCod ) ) % CRYPT_KEY_RANGE ) + 1;
                *pcScan = (unsigned char)iCod;
                break;
            }
        }

    return pcString;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC char * WINAPI fnEncrypt( char * szData )
 * PARAMETERS: szData : Pointer to the string to encript
 * RETURN    : szData
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Encryption d'un chaine de texte.
 *             Attention, seuls les caractères alphabetiques et numériques
 *             sont touchés.
 * --------------------------------------------------------------------
 */
EXPORT char * WINAPI fnEncrypt( char * szData )
{
    return CryptProcessTextString( szData, TRUE );
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * WINAPI fnDecrypt( char * szData )
 * PARAMETERS: szData: Pointeur to the string to decrypt
 * RETURN    : szData
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Decryption d'un chaine de texte.
 *             Attention, seuls les caractères alphabetiques et numériques
 *             sont touchés.
 * --------------------------------------------------------------------
 */
EXPORT char * WINAPI fnDecrypt( char * szData )
{
    return CryptProcessTextString( szData, FALSE );
}
/*---------------------------- END OF FILE -------------------------*/
