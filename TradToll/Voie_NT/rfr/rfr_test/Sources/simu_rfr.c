
/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU_RFR.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fonctions de test propres au module REFERENCE
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Referenc/test/Sources/simu_rfr.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:56   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   19 Mar 1998 14:35:04   DPI
 *  
 * 
 *    Rev 1.3   Jun 04 1997 19:11:42   DPI
 *  
 *
 *    Rev 1.2   Jun 03 1997 17:38:40   DPI
 *  
 * 
 *    Rev 1.1   Apr 28 1997 16:09:22   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:44:56   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

/* module noyau */
#include "noyau.h"
#include "tcp_ip.h"
#include "csr_lan.h"
#include "console.h"

#include "err.h"
#include "str.h"
#include "fic.h"
#include "fic_gere.h"

/* module REFERENCE */
#include "referenc.h"

#define LOC_DEF
#include ".\simu_glo.h"
#undef LOC_DEF

#include ".\simu0.h"
#include <csr_lan.h>

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/
PROTECTED void EnvoiRfr (struct_rfr_message *p_msg);

/*--------------- VARIABLES: --------------*/

//PROTECTED char tableau[128];


/*--------------- CODE: -------------------*/



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES:
*     entree: Boite aux lettres du module
*     retour: Rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Handles sending reference message from COM_LS to RFR module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SendReference( void)
{
   struct_rfr_message  *p_message;
   struct_rfr_message	msg;
   noyau_bal_id bal_src = SIMU.bal_msg;
   char  *chaine;
   window( 1, 1, 80, 25);
   cadre(80,13);


   p_message = &msg;
   
   p_message->entete.service = M_RFR_REFERENCE;
   p_message->entete.type_message = RFR_RECEPTION_REFERENCE;

   textcolor(YELLOW);
   gotoxy(2,2);
   cprintf("         MESSAGE REFERENCE \r");
   gotoxy(2,4);
   cprintf (" \t Reference file = ");

   SaisieChaine(p_message->u.message_ref.fichier, 50, 26, 4);
   chaine = p_message->u.message_ref.fichier;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     bal_src = -1;

   gotoxy(2,5);
   cprintf (" \t Reference = ");
   
   SaisieChaine(p_message->u.message_ref.reference, 50, 26, 5);
   chaine = p_message->u.message_ref.reference;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     bal_src = -1;

   gotoxy(2,7);
   cprintf("\t Acknowledge LAN_COPY?(O/N) = ");
   SaisieChaine(Lan_Copy, 2, 55, 7);
   chaine = Lan_Copy;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     strcpy(Lan_Copy,"N\0");

   if (bal_src != -1)
   	   EnvoiRfr(p_message);
}

PROTECTED void EnvoiRfr (struct_rfr_message *p_message)
{

	struct_rfr_message *p_msg = NULL;

	/* memory allocation for the message */
	ExitAlloue ((struct_neutre **)(&p_msg),sizeof(struct_rfr_message),
		POOL_SIMU);

	if (p_message->entete.service == M_RFR_MANUEL) 
	{
		strcpy(p_msg->u.message_manuel.fichier,p_message->u.message_manuel.fichier);
		strcpy(p_msg->u.message_manuel.id, p_message->u.message_manuel.id);
	}
	else
	{
		strcpy(p_msg->u.message_ref.fichier,p_message->u.message_ref.fichier);
		strcpy(p_msg->u.message_ref.reference, p_message->u.message_ref.reference);
	}
	
	p_msg->entete.service = p_message->entete.service;
	p_msg->entete.type_message = p_message->entete.type_message;
	
	/* send message to mailbox */
	ExitEnvoie (SIMU.bal_rfr,SIMU.bal_msg,(struct_neutre *)p_msg);
	
}


PROTECTED void SendManual( void)
{
   struct_rfr_message  *p_message;
   struct_rfr_message	msg;
   noyau_bal_id bal_src = SIMU.bal_msg;
   char  *chaine;
   window( 1, 1, 80, 25);
   cadre(80,13);


   p_message = &msg;
   
   p_message->entete.service = M_RFR_MANUEL;
   p_message->entete.type_message = RFR_FICHIER_MANUEL;

   textcolor(YELLOW);
   gotoxy(2,2);
   cprintf("         MESSAGE MANUEL \r");
   gotoxy(2,4);
   cprintf (" \t Manual file = ");

   SaisieChaine(p_message->u.message_manuel.fichier, 50, 26, 4);
   chaine = p_message->u.message_ref.fichier;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     bal_src = -1;

   gotoxy(2,5);
   cprintf (" \t File id = ");
   
   SaisieChaine(p_message->u.message_manuel.id, 50, 26, 5);
   chaine = p_message->u.message_ref.reference;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     bal_src = -1;

   gotoxy(2,7);
   cprintf("\t Acknowledge LAN_COPY?(O/N) = ");
   SaisieChaine(Lan_Copy, 2, 55, 7);
   chaine = Lan_Copy;
   if( ( (chaine[0] == 'q') || (chaine[0] == 'Q')) && (chaine[1] == '\0'))
     bal_src = -1;

   if (bal_src != -1)
   	   EnvoiRfr(p_message);
}


PROTECTED void ProcessMessages( void)
{
   char  chaine1[250];
   char  chaine2[250];
   struct_rfr_message  rfr_msg;

  
   window( 1, 1, 80, 25);
   cadre(80,13);
   textcolor(YELLOW);
   
   gotoxy(7,5);
   cprintf (" \t Reference file 1 = ");
   SaisieChaine(chaine1, 50, 26, 7);

   gotoxy(7,9);
   cprintf (" \t Reference file 2 = ");
   SaisieChaine(chaine2, 50, 26, 11);

   
   rfr_msg.entete.service = M_RFR_REFERENCE;
   rfr_msg.entete.type_message = RFR_RECEPTION_REFERENCE;

   strcpy(rfr_msg.u.message_ref.fichier, chaine1);
   strcpy(rfr_msg.u.message_ref.reference, "1");
   EnvoiRfr(&rfr_msg);

   strcpy(rfr_msg.u.message_ref.fichier, chaine2);
   strcpy(rfr_msg.u.message_ref.reference, "2");
   EnvoiRfr(&rfr_msg);
   

}


PROTECTED void ProcessStop( void)
{
   char  chaine[6];
   struct_lan_message  *p_lan_msg = NULL;
   struct_rfr_message  *p_rfr_msg = NULL;

   window( 1, 1, 80, 25);
   cadre(80,13);

   gotoxy(7,3);
   cprintf("\t LAN START?(O/N) = ");
   SaisieChaine(chaine, 3, 30, 6);
   if (strcmp(chaine, "N") != 0)
   {
	    ExitAlloue ((struct_neutre **)(&p_lan_msg),sizeof(struct_lan_message),
		POOL_SIMU);

	    p_lan_msg->entete.service = M_LAN_FICHIER;
	
    	/* send message to mailbox */
	   gotoxy(7,5);	      
	   cprintf("\t Ack lan start?(O/N) = ");
       SaisieChaine(Lan_Start, 30, 5, 6);
	   Lan_Start[strlen(Lan_Start)] = '\0';
	   if (strcmp(Lan_Start,"N") != 0)
	   {
	   	   p_lan_msg->entete.type_message = LAN_DEBUT_ACQ;
		   Lan_Started = TRUE;
	   }
	   else
	   	   p_lan_msg->entete.type_message = LAN_DEBUT_NACQ;

       ExitEnvoie (SIMU.bal_rfr,SIMU.bal_lan,(struct_neutre *)p_lan_msg);
   }
   else
   {
	  gotoxy(7,5);
	  cprintf("\t LAN END?(O/N) = ");
	  SaisieChaine(chaine, 30, 5, 6);
      if (strcmp(chaine, "N") != 0)
	  {

	    ExitAlloue ((struct_neutre **)(&p_lan_msg),sizeof(struct_lan_message),
		POOL_SIMU);

	    p_lan_msg->entete.service = M_LAN_FICHIER;		
		
		gotoxy(7,7);	      
		cprintf("\t Ack lan end?(O/N) = ");
		SaisieChaine(Lan_End, 30, 7, 6);
		Lan_End[strlen(Lan_End)] = '\0';
        if (strcmp(Lan_End,"N") != 0)
	   	   p_lan_msg->entete.type_message = LAN_FIN_ACQ;
	    else
	   	   p_lan_msg->entete.type_message = LAN_FIN_NACQ;

        ExitEnvoie (SIMU.bal_rfr,SIMU.bal_lan,(struct_neutre *)p_lan_msg);
	  }
	  else
	  {
		gotoxy(7,7);
		cprintf("\t RFR STOP?(O/N) = ");
		SaisieChaine(chaine, 30, 7, 6);
        if (strcmp(chaine,"N") != 0)
		{
		    ExitAlloue ((struct_neutre **)(&p_rfr_msg),sizeof(struct_rfr_message),
		                POOL_SIMU);

			p_rfr_msg->entete.service = M_RFR_ARRET;
			p_rfr_msg->entete.type_message = RFR_DEMANDE;

            ExitEnvoie (SIMU.bal_rfr,SIMU.bal_msg,(struct_neutre *)p_rfr_msg);
		}

	  }
   }

}

