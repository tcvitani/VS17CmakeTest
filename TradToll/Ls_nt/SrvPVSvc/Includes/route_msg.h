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

#ifndef ROUTE_MSG_H
#define ROUTE_MSG_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <msg_lc_status.h>
#include <msg_lc_event.h>
#include <msg_lc_transaction.h>
#include <msg_lc_bowl_stat.h>
#include <msg_lc_vault_stat.h>
#include <msg_lc_partly.h>
#include <msg_lc_coin_listing.h>
#include <msg_lc_counter_status.h>
#include <msg_lc_trace.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED BOOL ROUTE_Send_msg_sv_con_req(ACOM_CNX_HANDLE hCnxHandle);

PROTECTED BOOL ROUTE_Send_msg_sv_filt_dec(ACOM_CNX_HANDLE hCnxHandle);

PROTECTED BOOL ROUTE_Process_Msg_Status(struct MSG_STATUS *p_stat);

PROTECTED BOOL ROUTE_Process_Msg_Event(struct MSG_EVENT *p_event);

PROTECTED BOOL ROUTE_Process_Msg_Transaction(struct MSG_TRANSACTION *p_trs);

PROTECTED BOOL ROUTE_Process_Msg_Counters(struct MSG_COUNTER_STATUS *p_counters);

PROTECTED BOOL ROUTE_Process_Msg_Information(struct MSG_COMP_INF_TR *p_info);

PROTECTED struct MSG_PV_BOWL_REP *ROUTE_Process_Msg_Bowl(struct MSG_BOWL_STAT *p_stat);

PROTECTED struct MSG_PV_VAUL_REP *ROUTE_Process_Msg_Vault(struct MSG_VAULT_STAT *p_stat);

PROTECTED struct MSG_PV_PART_REP *ROUTE_Process_Msg_Partly(struct MSG_PARTLY *p_stat);

PROTECTED struct MSG_PV_DISK_REP *ROUTE_Process_Msg_Disk(struct MSG_COIN_LISTING *p_stat);

PROTECTED struct MSG_PV_TRAC_REP *ROUTE_Process_Msg_Trace(struct MSG_TRACE *p_stat);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/