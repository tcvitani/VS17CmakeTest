/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_wm.h														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef ROUTE_WM_H
#define ROUTE_WM_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <msg_pv_com_req.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

// event types used for the window messages
enum ROUTE_WM_TYPE
{
	ROUTE_WM_TOTAL_PLAZA_STATE = 1,    // NULL
	ROUTE_WM_ACK_ALARM,                 // struct MSG_PV_ACK_REQ *
	ROUTE_WM_COMMAND,                   // struct MSG_PV_COM_REQ *
	ROUTE_WM_ACK_ALARM_TEST,			// struct MSG_PV_ACK_REQ_REP *
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED BOOL ROUTE_Start(BOOL RealTimeUpdate);

PROTECTED BOOL ROUTE_Stop(DWORD TimeOut);

PROTECTED BOOL ROUTE_PostMessage(enum ROUTE_WM_TYPE type, PVOID Param);

PROTECTED BOOL ROUTE_Command(struct MSG_PV_COM_REQ *p_req);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
