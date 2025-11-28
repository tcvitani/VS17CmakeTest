/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/

/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: lan_ani.c
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Tache d'interface entre une application et les taches de
*         communication LAN.
*
* --------------------------------------------------------------------
* DESCRIPTION:
*       1 - GŠre les demandes de connexions,
*       2 - Transmet les demandes d'emission vers les taches concernées :
*              - Emission d'un message vers la tache EMISSION,
*              - Emission d'un fichier vers la tache FTP
*       3 - Transmet les messages recus du distant vers l'application
*
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_ani.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:52   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:14   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:36:00   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:58   bph
 *  
 * 
 *    Rev 1.16   05 Feb 1998 10:40:38   BPH
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <conio.h>
#include <io.h>
#include <string.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <lan_glob.h>

#include <atm_lan.h>
#include <lan_serv.h>
#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/


/*--------------- TYPEDEFS: ---------------*/



/*--------------- FUNCTIONS: ---------------*/
PROTECTED short int InitLan(void * param);
PROTECTED void LanGestionEtat(short int ident, struct_lan_gestion_etat * p_etat)
;
PROTECTED void ReceptionLanService(short int ident, struct_lan_message * p_msg);
PROTECTED void ViderBal(short int /*ident*/ );





/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitLan (void)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int InitLan(void *param)
{
   short int ident;
   
   /* recuperation du numero d'instance utilise */
   ident = (unsigned short)param;
   
   /**************************************************************/
   /* On positionne le temoin qui permet de savoir si l'arret de */
   /* la tache a ete demande                                     */
   /**************************************************************/
   LAN[ident].temoin_arret = FALSE;
   LAN[ident].nb_tache_arret = 0;
   
   /* Pas de connexion */
   LAN[ident].param.pipe = LAN_NO_PIPE_OPENED;
   
   /* etat courant */
   LAN[ident].msg_etat.serveur_msg = LAN[ident].msg_etat.serveur_msg_old = 
         LAN_ATTENTE_CONNEXION;
   LAN[ident].msg_etat.serveur_fic = LAN[ident].msg_etat.serveur_fic_old = 
         LAN_TRANSFERT_FICHIER_OK;
   LAN[ident].msg_etat.serveur_tps = LAN[ident].msg_etat.serveur_tps_old = 
         LAN_LIAISON_TPS_HS;
   return ident;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int Lan (void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps reel
* ROLE: Recuperer les messages qui arrivent dans la boite aux lettres
*       de la tache et les rediriger vers le reseau.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI Lan_ani(LPVOID param)
{
   struct_lan_message * p_message;
   short int ident;
   noyau_bal_id lan_bal;
  
   /********************************/
   /* Initialisation de la tache   */
   /* et changement de sa priorite */
   /********************************/
   
   DebutRegion();
   
   ident = InitLan(param);
   
   
   /* numero d'identificateur inconnu */
   if (ident >= LAN_NB_IDENT_MAX)
   {
      ExitBad();
   }
   
   // publication de la BAL ANI
   lan_bal = LAN[ident].lan_bal = PublieBAL(LAN[ident].nom_lan_bal, 
         NOYAU_BAL_ILLIMITEE);
   
   if (LAN[ident].lan_bal <= 0)
      ExitBad();
   
   ChangePriorite(TacheCourante(), LAN[ident].lan_priorite_max);
   
   FinRegion();
   
   // attente des autres bals
   LanAttendBAL(ident);
   
   
   
   
   /*************************/
   /* Traitement periodique */
   /*************************/
   while (TRUE)
   {
      /*****************************************************/
      /* Attente de message dans la BAL interne a la tache */
      /*****************************************************/
      
      _DebutRegion();
      
     
      /* BAL d'interface avec l'application */
      if (TestRecoit(lan_bal, (struct_neutre * *)&p_message) == NOYAU_BAL_MESS)
      {
        
         // message interne ou applicatif ?
         if (ATM_LAN_Reception_Message_BAL(LAN[ident].atm_lan, (struct_neutre *) 
                  p_message) == FALSE)
            ReceptionLanService(ident, p_message);
         
         ExitLibere((struct_neutre * *)&p_message);
      }
      
      if (LAN[ident].temoin_arret == FALSE)
         LanGestionEtat(ident, &LAN[ident].msg_etat);
      else
      {
         _FinRegion();
         Termine();
      }
      
      _FinRegion();
      
	  Sleep(LAN[ident].dwSleepTimeMs);

      // pour laisser le temps au reseau d'ecouler ses messages...
   }
   
   return 0;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void LanGestionEtat (short int ident)
* PARAMETRES: identificateur
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Verifie les changements d'etats et avertit l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanGestionEtat(short int ident, struct_lan_gestion_etat *p_etat)
{
   boolean flag_changement = FALSE;
   
   if (p_etat->serveur_msg != p_etat->serveur_msg_old)
      flag_changement = TRUE;
   
   if (p_etat->serveur_fic != p_etat->serveur_fic_old)
      flag_changement = TRUE;
   
   if (p_etat->serveur_tps != p_etat->serveur_tps_old)
      flag_changement = TRUE;
   
   if (flag_changement)
   {
      /* envoi du message d'etat a chaque demandeur du service */
      LanEnvoiService(ident, M_LAN_ETAT, LAN_NOUVEL_ETAT, p_etat);
      
      /* sauvegarde de l'etat courant */
      p_etat->serveur_msg_old = p_etat->serveur_msg;
      p_etat->serveur_fic_old = p_etat->serveur_fic;
      p_etat->serveur_tps_old = p_etat->serveur_tps;
   }
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionLanService( short int ident,
*                                                struct_lan_message *)
* PARAMETRES: pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Dirige les messages recus de l'application vers l'une des taches
*       EMIS/RECEP/FTP.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanService(short int ident, struct_lan_message *
      p_message)
{
   switch (p_message->entete.service)
   {
      case M_LAN_ARRET:
         ReceptionLanArret(ident, p_message);
         break;
      
      case M_LAN_DOP:
         ReceptionLanDop(ident, p_message);
         break;
      
      case M_LAN_ETAT:
         ReceptionLanEtat(ident, p_message);
         break;
      
      case M_LAN_CONNEXION:
         ReceptionLanConnexion(ident, p_message);
         break;
      
      case M_LAN_MESSAGE:
         ReceptionLanMessage(ident, p_message);
         break;
      
      case M_LAN_FICHIER:
         ReceptionLanFichier(ident, p_message);
         break;
      
      case M_LAN_HORAIRE:
         ReceptionLanHoraire(ident, p_message);
         break;
      
      //      case M_LAN_VIDER_BAL :
      //         ViderBal(ident);
      //         break;
      
      default:
         LanFichierTrace(ident, "LAN_ANI : message_id recu inconnu %d", 
               p_message->entete.service);
         break;
   }
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ViderBal(short int ident)
* PARAMETRES: identificateur
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Redirige les messages vers l'application ou les autres taches
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ViderBal(short int ident)
{
   struct_neutre * p_neutre;
   int i = 0;
   
   LanFichierTrace(ident, "***** Vidange BAL tache EMISSION ****");
   
   
   /* test des messages present dan le BAL et libération */
   /* ils sont donc perdus !!!                           */
   while (TestRecoit(LAN[ident].emis_bal, &p_neutre) == NOYAU_BAL_MESS)
   {
      LanFichierTrace(ident, " ** BAL %d : msg#%d = %d %d %d **", LAN[ident].
            emis_bal, i++, p_neutre->bl_id, p_neutre->bl_retour);
      ExitLibere(&p_neutre);
   }
}
