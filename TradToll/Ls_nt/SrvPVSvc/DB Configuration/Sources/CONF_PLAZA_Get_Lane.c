/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Lane.c											 */
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
	DB_VAR	*hName;
	DB_VAR	*hType;
	DB_CNX	*hDbCnx;
} 
Lane = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Lane(IN DWORD plazanum, OUT DWORD *lanenum)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Lane.hDbCnx = DB_CONNECTION_Open();
	if (Lane.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Lane() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Lane(%d,:LaneId); END;", plazanum);

		Lane.hStat = _DBOpenStatement(Lane.hDbCnx, szStatement);
		if (Lane.hStat == NULL)
		{
			_DBGetLastError(Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Lane() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Lane.hId = _DBBindPlaceHolderVariable(Lane.hStat, ":LaneId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_LANES);

		if (Lane.hId == NULL)
		{
			_DBGetLastError(Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Lane() => Error: %u, bind LaneId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Lane.hStat))
		{
			_DBGetLastError(Lane.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Lane() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Lane.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Lane.hId, Lane.dwConfIndex, lanenum, NULL);
		if (pvResult != lanenum)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Lane(), Lanenum = %d", *lanenum);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Lane.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Lane() => disconnect from database" );
		_DBCloseStatement(Lane.hStat);
		DB_CONNECTION_Close();
		Lane.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Lane(OUT DWORD *lanenum)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Lane.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Lane.hId, Lane.dwConfIndex, lanenum, NULL);
		if (pvResult != lanenum)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Lane(), Lanenum = %d", *lanenum);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Lane.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Lane() => disconnect from database!" );
		_DBCloseStatement(Lane.hStat);
		DB_CONNECTION_Close();
		Lane.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

