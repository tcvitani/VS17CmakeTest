/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Traitement de la reception Inter Tache
* FICHIER: RC_recpt.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Referenc/Sources/rc_recpt.c_v  $
 * 
 *    Rev 1.1   Sep 03 2001 13:11:24   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:52   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.9   03 Nov 1997 16:07:30   DPI
 * Version 4.00
 * 
 *    Rev 1.8   29 Oct 1997 19:13:54   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.7   Jul 09 1997 10:09:44   DPI
 * Changement de l'ordre des includes du
 * a evolution dans 'err.h'
 * 
 *    Rev 1.6   Jun 13 1997 10:52:44   DPI
 * Correction d'une bug dans les traces
 * 
 *    Rev 1.5   Jun 10 1997 14:58:02   DPI
 * Amelioration des traces
 * 
 *    Rev 1.4   Jun 04 1997 19:13:16   DPI
 *  
 * 
 *    Rev 1.3   Jun 03 1997 17:21:00   DPI
 * Ajout de traces
 * 
 *    Rev 1.2   Jun 03 1997 11:50:24   DPI
 * Integration du message Applicatif dans les
 * messages de services; on ne différencie plus 
 * à la reception la BalMessage et les bals 
 * applicatives
 * 
 *    Rev 1.1   Apr 28 1997 15:37:32   DPI
 *  
 *
 *    Rev 1.0   Apr 24 1997 15:02:24   DPI
 *  
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>

/* outils_C */
#include "str.h"
#include "err.h"

/* module noyau */
#include "noyau.h"
//#include "tcp_ip.h"
#include "csr_lan.h"
#include "horodate.h"

#define RC_RECPT_DEF

#include "rc_def.h"
#include "fic_gere.h"
#include "fic.h"

#include "referenc.h"

#include "rc_glob.h"

#include "rc_recpt.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PRIVATE void ReceptionRfrMessageReference (TpTTacheContext     Context,
                                           struct_rfr_message  *p_message);
PRIVATE void ReceptionRfrMessageManuel (TpTTacheContext     Context,
                                        struct_rfr_message  *p_message);
PRIVATE void ReceptionRfrMessageApplicatif (TpTTacheContext     Context,
                                            struct_rfr_message  *p_message);

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionRfrService
* PARAMETRES: - Context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionRfrService (TpTTacheContext     Context,
                                    struct_rfr_message  *p_message)
{
   ReferenceFichierTrace ("MSG_RFR: bal=%d",
                          p_message->entete.neutre.bl_retour);

   switch (p_message->entete.service)
   {
      case M_RFR_ARRET:
         ReferenceFichierTrace (", M_RFR_ARRET\n");
         Context->util.TemoinArret = TRUE;
         Context->action.ActionId = ACTION_MSG_TERMINATE;
         break;

      case M_RFR_REFERENCE:
         ReferenceFichierTrace (", M_RFR_REFERENCE\n");
         ReceptionRfrMessageReference( Context, p_message);
         break;

      case M_RFR_MANUEL:
         ReferenceFichierTrace (", M_RFR_MANUEL\n");
         ReceptionRfrMessageManuel( Context, p_message);
         break;

      case M_RFR_APPLICATIF:
         ReferenceFichierTrace (", M_RFR_APPLICATIF\n");
         ReceptionRfrMessageApplicatif( Context, p_message);
         break;

      default:
         ReferenceFichierTrace (", unknown service (%d)\n",
                                p_message->entete.service);
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionRfrAlarme
* PARAMETRES: - Context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionRfrAlarme (TpTTacheContext     Context,
                                   struct_hrd_message  *p_message)
{
   ReferenceFichierTrace ("MSG_HRD: bal=%d\n",
                          p_message->entete.neutre.bl_retour);

   switch (p_message->entete.service)
   {
      case M_HRD_GESTION_ALARME:
         Context->action.ActionId = ACTION_MSG_ALARME;
         if (p_message->entete.type_message == HRD_ALARME)
            Context->action.u.alarme.Id = p_message->u.msg_gestion_alarme.num_alarme;
         else
            ERR_ErreurFatale();
         break;

      default:
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionRfrFichier
* PARAMETRES: - le context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionRfrFichier (TpTTacheContext     Context,
                                    struct_lan_message  *p_message)
{
   char ext[MAX_PATH];

   ReferenceFichierTrace ("ReceptionRfrFichier: MSG_LAN: bal=%d",
                          p_message->entete.neutre.bl_retour);

   if (p_message->entete.service != M_LAN_FICHIER)
   {
      ReferenceFichierTrace (", unknown service (%d)\n",
                             p_message->entete.service);
      return;
   }

   switch (p_message->entete.type_message)
   {
      case LAN_DEBUT_ACQ:
         ReferenceFichierTrace(", LAN_DEBUT_ACQ\n");
         Context->action.ActionId = ACTION_MSG_SERVICE_FICHIER;
         break;

      case LAN_DEBUT_NACQ:
         /* Erreur de programmation */
         ERR_ErreurFatale();
         break;

      case LAN_FIN_ACQ:
         /* Event inattendu */
         ERR_ErreurFatale();
         break;

      case LAN_FIN_NACQ:
         /* Event inattendu */
         ERR_ErreurFatale();
         break;

      case LAN_RECEPTION_ACQ:
         /* cas normal, on ne fait rien */
         ReferenceFichierTrace(", LAN_RECEPTION_ACQ\n");
         Context->action.ActionId = NO_ACTION_TO_DO;
         break;

      case LAN_RECEPTION_NACQ:
         /* ceci denote une grave erreur dans le code */
         ERR_ErreurFatale();
         break;

      case LAN_RECEPTION_EFFECTUEE:
         ReferenceFichierTrace (", LAN_RECEPTION_EFFECTUEE => received file '%s'\n",
                                p_message->u.param_fic.fichier);
         // Recuperer uniquement nom fichier et extension
         FIC_splitpath (p_message->u.param_fic.fichier, NULL, NULL, Context->action.u.charger.Fichier, ext);
		 strcat_s(Context->action.u.charger.Fichier, sizeof(Context->action.u.charger.Fichier), ext);

         STR_strcpy( MAX_PATH, Context->action.u.charger.Path, p_message->u.param_fic.path);

         Context->action.ActionId = ACTION_MSG_FICHIER_CHARGER;
         break;

      case LAN_RECEPTION_NON_EFFECTUEE:
         ReferenceFichierTrace(", LAN_RECEPTION_NON_EFFECTUEE\n");
         // Recuperer uniquement nom fichier et extension
         FIC_splitpath (p_message->u.param_fic.fichier, NULL, NULL, Context->action.u.charger.Fichier, ext);
		 strcat_s(Context->action.u.charger.Fichier, sizeof(Context->action.u.charger.Fichier), ext);

         STR_strcpy( MAX_PATH, Context->action.u.charger.Path, p_message->u.param_fic.path);

         Context->action.ActionId = ACTION_MSG_FICHIER_NON_CHARGER;
         break;

      default:
         ReferenceFichierTrace (", unknown message type (%d)\n",
                                p_message->entete.type_message);
         break;
   }
}


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionRfrTime
* PARAMETRES: - le context de la tache
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement en l'absence de messages
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionRfrTime (TpTTacheContext     Context)
{
   Context->action.ActionId = ACTION_TIME;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:     void ReceptionRfrMessageReference
* PARAMETRES: - le context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void ReceptionRfrMessageReference (TpTTacheContext     Context,
                                           struct_rfr_message  *p_message)
{
   switch (p_message->entete.type_message)
   {
      case RFR_RECEPTION_REFERENCE:
         STR_strcpy( MAX_PATH, Context->action.u.reference.Fichier, p_message->u.message_ref.fichier);
         STR_strcpy( MAX_PATH, Context->action.u.reference.Reference, p_message->u.message_ref.reference);
         Context->action.ActionId = ACTION_MSG_REFERENCE;
         ReferenceFichierTrace ("reference file received [%s - ID %s]\n",
                                p_message->u.message_ref.fichier, p_message->u.message_ref.reference);
         break;

      default:
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    void ReceptionRfrMessageManuel
* PARAMETRES: - le context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionRfrMessageManuel (TpTTacheContext     Context,
                                          struct_rfr_message  *p_message)
{
   switch (p_message->entete.type_message)
   {
      case RFR_FICHIER_MANUEL:
         STR_strcpy( MAX_PATH, Context->action.u.manuel.Fichier, p_message->u.message_manuel.fichier);
         STR_strcpy( MAX_PATH, Context->action.u.manuel.Id, p_message->u.message_manuel.id);
         Context->action.ActionId = ACTION_MSG_MANUEL;
         break;

      default:
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    void ReceptionRfrMessageApplicatif
* PARAMETRES: - le context de la tache
*             - pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void ReceptionRfrMessageApplicatif (TpTTacheContext     Context,
                                            struct_rfr_message  *p_message)
{
   Context->action.ActionId = ACTION_MSG_APPLICATIF;
   Context->action.u.applicatif.Bal = p_message->entete.neutre.bl_retour;
   Context->action.u.applicatif.TypeMessage = p_message->entete.type_message;
}
