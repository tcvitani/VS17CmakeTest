#ifndef TLM_MESSAGE_H
#define TLM_MESSAGE_H

#include <protect.h>


PROTECTED void TLMFreeMailbox(noyau_bal_id bal);

PROTECTED void TLMSendMessage(dmv_inst_id		instance_id,
								   noyau_bal_id		destination_mailbox_id,
								   enum_srv_service	service,
								   enum_tlm_type	message_type);

PROTECTED boolean TLMStartService(dmv_inst_id inst_id,
							   enum_srv_service service_id,
							   noyau_bal_id bal_demandeur);

PROTECTED boolean TLMEndService(dmv_inst_id inst_id,
							 enum_srv_service service_id,
							 noyau_bal_id bal_demandeur);

PROTECTED boolean TLMStartSpyService(dmv_inst_id inst_id,
									 noyau_bal_id bal_demandeur,
									 enum_espion_sens sens,
									 enum_espion_nature nature);
PROTECTED void TLMSendStatus(dmv_inst_id inst_id, struct_srv_etat *psEtat);

PROTECTED void TLMSendCmdState(dmv_inst_id instance_id, 
							   struct_tlm_display *message, 
							   enum_srv_service service, 
							   enum_tlm_type message_type);

PROTECTED void TLMSendAcknowledgement(dmv_inst_id inst_id,
								 noyau_bal_id bal_dest,
								 enum_srv_service service_id,
								 enum_srv_type type_message);

PROTECTED void TLMSendSpyMessage(dmv_inst_id inst_id,
								  enum_espion_nature nature,
								  enum_espion_sens sens,
								  unsigned char * pBuffer,
								  unsigned int iLength);

PROTECTED void TLMSendMessageToIos(short siInstId,
										tlm_internal_message_type eTypeMessage,
										struct_tlm_message *pvParam);

PROTECTED void TLMSendMessageToAni(UINT instance_id,
										tlm_internal_message_type	message_type,
										void *pMsg);

#include <undef.h>
#endif
