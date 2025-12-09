#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <DmvGlobal.h>
#include <AffMessage.h>
#include <AffService.h>
#include <TlmMessage.h>
#include <TlmService.h>

#define LOC_DEF
#include<DmvAni.h>
#undef LOC_DEF
#include "InstTracer.h"
#include <memclass.h>

PRIVATE dmv_inst_id InitAni(PVOID param);
PRIVATE void ReceptionService(dmv_inst_id inst, struct_neutre *p_neutre);

PROTECTED DWORD WINAPI Ani(IN PVOID param)
{
	struct_neutre		*p_neutre;
	noyau_enum_retour	code_rtc;
	dmv_inst_id			inst_id;

	inst_id = InitAni(param);

	if (inst_id >= INSTANCE_MAX)
		ExitBad();

	ChangePriorite(TacheCourante(), gsDMV[inst_id].priority_max);

	while (TRUE)
	{
		code_rtc = Recoit(gsDMV[inst_id].ani_mbox, (struct_neutre **)(&p_neutre), NOYAU_ATTENTE_INFINIE);

		DebutRegionLocale();

		if (code_rtc == NOYAU_BAL_MESS)
		{
			if (p_neutre->bl_retour == gsDMV[inst_id].ios_mbox)
				ReceptionIos(inst_id, p_neutre);
			else
				ReceptionService(inst_id, p_neutre);

			ExitLibere((struct_neutre **)(&p_neutre));
		}

		FinRegionLocale();
	}
	return 0;
}

PRIVATE dmv_inst_id InitAni(PVOID param)
{
	dmv_inst_id	inst_id;

	inst_id = (dmv_inst_id)param;

	gsDMV[inst_id].ani_mbox = PublieBAL(gsDMV[inst_id].ani_mbox_name, 0);
	if (gsDMV[inst_id].ani_mbox <= 0)
		ExitBad();

	gsDMV[inst_id].ios_mbox = AttendBAL(gsDMV[inst_id].ios_mbox_name);
	if (gsDMV[inst_id].ios_mbox <= 0)
		ExitBad();

	memset(&gsDMV[inst_id].sStatus, 0, sizeof(int));

	return(inst_id);
}

PROTECTED void ReceptionIos(dmv_inst_id inst_id, struct_neutre *p_neutre)
{
	struct_ani_ios		*p_ios = (struct_ani_ios *)(0);
	struct_tlm_message  *pMessage = (struct_tlm_message*)(0);
	struct_b_etat_aff	*p_etat;
	int					i;
	DWORD dwSize;

	p_ios = (struct_ani_ios *)(p_neutre);
	pMessage = (struct_tlm_message*)(p_neutre);

	p_etat = &p_ios->u.ios_ani.sStatus;

	switch (p_ios->message_id)
	{
	case M_ACK_AFFICHAGE:
		MInstTracer_FichierTrace(inst_id, "DMV_ANI: ReceptionIOS() => Affichage Termine");
		break;

	case M_ERR_AFFICHAGE:
		MInstTracer_FichierTrace(inst_id, "DMV_ANI: ReceptionIOS() => Error affichage ");
		break;

	case M_CHANGEMENT_ETAT:
		MInstTracer_FichierTrace(inst_id, "DMV_ANI: ReceptionIOS() => Display status : Link Failure = %d", p_etat->aff_hs);

		memcpy(&gsDMV[inst_id].sStatus, p_etat, sizeof(struct_b_etat_aff));
		
		AFFSendStatus(inst_id, 0, SRV_TYP_NOUVEL_ETAT);

		if (p_etat->aff_hs)
		{
			gsDMV[inst_id].timeout_mbox = AFF_TIMEOUT_INFINITE;
		}
		else
		{
			if (SrvEstVide(gsDMV[inst_id].service[M_AFF_AFFICHAGE]))
				gsDMV[inst_id].timeout_mbox = AFF_TIMEOUT_SLOW;
			else
				gsDMV[inst_id].timeout_mbox = AFF_TIMEOUT_FAST;
		}
		break;

	case M_CHANGEMENT_VISU:
		MInstTracer_FichierTrace(inst_id, "DMV_ANI: ReceptionIOS() => Chgt VISU");
		dwSize = sizeof(gsDMV[inst_id].szCurrentLineTable[0]);
		for (i = 0; i<AFF_MAX_LIGNES; i++)
		{
			if (&p_ios->u.ios_ani.current_view[i] != NULL)
			{
				strncpy_s(gsDMV[inst_id].szCurrentLineTable[i], sizeof(gsDMV[inst_id].szCurrentLineTable[i]), p_ios->u.ios_ani.current_view[i], dwSize);
				gsDMV[inst_id].szCurrentLineTable[i][dwSize - 1] = '\0';
			}
		}

		AFFSendView(inst_id, 0, AFF_TYP_AFFICHAGE_COURANT);
		break;

	case TLM_MESSAGE_STATE_RECEIVED_ACQ:
		MInstTracer_FichierTrace(inst_id, "TLM_ANI: ReceptionIos() => Received new state: SIGN_ID='%d'", pMessage->srv.srv_tlm.u.display_srv.state);
		TLMSendCmdState(inst_id, &pMessage->srv.srv_tlm, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_ACQ);
		break;

	case TLM_MESSAGE_STATE_RECEIVED_NACQ:
		MInstTracer_FichierTrace(inst_id, "TLM_ANI: ReceptionIos() => Command failed to execute: SIGN_ID='%d'", pMessage->srv.srv_tlm.u.display_srv.state);
		TLMSendCmdState(inst_id, &pMessage->srv.srv_tlm, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_NACQ);
		break;

		//TODO : ETAT, ESPION, ARRET ==> common functions
	case M_ARRET:
	case TLM_MESSAGE_ETAT_CHANGED:
		MInstTracer_FichierTrace(inst_id, "DMV_ANI: ReceptionIOS() => ARRET effectu‚ ");
		//AFFGestionArretAni(inst_id);
		break;

	default:
		MInstTrtacer_FichierError(inst_id, "DMV_ANI  ***** ReceptionIOS() => msg_id '%d' inconnu *****",
								  p_ios->message_id);
		break;
	}
}

PRIVATE void ReceptionService(dmv_inst_id inst_id, struct_neutre *p_neutre)
{
	struct_message_generic  *p_recv_msg = (struct_message_generic *)(0);
	struct_neutre *p_neutre_copy = p_neutre;

	p_recv_msg = (struct_message_generic *)p_neutre_copy;

	switch (p_recv_msg->entete.service)
	{
	case M_AFF_AFFICHAGE:
		AFFProcessServiceMsg(inst_id, (struct_aff_message *)p_neutre);
		break;

	case M_AFF_VISU:
		AFFProcessViewMsg(inst_id, (struct_aff_message *)p_neutre);
		break;

	case M_TLM_MESSENGER_SERVICE:
		TLMProcessServiceMsg(inst_id, (struct_tlm_message *)p_neutre);
		break;

	case M_TLM_FLASHER_SERVICE:
		TLMProcessFlasherMsg(inst_id, (struct_tlm_message *)p_neutre);
		break;

	case M_SRV_ETAT:
		AFFProcessStatusMsg(inst_id, (struct_aff_message *)p_neutre);
		break;

	case M_SRV_ESPION:
		AFFProcessSpyMsg(inst_id, (struct_aff_message *)p_neutre);
		break;

	case M_SRV_ARRET:
		AFFProcessStopMsg(inst_id, (struct_aff_message *)p_neutre);
		break;

	default:
		if (p_recv_msg->entete.service >= BUTEE_AFF_SERVICE)
		{
			AFFSendAcknowledgement(inst_id, p_recv_msg->entete.neutre.bl_retour,
								   p_recv_msg->entete.service, SRV_TYP_SERVICE_INCONNU);

			TLMSendAcknowledgement(inst_id, p_recv_msg->entete.neutre.bl_retour,
								   p_recv_msg->entete.service, SRV_TYP_SERVICE_INCONNU);

			MInstTrtacer_FichierError(inst_id, "DMV_ANI ***** ReceptionService() => service_id '%d' unknown *****",
									  p_recv_msg->entete.service);
		}
		else
		{
			AFFSendAcknowledgement(inst_id, p_recv_msg->entete.neutre.bl_retour,
								   p_recv_msg->entete.service, SRV_TYP_SERVICE_NON_GERE);

			TLMSendAcknowledgement(inst_id, p_recv_msg->entete.neutre.bl_retour,
								   p_recv_msg->entete.service, SRV_TYP_SERVICE_NON_GERE);

			MInstTrtacer_FichierError(inst_id, "DMV_ANI ***** ReceptionService() => service_id '%d' not managed *****",
									  p_recv_msg->entete.service);
		}
		break;
	}
}