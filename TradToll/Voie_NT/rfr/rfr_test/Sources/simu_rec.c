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
 * $Log:   T:/MODULO/VoieNt/Referenc/test/Sources/simu_rec.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:56   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jun 04 1997 19:11:42   DPI
 *  
 * 
 *    Rev 1.1   Jun 03 1997 17:38:40   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:44:54   DPI
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <direct.h>

/* module noyau */
#include <noyau.h>
//#include <tcp_ip.h>
//#include <console.h>
#include <fic.h>

#include <csr_lan.h>
#include <referenc.h>
#include <rfr_appli.h>

#include ".\simu0.h"

#include ".\simu_glo.h"

#include <memclass.h>
/*------------------DEFINES:--------------------------*/


/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/
PROTECTED void AfficheAcquittement( char *, enum_rfr_type type);
PROTECTED void AfficheMessageRecu( enum_rfr_service , enum_rfr_type message);
PRIVATE void HandleLANMsg(struct_lan_message *p_lan);

/*------------------VARIABLES:------------------------*/


/*PRIVATE char *etat[8] = {"liaison HS ",
                           "liaison OK ",
                           "En cours",
                           "Termine",
                           "Erreur sur Numero ",
                           "Erreru sur Date ",
                           "Refuse",
                           ""};
                           */
PRIVATE boolean            temoin_arret;

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

PROTECTED DWORD WINAPI SimuRecoit (LPVOID p)
{
	struct_neutre   *p_neutre;
	noyau_enum_retour code_rtc;
	char bal_name[MAX_PATH];
	
	DebutRegion();
	
	/* Changement de la priorite de la tache  */
	ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;
	
	FinRegion();
	
	/* traitement */
	while ( TRUE )
	{
		DebutRegion();
		
		/* On surveille un message dans une des BALs */
		code_rtc = NOYAU_BAL_MESS;
		do
		{
			code_rtc = TestRecoit(SIMU.bal_simu,&p_neutre);
			if (code_rtc != NOYAU_BAL_MESS)
			{
				code_rtc = TestRecoit(SIMU.bal_lan,&p_neutre);
				if (code_rtc != NOYAU_BAL_MESS)
				{
					code_rtc = TestRecoit(SIMU.bal_msg,&p_neutre);
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
						strcpy (bal_name, "BAL COM (message)");
				}
				else
				{
					strcpy (bal_name, "BAL LAN (fichier)");
				}
				
			}
			else
				strcpy (bal_name, "BAL SIMU (main)");
			
			if( code_rtc == NOYAU_BAL_MESS )
			{
				/* fenetre de reception des messages */
				//window( 5, 17, 75, 22);
				//cadre(70,6);
				//textcolor(YELLOW);
				////gotoxy(2,2);
				cprintf("\n RECEPTION DANS BAL '%s' de tache %d\n", bal_name ,p_neutre->bl_retour ) ;
				AfficheReception(p_neutre);
				
				/* On libŠre la m‚moire */
				if( Libere (&p_neutre) != NOYAU_OK )
					cprintf("\nSIMU : PB Libere() \n");
				
			}
		}
		while (code_rtc == NOYAU_BAL_MESS);
		
		if( temoin_arret )
		{
			SIMU.flag_fin = TRUE;
			Termine();
		}
		
		FinRegion();
		
		DelaiTache(5);
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
PROTECTED void AfficheReception(struct_neutre *p_neutre)
{
   
   char      *fileid[36]	 =
   {"TCI", "TFT", "CUR", "CAL", "PRD", "AUT", "BAS", "OCM", "OTL", "CFG", "AF1", "AF2", "CIPCAL", "CIPCLS", "CIPHOL", "CIPOCM",
   "CIPOTL", "CIPPLZ", "CIPPRD", "CIPWHT", "CLS", "COMID", "COMNAME", "DPA", "DSG", "EPT", "ETCW", "ETL", "EXT", "FREE", "ITL",
   "REC", "TCL", "TFT_MODULAR", "VRN", "VSW"};
   struct_rfr_message *p_rfr_message = NULL;
   struct_lan_message *p_fic_message = NULL;
   struct_rfr_appli_msg *p_rfr_appli_msg = NULL;
   boolean            ok = FALSE;

   
   if( p_neutre->bl_id == SIMU.bal_simu)
   {
        ////gotoxy(4,4);
        ok = TRUE;
		
		p_rfr_appli_msg = ( struct_rfr_appli_msg *) p_neutre;
		cprintf("\n New %s file", fileid[p_rfr_appli_msg->u.file.file_id]);
   }
   else
   
   if( p_neutre->bl_id == SIMU.bal_msg)
   {
      p_rfr_message = ( struct_rfr_message *) p_neutre;
	  if (p_rfr_message->entete.service == M_RFR_ARRET)
      {
	    ////gotoxy(4,4);
		cprintf("\n--------------> RFR ARRET");
		ok = TRUE;
	  }
	  else		
        if( p_rfr_message->entete.service == M_RFR_REFERENCE)
		{
           if( p_rfr_message->entete.type_message == RFR_EMISSION_UPDATE)
		   {
                ////gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->LS : ID '%s', REF '%s', update '%d'\n",
					              p_rfr_message->u.message_update.id,
						          p_rfr_message->u.message_update.reference,
							      p_rfr_message->u.message_update.update);
			}
			else
			{
				//gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->LS ID = ???\n");
			}
		}
		else
		if( p_rfr_message->entete.service == M_RFR_MANUEL)
		{
			if( p_rfr_message->entete.type_message == RFR_FICHIER_MANUEL_ACK)
			{
				//gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->APP manuel : ACK ID '%s', FIC '%s'\n",
				               p_rfr_message->u.message_manuel.id,
					              p_rfr_message->u.message_manuel.fichier);
			}
			else
			if( p_rfr_message->entete.type_message == RFR_FICHIER_MANUEL_NACK)
			{
				//gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->APP manuel : NACK ID '%s', FIC '%s'\n",
			                      p_rfr_message->u.message_manuel.id,
				                  p_rfr_message->u.message_manuel.fichier);
			}
			else
			{
				//gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->LS ID = ???\n");
			}
		}
		else
		{
			//gotoxy(4,4);
			ok = TRUE;
			cprintf("\n----->LS ???\n");
		}
	}
	else if( p_neutre->bl_id == SIMU.bal_lan)
	{
		  p_fic_message = ( struct_lan_message *) p_neutre;
	
		  if( p_fic_message->entete.service == M_LAN_FICHIER)
		  {
			if( p_fic_message->entete.type_message == LAN_RECEPTION)
			{
				 //gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->FIC : '%s' => '%s'\n",
					              p_fic_message->u.param_fic.fichier,
						          p_fic_message->u.param_fic.path);

				HandleLANMsg(p_fic_message);

			}
			else if( p_fic_message->entete.type_message == LAN_DEBUT)
			{
				//gotoxy(4,4);
	            ok = TRUE;
		        cprintf("\n----->FIC : demande du service fichier \n");
			 }
			else
			{
				//gotoxy(4,4);
				ok = TRUE;
				cprintf("\n----->FIC ???\n");
			}
		}
	}
	else if( p_neutre->bl_id == SIMU.bal_hrd)
	{
	}


   if( ! ok)
   {
      //gotoxy(4,4);
      cprintf("\n----- ????\n");
   }
   //else
	  // /* Attente de la touche espace avant de poursuivre */
	  // while (getch () != (char)' ');
}

PRIVATE void HandleLANMsg(struct_lan_message *p_lan)
{
	struct_lan_message *p_lan_send = NULL;
	enum_lan_type	lan_type;
	char    FileName[MAX_PATH + 1];
	char    FileExt[MAX_PATH + 1];
	char    FileDrv[MAX_PATH + 1];
	char    FileDir[MAX_PATH + 1];
	char    DestPath[MAX_PATH + 1];


	if(!Lan_Started)
	{
		printf("\n\nLan service not started\n");
		return;

	}
    
	Lan_Copy[strlen(Lan_Copy)] = '\0';
	
    if (stricmp(Lan_Copy, "N\0") != 0)
	{

	    FIC_splitpath(p_lan->u.param_fic.fichier, FileDrv, FileDir, FileName, FileExt);

        sprintf(DestPath,"%s%s%s%s",p_lan->u.param_fic.path,"\\",FileName,FileExt);

		if (!FIC_copy(DestPath, p_lan->u.param_fic.fichier, FIC_COPY_WITH_DATE))
		{
			printf("Error copying file [%s] to [%s]", p_lan->u.param_fic.fichier, DestPath);
			lan_type = LAN_RECEPTION_NON_EFFECTUEE;	  
		}
		else
			lan_type = LAN_RECEPTION_EFFECTUEE;
	}
	else
		lan_type = LAN_RECEPTION_NON_EFFECTUEE;	  
	 

	 ExitAlloue ((struct_neutre **)(&p_lan_send),sizeof(struct_lan_message),
		POOL_SIMU);

	 p_lan_send->entete.service = M_LAN_FICHIER;
	 p_lan_send->entete.type_message = lan_type;
	 strcpy(p_lan_send->u.param_fic.fichier, p_lan->u.param_fic.fichier);
	 strcpy(p_lan_send->u.param_fic.path, p_lan->u.param_fic.path);
	
	/* send message to mailbox */
	ExitEnvoie (SIMU.bal_rfr,SIMU.bal_lan,(struct_neutre *)p_lan_send);
}

