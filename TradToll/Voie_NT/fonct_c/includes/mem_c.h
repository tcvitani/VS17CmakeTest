/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base utilisant la memoire
* FICHIER: MEM.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module
*            - encapsule les fonctions de base ( ex:malloc) en ajoutant
*              des garde-fous
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
*              Toutes ces fonctions en cas d'‚chec partent en
*              Erreur Fatale ( sauf si elles ne peuvent ‚chouer)
*              - La fonction 'free' est encapsul‚e de maniŠre … positionner
*                le pointeur lib‚r‚e … NULL.
*                Sa signature est donc diff‚rente, entre autre 'char **'
*                car le compilo n'aime pas 'void **'
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/MEM_C.H_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:46   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:42:04   bph
 *  
 * 
 *    Rev 1.4   02 Dec 1997 17:14:06   DPI
 * Ajout de la fonction 
 * MEM_GetMaxFreeMemoryBlock
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef MEM_H
#define MEM_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef MEM_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- TYPEDEF ---------------*/

/*--------------- FUNCTIONS: --------------*/

/* malloc */
#define  MEM_malloc( t)\
                           MEM_FullMalloc( __FILE__, __LINE__,\
                                           t)

EXPORT void    *WINAPI MEM_FullMalloc( char *__FILE, int __LINE,
                               int taille);

/* calloc */
#define  MEM_calloc( n, t)\
                           MEM_FullMalloc( __FILE__, __LINE__,\
                                           n, t)

EXPORT void    *WINAPI MEM_FullCalloc( char *__FILE, int __LINE,
                               int nb, int taille);
/* realloc */
#define  MEM_realloc( p, t)\
                           MEM_FullMalloc( __FILE__, __LINE__,\
                                           p, t)

EXPORT void    *WINAPI MEM_FullRealloc( char *__FILE, int __LINE,
                                 void *Ptr, int taille);

/* free
 * ATTENTION, la signature de la fonction FullFree est diff‚rente.
 * En effet, la variable contenant le pointeur lib‚r‚ est remise … NULL
 */
#define  MEM_free( p)\
                           MEM_FullFree( __FILE__, __LINE__,\
                                         &(( char*) p))
EXPORT void WINAPI MEM_FullFree( char *__FILE, int __LINE, char **Ptr);

/* Retourne la diff‚rence entre le nombre de Malloc,Calloc et le nombre de Free */
EXPORT signed long WINAPI MEM_NbAllocation( void);

/* d‚termine la taille memoire du plus gros bloc allouable */
EXPORT unsigned long WINAPI MEM_GetMaxFreeMemoryBlock(void);

#endif