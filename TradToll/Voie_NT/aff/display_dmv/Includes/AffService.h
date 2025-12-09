#ifndef AFF_SERVICE_H
#define AFF_SERVICE_H

#include <aff_ext.h>
#include <aff_util.h>
#include <DmvGlobal.h>

#include <protect.h>

PROTECTED void AFFProcessServiceMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu);
PROTECTED void AFFProcessViewMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu);
PROTECTED void AFFProcessStatusMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu);
PROTECTED void AFFProcessSpyMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu);
PROTECTED void AFFProcessStopMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu);
PROTECTED void AFFProcessDeviceInfoMsg(dmv_inst_id inst_id,struct_aff_message *p_message);

PROTECTED void AffManagement(dmv_inst_id inst_id, struct_aff_message *p_message, aff_infos *buffer);
PROTECTED void ResetManagement(dmv_inst_id inst_id, struct_aff_message *p_message);
PROTECTED void DimmingManagement(dmv_inst_id inst_id, struct_aff_message *p_message);

#undef PROTECTED
#undef INIT
#undef I
#endif 
