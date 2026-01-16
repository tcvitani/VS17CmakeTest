/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_LANE_Get_Eqt.c											 */
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
	DB_VAR	*hEventId;
	DB_VAR	*hSubEventId;
	DB_CNX	*hDbCnx;
}
Eqt = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_LANE_Get_First_Eqt(IN DWORD plazanum,
											  IN DWORD lanenum,
											  OUT DWORD *eqt_id,
											  OUT DWORD *alarm_event,
											  OUT DWORD *alarm_sub_event)
{
	void	*pvResult			= NULL;
	DWORD	dwErr				= 0;
	CHAR	szErr[200]			= { 0 };
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Eqt.hDbCnx = DB_CONNECTION_Open();
	if (Eqt.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_LANE_Get_First_Eqt() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN LANE.LANE_DB_Get_Eqt(%d, %d, :EqtId, :EventId, :SubEventId); END;", plazanum, lanenum);

		Eqt.hStat = _DBOpenStatement(Eqt.hDbCnx, szStatement);
		if (Eqt.hStat == NULL)
		{
			_DBGetLastError(Eqt.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_First_Eqt() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Eqt.hId = _DBBindPlaceHolderVariable(Eqt.hStat, ":EqtId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EQT);
		if (Eqt.hId == NULL)
		{
			_DBGetLastError(Eqt.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_First_Eqt() => Error: %u, bind EqtId : %s", dwErr, szErr);
			break;
		}

		Eqt.hEventId = _DBBindPlaceHolderVariable(Eqt.hStat, ":EventId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EQT);
		if (Eqt.hEventId == NULL)
		{
			_DBGetLastError(Eqt.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_First_Eqt() => Error: %u, bind EventId : %s", dwErr, szErr);
			break;
		}

		Eqt.hSubEventId = _DBBindPlaceHolderVariable(Eqt.hStat, ":SubEventId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_EQT);
		if (Eqt.hSubEventId == NULL)
		{
			_DBGetLastError(Eqt.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_First_Eqt() => Error: %u, bind SubEventId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Eqt.hStat))
		{
			_DBGetLastError(Eqt.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_First_Eqt() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Eqt.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Eqt.hId, Eqt.dwConfIndex, eqt_id, NULL);
		if (pvResult != eqt_id)
			break;

		pvResult = _DBGetVariableItemValue(Eqt.hEventId, Eqt.dwConfIndex, alarm_event, NULL);
		if (pvResult != alarm_event)
			break;

		pvResult = _DBGetVariableItemValue(Eqt.hSubEventId, Eqt.dwConfIndex, alarm_sub_event, NULL);
		if (pvResult != alarm_sub_event)
			break;

		NTSVCInfo("CONF_LANE_Get_First_Eqt(), Eqtid = %d, Event = %d, SubEvent = %d", *eqt_id, *alarm_event, *alarm_sub_event);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Eqt.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_LANE_Get_First_Eqt() => disconnect from database!" );
		_DBCloseStatement(Eqt.hStat);
		DB_CONNECTION_Close();
		Eqt.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_LANE_Get_Next_Eqt(OUT DWORD *eqt_id, OUT DWORD *alarm_event, OUT DWORD *alarm_sub_event)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Eqt.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Eqt.hId, Eqt.dwConfIndex, eqt_id, NULL);
		if (pvResult != eqt_id)
			break;

		pvResult = _DBGetVariableItemValue(Eqt.hEventId, Eqt.dwConfIndex, alarm_event, NULL);
		if (pvResult != alarm_event)
			break;

		pvResult = _DBGetVariableItemValue(Eqt.hSubEventId, Eqt.dwConfIndex, alarm_sub_event, NULL);
		if (pvResult != alarm_sub_event)
			break;

		NTSVCInfo("CONF_LANE_Get_Next_Eqt(), Eqtid = %d, Event = %d, SubEvent = %d", *eqt_id, *alarm_event, *alarm_sub_event);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Eqt.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_LANE_Get_Next_Eqt() => disconnect from database!" );
		_DBCloseStatement(Eqt.hStat);
		DB_CONNECTION_Close();
		Eqt.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

