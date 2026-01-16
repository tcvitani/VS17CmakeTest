/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_aut.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef ROUTE_AUT_H
#define ROUTE_AUT_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_aut.h>
#include <route_acom.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef PVOID aut_route_id;

typedef struct
{
	struct MSG_PV_COM_REQ * p_req;
	struct MSG_PV_COM_REQ_Lanenum *p_lane;
}
EVT_ROUTE_COMMAND_ARGS;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED BOOL ROUTE_AUT_Init(void);

PROTECTED BOOL ROUTE_AUT_New(aut_route_id *id, ACOM_CNX_HANDLE hCnxHandle);

PROTECTED BOOL ROUTE_AUT_Delete(aut_route_id *aut_route);

PROTECTED BOOL ROUTE_AUT_Send(aut_route_id id, aut_event_id event, PVOID args);

/*-------------------------------- VARIABLES: -------------------------------*/

PROTECTED aut_event EVT_ROUTE_CONNECTION;           // ACOM_CNX_HANDLE
PROTECTED aut_event EVT_ROUTE_DISCONNECTION;        // ACOM_CNX_HANDLE
PROTECTED aut_event EVT_ROUTE_LANE_CONNECTION;      // struct MSG_SV_CON_REP *
PROTECTED aut_event EVT_ROUTE_MSG_TRANSACTION;      // struct MSG_TRANSACTION *
PROTECTED aut_event EVT_ROUTE_MSG_INFORMATION;      // struct MSG_COMP_INF_TR *
PROTECTED aut_event EVT_ROUTE_MSG_STATUS;           // struct MSG_STATUS *
PROTECTED aut_event EVT_ROUTE_MSG_EVENT;            // struct MSG_EVENT *
PROTECTED aut_event EVT_ROUTE_MSG_BOWL;             // struct MSG_BOWL_STAT *
PROTECTED aut_event EVT_ROUTE_MSG_VAULT;            // struct MSG_VAULT_STAT *
PROTECTED aut_event EVT_ROUTE_MSG_PARTLY;           // struct MSG_PARTLY *
PROTECTED aut_event EVT_ROUTE_MSG_DISK;             // struct MSG_COIN_LISTING *
PROTECTED aut_event EVT_ROUTE_MSG_COUNTERS;         // struct MSG_COUNTER_STATUS *
PROTECTED aut_event EVT_ROUTE_COMMAND;              // EVT_ROUTE_COMMAND_ARGS *
PROTECTED aut_event EVT_ROUTE_MSG_TRACE;            // struct MSG_TRACE *

#endif

/*-------------------------------- END OF FILE ------------------------------*/