#ifndef DMV_ANI_H
#define DMV_ANI_H

#include <protect.h>

PROTECTED DWORD WINAPI Ani(PVOID param);

PROTECTED void ReceptionIos(dmv_inst_id inst_id, struct_neutre *p_neutre);

#undef PROTECTED
#undef I
#undef INIT
#endif