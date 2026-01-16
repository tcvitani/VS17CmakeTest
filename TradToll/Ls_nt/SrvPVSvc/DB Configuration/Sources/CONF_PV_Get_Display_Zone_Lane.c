/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Display_Zone_Lane.c								 */
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
Zone_Lane = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Display_Zone_Lane(IN DWORD plazanum,
														  IN DWORD disp_id,
														  IN DWORD zone_id,
														  OUT DWORD *lane_number)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Zone_Lane.hDbCnx = DB_CONNECTION_Open();
	if (Zone_Lane.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PLAZA.PLAZA_DB_PV_Get_Zone_Lane(%d, %d, %d, :Zone_LaneId); END;", 
					plazanum, 
					disp_id, 
					zone_id);

		Zone_Lane.hStat = _DBOpenStatement(Zone_Lane.hDbCnx, szStatement);
		if (Zone_Lane.hStat == NULL)
		{
			_DBGetLastError(Zone_Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Zone_Lane.hId = _DBBindPlaceHolderVariable(Zone_Lane.hStat, ":Zone_LaneId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_ZONE_LANES);
		if (Zone_Lane.hId == NULL)
		{
			_DBGetLastError(Zone_Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane() => Error: %u, bind Zone_LaneId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Zone_Lane.hStat))
		{
			_DBGetLastError(Zone_Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Zone_Lane.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Zone_Lane.hId, Zone_Lane.dwConfIndex, lane_number, NULL);
		if (pvResult != lane_number)
			break;

		NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane(), %d = %d", disp_id, zone_id, *lane_number);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Zone_Lane.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Display_Zone_Lane() => disconnect from database" );
		_DBCloseStatement(Zone_Lane.hStat);
		DB_CONNECTION_Close();
		Zone_Lane.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Display_Zone_Lane(OUT DWORD *lane_number)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Zone_Lane.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Zone_Lane.hId, Zone_Lane.dwConfIndex, lane_number, NULL);
		if (pvResult != lane_number)
			break;

		NTSVCInfo("CONF_PV_Get_Next_Display_Zone_Lane(), %d", *lane_number);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Zone_Lane.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Display_Zone_Lane() => disconnect from database" );
		_DBCloseStatement(Zone_Lane.hStat);
		DB_CONNECTION_Close();
		Zone_Lane.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/