/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base de gestion d'affichage
* FICHIER: TRACE.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module fournit des fonctions de gestion d'affichage.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier de code
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/TRACE.C_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:42:14   bph
 *  
 * 
 *    Rev 1.0   Mar 05 1997 15:14:54   HMO
 * Initial revision
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

//#define TRACE_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>

#include "trace.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: --------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- FUNCTIONS: --------------*/
PRIVATE void vprintf_defaut( const char *format, va_list arg_list);

/*--------------- VARIABLES: --------------*/

static TRC_TAffichage FonctionAffichage = vprintf_defaut;


TRC_TAffichage TRC_DefinirTraitementAffichage( TRC_TAffichage Fonction)
{
   TRC_TAffichage    retour;

   if( FonctionAffichage == vprintf_defaut)
       retour = NULL;
   else
       retour = FonctionAffichage;

   if( Fonction != NULL)
       FonctionAffichage = Fonction;
   else
       FonctionAffichage = vprintf_defaut;

   return( retour);
}


PUBLIC void TRC_printf ( const char *format, ...)
{
   va_list arg_list;

   va_start( arg_list, format);
   FonctionAffichage( format, arg_list);
   va_end( arg_list);

}



/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/


PRIVATE void vprintf_defaut( const char *format, va_list arg_list)
{
   vprintf( format, arg_list);
}
