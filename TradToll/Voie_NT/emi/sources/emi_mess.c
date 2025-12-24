/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_MESS.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions traitant les types de messages recus par
*         la tache EMISSION
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emi_mess.c_v  $
 * 
 *    Rev 1.1   Apr 12 2001 16:56:12   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 14:09:06   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.9   30 Oct 1997 15:53:52   DPI
 * Suppression du chrono, remplacer par HRD
 * Formule de calcul de l'heure
 * 
 *    Rev 1.8   29 Oct 1997 18:42:44   DPI
 * Evolution ExitAlloue... du noyau 6.00
 * 
 *    Rev 1.7   Aug 01 1997 17:30:24   HMO
 *  
 * 
 *    Rev 1.6   Jul 02 1997 15:29:18   HMO
 *  
 * 
 *    Rev 1.5   Jun 30 1997 16:08:32   HMO
 *  
 * 
 *    Rev 1.4   May 16 1997 13:32:58   ANA
 * Remplacement du fichier emi_act.c par msg_fic.c
 * 
 * 
 *    Rev 1.3   May 13 1997 16:51:34   ANA
 * Gestion de la sauvegarde sur disque et de la purge,
 * Nouvel algorithme de recherche du  fichier de départ.
 * 
 * 
 *    Rev 1.2   Apr 15 1997 10:03:38   ANA
 * Correction du bug sur le service FICHIER de TCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:48   ANA
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
#include <dos.h>
#include <io.h>

/* module NOYAU */
#include <noyau.h>
//#include <tcp_ip.h>
#include <str.h>

/* module LAN */
#include <csr_lan.h>

/* module Horodate */
#include "horodate.h"

#include "emi_fic.h"
#include "emi_glob.h"

#define LOC_DEF
#include "emi_mess.h"
#undef LOC_DEF

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
* SYNTAXE: PROTECTED short int EmiDebutService(short int service_id,
*                                              short int bal_dest)
* PARAMETRES:
*     entree: identificateur du service
*           : boite aux lettres de la tache utilisatrice du service
*     retour: demande valide ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache EMISSION
* ROLE: Acceptation ou refus de la demande de debut de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int EmiDebutService(enum_emi_service service_id,noyau_bal_id bal_dest)
{
   short int cr_demande;

   /* verifier que le service n'est pas deja pris par cette appli */
   /* et que la liaison serie est operationnelle */
   cr_demande = EmiVerifDemande(bal_dest,service_id,EMI_DEBUT);

   if( cr_demande == EMI_VALIDE )
   {
      /* des qu'un demandeur prend le service, il est en cours */
      EMI.service[service_id].etat     |= EMI_EN_COURS;
      EmiEnvoiAcquittement(bal_dest,service_id,EMI_DEBUT_ACQ);
   }
   else
   {
      EmiEnvoiAcquittement(bal_dest,service_id,EMI_DEBUT_NACQ);
      EmiFichierTrace("Debut Service %d non valide \n",service_id);
   }

   return(cr_demande);

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int FinService( short int service_id,
*                                       unsigned char bal_dest)
* PARAMETRES:
*     entree: identificateur du service
*           : boite aux lettres de la tache utilisatrice du service
*     retour: demande valide ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache EMISSION
* ROLE: Acceptation ou refus de la demande de fin de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int EmiFinService(enum_emi_service service_id,
				                      noyau_bal_id bal_dest)
{
   short int cr_demande;

   /* verifier que le service a bien ete pris par cette appli */
   cr_demande = EmiVerifDemande(bal_dest,service_id,EMI_FIN);

   if( cr_demande == EMI_VALIDE )
   {
      /* le service n'est lib‚r‚ que s'il n'y a plus de demandeur */
      EMI.service[service_id].etat     |= EMI_LIBRE;
      EmiEnvoiAcquittement(bal_dest, service_id, EMI_FIN_ACQ);
   }
   else
   {
      EmiEnvoiAcquittement(bal_dest, service_id, EMI_FIN_NACQ);
      EmiFichierTrace("Fin Service %d non valide \n",service_id);
   }

   return(cr_demande);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int EmiVerifDemande( short int bal,
*                                               enum_emi_service service,
*                                               enum_emi_type type)
* PARAMETRES:
*     entree: numero de boite aux lettres de la tache demandant une reponse
*           : service demande : arret / etat / restitution
*           : type de demande : debut / fin / demande de service
*     retour: booleen donnant autorisant ou non la demande
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Verification de la demande de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int EmiVerifDemande(noyau_bal_id bal,
                                    enum_emi_service service_id,
                                    enum_emi_type type)
{
   short int etat = EMI_NON_VALIDE;
   short int i;

   switch(type)
   {
      case EMI_DEBUT:
         /* tester si le service n'est pas deja pris */
         if( EMI.service[service_id].etat == EMI_EN_COURS )
         {
            /* recherche du premier jeton disponible pour le service */
            for( i=0; i<EMI_NB_JETONS; i++)
            {
               if( EMI.service[service_id].demandeur[i] == EMI_AUCUN )
               {
                  EMI.service[service_id].demandeur[i] = bal;
                  break;
               }
            }
            /* plus de jetons disponibles */
            if( i == EMI_NB_JETONS )
               etat = EMI_NON_VALIDE;
            else
               etat = EMI_VALIDE;
         }
         else
         {
            /* premier jeton pour le service */
            etat = EMI_VALIDE;
            EMI.service[service_id].demandeur[0] = bal;
         }
      break;

      case EMI_FIN:
      case EMI_DEMANDE:
         /* tester si le service est effectivement pris */
         if( EMI.service[service_id].etat == EMI_LIBRE )
         {
            etat = EMI_NON_VALIDE;
            break;
         }
         else
         {
            /* tester si le demandeur du service ou de fin du service
               est le meme que celui qui a ouvert ce service */
            for( i=0; i<EMI_NB_JETONS; i++)
            {
               if( EMI.service[service_id].demandeur[i] != bal)
                  etat |= EMI_NON_VALIDE;
               else
               {
                  if( type == EMI_FIN )
                     EMI.service[service_id].demandeur[i] = EMI_AUCUN;
                  etat |= EMI_VALIDE;
               }
            }
         }
      break;

      default:
         etat = EMI_NON_VALIDE;
      break;
   }

   if( etat == EMI_NON_VALIDE)
      EmiFichierTrace("Demande Service %d non valide \n",service_id);

   return(etat);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void EmiEnvoiAcquittement(unsigned char bal_dest,
*                                           short int service_id,
*                                           short int type_message)
* PARAMETRES:
*     entree: boite aux lettres destinataire
*           : identificateur du service
*           : type du message d'acquittement a renvoyer
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message d'acquittement vers l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiEnvoiAcquittement(noyau_bal_id bal_dest,
		      enum_emi_service service_id,enum_emi_type type_message)
{
   struct_emi_message  *p_message = NULL;

   ExitAlloue((struct_neutre **)(&p_message),
	       sizeof(struct_emi_message),EMI.util.pool);
   
   p_message->entete.service = service_id;
   p_message->entete.type_message = type_message;
   ExitEnvoie(bal_dest,EMI.util.bal_id,(struct_neutre *)(p_message));
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int EmiEnvoiFic( enum_lan_service, enum_lan_type)
* PARAMETRES:
* RETOUR:     envoi correct ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int EmiEnvoiFic(enum_lan_service service,enum_lan_type type,
                                struct_lan_gestion_fic *p_msg)
{
   short int retour = CSR_OK;
   struct_lan_message_externe message;

   message.entete.service      = service;
   message.entete.type_message = type;
   if( p_msg != NULL )
   {
      message.u.param_fic.fichier  = p_msg->fichier;
      message.u.param_fic.path     = p_msg->path;
      message.u.param_fic.hostname = p_msg->hostname;
   }

   if( EmiEnvoiLan( EMI.util.bal_fic, EMI.util.bal_id, EMI.util.pool, &message) == NOYAU_BAL_PLEINE)
   {
      ExitBad();
   }

   return retour;
}

PROTECTED noyau_enum_retour EmiEnvoiLan(noyau_bal_id bal_dest,
                                        noyau_bal_id bal_source,
                                        noyau_pool_id pool,
                                        struct_lan_message_externe *p_msg)
{
//	noyau_taille_bloc taille_alloue;
	struct_lan_message *p_msg_lan;

	//taille_alloue = sizeof(struct_lan_entete);

	switch(p_msg->entete.service)
	{
		case M_LAN_FICHIER:
			switch(p_msg->entete.type_message)
			{
				case LAN_EMISSION :
				case LAN_RECEPTION :
				case LAN_EMISSION_RENOMME :
				case LAN_RECEPTION_RENOMME :
					//taille_alloue += sizeof(struct_lan_gestion_fic);
					ExitAlloue((struct_neutre **)(&p_msg_lan),sizeof(struct_lan_message)/*taille_alloue*/,pool);
					p_msg_lan->entete.service = M_LAN_FICHIER;
					p_msg_lan->entete.type_message = p_msg->entete.type_message;
					STR_strcpy( _MAX_PATH,
								p_msg_lan->u.param_fic.fichier,
								p_msg->u.param_fic.fichier);
					STR_strcpy( _MAX_PATH,
								p_msg_lan->u.param_fic.path,
								p_msg->u.param_fic.path);
					STR_strcpy(	_MAX_PATH,
								p_msg_lan->u.param_fic.hostname,
						        p_msg->u.param_fic.hostname);
					break;

				case LAN_DEBUT:
				case LAN_FIN:
					ExitAlloue((struct_neutre **)(&p_msg_lan),sizeof(struct_lan_message)/*taille_alloue*/,pool);
					p_msg_lan->entete.service = p_msg->entete.service;
					p_msg_lan->entete.type_message = p_msg->entete.type_message;
					break;

				default:
					break;
		}
	}

	ExitEnvoie(bal_dest, bal_source, (struct_neutre *)(p_msg_lan));

	return NOYAU_OK;
}
