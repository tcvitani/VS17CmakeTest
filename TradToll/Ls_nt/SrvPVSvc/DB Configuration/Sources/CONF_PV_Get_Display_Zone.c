/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Display_Zone.c										 */
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
	DB_STMT	*hStat;
	DB_VAR	*hId;        // Handle of the zone id table by display
	DB_CNX	*hDbCnx;
}
Zone = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Display_Zone(IN DWORD plazanum,
													 IN DWORD disp_id,
													 OUT DWORD *zone_id)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Zone.hDbCnx = DB_CONNECTION_Open();
	if (Zone.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_Display_Zone() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PLAZA.PLAZA_DB_PV_Get_Display_Zone(%d, %d, :ZoneId); END;", 
					plazanum, 
					disp_id);

		Zone.hStat = _DBOpenStatement(Zone.hDbCnx, szStatement);
		if (Zone.hStat == NULL)
		{
			_DBGetLastError(Zone.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Zone() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Zone.hId = _DBBindPlaceHolderVariable(Zone.hStat, ":ZoneId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_ZONES);
		if (Zone.hId == NULL)
		{
			_DBGetLastError(Zone.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display_Zone() => Error: %u, bind ZoneId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Zone.hStat))
		{
			_DBGetLastError(Zone.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display_Zone() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Zone.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Zone.hId, Zone.dwConfIndex, zone_id, NULL);
		if (pvResult != zone_id)
			break;

		NTSVCInfo("CONF_PV_Get_First_Display_Zone(), %d = %d", disp_id, *zone_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Zone.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Display_Zone() => disconnect from database" );
		_DBCloseStatement(Zone.hStat);
		DB_CONNECTION_Close();
		Zone.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Display_Zone(OUT DWORD *zone_id)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Zone.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Zone.hId, Zone.dwConfIndex, zone_id, NULL);
		if (pvResult != zone_id)
			break;

		NTSVCInfo("CONF_PV_Get_Next_Display_Zone(), %d", *zone_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Zone.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Display_Zone() => disconnect from database" );
		_DBCloseStatement(Zone.hStat);
		DB_CONNECTION_Close();
		Zone.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/