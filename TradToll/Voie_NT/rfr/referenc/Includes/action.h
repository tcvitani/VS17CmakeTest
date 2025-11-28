/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des actions … faire
* FICHIER: action.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/action.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:40   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   03 Nov 1997 16:06:54   DPI
 * Version 4.00
 * 
 *    Rev 1.3   Jun 03 1997 15:26:18   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.2   Jun 03 1997 15:06:22   DPI
 * Supprimer dans EnregistrerActionReception
 * le param Enregistrer qui est obsolete
 * 
 *    Rev 1.1   May 12 1997 13:43:36   DPI
 * 1- Gerer les actions obligatoires
 * 2- Sur Manuel, gérer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 14:59:22   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ACTION_H
#define ACTION_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef ACTION_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/

/* sur les actions de reception */
PUBLIC void    EnregistrerActionReception( TpTTacheContext   Context,
                                           TFichier          *RefFichier,
                                           boolean           Change,
                                           boolean           FaireBefore,
                                           boolean           Manuel);
PUBLIC boolean ExecuterActionReception( TpTTacheContext  Context,
                                        TEnum_Reason     AppelRaison);

PUBLIC void    AjouterActionsAfter( TpTTacheContext  Context);

/* sur les actions d'alarmes */
PUBLIC void    EnregistrerActionAlarme( TpTTacheContext  Context,
                                        TIdContext       *IdContext,
                                        TFcn_DoIt        Faire,
                                        long             Indice,
                                        long             *PremiereAction,
                                        long             *DerniereAction);
PUBLIC boolean ExecuterActionAlarme( TpTTacheContext  Context);

/* sur les actions de messages */
PUBLIC void    EnregistrerActionMessage( TpTTacheContext  Context,
                                          TIdContext       *IdContext,
                                          TFcn_DoIt        Faire,
                                          long             Indice,
                                          long             *PremiereAction,
                                          long             *DerniereAction);
PUBLIC boolean ExecuterActionMessage( TpTTacheContext  Context);

/* sur les actions relatives a un ID */
PUBLIC boolean    AbandonnerActionsId( TpTTacheContext  Context,
                                       char             *IdReception);

/* sur TOUTES les actions */
PUBLIC void    AjouterActionsObligatoires( TpTTacheContext  Context);
PUBLIC void    AbandonnerActionsEnCours( TpTTacheContext  Context);
PUBLIC FIC_enum_retour AbandonnerUneAction( TpTTacheContext  Context,
                                            long             Position,
                                            boolean          Executer);



#endif
