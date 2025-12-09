#ifndef AFF_MESSAGE_H
#define AFF_MESSAGE_H

#include <DmvGlobal.h>
#include <protect.h>

PROTECTED enum_aff_validity AFFStartService(dmv_inst_id         inst_id,
											enum_aff_service	service_id,
											noyau_bal_id        bal_dest);

PROTECTED enum_aff_validity AFFStartSpyService(dmv_inst_id         inst_id,
											   noyau_bal_id        BalDemandeur,
											   enum_espion_nature  nature,
											   enum_espion_sens	   sens);

PROTECTED enum_aff_validity AFFEndService(dmv_inst_id      inst_id,
										  enum_aff_service service_id,
										  noyau_bal_id     bal_dest);

PROTECTED void AFFSendAcknowledgement(dmv_inst_id      inst_id,
									  noyau_bal_id     bal_dest,
									  enum_aff_service service_id,
									  enum_aff_type    type_message);

PROTECTED void AFFSendSpyMessage(dmv_inst_id  ident_aff,
							     unsigned char *msg_Espion,
							     short int longueur,
							     enum_espion_nature nature,
							     enum_espion_sens sens);

PROTECTED void AFFSendStatus(dmv_inst_id   inst_id,
							 noyau_bal_id  bal_dest,
							 enum_aff_type type);

PROTECTED void AFFSendView(dmv_inst_id   inst_id,
						   noyau_bal_id  bal_dest,
						   enum_aff_type type);


PROTECTED void AFFSendToIos(dmv_inst_id   inst_id,
							enum_ani_ios  msg_id,
							void          *parametre);

PROTECTED void AFFReleaseMailbox(noyau_bal_id bal);


#undef PROTECTED
#undef I
#undef INIT
#endif