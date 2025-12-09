#include <noyau.h>
#include <debug.h>
#include <module.h>

#include <InstTracer.h>
#include <TlmDmvGlobal.h>
#include <TlmDmvAni.h>
#include <TlmDmvUtil.h>
#define LOC_DEF
#include <TlmDmvServ.h>
#undef LOC_DEF

#include <memclass.h>

PROTECTED void Ani_ProcessTlmMsg(tlm_inst_id siInstId, struct_tlm_message *pMsg)
{

	switch (pMsg->entete.type_message)
	{
	case SRV_TYP_DEBUT:
	{
		enum_srv_type start_result;

		MInstTracer_FichierTrace(siInstId, "Ani_ProcessTlmMsg() - message SRV_TYP_DEBUT");

		// Check user list
		if (SrvEstVide(gsTLM[siInstId].service[M_TLM_MESSENGER_SERVICE]))
		{
			start_result = DebutService(siInstId, M_SRV_USER, pMsg->entete.neutre.bl_retour);
		}
		else
		{
			MInstTrtacer_FichierError(siInstId, "TLM_ANI: Bal '%d', Debut service return SRV_TYP_DEBUT_NACQ", pMsg->entete.neutre.bl_retour);
			TlmUtil_SendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEBUT_NACQ);
			return;
		}

	}
	break;

	case SRV_TYP_FIN:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', Fin service M_TLM_MESSENGER_SERVICE", pMsg->entete.neutre.bl_retour);
		FinService(siInstId, M_TLM_MESSENGER_SERVICE, pMsg->entete.neutre.bl_retour);
	}
	break;

	case SRV_TYP_SET: // set status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', SRV_TYP_SET M_TLM_MESSENGER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsTLM[siInstId].service[M_TLM_MESSENGER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			MInstTracer_FichierTrace(siInstId, "Ani_ProcessTlmMsg() - user NOK");
			TlmUtil_SendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_NACQ);
		}
		else
		{
			MInstTracer_FichierTrace(siInstId, "Ani_ProcessTlmMsg() - user OK");

			TlmUtil_SendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_ACQ);
			TlmUtil_SendMessageToIos(siInstId, TLM_MESSAGE_SET_REQUESTED, pMsg);
		}
	}
	break;

	case SRV_TYP_DEMANDE: // get status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', DEMANDE M_TLM_MESSENGER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsTLM[siInstId].service[M_TLM_MESSENGER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			TlmUtil_SendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEMANDE_NACQ);
		}
		else
		{
			TlmUtil_SendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEMANDE_ACQ);
		}

	}
	break;
	default:
	{
		MInstTrtacer_FichierError(siInstId, "TLM_ANI: Service M_TLM_MESSENGER_SERVICE => type '%d' unknown", pMsg->entete.type_message);
	}
	break;
	}
}


PROTECTED void Ani_ProcessArretMsg(tlm_inst_id siInstId,
								   struct_tlm_message *pMsg)
{
	switch (pMsg->entete.type_message)
	{
	case SRV_TYP_DEMANDE:
		gsTLM[siInstId].ulArretBalId = pMsg->entete.neutre.bl_retour;
		EnvoiAcquittement(siInstId, pMsg->entete.neutre.bl_retour, M_SRV_ARRET, SRV_TYP_ARRET_ACQ);
		// Sending ARRET message to the IOS thread
		TlmUtil_SendMessageToIos(siInstId, TLM_MESSAGE_STOP_REQUESTED, NULL);
		break;

	default:
		MInstTrtacer_FichierError(siInstId, "Pb message arret type inconnu %d de la bal %d", pMsg->entete.type_message, pMsg->entete.neutre.bl_retour);
		EnvoiAcquittement(siInstId, pMsg->entete.neutre.bl_retour, M_SRV_ARRET, SRV_TYP_SERVICE_INCONNU);
		break;
	}
}


PROTECTED void Ani_ProcessEtatMsg(IN tlm_inst_id inst_id,
								  IN struct_tlm_message *p_msg_recu)
{
	boolean		retour;

	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		MInstTracer_FichierTrace(inst_id, "TLM_SERV: Bal '%d', Debut service ETAT ", p_msg_recu->entete.neutre.bl_retour);
		retour = DebutService(inst_id, M_SRV_ETAT, p_msg_recu->entete.neutre.bl_retour);

		if (retour)
			EnvoiEtat(inst_id, (struct_srv_etat *)&gsTLM[inst_id].status);
		break;

	case SRV_TYP_FIN:
		MInstTracer_FichierTrace(inst_id, "TLM_SERV: Bal '%d', Fin service ETAT ", p_msg_recu->entete.neutre.bl_retour);
		FinService(inst_id, M_SRV_ETAT, p_msg_recu->entete.neutre.bl_retour);
		break;

	case SRV_TYP_DEMANDE:
		MInstTracer_FichierTrace(inst_id, "TLM_SERV: Bal '%d', Demande ETAT ", p_msg_recu->entete.neutre.bl_retour);
		if (SrvEstDemandeur(gsTLM[inst_id].service[M_SRV_ETAT], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			EnvoiAcquittement(inst_id, p_msg_recu->entete.neutre.bl_retour, M_SRV_ETAT, SRV_TYP_DEMANDE_ACQ);
			EnvoiEtat(inst_id, (struct_srv_etat *)&gsTLM[inst_id].status);
		}
		else
		{
			EnvoiAcquittement(inst_id, p_msg_recu->entete.neutre.bl_retour, M_SRV_ETAT, SRV_TYP_DEMANDE_NACQ);
		}
		break;

	default:
		MInstTrtacer_FichierError(inst_id, "TLM_SERV *** Service ETAT => type '%d' inconnu ***", p_msg_recu->entete.type_message);
		break;
	}
}


PROTECTED void Ani_ProcessEspionMsg(IN tlm_inst_id inst_id,
									IN struct_tlm_message * p_msg_recu)
{
	MInstTracer_FichierTrace(inst_id, "App -> Ani : M_SRV_ESPION message_type : %d", p_msg_recu->entete.type_message);

	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		DebutServiceEspion(inst_id, p_msg_recu->entete.neutre.bl_retour, p_msg_recu->srv.espion.sens, p_msg_recu->srv.espion.nature);
		break;

	case SRV_TYP_FIN:
		FinService(inst_id, M_SRV_ESPION, p_msg_recu->entete.neutre.bl_retour);
		break;

	default:
		EnvoiAcquittement(inst_id, p_msg_recu->entete.neutre.bl_retour, M_SRV_ESPION, SRV_TYP_SERVICE_INCONNU);
		MInstTrtacer_FichierError(inst_id, "Pb message dop type inconnu %d de la bal %d", p_msg_recu->entete.type_message, p_msg_recu->entete.neutre.bl_retour);
		break;
	}
}
