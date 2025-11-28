/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  
* FICHIER: TFT_TH.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: 
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* STRUCTURE DE DONNEES:
* --------------------------------------------------------------------
* NOTA:
* --------------------------------------------------------------------
* NOTA:
* --------------------------------------------------------------------
* NOTA 2:
* --------------------------------------------------------------------
* PEMM:
* --------------------------------------------------------------------
* HISTORIQUE:
*
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include <csrlc32.h>
#include "noyau.h"
#include <debug.h>
#include <run.h>
#include <reg.h>

#include "err.h"
#include "fic.h"
#include "str.h"


#include <tft_modular_interface.h>
#include <tft_glob.h>

#include <csr_files_tools.h>


/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- METHODES: ---------------*/
PRIVATE	void ReceptionMess(struct_tft_message *p_tft);


PROTECTED DWORD WINAPI RfrTft(void * pvFoo)
{
   struct_neutre       *p_neutre = NULL;
   noyau_enum_retour   code_rtc;

	
	ChangePriorite( TacheCourante(), RFR_TFT.priorite_max );

	/* attente et traitement des messages */
	while ( TRUE )
	{
       code_rtc = Recoit( RFR_TFT.rfr_tft_bal_id, &p_neutre, NOYAU_ATTENTE_INFINIE);

		DebutRegion();

       /* Messages issus de l'application */
		if( code_rtc == NOYAU_BAL_MESS )
		{
			ReceptionMess((struct_tft_message *)p_neutre);

			ExitLibere( &p_neutre );

		}

		FinRegion();
    }
}

PROTECTED void EnvoiCsrTft(enum_tft_type_services type)
{
	struct_tft_message *p_mess;

	if ((type == TFT_NEW_FILE) && (strcmp(RFR_TFT.NewFiles, "") == 0))
		return;
	
	RfrTftFichierTrace("RFR_TFT : EnvoiCsrTft ");
	ExitAlloue((struct_neutre **)&p_mess, sizeof(struct_tft_message), RFR_TFT.pool);

	p_mess->entete.service = M_TFT_FILE;
	p_mess->entete.type = type;
	
	if (type == TFT_NEW_FILE)
    {
		strcpy_s(p_mess->u.new_file.NewFiles, sizeof(p_mess->u.new_file.NewFiles), RFR_TFT.NewFiles);
	    RfrTftFichierTrace("TFT_NEW_FILE=%s",RFR_TFT.NewFiles);
    }
	
	ExitEnvoie(RFR_TFT.csr_tft_bal_id, RFR_TFT.rfr_tft_bal_id, (struct_neutre *)p_mess);
}

PRIVATE	void ReceptionMess(struct_tft_message *p_tft)
{
	switch(p_tft->entete.service)
	{
	case M_TFT_FILE:
		
		switch(p_tft->entete.type)
		{
		case TFT_DEBUT_ACK:
			RfrTftFichierTrace("RFR_TFT <- CSR_TFT : TFT_DEBUT_ACK");
			break;
		case TFT_DEBUT_NACK:
			RfrTftFichierDebug("RFR_TFT <- CSR_TFT : TFT_DEBUT_NACK");
			break;
		case TFT_FIN_ACK:
			RfrTftFichierTrace("RFR_TFT <- CSR_TFT : TFT_FIN_ACK");
			break;
		case TFT_FIN_NACK:
			RfrTftFichierDebug("RFR_TFT <- CSR_TFT : TFT_FIN_NACK");
			break;
		default:
			RfrTftFichierDebug("RFR_TFT <- CSR_TFT : Unknown message");
			break;
		}

		break;
	default:
		RfrTftFichierDebug("RFR_TFT <- CSR_TFT : Unknown message");
		break;
	}
}

