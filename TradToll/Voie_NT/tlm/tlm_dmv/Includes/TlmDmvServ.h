#ifndef TLM_DMV_SERV_H
#define TLM_DMV_SERV_H

#include <protect.h>

PROTECTED void Ani_ProcessArretMsg(tlm_inst_id inst_id, struct_tlm_message *p_msg_recu);
PROTECTED void Ani_ProcessEtatMsg(tlm_inst_id inst_id, struct_tlm_message *p_msg_recu);
PROTECTED void Ani_ProcessEspionMsg(tlm_inst_id inst_id, struct_tlm_message * p_msg_recu);
PROTECTED void Ani_ProcessTlmMsg(tlm_inst_id inst_id, struct_tlm_message * p_msg_recu);

#undef PROTECTED
#undef I
#undef INIT
#endif

