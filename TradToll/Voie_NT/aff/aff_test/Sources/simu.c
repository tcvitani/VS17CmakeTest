/*************************************************************************\
*
*  PROGRAM: Simu
*  PURPOSE: To test AFF module.
*  COMMENTS:
*
\*************************************************************************/
#include <windows.h>
#include <stdio.h>

#include <csrlc32.h>
#include <noyau.h>
#include <reg.h>

#define AFF_PROTOTYPES
#include <aff_ext.h>
#undef AFF_PROTOTYPES

#define LOC_DEF
#include "waff.h"
#undef LOC_DEF

#include <memclass.h>

void SimuRecoit(void);
void OpenMailBox();

/* Thread definition */
PRIVATE struct_tache t_simu[] =
{
    /* Activation, priorite, taille pile, point d'entree */
    { TRUE , THREAD_PRIORITY_NORMAL, 1024, (LPTHREAD_START_ROUTINE)(SimuRecoit), NULL, NULL, "SIMU" },
    { FALSE, 0                     , 0   , NULL                                , NULL, NULL, ""   }
	
};

/************************************************************************\
*
*  FUNCTION: LanceModule();
*
*  PURPOSE:  Start AFF module.
*
\************************************************************************/

int LanceModule(HWND hDlg, char *p_MbName)
{ 
	char                   szKey[ MAX_PATH + 1 ];
	enum_instance_result   rc;
	char                   temp[100];
	char				   BalName[100];

	sprintf_s(BalName, sizeof(BalName), "%s", p_MbName);

	sprintf_s(szKey, 
			sizeof(szKey),
			"%s%s%s%s%s%s",
			CSR_REG_KEYn_CSRBASE,
			CSR_REG_KEYn_LANE_BASE,
			CSR_REG_KEYn_CONFIG,
			MOD_REG_KEYn_MODULES,
			AFF_REG_KEYn_ModAFF,	
			BalName);

	//AFF_REG_KEYn_ModAFF,

	SIMU.hDLL = LoadLibrary(dll_name);

	if (SIMU.hDLL != NULL)
	{
		SIMU.LpFncAFFLance = (LPFNC_LANCE)GetProcAddress(SIMU.hDLL, FNC_AFF_LANCE);
		SIMU.LpFncAFFArret = (LPFNC_ARRET)GetProcAddress(SIMU.hDLL, FNC_AFF_ARRET);

		if ((!SIMU.LpFncAFFLance) || (!SIMU.LpFncAFFArret))
		{
			FreeLibrary(SIMU.hDLL);
			sprintf_s(temp, sizeof(temp), "Cannot find function \"%s\" or \"%s\" in \"%s\"", FNC_AFF_LANCE, FNC_AFF_ARRET, dll_name);
			MessageBox(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
			return 0;
		}
	}
	else
	{
		sprintf_s(temp, sizeof(temp), "Cannot load dll: \"%s\"", dll_name);
		MessageBox(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
		return 0;
	}

	//rc = SIMU.LpFncAFFLance(szKey, szInstName, &SIMU.aff_bal);
	rc = SIMU.LpFncAFFLance(szKey, BalName, &SIMU.aff_bal);
	if ( rc != INST_INIT_OK)
	{
		sprintf_s(temp, sizeof(temp), "Probleme AFFLance => %d \n ", rc);
		MessageBox(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
		return 0;
	}

	SIMU.aff_bal = AttendBAL( BalName );
	if ( SIMU.aff_bal <= 0 )
	{
		sprintf_s(temp, sizeof(temp), "Probleme AttendBAL(\"%s\")\n", "AFF_TEST");
		MessageBox(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
		return 0;
	}

	OpenMailBox();

	rc = LanceTache (t_simu);
	SIMU.hDlg=hDlg;
	if ( rc != NOYAU_OK)
	{
		SIMU.LpFncAFFArret(SIMU.aff_bal);
		FreeLibrary(SIMU.hDLL);
		MessageBox(NULL, "Probleme LanceTache()!", "ERROR", MB_OK | MB_ICONERROR);

		return 0;
	}

	return 1;
}

/************************************************************************\
*
*  FUNCTION: ArretModule();
*
*  PURPOSE:  Stop AFF module.
*
\************************************************************************/

int ArretModule(HWND hDlg)
{
	enum_instance_result   rc;
	char                   temp[50];

	// Module arret
	rc = SIMU.LpFncAFFArret(SIMU.aff_bal);
	if ( rc != INST_INIT_OK)
	{
		sprintf_s(temp, sizeof(temp), "Probleme AFFArret => %d ", rc);
		AddLBItem(hDlg, temp);
		return 0;
	}
	else
	{
		AddLBItem(hDlg, "Module arrete");
	}

	// Stop thread
	DebutRegion() ;
		SIMU.fin = TRUE;
	FinRegion();
	
	Sleep(100);

	rc=ArretTaches( t_simu );
	if ( rc != NOYAU_ARRET_TACHE_OK)
	{
		AddLBItem(hDlg, "Error stoping thread");
		return 0;
	}
	else
		AddLBItem(hDlg, "Simulateur arrete");

	SupprimeBAL ("SIMU_AFF_0");
	SupprimeBAL ("SIMU_AFF_1");
	SupprimeBAL ("SIMU_AFF_2");
	SupprimeBAL ("SIMU_AFF_3");

	FreeLibrary(SIMU.hDLL);
		
	return 1;
}

/************************************************************************\
*
*  FUNCTION: OpenMailBox();
*
*  PURPOSE:  Open MailBox of applicatin for communication with 
*            AFF module.
*
\************************************************************************/

void OpenMailBox()
{
	DebutRegion() ;
	SIMU.user[0].bal_id = PublieBAL ("SIMU_AFF_0", NOYAU_BAL_ILLIMITEE);
	SIMU.user[1].bal_id = PublieBAL ("SIMU_AFF_1", NOYAU_BAL_ILLIMITEE);
	SIMU.user[2].bal_id = PublieBAL ("SIMU_AFF_2", NOYAU_BAL_ILLIMITEE);
	SIMU.user[3].bal_id = PublieBAL ("SIMU_AFF_3", NOYAU_BAL_ILLIMITEE);

	SIMU.user[0].bal_id = AttendBAL ("SIMU_AFF_0");
	SIMU.user[1].bal_id = AttendBAL ("SIMU_AFF_1");
	SIMU.user[2].bal_id = AttendBAL ("SIMU_AFF_2");
	SIMU.user[3].bal_id = AttendBAL ("SIMU_AFF_3");

	strcpy_s(SIMU.user[0].bal_name, sizeof(SIMU.user[0].bal_name), "BAL_A");
	strcpy_s(SIMU.user[1].bal_name, sizeof(SIMU.user[1].bal_name), "BAL_B");
	strcpy_s(SIMU.user[2].bal_name, sizeof(SIMU.user[2].bal_name), "BAL_C");
	strcpy_s(SIMU.user[3].bal_name, sizeof(SIMU.user[3].bal_name), "BAL_D");

	SIMU.bal_id = SIMU.user[0].bal_id;
	
	FinRegion();
}

/************************************************************************\
*
*  FUNCTION: SimuRecoit();
*
*  PURPOSE:  Test is there message in MailBox. If message is presented
*            it will be printed.
*
\************************************************************************/

void SimuRecoit(void)
{
	HWND				hnDlg;
	short int           longueur;
	short int           code_rtc;
	struct_aff_message  *p_simu_message;
	struct_neutre       *p_neutre;
	int                 k, i;
	char                temp[50],lb_item[200];
	struct_b_etat_aff   *b_status;
	
	SIMU.fin=FALSE;
	hnDlg=SIMU.hDlg;

	DebutRegion() ;
	ChangePriorite( TacheCourante(), THREAD_PRIORITY_NORMAL );
	while (SIMU.fin==FALSE)
	{
		for( k=0; k<=3; k++ )
		{
			FinRegion();
			DelaiTache (3);
			DebutRegion() ;

			code_rtc = TestRecoit (SIMU.user[k].bal_id, (struct_neutre **)&p_neutre);
			if (code_rtc == NOYAU_BAL_MESS)
			{
				strcpy_s(lb_item, sizeof(lb_item),SIMU.user[k].bal_name);
				strcat_s(lb_item, sizeof(lb_item)," - ");

				p_simu_message = (struct_aff_message *)(p_neutre);

				switch(p_simu_message->entete.service)
				{
				// Messages from ARRET service
				case M_SRV_ARRET:
					switch (p_simu_message->entete.type_message)
					{
					case SRV_TYP_DEMANDE_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ARRET : SRV_TYP_DEMANDE_ACQ ");
						break;

					case SRV_TYP_DEMANDE_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ARRET : SRV_TYP_DEMANDE_NACQ ");
						break;

					case SRV_TYP_EFFECTUE:
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ARRET : SRV_TYP_EFFECTUE ");
						break;

					default :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ARRET : R‚ponse inconnue ");
						break;
					}
					break;
				// End of processing ARRET service

				// Messages from DEVICE INFO service
				case M_SRV_DEVICE_INFO:					
					switch (p_simu_message->entete.type_message)
					{						
						case SRV_TYP_DEBUT_ACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_DEBUT_ACQ ");
							break;

						case SRV_TYP_DEBUT_NACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_DEBUT_NACQ ");
							break;

						case SRV_TYP_FIN_ACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_FIN_ACQ ");
							break;

						case SRV_TYP_FIN_NACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_FIN_NACQ ");
							break;
						
						case SRV_TYP_GET_ACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_GET_ACQ ");
							break;

						case SRV_TYP_GET_NACQ :
							strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE: SRV_TYP_GET_NACQ ");
							break;

						case SRV_TYP_EFFECTUE:
							strcat_s(lb_item, sizeof(lb_item), "SRV_TYP_EFFECTUE");
							AddLBItem(hnDlg,lb_item);

							for (int iIdx = 0; iIdx < (int)p_simu_message->u.srv_device_info.nb_item; iIdx++)
							{
								sprintf_s(lb_item,
										sizeof(lb_item),
										"Name: %s, Data: %s, Status: %s",
										p_simu_message->u.srv_device_info.items[iIdx].name,
										p_simu_message->u.srv_device_info.items[iIdx].data,
										(p_simu_message->u.srv_device_info.items[iIdx].status == DEV_INFO_OK) ? "OK" : "NOK");
								AddLBItem(hnDlg,lb_item);
							}

							lb_item[0] = 0;
						break;
					}
					break;
				// End of processing DEVICE INFO service

				// Messages from AFFICHAGE service
				case M_AFF_AFFICHAGE:
					switch (p_simu_message->entete.type_message)
					{
					case SRV_TYP_DEBUT_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_DEBUT_ACQ ");
						break;

					case SRV_TYP_DEBUT_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_DEBUT_NACQ ");
						break;

					case SRV_TYP_FIN_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_FIN_ACQ ");
						break;

					case SRV_TYP_FIN_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_FIN_NACQ ");
						break;

					case SRV_TYP_DEMANDE_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_DEMANDE_ACQ ");
						break;

					case SRV_TYP_DEMANDE_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_DEMANDE_NACQ ");
						break;

					case SRV_TYP_TEST_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_TEST_ACQ ");
						break;

					case SRV_TYP_TEST_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_TEST_NACQ ");
						break;

					case SRV_TYP_EFFECTUE :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : SRV_TYP_EFFECTUE ");
						break;

					case AFF_TYP_ALLUME_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_ALLUME_ACQ ");
						break;
								
					case AFF_TYP_ALLUME_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_ALLUME_NACQ ");
						break;

					case AFF_TYP_RESET_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_RESET_ACQ ");
						break;
								
					case AFF_TYP_RESET_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_RESET_NACQ ");
						break;

					/**********************MSU*****************/

					case AFF_TYP_NEW_FILE_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_NEW_FILE_ACQ ");
						break;

					case AFF_TYP_NEW_FILE_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : AFF_TYP_NEW_FILE_NACQ ");
						break;

					/*******************************************/

					default :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_AFFICHAGE : R‚ponse inconnue ");
						break;
					}
					break;
				// End of processing AFFICHAGE service

				// Messages from ETAT service
				case M_SRV_ETAT:
					switch (p_simu_message->entete.type_message)
					{
					case SRV_TYP_DEBUT_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_DEBUT_ACQ ");
						break;

					case SRV_TYP_DEBUT_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_DEBUT_NACQ ");
						break;

					case SRV_TYP_FIN_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_FIN_ACQ ");
						break;

					case SRV_TYP_FIN_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_FIN_NACQ ");
						break;

					case SRV_TYP_DEMANDE_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_DEMANDE_ACQ ");
						break;

					case SRV_TYP_DEMANDE_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_DEMANDE_NACQ ");
						break;

					case SRV_TYP_NOUVEL_ETAT :
						b_status = (struct_b_etat_aff *)&p_simu_message->u.srv_etat.status;
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : SRV_TYP_NOUVEL_ETAT : ");
						if (b_status->aff_hs)
							sprintf_s(temp, sizeof(temp), "%s", " *** AFF HS ***");
						else
							sprintf_s(temp, sizeof(temp), "%s", " AFF OK");
				
						strcat_s(lb_item, sizeof(lb_item),temp);
						
						if (b_status->liaison_hs)
							sprintf_s(temp, sizeof(temp), "%s", "   *** Liaison AFF HS ***");
						else
							sprintf_s(temp, sizeof(temp), "%s", "   Liaison AFF OK");
						
						strcat_s(lb_item, sizeof(lb_item),temp);
						break;

					default :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ETAT : R‚ponse inconnue ");
						break;
					}
					break;
				// End of processing ETAT service

				// Messages from VISU service
				case M_AFF_VISU:
					switch (p_simu_message->entete.type_message)
					{
					case SRV_TYP_DEBUT_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_DEBUT_ACQ ");
						break;

					case SRV_TYP_DEBUT_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_DEBUT_NACQ ");
						break;

					case SRV_TYP_FIN_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_FIN_ACQ ");
						break;

					case SRV_TYP_FIN_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_FIN_NACQ ");
						break;

					case SRV_TYP_DEMANDE_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_DEMANDE_ACQ ");
						break;

					case SRV_TYP_DEMANDE_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : SRV_TYP_DEMANDE_NACQ ");
						break;

					case AFF_TYP_AFFICHAGE_COURANT :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : AFF_TYP_AFFICHAGE_COURANT");
						AddLBItem(hnDlg,lb_item);
						strcpy_s(lb_item, sizeof(lb_item),"                            Line1:");
						strcat_s(lb_item, sizeof(lb_item), p_simu_message->u.srv_visu.label[0]);
						AddLBItem(hnDlg,lb_item);
						strcpy_s(lb_item, sizeof(lb_item),"                            Line2:");
						strcat_s(lb_item, sizeof(lb_item), p_simu_message->u.srv_visu.label[1]);
						AddLBItem(hnDlg,lb_item);
						strcpy_s(lb_item, sizeof(lb_item),"                            Line3:");
						strcat_s(lb_item, sizeof(lb_item), p_simu_message->u.srv_visu.label[2]);
						break;

					default :
						strcat_s(lb_item, sizeof(lb_item),"M_AFF_VISU : R‚ponse inconnue ");
						break;
					}
					break;
				// End of processing VISU service

				// Messages from ESPION service 
				case M_SRV_ESPION:
					switch (p_simu_message->entete.type_message)
					{
 					case SRV_TYP_DEBUT_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : SRV_TYP_DEBUT_ACQ ");
						break;

					case SRV_TYP_DEBUT_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : SRV_TYP_DEBUT_NACQ ");
						break;

					case SRV_TYP_FIN_ACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : SRV_TYP_FIN_ACQ ");
						break;

					case SRV_TYP_FIN_NACQ :
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : SRV_TYP_FIN_NACQ ");
						break;

					case SRV_TYP_MESSAGE_ESPION:
						longueur = p_simu_message->u.srv_espion.taille;
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : SRV_TYP_MESSAGE_ESPION => ");
						for( i=0; i < longueur ; i++)
						{
							sprintf_s(temp, sizeof(temp), "%02X ", p_simu_message->u.srv_espion.donnee[i]);
							strcat_s(lb_item, sizeof(lb_item),temp);
						}
						break;

					default:
						strcat_s(lb_item, sizeof(lb_item),"M_SRV_ESPION : R‚ponse inconnue ");
						break;
					}
					break;
				// End of processing VISU service

				// Service unknown
				default:
					strcat_s(lb_item, sizeof(lb_item),"R‚ception : Service inconnu ");
					break;
				}
				AddLBItem(hnDlg,lb_item);

				Libere (&p_neutre);
			} 
		}
	}

	FinRegion();
	Termine();
} 

/************************************************************************\
*
*  FUNCTION: simu_envoi_aff();
*
*  PURPOSE:  Send message into AFF MailBox.
*
\************************************************************************/

void simu_envoi_aff (noyau_bal_id bal,short int service, short int type_message)
{
	struct_aff_message  *p_msg_emis;
	struct_aff_message  *p_message = &SIMU.message;

	ExitAlloue((struct_neutre **)(&p_msg_emis),
                sizeof(struct_aff_message),NOYAU_GetPoolId("SIMUAFF"));

	p_msg_emis->entete.service = service;
	p_msg_emis->entete.type_message = type_message;

    switch( service )
    {
    case M_AFF_AFFICHAGE:
		 memcpy( &p_msg_emis->u.srv_affichage,
				 &p_message->u.srv_affichage,
                 sizeof(p_message->u.srv_affichage));
		 memcpy( &p_msg_emis->u.srv_test,
				 &p_message->u.srv_test,
                 sizeof(p_message->u.srv_test));
         break;

    case M_SRV_ESPION:
         memcpy( &p_msg_emis->u.srv_espion,
                 &p_message->u.srv_espion,
                 sizeof(p_message->u.srv_espion));
         break;

    case M_SRV_ETAT:
    case M_SRV_ARRET:
	case M_AFF_VISU:
		break;	
	}

    ExitEnvoie(bal,SIMU.bal_id,(struct_neutre *)(p_msg_emis));
}