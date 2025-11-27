/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base de gestion d'erreurs
* FICHIER: ERR.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module fournit des fonctions de gestion d'erreur.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier de code
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/ERR.C_v  $
 * 
 *    Rev 1.2   13 Dec 1999 16:24:50   afx
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:42:10   bph
 *  
 * 
 *    Rev 1.4   Jul 28 1997 16:29:18   DPI
 * Gestion de la version de la librairie
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#include <noyau.h>

#include "err.h"

/*--------------- RESERVED: ---------------*/
           
#include "memclass.h"

/*--------------- EXTERNALS: --------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- VERSION: --------------*/


/*--------------- FUNCTIONS: --------------*/
PRIVATE void    Defaut_TraiterErreurFatale(  char *__FILE,
                                              int __LINE,
                                              void *Param);

/*--------------- VARIABLES: --------------*/

PRIVATE ERR_TErreurFatale    ErreurFatale = Defaut_TraiterErreurFatale;
PRIVATE void                 *ParamErreurFatale = NULL;



PUBLIC  ERR_TErreurFatale WINAPI ERR_DefinirTraitementFatale(
                                   ERR_TErreurFatale Fonction,
                                   void              *Param,
                                   void              **OldParam)
{
   ERR_TErreurFatale    retour;

   if( ErreurFatale == Defaut_TraiterErreurFatale)
       retour = NULL;
   else
       retour = ErreurFatale;

   if( Fonction != NULL)
       ErreurFatale = Fonction;
   else
       ErreurFatale = Defaut_TraiterErreurFatale;

   if( OldParam != NULL)
      *OldParam = ParamErreurFatale;
   ParamErreurFatale = Param;

   return( retour);
}

PUBLIC void WINAPI ERR_TraiterErreurFatale( char *__FILE, int __LINE)
{
  //NO_WARNING( what);
  ErreurFatale( __FILE, __LINE, ParamErreurFatale);
}



/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/




PRIVATE void    Defaut_TraiterErreurFatale(  char *__FILE,
                                              int __LINE,
                                              void *Param)
{
	FILE *pf;
	errno_t err;

   Param = Param;

   _flushall();
   _fcloseall();
   printf("%s %d\n", __FILE, __LINE);

   err = fopen_s(&pf, "fonct_c.err", "a+t");
   if (pf != NULL)
   {
	   fprintf (pf,"ERROR line %6u file %s", __LINE, __FILE);
	   fclose (pf);
   }

   ExitBad();
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC long ERR_Sauve_pointeur_nul( void)
* PARAMETRES:
* RETOUR: Les info en long a l'adresse nulle
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Protection sur les pointeurs nuls
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
/*
PUBLIC long ERR_SauvePointeurNull( void)
{
   long *ad_nulle = MK_FP(0, 0);

   return( *ad_nulle);
}
*/

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean ERR_Test_pointeur_nul( long AdresseNulle)
* PARAMETRES:
* RETOUR: FALSE : le pointeur NULL a ete modifie
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Protection sur les pointeurs nuls
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
/*
PUBLIC boolean ERR_TestPointeurNull( long AdresseNulle)
{
   long *ad_nulle = MK_FP(0, 0);

   if( AdresseNulle == *ad_nulle)
      return( TRUE);
   else
      return( FALSE);
}
*/