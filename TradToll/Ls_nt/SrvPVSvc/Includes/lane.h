/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     lane.h															 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef LANE_H
#define LANE_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <acom.h>
#include <msg_pv_plaz_rep.h>
#include <msg_pv_lane_rep.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define LANE_MAX_COUNTERS				5
#define LANE_MAX_DISPLAY				5

#define LANE_UNKNOWN_EQT_STATE			0
#define LANE_UNKNOWN_MODE_STATE			0
#define LANE_UNKNOWN_SHIFT_STATE		0

#define CLASS_ONTHEFLY1_SUBTYPE			6
#define CLASS_ONTHEFLY2_SUBTYPE			7
#define CLASS_RESET1_SUBTYPE			8
#define CLASS_RESET2_SUBTYPE			9

#define LANE_SHIFT_STATUS_TYPE			1
#define LANE_SHIFT_STATUS_SUBTYPE		4
#define LANE_NORMAL_SHIFT_OPEN			1

#define LANE_SHIFT_MODE_TYPE			1
#define LANE_SHIFT_MODE_SUBTYPE			10
#define LANE_MANUAL_MODE				4

#define VALIDATION_REQUEST_TYPE			4
#define VALIDATION_REQUEST_SUBTYPE		68
#define VALIDATION_REQUEST_STATUS		1
#define VALIDATION_REQUEST_COMMAND_ID	75

#define VALIDATION_RESPONSE_TYPE		4
#define VALIDATION_RESPONSE_SUBTYPE		69
#define VALIDATION_RESPONSE_STATUS		1
#define VALIDATION_RESPONSE_COMMAND_ID	76

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef enum
{
	DISP_COUNT_NOTHING = 0,
	DISP_COUNT_TRAFFIC,
	DISP_COUNT_VIOLATIONS,
	DISP_COUNT_UNACKED_ALARMS,
	DISP_COUNT_ALL_ALARMS,
	DISP_COUNT_TRAFFIC_OF_YESTERDAY
}
DISP_COUNT;

typedef enum
{
	ACTIVE_FREE_STATE = 0,
	INACTIVE_STATE,
	BUSY_STATE
}
LANE_ACTIVE_STATE;

struct DISP_CNF
{
	// set of counters by display
	DWORD id;
	DISP_COUNT count[LANE_MAX_COUNTERS];
	DWORD total[LANE_MAX_COUNTERS];
};

struct LANE_CNF
{
	// plaza number
	DWORD plaza_number;

	// ROUTESVC connection handle
	ACOM_CNX_HANDLE hCnxHandle;

	// lane status message
	BOOL lane_rep_altered;
	struct MSG_PV_LANE_REP *msg_lane;
	BOOL reset_daily_traffic;

	// traffic j-1
	BOOL init_yesterday_traffic;
	BOOL save_yesterday_traffic;
	DWORD yesterday_traffic;
	DWORD intermediate_traffic;

	// list of equipment
	HLIST list_eqt_cnf;

	// list of alarms
	BOOL list_alarm_altered;
	HLIST list_alarm;
	DWORD unacked_alarms;

	// set of counters by display
	BOOL display[LANE_MAX_DISPLAY][LANE_MAX_COUNTERS];

	LANE_ACTIVE_STATE	LaneActiveState;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED BOOL LANE_Init(void);

PROTECTED DWORD LANE_Get_Plaza_Number(struct LANE_CNF *p_lane_cnf);

PROTECTED DWORD LANE_Get_Number(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Connection(struct LANE_CNF *p_lane_cnf, ACOM_CNX_HANDLE hCnxHandle);

PROTECTED BOOL LANE_Set_Disconnection(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Get_Connection_State(struct LANE_CNF *p_lane_cnf);

PROTECTED ACOM_CNX_HANDLE LANE_Get_Connection_Handle(struct LANE_CNF *p_lane_cnf);

PROTECTED struct MSG_PV_LANE_REP *LANE_Build_msg_pv_lane_rep(struct LANE_CNF *p_lane_cnf, BOOL partial);

PROTECTED BOOL LANE_Delete_All(HLIST hList, struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Traffic(struct LANE_CNF *p_lane_cnf, DWORD daily_traffic, DWORD daily_violation);

PROTECTED BOOL LANE_Set_Daily_Traffic_Flag(struct LANE_CNF *p_lane_cnf, BOOL reset);

PROTECTED BOOL LANE_Get_Daily_Traffic_Flag(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Init_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Save_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Intermediate_Traffic(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Trs_Body(struct LANE_CNF *p_lane_cnf,
								 DWORD collector_id,
								 DWORD active_vault,
								 SYSTEMTIME *message_date,
								 DWORD daily_traffic,
								 DWORD daily_violation,
								 DWORD payment_type,
								 DWORD payment_sub_type,
								 DOUBLE tollfare,
								 DWORD currency_id,
								 CHAR *pExlLicencePlate);

PROTECTED BOOL LANE_Set_Trs_Classes(struct LANE_CNF *p_lane_cnf,
									DWORD transaction_class,
									DWORD detected_class,
									DWORD keyed_class);

PROTECTED BOOL LANE_Set_Comp_Classes(struct LANE_CNF *p_lane_cnf,
									 DWORD comp_class,
									 DWORD comp_num);

PROTECTED BOOL LANE_Reset_Trs_Anomaly(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Trs_Anomaly(struct LANE_CNF *p_lane_cnf,
									DWORD anomaly_id,
									DWORD state,
									CHAR *pDescription);

PROTECTED BOOL LANE_Reset_Trs_Id(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Trs_Id(struct LANE_CNF *p_lane_cnf, DWORD id);

PROTECTED BOOL LANE_Set_Counters(struct LANE_CNF *p_lane_cnf,
								 DWORD nb_display,
								 struct DISP_CNF display[LANE_MAX_DISPLAY]);

PROTECTED BOOL LANE_Set_Enl_Plate(struct LANE_CNF *p_lane_cnf, HLIST hListEntryInfo);

PROTECTED HLIST *LANE_Get_List_Alarm(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Is_Alarm_List_Altered(struct LANE_CNF *p_lane_cnf);

PROTECTED void LANE_Set_Alarm_Flag(struct LANE_CNF *p_lane_cnf, BOOL state);

PROTECTED void LANE_Set_Unacked_Alarms_Count(struct LANE_CNF *p_lane_cnf, DWORD count);

PROTECTED BOOL LANE_Set_Shift(struct LANE_CNF *p_lane_cnf, DWORD matricule, DWORD vault, DWORD state);

PROTECTED BOOL LANE_Set_Vault(struct LANE_CNF *p_lane_cnf, DWORD vault);

PROTECTED DWORD LANE_Get_Shift(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_Mode(struct LANE_CNF *p_lane_cnf, DWORD state);

PROTECTED BOOL LANE_Search_And_Set_Eqt(struct LANE_CNF *p_lane_cnf, DWORD dwEvent, DWORD sub_event, DWORD status);

PROTECTED BOOL LANE_Reset_All_Eqt(struct LANE_CNF *p_lane_cnf);

PROTECTED struct LANE_CNF * LANE_Build_Conf_From_DB(HLIST hList,
													DWORD plaza_number,
													DWORD lane_number,
													DWORD nb_display,
													struct DISP_CNF display[LANE_MAX_DISPLAY]);

PROTECTED BOOL LANE_Reset_List_Other(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL LANE_Set_List_Other_Element(struct LANE_CNF *p_lane_cnf, 
										   DWORD dwType, 
										   struct MSG_COMP_INF_TR_Elem_Info *p_lc_info);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/