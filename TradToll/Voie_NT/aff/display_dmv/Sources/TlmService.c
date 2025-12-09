#include <noyau.h>
#include <debug.h>
#include <module.h>

#include <InstTracer.h>
#include <DmvGlobal.h>
#include <DmvAni.h>
#include <TlmMessage.h>
#define LOC_DEF
#include <TlmService.h>
#undef LOC_DEF

#include <memclass.h>

PROTECTED void TLMProcessServiceMsg(dmv_inst_id siInstId, struct_tlm_message *pMsg)
{

	switch (pMsg->entete.type_message)
	{
	case SRV_TYP_DEBUT:
	{
		enum_srv_type start_result;

		MInstTracer_FichierTrace(siInstId, "TLMProcessServiceMsg() - message SRV_TYP_DEBUT");

		// Check user list
		if (SrvEstVide(gsDMV[siInstId].service[M_TLM_MESSENGER_SERVICE]))
		{
			start_result = TLMStartService(siInstId, M_TLM_MESSENGER_SERVICE, pMsg->entete.neutre.bl_retour);
		}
		else
		{
			MInstTrtacer_FichierError(siInstId, "TLM_ANI: Bal '%d', Debut service return SRV_TYP_DEBUT_NACQ", pMsg->entete.neutre.bl_retour);
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEBUT_NACQ);
			return;
		}

	}
	break;

	case SRV_TYP_FIN:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', Fin service M_TLM_MESSENGER_SERVICE", pMsg->entete.neutre.bl_retour);
		TLMEndService(siInstId, M_TLM_MESSENGER_SERVICE, pMsg->entete.neutre.bl_retour);
	}
	break;

	case SRV_TYP_SET: // set status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', SRV_TYP_SET M_TLM_MESSENGER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsDMV[siInstId].service[M_TLM_MESSENGER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			MInstTracer_FichierTrace(siInstId, "TLMProcessServiceMsg() - user NOK");
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_NACQ);
		}
		else
		{
			MInstTracer_FichierTrace(siInstId, "TLMProcessServiceMsg() - user OK");

			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_SET_ACQ);
			TLMSendMessageToIos(siInstId, TLM_MESSAGE_SET_REQUESTED, pMsg);
		}
	}
	break;

	case SRV_TYP_DEMANDE: // get status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', DEMANDE M_TLM_MESSENGER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsDMV[siInstId].service[M_TLM_MESSENGER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEMANDE_NACQ);
		}
		else
		{
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_MESSENGER_SERVICE, SRV_TYP_DEMANDE_ACQ);
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

PROTECTED void TLMProcessFlasherMsg(dmv_inst_id siInstId, struct_tlm_message *pMsg)
{

	switch (pMsg->entete.type_message)
	{
	case SRV_TYP_DEBUT:
	{
		enum_srv_type start_result;

		MInstTracer_FichierTrace(siInstId, "TLMProcessFlasherMsg() - message SRV_TYP_DEBUT");

		// Check user list
		if (SrvEstVide(gsDMV[siInstId].service[M_TLM_FLASHER_SERVICE]))
		{
			start_result = TLMStartService(siInstId, M_TLM_FLASHER_SERVICE, pMsg->entete.neutre.bl_retour);
		}
		else
		{
			MInstTrtacer_FichierError(siInstId, "TLM_ANI: Bal '%d', Debut service return SRV_TYP_DEBUT_NACQ", pMsg->entete.neutre.bl_retour);
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_FLASHER_SERVICE, SRV_TYP_DEBUT_NACQ);
			return;
		}

	}
	break;

	case SRV_TYP_FIN:
	{
		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', Fin service M_TLM_FLASHER_SERVICE", pMsg->entete.neutre.bl_retour);
		TLMEndService(siInstId, M_TLM_FLASHER_SERVICE, pMsg->entete.neutre.bl_retour);
	}
	break;

	case SRV_TYP_SET: // set status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', SRV_TYP_SET M_TLM_FLASHER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsDMV[siInstId].service[M_TLM_FLASHER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			MInstTracer_FichierTrace(siInstId, "TLMProcessServiceMsg() - user NOK");
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_FLASHER_SERVICE, SRV_TYP_SET_NACQ);
		}
		else
		{
			MInstTracer_FichierTrace(siInstId, "TLMProcessServiceMsg() - user OK");

			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_FLASHER_SERVICE, SRV_TYP_SET_ACQ);
			TLMSendMessageToIos(siInstId, TLM_MESSAGE_FLASHER_SET_REQUESTED, pMsg);
		}
	}
	break;

	case SRV_TYP_DEMANDE: // get status
	{
		BOOL user_subscribed;

		MInstTracer_FichierTrace(siInstId, "TLM_ANI: Bal '%d', DEMANDE M_TLM_FLASHER_SERVICE status", pMsg->entete.neutre.bl_retour);

		user_subscribed = SrvEstDemandeur(gsDMV[siInstId].service[M_TLM_FLASHER_SERVICE], pMsg->entete.neutre.bl_retour);

		if (user_subscribed == FALSE)
		{
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_FLASHER_SERVICE, SRV_TYP_DEMANDE_NACQ);
		}
		else
		{
			TLMSendMessage(siInstId, pMsg->entete.neutre.bl_retour, M_TLM_FLASHER_SERVICE, SRV_TYP_DEMANDE_ACQ);
		}

	}
	break;
	default:
	{
		MInstTrtacer_FichierError(siInstId, "TLM_ANI: Service M_TLM_FLASHER_SERVICE => type '%d' unknown", pMsg->entete.type_message);
	}
	break;
	}
}