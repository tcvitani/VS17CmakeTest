/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: alarm.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     alarm.c														 */
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
#include <ntsvc.h>
#include <alarm.h>
#include <conf_srvpv.h>
#include <alarm_aut.h>
#include <srvpv_main.h>
#include <plaza.h>
#include <db_wm.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define ALARM_REG_VAL_TEST_DATE				"TestAlarmDate"
#define ALARM_REG_VAL_LANE_COUNTER			"LaneAlarmCounterType"
#define ALARM_REG_VAL_TEST_ALARM_ACK_TYPE	"TestAlarmAckType"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct EVENT_CNF
{
	DWORD id;

	aut_alarm_mode ack_mode;

	DWORD level;

	CHAR wording[MSG_PV_MAX_ALARM_LABEL];
	CHAR directive[MSG_PV_MAX_DETAIL_SIZE];
};

struct ALARM_CNF
{
	DWORD type;
	DWORD sub_type;

	HLIST list_event_cnf;
};

struct STATUS_CNF
{
	HLIST list_alarm_cnf;

	// shift and mode
	DWORD pShiftAndModeEvtType;
	DWORD pShiftEvtSubType;
	DWORD pModeEvtSubType;

	// vaults
	DWORD pVault1EvtType;
	DWORD pVault2EvtType;

	BOOL bTestDate;

	DWORD LaneAlarmCounterType;

	BOOL bTestAlarmAckType;
};

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct STATUS_CNF STATUS_CNF = { 0 };

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PRIVATE BOOL ALARM_Init(void)
{
	DWORD				dwErr		= 0;
	NTSVC_PARAMETER_DEF	*psParams	= NULL;

	// registry parameters
	psParams = NTSVCOpenParameters(
		ALARM_REG_VAL_TEST_DATE,			REG_DWORD,	4,	0,	&STATUS_CNF.bTestDate,
		ALARM_REG_VAL_LANE_COUNTER,			REG_DWORD,	4,	1,	&STATUS_CNF.LaneAlarmCounterType,
		ALARM_REG_VAL_TEST_ALARM_ACK_TYPE,	REG_DWORD,	4,	0,	&STATUS_CNF.bTestAlarmAckType,
		NULL);

	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	return TRUE;
}

PROTECTED BOOL ALARM_Build_Conf_From_DB(DWORD plaza_number)
{
	struct ALARM_CNF	alarm_cnf		= { 0 };
	struct EVENT_CNF	event_cnf		= { 0 };
	struct ALARM_CNF	*p_alarm_cnf	= NULL;
	struct EVENT_CNF	*p_event_cnf	= NULL;
	BOOL				ret				= FALSE;

	// initialize registry parameters
	if (ALARM_Init() == FALSE)
		return FALSE;

	// get shift, mode and vault event types
	CONF_ALARM_Get_Shift_Mode_And_Vault_Events(&STATUS_CNF.pShiftAndModeEvtType,
											&STATUS_CNF.pShiftEvtSubType,
											&STATUS_CNF.pModeEvtSubType,
											&STATUS_CNF.pVault1EvtType,
											&STATUS_CNF.pVault2EvtType);

	// create alarms list
	if ((STATUS_CNF.list_alarm_cnf = List_New()) == NULL)
		return FALSE;

	// get alarms configuration from database/datafile
	ret = CONF_ALARM_Get_First_Alarm(&alarm_cnf.type, &alarm_cnf.sub_type);
	while (ret == TRUE)
	{
		p_alarm_cnf = List_ItemNew(STATUS_CNF.list_alarm_cnf, sizeof(struct ALARM_CNF));
		if (p_alarm_cnf == NULL)
			return FALSE;

		*p_alarm_cnf = alarm_cnf;
		List_AddHead(STATUS_CNF.list_alarm_cnf, p_alarm_cnf);

		if ((p_alarm_cnf->list_event_cnf = List_New()) == NULL)
			return FALSE;

		// search for basic information for the first event of this alarm
		ret = CONF_ALARM_Get_First_Event(alarm_cnf.type,
										alarm_cnf.sub_type,
										&event_cnf.id,
										&(DWORD)event_cnf.ack_mode,
										&event_cnf.level,
										event_cnf.wording,
										event_cnf.directive);

		while (ret)
		{
			p_event_cnf = List_ItemNew(p_alarm_cnf->list_event_cnf, sizeof(struct EVENT_CNF));
			if (p_event_cnf == NULL)
				return FALSE;

			*p_event_cnf = event_cnf;
			List_AddHead(p_alarm_cnf->list_event_cnf, p_event_cnf);

			ret = CONF_ALARM_Get_Next_Event(&event_cnf.id,
											&(DWORD)event_cnf.ack_mode,
											&event_cnf.level,
											event_cnf.wording,
											event_cnf.directive);
		}

		ret = CONF_ALARM_Get_Next_Alarm(&alarm_cnf.type, &alarm_cnf.sub_type);
	}

	return TRUE;
}

PROTECTED BOOL ALARM_Set_Lane_Shift_And_Mode(struct LANE_CNF *p_lane_cnf,
											 DWORD matricule,
											 DWORD vault,
											 DWORD type,
											 DWORD sub_type,
											 DWORD id)
{
	BOOL ret = FALSE;

	// if the shift related event received update the shift status
	if (type == STATUS_CNF.pShiftAndModeEvtType && sub_type == STATUS_CNF.pShiftEvtSubType)
	{
		// set shift status (closed, normal, maintenace, ...)
		ret |= LANE_Set_Shift(p_lane_cnf, matricule, vault, id + 1);
	}

	// if the mode related event received update the mode status
	if (type == STATUS_CNF.pShiftAndModeEvtType && sub_type == STATUS_CNF.pModeEvtSubType)
	{
		// set mode status (manual, automatic, free, ...)
		ret |= LANE_Set_Mode(p_lane_cnf, id + 1);
	}

	return ret;
}

PROTECTED BOOL ALARM_Set_Lane_Active_Vault(struct LANE_CNF *p_lane_cnf, DWORD vault, DWORD type)
{
	BOOL ret = FALSE;

	// if the vault related event received update active vault id
	if (type == STATUS_CNF.pVault1EvtType || type == STATUS_CNF.pVault2EvtType)
	{
		// set active vault id
		ret |= LANE_Set_Vault(p_lane_cnf, vault);
	}

	return ret;
}

PROTECTED BOOL ALARM_Search_And_Set_List(struct LANE_CNF *p_lane_cnf,
										DWORD type,
										DWORD sub_type,
										DWORD id,
										LPSYSTEMTIME event_time,
										CHAR *pDescription)
{
	HLIST *p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);
	struct ALARM_CNF *p_alar_cnf;
	struct EVENT_CNF *p_event_cnf;
	aut_alarm_id aut_alarm;
	struct MSG_PV_ALAR_REP_Event *p_event;
	struct ARG_EVT_ALARM_ON arg_alarm_on;
	BOOL ret = FALSE;
	BOOL bFound;

	p_alar_cnf = List_GetHead(STATUS_CNF.list_alarm_cnf);
	while (p_alar_cnf != NULL)
	{
		if (p_alar_cnf->type == type && p_alar_cnf->sub_type == sub_type)
			break;

		p_alar_cnf = List_GetNext(STATUS_CNF.list_alarm_cnf, p_alar_cnf);
	}

	if (p_alar_cnf == NULL)
		return FALSE;

	p_event_cnf = List_GetHead(p_alar_cnf->list_event_cnf);
	while (p_event_cnf != NULL)
	{
		if (p_event_cnf->id == id)
			break;

		p_event_cnf = List_GetNext(p_alar_cnf->list_event_cnf, p_event_cnf);
	}

	if (p_event_cnf == NULL)
		return FALSE;

	bFound = FALSE;
	aut_alarm = List_GetHead(*p_hlAlarm);
	while (aut_alarm != NULL)
	{
		p_event = ALARM_AUT_Get_Event(aut_alarm);

		if (p_event->type == type && p_event->subtype == sub_type)
		{
			if (p_event->id == id)
			{
				// take date and time into account
				if (STATUS_CNF.bTestDate )
				{
					if (p_event->appdate.wYear == event_time->wYear &&
						p_event->appdate.wMonth == event_time->wMonth &&
						p_event->appdate.wDay == event_time->wDay &&
						p_event->appdate.wHour == event_time->wHour &&
						p_event->appdate.wMinute == event_time->wMinute &&
						p_event->appdate.wSecond == event_time->wSecond)
					{
						bFound = TRUE;

						NTSVCInfo("ALARM_Search_And_Set_List() => ignore two identical alarms: (%lu/%lu/%lu at %d/%d/%d %d:%d:%d)",
							type, sub_type, id, event_time->wDay, event_time->wMonth, event_time->wYear,
							event_time->wHour, event_time->wMinute, event_time->wSecond);
					}
				}
				else
				{
					bFound = TRUE;

					arg_alarm_on.appdate = event_time;
					ALARM_AUT_Send(aut_alarm, EVT_ALARM_ON, &arg_alarm_on);

					List_Remove(*p_hlAlarm, aut_alarm);
					List_AddHead(*p_hlAlarm, aut_alarm);
				}
			}
			else
			{
				ALARM_AUT_Send(aut_alarm, EVT_ALARM_OFF, NULL);
			}
		}

		aut_alarm = List_GetNext(*p_hlAlarm, aut_alarm);
	}

	if (!bFound)
	{
		if (*p_hlAlarm == NULL)
			*p_hlAlarm = List_New();

		if (ALARM_AUT_New(*p_hlAlarm,
						&aut_alarm,
						p_lane_cnf,
						LANE_Get_Plaza_Number(p_lane_cnf),
						LANE_Get_Number(p_lane_cnf),
						type, sub_type, id,
						p_event_cnf->ack_mode,
						p_event_cnf->level,
						p_event_cnf->wording,
						p_event_cnf->directive,
						pDescription) == FALSE)
						return FALSE;

		List_AddHead(*p_hlAlarm, aut_alarm);

		arg_alarm_on.appdate = event_time;
		ALARM_AUT_Send(aut_alarm, EVT_ALARM_ON, &arg_alarm_on);
	}

	return TRUE;
}

PROTECTED BOOL ALARM_Search_And_Ack(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ *p_req)
{
	struct MSG_PV_ACK_REQ_Event		*p_alar_req = NULL;
	struct MSG_PV_ALAR_REP_Event	*p_event	= NULL;
	struct MSG_PV_ALAR_REP_Event	*p_copy		= NULL;
	struct MSG_PV_ALAR_REP			*p_db_req	= NULL;
	struct ARG_EVT_ALARM_ACKED		alarm_acked = { 0 };
	HLIST							*p_hlAlarm	= NULL;
	aut_alarm_id					aut_alarm	= NULL;

	p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);

	p_db_req = MSG_PV_ALAR_REP_New();
	if (p_db_req == NULL)
	{
		NTSVCError("ALARM_Search_And_Ack() => Error memory allocation: MSG_PV_ACK_REQ_New()!");

		return FALSE;
	}

	p_db_req->header = p_req->header;
	p_db_req->body.lanenum = p_req->body.lanenum;

	p_alar_req = MSG_PV_ACK_REQ_Get_First_Event(p_req->list_event);
	while (p_alar_req != NULL)
	{
		aut_alarm = List_GetHead(*p_hlAlarm);
		while (aut_alarm != NULL)
		{
			// get event
			p_event = ALARM_AUT_Get_Event(aut_alarm);

			// if we found the same alarm with the same event.
			if (p_event->plazanum == p_alar_req->plazanum &&
				p_event->lanenum == p_alar_req->lanenum &&
				p_event->type == p_alar_req->type &&
				p_event->subtype == p_alar_req->subtype &&
				p_event->id == p_alar_req->id)
			{
				// take date and time into account
				if (STATUS_CNF.bTestDate)
				{
					if (p_event->appdate.wYear == p_alar_req->appdate.wYear &&
						p_event->appdate.wMonth == p_alar_req->appdate.wMonth &&
						p_event->appdate.wDay == p_alar_req->appdate.wDay &&
						p_event->appdate.wHour == p_alar_req->appdate.wHour &&
						p_event->appdate.wMinute == p_alar_req->appdate.wMinute &&
						p_event->appdate.wSecond == p_alar_req->appdate.wSecond)
						break;
				}
				else
					break;
			}

			aut_alarm = List_GetNext(*p_hlAlarm, aut_alarm);
		}

		if (aut_alarm == NULL)
		{
			NTSVCError("ALARM_Search_And_Ack() => Request to acknowledge an unknown alarm! Plaza: %lu, Lane: %lu, Type: %lu, Subtype: %lu, Id: %lu)",
				p_alar_req->plazanum,
				p_alar_req->lanenum,
				p_alar_req->type,
				p_alar_req->subtype,
				p_alar_req->id);

			return FALSE;
		}

		SVC_Get_Time(&alarm_acked.ackdate);
		alarm_acked.matricule = p_req->body.matricule;
		alarm_acked.acked = FALSE;

		ALARM_AUT_Send(aut_alarm, EVT_ALARM_ACKED, &alarm_acked);

		if (alarm_acked.acked)
		{
			NTSVCInfo("ALARM_Search_And_Ack() => alarm acknowledgment: %s", p_event->label);

			p_copy = MSG_PV_ALAR_REP_Event_New(&p_db_req->list_event);
			if (p_copy == NULL)
			{
				NTSVCError("ALARM_Search_And_Ack() => Error memory allocation MSG_PV_ACK_REQ_Event_New()!");

				return FALSE;
			}

			*p_copy = *p_event;
		}

		p_alar_req = MSG_PV_ACK_REQ_Get_Next_Event(p_req->list_event, p_alar_req);
	}

	// send message to DB thread
	DB_PostMessage(DB_WM_ACK_ALARM, p_db_req);

	return TRUE;
}

PROTECTED BOOL ALARM_Search_And_Ack_Test(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ *p_req)
{
	struct MSG_PV_ACK_REQ_Event		*p_alar_req = NULL;
	struct MSG_PV_ALAR_REP_Event	*p_event = NULL;
	struct MSG_PV_ALAR_REP_Event	*p_copy = NULL;
	struct MSG_PV_ALAR_REP			*p_db_req = NULL;
	struct MSG_PV_ACK_REQ_REP		*p_req_rep = NULL;
	struct ARG_EVT_ALARM_ACKED		alarm_acked = { 0 };
	HLIST							*p_hlAlarm = NULL;
	aut_alarm_id					aut_alarm = NULL;

	// get list of alarms
	p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);

	// memory allocation
	p_db_req = MSG_PV_ALAR_REP_New();
	if (p_db_req == NULL)
	{
		NTSVCError("ALARM_Search_And_Ack_Test() => Error memory allocation: MSG_PV_ACK_REQ_New()!");
		return FALSE;
	}

	// copy data
	p_db_req->header = p_req->header;
	p_db_req->body.lanenum = p_req->body.lanenum;

	// research all events
	p_alar_req = MSG_PV_ACK_REQ_Get_First_Event(p_req->list_event);
	while (p_alar_req != NULL)
	{
		// reserach all alarms
		aut_alarm = List_GetHead(*p_hlAlarm);
		while (aut_alarm != NULL)
		{
			// get event
			p_event = ALARM_AUT_Get_Event(aut_alarm);

			// if we found the same alarm with the same event.
			if (p_event->plazanum == p_alar_req->plazanum &&
				p_event->lanenum == p_alar_req->lanenum &&
				p_event->type == p_alar_req->type &&
				p_event->subtype == p_alar_req->subtype &&
				p_event->id == p_alar_req->id)
			{
				// take date and time into account
				if (STATUS_CNF.bTestDate == TRUE)
				{
					if (p_event->appdate.wYear == p_alar_req->appdate.wYear &&
						p_event->appdate.wMonth == p_alar_req->appdate.wMonth &&
						p_event->appdate.wDay == p_alar_req->appdate.wDay &&
						p_event->appdate.wHour == p_alar_req->appdate.wHour &&
						p_event->appdate.wMinute == p_alar_req->appdate.wMinute &&
						p_event->appdate.wSecond == p_alar_req->appdate.wSecond)
						break;
				}
				else
					break;
			}

			aut_alarm = List_GetNext(*p_hlAlarm, aut_alarm);
		}

		// if not found
		if (aut_alarm == NULL)
		{
			if (aut_alarm == NULL)
			{
				NTSVCError("ALARM_Search_And_Ack_Test() => Request to acknowledge an unknown alarm! Plaza: %lu, Lane: %lu, Type: %lu, Subtype: %lu, Id: %lu)",
					p_alar_req->plazanum,
					p_alar_req->lanenum,
					p_alar_req->type,
					p_alar_req->subtype,
					p_alar_req->id);

				return FALSE;
			}

			return FALSE;
		}

		if (p_event->level == 2 || p_event->level == 4 || p_event->level == 5)
		{
			// alarm acknowledgment
			SVC_Get_Time(&alarm_acked.ackdate);
			alarm_acked.matricule = p_req->body.matricule;
			alarm_acked.acked = FALSE;

			ALARM_AUT_Send(aut_alarm, EVT_ALARM_ACKED, &alarm_acked);
		}
		else if (p_event->level == 0 || p_event->level == 1 || p_event->level == 3)
		{
			NTSVCInfo("ALARM_Search_And_Ack_Test() => alarm acknowledgment: %s", p_event->label);

			// memory allocation
			p_copy = MSG_PV_ALAR_REP_Event_New(&p_db_req->list_event);
			if (p_copy == NULL)
			{
				NTSVCError("ALARM_Search_And_Ack_Test() => Error memory allocation MSG_PV_ACK_REQ_Event_New()!");
				return FALSE;
			}

			// copy structure 
			*p_copy = *p_event;
		}

		// get next event
		p_alar_req = MSG_PV_ACK_REQ_Get_Next_Event(p_req->list_event, p_alar_req);
	}

	// prepare data to be sent on DB thread
	p_req_rep = ACK_REQ_REP_Duplicate(p_req, p_db_req);
	if (p_req_rep == NULL)
	{
		NTSVCError("ALARM_Search_And_Ack_Test() => Error memory allocation => ACK_REQ_REP_Duplicate()!");
		return FALSE;
	}

	// delete unnecessary data
	MSG_Delete_All(&p_db_req);

	// send message to DB thread
	DB_PostMessage(DB_WM_ACK_ALARM, p_req_rep);

	return TRUE;
}

PROTECTED BOOL ALARM_Ack_Test(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ_REP *p_req_rep)
{
	struct MSG_PV_ALAR_REP_Event	*p_alar_rep	= NULL;
	struct MSG_PV_ALAR_REP_Event	*p_event	= NULL;
	HLIST							*p_hlAlarm	= NULL;
	aut_alarm_id					aut_alarm	= NULL;
	struct MSG_PV_ACK_REQ			*p_req		= NULL;
	struct MSG_PV_ALAR_REP			*p_rep		= NULL;
	struct ARG_EVT_ALARM_ACKED		alarm_acked = { 0 };

	p_req = &p_req_rep->ack_req;
	p_rep = &p_req_rep->alar_rep;

	p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);

	p_alar_rep = MSG_PV_ALAR_REP_Get_First_Event(p_rep->list_event);
	while (p_alar_rep != NULL)
	{
		aut_alarm = List_GetHead(*p_hlAlarm);
		while (aut_alarm != NULL)
		{
			p_event = ALARM_AUT_Get_Event(aut_alarm);

			// if we have found the same alarm with the same event ...
			if (p_event->plazanum == p_alar_rep->plazanum &&
				p_event->lanenum == p_alar_rep->lanenum &&
				p_event->type == p_alar_rep->type &&
				p_event->subtype == p_alar_rep->subtype &&
				p_event->id == p_alar_rep->id)
			{
				// Taking into account the date ?
				break;
			}

			aut_alarm = List_GetNext(*p_hlAlarm, aut_alarm);
		}

		if (aut_alarm == NULL)
		{
			NTSVCError("ALARM_Ack_Test() => Request to acknowledge an unknown alarm! Plaza: %lu, Lane: %lu, Type: %lu, Subtype: %lu, Id: %lu)",
				p_alar_rep->plazanum,
				p_alar_rep->lanenum,
				p_alar_rep->type,
				p_alar_rep->subtype,
				p_alar_rep->id);

			return FALSE;
		}

		SVC_Get_Time(&alarm_acked.ackdate);
		alarm_acked.matricule = p_req->body.matricule;
		alarm_acked.acked = FALSE;

		ALARM_AUT_Send(aut_alarm, EVT_ALARM_ACKED, &alarm_acked);

		p_alar_rep = MSG_PV_ALAR_REP_Get_Next_Event(p_rep->list_event, p_alar_rep);
	}

	return TRUE;
}

PROTECTED struct MSG_PV_ALAR_REP * ALARM_Build_msg_pv_alar_rep(struct LANE_CNF *p_lane_cnf, BOOL partial)
{
	struct MSG_PV_ALAR_REP			*p_alar			= NULL;
	aut_alarm_id					aut_alarm_next	= NULL;
	aut_alarm_id					aut_alarm		= NULL;
	struct MSG_PV_ALAR_REP_Event	*p_event		= NULL;
	struct MSG_PV_ALAR_REP_Event	*p_alar_event	= NULL;
	HLIST							*p_hlAlarm		= NULL;

	p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);

	if (partial && !LANE_Is_Alarm_List_Altered(p_lane_cnf))
		return NULL;

	p_alar = MSG_PV_ALAR_REP_New();
	if (p_alar == NULL)
		return NULL;

	p_alar->header.id = SVC_Get_Msg_Id();
	p_alar->body.lanenum = LANE_Get_Number(p_lane_cnf);
	p_alar->body.unacked = 0;

	aut_alarm = List_GetTail(*p_hlAlarm);
	while (aut_alarm != NULL)
	{
		aut_alarm_next = List_GetPrevious(*p_hlAlarm, aut_alarm);

		if (ALARM_AUT_To_Be_Deleted(aut_alarm))
		{
			List_Remove(*p_hlAlarm, aut_alarm);
			ALARM_AUT_Delete(*p_hlAlarm, &aut_alarm);
		}
		else
		{
			p_event = ALARM_AUT_Get_Event(aut_alarm);

			p_alar_event = MSG_PV_ALAR_REP_Event_New(&p_alar->list_event);
			if (p_alar_event == NULL)
			{
				NTSVC_ERR("ALARM_Build_msg_pv_alar_rep() => Error memory allocation: MSG_PV_ALAR_REP_Event_New()!");
				MSG_PV_ALAR_REP_Delete_All(p_alar);

				return NULL;
			}

			*p_alar_event = *p_event;

			// count unacknowledged alarms
			switch (STATUS_CNF.LaneAlarmCounterType)
			{
				// count all unacknowledged alarms
				default:
				case 0: 
					p_alar->body.unacked++;
					break;

				// There are only unacknowledged alarms
				case 1:
					if (p_alar_event->state == 1)
						p_alar->body.unacked++;
					break;
			}
		}

		aut_alarm = aut_alarm_next;
	}

	// alarm list modified, set flag
	LANE_Set_Alarm_Flag(p_lane_cnf, FALSE);

	// update the number of unacknowledged alarms
	LANE_Set_Unacked_Alarms_Count(p_lane_cnf, p_alar->body.unacked);

	return p_alar;
}

PROTECTED BOOL ALARM_Delete_All(void)
{
	struct ALARM_CNF *p_alar_cnf = NULL;

	p_alar_cnf = List_GetHead(STATUS_CNF.list_alarm_cnf);
	while (p_alar_cnf != NULL)
	{
		List_DeleteAll(&p_alar_cnf->list_event_cnf, NULL, NULL);

		p_alar_cnf = List_GetNext(STATUS_CNF.list_alarm_cnf, p_alar_cnf);
	}

	List_DeleteAll(&STATUS_CNF.list_alarm_cnf, NULL, NULL);

	return TRUE;
}

PROTECTED BOOL ALARM_Lane_Delete_All(struct LANE_CNF *p_lane_cnf)
{
	aut_alarm_id	aut_alarm		= NULL;
	aut_alarm_id	aut_alarm_next	= NULL;
	HLIST			*p_hlAlarm		= NULL;

	p_hlAlarm = LANE_Get_List_Alarm(p_lane_cnf);

	aut_alarm = List_GetTail(*p_hlAlarm);
	while (aut_alarm != NULL)
	{
		aut_alarm_next = List_GetPrevious(*p_hlAlarm, aut_alarm);

		// remove from list
		List_Remove(*p_hlAlarm, aut_alarm);

		// delete
		ALARM_AUT_Delete(*p_hlAlarm, &aut_alarm);

		aut_alarm = aut_alarm_next;
	}

	List_Delete(p_hlAlarm);

	// alarm list modified, set flag
	LANE_Set_Alarm_Flag(p_lane_cnf, TRUE);

	return TRUE;
}

PROTECTED BOOL TestAlarmAckType(void)
{
	return STATUS_CNF.bTestAlarmAckType;
}

PROTECTED BOOL Delete_List(IN OUT HLIST *hList)
{
	PVOID pHead = NULL;

	if (hList == NULL)
		return TRUE;

	while ((pHead = List_RemoveHead(*hList)) != NULL)
		List_ItemDelete(*hList, &pHead);

	if (!List_Delete(hList))
		return FALSE;

	return TRUE;
}

PROTECTED BOOL ACK_REQ_REP_Delete_All(struct MSG_PV_ACK_REQ_REP *pAckRepReq)
{
	Delete_List(&pAckRepReq->ack_req.list_event);

	Delete_List(&pAckRepReq->alar_rep.list_event);

	free(pAckRepReq);
	pAckRepReq = NULL;

	return TRUE;
}

PROTECTED struct MSG_PV_ACK_REQ_REP * ACK_REQ_REP_Duplicate(struct MSG_PV_ACK_REQ *pAcqReq, struct MSG_PV_ALAR_REP *pAlarRep)
{
	struct MSG_PV_ACK_REQ_REP		*pAckReqRep			= NULL;
	struct MSG_PV_ACK_REQ_Event		*pAckReqEvent		= NULL;
	struct MSG_PV_ACK_REQ_Event		*pAckReqEventCopy	= NULL;
	struct MSG_PV_ALAR_REP_Event	*pAlarRepEvent		= NULL;
	struct MSG_PV_ALAR_REP_Event	*pAlarRepEventCopy	= NULL;

	// memory allocation
	pAckReqRep = (struct MSG_PV_ACK_REQ_REP *)calloc(1, sizeof(struct MSG_PV_ACK_REQ_REP));
	if (pAckReqRep == NULL)
	{
		NTSVCError("ACK_REQ_REP_Duplicate() => Error memory allocation: calloc()!");
		return NULL;
	}

	pAckReqRep->ack_req.header = pAcqReq->header;
	pAckReqRep->ack_req.body = pAcqReq->body;

	pAckReqEvent = MSG_PV_ACK_REQ_Get_First_Event(pAcqReq->list_event);
	while (pAckReqEvent != NULL)
	{
		pAckReqEventCopy = MSG_PV_ACK_REQ_Event_New(&pAckReqRep->ack_req.list_event);
		if (pAckReqEventCopy == NULL)
		{
			NTSVCError("ACK_REQ_REP_Duplicate() => Error memory allocation: MSG_PV_ACK_REQ_Event_New()!");
			return FALSE;
		}

		*pAckReqEventCopy = *pAckReqEvent;

		pAckReqEvent = MSG_PV_ACK_REQ_Get_Next_Event(pAcqReq->list_event, pAckReqEvent);
	}

	pAckReqRep->alar_rep.header = pAlarRep->header;
	pAckReqRep->alar_rep.body = pAlarRep->body;

	pAlarRepEvent = MSG_PV_ALAR_REP_Get_First_Event(pAcqReq->list_event);
	while (pAlarRepEvent != NULL)
	{
		pAlarRepEventCopy = MSG_PV_ALAR_REP_Event_New(&pAckReqRep->ack_req.list_event);
		if (pAlarRepEventCopy == NULL)
		{
			NTSVCError("ACK_REQ_REP_Duplicate() => Error memory allocation: MSG_PV_ALAR_REP_Event_New()!");
			return FALSE;
		}

		*pAlarRepEventCopy = *pAlarRepEvent;

		pAlarRepEvent = MSG_PV_ALAR_REP_Get_Next_Event(pAcqReq->list_event, pAlarRepEvent);
	}

	return pAckReqRep;
}