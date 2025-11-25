/*------ (v) 2000 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : CRYPT 
 * FICHIER    : CRYPT.H
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Cryptage
 * -----------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE : 28.09.2000 Make an Dll from the lib - NPL
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */
#ifndef CSR_CRYPT_H
#define CSR_CRYPT_H
/*--------------- INCLUDES: ---------------*/
/*--------------- RESERVED: ---------------*/
#ifdef CRYPT_DEF
#include <public.h>
#else
#include <export.h>
#endif
/*--------------- DEFINE: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: --------------*/
/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * WINAPI fnEncrypt( char * szData )
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
EXPORT char * WINAPI fnEncrypt( char * szData );

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * WINAPI fnDecrypt( char * szData )
 * PARAMETERS: pcString : Pointeur sur la chaine à crypter
 * RETURN    : pcString
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Decryption d'un chaine de texte.
 *             Attention, seuls les caractères alphabetiques et numériques
 *             sont touchés.
 * --------------------------------------------------------------------
 */
EXPORT char * WINAPI fnDecrypt( char * szData );
#endif

