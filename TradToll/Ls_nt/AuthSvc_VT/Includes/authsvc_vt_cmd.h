#ifndef AUTHSVC_CMD_H
#define AUTHSVC_CMD_H

#include <protect.h>

PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg );


#endif
