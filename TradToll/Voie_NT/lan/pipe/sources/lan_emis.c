/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_EMIS.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code de la tache d'EMISSION par LAN/IP
* --------------------------------------------------------------------
* DESCRIPTION: La tache LAN_EMIS est la tache de communication
*              Elle recoit des messages dans sa BAL de LAN_IP
*              et émet par le pipe les messages recus.
*              Elle émet periodiquement un "message de vie" pour surveiller
*              l'état de la connexion si aucun autre message n'est émis.
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_emis.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:54   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:35:56   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:54   bph
 *  
 * 
 *    Rev 1.19   09 Dec 1997 11:26:10   BPH
 * Ajout de la bal_emis_message pour les messages applicatifs
 * Limitation de cette bal et test de la stack ip
 *   
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <lan_glob.h>

#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/

#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: -----------------*/
/*--------------- TYPEDEFS: ----------------*/
/*--------------- FUNCTIONS: ---------------*/

PRIVATE void LanErreurEmis(short int);




/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: --------------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int LanEmission(void)
* PARAMETRES:
*     entree: rien
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps reel lan_emis
* ROLE: Receptionner les messages qui arrivent de la tache LanAni
*       et dialoguer avec le pipe par la liaison série.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI LanEmission(LPVOID param)
{
   noyau_enum_retour code_rtc;
   noyau_bal_id bal_emis;
   short int ident;
   struct_lan_message * p_message;
   enum_lan_erreur retour;
   
   /* initialisation de la tache et changement de priorite */
   DebutRegion();
   
   ident = (short)param;
   
   
   /* numero d'identificateur inconnu */
   if (ident >= LAN_NB_IDENT_MAX)
   {
      ExitBad();
   }
   
   // publication de la BAL EMI
   bal_emis = LAN[ident].emis_bal = PublieBAL(LAN[ident].nom_emis_bal, 100);
   
   if (LAN[ident].emis_bal <= 0)
      ExitBad();
   
   ChangePriorite(TacheCourante(), LAN[ident].lan_priorite_max);
   
   FinRegion();
   
   // attente des autres bals
   LanAttendBAL(ident);
   
   
   /* attente et traitement des messages issus de LAN_ANI*/
   while (TRUE)
   {
      code_rtc = Recoit(bal_emis, (struct_neutre * *)&p_message, 
            LAN_TIMEOUT_RECOIT);
      
      _DebutRegion();
      
      if (code_rtc == NOYAU_BAL_MESS)
      {
         switch (p_message->entete.type_message)
         {
            case LAN_EMISSION:
               /* messages applicatifs a transmettre vers le LS */
               do
               {
                  if (LAN[ident].msg_etat.serveur_msg != LAN_CONNEXION)
                     retour = LAN_NOK;
                  else
                     retour = LanEnvoiMessage(ident, p_message->u.message.
                           longueur, PROT_APPLICATIF, p_message->u.message.
                           contenu);
                  
                  // si l'emission echoue, on boucle sur l'envoi du meme message
                  // jusqu'a ce qu'il parte, les nouveaux sont stockés dans la BAL en attendant
                  if (LAN[ident].temoin_arret)
                  {
                     LanFichierTrace(ident, "LAN_EMIS : reception demande ARRET"
                           );
                     
                     /* acquittement vers la tache principale du module */
                     LanEnvoiAcquittement(ident, LAN[ident].lan_bal, M_LAN_ARRET
                           , LAN_DEMANDE_ACQ);
                     
                     _FinRegion();
                     
                     /* fin de la tache dans le prochain FinRegion();*/
                     Termine();
                  }
                  
                  if (retour != LAN_OK)
                  {
                     _FinRegion();
					 Sleep(LAN[ident].dwSleepTimeMs);
                     _DebutRegion();
                  }
               
               } while(retour != LAN_OK);
               break;
            
            default:
               LanFichierDebug(ident, 
                     "LAN_EMIS : reception type inconnu %d ds bal_emis_mess", 
                     p_message->entete.type_message);
               break;
         }
         
         ExitLibere((struct_neutre * *)(&p_message));
      }
      
      if (LAN[ident].temoin_arret)
      {
         LanFichierTrace(ident, "LAN_EMIS : reception demande ARRET");
         
         /* acquittement vers la tache principale du module */
         LanEnvoiAcquittement(ident, LAN[ident].lan_bal, M_LAN_ARRET, 
               LAN_DEMANDE_ACQ);
         
         _FinRegion();
         
         /* fin de la tache dans le prochain FinRegion();*/
         Termine();
      }
      
      _FinRegion();
   }
   
   return 0;
}


PROTECTED enum_lan_erreur LanEnvoiMessage(short int ident, DWORD MsgSize, 
      enum_prot_type MsgType, void *data)
{
   struct_lan_msg message;
   enum_lan_erreur result = LAN_OK;
   enum_lan_msg_dop type_dop;
   
   /* initialisation */
   memset(&message, 0, LAN_LG_TRAME);
   
   message.prot.MsgSize = MsgSize;
   message.prot.MsgType = MsgType;
   GetSystemTimeAsFileTime((FILETIME *)&message.prot.MsgDate);
   
   if (MsgSize > 0)
      memcpy(&message.data, data, MsgSize);
   
   result = LanTransmit(ident, LAN[ident].param.pipe, &message);
   
   if (result == LAN_OK)
   {
      
      /* DOP */
      switch (MsgType)
      {
         case PROT_DEMANDE_VIE:
         case PROT_REPONSE_VIE:
            type_dop = LAN_DOP_VIE;
            break;
         
         default:
         case PROT_APPLICATIF:
            type_dop = LAN_DOP_MSG;
            
            LanFichierTrace(ident, "LAN_EMI : emission du msg '%s' ", message.
                  data);
            break;
      }
      
      LanEnvoiDop(ident, message.data, (unsigned short)(MsgSize), SRV_ESPION_SORTANT, 
            type_dop);
   }
   else
   {
      LanErreurEmis(ident);
      
      result = LAN_NOK;
   }
   
   return result;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void LanErreurEmis(short int ident)
* PARAMETRES: identificateur
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Avertit l'application lors d'un problŠme d'emission
*       avec rupture de la connexion
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanErreurEmis(short int ident)
{
   SYSTEMTIME today;
   
   /* un problŠme d'emission de message est survenu */
   /*** Récupération de la date et de l'heure */
   GetLocalTime(&today);
   LanFichierTrace(ident, 
         "LAN_EMIS : => pb emission le %2d/%2d/%4u a %02d-%02d-%02d ", today.
         wDay, today.wMonth, today.wYear, today.wHour, today.wMinute, today.
         wSecond);
   LAN[ident].msg_etat.serveur_msg = LAN_DEGRADE;
   
   //   LanEnvoiService(ident,
   //                   M_LAN_ETAT,LAN_NOUVEL_ETAT,&LAN[ident].msg_etat);
   
   /* deconnexion / reconnexion automatique */
   LanBreak(ident);
}
