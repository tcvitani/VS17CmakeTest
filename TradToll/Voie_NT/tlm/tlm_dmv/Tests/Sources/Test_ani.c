#include <windows.h>
#include <resource.h>

#include <noyau.h>

#include <test.h>
#include <Test_ani.h>
#include <memclass.h>

// Test is there message in MailBox
void SimuRecoit(void)
{
	HWND hnDlg;
	short int longueur;
	short int code_rtc;
	struct_tlm_message *p_simu_message;
	struct_neutre *p_neutre;
	struct_tlm_status *b_status;
	char temp[400] = {0}, lb_item[400], result[256] = {0};

	SIMU.fin = FALSE;
	hnDlg = SIMU.hDlg;

	ChangePriorite(TacheCourante(), THREAD_PRIORITY_NORMAL);

	while (SIMU.fin == FALSE)
	{
		Sleep(10);
		memset(lb_item, 0, sizeof(lb_item));

		code_rtc = TestRecoit(SIMU.test_bal, (struct_neutre **)&p_neutre);

		DebutRegionLocale();
		if (code_rtc == NOYAU_BAL_MESS)
		{
			memset(temp, 0, sizeof(temp));
			p_simu_message = (struct_tlm_message *)(p_neutre);
			switch (p_simu_message->entete.service)
			{
			// Messages from ARRET service
			case M_SRV_ARRET:
				switch (p_simu_message->entete.type_message)
				{
				case SRV_TYP_ARRET_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ARRET : SRV_TYP_ARRET_ACQ ");
					break;

				case SRV_TYP_ARRET_EFFECTUE:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ARRET : SRV_TYP_ARRET_EFFECTUE ");
					break;

				default:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ARRET : Response unknown ");
					break;
				}
				break;
				// End of processing ARRET service

			// Messages from ETAT service
			case M_SRV_ETAT:
				switch (p_simu_message->entete.type_message)
				{
				case SRV_TYP_DEBUT_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_DEBUT_ACQ ");
					break;

				case SRV_TYP_DEBUT_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_DEBUT_NACQ ");
					break;

				case SRV_TYP_FIN_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_FIN_ACQ ");
					break;

				case SRV_TYP_FIN_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_FIN_NACQ ");
					break;

				case SRV_TYP_DEMANDE_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_DEMANDE_ACQ ");
					break;

				case SRV_TYP_DEMANDE_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_DEMANDE_NACQ ");
					break;

				case SRV_TYP_NOUVEL_ETAT:
					b_status = (struct_tlm_status *)&p_simu_message->srv.srv_etat.status;
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : SRV_TYP_NOUVEL_ETAT : ");

					if (b_status->link_failure)
						strcat_s(lb_item, sizeof(lb_item), "   *** Link NOK ***");
					else
						strcat_s(lb_item, sizeof(lb_item), "   *** Link OK ***");
					break;

				default:
					strcat_s(lb_item, sizeof(lb_item), "M_SRV_ETAT : Response unknown");
					break;
				}
				break;
			// End of processing ETAT service

			// Messages from VDAC trigger service
			case M_TLM_MESSENGER_SERVICE:
				switch (p_simu_message->entete.type_message)
				{
				case SRV_TYP_DEBUT_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_DEBUT_ACQ ");
					break;
				case SRV_TYP_DEBUT_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_DEBUT_NACQ ");
					break;
				case SRV_TYP_FIN_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_FIN_ACQ ");
					break;
				case SRV_TYP_FIN_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_FIN_NACQ ");
					break;
				case SRV_TYP_SET_ACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_SET_ACQ ");
					break;
				case SRV_TYP_SET_NACQ:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE : SRV_TYP_SET_NACQ ");
					break;

				case SRV_TYP_TLM_STATE_RECEIVED:
					strcat_s(lb_item, sizeof(lb_item), "SRV_TYP_TLM_STATE_RECEIVED:");
					sprintf_s(lb_item, sizeof(lb_item), "State received %d", p_simu_message->srv.srv_tlm.u.display_srv.state);

					break;
				default:
					strcat_s(lb_item, sizeof(lb_item), "M_TLM_MESSENGER_SERVICE: Unknown message");
					break;
				}
				break;

				// Messages from ESPION service
				case M_SRV_ESPION:
					switch (p_simu_message->entete.type_message)
					{
					case SRV_TYP_DEBUT_ACQ:
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : SRV_TYP_DEBUT_ACQ ");
						break;

					case SRV_TYP_DEBUT_NACQ:
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : SRV_TYP_DEBUT_NACQ ");
						break;

					case SRV_TYP_FIN_ACQ:
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : SRV_TYP_FIN_ACQ ");
						break;

					case SRV_TYP_FIN_NACQ:
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : SRV_TYP_FIN_NACQ ");
						break;

					case SRV_TYP_MESSAGE_ESPION:
						longueur = p_simu_message->srv.espion.taille;
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : SRV_TYP_MESSAGE_ESPION => ");
						if (p_simu_message->srv.espion.sens == SRV_ESPION_SORTANT)
							strcat_s(lb_item, sizeof(lb_item), "Write( ");
						else
							strcat_s(lb_item, sizeof(lb_item), "Read( ");

						strcat_s(lb_item, sizeof(lb_item), p_simu_message->srv.espion.donnee);

						strcat_s(lb_item, sizeof(lb_item), ")");
						break;

					default:
						strcat_s(lb_item, sizeof(lb_item), "M_SRV_ESPION : Response unknown");
						break;
					}
					break;

				default:
					strcpy_s(result, sizeof(result), "Reception : Service unknown ");
					break;

			} //switch-case
			ExitLibere((struct_neutre **)&(p_neutre));
		} //if
		FinRegionLocale();
		if (strlen(lb_item) != 0)
			AddLBItem(hnDlg, lb_item);
	} //while
	Termine();
}