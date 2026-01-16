/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     lane.c															 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <stdio.h>
#include <plaza.h>
#include <lane.h>
#include <alarm.h>
#include <srvpv_main.h>
#include <ntsvc.h>
#include <conf_srvpv.h>
#include <msg_lc_comp_inf_tr.h>
#include <msg_lc_transaction.h>
#include <msg_pv_lane_rep.h>
#include <route_msg_information.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define LANE_REG_VAL_RAZ_ALL_INFO	  "LaneInfoRazOnComFailure"
#define LANE_REG_VAL_CLASS_ONTHEFLY_1 "ClassOnTheFly1Position"
#define LANE_REG_VAL_CLASS_ONTHEFLY_2 "ClassOnTheFly2Position"
#define LANE_REG_VAL_CLASS_RESET_1    "ClassReset1Position"
#define LANE_REG_VAL_CLASS_RESET_2    "ClassReset2Position"
#define LANE_REG_VAL_CLASS_KEYIN      "ClassKeyinPosition" 

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct EQT_CNF
{
	// identifier of the associated alarm
	DWORD alarm_event;
	DWORD alarm_sub_event;

	// pointer to the corresponding item in the lane status message
	struct MSG_PV_LANE_REP_Equipment *p_msg_eqt;
};

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct
{
	BOOL bRazAllInfo;

	DWORD dwClassOnTheFly1Position;
	DWORD dwClassOnTheFly2Position;
	DWORD dwClassReset1Position;
	DWORD dwClassReset2Position;
	DWORD dwClassKeyinPosition;
}
LANE_PARAM;

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL LANE_Init(void)
{
	DWORD				dwErr		= 0;
	NTSVC_PARAMETER_DEF *psParams	= NULL;

	// registry parameters
	psParams = NTSVCOpenParameters(
		LANE_REG_VAL_RAZ_ALL_INFO,		REG_DWORD,	4,	0,	&LANE_PARAM.bRazAllInfo,
		LANE_REG_VAL_CLASS_ONTHEFLY_1,	REG_DWORD,	4,	0,	&LANE_PARAM.dwClassOnTheFly1Position,
		LANE_REG_VAL_CLASS_ONTHEFLY_2,	REG_DWORD,	4,	0,	&LANE_PARAM.dwClassOnTheFly2Position,
		LANE_REG_VAL_CLASS_RESET_1,		REG_DWORD,	4,	0,	&LANE_PARAM.dwClassReset1Position,
		LANE_REG_VAL_CLASS_RESET_2,		REG_DWORD,	4,	0,	&LANE_PARAM.dwClassReset2Position,
		LANE_REG_VAL_CLASS_KEYIN,		REG_DWORD,	4,	0,	&LANE_PARAM.dwClassKeyinPosition,
		NULL);

	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	return TRUE;
}

PROTECTED DWORD LANE_Get_Plaza_Number(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->plaza_number;
}

PROTECTED DWORD LANE_Get_Number(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->msg_lane->body.lanenum;
}

PROTECTED BOOL LANE_Set_Connection(struct LANE_CNF *p_lane_cnf, ACOM_CNX_HANDLE hCnxHandle)
{
	p_lane_cnf->msg_lane->body.connection_state = TRUE;
	p_lane_cnf->hCnxHandle = hCnxHandle;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Set_Disconnection(struct LANE_CNF *p_lane_cnf)
{
	p_lane_cnf->msg_lane->body.connection_state = FALSE;
	p_lane_cnf->hCnxHandle						= 0;

	// if we need to delete certain data ...
	if (LANE_PARAM.bRazAllInfo)
	{
		// state of shift unknow, reset collector and last transaction
		LANE_Set_Shift(p_lane_cnf, 0, 0, LANE_UNKNOWN_SHIFT_STATE);

		// state of mode unknown
		LANE_Set_Mode(p_lane_cnf, LANE_UNKNOWN_MODE_STATE);

		// delete the list of transaction anomalies
		LANE_Reset_Trs_Anomaly(p_lane_cnf);

		// clear other list
		LANE_Reset_List_Other(p_lane_cnf);

		// delete the list of lane alarms
		ALARM_Lane_Delete_All(p_lane_cnf);

		// reset equipment status
		LANE_Reset_All_Eqt(p_lane_cnf);
	}

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Get_Connection_State(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->msg_lane->body.connection_state;
}

PROTECTED ACOM_CNX_HANDLE LANE_Get_Connection_Handle(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->hCnxHandle;
}

PROTECTED struct MSG_PV_LANE_REP *LANE_Build_msg_pv_lane_rep(struct LANE_CNF * p_lane_cnf, BOOL partial)
{
	// if only changes are requested and if the status of the lane has not been changed
	// do not return anything!
	if (partial && !p_lane_cnf->lane_rep_altered)
		return NULL;

	// reset the flag of alteration
	p_lane_cnf->lane_rep_altered = FALSE;

	// "deep copy" ff the current state of the lane
	return MSG_Duplicate(p_lane_cnf->msg_lane);
}

PROTECTED HLIST *LANE_Get_List_Alarm(struct LANE_CNF *p_lane_cnf)
{
	return &p_lane_cnf->list_alarm;
}

PROTECTED BOOL LANE_Is_Alarm_List_Altered(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->list_alarm_altered;
}

PROTECTED void LANE_Set_Alarm_Flag(struct LANE_CNF *p_lane_cnf, BOOL state)
{
	p_lane_cnf->list_alarm_altered = state;
}

PROTECTED void LANE_Set_Unacked_Alarms_Count(struct LANE_CNF *p_lane_cnf, DWORD count)
{
	p_lane_cnf->unacked_alarms = count;
}

PROTECTED BOOL LANE_Set_Shift(struct LANE_CNF *p_lane_cnf, DWORD matricule, DWORD vault, DWORD state)
{
	// shift state: closed, open, maintenance [0, 1, 2]
	p_lane_cnf->msg_lane->body.lane_state = state;

	// if shift is unknown, reset collector id and active vault
	if (state == LANE_UNKNOWN_SHIFT_STATE)
	{
		p_lane_cnf->msg_lane->body.collector_id = 0L;
		p_lane_cnf->msg_lane->body.active_vault = 0L;
	}
	// if shift is closed, reset collector id and active vault
	else if (state == 1)
	{
		p_lane_cnf->msg_lane->body.collector_id = 0L;
	}
	// otherwise take collecotr id of the open lane
	else
	{		
		p_lane_cnf->msg_lane->body.collector_id = matricule;
		//p_lane_cnf->msg_lane->body.active_vault = vault;
	}

	// reset transaction date and time
	ZeroMemory(&p_lane_cnf->msg_lane->body.date, sizeof(p_lane_cnf->msg_lane->body.date));

	// reset payment type label
	ZeroMemory(p_lane_cnf->msg_lane->body.payment_type, sizeof(p_lane_cnf->msg_lane->body.payment_type));

	// reset currency label
	ZeroMemory(p_lane_cnf->msg_lane->body.currency, sizeof(p_lane_cnf->msg_lane->body.currency));

	// reset toll fare
	p_lane_cnf->msg_lane->body.tollfare = 0L;

	// reset vehicle class
	LANE_Set_Trs_Classes(p_lane_cnf, 0UL, 0UL, 0UL);

	// reset other list
	LANE_Reset_List_Other(p_lane_cnf);

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED DWORD LANE_Get_Shift(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->msg_lane->body.lane_state;
}

PROTECTED BOOL LANE_Set_Mode(struct LANE_CNF *p_lane_cnf, DWORD state)
{
	p_lane_cnf->msg_lane->body.lane_mode = state;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Set_Vault(struct LANE_CNF *p_lane_cnf, DWORD vault)
{
	if (p_lane_cnf->msg_lane->body.active_vault != vault)
	{
		p_lane_cnf->msg_lane->body.active_vault = vault;

		// mark the lane status as modified
		p_lane_cnf->lane_rep_altered = TRUE;
	}

	return TRUE;
}

PROTECTED BOOL LANE_Search_And_Set_Eqt(struct LANE_CNF *p_lane_cnf, DWORD dwEvent, DWORD sub_event, DWORD status)
{
	struct EQT_CNF *pEqtCnf;
	BOOL			ret = FALSE;

	pEqtCnf = List_GetHead(p_lane_cnf->list_eqt_cnf);
	while (pEqtCnf != NULL)
	{
		if (pEqtCnf->alarm_event == dwEvent &&
			pEqtCnf->alarm_sub_event == sub_event)
		{
			pEqtCnf->p_msg_eqt->state = status + 1;

			// mark the lane status as modified
			p_lane_cnf->lane_rep_altered = TRUE;

			ret = TRUE;
			break;
		}

		pEqtCnf = List_GetNext(p_lane_cnf->list_eqt_cnf, pEqtCnf);
	}

	return ret;
}

PROTECTED BOOL LANE_Reset_All_Eqt(struct LANE_CNF *p_lane_cnf)
{
	struct EQT_CNF *pEqtCnf	= NULL;
	BOOL			bRet	= TRUE;

	pEqtCnf = List_GetHead(p_lane_cnf->list_eqt_cnf);
	while (pEqtCnf != NULL)
	{
		pEqtCnf->p_msg_eqt->state = LANE_UNKNOWN_EQT_STATE;

		pEqtCnf = List_GetNext(p_lane_cnf->list_eqt_cnf, pEqtCnf);
	}

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

// MSG COUNTERS STATUS
PROTECTED BOOL LANE_Set_Traffic(struct LANE_CNF *p_lane_cnf, DWORD  daily_traffic, DWORD  daily_violation)
{
	BOOL bRet = TRUE;

	// copy counters
	p_lane_cnf->msg_lane->body.daily_traffic = daily_traffic;
	p_lane_cnf->msg_lane->body.daily_violation = daily_violation;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

PROTECTED BOOL LANE_Set_Daily_Traffic_Flag(struct LANE_CNF *p_lane_cnf, BOOL reset)
{
	p_lane_cnf->reset_daily_traffic = reset;

	return TRUE;
}

PROTECTED BOOL LANE_Get_Daily_Traffic_Flag(struct LANE_CNF *p_lane_cnf)
{
	return p_lane_cnf->reset_daily_traffic;
}

PROTECTED BOOL LANE_Init_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf)
{
	DWORD				dwErr				= 0;
	NTSVC_PARAMETER_DEF *psParams			= NULL;
	CHAR				szStatement[1024]	= { 0 };

	if (p_lane_cnf->init_yesterday_traffic == FALSE)
	{
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement), 
					"CounterP%dL%d", 
					p_lane_cnf->plaza_number, 
					p_lane_cnf->msg_lane->body.lanenum);
		
		psParams = NTSVCOpenParameters(szStatement, REG_DWORD, 4, 0, &p_lane_cnf->yesterday_traffic, NULL);
		if (psParams == NULL)
			return FALSE;

		if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
			return FALSE;

		p_lane_cnf->init_yesterday_traffic = TRUE;

		NTSVCCloseParameters(psParams);
	}

	return TRUE;
}

PROTECTED BOOL LANE_Set_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf)
{
	BOOL bRet = TRUE;

	// copy counters
	p_lane_cnf->yesterday_traffic = p_lane_cnf->intermediate_traffic;

	// mark as not saved
	p_lane_cnf->save_yesterday_traffic = TRUE;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

PROTECTED BOOL LANE_Save_Yesterday_Traffic(struct LANE_CNF *p_lane_cnf)
{
	DWORD				dwErr				= 0;
	NTSVC_PARAMETER_DEF	*psParams			= NULL;
	CHAR				szStatement[1024]	= { 0 };

	if (p_lane_cnf->save_yesterday_traffic == TRUE)
	{
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement), 
					"CounterP%dL%d", 
					p_lane_cnf->plaza_number, 
					p_lane_cnf->msg_lane->body.lanenum);

		psParams = NTSVCOpenParameters(szStatement, REG_DWORD, 4, 0, &p_lane_cnf->yesterday_traffic, NULL);
		if (psParams == NULL)
			return FALSE;

		if (NTSVCSaveParameters(psParams, &dwErr) != ERROR_SUCCESS)
			return FALSE;

		p_lane_cnf->save_yesterday_traffic = FALSE;

		NTSVCCloseParameters(psParams);
	}

	return TRUE;
}

PROTECTED BOOL LANE_Set_Intermediate_Traffic(struct LANE_CNF *p_lane_cnf)
{
	BOOL bRet = TRUE;

	// copy counters
	p_lane_cnf->intermediate_traffic = p_lane_cnf->msg_lane->body.daily_traffic;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

// MSG TRANSACTION
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
								 CHAR *pExlLicencePlate)
{
	BOOL bRet = TRUE;

	// copy collector id and active vault
	p_lane_cnf->msg_lane->body.collector_id = collector_id;
	p_lane_cnf->msg_lane->body.active_vault = active_vault;

	// copy date and time of the transaction
	p_lane_cnf->msg_lane->body.date = *message_date;

	// copy counters
	p_lane_cnf->msg_lane->body.daily_traffic = daily_traffic;
	p_lane_cnf->msg_lane->body.daily_violation = daily_violation;

	// get payment type label
	bRet &= PLAZA_Get_Payment(payment_type, payment_sub_type, p_lane_cnf->msg_lane->body.payment_type);

	// get currency label
	bRet &= PLAZA_Get_Currency(currency_id, p_lane_cnf->msg_lane->body.currency);

	// get toll fare
	p_lane_cnf->msg_lane->body.tollfare = tollfare;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	// copy licence plate
	strcpy_s(p_lane_cnf->msg_lane->body.exl_licence_plate, sizeof(p_lane_cnf->msg_lane->body.exl_licence_plate), pExlLicencePlate);

	return bRet;
}

PROTECTED BOOL LANE_Set_Counters(struct LANE_CNF *p_lane_cnf,
								 DWORD nb_display,
								 struct DISP_CNF display[LANE_MAX_DISPLAY])
{
	DWORD	dwDisplay	= 0;
	DWORD	dwCnt		= 0;
	BOOL	bRet		= TRUE;

	// browse all counters of all displays to test 
	// if the lane should be added to the calculations
	for (dwDisplay = 0; dwDisplay < nb_display; dwDisplay++)
	{
		for (dwCnt = 0; dwCnt < LANE_MAX_COUNTERS; dwCnt++)
		{
			if (p_lane_cnf->display[dwDisplay][dwCnt] == TRUE)
			{
				switch (display[dwDisplay].count[dwCnt])
				{
					case DISP_COUNT_NOTHING:
						break;

					case DISP_COUNT_TRAFFIC:
						display[dwDisplay].total[dwCnt] += p_lane_cnf->msg_lane->body.daily_traffic;
						break;

					case DISP_COUNT_VIOLATIONS:
						display[dwDisplay].total[dwCnt] += p_lane_cnf->msg_lane->body.daily_violation;
						break;

					case DISP_COUNT_UNACKED_ALARMS:
						display[dwDisplay].total[dwCnt] += p_lane_cnf->unacked_alarms;
						break;

					case DISP_COUNT_ALL_ALARMS:
						display[dwDisplay].total[dwCnt] += List_GetCount(p_lane_cnf->list_alarm);
						break;

					case DISP_COUNT_TRAFFIC_OF_YESTERDAY:
						if (LANE_Init_Yesterday_Traffic(p_lane_cnf) == TRUE && LANE_Save_Yesterday_Traffic(p_lane_cnf) == TRUE)
							display[dwDisplay].total[dwCnt] += p_lane_cnf->yesterday_traffic;
						break;

					default:
						bRet = FALSE;
						break;
				}
			}
		}
	}

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

PROTECTED BOOL LANE_Set_Trs_Classes(struct LANE_CNF *p_lane_cnf,
									DWORD transaction_class,
									DWORD detected_class,
									DWORD keyed_class)
{
	struct MSG_PV_LANE_REP_Class	*pClass;
	BOOL							bReturn		= TRUE;
	BOOL							bResetClass	= FALSE;

	// the 3 classes are stored in a list whose elements are pre-allocated in the init of LANE_CNF
	if ((transaction_class == 0UL) && (detected_class == 0UL) && (keyed_class == 0UL))
		bResetClass = TRUE;
	else
		bResetClass = FALSE;

	// it is defined that the first is the class used in the transaction
	pClass = MSG_PV_LANE_REP_Get_First_Class(p_lane_cnf->msg_lane->list_class);
	if (pClass == NULL)
	{
		bReturn = FALSE;
	}
	else if (bResetClass)
	{
		pClass->value = 0UL;
	}
	else if ((LANE_PARAM.dwClassOnTheFly1Position != 1) &&
			(LANE_PARAM.dwClassOnTheFly2Position != 1) &&
			(LANE_PARAM.dwClassReset1Position != 1) &&
			(LANE_PARAM.dwClassReset2Position != 1) &&
			(LANE_PARAM.dwClassKeyinPosition != 1))
	{
		pClass->value = transaction_class;
	}

	// the second one is detected class (eg. post-dac)
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bReturn = FALSE;
	}
	else if (bResetClass)
	{
		pClass->value = 0UL;
	}
	else if ((LANE_PARAM.dwClassOnTheFly1Position != 2) &&
			(LANE_PARAM.dwClassOnTheFly2Position != 2) &&
			(LANE_PARAM.dwClassReset1Position != 2) &&
			(LANE_PARAM.dwClassReset2Position != 2) &&
			(LANE_PARAM.dwClassKeyinPosition != 2))
	{
		pClass->value = detected_class;
	}

	// the 3rd one is keyed class
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bReturn = FALSE;
	}
	else if (bResetClass)
	{
		pClass->value = 0UL;
	}
	else if ((LANE_PARAM.dwClassOnTheFly1Position != 3) &&
			(LANE_PARAM.dwClassOnTheFly2Position != 3) &&
			(LANE_PARAM.dwClassReset1Position != 3) &&
			(LANE_PARAM.dwClassReset2Position != 3) &&
			(LANE_PARAM.dwClassKeyinPosition != 3))
	{
		pClass->value = keyed_class;
	}

	// the 4th class shold be reset to zero
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bReturn = FALSE;
	}
	else if (bResetClass)
	{
		pClass->value = 0UL;
	}
	else if ((LANE_PARAM.dwClassOnTheFly1Position != 4) &&
			(LANE_PARAM.dwClassOnTheFly2Position != 4) &&
			(LANE_PARAM.dwClassReset1Position != 4) &&
			(LANE_PARAM.dwClassReset2Position != 4) &&
			(LANE_PARAM.dwClassKeyinPosition != 4))
	{
		pClass->value = 0UL;
	}

	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
		bReturn = FALSE;
	else if (bResetClass)
		pClass->value = 0UL;
	else
		pClass->value = transaction_class;

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bReturn;
}

PROTECTED BOOL LANE_Set_Comp_Classes(struct LANE_CNF *p_lane_cnf, DWORD comp_class, DWORD comp_num)
{
	struct MSG_PV_LANE_REP_Class	*pClass	= NULL;
	BOOL							bRet	= TRUE;

	// the 3 classes are stored in a list whose elements are pre-allocated in the init of LANE_CNF

	// it is defined that the first is the class used in the transaction
	pClass = MSG_PV_LANE_REP_Get_First_Class(p_lane_cnf->msg_lane->list_class);
	if (pClass == NULL)
	{
		bRet = FALSE;
	}
	else if (((comp_num == CLASS_ONTHEFLY1_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly1Position == 1)) ||
		((comp_num == CLASS_ONTHEFLY2_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly2Position == 1)) ||
		((comp_num == CLASS_RESET1_SUBTYPE) && (LANE_PARAM.dwClassReset1Position == 1)) ||
		((comp_num == CLASS_RESET2_SUBTYPE) && (LANE_PARAM.dwClassReset2Position == 1)) /*||
		((comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE) && (LANE_PARAM.dwClassKeyinPosition == 1))*/)
	{
		pClass->value = comp_class;
	}
	else if (comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE &&
		((LANE_PARAM.dwClassReset1Position == 1) || (LANE_PARAM.dwClassReset2Position == 1)))
	{
		pClass->value = 0;
	}

	// the second one is detected class (eg. post-dac)
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bRet = FALSE;
	}
	else if (((comp_num == CLASS_ONTHEFLY1_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly1Position == 2)) ||
		((comp_num == CLASS_ONTHEFLY2_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly2Position == 2)) ||
		((comp_num == CLASS_RESET1_SUBTYPE) && (LANE_PARAM.dwClassReset1Position == 2)) ||
		((comp_num == CLASS_RESET2_SUBTYPE) && (LANE_PARAM.dwClassReset2Position == 2)) /*||
		((comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE) && (LANE_PARAM.dwClassKeyinPosition == 2))*/)
	{
		pClass->value = comp_class;
	}
	else if (comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE &&
		((LANE_PARAM.dwClassReset1Position == 2) || (LANE_PARAM.dwClassReset2Position == 2)))
	{
		pClass->value = 0;
	}

	// the 3rd one is keyed class
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bRet = FALSE;
	}
	else if (((comp_num == CLASS_ONTHEFLY1_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly1Position == 3)) ||
		((comp_num == CLASS_ONTHEFLY2_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly2Position == 3)) ||
		((comp_num == CLASS_RESET1_SUBTYPE) && (LANE_PARAM.dwClassReset1Position == 3)) ||
		((comp_num == CLASS_RESET2_SUBTYPE) && (LANE_PARAM.dwClassReset2Position == 3)) /*||
		((comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE) && (LANE_PARAM.dwClassKeyinPosition == 3))*/)
	{
		pClass->value = comp_class;
	}
	else if (comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE &&
		((LANE_PARAM.dwClassReset1Position == 3) || (LANE_PARAM.dwClassReset2Position == 3)))
	{
		pClass->value = 0;
	}

	// the 4th class shold be reset to zero
	pClass = MSG_PV_LANE_REP_Get_Next_Class(p_lane_cnf->msg_lane->list_class, pClass);
	if (pClass == NULL)
	{
		bRet = FALSE;
	}
	else if (((comp_num == CLASS_ONTHEFLY1_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly1Position == 4)) ||
		((comp_num == CLASS_ONTHEFLY2_SUBTYPE) && (LANE_PARAM.dwClassOnTheFly2Position == 4)) ||
		((comp_num == CLASS_RESET1_SUBTYPE) && (LANE_PARAM.dwClassReset1Position == 4)) ||
		((comp_num == CLASS_RESET2_SUBTYPE) && (LANE_PARAM.dwClassReset2Position == 4)) /*||
		((comp_num == CLASS_KEYIN_SUBTYPE) && (LANE_PARAM.dwClassKeyinPosition == 4))*/)
	{
		pClass->value = comp_class;
	}
	else if (comp_num == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE &&
		((LANE_PARAM.dwClassReset1Position == 4) || (LANE_PARAM.dwClassReset2Position == 4)))
	{
		pClass->value = 0;
	}

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return bRet;
}

PROTECTED BOOL LANE_Reset_Trs_Anomaly(struct LANE_CNF *p_lane_cnf)
{
	// delete list of the lane anomalies
	MSG_Delete_All_List(&p_lane_cnf->msg_lane->list_anomaly);

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Set_Trs_Anomaly(struct LANE_CNF *p_lane_cnf,
									DWORD anomaly_id,
									DWORD state,
									CHAR *pDescription)
{
	struct MSG_PV_LANE_REP_Anomaly	sAnomaly	= { 0 };
	struct MSG_PV_LANE_REP_Anomaly	*pAnomaly	= NULL;

	if (PLAZA_Get_Anomaly(anomaly_id, sAnomaly.label) == FALSE)
		return FALSE;

	// create the lane anomaly
	pAnomaly = MSG_PV_LANE_REP_Anomaly_New(&p_lane_cnf->msg_lane->list_anomaly);
	if (pAnomaly == NULL)
	{
		NTSVC_ERR("LANE_Set_Trs_Anomaly() => Error memory allocation: MSG_PV_LANE_REP_Anomaly_New()!");

		return FALSE;
	}

	sAnomaly.id = anomaly_id;

	strcpy_s(sAnomaly.description, sizeof(sAnomaly.description), pDescription);

	*pAnomaly = sAnomaly;

	return TRUE;
}

PROTECTED BOOL LANE_Reset_Trs_Id(struct LANE_CNF *p_lane_cnf)
{
	// delete list
	MSG_Delete_All_List(&p_lane_cnf->msg_lane->list_other);

	// mark the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Set_Trs_Id(struct LANE_CNF *p_lane_cnf, DWORD id)
{
	struct MSG_PV_LANE_REP_Other	sOther	= { 0 };
	struct MSG_PV_LANE_REP_Other	*pOther	= NULL;
	BOOL							bFlag	= FALSE;

	bFlag = FALSE;

	pOther = MSG_PV_LANE_REP_Get_First_Other(p_lane_cnf->msg_lane->list_other);
	if (pOther == NULL)
	{
		// create other item
		pOther = MSG_PV_LANE_REP_Other_New(&p_lane_cnf->msg_lane->list_other);
		if (pOther == NULL)
		{
			NTSVC_ERR("LANE_Set_Trs_Id() => Error memory allocation: MSG_PV_LANE_REP_Other_New()");

			return FALSE;
		}

		sOther.id = PV_CMPL_INFO_TRS_ID;
		_ltoa_s(id, sOther.label, sizeof(sOther.label), 10);

		*pOther = sOther;
		bFlag = TRUE;
	}
	else
	{
		while (pOther != NULL)
		{
			if (pOther->id == 1)
			{
				sOther.id = PV_CMPL_INFO_TRS_ID;
				_ltoa_s(id, sOther.label, sizeof(sOther.label), 10);

				*pOther = sOther;
				bFlag = TRUE;

				break;
			}

			pOther = MSG_PV_LANE_REP_Get_Next_Other(p_lane_cnf->msg_lane->list_other, pOther);
		}
	}

	if (!bFlag)
	{
		// create other item
		pOther = MSG_PV_LANE_REP_Other_New(&p_lane_cnf->msg_lane->list_other);
		if (pOther == NULL)
		{
			NTSVC_ERR("LANE_Set_Trs_Id() => Error memory allocation: MSG_PV_LANE_REP_Other_New()");

			return FALSE;
		}

		sOther.id = PV_CMPL_INFO_TRS_ID;
		_ltoa_s(id, sOther.label, sizeof(sOther.label), 10);

		*pOther = sOther;
	}

	return TRUE;
}

PRIVATE DWORD LANE_MappingInfoCode(DWORD dwType, DWORD dwCode)
{
	DWORD dwInfoCode = 0;

	// default value
	dwInfoCode = dwCode;

	// mapping code for PV (TST, LSDU) application
	switch (dwType)
	{
		case CMPL_INFO_MSG_TYPE_VEHICLE:
			switch (dwCode)
			{
				case CMPL_INFO_MSG_CODE_CLS_RESET_STYPE:
					dwInfoCode = PV_CMPL_INFO_CLS_KEYIN_STYPE;
					break;

				case CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE:
					dwInfoCode = PV_CMPL_INFO_CLS_KEYIN_STYPE;
					break;

				case CMPL_INFO_MSG_CODE_CLS_MOTO_A:
					dwInfoCode = PV_CMPL_INFO_CLS_MOTO_A;
					break;

				case CMPL_INFO_MSG_CODE_CLS_MOTO_B:
					dwInfoCode = PV_CMPL_INFO_CLS_MOTO_B;
					break;

				default:
					break;
			}
			break;

		case CMPL_INFO_MSG_TYPE_PV:
			switch (dwCode)
			{
				case CMPL_INFO_MSG_CODE_PV_FARE:
					dwInfoCode = PV_CMPL_INFO_PV_FARE;
					break;

				case CMPL_INFO_MSG_CODE_PV_PAYMENT_TYPE:
					dwInfoCode = PV_CMPL_INFO_PV_PAYMENT_TYPE;
					break;

				case CMPL_INFO_MSG_CODE_PV_CLASS:
					dwInfoCode = PV_CMPL_INFO_PV_CLASS;
					break;

				case CMPL_INFO_MSG_CODE_PV_TRAFFIC_LIGHT:
					dwInfoCode = PV_CMPL_INFO_PV_TRAFFIC_LIGHT;
					break;

				default:
					break;
			}
			break;

		case CMPL_INFO_MSG_TYPE_FARE_INFO:
			switch (dwCode)
			{
				case CMPL_INFO_MSG_CODE_FARE_AMOUNT_PAID:
					dwInfoCode = PV_CMPL_INFO_PV_AMOUNT_PAID;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	return dwInfoCode;
}

PRIVATE void LANE_FillOther(DWORD dwInfoCode,
							struct MSG_COMP_INF_TR_Elem_Info *pInfo,
							struct MSG_PV_LANE_REP_Other *pOther)
{
	pOther->id = dwInfoCode;

	if (pInfo->info_value.type == MSG_FIELD_VARSTR)
		sprintf_s(pOther->label, sizeof(pOther->label), "%s", pInfo->info_value.String);
	else if (pInfo->info_value.type == MSG_FIELD_FLOAT)
		sprintf_s(pOther->label, sizeof(pOther->label), "%010.2f", pInfo->info_value.Float);
	else if (pInfo->info_value.type == MSG_FIELD_DWORD)
		sprintf_s(pOther->label, sizeof(pOther->label), "%d", pInfo->info_value.Dword);
}

PROTECTED BOOL LANE_Set_List_Other_Element(struct LANE_CNF *p_lane_cnf,
										   DWORD dwType,
										   struct MSG_COMP_INF_TR_Elem_Info *p_lc_info)
{
	struct MSG_PV_LANE_REP_Other	*pOther		= NULL;
	DWORD							dwInfoCode	= 0;
	BOOL							bFlag		= FALSE;

	bFlag = FALSE;

	dwInfoCode = LANE_MappingInfoCode(dwType, p_lc_info->code);

	pOther = MSG_PV_LANE_REP_Get_First_Other(p_lane_cnf->msg_lane->list_other);
	if (pOther == NULL)
	{
		// create element in list
		pOther = MSG_PV_LANE_REP_Other_New(&p_lane_cnf->msg_lane->list_other);
		if (pOther == NULL)
		{
			NTSVC_ERR("LANE_Set_List_Other_Element() => Error memory allocation: MSG_PV_LANE_REP_Other_New()");
			return FALSE;
		}

		LANE_FillOther(dwInfoCode, p_lc_info, pOther);
		bFlag = TRUE;
	}
	else
	{
		while (pOther != NULL)
		{
			if (pOther->id == dwInfoCode)
			{
				LANE_FillOther(dwInfoCode, p_lc_info, pOther);
				bFlag = TRUE;
				break;
			}

			pOther = MSG_PV_LANE_REP_Get_Next_Other(p_lane_cnf->msg_lane->list_other, pOther);
		}
	}

	if (!bFlag)
	{
		// create element in list
		pOther = MSG_PV_LANE_REP_Other_New(&p_lane_cnf->msg_lane->list_other);
		if (pOther == NULL)
		{
			NTSVC_ERR("LANE_Set_List_Other_Element() => Error memory allocation: MSG_PV_LANE_REP_Other_New()");
			return FALSE;
		}

		LANE_FillOther(dwInfoCode, p_lc_info, pOther);
	}

	return TRUE;
}

PROTECTED BOOL LANE_Reset_List_Other(struct LANE_CNF *p_lane_cnf)
{
	// delete list
	MSG_Delete_All_List(&p_lane_cnf->msg_lane->list_other);

	// marke the lane status as modified
	p_lane_cnf->lane_rep_altered = TRUE;

	return TRUE;
}

PROTECTED BOOL LANE_Set_Enl_Plate(struct LANE_CNF *p_lane_cnf, HLIST entry_info)
{
	struct MSG_TRANSACTION_Entry_Info *p_entry_info = NULL;

	ZeroMemory(p_lane_cnf->msg_lane->body.enl_licence_plate, sizeof(p_lane_cnf->msg_lane->body.enl_licence_plate));

	p_entry_info = MSG_TRANSACTION_Get_First_Entry_Info(entry_info);
	if (p_entry_info == NULL)
		return FALSE;

	strcpy_s(p_lane_cnf->msg_lane->body.enl_licence_plate,
			sizeof(p_lane_cnf->msg_lane->body.enl_licence_plate),
			p_entry_info->entry_lic_plate);

	return TRUE;
}

// DATA BASE

PROTECTED struct LANE_CNF * LANE_Build_Conf_From_DB(HLIST hList,
													DWORD plaza_number,
													DWORD lane_number,
													DWORD nb_display,
													struct DISP_CNF display[LANE_MAX_DISPLAY])
{
	struct LANE_CNF						*p_lane_cnf		= NULL;
	struct MSG_PV_LANE_REP_Equipment	*p_eqt			= NULL;
	struct EQT_CNF						*pEqtCnf		= NULL;
	struct MSG_PV_LANE_REP_Class		*pClass			= NULL;
	DWORD								eqt_id			= 0;
	DWORD								alarm_event		= 0;
	DWORD								alarm_sub_event	= 0;
	DWORD								i				= 0;
	BOOL								ret;

	// creta one item in lane configuration list
	p_lane_cnf = List_ItemNew(hList, sizeof(struct LANE_CNF));
	if (p_lane_cnf == NULL)
		return NULL;

	// creta lane message
	p_lane_cnf->msg_lane = MSG_PV_LANE_REP_New();
	if (p_lane_cnf->msg_lane == NULL)
		return NULL;

	// get message id
	p_lane_cnf->msg_lane->header.id = SVC_Get_Msg_Id();

	// copy plaza number
	p_lane_cnf->plaza_number = plaza_number;

	// set configuration info
	p_lane_cnf->msg_lane->body.lanenum = lane_number;
	p_lane_cnf->msg_lane->body.lane_state = LANE_UNKNOWN_SHIFT_STATE;
	p_lane_cnf->reset_daily_traffic = FALSE;
	p_lane_cnf->init_yesterday_traffic = FALSE;
	p_lane_cnf->save_yesterday_traffic = FALSE;
	p_lane_cnf->yesterday_traffic = 0;
	p_lane_cnf->intermediate_traffic = 0;
	p_lane_cnf->lane_rep_altered = TRUE;

	// initialize alarm list
	p_lane_cnf->list_alarm = NULL;
	p_lane_cnf->list_alarm_altered = TRUE;

	// cretae list with equipment configuration
	if ((p_lane_cnf->list_eqt_cnf = List_New()) == NULL)
		return NULL;

	// get first equipment from DB
	ret = CONF_LANE_Get_First_Eqt(plaza_number, lane_number, &eqt_id, &alarm_event, &alarm_sub_event);
	while (ret == TRUE)
	{
		// allocate memory for equipment
		p_eqt = MSG_PV_LANE_REP_Equipment_New(&(p_lane_cnf->msg_lane->list_equipment));
		if (p_eqt == NULL)
			return NULL;

		// get equipment id
		p_eqt->id = eqt_id;

		// set equipment state to unknown
		p_eqt->state = LANE_UNKNOWN_EQT_STATE; 

		// allocate memory for equipment configuration
		pEqtCnf = List_ItemNew(hList, sizeof(struct EQT_CNF));
		if (pEqtCnf == NULL)
			return FALSE;

		// set eqipment configuration info
		pEqtCnf->alarm_event = alarm_event;
		pEqtCnf->alarm_sub_event = alarm_sub_event;
		pEqtCnf->p_msg_eqt = p_eqt;

		// add eqipment onfiguration to equipment list
		List_AddHead(p_lane_cnf->list_eqt_cnf, pEqtCnf);

		// get next equipment from DB
		ret = CONF_LANE_Get_Next_Eqt(&eqt_id, &alarm_event, &alarm_sub_event);
	}

	// cretae class list
	for (i = 0; i < 5; i++)
	{
		pClass = MSG_PV_LANE_REP_Class_New(&p_lane_cnf->msg_lane->list_class);
		if (pClass == NULL)
			return NULL;

		// set default value
		pClass->value = 0;
	}

	// set list of anomalies to NULL
	p_lane_cnf->msg_lane->list_anomaly = NULL;

	// get counters configuration
	for (i = 0; i < nb_display; i++)
	{
		ret = CONF_LANE_Get_Counter(plaza_number, lane_number, display[i].id, p_lane_cnf->display[i]);
		if (ret == FALSE)
			return NULL;
	}

	return p_lane_cnf;
}

PROTECTED BOOL LANE_Delete_All(HLIST hList, struct LANE_CNF *p_lane_cnf)
{
	// delete other list
	MSG_Delete_All_List(&p_lane_cnf->msg_lane->list_other);

	// delete equipment configuration list
	List_DeleteAll(&p_lane_cnf->list_eqt_cnf, NULL, NULL);

	// delete lane message
	MSG_PV_LANE_REP_Delete_All(p_lane_cnf->msg_lane);

	// delet lane configuration item from list
	List_ItemDelete(hList, &p_lane_cnf);

	// delet all alarms from list
	ALARM_Lane_Delete_All(p_lane_cnf);

	return TRUE;
}

/*-------------------------------- END OF FILE ------------------------------*/
