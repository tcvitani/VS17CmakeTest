/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_LANE_Get_Counter.c										 */
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
	DB_CNX	*hDbCnx;
}
Counter = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_LANE_Get_Counter(IN DWORD plazanum,
											IN DWORD lanenum,
											IN DWORD disp_id,
											OUT BOOL counter[LANE_MAX_COUNTERS])
{
	void	*pvResult = NULL;
	DWORD	dwErr								= 0;
	CHAR	szErr[200]							= { 0 };
	CHAR	szStatement[1024]					= { 0 };
	CHAR	szCounters[LANE_MAX_COUNTERS * 2]	= { 0 };
	BOOL	bRet								= FALSE;

	Counter.hDbCnx = DB_CONNECTION_Open();
	if (Counter.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_LANE_Get_Counter() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN LANE.LANE_DB_Get_Counter(%d, %d, %d, :CounterId); END;", plazanum, lanenum, disp_id);

		Counter.hStat = _DBOpenStatement(Counter.hDbCnx, szStatement);
		if (Counter.hStat == NULL)
		{
			_DBGetLastError(Counter.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_Counter() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Counter.hId = _DBBindPlaceHolderVariable(Counter.hStat, ":CounterId", DB_TYPE_STR, LANE_MAX_COUNTERS * 2, 0);
		if (Counter.hId == NULL)
		{
			_DBGetLastError(Counter.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_Counter() => Error: %u, bind CounterId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Counter.hStat))
		{
			_DBGetLastError(Counter.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_LANE_Get_Counter() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Counter.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Counter.hId, Counter.dwConfIndex, szCounters, NULL);
		if (pvResult != szCounters)
			break;

		if (sscanf_s(szCounters, "%d|%d|%d|%d|%d", &counter[0], &counter[1], &counter[2], &counter[3], &counter[4]) < LANE_MAX_COUNTERS)
			break;

		NTSVCInfo("CONF_LANE_Get_Counter(), ( %d %d %d %d %d )", counter[0], counter[1], counter[2], counter[3], counter[4]);

		bRet = TRUE;
	} 
	while (FALSE);

	if (Counter.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_LANE_Get_Counter() => disconnect from database" );
		_DBCloseStatement(Counter.hStat);
		DB_CONNECTION_Close();
		Counter.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/