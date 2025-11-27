/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base utilisant les string
* FICHIER: STR.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module
*            - encapsule les fonctions de base ( ex:strcpy) en ajoutant
*              des garde-fous
*            - fournit des fonctions de tests en plus
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
*              Toutes ces fonctions en cas d'‚chec partent en
*              Erreur Fatale ( sauf si elles ne peuvent ‚chouer)
* --------------------------------------------------------------------
* NOTA: La plupart des fonctions ( ex: STR_strcat) existe avec l'extension
*       '_s'.
*       Cette extension signifie que la fonction prend comme taille
*       le sizeof du parametre destination.
*ATTENTION: il ne faut pas faire d'increment dans le parametre
*           ex: STR_strcpy( tab[i++], "") -> double increment
*ATTENTION: cela ne marche que si le parametre est un tableau, car si c'est
*           un pointeur, le sizeof donnera 4 octets
* --------------------------------------------------------------------
* NOTA: La fonction strcmp n'est pas encapsul‚ car:
*       - il n'a pas de danger en cas de d‚passement m‚moire, puisqu'on
*         ne fait que de la lecture
*       - on est suppos‚ avoir au pr‚alable v‚rifi‚ la taille
*       - on ne connait pas forcement la taille des chaines qu'on
*         compare.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/STR.H_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:46   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:42:04   bph
 *  
 * 
 *    Rev 1.5   16 Jan 1998 11:04:02   DPI
 * Generalisation de '_s'
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef STR_H
#define STR_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef STR_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- TYPEDEF ---------------*/

/*--------------- FUNCTIONS: --------------*/


/* la fonction STR_Verify v‚rifie … posteriori si la chaine n'est pas
 * plus longue que sa Taille autoris‚e.
 * Ex: lors de l'emploi du sscanf, du sprintf, ...
 */
#define  STR_verify_s( s)\
                           STR_verify( sizeof(s), s)
#define  STR_verify( t, s)\
                           STR_FullVerify( __FILE__, __LINE__,\
                                           t, s)

EXPORT void    WINAPI STR_FullVerify( char *__FILE, int __LINE,
                            int Taille, char *Source);

/* strcpy */
// la fonction STR_strcpy_s conserve la signature int‚grale de strcpy
// et prends comme taille le sizeof du 1er parametre
#define STR_strcpy_s( d, s)\
                             STR_strcpy( sizeof(d), d, s)
#define STR_strcpy( t, d, s)\
                             STR_FullStrcpy( __FILE__, __LINE__,\
                                             t, d, s)

EXPORT char   *WINAPI STR_FullStrcpy( char *__FILE, int __LINE,
                        int Taille, char *Dest, char *Source);

/* strncpy */
/* cette fonction garantit que la chaine Dest est bien NULL termin‚e */
#define STR_strncpy( t, d, s)\
                             STR_FullStrncpy( __FILE__, __LINE__,\
                                             t, d, s)

EXPORT char   *WINAPI STR_FullStrncpy( char *__FILE, int __LINE,
                        int Taille, char *Dest, char *Source);

/* strcat */
#define STR_strcat_s( d, s)\
                             STR_strcat( sizeof(d), d, s)
#define STR_strcat( t, d, s)\
                             STR_FullStrcat( __FILE__, __LINE__,\
                                             t, d, s)

EXPORT char  *WINAPI STR_FullStrcat( char *__FILE, int __LINE,
                       int Taille, char *Dest, char *Source);

/* strlen */
#define STR_strlen_s( s)\
                             STR_strlen( sizeof(s), s)
#define STR_strlen( t, s)\
                             STR_FullStrlen( __FILE__, __LINE__,\
                                             t, s)

EXPORT int  WINAPI STR_FullStrlen( char *__FILE, int __LINE,
                            int Taille, char *Source);


#endif
