#ifndef TLM_SERVICE_H
#define TLM_SERVICE_H

#include <protect.h>

PROTECTED void TLMProcessServiceMsg(dmv_inst_id inst_id, struct_tlm_message * p_msg_recu);
PROTECTED void TLMProcessFlasherMsg(dmv_inst_id inst_id, struct_tlm_message * p_msg_recu);

#undef PROTECTED
#undef I
#undef INIT
#endif

