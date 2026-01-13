#ifndef AUTHSVC_DB_H
#define AUTHSVC_DB_H

#include <protect.h>

// ----------------- MACROS ----------------------

#define DB_NAL ((void*)(0xFFFFFFFF))

#define DB_DATE_CHAR_LENGHT 20
#define DB_ID_ENTRY_TRS_LENGHT 12
#define DB_ID_ENTRY_TRS_LENGHT_V3 24
#define DB_ENTRY_VRN_LENGHT 20
#define DB_ENTRY_VRN_COUNTRY 3
#define DB_ENTRY_VRN_PROVIDER 6
#define DB_ENTRY_ACCEPTED_DAYS 2

#define AUTHSVC_INTERNAL_SERVER_ERROR 999998

// ----------------- TYPES -----------------------


// ----------------- FUNCTIONS -------------------

PROTECTED BOOL DBOpen();
PROTECTED void DBCleanup();
PROTECTED BOOL DBIfShouldRetry(DWORD dwErr);

PROTECTED BOOL DBGetIsDatabaseMissing();
PROTECTED unsigned long long DBGetLastTimeDbCnxUsed();

PROTECTED DWORD DBAuthVT_EnlReq(struct MSG_LC_AUTH_VT_ENL_REQ *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP *psAuthRsp);
PROTECTED DWORD DBAuthVT_ExlReq(struct MSG_LC_AUTH_VT_EXL_REQ *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP *psAuthRsp);

PROTECTED DWORD DBAuthVT_EnlReqV2(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthRsp);
PROTECTED DWORD DBAuthVT_ExlReqV2(struct MSG_LC_AUTH_VT_EXL_REQ_V2 *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthRsp);

PROTECTED DWORD DBAuthVT_EnlReqV3(struct MSG_LC_AUTH_VT_ENL_REQ_V3 *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthRsp);
PROTECTED DWORD DBAuthVT_ExlReqV3(struct MSG_LC_AUTH_VT_EXL_REQ_V3 *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthRsp);

PROTECTED DWORD DBDoDummyDBReq();
#endif
