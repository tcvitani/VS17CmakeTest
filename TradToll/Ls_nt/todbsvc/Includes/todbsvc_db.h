#ifndef TODBSVC_DB_H
#define TODBSVC_DB_H

#include <protect.h>

#define DB_NAL ((void*)(0xFFFFFFFF))

#define DB_FILE_VERIFY   0x00000001
#define DB_FILE_BACKUP   0x00000002
#define DB_FILE_LAST_TRY 0x00000004

PROTECTED DWORD DBInitRequests();
PROTECTED DWORD DBGetLanes();
PROTECTED DWORD DBIncorporate( DB_CNX ** phDbCnx, char * pcFile, TODBSVC_LANE * psLane, DWORD * pdwMsgCount, DWORD dwMode );
PROTECTED DWORD DBGetLastTreatedNumber( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane );
PROTECTED DWORD DBSetLastTreatedNumber( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane );
PROTECTED DWORD DBBeginLoad( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane, BOOL bBackup );
PROTECTED DWORD DBEndLoad( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane, BOOL bErr );
PROTECTED DWORD DBInsertMessage( DB_CNX ** phDbCnx, char * pcMsg, DWORD dwSize );


#endif
