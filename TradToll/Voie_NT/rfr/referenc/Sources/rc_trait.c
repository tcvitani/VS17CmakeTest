/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement principal
* FICHIER: rc_trait.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
*             Ce fichier contient le traitement principal de la tache
*             de R‚ception du fichier de R‚ference.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/rc_trait.c_v  $
 * 
 *    Rev 1.1   22 Mar 2001 10:03:54   FR
 * - correction du blocage du module suite à une erreur de transfert de liste
 * - correction de la gestion de l'état de prise en compte d'une nouvelle référence
 * 
 * 
 *    Rev 1.0   14 Dec 1999 15:17:52   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.17   19 Mar 1998 14:32:16   DPI
 * - sur MESSAGE_MANUEL, le fichier manuel est vérifié
 * - MESSAGE_MANUEL_ACK est envoyé à la tache appelante
 * - sur Cold Start, tous les repertoires applicatifs
 *   sont effacés
 * 
 *    Rev 1.16   03 Nov 1997 16:07:32   DPI
 * Version 4.00
 * 
 *    Rev 1.15   30 Oct 1997 11:57:20   DPI
 * Integration de FICTR_copy de OUTILS_C 2.00
 * 
 *    Rev 1.14   Jul 28 1997 18:30:34   DPI
 * Gestion de la version
 * 
 *    Rev 1.13   Jul 18 1997 17:57:58   DPI
 *
 * 
 *    Rev 1.12   Jul 02 1997 11:46:08   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.11   Jun 18 1997 11:36:02   DPI
 * Enrichissement des traces
 * 
 *    Rev 1.10   Jun 13 1997 10:42:54   DPI
 *  
 *
 *    Rev 1.9   Jun 12 1997 14:02:50   DPI
 * Correction d'une transition impasse.
 * 
 *    Rev 1.8   Jun 11 1997 17:51:52   DPI
 *  
 * 
 *    Rev 1.7   Jun 11 1997 17:50:28   DPI
 * Encore ameliorer les traces
 * 
 *    Rev 1.6   Jun 10 1997 16:46:08   DPI
 * Amelioration des traces
 * 
 *    Rev 1.5   Jun 10 1997 15:24:52   DPI
 * Amelioration de la trace
 * 
 *    Rev 1.4   Jun 04 1997 19:13:16   DPI
 *  
 * 
 *    Rev 1.3   May 13 1997 09:52:24   DPI
 * Blocage du temps sur alarme
 * 
 *    Rev 1.2   May 12 1997 13:44:40   DPI
 * 1- Generer les actions obligatoires
 * 2- Sur Manuel, gerer comme cas standard
 * 
 *    Rev 1.1   Apr 24 1997 16:45:46   DPI
 * Annuler un reveil qui a sonné après son
 * traitement
 * 
 *    Rev 1.0   Apr 24 1997 15:02:24   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define RC_TRAIT_DEF

/* gonfler la taille de la pile */
//extern unsigned _stklen = 64536U;

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "err.h"
#include "fic.h"
#include "str.h"

#include "fic_gere.h"

#include "rc_def.h"

#include "fic_ref.h"

#include "referenc.h"

#include "rc_glob.h"

#include "rc_emis.h"
#include "item_fic.h"
#include "refer.h"
#include "action.h"
#include "alarme.h"
#include "message.h"
#include "path.h"
#include "comp_ref.h"

#include "rc_conf.h"

/* ### FICTR_xxx remplace par FIC_xxx */
#include "fic.h"

#include "rc_trait.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/
#define  FORMAT_TRACE_EVENT      ", Event '%s'\n"
#define  FORMAT_TRACE_PHASE      "\n => Phase '%s'"
#define  FORMAT_TRACE_ACTION     "--- Action '%s' "

/*--------------- RESERVED: ---------------*/

PRIVATE void DemarreReference( TpTTacheContext Context, boolean ExisteFichier);

/*--------------- FUNCTION: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ReferenceTraitement
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Effectue les traitements fonctionnels
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReferenceTraitement( TpTTacheContext    Context)
{
   char             fichier_tmp[ RFR_MAX_FIC];
   boolean          ok;
   boolean          encore;
   boolean          existeAncien;
   TEnum_Event      event;

   event = NOTHING;
   switch( Context->action.ActionId)
   {
      case ACTION_START: /* demarrage des services FILES */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "START");
              if ( Context->PhaseEnCours == NO_STEP)
              {
				   event = START_REFERENCE;
              }
              break;

      case ACTION_MSG_SERVICE_FICHIER : /* reception du message de service fichier */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_SERVICE_FICHIER");
              Context->ServiceFichierOuvert = TRUE;
              break;

      case ACTION_MSG_REFERENCE: /* reception du message de r‚f‚rence */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_REFERENCE");
              if( ( Context->PhaseEnCours == WAITING_REFERENCE_FILE) &&
                  ( Context->ServiceFichierOuvert))
              {
                 ReferenceFichierTrace(" Nouvelle ref '%s'",
                                  Context->action.u.reference.Fichier);
                 event = REFERENCE_MESSAGE;
              }
              else
              {
                 if( ! Context->ServiceFichierOuvert)
                 {
                    ReferenceFichierTrace(" Service Fichier Non Ouvert,");
                 }

                 ReferenceFichierTrace(" Ref for future '%s'",
                                       Context->action.u.reference.Fichier);
                 event = REFERENCE_MESSAGE_FOR_FUTURE;
              }
              break;

      case ACTION_MSG_MANUEL: /* reception du message de r‚f‚rence */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_MANUEL");
              if( Context->PhaseEnCours == WAITING_REFERENCE_FILE)
              {
                 ReferenceFichierTrace(" accept‚");
                 event = MANUAL_CHANGE;
              }
              else
              {
                 ReferenceFichierTrace(" refus‚\n");
                 RC_EmettreMessageManuel( Context->util.BalRetour,
                                          Context->util.BalLocale,
                                          Context->util.NumeroPool,
                                          Context->action.u.manuel.Id,
                                          Context->action.u.manuel.Fichier,
                                          FALSE);
              }
              break;

      case ACTION_MSG_FICHIER_NON_CHARGER: /* FIC a echoue la copie */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_FICHIER_NON_CHARGER");
              if( strcmp( Context->action.u.reference.Fichier,
                          Context->NomFichierNouveau) == 0)
              {
                 if( Context->PhaseEnCours == LOADING_REFERENCE_FILE)
                 {
                    ReferenceFichierTrace( " Ref '%s'",
                                           Context->action.u.charger.Fichier);
                    event = FAILURE_LOAD_REFERENCE_FILE;
                 }
              }
              else
              {
                 if( Context->PhaseEnCours == WAITING_FILES)
                 {
                    ReferenceFichierTrace( " Fichier '%s'",
                                           Context->action.u.charger.Fichier);
                    event = FAILURE_LOAD_FILE;
                 }
              }
              break;

      case ACTION_MSG_FICHIER_CHARGER: /* FIC a fini la copie */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_FICHIER_CHARGER");
              if( strcmp( Context->action.u.reference.Fichier,
                          Context->NomFichierNouveau) == 0)
              {
                 if( Context->PhaseEnCours == LOADING_REFERENCE_FILE)
                 {
                    ReferenceFichierTrace(" Ref '%s'",
                                           Context->action.u.charger.Fichier);
                    event = REFERENCE_FILE_LOADED;
                 }
              }
              else
              {
                 if( Context->PhaseEnCours == WAITING_FILES)
                 {
                    ReferenceFichierTrace(" Fichier '%s'",
                                           Context->action.u.charger.Fichier);
                    event = FILE_LOADED;
                 }
              }
              break;

      case ACTION_MSG_APPLICATIF: /* un evenement applicatif s'est produit */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_APPLICATIF");
               if( Context->PhaseEnCours == WAITING_REFERENCE_FILE)
               {
                  ReferenceFichierTrace(" accept‚");
                  event = RECEIVE_MESSAGE_ACTION;
               }
               else
               {
                  ReferenceFichierTrace(" refus‚\n");
               }
               break;

      case ACTION_MSG_ALARME: /* une alarme s'est produite */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_ALARME");
               if( Context->PhaseEnCours == WAITING_REFERENCE_FILE)
               {
                  ReferenceFichierTrace(" accept‚e");
                  event = RECEIVE_ALARM_ACTION;
               }
               else
               {
                  ReferenceFichierTrace(" refus‚e\n");
               }
               break;

      case ACTION_MSG_TERMINATE: /* Terminer */
              ReferenceFichierTrace( FORMAT_TRACE_ACTION, "MSG_TERMINATE");
               event = TERMINATE_REFERENCE;
               break;

      case ACTION_TIME: /* action sans r‚ception de message */
               if( Context->PhaseEnCours == COMPARING_REFERENCE_FILES)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = COMPARE_REFERENCE_FILE;
               }
               else if( Context->PhaseEnCours == DOING_RECEPTION_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = DO_RECEPTION_ACTION;
               }
               else if( Context->PhaseEnCours == CLOSING_RECEPTION_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = CLOSE_RECEPTION_ACTIONS;
               }
               else if( Context->PhaseEnCours == DOING_ALARM_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = DO_ALARM_ACTION;
               }
               else if( Context->PhaseEnCours == CLOSING_ALARM_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = CLOSE_ALARM_ACTIONS;
               }
               else if( Context->PhaseEnCours == DOING_MESSAGE_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = DO_MESSAGE_ACTION;
               }
               else if( Context->PhaseEnCours == CLOSING_MESSAGE_ACTIONS)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = CLOSE_MESSAGE_ACTIONS;
               }
               else if( ( Context->PhaseEnCours == WAITING_REFERENCE_FILE) &&
                        ( Context->ServiceFichierOuvert) &&
                        ( Context->NomFichierFutur[ 0] != '\0'))
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  /* il faut vider ces champs */
                  Context->action.u.reference.Fichier[0] = '\0';
                  Context->action.u.reference.Reference[0] = '\0';
                  event = REFERENCE_MESSAGE;
               }
               else if( Context->PhaseEnCours == WAITING_FILES)
               {
                  ReferenceFichierTrace( FORMAT_TRACE_ACTION, "TIME");
                  event = NO_FILE_LOADED;
               }
               else
               {
                  event = NOTHING;
               }
               break;

      case NO_ACTION_TO_DO:
               break;

      default: /* pas d'event => Impossible */
               ERR_ErreurFatale();
               break;
   }
   Context->action.ActionId = NO_ACTION_TO_DO;

   /* Fin de l'‚valuation de l'action */
   /*---------------------------------------------------------*/
   ReferenceFichierTrace( "ReferenceTraitement - event:%u",event);


   switch( event)
   {
      case START_REFERENCE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "START_REFERENCE");
               /* on determine le nom du fichier de reference courant
                * qui va ˆtre utilise en mode DEGRADE comme un fichier
                * de reference standard
                */
               FIC_makepath( fichier_tmp, RFR.disque_virtuel, RFR.path_reference,
                                           FICHIER_REFERENCE_COURANT, NULL);
               RC_LireConfiguration( Context);

               if( Context->NomFichierNouveau[ 0] != '\0')
               {
                  /* il y a un fichier de reference a mettre en oeuvre */
                  ReferenceFichierTrace( "Ref '%s'\n", Context->NomFichierNouveau);

                  /* y a-t'il un fichier de reference ancien */
                  existeAncien = ( Context->NomFichierCourant[ 0] != '\0');
                  if( existeAncien)
                  {
                     if( ! EchangerFichierReference( Context->GereAncien, Context->NomFichierCourant, "r+t"))
                     {
                        /* si on ne le trouve pas, on considŠre
                         * qu'il n'existe pas
                         */
                        Context->NomFichierCourant[ 0] = '\0';
                        Context->ReferenceFichierCourant[ 0] = '\0';
                        existeAncien = FALSE;
                     }
                  }

                  /* l'ancien n'existe pas, on considŠre le fichier de ref courant
                   * comme l'ancien fichier de ref, et on le cr‚e
                   */
                  if( ! existeAncien)
                  {
                     if( ! EchangerFichierReference( Context->GereAncien, fichier_tmp,
                                                     "a+t"))
                     {
                        ERR_ErreurFatale();
                     }
                  }

                  //DemarreReference( Context, TRUE);
               }
               else
               {
                  /* il n'y a pas de fichier de reference standard.
                   * On cr‚e le fichier courant et on le laisse vide
                   */
                  ReferenceFichierTrace( "No Ref\n");
                  if( ! EchangerFichierReference( Context->GereAncien,
                                                  fichier_tmp,
                                                  "a+t"))
                      ERR_ErreurFatale();

                  //DemarreReference( Context, FALSE);
               }
               /*--------------------------------------------*/
               /* le d‚marrage est fini */
               Context->util.Demarrage = FALSE;

               /*--------------------------------------------*/
               /* on ouvre le service Fichier */
               RC_OuvrirServiceFichier( Context->util.BalFichier,
                                        Context->util.BalLocale,
                                        Context->util.NumeroPool);
               /* on n'attends pas d'acquittement */
               Context->PhaseEnCours = WAITING_REFERENCE_FILE;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               ReferenceFichierTrace("\n");
               break;

      case REFERENCE_MESSAGE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "REFERENCE_MESSAGE");
               if( Context->action.u.reference.Fichier[0] == '\0')
               {
                  /* on utilise un message d‚j… recu */
                  ChangerPathTransfert( Context->NomFichierNouveau, Context->NomFichierFutur);
                  STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierNouveau, Context->ReferenceFichierFutur);
               }
               else
               {
                  STR_strcpy( RFR_MAX_FIC, Context->NomFichierFutur, Context->action.u.reference.Fichier);
                  ChangerPathTransfert( Context->NomFichierNouveau, Context->action.u.reference.Fichier);
                  STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierNouveau, Context->action.u.reference.Reference);
               }

               /* Context->NomFichierFutur contient le nom du futur fichier */
               ok = DemanderFichierReference( Context);
               if( ! ok)
               {
                  /* c'est le mˆme fichier de r‚f‚rence */
                  RC_EmettreMessageUpdate( Context->util.BalMessage,
                                           Context->util.BalLocale,
                                           Context->util.NumeroPool,
                                           Context->IdFichierReference,
                                           Context->action.u.reference.Reference,
                                           ALREADY_CHECKED_FILE);
                  Context->NomFichierNouveau[ 0] = '\0';
                  Context->ReferenceFichierNouveau[ 0] = '\0';
               }
               else
               {
                  /* c'est un nouveau */
                  /* le nom du fichier est le nom distant */
                  STR_strcpy( RFR_MAX_FIC, Context->NomFichierNouveau,
                                       Context->NomFichierFutur);
                  RC_DemanderChargementFichier( Context->util.BalFichier,
                                                Context->util.BalLocale,
                                                Context->util.NumeroPool,
                                                Context->NomFichierNouveau,
                                                RFR.path_transfert);
                                                //NULL);
                  Context->PhaseEnCours = LOADING_REFERENCE_FILE;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "LOADING_REFERENCE_FILE");
               }

               Context->NomFichierFutur[ 0] = '\0';
               Context->ReferenceFichierFutur[ 0] = '\0';
               ReferenceFichierTrace("\n");
               break;

      case REFERENCE_MESSAGE_FOR_FUTURE :
               ReferenceFichierTrace(", Event '%s'\n", "REFERENCE_MESSAGE_FOR_FUTURE");
               if( Context->NomFichierFutur[ 0] != '\0')
               {
                  if( _stricmp( Context->ReferenceFichierFutur,
                               Context->action.u.reference.Reference) != 0)
                  {
                    /* on perd une demande */
                    RC_EmettreMessageUpdate( Context->util.BalMessage,
                                             Context->util.BalLocale,
                                             Context->util.NumeroPool,
                                             Context->IdFichierReference,
                                             Context->ReferenceFichierFutur,
                                             TRASHING_FILE);
                  }
               }
               /* on m‚morise qu'il faudra changer de fichier de ref */
               /* le nom du fichier est le nom distant */
               STR_strcpy( RFR_MAX_FIC, Context->NomFichierFutur, Context->action.u.reference.Fichier);
               STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierFutur,
                                    Context->action.u.reference.Reference);
               ReferenceFichierTrace("\n");
               break;


      case MANUAL_CHANGE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "MANUAL_CHANGE");
               ok = VerifierFichierManuel( Context);
               if( ! ok)
               {
                 RC_EmettreMessageManuel( Context->util.BalRetour,
                                          Context->util.BalLocale,
                                          Context->util.NumeroPool,
                                          Context->action.u.manuel.Id,
                                          Context->action.u.manuel.Fichier,
                                          FALSE);
               }
               else
               {
                  ok = AbandonnerActionsId( Context, Context->action.u.manuel.Id);
                  if( ! ok)
                  {
                     /* pb */
                     ERR_ErreurFatale();
                  }
                  else
                  {
                     STR_strcpy( RFR_MAX_FIC, Context->NomFichierNouveau, FICHIER_REFERENCE_MANUELLE);
                     STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierNouveau, ID_REFERENCE_MANUELLE);

                     CreerFichierReferenceManuel( Context);
                     AcquitterFichierReference( Context, TRUE);

                     encore = ExecuterActionReception( Context, MANUAL);
                     ERR_EstVrai( ! encore);

                     RC_EmettreMessageManuel( Context->util.BalRetour,
                                              Context->util.BalLocale,
                                              Context->util.NumeroPool,
                                              Context->action.u.manuel.Id,
                                              Context->action.u.manuel.Fichier,
                                              TRUE);
                  }
               }
               ReferenceFichierTrace("\n");
               break;

      case FAILURE_LOAD_REFERENCE_FILE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "FAILURE_LOAD_REFERENCE_FILE");
               RC_EmettreMessageUpdate( Context->util.BalMessage,
                                        Context->util.BalLocale,
                                        Context->util.NumeroPool,
                                        Context->IdFichierReference,
                                        Context->ReferenceFichierNouveau,
                                        UNLOAD_FILE);
               Context->NomFichierNouveau[ 0] = '\0';
               Context->ReferenceFichierNouveau[ 0] = '\0';
               Context->PhaseEnCours = WAITING_REFERENCE_FILE;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               ReferenceFichierTrace("\n");
               break;

      case FAILURE_LOAD_FILE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "FAILURE_LOAD_FILE");
               RecevoirUnFichier( Context, Context->action.u.reference.Fichier, FALSE);
               ReferenceFichierTrace("\n");
               break;

      case REFERENCE_FILE_LOADED :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "REFERENCE_FILE_LOADED");
               /* on teste si le fichier est correcte dans l'espace de
                * transfert, avant de le recopier
                */
               ChangerPathTransfert( Context->NomFichierNouveau,
                                     Context->action.u.reference.Fichier);
               RecevoirFichierReference( Context);
               ok = EstCorrecteReference( Context->GereNouveau);
			   Fichier_Fermer(Context->GereNouveau);
               if( ! ok)
               {
                  // Debut FR 20/03/01
                  Context->TousFichiersValides = FALSE;
                  // Fin FR 20/03/01
                  AcquitterFichierReference( Context, FALSE);
                  Context->PhaseEnCours = WAITING_REFERENCE_FILE;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               }
               else
               {
				   /* Le fichier est test‚, il doit ˆtre copier dans l'espace
                   * de r‚f‚rence
                   */

                  ChangerPathReference( Context->NomFichierNouveau,
                                        Context->action.u.reference.Fichier);
                  ChangerPathTransfert( fichier_tmp,
                                        Context->action.u.reference.Fichier);

                  /* on efface l'ancien fichier de mˆme nom */
                  if( _access( Context->NomFichierNouveau, 0) == 0)
                     FIC_unlink( Context->NomFichierNouveau);
				
				  //Copy ref file path - to delete old ref file
				  strncpy_s(Context->RefFileToDelete, sizeof(Context->RefFileToDelete), Context->RefFileCurrent, sizeof(Context->RefFileToDelete));
				  Context->RefFileToDelete[sizeof(Context->RefFileToDelete)-1]='\0';
				  strncpy_s(Context->RefFileCurrent, sizeof(Context->RefFileCurrent), Context->NomFichierNouveau, sizeof(Context->RefFileCurrent));
				  Context->RefFileCurrent[sizeof(Context->RefFileCurrent)-1]='\0';

					/* ### FICTR_xxx remplace par FIC_xxx */
                  ok = FIC_copy( Context->NomFichierNouveau, fichier_tmp,
                                   FIC_COPY_WITH_DATE/*, RFR_TIMEOUT_RAPIDE*/);

                  ERR_EstVrai( ok);

                  /* il faut de nouveau mettre … jour les gestionnaires */
                  RecevoirFichierReference( Context);

                  /* il faut l'effacer de l'espace de transfert aprŠs avoir
                   * mise a jour les gestionnaires
                   */
                  FIC_unlink( fichier_tmp);

                  RC_EmettreMessageUpdate( Context->util.BalMessage,
                                           Context->util.BalLocale,
                                           Context->util.NumeroPool,
                                           Context->IdFichierReference,
                                           Context->ReferenceFichierNouveau,
                                           CHECKED_FILE);
                  AbandonnerActionsEnCours( Context);
                  Context->PhaseEnCours = COMPARING_REFERENCE_FILES;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "COMPARING_REFERENCE_FILES");
               }
               ReferenceFichierTrace("\n");
               break;

      case FILE_LOADED :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "FILE_LOADED");
               RecevoirUnFichier( Context, Context->action.u.reference.Fichier, TRUE);
               ReferenceFichierTrace("\n");
               break;

      case NO_FILE_LOADED :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "NO_FILE_LOADED");
               if( Context->NbFichiersReclames == 0)
               {
                  /* tous les fichiers ont ‚t‚ re‡us et v‚rifi‚s
                   * on peut acquitter le fichier de reference.
                   */
                  AcquitterFichierReference( Context, TRUE);
                  AjouterActionsAfter( Context);
                  AjouterActionsObligatoires( Context);
                  ReferenceFichierTrace("\n\n--------------------------------------------------------\n");
                  ReferenceFichierTrace("Remplacement de l'ancien fichier de reference par celui cree\n");
                  Context->PhaseEnCours = DOING_RECEPTION_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "DOING_RECEPTION_ACTIONS");
               }
               ReferenceFichierTrace("\n");
               break;


      case COMPARE_REFERENCE_FILE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "COMPARE_REFERENCE_FILE");
               ComparerFichiersReference( Context);
               Context->PhaseEnCours = WAITING_FILES;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_FILES");
               ReferenceFichierTrace("\n");
               break;

      case DO_RECEPTION_ACTION : /* effectuer une action de r‚ception */
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "DO_RECEPTION_ACTION");
               encore = ExecuterActionReception( Context, DO_IT);
               if( ! encore)
               {
                  Context->PhaseEnCours = CLOSING_RECEPTION_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "CLOSING_RECEPTION_ACTIONS");
				  //MTO - delete old ref file:
				  if(Context->RefFileToDelete[0]!='\0')
				  {
					  if(strcmp(Context->RefFileToDelete, Context->RefFileCurrent) != 0)
					  {
						  if(_access( Context->RefFileToDelete, 0) == 0)
						  {
							  FIC_unlink( Context->RefFileToDelete);
							  ReferenceFichierTrace( "ReferenceTraitement()::DO_RECEPTION_ACTION REF file [ %s ] deleted", Context->RefFileToDelete);
						  }
						  else
							ReferenceFichierTrace( "ReferenceTraitement()::DO_RECEPTION_ACTION REF file [ %s ] cannot be deleted -> does not exist on disk!", Context->RefFileToDelete);
					  }
					  else
						ReferenceFichierTrace( "ReferenceTraitement()::DO_RECEPTION_ACTION REF file [ %s ] cannot be deleted -> same as current REF file!", Context->RefFileToDelete);
				  }
				  else
					ReferenceFichierTrace( "ReferenceTraitement()::DO_RECEPTION_ACTION REF file cannot be deleted -> is empty!");			  

				  //MOB - delete unused regKeys
				  DeleteUnusedRegKey(Context);
               }
               break;

      case CLOSE_RECEPTION_ACTIONS :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "CLOSE_RECEPTION_ACTIONS");
               /* toutes les actions sur  r‚ception sont faites */
               Context->PhaseEnCours = WAITING_REFERENCE_FILE;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               break;

      case RECEIVE_ALARM_ACTION : /* enregistrer le d‚clenchement d 1 alarme */
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "RECEIVE_ALARM_ACTION");
               ok = RecevoirAlarme( Context, &(Context->action.u.alarme.Id));
               if( ok)
               {
                  Context->PhaseEnCours = DOING_ALARM_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "DOING_ALARM_ACTIONS");
               }
               break;

      case DO_ALARM_ACTION : /* effectuer une action d'alarme */
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "DO_ALARM_ACTION");
               encore = ExecuterActionAlarme( Context);
               if( ! encore)
               {
                  EffacerAlarmeUsee( Context, &(Context->action.u.alarme.Id));
                  Context->PhaseEnCours = CLOSING_ALARM_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "CLOSING_ALARM_ACTIONS");
               }
               break;

      case CLOSE_ALARM_ACTIONS :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "CLOSE_ALARM_ACTIONS");
               Context->AlarmeEnCours = NO_ALARME;
               Context->PhaseEnCours  = WAITING_REFERENCE_FILE;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               ReferenceFichierTrace("\n");
               break;

      case RECEIVE_MESSAGE_ACTION : /* enregistrer la reception d'un message applicatif */
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "RECEIVE_MESSAGE_ACTION");
               ok = RecevoirMessage( Context,
                                     Context->action.u.applicatif.Bal,
                                     Context->action.u.applicatif.TypeMessage);
               if( ok)
               {
                  Context->PhaseEnCours = DOING_MESSAGE_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "DOING_MESSAGE_ACTIONS");
               }
               ReferenceFichierTrace("\n");
               break;

      case DO_MESSAGE_ACTION : /* effectuer une action d'alarme */
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "DO_MESSAGE_ACTION");
               encore = ExecuterActionMessage( Context);
               if( ! encore)
               {
                  Context->PhaseEnCours = CLOSING_MESSAGE_ACTIONS;
                  ReferenceFichierTrace( FORMAT_TRACE_PHASE, "CLOSING_MESSAGE_ACTIONS");
               }
               ReferenceFichierTrace("\n");
               break;

      case CLOSE_MESSAGE_ACTIONS :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "CLOSE_MESSAGE_ACTIONS");
               Context->MessageEnCours = NO_MESSAGE;
               Context->PhaseEnCours  = WAITING_REFERENCE_FILE;
               ReferenceFichierTrace( FORMAT_TRACE_PHASE, "WAITING_REFERENCE_FILE");
               ReferenceFichierTrace("\n");
               break;

      case TERMINATE_REFERENCE :
               ReferenceFichierTrace( FORMAT_TRACE_EVENT, "TERMINATE_REFERENCE");
               /* ici on ne fait que la partie li‚e … la phase en cours */
               if( Context->PhaseEnCours == COMPARING_REFERENCE_FILES)
               {
                  if( ! FermerFichierReference( Context->GereNouveau))
                     ERR_ErreurFatale();
                  if( ! FermerFichierReference( Context->GereCreer))
                     ERR_ErreurFatale();
               }

               /* le fichier suivant est ouvert dans le traitement */
               if( ! FermerFichierReference( Context->GereAncien))
                  ERR_ErreurFatale();
               ReferenceFichierTrace("\n");
               break;

      case NOTHING:
              break;

      default:
              ERR_ErreurFatale();
              break;
   }
}


/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE : DemarreReference
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Demarre le traitement du fichier de reference
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void DemarreReference( TpTTacheContext Context, boolean ExisteFichier)
{
   boolean     encore;

   if( ExisteFichier)
   {
      RecevoirFichierReference( Context);
      Context->PhaseEnCours = COMPARING_REFERENCE_FILES;
      /*-----------------------------------------------*/

      ComparerFichiersReference( Context);
      if( Context->NbFichiersReclames != 0)
         ERR_ErreurFatale();
      Context->PhaseEnCours = WAITING_FILES;
      /*----------------------------------*/

      AcquitterFichierReference( Context, TRUE);
      AjouterActionsAfter( Context);
   }

   /* dans tous les cas, il faut ajouter les actions obligatoires */
   AjouterActionsObligatoires( Context);
   Context->PhaseEnCours = DOING_RECEPTION_ACTIONS;

   /*--------------------------------------------*/

   encore = TRUE;
   while( encore)
   {
      encore = ExecuterActionReception( Context, START);
   }
   /*--------------------------------------------*/

   Context->PhaseEnCours = WAITING_REFERENCE_FILE;
}

