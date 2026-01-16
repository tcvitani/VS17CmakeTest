/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Ack.c												 */
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
#include <srvpv_dbif.h>
#include <ntsvc.h>
#include <db_connection.h>
#include <conf_srvpv.h>
#include <conf_define.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct
{
	DWORD	dwConfIndex;
	DB_STMT	*hStat;
	DB_VAR	*hPlaza;
	DB_VAR	*hLane;
	DB_VAR	*hType;
	DB_VAR	*hSubType;
	DB_VAR	*hEvent;
	DB_VAR	*hAppDate;
	DB_VAR	*hAckDate;
	DB_VAR	*hMatricule;
	DB_CNX	*hDbCnx;
}
Ack = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: BOOL CONF_ALARM_Ack (struct MSG_PV_ALAR_REP *p_alar)               */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED BOOL WINAPI CONF_ALARM_Ack(struct MSG_PV_ALAR_REP *p_alar)
{
	void							*pvResult			= NULL;
	DWORD							dwErr				= 0;
	CHAR							szErr[200]			= { 0 };
	CHAR							szStatement[1024]	= { 0 };
	char							szAppDate[15]		= { 0 };
	char							szAckDate[15]		= { 0 };
	BOOL							bRet				= FALSE;
	struct	MSG_PV_ALAR_REP_Event	*p_event			= NULL;

	// if list empty there is nothing to do
	if (List_IsEmpty(p_alar->list_event))
		return TRUE;

	Ack.hDbCnx = DB_CONNECTION_Open();
	if (Ack.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_ALARM_Ack() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN ALARM.ALARM_DB_Ack( :Plaza, :Lane, :Type, :SubType, :Event, :AppDate, :AckDate, :Matricule); END;");

		Ack.hStat = _DBOpenStatement(Ack.hDbCnx, szStatement);
		if (Ack.hStat == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => _DBOpenStatement(), Error: %u,  %s", dwErr, szErr);
			break;
		}

		// Associate variables  
		Ack.hPlaza = _DBBindPlaceHolderVariable(Ack.hStat, ":Plaza", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hPlaza == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind Plaza: %s", dwErr, szErr);
			break;
		}

		Ack.hLane = _DBBindPlaceHolderVariable(Ack.hStat, ":Lane", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hLane == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind Lane: %s", dwErr, szErr);
			break;
		}

		Ack.hType = _DBBindPlaceHolderVariable(Ack.hStat, ":Type", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hType == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind Type: %s", dwErr, szErr);
			break;
		}

		Ack.hSubType = _DBBindPlaceHolderVariable(Ack.hStat, ":SubType", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hSubType == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind SubType: %s", dwErr, szErr);
			break;
		}

		Ack.hEvent = _DBBindPlaceHolderVariable(Ack.hStat, ":Event", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hEvent == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind Event: %s", dwErr, szErr);
			break;
		}

		Ack.hAppDate = _DBBindPlaceHolderVariable(Ack.hStat, ":AppDate", DB_TYPE_STR, CONF_MAX_DATE, List_GetCount(p_alar->list_event));
		if (Ack.hAppDate == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind AppDate: %s", dwErr, szErr);
			break;
		}

		Ack.hAckDate = _DBBindPlaceHolderVariable(Ack.hStat, ":AckDate", DB_TYPE_STR, CONF_MAX_DATE, List_GetCount(p_alar->list_event));
		if (Ack.hAckDate == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind AckDate: %s", dwErr, szErr);
			break;
		}

		Ack.hMatricule = _DBBindPlaceHolderVariable(Ack.hStat, ":Matricule", DB_TYPE_INT, sizeof(DWORD), List_GetCount(p_alar->list_event));
		if (Ack.hMatricule == NULL)
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => Error: %u, bind Matricule: %s", dwErr, szErr);
			break;
		}

		Ack.dwConfIndex = 0;
		p_event = MSG_PV_ALAR_REP_Get_First_Event(p_alar->list_event);
		while (p_event != NULL)
		{
			pvResult = _DBSetVariableItemValue(Ack.hPlaza, Ack.dwConfIndex, &p_event->plazanum, sizeof(DWORD));
			if (pvResult != &p_event->plazanum)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hPlaza): %s", dwErr, szErr);
				break;
			}

			pvResult = _DBSetVariableItemValue(Ack.hLane, Ack.dwConfIndex, &p_event->lanenum, sizeof(DWORD));
			if (pvResult != &p_event->lanenum)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hLane): %s", dwErr, szErr);
				break;
			}

			pvResult = _DBSetVariableItemValue(Ack.hType, Ack.dwConfIndex, &p_event->type, sizeof(DWORD));
			if (pvResult != &p_event->type)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hType): %s", dwErr, szErr);
				break;
			}

			pvResult = _DBSetVariableItemValue(Ack.hSubType, Ack.dwConfIndex, &p_event->subtype, sizeof(DWORD));
			if (pvResult != &p_event->subtype)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hSubType): %s", dwErr, szErr);
				break;
			}

			pvResult = _DBSetVariableItemValue(Ack.hEvent, Ack.dwConfIndex, &p_event->id, sizeof(DWORD));
			if (pvResult != &p_event->id)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hEvent): %s", dwErr, szErr);
				break;
			}

			_snprintf_s(szAppDate,
				_countof(szAppDate),
				sizeof(szAppDate),
				"%04d%02d%02d%02d%02d%02d\0",
				p_event->appdate.wYear, p_event->appdate.wMonth, p_event->appdate.wDay,
				p_event->appdate.wHour, p_event->appdate.wMinute, p_event->appdate.wSecond);

			pvResult = _DBSetVariableItemValue(Ack.hAppDate, Ack.dwConfIndex, &szAppDate, (DWORD)strlen(szAppDate) + 1);
			if (pvResult != &szAppDate)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hAppDate): %s", dwErr, szErr);
				break;
			}

			_snprintf_s(szAckDate,
				_countof(szAckDate),
				sizeof(szAckDate),
				"%04d%02d%02d%02d%02d%02d\0",
				p_event->ackdate.wYear, p_event->ackdate.wMonth, p_event->ackdate.wDay,
				p_event->ackdate.wHour, p_event->ackdate.wMinute, p_event->ackdate.wSecond);

			pvResult = _DBSetVariableItemValue(Ack.hAckDate, Ack.dwConfIndex, &szAckDate, (DWORD)strlen(szAckDate) + 1);
			if (pvResult != &szAckDate)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hAckdate): %s", dwErr, szErr);
				break;
			}

			pvResult = _DBSetVariableItemValue(Ack.hMatricule, Ack.dwConfIndex, &p_event->matricule, sizeof(DWORD));
			if (pvResult != &p_event->matricule)
			{
				_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
				NTSVCInfo("CONF_ALARM_Ack() => Error: %u, DBSetVariableItemValue(hMatricule): %s", dwErr, szErr);
				break;
			}

			Ack.dwConfIndex++;
			p_event = MSG_PV_ALAR_REP_Get_Next_Event(p_alar->list_event, p_event);
		}

		// Execute the request
		if (!_DBExecuteStatement(Ack.hStat))
		{
			_DBGetLastError(Ack.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Ack() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		bRet = TRUE;
	} 
	while (FALSE);

	if (Ack.hDbCnx != NULL)
	{
		// NTSVCInfo("CONF_ALARM_Ack() => disconnect from database!");
		_DBCloseStatement(Ack.hStat);
		DB_CONNECTION_Close();
		Ack.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
