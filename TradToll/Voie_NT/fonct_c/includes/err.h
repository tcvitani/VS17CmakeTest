/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base de gestion d'erreurs
* FICHIER: ERR.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module fournit des fonctions de gestion d'erreur.
* --------------------------------------------------------------------
* DESCRIPTION:
*           - Il est possible de traiter une erreur fatale sans avoir au
*             pr‚alable appeler ERR_DefinirTraitementFatale.
*             Dans ce cas, c'est un traitement par d‚faut qui est appel‚.
*
*           - Il est possible de changer dynamiquement le traitement
*             d'erreur, en rappelant ERR_DefinirTraitementFatale, lequel
*             retourne les anciens paramŠtre et fonctions d'erreur.
*
*           - Lors d'une erreur fatale, il est possible d'appeler
*             ERR_ErreurFatale, qui prend le __FILE et __LINE du fichier
*             d'appel, ou
*             ERR_TraiterErreurFatale, qui permet de renseigner les __FILE
*             et __LINE d'un fichier appelant.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/ERR.H_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:44   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:42:02   bph
 *  
 * 
 *    Rev 1.9   17 Mar 1998 18:12:50   DPI
 * Ajout des fonctions FIC_Floppy
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ERR_H
#define ERR_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/

#ifdef ERR_DEF
   #include <public.h>
#else
   #include <export.h>
#endif

#include <windows.h>

/*--------------- VERSION ---------------*/


/*--------------- TYPEDEF ---------------*/
/* la macro ci-dessous est utilis‚ pour eviter les warnings quand
 * on n'utilise pas tous les paramŠtres de la fonction
 */
#define NO_WARNING(t)     t=t

/*--------------- TYPEDEF ---------------*/

typedef   void    (*ERR_TErreurFatale)(  char   *__FILE,
                                         int    __LINE,
                                         void   *Param);

/*--------------- FUNCTIONS: --------------*/

/* Si Fonction vaut NULL, alors on utilise le traitement d'erreur
 * par d‚faut.
 */
EXPORT ERR_TErreurFatale WINAPI ERR_DefinirTraitementFatale(
                                   ERR_TErreurFatale Fonction,
                                   void              *Param,
                                   void              **OldParam);

/* Pour d‚clencher le traitement d'erreur FATALE */
#define ERR_EstVrai( t)      if( !(t)) ERR_ErreurFatale()

#define ERR_ErreurFatale()\
                           ERR_TraiterErreurFatale( __FILE__, __LINE__)
EXPORT void  WINAPI ERR_TraiterErreurFatale( char *__FILE, int __LINE);

/*--------------------------------------------------------------*/
/* Traitement relatif … la suirveillance du pointeur NULL       */

/* Le contenu du pointeur NULL est retourn‚ */
//EXPORT long          ERR_SauvePointeurNull( void);

/* Le contenu du pointeur NULL est utilis‚ pour la v‚rification */
/* FALSE est retourn‚ s'il y a eu une modification du pointeur */
//EXPORT boolean       ERR_TestPointeurNull( long AdresseNulle);

#endif
