/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     plaza.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <ntsvc.h>
#include <srvpv_main.h>
#include <conf_srvpv.h>
#include <plaza.h>
#include <lane.h>
#include <alarm.h>
#include <pv_wm.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define PLAZA_REG_VAL_CLASS_FILTER			"FilterClass"
#define PLAZA_REG_VAL_PAYMENT_FILTER		"FilterPayment"
#define PLAZA_REG_VAL_PAYMENT_SUB_TYPE_USE  "PaymentSubTypeUse"
#define PLAZA_REG_VAL_TRANSACTION_DATE_USE	"TransactionDateUse"
#define PLAZA_REG_VAL_TRANSACTION_ID_USE	"TransactionIdUse"
#define PLAZA_REG_VAL_DEFAULT_PLAZA_NUMBER	"DefaultPlazaNumber"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct ANO_CNF
{
	DWORD id;
	CHAR  label[MSG_PV_MAX_ANOMALY_LABEL];
};

struct CUR_CNF
{
	DWORD id;
	CHAR  label[MSG_PV_MAX_CURRENCY_LABEL];
};

struct PAY_CNF
{
	DWORD id;
	CHAR  label[MSG_PV_MAX_PAYMENT_LABEL];

	HLIST list_pay_sub_type_cnf;
};

struct PAY_SUB_TYPE_CNF
{
	DWORD id;
	CHAR  label[MSG_PV_MAX_PAYMENT_LABEL];
};

struct CLASS_CNF
{
	DWORD id;
	CHAR label[MSG_PV_MAX_CLASS_LABEL];
};

struct COIN_CNF
{
	DWORD id;
	CHAR  label[MSG_PV_MAX_COIN_LABEL];
	DWORD currency_id;
	DOUBLE value;
	DWORD bag_count;
};

struct PLZ_CNF
{
	DWORD id;
	DWORD plaza_number;

	HLIST list_lane_cnf;

	DWORD nb_display;
	struct DISP_CNF display[LANE_MAX_DISPLAY];

	HLIST list_cur_cnf;

	HLIST list_pay_cnf;

	HLIST list_class_cnf;

	HLIST list_ano_cnf;

	HLIST list_coin_cnf;
};

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct PLZ_CNF PLZ_CNF = { 0 };

PRIVATE struct
{
	BOOL	bClassFilter;
	BOOL	bPaymentFilter;
	BOOL	bPaymentSubTypeUse;
	BOOL	bTransactionDateUse;
	BOOL	bTransactionIdUse;
	DWORD	dwDefaultPlazaNumber;
}
PLAZA_PARAM = { 0 };

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL PLAZA_Init(void)
{
	DWORD				dwErr = 0;
	NTSVC_PARAMETER_DEF	*psParams = NULL;

	// registry parameters
	psParams = NTSVCOpenParameters(
		PLAZA_REG_VAL_CLASS_FILTER,			REG_DWORD,	4,			0,		&PLAZA_PARAM.bClassFilter,
		PLAZA_REG_VAL_PAYMENT_FILTER,		REG_DWORD,	4,			0,		&PLAZA_PARAM.bPaymentFilter,
		PLAZA_REG_VAL_PAYMENT_SUB_TYPE_USE, REG_DWORD,	4,			0,		&PLAZA_PARAM.bPaymentSubTypeUse,
		PLAZA_REG_VAL_TRANSACTION_DATE_USE, REG_DWORD,	4,			0,		&PLAZA_PARAM.bTransactionDateUse,
		PLAZA_REG_VAL_TRANSACTION_ID_USE,	REG_DWORD,	4,			0,		&PLAZA_PARAM.bTransactionIdUse,
		PLAZA_REG_VAL_DEFAULT_PLAZA_NUMBER, REG_DWORD,	4,			0,		&PLAZA_PARAM.dwDefaultPlazaNumber,
		NULL);

	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	return TRUE;
}

PROTECTED struct LANE_CNF * PLAZA_Search_Lane(DWORD lane_number)
{
	struct LANE_CNF *p_lane_cnf = NULL;

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if (LANE_Get_Number(p_lane_cnf) == lane_number)
			break;

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return p_lane_cnf;
}


PROTECTED BOOL PLAZA_Search_For_Manual_Lane(void)
{
	struct LANE_CNF *p_lane_cnf = NULL;

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if ((p_lane_cnf->msg_lane->body.lane_state - 1 == LANE_NORMAL_SHIFT_OPEN) &&
			(p_lane_cnf->msg_lane->body.lane_mode - 1 == LANE_MANUAL_MODE))
			return TRUE;

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return FALSE;
}

PROTECTED struct LANE_CNF * PLAZA_Search_For_ActiveFree_Lane(void)
{
	struct LANE_CNF *p_lane_cnf = NULL;

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if ((p_lane_cnf->LaneActiveState == ACTIVE_FREE_STATE) &&
			(p_lane_cnf->msg_lane->body.lane_state - 1 == LANE_NORMAL_SHIFT_OPEN) &&
			(p_lane_cnf->msg_lane->body.lane_mode - 1 == LANE_MANUAL_MODE))
			break;

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return p_lane_cnf;
}

PROTECTED BOOL PLAZA_Search_For_Busy_Lane(void)
{
	struct LANE_CNF *p_lane_cnf = NULL;

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if ((p_lane_cnf->LaneActiveState == BUSY_STATE) &&
			(p_lane_cnf->msg_lane->body.lane_state - 1 == LANE_NORMAL_SHIFT_OPEN) &&
			(p_lane_cnf->msg_lane->body.lane_mode - 1 == LANE_MANUAL_MODE))
			return TRUE;

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return FALSE;
}

PROTECTED DWORD PLAZA_Get_Number(void)
{
	return PLZ_CNF.plaza_number;
}

PROTECTED DWORD PLAZA_Count_Lanes(void)
{
	return List_GetCount(PLZ_CNF.list_lane_cnf);
}

PROTECTED BOOL PLAZA_Get_Currency(DWORD currency_id, CHAR label[MSG_PV_MAX_CURRENCY_LABEL])
{
	struct CUR_CNF *p_cur_cnf;

	label[0] = '\0';

	p_cur_cnf = List_GetHead(PLZ_CNF.list_cur_cnf);
	while (p_cur_cnf != NULL)
	{
		if (p_cur_cnf->id == currency_id)
			break;

		p_cur_cnf = List_GetNext(PLZ_CNF.list_cur_cnf, p_cur_cnf);
	}

	if (p_cur_cnf == NULL)
		return FALSE;

	// copy label
	strzcpy(label, MSG_PV_MAX_CURRENCY_LABEL, p_cur_cnf->label, MSG_PV_MAX_CURRENCY_LABEL);

	return TRUE;
}

PROTECTED BOOL PLAZA_Get_Payment_Filter(void)
{
	return PLAZA_PARAM.bPaymentFilter;
}

PROTECTED BOOL PLAZA_Get_Payment_Sub_Type_Use(void)
{
	return PLAZA_PARAM.bPaymentSubTypeUse;
}

PROTECTED BOOL PLAZA_Get_Transaction_Date_Use(void)
{
	return PLAZA_PARAM.bTransactionDateUse;
}

PROTECTED BOOL PLAZA_Get_Transaction_Id_Use(void)
{
	return PLAZA_PARAM.bTransactionIdUse;
}

PROTECTED BOOL PLAZA_Get_Payment(DWORD payment_id, DWORD payment_sub_id, CHAR label[MSG_PV_MAX_PAYMENT_LABEL])
{
	struct PAY_CNF			*p_pay_cnf			= NULL;
	struct PAY_SUB_TYPE_CNF	*p_pay_sub_type_cnf	= NULL;

	label[0] = '\0';

	p_pay_cnf = List_GetHead(PLZ_CNF.list_pay_cnf);
	while (p_pay_cnf != NULL)
	{
		if (p_pay_cnf->id == payment_id)
			break;

		p_pay_cnf = List_GetNext(PLZ_CNF.list_pay_cnf, p_pay_cnf);
	}

	if (p_pay_cnf == NULL)
		return FALSE;

	if (payment_sub_id != 0)
	{
		p_pay_sub_type_cnf = List_GetHead(p_pay_cnf->list_pay_sub_type_cnf);

		while (p_pay_sub_type_cnf != NULL)
		{
			if (p_pay_sub_type_cnf->id == payment_sub_id)
				break;

			p_pay_sub_type_cnf = List_GetNext(p_pay_cnf->list_pay_sub_type_cnf, p_pay_sub_type_cnf);
		}

		if (p_pay_sub_type_cnf == NULL)
			return FALSE;

		// copy label
		strzcpy(label, MSG_PV_MAX_PAYMENT_LABEL, p_pay_sub_type_cnf->label, MSG_PV_MAX_PAYMENT_LABEL);
	}
	else
		strzcpy(label, MSG_PV_MAX_PAYMENT_LABEL, p_pay_cnf->label, MSG_PV_MAX_PAYMENT_LABEL);

	return TRUE;
}

PROTECTED BOOL PLAZA_Get_Class_Filter(void)
{
	return PLAZA_PARAM.bClassFilter;
}

PROTECTED BOOL PLAZA_Get_Class(DWORD class_id, CHAR label[MSG_PV_MAX_CLASS_LABEL])
{
	struct CLASS_CNF *p_class_cnf = NULL;

	label[0] = '\0';

	p_class_cnf = List_GetHead(PLZ_CNF.list_class_cnf);
	while (p_class_cnf != NULL)
	{
		if (p_class_cnf->id == class_id)
			break;

		p_class_cnf = List_GetNext(PLZ_CNF.list_class_cnf, p_class_cnf);
	}

	if (p_class_cnf == NULL)
		return FALSE;

	// copy label
	strzcpy(label, MSG_PV_MAX_CLASS_LABEL, p_class_cnf->label, MSG_PV_MAX_CLASS_LABEL);

	return TRUE;
}

PROTECTED BOOL PLAZA_Get_Anomaly(DWORD anomaly_id, CHAR label[MSG_PV_MAX_ANOMALY_LABEL])
{
	struct ANO_CNF *p_ano_cnf = NULL;

	label[0] = '\0';

	p_ano_cnf = List_GetHead(PLZ_CNF.list_ano_cnf);
	while (p_ano_cnf != NULL)
	{
		if (p_ano_cnf->id == anomaly_id)
			break;

		p_ano_cnf = List_GetNext(PLZ_CNF.list_ano_cnf, p_ano_cnf);
	}

	if (p_ano_cnf == NULL)
		return FALSE;

	// copy label
	strzcpy(label, MSG_PV_MAX_ANOMALY_LABEL, p_ano_cnf->label, MSG_PV_MAX_ANOMALY_LABEL);

	return TRUE;
}

PROTECTED BOOL PLAZA_Get_Coin(IN DWORD coin_id,
							  OUT CHAR currency_label[MSG_PV_MAX_CURRENCY_LABEL],
							  OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
							  OUT DOUBLE *value,
							  OUT DWORD *bag)
{
	struct COIN_CNF *p_coin_cnf = NULL;

	currency_label[0] = '\0';
	coin_label[0] = '\0';
	*value = 0.0;
	*bag = 0;

	p_coin_cnf = List_GetHead(PLZ_CNF.list_coin_cnf);
	while (p_coin_cnf != NULL)
	{
		if (p_coin_cnf->id == coin_id)
			break;

		p_coin_cnf = List_GetNext(PLZ_CNF.list_coin_cnf, p_coin_cnf);
	}

	if (p_coin_cnf == NULL)
		return FALSE;

	// copy info
	strzcpy(coin_label, MSG_PV_MAX_COIN_LABEL, p_coin_cnf->label, MSG_PV_MAX_COIN_LABEL);
	*value = p_coin_cnf->value;
	*bag = p_coin_cnf->bag_count;

	// get currency label
	if (PLAZA_Get_Currency(p_coin_cnf->currency_id, currency_label) == FALSE)
		return FALSE;

	return TRUE;
}

PROTECTED BOOL PLAZA_Reset_Traffic_Counters_For_All_Disconnected_Lanes(struct LANE_CNF *p_lane_cnf, BOOL reset)
{
	BOOL bRet = FALSE;

	// if it is not a reset of the counters following a passage at zero hour
	// there is nothing to do!
	if (reset == FALSE)
		return TRUE;

	// set daily trafic reset flag to TRUE
	LANE_Set_Daily_Traffic_Flag(p_lane_cnf, TRUE);

	// the intermediate traffic j-1 is saved
	LANE_Set_Intermediate_Traffic(p_lane_cnf);

	// then we check if all the connected lanes have this flag mounted ...
	bRet = TRUE;
	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if (LANE_Get_Connection_State(p_lane_cnf) && !LANE_Get_Daily_Traffic_Flag(p_lane_cnf))
		{
			bRet = FALSE;
			break;
		}

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	// ... if this is the case and for all lanes
	if (bRet == TRUE)
	{
		p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
		while (p_lane_cnf != NULL)
		{
			if (!LANE_Get_Connection_State(p_lane_cnf) && !LANE_Get_Daily_Traffic_Flag(p_lane_cnf))
			{
				LANE_Set_Intermediate_Traffic(p_lane_cnf);
				LANE_Set_Traffic(p_lane_cnf, 0UL, 0UL);
			}

			//  and set this flag to FALSE for ALL channels!
			LANE_Set_Daily_Traffic_Flag(p_lane_cnf, FALSE);

			// and the traffic j-1 is fixed for all the channels
			LANE_Set_Yesterday_Traffic(p_lane_cnf);

			p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
		}
	}

	return TRUE;
}

PROTECTED BOOL PLAZA_Disconnect_All_Lanes(ACOM_CNX_HANDLE hCnxHandle)
{
	struct LANE_CNF *p_lane_cnf = NULL;

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		if (LANE_Get_Connection_Handle(p_lane_cnf) == hCnxHandle)
			LANE_Set_Disconnection(p_lane_cnf);

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return TRUE;
}

PROTECTED BOOL PLAZA_Set_Lane_Connexion_State(struct MSG_SV_CON_REP *p_con, ACOM_CNX_HANDLE hCnxHandle)
{
	struct LANE_CNF *p_lane	= NULL;
	BOOL			ret		= TRUE;

	p_lane = PLAZA_Search_Lane(p_con->body.lanenum);
	if (p_lane == NULL)
		return FALSE;

	if (SVC_Is_Link_Status_Cheking_Used())
		CheckLaneLinkStatus(p_con->body.lanenum, p_con->body.state);

	if (p_con->body.state == TRUE)
		return LANE_Set_Connection(p_lane, hCnxHandle);

	return LANE_Set_Disconnection(p_lane);
}

PROTECTED BOOL PLAZA_Build_Counters(void)
{
	struct LANE_CNF *p_lane_cnf	= NULL;
	DWORD			i			= 0;
	BOOL			bRet		= TRUE;

	for (i = 0; i < PLZ_CNF.nb_display; i++)
		memset(PLZ_CNF.display[i].total, 0, sizeof(PLZ_CNF.display[i].total));

	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL && bRet == TRUE)
	{
		bRet &= LANE_Set_Counters(p_lane_cnf, PLZ_CNF.nb_display, PLZ_CNF.display);

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	return bRet;
}

PROTECTED struct MSG_PV_PLAZ_REP *PLAZA_Build_msg_pv_plz_rep(BOOL partial)
{
	struct MSG_PV_PLAZ_REP *msg_plz;
	struct MSG_PV_LANE_REP *msg_pv_lane_rep;
	struct MSG_PV_ALAR_REP *msg_pv_alar_rep;
	struct LANE_CNF *p_lane_cnf;
	struct MSG_PV_PLAZ_REP_Lane *p_lane;
	struct MSG_PV_PLAZ_REP_Alarm *p_alarm;
	struct MSG_PV_PLAZ_REP_Display * p_disp;
	DWORD i;
	BOOL altered = FALSE;

	// allocate memory for the plaza message
	msg_plz = MSG_PV_PLAZ_REP_New();
	if (msg_plz == NULL)
		return NULL;

	// get mesage id and plaza number
	msg_plz->header.id = SVC_Get_Msg_Id();
	msg_plz->body.plazanum = PLZ_CNF.plaza_number;

	// for all lanes for this plaza
	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		// allocate message for lane status
		msg_pv_lane_rep = LANE_Build_msg_pv_lane_rep(p_lane_cnf, partial);
		if (msg_pv_lane_rep != NULL)
		{
			// allocate an element of lane list
			p_lane = MSG_PV_PLAZ_REP_Lane_New(&msg_plz->list_lane);
			if (p_lane == NULL)
			{
				NTSVC_ERR("PLAZA_Build_msg_pv_plz_rep() => Error memory allocation: MSG_PV_PLAZ_REP_Lane_New()!");

				altered = FALSE;
				break;
			}

			// recopy the lane status message
			p_lane->msg_pv_lane_rep = msg_pv_lane_rep;

			// at least one change ...
			altered = TRUE;
		}

		// create list of alarms for this lane
		msg_pv_alar_rep = ALARM_Build_msg_pv_alar_rep(p_lane_cnf, partial);
		if (msg_pv_alar_rep != NULL)
		{
			// allocate an element of list
			p_alarm = MSG_PV_PLAZ_REP_Alarm_New(&msg_plz->list_alarm);
			if (p_alarm == NULL)
			{
				NTSVC_ERR("PLAZA_Build_msg_pv_plz_rep() => Error memory allocation: MSG_PV_PLAZ_REP_Alarm_New()!");

				altered = FALSE;
				break;
			}

			// recopy alarms reply message
			p_alarm->msg_pv_alar_rep = msg_pv_alar_rep;

			// at least one change ...
			altered = TRUE;
		}

		// get next lane
		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	// if at least one change
	if (altered == TRUE)
	{
		// update counters
		if (PLAZA_Build_Counters() == FALSE)
			NTSVC_ERR("PLAZA_Build_msg_pv_plz_rep() => error execute: PLAZA_Build_Counters()!");

		// Aad counters to the status message
		for (i = 0; i < PLZ_CNF.nb_display; i++)
		{
			p_disp = MSG_PV_PLAZ_REP_Display_New(&msg_plz->list_display);
			p_disp->id = PLZ_CNF.display[i].id;
			p_disp->total1 = PLZ_CNF.display[i].total[0];
			p_disp->total2 = PLZ_CNF.display[i].total[1];
			p_disp->total3 = PLZ_CNF.display[i].total[2];
			p_disp->total4 = PLZ_CNF.display[i].total[3];
			p_disp->total5 = PLZ_CNF.display[i].total[4];
		}
	}
	else
	{
		// ff no change, the message is released
		MSG_PV_PLAZ_REP_Delete_All(msg_plz);
		msg_plz = NULL;
	}

	return msg_plz;
}

PROTECTED BOOL PLAZA_Build_Conf_From_DB(void)
{
	struct LANE_CNF			*p_lane_cnf			= NULL;
	struct CUR_CNF			cur					= { 0 };
	struct CUR_CNF			*p_cur_cnf			= NULL;
	struct PAY_CNF			pay					= { 0 };
	struct PAY_CNF			*p_pay_cnf			= NULL;
	struct PAY_SUB_TYPE_CNF pay_sub_type		= { 0 };
	struct PAY_SUB_TYPE_CNF *p_pay_sub_type_cnf	= NULL;
	struct CLASS_CNF		cla					= { 0 };
	struct CLASS_CNF		*p_class_cnf		= NULL;
	struct ANO_CNF			ano					= { 0 };
	struct ANO_CNF			*p_ano_cnf			= NULL;
	struct COIN_CNF			coin				= { 0 };
	struct COIN_CNF			*p_coin_cnf			= NULL;
	DWORD					lane_number			= 0;
	BOOL					ret					= TRUE;
	HLIST					hLinkStatus			= NULL;
	struct_link_status		*pItem				= NULL;

	// init plaza
	ret = PLAZA_Init();

	// get plaza number
	ret = CONF_PLAZA_Get_First_Plaza(&PLZ_CNF.plaza_number, PLAZA_PARAM.dwDefaultPlazaNumber);
	if (ret == FALSE)
		return FALSE;

	PLZ_CNF.nb_display = 0;
	// get first display
	ret = CONF_PLAZA_Get_First_Display(PLZ_CNF.plaza_number, 
									&PLZ_CNF.display[PLZ_CNF.nb_display].id, 
									PLZ_CNF.display[PLZ_CNF.nb_display].count);
	while (ret)
	{
		PLZ_CNF.nb_display++;

		// get next display
		ret = CONF_PLAZA_Get_Next_Display(&PLZ_CNF.display[PLZ_CNF.nb_display].id, 
										PLZ_CNF.display[PLZ_CNF.nb_display].count);
	}

	// allocate memory for lane list
	PLZ_CNF.list_lane_cnf = List_New();
	if (PLZ_CNF.list_lane_cnf == NULL)
		return FALSE;

	// allocate memory for link status list
	hLinkStatus = LaneLinkStatusListNew();
	if (hLinkStatus == NULL)
		return FALSE;

	ret = LANE_Init();

	// get first lane
	ret = CONF_PLAZA_Get_First_Lane(PLZ_CNF.plaza_number, &lane_number);
	while (ret == TRUE)
	{
		// get lane configuration from DB
		p_lane_cnf = LANE_Build_Conf_From_DB(PLZ_CNF.list_lane_cnf, PLZ_CNF.plaza_number, lane_number, PLZ_CNF.nb_display, PLZ_CNF.display);
		if (p_lane_cnf == NULL)
			return FALSE;

		// add an item to the list of lanes
		List_AddTail(PLZ_CNF.list_lane_cnf, p_lane_cnf);

		// create and add an item tt the link status list
		pItem = (struct_link_status *)List_ItemNew(hLinkStatus, sizeof(struct_link_status));
		memset(pItem, 0, sizeof(struct_link_status));
		List_AddTail(hLinkStatus, pItem);
		sprintf_s(pItem->szName, sizeof(pItem->szName), "%d", lane_number);

		// get next lane
		ret = CONF_PLAZA_Get_Next_Lane(&lane_number);
	}

	// Create the list of alarms for this plaza number
	if (ALARM_Build_Conf_From_DB(PLZ_CNF.plaza_number) == FALSE)
		return FALSE;

	// allocate memory for the currency list
	PLZ_CNF.list_cur_cnf = List_New();
	if (PLZ_CNF.list_cur_cnf == NULL)
		return FALSE;

	ret = CONF_PLAZA_Get_First_Currency(&cur.id, cur.label);
	while (ret == TRUE)
	{
		// create an item element in the list
		p_cur_cnf = List_ItemNew(PLZ_CNF.list_cur_cnf, sizeof(struct CUR_CNF));
		if (p_cur_cnf == NULL)
			return FALSE;

		*p_cur_cnf = cur;

		// add an item to the currenncy list
		List_AddTail(PLZ_CNF.list_cur_cnf, p_cur_cnf);

		// get next currency
		ret = CONF_PLAZA_Get_Next_Currency(&cur.id, cur.label);
	}

	// allocate memory for payment type list
	PLZ_CNF.list_pay_cnf = List_New();
	if (PLZ_CNF.list_pay_cnf == NULL)
		return FALSE;

	ret = CONF_PLAZA_Get_First_Payment(&pay.id, pay.label);
	while (ret == TRUE)
	{
		// if payment sub-type in use
		if (PLAZA_Get_Payment_Sub_Type_Use() == TRUE)
		{
			// allocate memory for payment sub-type list
			pay.list_pay_sub_type_cnf = List_New();
			if (pay.list_pay_sub_type_cnf == NULL)
				return FALSE;

			ret = CONF_PLAZA_Get_First_Payment_Sub_Type(pay.id, &pay_sub_type.id, pay_sub_type.label);
			while (ret == TRUE)
			{
				// create an item element in the list
				p_pay_sub_type_cnf = List_ItemNew(pay.list_pay_sub_type_cnf, sizeof(struct PAY_SUB_TYPE_CNF));
				if (p_pay_sub_type_cnf == NULL)
					return FALSE;

				*p_pay_sub_type_cnf = pay_sub_type;

				// add an item to the payment sub-type list
				List_AddTail(pay.list_pay_sub_type_cnf, p_pay_sub_type_cnf);

				// get next payment sub-type
				ret = CONF_PLAZA_Get_Next_Payment_Sub_Type(&pay_sub_type.id, pay_sub_type.label);
			}
		}

		// create an item element in the list
		p_pay_cnf = List_ItemNew(PLZ_CNF.list_pay_cnf, sizeof(struct PAY_CNF));
		if (p_pay_cnf == NULL)
			return FALSE;

		*p_pay_cnf = pay;

		// add an item to the payment type list
		List_AddTail(PLZ_CNF.list_pay_cnf, p_pay_cnf);

		// get next payment type
		ret = CONF_PLAZA_Get_Next_Payment(&pay.id, pay.label);
	}

	// allocate memory for the class list
	PLZ_CNF.list_class_cnf = List_New();
	if (PLZ_CNF.list_class_cnf == NULL)
		return FALSE;

	if (PLAZA_Get_Class_Filter() == TRUE)
	{
		ret = CONF_PLAZA_Get_First_Class(&cla.id, cla.label);
		while (ret == TRUE)
		{
			// create an item element in the list
			p_class_cnf = List_ItemNew(PLZ_CNF.list_class_cnf, sizeof(struct CLASS_CNF));
			if (p_class_cnf == NULL)
				return FALSE;

			*p_class_cnf = cla;

			// add an item to the class list
			List_AddTail(PLZ_CNF.list_class_cnf, p_class_cnf);

			// get next class
			ret = CONF_PLAZA_Get_Next_Class(&cla.id, cla.label);
		}
	}

	// allocate memory for the anomaly list
	PLZ_CNF.list_ano_cnf = List_New();
	if (PLZ_CNF.list_ano_cnf == NULL)
		return FALSE;

	ret = CONF_PLAZA_Get_First_Anomaly(&ano.id, ano.label);
	while (ret == TRUE)
	{
		// create an item element in the list
		p_ano_cnf = List_ItemNew(PLZ_CNF.list_ano_cnf, sizeof(struct ANO_CNF));
		if (p_ano_cnf == NULL)
			return FALSE;

		*p_ano_cnf = ano;

		// add an item to the anomaly list
		List_AddTail(PLZ_CNF.list_ano_cnf, p_ano_cnf);

		// get next anomaly
		ret = CONF_PLAZA_Get_Next_Anomaly(&ano.id, ano.label);
	}

	// allocate memory for the coin list
	PLZ_CNF.list_coin_cnf = List_New();
	if (PLZ_CNF.list_coin_cnf == NULL)
		return FALSE;

	ret = CONF_PLAZA_Get_First_Coin(&coin.id, &coin.currency_id, coin.label, &coin.value, &coin.bag_count);
	while (ret == TRUE)
	{
		// create an item element in the list
		p_coin_cnf = List_ItemNew(PLZ_CNF.list_coin_cnf, sizeof(struct COIN_CNF));
		if (p_coin_cnf == NULL)
			return FALSE;

		*p_coin_cnf = coin;

		// add an item to the coin list
		List_AddTail(PLZ_CNF.list_coin_cnf, p_coin_cnf);

		// get next coin
		ret = CONF_PLAZA_Get_Next_Coin(&coin.id, &coin.currency_id, coin.label, &coin.value, &coin.bag_count);
	}

	// always returns FALSE but allows to terminate the connection with the database
	CONF_PLAZA_Get_Next_Plaza(&PLZ_CNF.plaza_number, PLAZA_PARAM.dwDefaultPlazaNumber);

	return TRUE;
}

PROTECTED BOOL PLAZA_Delete_All(void)
{
	struct LANE_CNF *p_lane_cnf;

	// remove all lanes from the plaza
	p_lane_cnf = List_GetHead(PLZ_CNF.list_lane_cnf);
	while (p_lane_cnf != NULL)
	{
		LANE_Delete_All(PLZ_CNF.list_lane_cnf, p_lane_cnf);

		p_lane_cnf = List_GetNext(PLZ_CNF.list_lane_cnf, p_lane_cnf);
	}

	// delet list of lanes
	List_DeleteAll(&PLZ_CNF.list_lane_cnf, NULL, NULL);

	// delet alarms list
	ALARM_Delete_All();

	// delete list of currencies
	List_DeleteAll(&PLZ_CNF.list_cur_cnf, NULL, NULL);

	// delete list of payment types
	List_DeleteAll(&PLZ_CNF.list_pay_cnf, NULL, NULL);

	// delete list of anomalies
	List_DeleteAll(&PLZ_CNF.list_ano_cnf, NULL, NULL);

	// delete list of coins
	List_DeleteAll(&PLZ_CNF.list_coin_cnf, NULL, NULL);

	// delete list of classes 
	List_DeleteAll(&PLZ_CNF.list_class_cnf, NULL, NULL);

	// delete link status list
	LaneLinkStatusListDelete();

	//and currently changed link status to send
	Delete_LinkStatusList();

	return TRUE;
}

/*-------------------------------- END OF FILE ------------------------------*/