#ifndef PLAZA_H
#define PLAZA_H

#include <windows.h>
#include <msg_pv_conf_rep.h>

#include <protect.h>

PROTECTED BOOL PV_Build_Msg_Conf_From_DB (void);

PROTECTED struct MSG_PV_CONF_REP *PV_Build_msg_pv_conf_rep (void);

PROTECTED BOOL PV_Delete_Conf (void);

#endif