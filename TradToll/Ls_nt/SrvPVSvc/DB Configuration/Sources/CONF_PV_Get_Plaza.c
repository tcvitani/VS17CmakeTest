/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Plaza.c											 */
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
	DB_VAR	*hName;
	DB_CNX	*hDbCnx;
}
Plaza = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Plaza(OUT DWORD *plazanum,
											  OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
											  IN DWORD dwDefaultPlazaNumber)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Plaza.hDbCnx = DB_CONNECTION_Open();
	if (Plaza.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_Plaza() => Error: connection to database impossible!");
		return FALSE;
	}

	// if the default plaza number defined take it, or get plaza number from database 
	if (dwDefaultPlazaNumber > 0)
	{
		*plazanum = dwDefaultPlazaNumber;
		return TRUE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_PV_Get_Plaza(:PlazaId,:PlazaName); END;");

		Plaza.hStat = _DBOpenStatement(Plaza.hDbCnx, szStatement);
		if (Plaza.hStat == NULL)
		{
			_DBGetLastError(Plaza.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Plaza() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Plaza.hId = _DBBindPlaceHolderVariable(Plaza.hStat, ":PlazaId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_PLAZA);
		if (Plaza.hId == NULL)
		{
			_DBGetLastError(Plaza.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Plaza() => Error: %u, bind PlazaId : %s", dwErr, szErr);
			break;
		}

		Plaza.hName = _DBBindPlaceHolderVariable(Plaza.hStat, ":PlazaName", DB_TYPE_STR, CONF_MAX_PLAZA_NAME, CONF_MAX_PLAZA);
		if (Plaza.hName == NULL)
		{
			_DBGetLastError(Plaza.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Plaza() => Error: %u, bind PlazaName : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Plaza.hStat))
		{
			_DBGetLastError(Plaza.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Plaza() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Plaza.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Plaza.hId, Plaza.dwConfIndex, plazanum, NULL);
		if (pvResult != plazanum)
			break;

		StrSize = MSG_PV_MAX_PLAZA_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Plaza.hName, Plaza.dwConfIndex, plazaname, &StrSize);
		if (pvResult != plazaname)
			break;

		NTSVCInfo("CONF_PV_Get_First_Plaza(), %s = %d", plazaname, *plazanum);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Plaza.hDbCnx != NULL)
	{
		if (Plaza.hStat != NULL)
			_DBCloseStatement(Plaza.hStat);

		DB_CONNECTION_Close();
		Plaza.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Plaza(OUT DWORD *plazanum,
											 OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
											 IN DWORD dwDefaultPlazaNumber)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;

	// if the default plaza number defined do not search for next plaza in data base
	if (dwDefaultPlazaNumber < 1)
	{
		do
		{
			Plaza.dwConfIndex++;
			pvResult = _DBGetVariableItemValue(Plaza.hId, Plaza.dwConfIndex, plazanum, NULL);
			if (pvResult != plazanum)
				break;

			StrSize = MSG_PV_MAX_PLAZA_NAME;
			pvResult = _DBGetVariableItemValueTruncate(Plaza.hName, Plaza.dwConfIndex, plazaname, &StrSize);
			if (pvResult != plazaname)
				break;

			NTSVCInfo("CONF_PV_Get_Next_Plaza(), %s = %d", plazaname, *plazanum);

			bRet = TRUE;
		} while (FALSE);
	}

	if (!bRet && Plaza.hDbCnx != NULL)
	{
		if(Plaza.hStat != NULL)
			_DBCloseStatement(Plaza.hStat);

		DB_CONNECTION_Close();
		Plaza.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

