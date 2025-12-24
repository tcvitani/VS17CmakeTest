/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_SERV.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions de demande de service
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emi_serv.c_v  $
 * 
 *    Rev 1.1   Apr 12 2001 16:56:12   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 14:09:06   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.18   06 Nov 1997 15:18:32   DPI
 * Integration DUT 9.00
 * 
 *    Rev 1.17   03 Nov 1997 10:59:18   DPI
 * Reprise de la gestion de HRD
 * 
 *    Rev 1.16   30 Oct 1997 15:54:10   DPI
 * Suppression du chrono, remplacer par HRD
 * Formule de calcul de l'heure
 * 
 *    Rev 1.15   29 Oct 1997 19:42:12   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 * 
 *    Rev 1.14   29 Oct 1997 18:42:46   DPI
 * Evolution ExitAlloue... du noyau 6.00
 * 
 *    Rev 1.13   Aug 01 1997 17:31:24   HMO
 *  
 * 
 *    Rev 1.12   Jul 09 1997 17:36:24   HMO
 *  
 * 
 *    Rev 1.11   Jul 08 1997 11:07:50   DPI
 * Arret meme si la tache backup refuse
 * 
 *    Rev 1.10   Jul 02 1997 15:29:22   HMO
 *  
 * 
 *    Rev 1.9   Jul 01 1997 16:07:52   HMO
 *  
 * 
 *    Rev 1.8   Jun 30 1997 16:08:34   HMO
 *  
 * 
 *    Rev 1.7   Jun 20 1997 17:35:32   HMO
 * Gestion du fichier Cold Start jusqu'a ce qu'il soit envoyé, même sur warm start
 * et suppression du fichier Cold Start quand il a été envoyé
 * 
 *    Rev 1.6   Jun 10 1997 12:00:00   DPI
 * Ouverture du service FTP une seule fois.
 * Idem fermeture
 * 
 *    Rev 1.5   May 27 1997 09:30:56   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.4   May 16 1997 13:33:04   ANA
 * Remplacement du fichier emi_act.c par msg_fic.c
 * 
 * 
 *    Rev 1.3   May 13 1997 16:51:34   ANA
 * Gestion de la sauvegarde sur disque et de la purge,
 * Nouvel algorithme de recherche du  fichier de départ.
 * 
 * 
 *    Rev 1.2   Apr 15 1997 10:03:38   ANA
 * Correction du bug sur le service FICHIER deTCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:50   ANA
 * Nouvelle gestion du "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:24:56   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "err.h"

/* module NOYAU */
#include <noyau.h>
//#include <tcp_ip.h>

/* module LAN */
#include <csr_lan.h>

/* module Horodate */
#include "horodate.h"

#include "emi_fic.h"
#include "emi_glob.h"

#define LOC_DEF
#include "emi_serv.h"
#undef LOC_DEF

#include "emi_mess.h"

#include <fic_conf.h>
#include <fic.h>
#include <msg_fic.h>
/*--------------- RESERVED: ----------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: -----------------*/

/*--------------- TYPEDEFS: ----------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: --------------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ReceptionEmiArret(struct_emi_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant l'arret du module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionEmiArret(struct_emi_message *p_message)
{
   enum_hrd_retour       retour;

   switch(p_message->entete.type_message)
   {
      case EMI_DEMANDE:
         EmiFichierTrace("Demande ARRET recu de BAL %d\n",
                                     p_message->entete.neutre.bl_retour);
         EMI.service[M_LAN_ARRET].demandeur[0] = p_message->entete.neutre.bl_retour;

         EmiEnvoiBackup(M_EMI_ARRET, NULL);
         retour = HRDAnnuleReveil (EMI.util.reveil);
         if( retour != HRD_OK)
            EmiFichierTrace( "HRD a refuse d'annuler le chrono.\n");
         else
            EmiFichierTrace( "HRD a annule le chrono.\n");
      break;

      case EMI_DEMANDE_NACQ :
         /* la demande a echoue, cependant, il faut s'arreter */
      case EMI_DEMANDE_ACQ :
         /* acquittement de l'arret de la tache BACKUP */
         /* Le test est fait sur le bal_id car il n'y a que backup */
         /* pour emettre ce message en interne */
         if( p_message->entete.neutre.bl_retour == EMI.util.bal_id )
            EMI.temoin_arret_emission = TRUE;
      break;

      default:
         /* reception d un message errone : message_id inconnu ... */
         ERR_ErreurFatale();
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionEmiEtat(struct_emi_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant les demandes d'etat
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionEmiEtat(struct_emi_message *p_message)
{
   short int cr_service;

   switch(p_message->entete.type_message)
   {
      case EMI_DEBUT:
         EmiFichierTrace("DEBUT ETAT recu de BAL %d\n",
                          			  p_message->entete.neutre.bl_retour);

         cr_service = EmiDebutService(M_EMI_ETAT,p_message->entete.neutre.bl_retour);
         if( cr_service == EMI_VALIDE )
         {
            /* remonte l'etat */
            EmiEnvoiAppli(M_EMI_ETAT,EMI_NOUVEL_ETAT,&EMI.msg_etat);
         }
      break;

      case EMI_FIN:
         EmiFichierTrace("FIN ETAT recu de BAL %d\n",
			                          p_message->entete.neutre.bl_retour);
         cr_service = EmiFinService(M_EMI_ETAT,p_message->entete.neutre.bl_retour);
      break;

      case EMI_DEMANDE:
         EmiFichierTrace("DEMANDE ETAT recu de BAL %d\n",
			                           p_message->entete.neutre.bl_retour);

         cr_service = EmiVerifDemande(p_message->entete.neutre.bl_retour,
                                       M_EMI_ETAT, EMI_DEMANDE);
         if( cr_service == EMI_VALIDE)
         {
            EmiEnvoiAcquittement(p_message->entete.neutre.bl_retour,
                                  M_EMI_ETAT,EMI_DEMANDE_ACQ);

            EmiEnvoiAppli(M_EMI_ETAT,EMI_NOUVEL_ETAT,&EMI.msg_etat);
         }
         else
            EmiEnvoiAcquittement(p_message->entete.neutre.bl_retour,
                                 M_EMI_ETAT,EMI_DEMANDE_NACQ);
      break;

      default:
         /* reception d un message errone : message_id inconnu ... */
         ERR_ErreurFatale();
      break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionEmiRestitution(struct_emi_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Transmettre les demandes de backup : transmetre a la tache
*       backup : service "monocoup", une seule demande suffit
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionEmiRestitution(struct_emi_message *p_message)
{
   switch(p_message->entete.type_message)
   {
      case EMI_DEMANDE:
         EmiFichierTrace("RESTITUTION recu de BAL %d\n",
			                             p_message->entete.neutre.bl_retour);
         EMI.service[M_EMI_RESTITUTION].demandeur[0] = p_message->entete.neutre.bl_retour;
         EMI.flag_restitution = TRUE;
         EmiEnvoiBackup(M_EMI_RESTITUTION,&p_message->u.msg_restit);
      break;

      default:
         /* reception d un message errone : message_id inconnu ... */
         ExitBad();
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionEmiPurge(struct_emi_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter une demande de purge des fichiers
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionEmiPurge(struct_emi_message *p_message)
{
   short int cr_service;

   switch(p_message->entete.type_message)
   {
      case EMI_DEBUT:
         EmiFichierTrace("DEBUT PURGE recu de BAL %d\n",p_message->entete.neutre.bl_retour);
         cr_service = EmiDebutService(M_EMI_PURGE,p_message->entete.neutre.bl_retour);
         if( cr_service == EMI_VALIDE )
         {
            /* envoi vers la tache BACKUP pour recherche */
            /* et purge des fichiers */
            EmiEnvoiBackup(M_EMI_PURGE,&p_message->u.msg_restit);
         }
      break;

      case EMI_FIN:
         EmiFichierTrace("FIN PURGE recu de BAL %d\n",p_message->entete.neutre.bl_retour);
         cr_service = EmiFinService(M_EMI_PURGE,p_message->entete.neutre.bl_retour);
      break;

      default:
         /* reception d un message errone : message_id inconnu ... */
         ExitBad();
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ReceptionEmiConfig(struct_emi_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Handles authorization message
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionEmiConfig(struct_emi_message *p_message)
{
	
	EMI.plaza_number = p_message->u.msg_config_data.plaza_number;
	EMI.lane_number = p_message->u.msg_config_data.lane_number;
	EMI.authorized = p_message->u.msg_config_data.authorization;
	EMI.cold_start = p_message->u.msg_config_data.cold_start;
	
	EmiFichierTrace("ReceptionEmiConfig => plaza = %u, lane = %u, authorization = %u, cold_start = %u",
		EMI.plaza_number, EMI.lane_number, EMI.authorized, EMI.cold_start);
	
	// cold start ?
	if( EMI.cold_start )
	{
		EMI.num_fichier.courant = 1L;
	}
	else
	{
		if (!EMI.config_received) // Si la config n'a pas deja ete recue
		{
			// recherche du premier fichier a emettre
			if (!RechercheFichierInitEmettre (FICHIER_COLD_START, &EMI.num_fichier.courant))
				ExitBad();
		}
	}

	EmiFichierTrace("EMISSION: fichier %8ld\n", EMI.num_fichier.courant);
	
	EMI.config_received = TRUE;
	
}


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void EmiEnvoiBackup(enum_emi_service service,,void *p_msg )
* PARAMETRES:
*     entree: service
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message vers la tache BACKUP
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiEnvoiBackup(enum_emi_service service, void *p_msg)
{
   struct_emi_message  *p_msg_emis = NULL;

   ExitAlloue((struct_neutre **)(&p_msg_emis),sizeof(struct_emi_message),
		EMI.util.pool);
   
   p_msg_emis->entete.service = service;

   switch(service )
   {
      case M_EMI_RESTITUTION:
         memcpy(&p_msg_emis->u.msg_restit,(struct_emi_gestion_restit *)p_msg,
                   sizeof(struct_emi_gestion_restit));
      break;

      case M_EMI_PURGE:
         memcpy(&p_msg_emis->u.msg_purge,(struct_emi_gestion_purge *)p_msg,
                   sizeof(struct_emi_gestion_purge));
      break;

      default:
      break;
   }

   ExitEnvoie(EMI.util.bal_backup,EMI.util.bal_id,(struct_neutre *)p_msg_emis);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void EmiEnvoiAppli(unsigned char bal_dest,
*                  			    short int msg)
* PARAMETRES:
*     entree: Boite aux lettres destinataire du message
*           : Message
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message vers l'application aux taches
*       qui ont ouvert le service concern‚.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiEnvoiAppli(enum_emi_service service, enum_emi_type type,
                             void *p_msg)
{
   struct_emi_message  *p_msg_emis = (struct_emi_message *)(0);
   noyau_bal_id bal_dest;
   short int i = 0;

   /* si le service est en cours */
   if( EMI.service[service].etat == EMI_EN_COURS )
   {
      bal_dest = EMI.service[service].demandeur[i];

      /* envoie a tous les demandeurs du service */
      while( bal_dest != EMI_AUCUN && i<EMI_NB_JETONS)
      {
         ExitAlloue((struct_neutre **)(&p_msg_emis),
	          sizeof(struct_emi_message),EMI.util.pool);
         
         p_msg_emis->entete.service = service;
         p_msg_emis->entete.type_message = type;

         /* en fonction du service */
         switch(service )
         {
            case M_EMI_ETAT:
               memcpy(&p_msg_emis->u.msg_etat,(struct_emi_gestion_etat *)p_msg, sizeof(struct_emi_gestion_etat));
            break;

            default:
            break;
         }

         ExitEnvoie(bal_dest,EMI.util.bal_id,(struct_neutre *)p_msg_emis);
         bal_dest = EMI.service[service].demandeur[++i];
      }
   }
   else
   {
      EmiFichierTrace("EmiEnvoiAppli => service %d non active\n",service);
   }
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionEmiFic(struct_emi_message *)
* PARAMETRES: pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages recus du module LAN
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void ReceptionEmiFic(struct_lan_message *p_message)
{
   switch(p_message->entete.service)
   {
      case M_LAN_FICHIER :
         switch(p_message->entete.type_message)
         {
            case LAN_DEBUT_ACQ :
               /* l'emission de fichier est possible */
               EMI.flag_emission = TRUE;
            break;

            case LAN_FIN_ACQ :
               /* l'emission de fichier est termin‚e */
               EMI.flag_emission = FALSE;
            break;

            case LAN_EMISSION_RENOMME_EFFECTUEE:
			case LAN_EMISSION_EFFECTUEE:

               /* renommer le fichier emis */
               RenommerFichier(p_message->u.param_fic.fichier);

               /* preparer le prochain fichier */
               FichierSuivant(TRUE,p_message->u.param_fic.fichier);
            break;

            /* l'emission du fichier n'a pas r‚ussie : nouvelle tentative */
            case LAN_EMISSION_RENOMME_NON_EFFECTUEE:
			case LAN_EMISSION_NON_EFFECTUEE:
            case LAN_EMISSION_NACQ:
               FichierSuivant(FALSE,p_message->u.param_fic.fichier);
            break;

            case LAN_DEBUT_NACQ :
               ERR_ErreurFatale();
            break;

            case LAN_FIN_NACQ:
            case LAN_EMISSION_ACQ :
            default:
            break;
         }

      break;

      default :
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void FichierSuivant( boolean flag_fic,
*                                            char *path_fichier)
* PARAMETRES: Flag sur l'etat de l'acauittement FIC recu
*             path du fichier transmis
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Incremente le numero du fichier courant a emettre
*       Lance le chrono d'emission automatique ou
*       envoie un evenement a la tache de BACKUP pour signaler
*       l'acquittement FIC.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void FichierSuivant(boolean flag_fic,char *path_fichier)
{
   long num_fichier_recu = -1L;
   //char *ptr_nom;
   char drive[MAX_PATH];
   char path[MAX_PATH];
   char ficname[MAX_PATH];
   char extension[MAX_PATH];

   /* verifier le fichier emis sur recherche du nom seul */
   //ptr_nom = path_fichier + strlen(path_fichier) -
            //( FICHIER_TAILLE_NOM + FICHIER_TAILLE_EXT ) +3;

   FIC_splitpath(path_fichier, drive, path, ficname, extension);
   
   //if( ptr_nom != NULL)
   if(strcmp("", ficname) != 0)
      //num_fichier_recu = atol( ptr_nom );
	  num_fichier_recu = atol( ficname );

   /* fichier courant */
   if( num_fichier_recu == EMI.num_fichier.courant )
   {
      /* si acquittement FIC ok, on peut passer au fichier suivant */
      if( flag_fic )
         EMI.num_fichier.courant ++;
   }
   /* Acquittement d'un fichier de backup */
   else if( EMI.flag_restitution  && num_fichier_recu == EMI.num_fichier.backup)
   {
      /* si acquittement FIC ok, on peut passer au fichier suivant */
      if( flag_fic )
         EMI.num_fichier.backup ++;

      /* signaler le compte-rendu FIC : correct ou non */
      if( SignalEvt(EMI.event) != NOYAU_OK )
        ExitBad();

   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int EmettreFichier( char *fichier)
* PARAMETRES: fichier : fichier … emettre
* RETOUR:     envoi correct ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: envoie un fichier au module LAN pour une emission par FIC
*      vers le distant
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int EmettreFichier(char *fichier, char *path_distant,
                      noyau_bal_id bal_dest,noyau_bal_id bal_source)
{
   short int retour = CSR_OK;
   struct_lan_message_externe  message;

   message.entete.service = M_LAN_FICHIER;
   //message.entete.type_message = LAN_EMISSION;
   message.entete.type_message = LAN_EMISSION_RENOMME;

   message.u.param_fic.fichier = fichier;
   message.u.param_fic.path    = path_distant;
   /* pas de nom pour le distant : serveur FIC par defaut */
   message.u.param_fic.hostname = EMI.util.DIR.hostname;

   if( EmiEnvoiLan( bal_dest, bal_source, EMI.util.pool, &message) == NOYAU_BAL_PLEINE)
   {
      ExitBad();
   }

   return retour;
}
