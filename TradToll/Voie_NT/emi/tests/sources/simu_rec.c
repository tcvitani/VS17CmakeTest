/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU0.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Sources/simu_rec.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:10   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.6   May 27 1997 09:34:02   ANA
 *  
 * 
 *    Rev 1.5   May 16 1997 13:35:24   ANA
 * Test de la creation  et changement de fichier
 * avec emission en parallèle
 * 
 *    Rev 1.4   May 13 1997 17:12:14   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 * 
 *    Rev 1.3   Apr 15 1997 10:07:08   ANA
 * Correction bug sur le service FICHIER de TCP/IP
 * 
 *    Rev 1.2   Apr 07 1997 16:00:12   ANA
 *  
 * 
 *    Rev 1.1   Apr 07 1997 13:53:36   ANA
 * Mise au point pour avoir deux taches distinctes :
 * emission et reception de messages vers le module
 * 
 *    Rev 1.0   Mar 21 1997 09:29:38   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
//#include <mem.h>
#include <conio.h>
#include <string.h>
//#include <dir.h>

/* module noyau */
#include <noyau.h>
//#include <tcp_ip.h>
//#include <console.h>

//#include <tcp.h>
#include <csr_lan.h>
//#include "emi_fic.h"

//#include <fic_conf.h>
#include <fic.h>
//#include <msg_fic.h>

#include <simu0.h>
#include <simu_glo.h>

#include <memclass.h>
/*------------------DEFINES:--------------------------*/


/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/
PROTECTED void AfficheAcquittement( char *, enum_emi_type type);
PROTECTED void AfficheMessageRecu( enum_emi_service , enum_emi_type message);
//PRIVATE void HandleLan(struct_neutre *p_neutre);

/*------------------VARIABLES:------------------------*/

PRIVATE char *etat[13] = {"HS",
                         "OK",
                         "Backup inactif",
                         "Backup en cours",
                         "Purge en cours",
                         "Backup Termine complet",
                         "Backup Termine incomplet",
                         "Purge terminee",
                         "backup : Erreur sur Numero ",
                         "backup : Erreur sur Date ",
                         "Backup: pas de fichier",
                         ""};

PRIVATE char *services[5] = { "ARRET",
							 "ETAT",
							 "RESTITUTION",
							 "PURGE",
							 "CONFIG"
							};
//PRIVATE long num_transfert = 0L;
PRIVATE boolean temoin_arret;

/*-------------------CODE:----------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void simu_recoit()
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED DWORD WINAPI SimuRecoit(void *param)
{
   struct_neutre   *p_neutre;
   noyau_enum_retour code_rtc;
   char bal_name[MAX_PATH];


   /* Changement de la priorite de la tache  */
    DebutRegion();

   ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;

   FinRegion();

	/* traitement */
	while ( 1 )
	{
		DebutRegion();
		
		/* On surveille un message dans une des BALs */
		code_rtc = NOYAU_BAL_MESS;
		do
		{
			code_rtc = TestRecoit(SIMU.bal_simu,&p_neutre);
			if (code_rtc != NOYAU_BAL_MESS)
			{

#ifndef VRAI_HORODATE
					code_rtc = TestRecoit(SIMU.bal_hrd,&p_neutre);
					if (code_rtc != NOYAU_BAL_MESS)
						strcpy (bal_name, "(void)");
					else
						strcpy (bal_name, "BAL HRD (alarme)");
#else
						strcpy (bal_name, "(void)");
#endif
			
			}
			else
				strcpy (bal_name, "BAL SIMU (main)");
			
			if( code_rtc == NOYAU_BAL_MESS )
			{
				/* fenetre de reception des messages */
				//window( 5, 17, 75, 22);
				//cadre(70,6);
				//textcolor(YELLOW);
				//gotoxy(2,2);
				cprintf(" RECEPTION DANS BAL '%s' de tache %d\n", bal_name ,p_neutre->bl_retour ) ;
				AfficheReception(p_neutre);
				
				
				/* On libŠre la m‚moire */
				if( Libere (&p_neutre) != NOYAU_OK )
					cprintf("SIMU : PB Libere() \n");
				
			}
		}
		while (code_rtc == NOYAU_BAL_MESS);
		
		if( temoin_arret )
		{
			SIMU.flag_fin = TRUE;
			Termine();
		}
		
		FinRegion();
		
		DelaiTache(10);
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AfficheReceptiont(struct_neutre *p_neutre)
* PARAMETRES: Pointeur sur la structure neutre du message recu
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche les messages recus du module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AfficheReception( struct_neutre *p_neutre)
{
   struct_emi_message *p_simu_message = (struct_emi_message *)(0);
   struct_lan_message *p_fic_message = (struct_lan_message *)(0);
   boolean            ok = FALSE;
   boolean         close = FALSE;
   //struct_emi_message  msg;

   
   if( p_neutre->bl_id == SIMU.bal_simu)
   {
        //gotoxy(4,4);
        ok = TRUE;

		p_simu_message = (struct_emi_message *)p_neutre;	

		switch(p_simu_message->entete.type_message)
		{
		case EMI_DEBUT_ACQ:
			cprintf("ACQ start : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_DEBUT_NACQ:
			cprintf("NACQ start : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_FIN_ACQ:
			cprintf("ACQ end : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_FIN_NACQ:
			cprintf("NACQ end : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_DEMANDE_ACQ:
			cprintf("ACQ request : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_DEMANDE_NACQ:
			cprintf("NACQ request : %s", services[p_simu_message->entete.service]);
			break;
		case EMI_NOUVEL_ETAT:
			//gotoxy(4,4);
			cprintf("ETAT liaison : %s \n\r",etat[p_simu_message->u.msg_etat.liaison]);
			//gotoxy(4,5);
			cprintf(" %s \n\r",etat[p_simu_message->u.msg_etat.backup]);
			break;
		default:
			cprintf("----------------?!!");
		}
   }
/*  else 
	   if (p_neutre->bl_id == SIMU.bal_lan)
	   {
			//gotoxy(4,4);
			ok = TRUE;			

		   p_fic_message = (struct_lan_message *)p_neutre;

			switch( p_fic_message->entete.type_message)
			{
			case LAN_DEBUT :
				//gotoxy(4,4);
				cprintf(" Debut de transfert FIC OK\n\r");
     			HandleLan(p_neutre);
				break;
			case LAN_FIN :
				//gotoxy(4,5);
				cprintf("Fin de transfert FIC \n\r");
     			HandleLan(p_neutre);
			break;
			
			case LAN_EMISSION_RENOMME:
				//gotoxy(4,5);
				cprintf("File transfer : %s", ((struct_lan_message *)p_neutre)->u.param_fic.path);
     			HandleLan(p_neutre);
				break;

			default :
				cprintf("Service FICHIER: message inconnu \n\r");
				break;
			}
	   }*/

		
   if( ! ok)
   {
      //gotoxy(4,4);
      cprintf("----- ????\n");
   }
   else
  	   /* Attente de la touche espace avant de poursuivre */
	   while (getch () != (char)' ');
}


/*PRIVATE void HandleLan(struct_neutre *p_neutre)
{
   struct_lan_message *p_fic_message = NULL;
   struct_lan_message *p_lan_send = NULL;

   p_fic_message = (struct_lan_message *)p_neutre;
 
   switch (p_fic_message->entete.type_message)
   {
   case LAN_DEBUT:
		ExitAlloue((struct_neutre **)(&p_lan_send), sizeof(struct_lan_message),POOL_SIMU);
		p_lan_send->entete.service = p_fic_message->entete.service;
		p_lan_send->entete.type_message = LAN_DEBUT_ACQ;
		ExitEnvoie(SIMU.bal_emi, SIMU.bal_lan, (struct_neutre *)(p_lan_send));
	   break;
   case LAN_FIN:
	   ExitAlloue((struct_neutre **)(&p_lan_send), sizeof(struct_lan_message),POOL_SIMU);
	   p_lan_send->entete.service = p_fic_message->entete.service;
	   p_lan_send->entete.type_message = LAN_FIN_ACQ;
	   ExitEnvoie(SIMU.bal_emi, SIMU.bal_lan, (struct_neutre *)(p_lan_send));
	   break;
   case LAN_EMISSION:
	   break;
   case LAN_EMISSION_RENOMME:
	   ExitAlloue((struct_neutre **)(&p_lan_send), sizeof(struct_lan_message),POOL_SIMU);
	   p_lan_send->entete.service = p_fic_message->entete.service;
		if (SIMU.lan_ok)
		{
			if (!FIC_copy(p_fic_message->u.param_fic.path, p_fic_message->u.param_fic.fichier, FIC_COPY_WITH_DATE))	
				p_lan_send->entete.type_message = LAN_EMISSION_RENOMME_NON_EFFECTUEE;
			else
				p_lan_send->entete.type_message = LAN_EMISSION_RENOMME_EFFECTUEE;
		}
		else
			p_lan_send->entete.type_message = LAN_EMISSION_RENOMME_NON_EFFECTUEE;
       strcpy(p_lan_send->u.param_fic.fichier, p_fic_message->u.param_fic.fichier);
	   strcpy(p_lan_send->u.param_fic.hostname, p_fic_message->u.param_fic.hostname);	
       strcpy(p_lan_send->u.param_fic.path, p_fic_message->u.param_fic.path);
	   ExitEnvoie(SIMU.bal_emi, SIMU.bal_lan, (struct_neutre *)(p_lan_send));
	   break;
   default:
	   cprintf("Request error");
	   break;
   }
}*/