/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des actions
* FICHIER: ACTION.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
*             Ce fichier traite les actions.
*             Une action est ce que le moteur doit faire lors d'un
*             evenement. Elle utilise un IdContext.
*             Les evenements sont :
*             - reception d'un fichier de r‚f‚rence
*             - d‚clenchement d'un r‚veil
*             - message inter-tache recu
*
*             Avant de pouvoir traiter une action, il faut d'abord qu'elle
*             soit enregistr‚e, suivant l'un ou l'autre des events.
*             Pour un event de reception, on ajoute … la fin de l'analyse
*             des actions a executer aprŠs avoir effectu‚ les it‚rations
*
*             Lors de l'apparition de l'event, elle est execut‚e  suivant
*             l'un ou l'autre des events.
*
*             Lors de la reception d'un fichier de r‚f‚rence, il faut
*             abandonner toutes les actions en cours.
*
*             Dans le cadre applicatif, il peut ˆtre n‚cessaire de
*             supprimer une action qui ne sera pas execut‚e, exemple:
*             - on d‚clare une action sur Message
*             - et une action sur Reveil, si le message n'arrive pas.
*             - Donc, si le message arrive, il faut oublier le Reveil
* --------------------------------------------------------------------
* CHAINAGE:
*           Chaque action est doublement chain‚e:
*           1 - sur le type d'event
*           2 - sur l'ID
*           Je ne suis pas sur qu'on se serve du chainage sur l'ID
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/action.c_v  $
 * 
 *    Rev 1.3   24 Sep 2001 15:39:32   FROUGIET
 *  
 * 
 *    Rev 1.2   22 Mar 2001 10:03:46   FR
 * - correction du blocage du module suite à une erreur de transfert de liste
 * - correction de la gestion de l'état de prise en compte d'une nouvelle référence
 * 
 * 
 *    Rev 1.1   31 Jan 2001 17:16:28   FR
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:46   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.13   03 Nov 1997 16:07:20   DPI
 * Version 4.00
 * 
 *    Rev 1.12   Jun 18 1997 18:46:02   DPI
 * Quand on determine si l'ancien fichier est
 * effacable, on teste s'il existe dans son
 * repertoire definitif
 * 
 *    Rev 1.11   Jun 18 1997 18:15:26   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.10   Jun 18 1997 11:36:00   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.9   Jun 18 1997 10:38:14   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.8   Jun 13 1997 10:42:52   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.7   Jun 04 1997 19:13:02   DPI
 *  
 * 
 *    Rev 1.6   Jun 03 1997 17:39:36   DPI
 * Bug a la liberation d'un message
 * 
 *    Rev 1.5   Jun 03 1997 15:32:26   DPI
 *  
 * 
 *    Rev 1.4   Jun 03 1997 15:25:18   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.3   Jun 03 1997 15:06:22   DPI
 * Supprimer dans EnregistrerActionReception
 * le param Enregistrer qui est obsolete
 * 
 *    Rev 1.2   May 15 1997 14:28:40   DPI
 *  
 * 
 *    Rev 1.1   May 12 1997 13:44:30   DPI
 * 1- Generer les actions obligatoires
 * 2- Sur Manuel, gerer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 15:02:06   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define  ACTION_DEF

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

#include "fic_act.h"
#include "fic_alar.h"
#include "fic_id.h"
#include "fic_idcn.h"
#include "fic_msg.h"
#include "fic_ref.h"

#include "path.h"
#include "rc_glob.h"
#include "rc_emis.h"
#include "idcontex.h"
#include "action.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/

/*--------------- PRIVATE: ---------------*/
PUBLIC boolean DetecterAncienFichier( TpTTacheContext  Context,
                                      TReception       *Id,
                                      TFichier         *RefFichier,
                                      TFichier         *RefAncienFichier,
                                      boolean          *Effacable);
PRIVATE void    InitialiserAction( TpTTacheContext  Context,
                                   TFcn_DoIt        Faire,
                                   TReceptionAction *Action,
                                   TIdContext       *IdContext,
                                   TEnum_TypeAction TypeAction,
                                   long             IndiceType);
PRIVATE void    EnregistrerAction( TpTTacheContext  Context,
                                   TReceptionAction *Action,
                                   TReception       *Id,
                                   long             *PremiereAction_EVENT,
                                   long             *DerniereAction_EVENT);
PRIVATE boolean  ExecuterAction( TpTTacheContext  Context,
                                long             ActionPosition,
                                TEnum_Reason     AppelRaison,
                                boolean          DeleteActionOnError,	// FR 20/03/01
                                long             *PremiereAction_EVENT,
                                long             *DerniereAction_EVENT);
PRIVATE void    DeplacerAction( TpTTacheContext  Context,
                                TReceptionAction *Action,
                                long             Position,
                                TReception       *Id,
                                long             *PremiereAction_EVENT,
                                long             *DerniereAction_EVENT);
PRIVATE boolean RetirerAction( TpTTacheContext  Context,
                               TReceptionAction *Action,
                               long             Position,
                               TReception       *Id,
                               long             *PremiereAction_EVENT,
                               long             *DerniereAction_EVENT);

PRIVATE void    LierAction( TpTTacheContext  Context,
                            TReceptionAction *Action,
                            long             ActionPosition,
                            TReception       *Id,
                            long             *PremiereAction_EVENT,
                            long             *DerniereAction_EVENT);
PRIVATE void    DelierAction( TpTTacheContext  Context,
                              TReceptionAction *Action,
                              TReception       *Id,
                              long             *PremiereAction_EVENT,
                              long             *DerniereAction_EVENT);

/*--------------- FUNCTIONS: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerActionReception
* PARAMETRES: - le contexte de la tache
*             - une structure d‚crivant un fichier item
*             - un boolean indiquant si l'action de r‚ception est du a
*               un changement du fichier (TRUE), ou simplement … la
*               reception du fichier
*             - un boolean indiquant si une action doit ˆtre
*               enregistrer pour cet item, ou s'il faut juste g‚n‚rer
*               l'action BEFORE
*             - un boolean indiquant si l'action BEFORE ne doit pas
*               ˆtre g‚n‚r‚. N'a de sens que pour it‚ration non nulle
*               l'action BEFORE
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre l'action a executer sur reception
* --------------------------------------------------------------------
* NOTA: Ajout de l'action BEFORE sur iteration non nulle
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void EnregistrerActionReception( TpTTacheContext  Context,
                                        TFichier         *RefFichier,
                                        boolean          Change,
                                        boolean          FaireBefore,
                                        boolean          Manuel)
{
   FIC_enum_retour      retour;
   TFichier             refFichier;
   TReception           *id;
   TReceptionAction     action;
   TIdContext           idContext;
   TFichier             refAncienFichier;
   boolean              existeAncienFichier = FALSE;
   boolean              effacable;

   /* recopier le paramŠtre dans une variable garantie
    * a cause d'un defaut du FIC_GERE
    */
   refFichier = *RefFichier;

   /* ce type de fichier doit ˆtre attendu par l'applicatif */
   retour = LireIdIdReception( Context->GereId, refFichier.Id, &id);
   ERR_EstVrai( retour == FIC_OK);

   /* chercher l'ancien fichier et voir s'il est effacable */
   existeAncienFichier = DetecterAncienFichier( Context,
                                                id,
                                                &refFichier,
                                                &refAncienFichier,
                                                &effacable);

   /* si c'est la premiere action a ajouter et si l'it‚ration est non nulle
    * si c'est demand‚
    * il faut cr‚er une premiŠre action avec l'It‚ration … BEFORE
    */
   if( ( id->PremiereAction == NO_ACTION) && ( refFichier.Iteration != 0) &&
       ( FaireBefore) )
   {
      /* initialiser l'IdContext ainsi que l'action sur r‚ception */
      InitialiserIdContext( Context, NULL, NULL, FALSE, id, &idContext, Change, Manuel);
      idContext.Iteration = BEFORE_ITERATIONS;
      idContext.Change = FALSE;

      InitialiserAction( Context, id->Faire, &action, &idContext, RECEPTION, 0);
      EnregistrerAction( Context, &action, id,
                         &(Context->PremiereActionReception),
                         &(Context->DerniereActionReception));
   }

   /* initialiser l'IdContext */
   if( existeAncienFichier)
      InitialiserIdContext( Context, &refFichier, &refAncienFichier,
                            effacable, id, &idContext, Change, Manuel);
   else
      InitialiserIdContext( Context, &refFichier, NULL,
                            effacable, id, &idContext, Change, Manuel);

   /* initialiser l'action sur r‚ception */
   InitialiserAction( Context, id->Faire, &action, &idContext, RECEPTION, 0);

   EnregistrerAction( Context, &action, id,
                      &(Context->PremiereActionReception),
                      &(Context->DerniereActionReception));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ExecuterActionReception
* PARAMETRES: - le contexte de la tache
* RETOUR:     - TRUE si une action reste a faire
*             - FALSE si non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Execute une action sur reception
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean ExecuterActionReception( TpTTacheContext  Context,
                                        TEnum_Reason     AppelRaison)
{
   if( Context->PremiereActionReception == NO_ACTION)
      return( FALSE);

   ExecuterAction( Context, Context->PremiereActionReception, AppelRaison,
                      TRUE,	// FR 20/03/01
                      &(Context->PremiereActionReception),
                      &(Context->DerniereActionReception));

   return( Context->PremiereActionReception != NO_ACTION);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerActionAlarme
* PARAMETRES: - le contexte de la tache
*             - le context de l'action
*             - la fonction … appeler
*             - L'indice de l'alarme
*             - le chainage pour cette alarme
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre l'action a executer sur d'une alarme
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void EnregistrerActionAlarme( TpTTacheContext  Context,
                                     TIdContext       *IdContext,
                                     TFcn_DoIt        Faire,
                                     long             Indice,
                                     long             *PremiereAction,
                                     long             *DerniereAction)
{
   FIC_enum_retour      retour;

   TReception           *id;
   TReceptionAction     action;

   /* ce type de fichier doit ˆtre attendu par l'applicatif */
   retour = LirePosIdReception( Context->GereId, IdContext->IdReception, &id);
   ERR_EstVrai( retour == FIC_OK);

   /* initialiser l'IdContext ainsi que l'action sur r‚ception */
   InitialiserAction( Context, Faire, &action, IdContext, ALARME, Indice);

   EnregistrerAction( Context, &action, id, PremiereAction, DerniereAction);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ExecuterActionAlarme
* PARAMETRES: - le contexte de la tache
* RETOUR:     - TRUE si une action reste a faire
*             - FALSE si non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Execute une action sur alarme
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean ExecuterActionAlarme( TpTTacheContext  Context)
{
   FIC_enum_retour      retour;
   TReceptionAlarme     *alarme_lue;
   boolean              encore;

   /* existe-t'il une alarme pour cette date et heure */
   retour = LirePosAlarme( Context->GereAlarmes,
                           Context->AlarmeEnCours,
                           &alarme_lue);
   if( retour != FIC_OK)
      return( FALSE);

   /* n'aurait-on pas resett‚ les actions alors qu'on recevait cette alarme
    */
   if( alarme_lue->PremiereAction != NO_ACTION)
   {
      ExecuterAction( Context, alarme_lue->PremiereAction, DO_IT,
                      FALSE,	// FR 20/03/01
                      &(alarme_lue->PremiereAction),
                      &(alarme_lue->DerniereAction));
   }

   encore = ( alarme_lue->PremiereAction != NO_ACTION);
   if( encore)
   {
      retour = EcrirePosAlarme( Context->GereAlarmes, alarme_lue->Position, alarme_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      retour = LibererPosAlarme( Context->GereAlarmes, alarme_lue->Position);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( encore);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerActionMessage
* PARAMETRES: - le contexte de la tache
*             - le context de l'action
*             - la fonction … appeler
*             - L'indice du message
*             - le chainage pour cette alarme
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre l'action a executer sur reception d'un message
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void EnregistrerActionMessage( TpTTacheContext  Context,
                                      TIdContext       *IdContext,
                                      TFcn_DoIt        Faire,
                                      long             Indice,
                                      long             *PremiereAction,
                                      long             *DerniereAction)
{
   FIC_enum_retour      retour;

   TReception           *id;
   TReceptionAction    action;

   /* ce type de fichier doit ˆtre attendu par l'applicatif */
   retour = LirePosIdReception( Context->GereId, IdContext->IdReception, &id);
   ERR_EstVrai( retour == FIC_OK);

   /* initialiser l'IdContext ainsi que l'action sur r‚ception */
   InitialiserAction( Context, Faire, &action, IdContext, MESSAGE, Indice);

   EnregistrerAction( Context, &action, id, PremiereAction, DerniereAction);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ExecuterActionMessage
* PARAMETRES: - le contexte de la tache
* RETOUR:     - TRUE si une action reste a faire
*             - FALSE si non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Execute une action sur message applicatif
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean ExecuterActionMessage( TpTTacheContext  Context)
{
   FIC_enum_retour      retour;
   TReceptionMessage    *message_lu;
   boolean              encore;

   /* existe-t'il un message */
   retour = LirePosMessage( Context->GereMessages,
                            Context->MessageEnCours,
                            &message_lu);
   if( retour != FIC_OK)
      return( FALSE);

   /* n'aurait-on pas resett‚ les actions alors qu'on recevait ce message
    */
   if ( message_lu->PremiereAction != NO_ACTION)
   {
      ExecuterAction( Context, message_lu->PremiereAction, DO_IT,
                      FALSE,	// FR 20/03/01
                      &(message_lu->PremiereAction),
                      &(message_lu->DerniereAction));
   }

   encore = ( message_lu->PremiereAction != NO_ACTION);
   if( encore)
   {
      retour = EcrirePosMessage( Context->GereMessages, message_lu->Position, message_lu);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      retour = LibererPosMessage( Context->GereMessages, message_lu->Position);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( encore);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AjouterActionsAfter
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ajoute les actions AFTER a tous les ID le n‚cessitant
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void AjouterActionsAfter( TpTTacheContext  Context)
{
   FIC_enum_retour      retour;

   TReception           *id;
   TReceptionAction     action;
   TReceptionAction     *action_lue;
   TIdContext           idContext;
   long                 idPosition;

   idPosition = 0;
   retour = FIC_OK;
   while( retour == FIC_OK)
   {
      idPosition++;
      retour = LirePosIdReception( Context->GereId, idPosition, &id);
      if( retour == FIC_OK)
      {
         /* est-ce qu'il existe une action */
         if( id->PremiereAction != NO_ACTION)
         {
            retour = LirePosAction( Context->GereActions,
                                    id->PremiereAction,
                                    &action_lue);
            ERR_EstVrai( retour == FIC_OK);

            /* si c'est la premiere action est de type BEFORE
             * il faut cr‚er une nouvelle action avec la raison … AFTER
             */
            if( action_lue->Iteration == BEFORE_ITERATIONS)
            {
               /* initialiser l'IdContext ainsi que l'action sur r‚ception */
               InitialiserIdContext( Context, NULL, NULL, FALSE, id, &idContext, FALSE, FALSE);
               idContext.Iteration = AFTER_ITERATIONS;
               idContext.Change = FALSE;

               InitialiserAction( Context, id->Faire, &action, &idContext, RECEPTION, 0);
               EnregistrerAction( Context, &action, id,
                                  &(Context->PremiereActionReception),
                                  &(Context->DerniereActionReception));
            }
         }
      }
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AjouterActionsObligatoires
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ajoute les actions de MISSING a tous les ID ‚tant obligatoire
*       et n'ayant aucune action pr‚vu ( donc aucun fichier)
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void AjouterActionsObligatoires( TpTTacheContext  Context)
{
   FIC_enum_retour      retour;

   TReception           *id;
   TReceptionAction     action;
   TIdContext           idContext;
   long                 idPosition;

   idPosition = 0;
   retour = FIC_OK;
   while( retour == FIC_OK)
   {
      idPosition++;
      retour = LirePosIdReception( Context->GereId, idPosition, &id);
      if( retour == FIC_OK)
      {
         /* est-ce qu'il existe une action et est-ce obligatoire */
         if( ( id->PremiereAction == NO_ACTION) && ( id->Obligatoire))
         {
            /* initialiser l'IdContext ainsi que l'action sur r‚ception */
            InitialiserIdContext( Context, NULL, NULL, FALSE, id, &idContext, TRUE, FALSE);
            idContext.Iteration = MISSING_ITERATION;
            idContext.Change = FALSE;

            InitialiserAction( Context, id->Faire, &action, &idContext, RECEPTION, 0);
            EnregistrerAction( Context, &action, id,
                               &(Context->PremiereActionReception),
                               &(Context->DerniereActionReception));
         }
      }
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AbandonnerActionsEnCours
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Abandonne toutes les actions enregistr‚es, en retirant les
*       IdContext.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void AbandonnerActionsEnCours( TpTTacheContext  Context)
{
   FIC_enum_retour      retour;
   long                 position;

   position = 1;
   retour = FIC_OK;
   while( retour != FIC_FINI)
   {
      retour = AbandonnerUneAction( Context, position, TRUE);
      position ++;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AbandonnerActionsId
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Abandonne toutes les actions enregistr‚es pour un ID donn‚,
*       en retirant les IdContext.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean AbandonnerActionsId( TpTTacheContext  Context,
                                    char             *IdReception)
{
   FIC_enum_retour      retour;
   long                 position;
   TReception           *id;

   /* lire l'Id de reception */
   retour = LireIdIdReception( Context->GereId, IdReception, &id);
   if( retour != FIC_OK)
       return( FALSE);

   position = id->IdReception;
   while( id->PremiereAction != NO_ACTION)
   {
      retour = AbandonnerUneAction( Context, id->PremiereAction, TRUE);

      /* il faut relire, car FIC_GERE peut avoir chang‚ la valeur de l'ID */
      retour = LirePosIdReception( Context->GereId, position, &id);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AbandonnerAction
* PARAMETRES: - le contexte de la tache
*             - la position de l'action a abandonner
*             - TRUE si l'action doit ˆtre execut‚e
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Abandonne une action enregistr‚, en retirant le
*       IdContext si n‚cessaire
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour AbandonnerUneAction( TpTTacheContext  Context,
                                            long             Position,
                                            boolean          Executer)
{
   FIC_enum_retour      retour;
   TReceptionAction     *action_lue;
   TReceptionAction     action;

   TReceptionAlarme     *alarme_lue;
   TReceptionMessage     *message_lu;
   long                 *premiereAction_EVENT;
   long                 *derniereAction_EVENT;
   TEnum_Reason         raison;

   retour = LirePosAction( Context->GereActions,
                              Position,
                              &action_lue);
   if ( retour != FIC_OK)
      return( FIC_FINI);

   if ( action_lue->IndiceAction != ITEM_OCCUPE)
      return( FIC_NOK);

   action = *action_lue;
   if( action.TypeAction == RECEPTION)
   {
      premiereAction_EVENT = &(Context->PremiereActionReception);
      derniereAction_EVENT = &(Context->DerniereActionReception);
   }
   else if( action.TypeAction == ALARME)
   {
      /* lire l'alarme */
      retour = LirePosAlarme( Context->GereAlarmes,
                              action.IndiceType,
                              &alarme_lue);
      ERR_EstVrai( retour == FIC_OK);
      premiereAction_EVENT = &(alarme_lue->PremiereAction);
      derniereAction_EVENT = &(alarme_lue->DerniereAction);
   }
   else if( action.TypeAction == MESSAGE)
   {
         /* lire le message */
         retour = LirePosMessage( Context->GereMessages,
                                  action.IndiceType,
                                  &message_lu);
         ERR_EstVrai( retour == FIC_OK);
         premiereAction_EVENT = &(message_lu->PremiereAction);
         derniereAction_EVENT = &(message_lu->DerniereAction);
  }

   if( Executer)
       raison = ABORT;
   else
       raison = NO_REASON;

   ExecuterAction( Context, Position, raison,
                   FALSE,	// FR 20/03/01
                   premiereAction_EVENT, derniereAction_EVENT);

   if( action.TypeAction == ALARME)
   {
      /* ecrire l'alarme */
      retour = EcrirePosAlarme( Context->GereAlarmes,
                                action.IndiceType,
                                alarme_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else if( action.TypeAction == MESSAGE)
   {
      /* ecrire le message */
      retour = EcrirePosMessage( Context->GereMessages,
                                 action.IndiceType,
                                 message_lu);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( FIC_OK);
}



/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   DetecterAncienFichier
* PARAMETRES: - le contexte de la tache
*             - une structure d‚crivant un fichier item
*             - un boolean indiquant si l'action de r‚ception est du a
*               un changement du fichier (TRUE), ou simplement … la
*               reception du fichier
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Trouve le nom de l'ancien fichier, et d‚termine s'il est effacable
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean DetecterAncienFichier( TpTTacheContext  Context,
                                      TReception       *Id,
                                      TFichier         *RefFichier,
                                      TFichier         *RefAncienFichier,
                                      boolean          *Effacable)
{
   FIC_enum_retour      retour;
   char                 fichier_source[ RFR_MAX_PATH];

   TFichier             *refAncienFichier_lu;

   /* chercher l'ancien fichier */
   retour = LireIdReference( Context->GereAncien, RefFichier->Id,
                             RefFichier->Iteration, NULL,
                             NULL, &refAncienFichier_lu);

   if( retour != FIC_OK)
      return( FALSE);

   /* on m‚morise ici pour le retour */
   *RefAncienFichier = *refAncienFichier_lu;

   /* ce fichier est-il dans la r‚f‚rence cr‚e jusque l…*/
   retour = LireIdReference( Context->GereCreer, NULL, 0, NULL,
                             RefAncienFichier->Fichier, &refAncienFichier_lu);

   if( retour != FIC_OK)
   {
      /* il faut v‚rifier aussi qu'il n'est pas dans les ID non encore
       * trait‚s
       */
      retour = LireIdReference( Context->GereNouveau, NULL, 0, NULL,
                                RefAncienFichier->Fichier, &refAncienFichier_lu);
   }

   *Effacable = ( retour != FIC_OK);

   /* maintenant il faut verifier qu'il est sur le disque dur */
   if( *Effacable)
   {
      ChangerPath( fichier_source, RefAncienFichier->Fichier, Id->Path);
      if( _access( fichier_source, 0) != 0)
        *Effacable = FALSE;
   }

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   InitialiserAction
* PARAMETRES: - le contexte de la tache
*             - l'id du traitement
*             - la fonction Faire … appeler
*             - l'id context
*             - le type de l'action
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Initialise les champs de l'action avec les donn‚es en entr‚es
*       et incr‚mente le nombre d'actions pour cet IdContext
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void    InitialiserAction( TpTTacheContext  Context,
                                   TFcn_DoIt        Faire,
                                   TReceptionAction *Action,
                                   TIdContext       *IdContext,
                                   TEnum_TypeAction TypeAction,
                                   long             IndiceType)
{
   long                 position;

   EnregistrerIdContext( Context, IdContext, &position);

   /* initialiser l'Action */

   Action->IdReception = IdContext->IdReception;
   Action->Iteration   = IdContext->Iteration;
   Action->Faire       = Faire;

   Action->TypeAction  = TypeAction;
   Action->IndiceType  = IndiceType;

   Action->ActionPrecedente_ID            = NO_ACTION;
   Action->ActionSuivante_ID              = NO_ACTION;
   Action->ActionPrecedente_EVENT         = NO_ACTION;
   Action->ActionSuivante_EVENT           = NO_ACTION;

   /* enregistrer le lien avec l'IdContext */
   Action->IndiceIdContext = position;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerAction
* PARAMETRES: - le contexte de la tache
*             - l'action … ex‚cuter
*             - l'id de reception
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre l'action a executer, en respectant le chainage des
*       ID
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void EnregistrerAction( TpTTacheContext  Context,
                                TReceptionAction *Action,
                                TReception       *Id,
                                long             *PremiereAction_EVENT,
                                long             *DerniereAction_EVENT)
{
   FIC_enum_retour      retour;
   long                 actionPosition;

   /* on ecrit l'action pour obtenir sa position */
   retour = AjouterPosAction( Context->GereActions, Action, &actionPosition);
   ERR_EstVrai( retour == FIC_OK);

   LierAction( Context, Action, actionPosition, Id,
               PremiereAction_EVENT, DerniereAction_EVENT);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ExecuterAction
* PARAMETRES: - le contexte de la tache
*             - la position de l'action
*             - la raison d'appel
* RETOUR:     - TRUE si une action reste a faire
*             - FALSE si non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Execute une action
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean ExecuterAction( TpTTacheContext  Context,
                               long             ActionPosition,
                               TEnum_Reason     AppelRaison,
                               boolean          DeleteActionOnError,		// FR 20/03/01
                               long             *PremiereAction_EVENT,
                               long             *DerniereAction_EVENT)
{
   FIC_enum_retour      retour;
   TReceptionAction     action;
   TReceptionAction     *action_lue;
   TIdContext           *idContext;
   TReception           *id;
   TEnum_Done           fait;
   char                 commentaire[ RFR_MAX_COMMENTAIRE];
   boolean              mise_en_oeuvre;

   /* lire l'action */
   retour = LirePosAction( Context->GereActions,
                           ActionPosition,
                           &action_lue);
   ERR_EstVrai( retour == FIC_OK);
   action = *action_lue;
   commentaire[ 0] = '\0';

   /* lire le context */
   retour = LirePosIdContext( Context->GereIdContext,
                              action.IndiceIdContext,
                              &idContext);
   ERR_EstVrai( retour == FIC_OK);

   if( idContext->Iteration >= 0)
   {
      ReferenceFichierTrace( "Id '%s', It '%d', File '%s'", idContext->Id,
                             idContext->Iteration, idContext->NouveauFichier);
      ReferenceFichierTrace( "OLD[ File '%s', effacable (0,1) '%d'] ",
                             idContext->AncienFichier, idContext->EstEffacableAncienFichier);
   }
   if( idContext->Iteration == BEFORE_ITERATIONS)
      ReferenceFichierTrace( "Id '%s', BEFORE_ITERATIONS ",
                             idContext->Id);
   if( idContext->Iteration == AFTER_ITERATIONS)
      ReferenceFichierTrace( "Id '%s', AFTER_ITERATIONS",
                             idContext->Id);
   if( idContext->Iteration == MISSING_ITERATION)
      ReferenceFichierTrace( "Id '%s', MISSING_ITERATION",
                             idContext->Id);

   /* appeler la fonction faire */
   idContext->AppelRaison = AppelRaison;
   if( AppelRaison == NO_REASON)
   {
        fait = DONE;
   }
   else
   {
     if( action.Faire != NULL)
     {
        DebutRegion();
        fait = action.Faire( idContext, commentaire);
        FinRegion();
        if( commentaire[0] != '\0')
           ReferenceFichierTrace( "(%s) ", commentaire);
     }
     else
        fait = DONE;
   }

   /* une action effectu‚e en ABORT ou en erreur ne doit pas ˆtre refaite */
   if (( AppelRaison == ABORT) || ( AppelRaison == RC_ERROR))
      fait = DONE;

   /* ecrire le context */
   retour = EcrirePosIdContext( Context->GereIdContext,
                                action.IndiceIdContext,
                                idContext);
   ERR_EstVrai( retour == FIC_OK);

   /* lire l'Id de reception */
   retour = LirePosIdReception( Context->GereId, action.IdReception, &id);
   ERR_EstVrai( retour == FIC_OK);

   /* en fonction du resultat de la fonction */
   if( fait == NOT_DONE)
   {
      // Debut FR 20/03/01
      if (DeleteActionOnError)
      {
         ReferenceFichierTrace( " : NOT_DONE , RetirerAction \n");
         RetirerAction( Context,  &action, ActionPosition, id,
                        PremiereAction_EVENT, DerniereAction_EVENT);
      }
      else
      // Fin FR 20/03/01
      {
         ReferenceFichierTrace( " : NOT_DONE , DeplacerAction \n");
         DeplacerAction( Context,  &action, ActionPosition, id,
                         PremiereAction_EVENT, DerniereAction_EVENT);
      }
   }
   else
   {
      ReferenceFichierTrace( " : DONE\n");
      mise_en_oeuvre = RetirerAction( Context,  &action, ActionPosition, id,
                                      PremiereAction_EVENT,
                                      DerniereAction_EVENT);
      /* on ne mets pas en oeuvre une action qui est hors iteration */
      mise_en_oeuvre = mise_en_oeuvre &&
                       ( idContext->Iteration != BEFORE_ITERATIONS ) &&
                       ( idContext->Iteration != AFTER_ITERATIONS );

      /* on ne mets pas en oeuvre une action qui n'est pas effectu‚e */
      mise_en_oeuvre = mise_en_oeuvre &&
                       (( AppelRaison == DO_IT) || ( AppelRaison == START)
                                                || ( AppelRaison == MANUAL));

      if( mise_en_oeuvre && strcmp(commentaire, RFR_FILE_ALREADY_PRESENT) != 0)
      {
          RC_EmettreMessageUpdate( Context->util.BalMessage,
                                   Context->util.BalLocale,
                                   Context->util.NumeroPool,
                                   idContext->Id,
                                   idContext->NouvelleReference,
                                   ACTIVATED_FILE);
		  // MFR start 2007/5/22
		  ReferenceFichierTrace("<--|Message ACTIVATED_FILE, file =%s", Context->ReferenceFichierCourant);
		  // MFR end 2007/5/22
		  
		  //MTO
		  if( idContext->Iteration >= 0)
		  {
			if( idContext->EstEffacableAncienFichier)
			{
				FIC_unlink( idContext->AncienFichier );
				ReferenceFichierTrace("Deleted old file => %s", idContext->AncienFichier);
			}
		  }
		  //END MTO
      }
   }

   return( fait == DONE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RetirerAction
* PARAMETRES: - le contexte de la tache
*             - l'action
*             - l'indice de l'action
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Retire une action qui a ete execut‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE boolean   RetirerAction( TpTTacheContext  Context,
                                TReceptionAction *Action,
                                long             Position,
                                TReception       *Id,
                                long             *PremiereAction_EVENT,
                                long             *DerniereAction_EVENT)
{
   FIC_enum_retour      retour;
   boolean              aucune_action;

   aucune_action = RetirerIdContext( Context, Action->IndiceIdContext);

   DelierAction( Context, Action, Id,
                 PremiereAction_EVENT, DerniereAction_EVENT);

   /* on libere l'action */
   retour = LibererPosAction( Context->GereActions, Position);
   ERR_EstVrai( retour == FIC_OK);

   return( aucune_action);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   DeplacerAction
* PARAMETRES: - le contexte de la tache
*             - l'action
*             - l'indice de l'action
*             - le type d'action
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: D‚place une action qui a ete execut‚ mais n'a pas r‚ussi … la fin
*       de la liste
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void DeplacerAction( TpTTacheContext  Context,
                             TReceptionAction *Action,
                             long             Position,
                             TReception       *Id,
                             long             *PremiereAction_EVENT,
                             long             *DerniereAction_EVENT)
{
   DelierAction( Context, Action, Id,
                 PremiereAction_EVENT, DerniereAction_EVENT);
   LierAction( Context, Action, Position, Id,
               PremiereAction_EVENT, DerniereAction_EVENT);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   LierAction
* PARAMETRES: - le contexte de la tache
*             - l'action … ex‚cuter
*             - sa position
*             - l'id de reception
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre l'action a executer, en respectant les chainages.
*       Une action est toujours ins‚r‚e en fin de chainage
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void LierAction( TpTTacheContext  Context,
                         TReceptionAction *Action,
                         long             ActionPosition,
                         TReception       *Id,
                         long             *PremiereAction_EVENT,
                         long             *DerniereAction_EVENT)
{
   FIC_enum_retour      retour;
   TReceptionAction     *action_lue;

   /*------------------------------------------------------------------*/
   /* il faut inserer l'action dans le chainage des actions sur EVENT  */
   if ( *PremiereAction_EVENT == NO_ACTION)
   {
      *PremiereAction_EVENT = ActionPosition;
      *DerniereAction_EVENT = ActionPosition;
   }
   else
   {
      /* lire la derniŠre action, la modifier et l'ecrire */
      retour = LirePosAction( Context->GereActions,
                              *DerniereAction_EVENT,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionSuivante_EVENT = ActionPosition;
      retour = EcrirePosAction( Context->GereActions,
                                *DerniereAction_EVENT,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);

      /* modifier l'action qu'on vient de cr‚er */
      Action->ActionPrecedente_EVENT = *DerniereAction_EVENT;
      retour = EcrirePosAction( Context->GereActions,
                                ActionPosition,
                                Action);
      ERR_EstVrai( retour == FIC_OK);

      *DerniereAction_EVENT = ActionPosition;
   }

   /*---------------------------------------------------------------------*/
   /* il faut inserer l'action dans le chainage des actions pour cet id */
   if( Id->PremiereAction == NO_ACTION)
   {
      Id->PremiereAction = ActionPosition;
      Id->DerniereAction = ActionPosition;
      retour = EcrirePosIdReception( Context->GereId,
                                     Id->IdReception,
                                     Id);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      /* lire la derniŠre action, la modifier et l'ecrire */
      retour = LirePosAction( Context->GereActions,
                              Id->DerniereAction,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionSuivante_ID = ActionPosition;
      retour = EcrirePosAction( Context->GereActions,
                                Id->DerniereAction,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);

      /* modifier l'action qu'on vient de cr‚er */
      Action->ActionPrecedente_ID = Id->DerniereAction;
      retour = EcrirePosAction( Context->GereActions,
                                ActionPosition,
                                Action);
      ERR_EstVrai( retour == FIC_OK);

      /* modifier l'id */
      Id->DerniereAction = ActionPosition;
      retour = EcrirePosIdReception( Context->GereId,
                                     Id->IdReception,
                                     Id);
      ERR_EstVrai( retour == FIC_OK);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   DelierAction
* PARAMETRES: - le contexte de la tache
*             - l'action
*             - l'indice de l'action
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Retire une action qui a ete execut‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void DelierAction( TpTTacheContext  Context,
                           TReceptionAction *Action,
                           TReception       *Id,
                           long             *PremiereAction_EVENT,
                           long             *DerniereAction_EVENT)
{
   FIC_enum_retour      retour;
   TReceptionAction     *action_lue;

   /*----------------------------------------------------------------
    * il faut retirer l'action du chainage des actions sur EVENT
    */
   /* lire l'action pr‚c‚dente, la modifier et l'ecrire */
   if ( Action->ActionPrecedente_EVENT != NO_ACTION)
   {
      retour = LirePosAction( Context->GereActions,
                              Action->ActionPrecedente_EVENT,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionSuivante_EVENT = Action->ActionSuivante_EVENT;
      retour = EcrirePosAction( Context->GereActions,
                                Action->ActionPrecedente_EVENT,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
      *PremiereAction_EVENT = Action->ActionSuivante_EVENT;

   /* lire l'action suivante, la modifier et l'ecrire */
   if ( Action->ActionSuivante_EVENT != NO_ACTION)
   {
      retour = LirePosAction( Context->GereActions,
                              Action->ActionSuivante_EVENT,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionPrecedente_EVENT = Action->ActionPrecedente_EVENT;
      retour = EcrirePosAction( Context->GereActions,
                                Action->ActionSuivante_EVENT,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
      *DerniereAction_EVENT = Action->ActionPrecedente_EVENT;

   /*---------------------------------------------------------------------*/
   /* il faut retirer l'action dans le chainage des actions pour cet id */
   /* lire l'action pr‚c‚dente, la modifier et l'ecrire */
   if ( Action->ActionPrecedente_ID != NO_ACTION)
   {
      retour = LirePosAction( Context->GereActions,
                              Action->ActionPrecedente_ID,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionSuivante_ID = Action->ActionSuivante_ID;
      retour = EcrirePosAction( Context->GereActions,
                                Action->ActionPrecedente_ID,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
     Id->PremiereAction = Action->ActionSuivante_ID;

   /* lire l'action suivante, la modifier et l'ecrire */
   if ( Action->ActionSuivante_ID != NO_ACTION)
   {
      retour = LirePosAction( Context->GereActions,
                              Action->ActionSuivante_ID,
                              &action_lue);
      ERR_EstVrai( retour == FIC_OK);

      action_lue->ActionPrecedente_ID = Action->ActionPrecedente_ID;
      retour = EcrirePosAction( Context->GereActions,
                                Action->ActionSuivante_ID,
                                action_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
     Id->DerniereAction = Action->ActionPrecedente_ID;

   /* ecrire l'id */
   retour = EcrirePosIdReception( Context->GereId,
                                  Id->IdReception,
                                  Id);
   ERR_EstVrai( retour == FIC_OK);

   /* resetter les liens de l'actions en cours */
   Action->ActionPrecedente_EVENT = NO_ACTION;
   Action->ActionSuivante_EVENT = NO_ACTION;
   Action->ActionSuivante_ID = NO_ACTION;
   Action->ActionPrecedente_ID = NO_ACTION;
}




