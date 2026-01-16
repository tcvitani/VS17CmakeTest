/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SRVPV_WM
 * FILE       : SRVPV_WM.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : communication asynchrone mailslot named pipe
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef PV_WM_H
#define PV_WM_H

// Types standards (DWORD, LONG, BOOL etc...)
#include <windows.h>
#include <acom.h>
#include <msg_pv_log_rep.h>
#include <msg_pv_log_req.h>
#include <msg_pv_user_rep.h>
#include <msg_pv_user_req.h>
#include <msg_pv_auth_rep.h>
#include <msg_pv_serv_rep.h>

#include <protect.h>


// WINDOW MESSAGES

// Type des événements signalé dans les window messages
enum PV_WM_TYPE 
{
    PV_WM_PLAZA_STATE  = 1,     // struct MSG_PV_PLAZ_REP *
    PV_WM_ALARM_STATE,          // struct MSG_PV_ALAR_REP *
    PV_WM_SERVER_COM_FAILURE,   // NULL
    PV_WM_DATA_BASE_FAILURE,    // ACOM_CNX_HANDLE *
    PV_WM_RELOAD,               // NULL
	PV_WM_COMP_INFO,			// struct MSG_PV_INFO_REP *
    PV_WM_BOWL_STATUS,          // struct MSG_PV_BOWL_REP *
    PV_WM_VAULT_STATUS,         // struct MSG_PV_VAUL_REP *
	PV_WM_PARTLY,				// struct MSG_PV_PART_REP *
    PV_WM_DISK	,				// struct MSG_PV_DISK_REP *
	PV_WM_TRACE,				// struct MSG_PV_TRAC_REP *
    PV_WM_USER_LOGIN_REP,       // struct PV_WM_PARAM_USER_LOGIN_REP *
    PV_WM_USER_PROFILE_REP,     // struct PV_WM_PARAM_USER_PROFILE_REP *
    PV_WM_USER_AUTH_REP,        // struct PV_WM_PARAM_USER_AUTH_REP *
    PV_WM_DB_CMD_REP,           // struct PV_WM_PARAM_DB_CMD_REP *
    PV_WM_CMD_LIST_USERS,       // ACOM_CNX_HANDLE *
	PV_WM_CONNECTION_STATE
};

struct PV_WM_PARAM_USER_LOGIN_REP
{
    ACOM_CNX_HANDLE hCnxHandle;
    struct MSG_PV_LOG_REQ_Body req;
    struct MSG_PV_LOG_REP_Body user;
    DWORD user_ok;
    DWORD profile_max_connections;
};

struct PV_WM_PARAM_USER_PROFILE_REP
{
    ACOM_CNX_HANDLE hCnxHandle;
    struct MSG_PV_USER_REQ_Body req;
    struct MSG_PV_USER_REP *p_profile;
};

struct PV_WM_PARAM_USER_AUTH_REP
{
    ACOM_CNX_HANDLE hCnxHandle;
    struct MSG_PV_AUTH_REP_Body user;
};

struct PV_WM_PARAM_DB_CMD_REP
{
    ACOM_CNX_HANDLE hCnxHandle;
    CHAR string[MSG_PV_MAX_MESSAGE_SIZE];
};

typedef struct
{
	CHAR		szName[MAX_PATH];
	BOOL		bConnected;
	SYSTEMTIME	stTime;
}
struct_link_status;

DWORD	dwNbOfPVConnection;		//Number of connected PV able for validation. (Communication with automatic lane)

// ------------------ FONCTIONS ---------------------

PROTECTED BOOL PV_Start(void);
   
PROTECTED BOOL PV_Stop(DWORD TimeOut);

PROTECTED BOOL PV_PostMessage(enum PV_WM_TYPE type, PVOID Param);

PROTECTED DWORD PV_Count_User_Connections(CHAR profile_name[MSG_PV_MAX_USER_PROFILE_NAME]);

PROTECTED VOID SendMsgLinkStatus(void);

PROTECTED void CheckLaneLinkStatus(DWORD dwLaneNum, BOOL bLaneState);
PROTECTED HLIST LaneLinkStatusListNew(void);
PROTECTED BOOL LaneLinkStatusListDelete(void);
PROTECTED VOID Delete_LinkStatusList();

PROTECTED PVOID New_List_Item(IN OUT HLIST *hList, IN DWORD dwSizeOfMessage);
PROTECTED PVOID List_Get_First_Item(IN HLIST hList);
PROTECTED PVOID List_Get_Next_Item(IN HLIST hList, IN PVOID hMsg);
PROTECTED BOOL AddItemTo_LinkStatusList(struct_link_status *pLinkStatus);

#endif
