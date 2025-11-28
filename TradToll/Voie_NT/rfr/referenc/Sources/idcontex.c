/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement d'un IdContext
* FICHIER: item_fic.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
*             Un IdContext est une variable fourni … une action
*             contenant toutes les infos dont elle a besoin.
*
*             Il est identifi‚ par le couple Id/It‚ration.
*             Il est partag‚ entre toutes les actions qui doivent ˆtre
*             effectu‚es sur un Id/It‚ration, et pr‚cise le nombre
*             d'actions qui y sont li‚s
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/idcontex.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.9   03 Nov 1997 16:07:26   DPI
 * Version 4.00
 * 
 *    Rev 1.8   Jun 27 1997 10:50:30   DPI
 * Sur iteration absente, mettre une valeur 
 * dans la reference pour emission vers le LS
 * 
 * 
 *    Rev 1.8   Jun 27 1997 10:46:56   DPI
 * Sur iteration absente, mettre une valeur 
 * dans la reference pour emission vers le LS
 * 
 *    Rev 1.7   Jun 18 1997 18:47:04   DPI
 * STR_strcpy avec une constante fausse
 * 
 *    Rev 1.6   Jun 18 1997 18:15:26   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.5   Jun 18 1997 17:27:22   DPI
 * Les actions étaitent effectuées sur 'd:\transfer'
 * au lieu d'être appelés avec le chemin 
 * de l'id.
 * 
 *    Rev 1.4   Jun 04 1997 19:13:10   DPI
 *  
 * 
 *    Rev 1.3   Jun 03 1997 15:32:26   DPI
 *  
 * 
 *    Rev 1.2   Jun 03 1997 15:25:18   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.1   May 14 1997 14:03:14   DPI
 * Ajout de la Bal et du Pool dans l'IdContext
 * 
 *    Rev 1.0   Apr 24 1997 15:02:16   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define  ITEM_FIC_DEF

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
#include "referenc.h"

#include "fic_idcn.h"

#include "rc_glob.h"
#include "idcontex.h"
#include "path.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/

/*--------------- RESERVED: ---------------*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   InitialiserIdContext
* PARAMETRES: - le contexte de la tache
*             - la reference du fichier ( peut ˆtre NULL)
*             - l'id du traitement
*             - l'action
*             - un boolean indiquant si l'action de r‚ception est du a
*               un changement du fichier (TRUE), ou simplement … la
*               reception du fichier de r‚ference
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Initialise les champs de l'IdContext
* --------------------------------------------------------------------
* NOTA:
*      Si Change  est TRUE, alors le fichier est d'abord rapatri‚ dans
*      un r‚pertoire temporaire puis est copi‚ … l'emplacement demand‚.
*      Sinon, il est d‚j… dans son bon r‚pertoire
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void InitialiserIdContext( TpTTacheContext  Context,
                                  TFichier         *RefFichier,
                                  TFichier         *RefAncienFichier,
                                  boolean          EffacableAncienFichier,
                                  TReception       *Id,
                                  TIdContext       *IdContext,
                                  boolean          Change,
                                  boolean          Manuel)
{
   char        fichier_source[ RFR_MAX_PATH];

   /* initialiser l'IdContext */
   IdContext->NumPool   = Context->util.NumeroPool;
   IdContext->BalLocale = Context->util.BalLocale;

   STR_strcpy( RFR_MAX_ID, IdContext->Id, Id->Id);
   IdContext->IdReception = Id->IdReception;

   IdContext->AppelRaison = VERIFY;
   IdContext->Change = Change;

   /* copier les donn‚es du nouveau fichier */
   if( RefFichier != NULL)
   {
      IdContext->Iteration = RefFichier->Iteration;

      //SARMEK _ dodala if
	  if (!Manuel)
		ChangerPath( fichier_source, RefFichier->Fichier, Id->Path);
	  else
		  STR_strcpy(MAX_PATH, fichier_source, RefFichier->Fichier);

      if( Manuel)
      {
         /* il faut verifier que le fichier saisit manuellement est
          * deja dans le bon repertoire
          */
         ERR_EstVrai( _strcmpi( fichier_source, RefFichier->Fichier) == 0);
      }

      STR_strcpy( RFR_MAX_FIC, IdContext->NouveauFichier, fichier_source);
      STR_strcpy( RFR_MAX_REF, IdContext->NouvelleReference, RefFichier->Reference);
   }
   else
   {
      IdContext->Iteration = 0;
      IdContext->NouveauFichier[ 0] = '\0';
      STR_strcpy( RFR_MAX_REF, IdContext->NouvelleReference,
                                     ID_REFERENCE_ABSENTE);
   }


   /* copier les donn‚es de l'ancien fichier s'il existe */
   if( RefAncienFichier != NULL)
   {
       ChangerPath( fichier_source, RefAncienFichier->Fichier, Id->Path);
       STR_strcpy( RFR_MAX_FIC, IdContext->AncienFichier, fichier_source);
       STR_strcpy( RFR_MAX_REF, IdContext->AncienneReference, RefAncienFichier->Reference);
       IdContext->EstEffacableAncienFichier = EffacableAncienFichier;
   }
   else
   {
       IdContext->AncienFichier[ 0] = '\0';
       IdContext->AncienneReference[ 0] = '\0';
       IdContext->EstEffacableAncienFichier = FALSE;
   }

   IdContext->BufferParam[ 0] = '\0';

   IdContext->IndiceIdContext = 0;
   IdContext->NbActions = 0;
   IdContext->TacheContext = Context;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerIdContext
* PARAMETRES: - le contexte de la tache
*             - l'id context
*             - la position ou l'idContext a ‚t‚ ‚crit
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void    EnregistrerIdContext( TpTTacheContext  Context,
                                     TIdContext       *IdContext,
                                     long             *Position)
{
   FIC_enum_retour      retour;

   IdContext->NbActions ++;
   if( IdContext->NbActions == 1)
   {
      /* il s'agit de la premiere action utilisant cet IdContext */
      retour = AjouterPosIdContext( Context->GereIdContext, IdContext, Position);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      *Position = IdContext->Position;
      retour = EcrirePosIdContext( Context->GereIdContext, *Position, IdContext);
      ERR_EstVrai( retour == FIC_OK);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RetirerIdContext
* PARAMETRES: - le contexte de la tache
*             - l'action
*             - l'indice de l'action
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Retire un IdContext, en v‚rifiant qu'aucune action ne s'en sert
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean RetirerIdContext( TpTTacheContext  Context,
                                 long             Position)
{
   FIC_enum_retour      retour;
   TIdContext           *idContext;
   boolean              fini;

   /* lire le context */
   retour = LirePosIdContext( Context->GereIdContext,
                              Position,
                              &idContext);
   ERR_EstVrai( retour == FIC_OK);

   idContext->NbActions --;

   /* on libere l'IdContext si l'action ‚tait la derniŠre … l'utiliser
    * sinon on la r‚ecrit
    */
   fini = ( idContext->NbActions == 0);
   if( fini)
   {
      retour = LibererPosIdContext( Context->GereIdContext, Position);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      retour = EcrirePosIdContext( Context->GereIdContext, Position, idContext);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( fini);
}


/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
