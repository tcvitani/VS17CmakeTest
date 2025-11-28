/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des alarmes
* FICHIER: alarme.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
*             Gestion des demandes d'alarmes par l'applicatif,
*             et des declenchements d'alarmes
* --------------------------------------------------------------------
* A NOTER:
*         Lors du declenchement d'une alarme, il faut bloquer le temps
*         et le relancer lorsque l'alarme est entierement traite
*         Cependant HRD est mono jeton sur la suspension du temps.
*         Par securite, si on bloque 2 fois le temps, ou si on le relance
*         2 fois, on fait une erreur fatale
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/alarme.c_v  $
 * 
 *    Rev 1.1   Sep 03 2001 13:11:20   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:48   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.6   03 Nov 1997 16:07:20   DPI
 * Version 4.00
 * 
 *    Rev 1.5   Jun 04 1997 19:13:04   DPI
 *  
 * 
 *    Rev 1.4   May 13 1997 09:52:22   DPI
 * Blocage du temps sur alarme
 * 
 *    Rev 1.3   Apr 28 1997 15:37:24   DPI
 *  
 * 
 *    Rev 1.2   Apr 25 1997 14:56:48   DPI
 * Integration de Horodate 2.00
 * 
 *    Rev 1.1   Apr 24 1997 16:44:18   DPI
 * Ajout de EffacerAlarmeUsee
 * 
 *    Rev 1.0   Apr 24 1997 15:02:06   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "noyau.h"
#include "horodate.h"
#include "err.h"
#include "fic.h"
#include "str.h"
#include "fic_gere.h"

#include "rc_struc.h"
#include "referenc.h"
#include "rc_def.h"
#include "rc_glob.h"
#include "action.h"

#include "fic_act.h"
#include "fic_alar.h"


#define  ALARME_DEF
	#include "alarme.h"
#undef ALARME_DEF	
/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PRIVATE void InitialiserAlarme (TReceptionAlarme  *Alarme,
                                TpTDateHeure      DateHeure);

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AttendreAlarme
* PARAMETRES: - le contexte de l'action
*             - la date et heure de l'alarme voulue
*             - l'identifiant de l'alarme en retour
*             - la fonction a appeler sur declenchement du chrono
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre une nouvelle alarme si aucune n'est deja attendue
* --------------------------------------------------------------------
* NOTA : L'identifiant de l'alarme en retour est en fait la position
*        de l'action.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT boolean WINAPI AttendreAlarme (TpTIdContext  IdContext,
                                      TpTDateHeure  DateHeure,
                                      TIdAlarme     *IdAlarme,
                                      TFcn_DoIt     Faire)
{
   FIC_enum_retour      retour;
   TpTTacheContext      tacheContext;
   TReceptionAlarme     alarme;
   TReceptionAlarme     *alarme_lue;
   enum_hrd_reveil      ok_reveil;

   tacheContext = ( TpTTacheContext) IdContext->TacheContext;

   /* existe-t'il une alarme pour cette date et heure */
   retour = LireIdAlarme( tacheContext->GereAlarmes, DateHeure,
                          NULL, &alarme_lue);
   if( retour != FIC_OK)
   {
     InitialiserAlarme( &alarme, DateHeure);
     /* demander une alarme a Horodeate */
     ok_reveil = HRDArmeReveilPonctuel( DateHeure->Date,
                            DateHeure->Heure,
                            tacheContext->util.BalLocale,
                            &(alarme.AlarmeExterne));
     if( ok_reveil != HRD_REVEIL_OK)
     {
       return( FALSE);
     }

     retour = AjouterPosAlarme( tacheContext->GereAlarmes, &alarme, IdAlarme);
     ERR_EstVrai( retour == FIC_OK);

     /* il faut lire l'alarme pour valoriser son IdAlarme */
     retour = LireIdAlarme( tacheContext->GereAlarmes, DateHeure,
                          NULL, &alarme_lue);
     ERR_EstVrai( retour == FIC_OK);
   }

   EnregistrerActionAlarme( tacheContext, IdContext, Faire,
                            alarme_lue->Position,
                            &(alarme_lue->PremiereAction),
                            &(alarme_lue->DerniereAction));

   *IdAlarme = alarme_lue->DerniereAction;

   retour = EcrirePosAlarme( tacheContext->GereAlarmes,
                    alarme_lue->Position,
                    alarme_lue);
   ERR_EstVrai( retour == FIC_OK);

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RecevoirAlarme
* PARAMETRES: - le contexte de l'action
*             - l'identifiant de l'alarme externe
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean RecevoirAlarme (TpTTacheContext  Context,
                               TAlarmeExterne   *AlarmeExterne)
{
   FIC_enum_retour      retour;
   TReceptionAlarme     *alarme_lue;
   enum_hrd_evolution_temps         retour_temps;

   /* existe-t'il une alarme pour cette date et heure */
   retour = LireIdAlarme( Context->GereAlarmes, NULL,
                          AlarmeExterne, &alarme_lue);
   if( retour != FIC_OK)
     return(FALSE);

   Context->AlarmeEnCours = alarme_lue->Position;

   /* bloquer le temps */
   retour_temps = HRDSuspendEvolutionDuTemps ( Context->util.BalLocale);
   ERR_EstVrai( retour_temps == HRD_TEMPS_SUSPENDU);

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EffacerAlarmeUsee
* PARAMETRES: - le contexte de l'action
*             - l'identifiant de l'alarme externe
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Quand l'alarme se declenche, elle n'est pas retiree d'Horodate
*       pour eviter que quelqu'un ne rearme le meme Id.
*       Donc il faut l'effacer quand on a fini le traitement
*       Il faut egalement relancer le temps
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean EffacerAlarmeUsee (TpTTacheContext  Context,
                                  TAlarmeExterne   *AlarmeExterne)
{
   enum_hrd_evolution_temps         retour_temps;

   HRDAnnuleReveil( *AlarmeExterne);

   retour_temps = HRDReprendEvolutionDuTemps ( Context->util.BalLocale);
   ERR_EstVrai( retour_temps == HRD_TEMPS_EN_EVOLUTION);

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   OublierAlarme
* PARAMETRES: - le contexte de l'action
*             - l'identifiant de l'alarme
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Permet de supprimer une alarme sans declencher l'action
* --------------------------------------------------------------------
* NOTA : L'identifiant de l'alarme en retour est en fait la position
*        de l'action.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT boolean WINAPI OublierAlarme (TpTIdContext  IdContext,
                                     TIdAlarme     *IdAlarme)
{
   FIC_enum_retour      retour;
   TpTTacheContext      tacheContext;
   TReceptionAction     *action_lue;
   TReceptionAlarme     *alarme_lue;
   long                 indiceAlarme;
   boolean              encore;
   TAlarmeExterne       alarmeExterne;

   tacheContext = ( TpTTacheContext) IdContext->TacheContext;

   if ( *IdAlarme == NO_ALARME)
      return( FALSE);

   /* lire l'action */
   retour = LirePosAction( tacheContext->GereActions, *IdAlarme, &action_lue);
   ERR_EstVrai( retour == FIC_OK);

   /* verifier que c'est une alarme */
   if( action_lue->TypeAction != ALARME)
      return( FALSE);

   /* stocker l'indice de l'alarme pour savoir si elle n'est plus attendue */
   indiceAlarme = action_lue->IndiceType;
   retour = LirePosAlarme( tacheContext->GereAlarmes,
                           indiceAlarme, &alarme_lue);

   retour = AbandonnerUneAction( tacheContext, *IdAlarme, FALSE);
   *IdAlarme = NO_ALARME;

   if( retour !=  FIC_OK)
      return( FALSE);

   /* reste-t'il une action pour cette date et heure ? */
   retour = LirePosAlarme( tacheContext->GereAlarmes,
                           indiceAlarme, &alarme_lue);
   encore = ( alarme_lue->PremiereAction != NO_ACTION);
   if( encore)
   {
      retour = EcrirePosAlarme( tacheContext->GereAlarmes, alarme_lue->Position, alarme_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      alarmeExterne = alarme_lue->AlarmeExterne;

      /* il faut annuler le chrono.
       * Nota, il est peut etre deja tombe */
      HRDAnnuleReveil( alarmeExterne);

      retour = LibererPosAlarme( tacheContext->GereAlarmes, alarme_lue->Position);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( retour !=  FIC_NOK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   InitialiserAlarme
* PARAMETRES: - l'alarme
*             - la date et heure de l'alarme voulue
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void InitialiserAlarme (TReceptionAlarme  *Alarme,
                                TpTDateHeure      DateHeure)
{
   Alarme->Etat = WAITING_ACK;
   Alarme->DateHeure = *DateHeure;
   Alarme->AlarmeExterne = 0;
   Alarme->PremiereAction = NO_ACTION;
   Alarme->DerniereAction = NO_ACTION;
}
