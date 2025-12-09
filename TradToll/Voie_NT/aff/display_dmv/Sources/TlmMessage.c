#include <noyau.h>
#include <debug.h>
#include <module.h>
#include <csr_srv.h>
#include <InstTracer.h>
#include <DmvGlobal.h>
#define LOC_DEF
#include <TlmMessage.h>
#undef LOC_DEF
/*--------------------------- RESERVED:  --------------------------*/
#include <memclass.h>
/*--------------------------- EXTERNALS: --------------------------*/
/*--------------------------- DEFINES:   --------------------------*/
/*--------------------------- TYPEDEFS:  --------------------------*/
/*--------------------------- FUNCTIONS: --------------------------*/
PRIVATE BOOL WINAPI EnvoiDemandeurEspion(PVOID Param,
										 noyau_bal_id BalDemandeur,
										 PVOID DataDuDemandeur);
PRIVATE BOOL WINAPI EnvoiDemandeur(PVOID Param,
								   noyau_bal_id BalDemandeur,
								   PVOID DataDuDemandeur);
/*--------------------------- VARIABLES: --------------------------*/
/*--------------------------- CODE: -------------------------------*/


//|//
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: This function deallocates memory in the mail box
//				occupied by the messages.
// NOTES:		-
// PARAMS:		mailbox_id	- Mailbox ID.
// RETURNS:		-
///////////////////////////////////////////////////////////////////////////////
PROTECTED void TLMFreeMailbox(noyau_bal_id mailbox_id)
{
	struct_neutre *header;

	while ((TestRecoit(mailbox_id, (struct_neutre**)(&header)) == NOYAU_BAL_MESS))
	{
		ExitLibere((struct_neutre**)(&header));
	}
}

//|//
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: The function sends the message to the destination mailbox.
// NOTES:		-
// PARAMS:		instance_id				- Instance ID.
//				destination_mailbox_id	- Destination mailbox.
//				service					- Target service.
//				message_type			- Message type.
// RETURNS:		-
///////////////////////////////////////////////////////////////////////////////
PROTECTED void TLMSendMessage(dmv_inst_id		instance_id,
								   noyau_bal_id		destination_mailbox_id,
								   enum_srv_service	service,
								   enum_tlm_type		message_type
								   )
{
	struct_tlm_message	*message = NULL;
	noyau_enum_retour	send_result;

	ExitAlloue((struct_neutre**)&(message), sizeof(struct_tlm_message), gsDMV[instance_id].pool);

	message->entete.neutre.bl_retour = gsDMV[instance_id].ani_mbox;
	message->entete.service = service;
	message->entete.type_message = message_type;

	MInstTracer_FichierTrace(instance_id, "TLM_UTIL: TLMSendMessage() => destination=%d, service=%d, message type=%d",
				 destination_mailbox_id, message->entete.service, message->entete.type_message);

	send_result = Envoie(destination_mailbox_id, gsDMV[instance_id].ani_mbox, (struct_neutre*)message);

	if (send_result != NOYAU_OK)
	{
		ExitLibere((struct_neutre**)&message);
	}
}

PROTECTED boolean TLMStartService(dmv_inst_id inst_id,
							   enum_srv_service service_id,
							   noyau_bal_id bal_demandeur)
{
	enum_srv_type ret = SRV_TYP_DEBUT_NACQ;

	MInstTracer_FichierTrace(inst_id, "DebutService() - message SSERVICE");

	if (SrvEstDemandeur(gsDMV[inst_id].service[service_id], bal_demandeur) == FALSE)
		if (SrvAjouteDemandeur(gsDMV[inst_id].service[service_id], bal_demandeur, 0L) != NULL)
			ret = SRV_TYP_DEBUT_ACQ;

	TLMSendAcknowledgement(inst_id, bal_demandeur, service_id, ret);

	if (ret == SRV_TYP_DEBUT_NACQ)
		return FALSE;

	return TRUE;
}

PROTECTED boolean TLMEndService(dmv_inst_id inst_id,
							 enum_srv_service service_id,
							 noyau_bal_id bal_demandeur)
{
	PVOID pSrvData;

	// recherche le demandeur de ce service
	pSrvData = SrvRechercheDemandeur(gsDMV[inst_id].service[service_id], bal_demandeur, NULL, NULL);
	if (pSrvData == NULL)
	{
		TLMSendAcknowledgement(inst_id, bal_demandeur, service_id, SRV_TYP_FIN_NACQ);
		return FALSE;
	}

	SrvEnleveDemandeur(gsDMV[inst_id].service[service_id], &pSrvData);
	TLMSendAcknowledgement(inst_id, bal_demandeur, service_id, SRV_TYP_FIN_ACQ);

	return TRUE;
}


PROTECTED boolean TLMStartSpyService(dmv_inst_id inst_id,
									 noyau_bal_id bal_demandeur,
									 enum_espion_sens sens,
									 enum_espion_nature nature)
{
	enum_srv_type ret;
	struct_srv_espion *pSrvData;
	ret = SRV_TYP_DEBUT_NACQ;

	if (SrvEstDemandeur(gsDMV[inst_id].service[M_SRV_ESPION], bal_demandeur) == FALSE)
	{
		pSrvData = SrvAjouteDemandeur(gsDMV[inst_id].service[M_SRV_ESPION], bal_demandeur, sizeof(struct_srv_espion));
		if (pSrvData != NULL)
		{
			pSrvData->sens = sens;
			pSrvData->nature = nature;
			ret = SRV_TYP_DEBUT_ACQ;
		}
	}

	TLMSendAcknowledgement(inst_id, bal_demandeur, M_SRV_ESPION, ret);

	if (ret == SRV_TYP_DEBUT_NACQ)
		return FALSE;

	return TRUE;
}

PROTECTED void TLMSendAcknowledgement(IN dmv_inst_id inst_id,
								 IN noyau_bal_id bal_dest,
								 IN enum_srv_service service_id,
								 IN enum_srv_type type_message)
{
	struct_tlm_message *p_msg_emis = NULL;

	ExitAlloue((struct_neutre **)&(p_msg_emis), sizeof(struct_tlm_message), gsDMV[inst_id].pool);

	p_msg_emis->entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
	p_msg_emis->entete.service = service_id;
	p_msg_emis->entete.type_message = type_message;

	MInstTracer_FichierTrace(inst_id, "Ani -> App : service_%d, message_%d", service_id, type_message);

	ExitEnvoie(bal_dest, gsDMV[inst_id].ani_mbox, (struct_neutre *)(p_msg_emis));
}

PROTECTED void TLMSendStatus(dmv_inst_id inst_id, struct_srv_etat *psEtat)
{
	struct_tlm_message p_msg;
	struct_tlm_send_service data = { inst_id, &p_msg };

	p_msg.entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
	p_msg.entete.service = M_SRV_ETAT;
	p_msg.entete.type_message = SRV_TYP_NOUVEL_ETAT;

	memcpy(&p_msg.srv.srv_etat, psEtat, sizeof(struct_srv_etat));

	SrvPourChaqueDemandeur(gsDMV[inst_id].service[M_SRV_ETAT], EnvoiDemandeur, &data);
}

PROTECTED void TLMSendCmdState(dmv_inst_id instance_id, struct_tlm_display *message, enum_srv_service	service, enum_tlm_type	message_type)
{
	struct_tlm_message p_msg;
	struct_tlm_send_service	callback_param = { instance_id, &p_msg };

	if (!SrvEstVide(gsDMV[instance_id].service[M_TLM_MESSENGER_SERVICE]))
	{
		MInstTracer_FichierTrace(instance_id, "TLMSendCmdState() - message");
		// Filling the structure
		p_msg.entete.neutre.bl_retour = gsDMV[instance_id].ani_mbox;
		p_msg.entete.service = service;
		p_msg.entete.type_message = message_type;

		memcpy(&p_msg.srv.srv_tlm, message, sizeof(struct_tlm_display));

		SrvPourChaqueDemandeur(gsDMV[instance_id].service[M_TLM_MESSENGER_SERVICE], EnvoiDemandeur, &callback_param);
	}

}


PROTECTED void TLMSendSpyMessage(IN dmv_inst_id inst_id,
								  IN enum_espion_nature nature,
								  IN enum_espion_sens sens,
								  IN unsigned char * pBuffer,
								  IN unsigned int iLength)
{
	//function for handling dop service
	struct_tlm_message p_msg_emis;
	struct_tlm_send_service data = { inst_id, &p_msg_emis };

	// fill message structure wit data
	p_msg_emis.entete.service = M_SRV_ESPION;
	p_msg_emis.entete.type_message = SRV_TYP_MESSAGE_ESPION;
	p_msg_emis.srv.espion.nature = nature;
	p_msg_emis.srv.espion.sens = sens;
	p_msg_emis.srv.espion.taille = iLength;
	memcpy(p_msg_emis.srv.espion.donnee, pBuffer, iLength);

	SrvPourChaqueDemandeur(gsDMV[inst_id].service[M_SRV_ESPION], EnvoiDemandeurEspion, &data);
}

/**/
/*****************************************************************************/
/*SYNTAX: void TLMSendMessageToIos(short siInstId,                      */
/*                         tlm_internal_message_type eTypeMessage,           */
/*                         struct_tlm_message *pvParam)                      */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the IOS thread.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) short siInstId                - Instance ID.                   */
/*      (IN ) enum_tlm_ani_ios eTypeMessage - Message type.                  */
/*      (IN ) void *pvParam                 - Pointer to the message         */
/*                                            structure.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void TLMSendMessageToIos(short siInstId,
										tlm_internal_message_type eTypeMessage,
										struct_tlm_message *pvParam)
{
	struct_ani_ios *output_message = NULL;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&output_message), sizeof(struct_tlm_message), gsDMV[siInstId].pool);

	switch (eTypeMessage)
	{
	case TLM_MESSAGE_SET_REQUESTED:
	{
		output_message->message_id = M_TLM_MESSAGE_SET;
		memcpy(&output_message->u.ani_ios.u.tlm, &pvParam->srv.srv_tlm.u.display_srv, sizeof(struct_tlm_service));
	}
	break;

	case TLM_MESSAGE_GET_REQUESTED:
	{
		output_message->message_id = M_TLM_MESSAGE_GET;
		memcpy(&output_message->u.ani_ios.u.tlm, &pvParam->srv.srv_tlm.u.display_srv, sizeof(struct_tlm_service));
	}
	break;

	case TLM_MESSAGE_FLASHER_GET_REQUESTED:
	{
		output_message->message_id = M_TLM_FLASHER_GET;
		memcpy(&output_message->u.ani_ios.u.flasher, &pvParam->srv.srv_tlm.u.flasher_srv, sizeof(struct_tlm_service));
	}
	break;

	case TLM_MESSAGE_FLASHER_SET_REQUESTED:
	{
		output_message->message_id = M_TLM_FLASHER_SET;
		memcpy(&output_message->u.ani_ios.u.flasher, &pvParam->srv.srv_tlm.u.flasher_srv, sizeof(struct_tlm_service));
	}
	break;
	}
	

	MInstTracer_FichierTrace(siInstId, "TLM_ANI: EnvoiIOS() => type_message '%d' ", output_message->message_id);

	// Sending the message
	if (Envoie(gsDMV[siInstId].ios_mbox, gsDMV[siInstId].ani_mbox, (struct_neutre *)output_message) != NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&output_message);
	}
}

PRIVATE BOOL WINAPI EnvoiDemandeur(IN PVOID Param,
								   IN noyau_bal_id BalDemandeur,
								   IN PVOID DataDuDemandeur)
{
	struct_tlm_send_service	*p_data = (PVOID)Param;
	struct_tlm_message			*p_msg_emis;

	ExitAlloue((struct_neutre **)(&p_msg_emis), sizeof(struct_tlm_message), gsDMV[p_data->inst_id].pool);

	memcpy(p_msg_emis, p_data->p_msg, sizeof(struct_tlm_message));
	ExitEnvoie(BalDemandeur, gsDMV[p_data->inst_id].ani_mbox, (struct_neutre *)(p_msg_emis));

	return TRUE;
}

PRIVATE BOOL WINAPI EnvoiDemandeurEspion(IN PVOID Param,
										 IN noyau_bal_id BalDemandeur,
										 IN PVOID DataDuDemandeur)
{
	struct_tlm_send_service *p_param = Param;
	struct_srv_espion *p_data = DataDuDemandeur;
	struct_tlm_message *p_msg_emis;

	// test si demandeur interesse ou non
	if ((p_data->nature == SRV_ESPION_MESSAGE_SERVICE || p_param->p_msg->srv.espion.nature == SRV_ESPION_MESSAGE_DONNEES) &&
		(p_data->sens == SRV_ESPION_ENTRANT_SORTANT || p_data->sens == p_param->p_msg->srv.espion.sens))
	{

		ExitAlloue((struct_neutre **)(&p_msg_emis), sizeof(struct_tlm_message), gsDMV[p_param->inst_id].pool);

		// recopie du message
		*p_msg_emis = *(p_param->p_msg);

		ExitEnvoie(BalDemandeur, gsDMV[p_param->inst_id].ani_mbox, (struct_neutre *)(p_msg_emis));
	}
	// en renvoyant TRUE, on passe au demandeur suivant
	return TRUE;
}

//|//
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: The function sends the message to the ANI thread.
// NOTES:		-
// PARAMS:		instance_id				- Instance ID.
//				message_type			- Message type.
//				message					- ?? purpose unknown...
// RETURNS:		-
///////////////////////////////////////////////////////////////////////////////
PROTECTED void TLMSendMessageToAni(UINT instance_id,
										tlm_internal_message_type	message_type,
										void *pMsg)
{
	struct_tlm_message	*message = NULL;
	noyau_enum_retour	send_result;

	ExitAlloue((struct_neutre**)(&message), sizeof(struct_tlm_message), gsDMV[instance_id].pool);

	message->entete.type_message = message_type;

	switch (message_type)
	{
	case TLM_MESSAGE_STATE_RECEIVED_ACQ:
		memcpy(&message->srv.srv_tlm, pMsg, sizeof(struct_tlm_service));
		break;

	case TLM_MESSAGE_STATE_RECEIVED_NACQ:
		memcpy(&message->srv.srv_tlm, pMsg, sizeof(struct_tlm_service));
		break;

	default:
		break;
	}

	MInstTracer_FichierTrace((short int)instance_id, "TLM_IOS: TLMSendMessageToAni() => message_type '%d'", message->entete.type_message);

	send_result = Envoie(gsDMV[instance_id].ani_mbox,
						 gsDMV[instance_id].ios_mbox,
						 (struct_neutre*)message);

	if (send_result != NOYAU_OK)
	{
		ExitLibere((struct_neutre**)&message);
	}
}