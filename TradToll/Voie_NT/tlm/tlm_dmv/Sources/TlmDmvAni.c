#include <noyau.h>
#include <debug.h>
#include <module.h>
#include <InstTracer.h>
#include <TlmDmvGlobal.h>
#include <TlmDmvUtil.h>
#include <TlmDmvServ.h>
#define LOC_DEF
#include <TlmDmvAni.h>
#undef LOC_DEF

#include <memclass.h>

PRIVATE tlm_inst_id InitAni(PVOID param);
PRIVATE void ReceptionService(IN tlm_inst_id inst_id, IN struct_neutre *p_neutre);
PRIVATE void ReceptionIos(tlm_inst_id inst_id, struct_neutre *p_neutre);
PRIVATE void StopAni(tlm_inst_id siInstId);


PROTECTED DWORD WINAPI Ani(IN PVOID param)
{
	struct_neutre		*p_neutre;
	noyau_enum_retour	code_rtc;
	noyau_bal_id		ani_mail_id;
	noyau_bal_id		ios_mail_id;
	tlm_inst_id			inst_id;

	inst_id = InitAni(param);

	if (inst_id >= TLM_INSTANCE_MAX)
		ExitBad();

	ani_mail_id = gsTLM[inst_id].ani_bal;
	ios_mail_id = gsTLM[inst_id].ios_bal;

	ChangePriorite(TacheCourante(), gsTLM[inst_id].priorite_max);

	while (TRUE)
	{
		code_rtc = Recoit(ani_mail_id, (struct_neutre **)(&p_neutre), NOYAU_ATTENTE_INFINIE);

		DebutRegionModule(inst_id);
		if (code_rtc == NOYAU_BAL_MESS)
		{
			if (p_neutre->bl_retour == ios_mail_id)
				ReceptionIos(inst_id, p_neutre);
			else
				ReceptionService(inst_id, p_neutre);

			ExitLibere((struct_neutre **)(&p_neutre));
		}

		FinRegionModule(inst_id);
	}

	return 0;
}


PRIVATE tlm_inst_id InitAni(PVOID param)
{
	tlm_inst_id	inst_id;

	inst_id = (tlm_inst_id)param;

	gsTLM[inst_id].ani_bal = PublieBAL(gsTLM[inst_id].nom_bal_ani, 0);
	if (gsTLM[inst_id].ani_bal <= 0)
		ExitBad();

	gsTLM[inst_id].ios_bal = AttendBAL(gsTLM[inst_id].nom_bal_ios);
	if (gsTLM[inst_id].ios_bal <= 0)
		ExitBad();

	memset(&gsTLM[inst_id].status, 0, sizeof(struct_tlm_status));

	return(inst_id);
}


PRIVATE void ReceptionService(IN tlm_inst_id inst_id, IN struct_neutre *p_neutre)
{
	struct_tlm_message  *p_recv_msg = (struct_tlm_message *)(0);

	p_recv_msg = (struct_tlm_message *)p_neutre;

	switch (p_recv_msg->entete.service)
	{
	case M_SRV_USER:
		MInstTracer_FichierTrace(inst_id, "ReceptionService() - service M_SRV_USER");
		Ani_ProcessTlmMsg(inst_id, p_recv_msg);
		break;

	case M_SRV_ARRET:
		MInstTracer_FichierTrace(inst_id, "ReceptionService() - service M_SRV_ARRET");
		Ani_ProcessArretMsg(inst_id, p_recv_msg);
		break;

	case M_SRV_ETAT:
		MInstTracer_FichierTrace(inst_id, "ReceptionService() - service M_SRV_ETAT");
		Ani_ProcessEtatMsg(inst_id, p_recv_msg);
		break;

	case M_SRV_ESPION:
		MInstTracer_FichierTrace(inst_id, "ReceptionService() - service M_SRV_ESPION");
		Ani_ProcessEspionMsg(inst_id, p_recv_msg);
		break;

	default:
		MInstTracer_FichierTrace(inst_id, "recept service inconnu %d de la bal %d", p_recv_msg->entete.service, p_recv_msg->entete.neutre.bl_retour);
		EnvoiAcquittement(inst_id, p_recv_msg->entete.neutre.bl_retour, p_recv_msg->entete.service, SRV_TYP_SERVICE_INCONNU);
		break;
	}
}


PRIVATE void ReceptionIos(tlm_inst_id siInstId, struct_neutre *pNeutre)
{
	struct_tlm_message *pMessage = (struct_tlm_message *)pNeutre;

	switch (pMessage->entete.type_message)
	{
	case TLM_MESSAGE_STOP_EXECUTED:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: ReceptionIos() => IOS stopped");
		StopAni(siInstId);
	}
	break;

	case TLM_MESSAGE_STATE_RECEIVED_ACQ:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: ReceptionIos() => Received new state: SIGN_ID='%d'", pMessage->srv.srv_tlm.u.display_srv.state);
		EnvoiCmdState(siInstId, &pMessage->srv.srv_tlm, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_ACQ);
	}
	break;

	case TLM_MESSAGE_STATE_RECEIVED_NACQ:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: ReceptionIos() => Command failed to execute: SIGN_ID='%d'", pMessage->srv.srv_tlm.u.display_srv.state);
		EnvoiCmdState(siInstId, &pMessage->srv.srv_tlm, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_NACQ);
	}
	break;

	case TLM_MESSAGE_ETAT_CHANGED:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: ReceptionIos() => Received new etat from IOS. [link=%d, state=%d]", gsTLM[siInstId].status.link_failure,
					 gsTLM[siInstId].status.state_error);

		memcpy(&gsTLM[siInstId].status, &pMessage->srv.srv_etat.status, sizeof(gsTLM[siInstId].status));
		EnvoiEtat(siInstId, (struct_srv_etat *)&gsTLM[siInstId].status);
	}
	break;

	default:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: ReceptionIos() => unknown message type '%d'", pMessage->entete.type_message);
	}
	break;
	}
}

PRIVATE void StopAni(tlm_inst_id inst_id)
{
	EnvoiAcquittement(inst_id, gsTLM[inst_id].ulArretBalId, M_SRV_ARRET, SRV_TYP_ARRET_EFFECTUE);
	FinRegionModule(inst_id);

	TlmUtil_FreeMailbox(gsTLM[inst_id].ani_bal);

	Termine();
}
