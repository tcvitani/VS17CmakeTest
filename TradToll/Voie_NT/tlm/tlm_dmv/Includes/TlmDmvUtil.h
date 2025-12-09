#ifndef TLM_DMV_UTIL_H
#define TLM_DMV_UTIL_H

#include <protect.h>


PROTECTED void TlmUtil_FreeMailbox(noyau_bal_id bal);

PROTECTED void TlmUtil_SendMessage(tlm_inst_id		instance_id,
								   noyau_bal_id		destination_mailbox_id,
								   enum_srv_service	service,
								   enum_tlm_type	message_type);

PROTECTED boolean DebutService(tlm_inst_id inst_id,
							   enum_srv_service service_id,
							   noyau_bal_id bal_demandeur);

PROTECTED boolean FinService(tlm_inst_id inst_id,
							 enum_srv_service service_id,
							 noyau_bal_id bal_demandeur);

PROTECTED boolean DebutServiceEspion(tlm_inst_id inst_id,
									 noyau_bal_id bal_demandeur,
									 enum_espion_sens sens,
									 enum_espion_nature nature);
PROTECTED void EnvoiEtat(tlm_inst_id inst_id, struct_srv_etat *psEtat);

PROTECTED void EnvoiCmdState(tlm_inst_id instance_id, 
							 struct_tlm_display *message, 
							 enum_srv_service service, 
							 enum_tlm_type message_type);

PROTECTED void EnvoiAcquittement(tlm_inst_id inst_id,
								 noyau_bal_id bal_dest,
								 enum_srv_service service_id,
								 enum_srv_type type_message);

PROTECTED void EnvoiMessageEspion(tlm_inst_id inst_id,
								  enum_espion_nature nature,
								  enum_espion_sens sens,
								  unsigned char * pBuffer,
								  unsigned int iLength);

PROTECTED void TlmUtil_SendMessageToIos(short siInstId,
										enum_tlm_ani_ios eTypeMessage,
										struct_tlm_message *pvParam);

PROTECTED void TlmUtil_SendMessageToAni(UINT instance_id,
										tlm_internal_message_type	message_type,
										void *pMsg);

#include <undef.h>
#endif
