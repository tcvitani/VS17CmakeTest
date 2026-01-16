/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Event.c											 */
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
#include <conf_srvpv.h>
#include <conf_define.h>
#include <Db_connection.h>
#include <Msg_Pv_Define.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct
{
	DWORD	dwConfIndex;
	DB_STMT *hStat;
	DB_VAR	*hId;
	DB_VAR	*hAckMode;
	DB_VAR	*hLevel;
	DB_VAR	*hWording;
	DB_VAR	*hDirect;
	DB_CNX	*hDbCnx;
}
Event = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_ALARM_Get_First_Event(IN DWORD alarm_type,
												 IN DWORD alarm_sub_type,
												 OUT DWORD *event_id,
												 OUT DWORD *event_ack_mode,
												 OUT DWORD *event_level,
												 OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
												 OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	dwErr				= 0;
	DWORD	dwStrSize			= 0;

	Event.hDbCnx = DB_CONNECTION_Open();
	if (Event.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN ALARM.ALARM_DB_Get_Event( %d, %d, :Id, :AckMode, :Level, :Wording, :Directive); END;", alarm_type, alarm_sub_type);

		Event.hStat = _DBOpenStatement(Event.hDbCnx, szStatement);
		if (Event.hStat == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Event.hId = _DBBindPlaceHolderVariable(Event.hStat, ":Id", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EVENT);
		if (Event.hId == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: %u, bind EventId : %s", dwErr, szErr);
			break;
		}

		Event.hAckMode = _DBBindPlaceHolderVariable(Event.hStat, ":AckMode", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EVENT);
		if (Event.hAckMode == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: %u, bind EventAckMode : %s", dwErr, szErr);
			break;
		}

		Event.hLevel = _DBBindPlaceHolderVariable(Event.hStat, ":Level", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EVENT);
		if (Event.hLevel == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: %u, bind EventLevel : %s", dwErr, szErr);
			break;
		}

		Event.hWording = _DBBindPlaceHolderVariable(Event.hStat, ":Wording", DB_TYPE_STR, CONF_MAX_EVENT_WORDING, CONF_MAX_EVENT);
		if (Event.hWording == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: %u, bind EventWording : %s", dwErr, szErr);
			break;
		}

		Event.hDirect = _DBBindPlaceHolderVariable(Event.hStat, ":Directive", DB_TYPE_STR, CONF_MAX_EVENT_DIRECTIVE, CONF_MAX_EVENT);
		if (Event.hDirect == NULL)
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => Error: %u, bind EventDirective : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Event.hStat))
		{
			_DBGetLastError(Event.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Event() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Event.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Event.hId, Event.dwConfIndex, event_id, NULL);
		if (pvResult != event_id)
			break;

		pvResult = _DBGetVariableItemValue(Event.hAckMode, Event.dwConfIndex, event_ack_mode, NULL);
		if (pvResult != event_ack_mode)
			break;

		pvResult = _DBGetVariableItemValue(Event.hLevel, Event.dwConfIndex, event_level, NULL);
		if (pvResult != event_level)
			break;

		dwStrSize = MSG_PV_MAX_ALARM_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Event.hWording, Event.dwConfIndex, event_wording, &dwStrSize);
		if (pvResult != event_wording)
			break;

		dwStrSize = MSG_PV_MAX_DETAIL_SIZE;
		pvResult = _DBGetVariableItemValueTruncate(Event.hDirect, Event.dwConfIndex, event_directive, &dwStrSize);
		if (pvResult != event_directive)
			break;

		NTSVCInfo("CONF_ALARM_Get_First_Event(), Id = %d, ackmode = %d, level = %d, wording = %s, directive = %s",
			*event_id, *event_ack_mode, *event_level, event_wording, event_directive);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Event.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_ALARM_Get_First_Event() => disconnect from database!" );
		_DBCloseStatement(Event.hStat);
		DB_CONNECTION_Close();
		Event.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_ALARM_Get_Next_Event(OUT DWORD *event_id,
												OUT DWORD *event_ack_mode,
												OUT DWORD *event_level,
												OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
												OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	dwStrSize	= 0;

	bRet = FALSE;
	do
	{
		Event.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Event.hId, Event.dwConfIndex, event_id, NULL);
		if (pvResult != event_id)
			break;

		pvResult = _DBGetVariableItemValue(Event.hAckMode, Event.dwConfIndex, event_ack_mode, NULL);
		if (pvResult != event_ack_mode)
			break;

		pvResult = _DBGetVariableItemValue(Event.hLevel, Event.dwConfIndex, event_level, NULL);
		if (pvResult != event_level)
			break;

		dwStrSize = MSG_PV_MAX_ALARM_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Event.hWording, Event.dwConfIndex, event_wording, &dwStrSize);
		if (pvResult != event_wording)
			break;

		dwStrSize = MSG_PV_MAX_DETAIL_SIZE;
		pvResult = _DBGetVariableItemValueTruncate(Event.hDirect, Event.dwConfIndex, event_directive, &dwStrSize);
		if (pvResult != event_directive)
			break;

		NTSVCInfo("CONF_ALARM_Get_Next_Event(), Id = %d, ackmode = %d, level = %d, wording = %s, directive = %s",
			*event_id, *event_ack_mode, *event_level, event_wording, event_directive);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Event.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_ALARM_Get_Next_Event() => disconnect from database!" );
		_DBCloseStatement(Event.hStat);
		DB_CONNECTION_Close();
		Event.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/