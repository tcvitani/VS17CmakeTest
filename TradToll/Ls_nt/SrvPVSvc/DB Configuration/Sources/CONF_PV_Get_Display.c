/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Display.c											 */
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
	DB_VAR	*hId;        // Handle of the display id table
	DB_CNX	*hDbCnx;
}
Display = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Display(IN DWORD plazanum, OUT DWORD *disp_id)
{
	void	*pvResult			= NULL;
	char	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Display.hDbCnx = DB_CONNECTION_Open();

	if (Display.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_Display() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement,
					_countof(szStatement),
					sizeof(szStatement),
					"BEGIN PLAZA.PLAZA_DB_PV_Get_Display(%d, :DisplayId); END;",
					plazanum);

		Display.hStat = _DBOpenStatement(Display.hDbCnx, szStatement);
		if (Display.hStat == NULL)
		{
			_DBGetLastError(Display.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Display.hId = _DBBindPlaceHolderVariable(Display.hStat, ":DisplayId", DB_TYPE_INT, sizeof(DWORD), LANE_MAX_DISPLAY);
		if (Display.hId == NULL)
		{
			_DBGetLastError(Display.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display() => Error: %u, bind Display.hId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Display.hStat))
		{
			_DBGetLastError(Display.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Display() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Display.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Display.hId, Display.dwConfIndex, disp_id, NULL);
		if (pvResult != disp_id)
			break;

		NTSVCInfo("CONF_PV_Get_First_Display(), %d = %d", plazanum, *disp_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Display.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Display() => disconnect from database" );
		_DBCloseStatement(Display.hStat);
		DB_CONNECTION_Close();
		Display.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Display(OUT DWORD *disp_id)
{
	void	*pvResult	= NULL;
	char	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Display.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Display.hId, Display.dwConfIndex, disp_id, NULL);
		if (pvResult != disp_id)
			break;

		NTSVCInfo("CONF_PV_Get_Next_Display(), %d", *disp_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Display.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_Next_Display() => disconnect from database!" );
		_DBCloseStatement(Display.hStat);
		DB_CONNECTION_Close();
		Display.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/