/*------   (v) 1998 CS-Route   -----------    Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_REC .C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: tache RECEPTION
* --------------------------------------------------------------------
* DESCRIPTION: Lecture alternative de la BAL et de la socket
*
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_rec.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:50:00   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* 
*    Rev 1.2   Jan 20 1999 11:36:06   bph
*  
* 
*    Rev 1.1   02 Oct 1998 11:46:00   bph
*  
* 
*    Rev 1.16   03 Nov 1997 11:14:36   BPH
* Passage en ExitAlloue, ExitLibere, ExitEnvoie
* Utilisation du debug noyau 6
* 
*
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <string.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <atm_lan.h>
#include <lan_glob.h>
#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PRIVATE enum_lan_erreur GestionReception(short int ident, HANDLE hPipe, DWORD 
      delai);

PRIVATE void LanErreurRecep(short int ident);






/*--------------- VARIABLES: --------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int LanReception (void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps reel RECEPTION
* ROLE: Recuperer les messages qui arrivent dans la boite aux lettres
*       De la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI LanReception(LPVOID param)
{
   short int ident;
   
   /* initialisation de la tache et changement de priorite */
   DebutRegion();
   
   ident = (short)param;
   
   
   /* numero d'identificateur inconnu */
   if (ident >= LAN_NB_IDENT_MAX)
   {
      ExitBad();
   }
   
   ChangePriorite(TacheCourante(), LAN[ident].lan_priorite_max);
   
   FinRegion();
   
   // attente des autres bals
   LanAttendBAL(ident);
   
   
   /* attente et traitement des messages issus de LAN */
   while (TRUE)
   {
      _DebutRegion();
      
      
      /* surveiller le socket */
      if (GestionReception(ident, LAN[ident].param.pipe, 500L) != LAN_OK)
      {
         if (LAN[ident].temoin_arret)
         {
            LanFichierTrace(ident, "LAN_REC : reception demande ARRET");
            
            /* acquittement vers la tache principale du module */
            LanEnvoiAcquittement(ident, LAN[ident].lan_bal, M_LAN_ARRET, 
                  LAN_DEMANDE_ACQ);
            
            _FinRegion();
            
            Termine();
         }
         else
         {
            _FinRegion();
			Sleep(LAN[ident].dwSleepTimeMs);
            _DebutRegion();
         }
      }
      
      _FinRegion();
   }
   
   return 0;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void GestionReception(int socket)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Gestion des messages recus par le reseau
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE enum_lan_erreur GestionReception(short int ident, HANDLE hPipe, DWORD 
      delai)
{
   enum_lan_erreur result;
   struct_lan_gestion_msg msg_recu;
   struct_lan_msg * p_message = &LAN[ident].read_message;
   
   if (LAN[ident].msg_etat.serveur_msg != LAN_CONNEXION)
      return LAN_NOK;
   
   /* lecture du message a recevoir */
   _FinRegion();
   result = LanReceive(ident, hPipe, p_message, sizeof(struct_lan_msg), delai);
   _DebutRegion();
   
   
   /* filtre sur les messages applicatifs  ou les messages de service */
   if (result == LAN_OK)
   {
      if (ATM_LAN_Reception_Message_Reseau(LAN[ident].atm_lan, &p_message->prot) 
            == FALSE)
      {
         // reception d'un message applicatif
         
         msg_recu.longueur = p_message->prot.MsgSize;
         
         /* recopie du contenu du message */
         memcpy(msg_recu.contenu, &p_message->data, msg_recu.longueur);
         
         /********************/
         /* Envoi du message */
         /********************/
         LanEnvoiService(ident, M_LAN_MESSAGE, LAN_RECEPTION, (char *)&msg_recu);
         
         LanFichierTrace(ident, "LAN_REC : reception du msg '%s' ", p_message->
               data);
         
         LanEnvoiDop(ident, p_message->data, (short)(p_message->prot.MsgSize), 
               SRV_ESPION_ENTRANT, LAN_DOP_MSG);
      }
      else
         
         // réception d'un message de vie
         LanEnvoiDop(ident, p_message->data, (short)(p_message->prot.MsgSize), 
               SRV_ESPION_ENTRANT, LAN_DOP_VIE);
      
      /* reinitialisation du buffer de reception */
      /* pour les traces, cela permet d'avoir le caractére de fin de chaine */
      memset(p_message, 0, LAN_LG_TRAME);
   }
   else if (result == LAN_NOK)
   {
      LanErreurRecep(ident);
   }
   
   return result;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void LanErreurRecep (void);
* PARAMETERS: aucun
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: afficher les erreurs survenues a la reception dans la pipe
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void LanErreurRecep(short int ident)
{
   SYSTEMTIME today;
   
   /* un problŠme d'emission de message est survenu */
   /*** R‚cup‚ration de la date et de l'heure */
   GetLocalTime(&today);
   LanFichierTrace(ident, " =>  pb  reception le %2d/%2d/%4u a %02d-%02d-%02d", 
         today.wDay, today.wMonth, today.wYear, today.wHour, today.wMinute, 
         today.wSecond);
   
   LAN[ident].msg_etat.serveur_msg = LAN_DEGRADE;
   
   /* deconnexion / reconnexion automatique */
   LanBreak(ident);
}
