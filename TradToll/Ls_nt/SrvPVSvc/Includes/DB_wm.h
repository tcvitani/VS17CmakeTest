/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     DB_WM.h														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef DB_WM_H
#define DB_WM_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <acom.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

// WINDOW MESSAGES
enum DB_WM_TYPE
{
	DB_WM_ACK_ALARM = 1,		// struct MSG_PV_ALAR_REQ *
	DB_WM_USER_LOGIN_REQ,       // struct MSG_PV_LOG_REQ *
	DB_WM_USER_PROFILE_REQ,     // struct MSG_PV_USER_REQ *
	DB_WM_USER_AUTH_REQ,        // struct MSG_PV_AUTH_REQ *
	DB_WM_CMD_REQ,              // struct MSG_PV_COM_REQ *
	DB_WM_CONNECT_STAUS_REQ,    // NULL
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED BOOL DB_Start(void);

PROTECTED BOOL DB_Stop(DWORD TimeOut);

PROTECTED BOOL DB_PostMessage(enum DB_WM_TYPE type, PVOID Param);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
