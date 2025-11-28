/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Gestion du fichier de configuration
* FICHIER: rc_conf.c
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/rc_conf.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:07:28   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jun 04 1997 19:13:14   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:20   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define RC_CONF_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "err.h"
#include "fic.h"
#include "str.h"
#include "fic_conf.h"

#include "fic_gere.h"

#include "rc_def.h"

#include "fic_ref.h"
#include "fic_act.h"
#include "fic_alar.h"
#include "fic_msg.h"
#include "fic_id.h"
#include "fic_idcn.h"

#include "referenc.h"

#include "rc_glob.h"
#include "rc_conf.h"


/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- CONSTANTES: ---------------*/

#define  ID_FIC_REF            "FICHIER_REFERENCE"
#define  ID_REF_FIC_REF        "REF_FICHIER_REFERENCE"

#define  ITERATION_REF_COURANT         0
#define  ITERATION_REF_ANCIEN          1

/*--------------- RESERVED: ---------------*/

/*--------------- FUNCTION: ---------------*/

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int RC_LireConfiguration(void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_LireConfiguration( TpTTacheContext Context)
{
   TValeur              valeur;
   FIC_enum_retour      retour;

   /* lire le fichier nouveau */
   retour = LireFichierConfig( Context->GereConfig, ID_FIC_REF,
                               ITERATION_REF_COURANT, &valeur);
   if( retour == FIC_OK)
      STR_strcpy( RFR_MAX_FIC, Context->NomFichierNouveau, valeur.Contenu);
   else
      Context->NomFichierNouveau[ 0] = '\0';

   retour = LireFichierConfig( Context->GereConfig, ID_REF_FIC_REF,
                               ITERATION_REF_COURANT, &valeur);
   if( retour == FIC_OK)
      STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierNouveau, valeur.Contenu);
   else
      Context->ReferenceFichierNouveau[ 0] = '\0';

  /*------------------------*/
  /* lire le fichier ancien */
   retour = LireFichierConfig( Context->GereConfig, ID_FIC_REF,
                               ITERATION_REF_ANCIEN, &valeur);
   if( retour == FIC_OK)
      STR_strcpy( RFR_MAX_FIC, Context->NomFichierCourant, valeur.Contenu);
   else
      Context->NomFichierCourant[ 0] = '\0';

   retour = LireFichierConfig( Context->GereConfig, ID_REF_FIC_REF,
                               ITERATION_REF_ANCIEN, &valeur);
   if( retour == FIC_OK)
      STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierCourant, valeur.Contenu);
   else
      Context->ReferenceFichierCourant[ 0] = '\0';
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int RC_EcrireNouvelleConfiguration
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_EcrireNouvelleConfiguration( TpTTacheContext Context)
{
   TGereFic             gereNouveau;
   TValeur              idConfig;
   TValeur              valConfig;
   char                 itemConfig[ CONF_MAX_ITEM];

   FIC_enum_retour      retour;

   /* ouvrir le fichier de config temporaire */
   if( ! InitFichierConfig(  &gereNouveau, FICHIER_CONFIG_TMP, "a+t",
                               itemConfig, &idConfig, &valConfig))
         ERR_ErreurFatale();

   /* ajoute le fichier de reference nouveau */
   retour = AjouterFichierConfig( &gereNouveau, ID_FIC_REF,
                                  ITERATION_REF_COURANT,
                                  Context->NomFichierNouveau);
   ERR_EstVrai( retour == FIC_OK);

   retour = AjouterFichierConfig( &gereNouveau, ID_REF_FIC_REF,
                                  ITERATION_REF_COURANT,
                                  Context->ReferenceFichierNouveau);

   /* ajoute le fichier de reference ancien s'il existe ET s'il n'a pas
    * le mˆme nom que le nouveau
    */
   if( ( Context->NomFichierCourant[ 0] != '\0') &&
       ( _stricmp( Context->NomFichierNouveau, Context->NomFichierCourant) != 0))
   {
      retour = AjouterFichierConfig( &gereNouveau, ID_FIC_REF,
                                     ITERATION_REF_ANCIEN,
                                     Context->NomFichierCourant);
      ERR_EstVrai( retour == FIC_OK);

      retour = AjouterFichierConfig( &gereNouveau, ID_REF_FIC_REF,
                                     ITERATION_REF_ANCIEN,
                                     Context->ReferenceFichierCourant);
      ERR_EstVrai( retour == FIC_OK);
   }

   /* fermer les deux fichiers */
   if( ! FermerFichierConfig( Context->GereConfig))
       ERR_ErreurFatale();
   if( ! FermerFichierConfig( &gereNouveau))
       ERR_ErreurFatale();

   /* remplacer l'ancien par le nouveau */
   FIC_unlink( FICHIER_CONFIG_COURANT);
   FIC_rename( FICHIER_CONFIG_TMP, FICHIER_CONFIG_COURANT);
   EchangerFichierConfig( Context->GereConfig, FICHIER_CONFIG_COURANT, "r+t");
}




/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/




