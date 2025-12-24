/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU_EMI.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fonctions de test propres au module EMISSION
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Sources/simu_emi.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:10   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   May 16 1997 13:35:24   ANA
 * Test de la creation  et changement de fichier
 * avec emission en parallèle
 * 
 *    Rev 1.3   May 13 1997 17:12:12   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 * 
 *    Rev 1.2   Apr 15 1997 10:07:06   ANA
 * Correction bug sur le service FICHIER de TCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 13:53:34   ANA
 * Mise au point pour avoir deux taches distinctes :
 * emission et reception de messages vers le module
 * 
 *    Rev 1.0   Mar 21 1997 09:29:36   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
//#include <mem.h>
#include <string.h>
//#include <console.h>

/* module noyau */
#include <noyau.h>
//#include <tcp_ip.h>

//#include <tcp.h>
#include <csr_lan.h>

/* module EMISSION de FICHIER */

#include <fic_conf.h>
//#include <msg_fic.h>
#include <simu0.h>

#define LOC_DEF
#include <simu_glo.h>
#undef LOC_DEF


/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/


PRIVATE void GestionParDate( struct_emi_gestion_restit *p_msg);
PRIVATE void GestionParNumero( struct_emi_gestion_restit *p_msg);
PRIVATE BOOLEAN MenuAuth(void);
PRIVATE BOOLEAN MenuCold(void);
PRIVATE enum_emi_type MenuEtat(void);

 

/*--------------- VARIABLES: --------------*/

PRIVATE char tableau[10];


/*--------------- CODE: -------------------*/



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void MenuRestitution(short int bal_ident)
* PARAMETRES:
*     entree: Boite aux lettres du module
*     retour: Rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: permet de choisir les types de messages pour le DOP
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int MenuRestitution(void)
{
   int type ;
   char fin = 0;

   //window( 35, 10, 75, 18);
   //cadre(40,8);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf("         SERVICE RESTITUTION \r");
   //gotoxy(2,4);
   cprintf (" \t 1 - Per date\r");
   //gotoxy(2,5);
   cprintf (" \t 2 - Per file number\r");
   //gotoxy(2,7);
   cprintf ("    Choix => ");

   while ( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         type = getche();
         type = atoi((char *)&type);
         if( type == RESTIT_DATE || type == RESTIT_NUMERO )
            break;
      }
   }
   return(type);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SimuEnvoiEmi( short int bal_dest, short int service,
*                           short int type_message, void *p_message)
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi des messages vers le module test‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SimuEnvoie(noyau_bal_id bal_dest,noyau_bal_id bal_src, struct_emi_message *p_msg)
{

   struct_emi_message  *p_message = NULL;

   ExitAlloue((struct_neutre **)(&p_message), sizeof(struct_emi_message),POOL_SIMU);
      
   memcpy(p_message, p_msg, sizeof(struct_emi_message));
   
   ExitEnvoie(bal_dest, bal_src, (struct_neutre *)(p_message));
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void GestionParNumero(  )
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Choix des messages a transmettre
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void GestionParNumero( struct_emi_gestion_restit *p_msg)
{
   /* nouvelle fenetre */
  // //window( 3, 14, 80, 25);
   cadre(80,11);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf(" NUMERO DU PREMIER FICHIER A RESTITUER :  ");
   SaisieChaine(&tableau[0]);
   p_msg->t.numero.debut = atol(tableau);
   //gotoxy(2,3);
   cprintf(" NUMERO DU DERNIER FICHIER A RESTITUER :  ");
   SaisieChaine(&tableau[0]);
   p_msg->t.numero.fin = atol(tableau);
   //gotoxy(2,4);
   cprintf(" AUTOMATIQUE/MAUNEL(a/m) :   ");
   SaisieChaine(&tableau[0]);
   if ((strcmp(tableau, "a") == 0) || (strcmp(tableau, "A") == 0))
	   p_msg->mode = EMI_RESTIT_AUTOMATIQUE;
   else
	   p_msg->mode = EMI_RESTIT_MANUAL;
   //gotoxy(2,5);
   cprintf(" LOCAL/DISTANT (l/d):   ");
   SaisieChaine(&tableau[0]);
   if ((strcmp(tableau, "d") == 0) || (strcmp(tableau, "D") == 0))
 	   p_msg->action = RESTIT_FIC;
  else
	   p_msg->action = RESTIT_DISQUE;

   //clrscr();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void GestionParDate(  )
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Choix des messages a transmettre
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void GestionParDate( struct_emi_gestion_restit *p_msg)
{
   /* nouvelle fenetre */
   //window( 3, 14, 80, 25);
   //cadre(80,11);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf(" Premier jour a restituer(AAAAMMJJ) :  ");
   SaisieChaine(p_msg->t.date.debut);

   //gotoxy(2,3);
   cprintf(" Dernier jour a restituer (AAAAMMJJ):  ");
   SaisieChaine(p_msg->t.date.fin);
   //gotoxy(2,4);
   cprintf(" AUTOMATIQUE/MAUNEL(a/m) :   ");
   SaisieChaine(&tableau[0]);
   if ((strcmp(tableau, "a") == 0) || (strcmp(tableau, "A") == 0))
	   p_msg->mode = EMI_RESTIT_AUTOMATIQUE;
   else
	   p_msg->mode = EMI_RESTIT_MANUAL;

   //gotoxy(2,5);
   cprintf(" LOCAL/DISTANT (l/d):   ");
   SaisieChaine(&tableau[0]);
   if ((strcmp(tableau, "d") == 0) || (strcmp(tableau, "D") == 0))
	   p_msg->action = RESTIT_FIC;
   else
	   p_msg->action = RESTIT_DISQUE;

   //clrscr();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SendRestitution(  )
* PARAMETRES: 
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Retrieves parameters and sends request for backup 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SendRestitution(void)
{
	struct_emi_message        msg;
	struct_emi_message  *p_message = NULL;

	p_message = &msg;
	
	//Request for restitution
	
	msg.entete.service = M_EMI_RESTITUTION;
	msg.entete.type_message = EMI_DEMANDE;
	
	
	msg.u.msg_restit.type = MenuRestitution();

	if (msg.u.msg_restit.type == RESTIT_DATE)
		GestionParDate(&(msg.u.msg_restit));
	else 
		GestionParNumero(&(msg.u.msg_restit));


	SimuEnvoie(SIMU.bal_emi, SIMU.bal_simu, p_message);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SendEtat(  )
* PARAMETRES: 
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Requests etat service 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SendEtat(void)
{
	//struct_emi_message msg;
	struct_emi_message  *p_msg;

	ExitAlloue((struct_neutre **)(&p_msg), sizeof(struct_emi_message),POOL_SIMU);

	p_msg->entete.service = M_EMI_ETAT;
	
	p_msg->entete.type_message = MenuEtat();

    ExitEnvoie(SIMU.bal_emi, SIMU.bal_simu, (struct_neutre *)(p_msg));
}


PRIVATE enum_emi_type MenuEtat(void)
{
	int type;
	char fin = 0;
	
   //window( 1, 10, 75, 25);
   cadre(40,15);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf("         SERVICE ETAT \r");
   //gotoxy(2,4);
   cprintf (" \t 1 - Start\r");
   //gotoxy(2,5);
   cprintf (" \t 2 - Request\r");
   //gotoxy(2,6);
   cprintf (" \t 3 - Stop\r");
   //gotoxy(2,8);
   cprintf ("    Choix => ");

   while ( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         type = getche();
         type = atoi((char *)&type);
		 if ((type == 1) || (type == 2) || (type == 3))
			 break;

	  }
   }

	switch(type)
	{
		case 1:
			return(EMI_DEBUT);
		case 2:
			return(EMI_DEMANDE);
		case 3:
			return(EMI_FIN);
		default:
				return(-1);
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SendEtat(  )
* PARAMETRES: 
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Sends authorization 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SendConfig(void)
{
	struct_emi_message msg;

	msg.entete.service = M_EMI_CONFIG;
	
	msg.entete.type_message = EMI_DEBUT;

	msg.u.msg_config_data.authorization = MenuAuth();
	msg.u.msg_config_data.cold_start = MenuCold();
	msg.u.msg_config_data.lane_number = 12;
	msg.u.msg_config_data.plaza_number = 7;

    SimuEnvoie(SIMU.bal_emi,SIMU.bal_simu, &msg);
}

PRIVATE BOOLEAN MenuAuth(void)
{
	int type;	
	char fin = 0;
   
   //window( 1, 10, 75, 25);
   cadre(40,15);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf("         AUTHORIZATION \r");
   //gotoxy(2,4);
   cprintf (" \t 1 - True\r");
   //gotoxy(2,5);
   cprintf (" \t 2 - False\r");
   //gotoxy(2,8);
   cprintf ("    Choix => ");

   while ( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         type = getche();
         type = atoi((char *)&type);
		 if ((type == 1) || (type == 2))
			 break;

	  }
   }

	switch(type)
	{
		case 1:
			return(TRUE);
		case 2:
			return(FALSE);
		default:
			return(FALSE);
	}
}

PRIVATE BOOLEAN MenuCold(void)
{
	int type;	
	char fin = 0;
   
   //window( 1, 10, 75, 25);
   cadre(40,15);

   //textcolor(YELLOW);
   //gotoxy(2,2);
   cprintf("         COLD START ? \r");
   //gotoxy(2,4);
   cprintf (" \t 1 - True\r");
   //gotoxy(2,5);
   cprintf (" \t 2 - False\r");
   //gotoxy(2,8);
   cprintf ("    Choix => ");

   while ( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         type = getche();
         type = atoi((char *)&type);
		 if ((type == 1) || (type == 2))
			 break;

	  }
   }

	switch(type)
	{
		case 1:
			return(TRUE);
		case 2:
			return(FALSE);
		default:
			return(FALSE);
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SendArret(  )
* PARAMETRES: 
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: requests stop from EMI 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SendArret(void)
{
	struct_emi_message        msg;
	
	msg.entete.service = M_EMI_ARRET;
	msg.entete.type_message = EMI_DEMANDE;

    SimuEnvoie(SIMU.bal_emi,SIMU.bal_simu, &msg);
}

PROTECTED void NotifyChange(void)
{
	boolean fin = FALSE;
		
   //window( 35, 10, 75, 18);
   cadre(40,8);
	
   //gotoxy(2,2);
   cprintf("New file %08d", SIMU.num_fichier);
   while( fin == FALSE)
   {
      if( kbhit())
      {
         /* R‚cup‚ration de la touche clavier */
         switch (getch())
         {
            case CR :
               fin = TRUE;
            break;

			default:
				break;
         }
      }
   }

}