/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement d'un fichier item du fichier de reference
* FICHIER: item_fic.C
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/item_fic.c_v  $
 * 
 *    Rev 1.2   Sep 03 2001 13:11:22   sbatiot
 *  
 * 
 *    Rev 1.1   22 Mar 2001 10:03:50   FR
 * - correction du blocage du module suite à une erreur de transfert de liste
 * - correction de la gestion de l'état de prise en compte d'une nouvelle référence
 * 
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.12   19 Mar 1998 14:32:16   DPI
 * - sur MESSAGE_MANUEL, le fichier manuel est vérifié
 * - MESSAGE_MANUEL_ACK est envoyé à la tache appelante
 * - sur Cold Start, tous les repertoires applicatifs
 *   sont effacés
 * 
 *    Rev 1.11   11 Feb 1998 16:32:06   DPI
 * Correction d'un bug d'effacement de fichier
 * 
 *    Rev 1.10   03 Nov 1997 16:07:26   DPI
 * Version 4.00
 * 
 *    Rev 1.9   30 Oct 1997 11:57:20   DPI
 * Integration de FICTR_copy de OUTILS_C 2.00
 * 
 *    Rev 1.8   Jul 28 1997 18:30:32   DPI
 * Gestion de la version
 * 
 *    Rev 1.7   Jun 18 1997 17:27:22   DPI
 * Les actions étaitent effectuées sur 'd:\transfer'
 * au lieu d'être appelés avec le chemin 
 * de l'id.
 * 
 *    Rev 1.6   Jun 18 1997 11:36:34   DPI
 * Correction d'un bug, paramètre NULL au lieu
 * de char [ n]
 *
 *    Rev 1.5   Jun 11 1997 17:50:58   DPI
 * Amelioration des traces
 * 
 *    Rev 1.4   Jun 04 1997 19:13:10   DPI
 *  
 * 
 *    Rev 1.3   Jun 03 1997 15:32:26   DPI
 *  
 * 
 *    Rev 1.2   Jun 03 1997 15:25:52   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.1   Jun 03 1997 15:06:22   DPI
 * Supprimer dans EnregistrerActionReception
 * le param Enregistrer qui est obsolete
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

#include "fic_ref.h"
#include "fic_id.h"

#include "referenc.h"

#include "rc_glob.h"
#include "rc_emis.h"
#include "path.h"
#include "idcontex.h"
#include "action.h"

/* ### FICTR_xxx remplace par FIC_xxx */
#include "fic.h"

#include "item_fic.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PRIVATE TEnum_Verified VerifierFichierRecu (TpTTacheContext  Context,
                                            TFichier         *RefFichier,
                                            TReception       *Id);
PRIVATE TEnum_Verified VerifierFichier (TpTTacheContext  Context,
                                        TIdContext       *IdContext,
                                        TReception       *Id);
PRIVATE void TraiterFichierVerifie (TpTTacheContext  Context,
                                    TEnum_Verified   Verifie,
                                    TFichier         *RefFichier,
                                    TReception       *Id);

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RecevoirUnFichier
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Public
* ROLE: Appeler des que la tache LAN a rapatrie le fichier sur la VM
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void RecevoirUnFichier (TpTTacheContext  Context,
                               char             *FichierRecu,
                               boolean          Charge)
{
   FIC_enum_retour      retour;
   TEnum_Verified       verifie;
   TFichier             *refFichier_lu;
   TFichier             refFichier;
   TReception           *id;
   char                 fichier_source[ RFR_MAX_PATH];

	ReferenceFichierTrace("RFR File receiced[%s] ", FichierRecu);
   /* Identifier l'item concerne au moyen du nom de fichier */
   retour = LireIdReference( Context->GereNouveau, NULL, 0, NULL,
                             FichierRecu, &refFichier_lu);

   if ( retour != FIC_OK)    /* ce fichier n'est pas attendu */
   {
      ChangerPathTransfert( fichier_source, FichierRecu);
      FIC_unlink( fichier_source);
	  ReferenceFichierTrace("RFR File NOK[%s] ", fichier_source);
      return;
   }
   /* Pas genial, le FIC_GERE
    * Il faut copier le contenu de refFichier_lu dans ref_fichier,
    * pour eviter qu'un appel subsequent a FIC_GERE ne change la valeur
    */
   refFichier = *refFichier_lu;
  
   /* on attends un fichier de moins */
   Context->NbFichiersReclames --;

   /* ce type de fichier doit etre attendu par l'applicatif */
   retour = LireIdIdReception( Context->GereId, refFichier.Id, &id);
   ERR_EstVrai( retour == FIC_OK);

   /* on a pas reussi a le charger, mais on ne l'attend plus */
   if( ! Charge)
   {
      TraiterFichierVerifie( Context, NACK, &refFichier, id);

// Debut FR 20/03/01
      Context->TousFichiersValides = FALSE;
// Fin FR 20/03/01

      RC_EmettreMessageUpdate( Context->util.BalMessage,
                               Context->util.BalLocale,
                               Context->util.NumeroPool,
                               refFichier.Id,
                               refFichier.Reference,
                               UNLOAD_FILE);
      return;
   }

// Crash in VerifierFichierRecu
   verifie = VerifierFichierRecu( Context, &refFichier, id);

   TraiterFichierVerifie( Context, verifie, &refFichier, id);

   if ( verifie == NACK)
   {
      Context->TousFichiersValides = FALSE;
      RC_EmettreMessageUpdate( Context->util.BalMessage,
                               Context->util.BalLocale,
                               Context->util.NumeroPool,
                               refFichier.Id,
                               refFichier.Reference,
                               INVALID_FILE);
   }
   else
   {
      RC_EmettreMessageUpdate( Context->util.BalMessage,
                               Context->util.BalLocale,
                               Context->util.NumeroPool,
                               refFichier.Id,
                               refFichier.Reference,
                               CHECKED_FILE);
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   VerifierFichierManuel
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Public
* ROLE: Appeler des que un fichier a ete manuellement mis dans son
*       repertoire destination
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean VerifierFichierManuel (TpTTacheContext  Context)
{
   TFichier             refFichier;
   FIC_enum_retour      retour;
   TReception           *id;
   TIteration           iteration;
   TIdContext           idContext;
   TEnum_Verified       verifie;

   /* est-ce qu'on attends ce type de fichier */
   retour = LireIdIdReception( Context->GereId,
                               Context->action.u.manuel.Id,
                               &id);
   if ( retour != FIC_OK)    /* ce fichier n'est pas attendu */
   {
      FIC_unlink( Context->action.u.manuel.Fichier);
      return( FALSE);
   }

   /* si le fichier est iterable, la valeur 0 n'etant pas accepte, il faut
    * mettre 1
    */
   if( id->Iterable)
       iteration = 1;
   else
       iteration = 0;

   /*--------------------------------*/
   STR_strcpy( RFR_MAX_ID, refFichier.Id, Context->action.u.manuel.Id);
   STR_strcpy( RFR_MAX_FIC, refFichier.Fichier, Context->action.u.manuel.Fichier);
   STR_strcpy( RFR_MAX_REF, refFichier.Reference, ID_REFERENCE_MANUELLE);
   refFichier.Iteration = iteration;
   refFichier.Hostname[ 0] = '\0';

   /* initialiser l'IdContext pour faire la verification */
   InitialiserIdContext( Context, &refFichier, NULL, FALSE,
                                  id, &idContext, TRUE, FALSE);

   //SARMEK
   strcpy_s(idContext.NouveauFichier, sizeof(idContext.NouveauFichier), Context->action.u.manuel.Fichier);

   verifie = VerifierFichier( Context, &idContext, id);
   if( verifie == NACK)
      FIC_unlink( Context->action.u.manuel.Fichier);

   return( verifie == ACK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   VerifierFichierRecu
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Appeler des que la tache LAN a rapatrie le fichier sur la VM
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE TEnum_Verified VerifierFichierRecu (TpTTacheContext  Context,
                                            TFichier         *RefFichier,
                                            TReception       *Id)
{
   char                 fichier_source[ RFR_MAX_PATH];
   TIdContext           idContext;
   TEnum_Verified       verifie;
   //crt 04/05/2007
   DWORD				 dwAttrib;
   //

   /* initialiser l'IdContext pour faire la verification */
   InitialiserIdContext( Context, RefFichier, NULL, FALSE,
                                  Id, &idContext, TRUE, FALSE);

   /* la verification se fait sur le fichier sur le disque virtuel */
   ChangerPathTransfert( fichier_source, RefFichier->Fichier);
   //crt 09/05/2007
   dwAttrib = GetFileAttributes(fichier_source); 
   if (dwAttrib & FILE_ATTRIBUTE_READONLY) 
   { 
	   SetFileAttributes(fichier_source, dwAttrib & !FILE_ATTRIBUTE_READONLY);
	   ReferenceFichierTrace("crt:VerifierFichierRecu: SetFileAttributes OK");
	   verifie = ACK;
	   
	   if (!SetFileAttributes(fichier_source, dwAttrib & !FILE_ATTRIBUTE_READONLY));
	   {
		   verifie = NACK;	
	   }
	   
   }
   //
   STR_strcpy( RFR_MAX_FIC, idContext.NouveauFichier, fichier_source);

   verifie = VerifierFichier( Context, &idContext, Id);

   return( verifie);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   VerifierFichier
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Appeler des que la tache LAN a rapatrie le fichier sur la VM
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE TEnum_Verified VerifierFichier (TpTTacheContext  Context,
                                        TIdContext       *IdContext,
                                        TReception       *Id)
{
   TEnum_Verified       verifie;
   char                 Commentaire[ RFR_MAX_COMMENTAIRE];

   ReferenceFichierTrace("Verification Id '%s' File '%s'", IdContext->Id, IdContext->NouveauFichier);

   NO_WARNING( Context);

   /* verifier le fichier */
   if( Id->Verifier != NULL)
       verifie = Id->Verifier( IdContext, Commentaire);
   else
       verifie = ACK;

   if ( verifie == NACK)
   {
      ReferenceFichierTrace("ECHEC : '%s'\n", Commentaire);
   }
   else
   {
      ReferenceFichierTrace("OK\n");
   }

   return( verifie);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   TraiterFichierVerifie
* PARAMETRES: - le contexte de la tache
*             - l'etat de verification( TRUE= BON)
*             - l'item dans le fichier de reference
*             - l'Id de reference
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void TraiterFichierVerifie (TpTTacheContext  Context,
                                    TEnum_Verified   Verifie,
                                    TFichier         *RefFichier,
                                    TReception       *Id)
{
   FIC_enum_retour      retour;
   TFichier             *refFichier_lu;
   char                 fichier_source[ RFR_MAX_PATH];
   char                 fichier_dest[ RFR_MAX_PATH];
   boolean              ok;

   if ( Verifie == NACK)
   {
      /* le fichier etant errone, on ne le prend pas dans
       * notre reference
       * Il faut prendre celui de l'ancienne reference si l'iteration
       * est nulle, et si l'ancienne reference existe
       *
       * Attention
       * S'il s'agit d'une iteration non nulle, elle est
       * carrement oubliee
       */
      if( RefFichier->Iteration == 0)
      {
         retour = LireIdReference( Context->GereAncien, RefFichier->Id,
                                   RefFichier->Iteration,
                                   NULL, NULL,
                                   &refFichier_lu);
         if( retour == FIC_OK)
         {
            /* enregistrer dans le fichier de reference */
            EcrirePosReference( Context->GereCreer, refFichier_lu->Id,
                                                    refFichier_lu->Iteration,
                                                    refFichier_lu->Fichier,
                                                    refFichier_lu->Reference,
                                                    refFichier_lu->Hostname);

            /* on enregistre quand meme une action */
            EnregistrerActionReception( Context, refFichier_lu, TRUE, TRUE, FALSE);

            /* nota:
             * il faut d'abord enregistrer la reference avant l'action,
             * car sinon le fichier est declare effacable
             */
         }
      }
   }
   else
   {
      EnregistrerActionReception( Context, RefFichier, TRUE, TRUE, FALSE);

      /* enregistrer aussi dans le fichier de reference */
      EcrirePosReference( Context->GereCreer, RefFichier->Id,
                                              RefFichier->Iteration,
                                              RefFichier->Fichier,
                                              RefFichier->Reference,
                                              RefFichier->Hostname);

      /* il faut copier le fichier a son emplacement definitif */
      ChangerPathTransfert( fichier_source, RefFichier->Fichier);
      ChangerPath( fichier_dest, fichier_source, Id->Path);
      if( _access( fichier_dest, 0) == 0)
          FIC_unlink( fichier_dest);
	
	  /* ### FICTR_xxx remplace par FIC_xxx */
      ok = FIC_copy( fichier_dest, fichier_source,
                       FIC_COPY_WITH_DATE /*, RFR_TIMEOUT_RAPIDE*/);
      ERR_EstVrai( ok);

      FIC_unlink( fichier_source);

   }
}
