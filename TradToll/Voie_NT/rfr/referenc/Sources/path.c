/*------   (v) 1997 CS-Route   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Gestion du path d'un fichier
* FICHIER: path.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/path.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:07:28   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jun 04 1997 19:13:12   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:18   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define PATH_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "err.h"
#include "fic.h"
#include "str.h"

#include "rc_def.h"

#include "fic_gere.h"
#include "rc_glob.h"

#include "path.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/

/*--------------- RESERVED: ---------------*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ChangerPathTransfert
* PARAMETRES: - le nom du fichier destination dans la r‚f‚rence
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Translate le nom du fichier distant en un nom local
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void ChangerPathTransfert( char *FichierLocal, char *FichierSource)
{
   char             nomFichier[ RFR_MAX_PATH];
   char             extFichier[ RFR_MAX_PATH];

   FIC_splitpath( FichierSource, NULL, NULL, nomFichier, extFichier);
   FIC_makepath ( FichierLocal, NULL, RFR.path_transfert, nomFichier, extFichier);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ChangerPathReference
* PARAMETRES: - le nom du fichier destination dans la r‚f‚rence
*             - le nom et le path du fichier
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Translate le nom du fichier origine vers la r‚f‚rence sur le
*       disque dur
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void ChangerPathReference( char *FichierLocal, char *FichierSource)
{
   char             nomFichier[ RFR_MAX_PATH];
   char             extFichier[ RFR_MAX_PATH];

   FIC_splitpath( FichierSource, NULL, NULL, nomFichier, extFichier);
   FIC_makepath ( FichierLocal, RFR.disque, RFR.path_reference, nomFichier, extFichier);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ChangerPathVirtuel
* PARAMETRES: - le nom du fichier virtuel en sortie
*             - le nom et le path du fichier origine
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Translate le nom du fichier local en un nom virtuel en changeant
*       juste le disque
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void ChangerPathVirtuel( char *FichierVirtuel, char *FichierLocal)
{
   char             nomFichier[ RFR_MAX_PATH];
   char             extFichier[ RFR_MAX_PATH];
   char             chemin    [ RFR_MAX_PATH];

   FIC_splitpath( FichierLocal, NULL, chemin, nomFichier, extFichier);
   FIC_makepath ( FichierVirtuel, RFR.disque_virtuel, chemin, nomFichier, extFichier);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ChangerPath
* PARAMETRES: - le nom du fichier
*             - le nom et le path du fichier origine
*             - le path
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Translate le nom du fichier origine vers un nouveau path
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void ChangerPath( char *FichierDest, char *FichierSource, char *Path)
{
   char             nomFichier[ RFR_MAX_PATH];
   char             extFichier[ RFR_MAX_PATH];

   FIC_splitpath( FichierSource, NULL, NULL, nomFichier, extFichier);
   FIC_makepath ( FichierDest, NULL, Path, nomFichier, extFichier);
}

/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
