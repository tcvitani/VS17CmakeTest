/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base de gestion d'affichage
* FICHIER: TRACE.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module fournit des fonctions de gestion d'affichage.
* --------------------------------------------------------------------
* DESCRIPTION:
*           - Il est possible de traiter un affichage sans avoir au
*             pr‚alable appeler TRC_DefinirTraitementAffichage.
*             Dans ce cas, c'est un traitement par d‚faut qui est appel‚.
*
*           - Il est possible de changer dynamiquement le traitement
*             d'affichage, en rappelant TRC_DefinirTraitementAffichage,
*             lequel retourne l'anciene fonction d'affichage.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/TRACE.H_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:48   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:42:06   bph
 *  
 * 
 *    Rev 1.0   Mar 05 1997 15:15:00   HMO
 * Initial revision
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef TRACE_H
#define TRACE_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#include <stdarg.h>
#ifdef TRACE_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- TYPEDEF ---------------*/

/*--------------- TYPEDEF ---------------*/

typedef void (*TRC_TAffichage)( const char *format, va_list arg_list);

/*--------------- FUNCTIONS: --------------*/

/* Si Fonction vaut NULL, alors on utilise l'affichage par d‚faut. */
EXPORT TRC_TAffichage WINAPI TRC_DefinirTraitementAffichage( TRC_TAffichage Fonction);

EXPORT void WINAPI TRC_printf ( const char *format, ...);


#endif
