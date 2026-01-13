#ifndef TOLANESSVC_DB_H
#define TOLANESSVC_DB_H

#include <protect.h>

PROTECTED DWORD DBInitRequests();
PROTECTED DWORD DBGetLanes();
PROTECTED void DBCleanup();

#endif
