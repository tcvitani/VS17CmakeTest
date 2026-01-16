/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Forbidden_Function.c							 */
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
	DB_VAR	*hName;
	DB_VAR	*hFuncId;
	DB_VAR	*hFuncMode;
	DB_CNX	*hDbCnx;
}
Forbidden_Function = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Forbidden_Function(IN char *name,
																IN DWORD matricule,
																OUT DWORD *func_id,
																OUT DWORD *func_mode)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Forbidden_Function.hDbCnx = DB_CONNECTION_Open();
	if (Forbidden_Function.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PV.PV_DB_Get_Forbidden_Function(:Name, %d, :FuncId , :FuncMode); END;", 
					matricule);

		Forbidden_Function.hStat = _DBOpenStatement(Forbidden_Function.hDbCnx, szStatement);
		if (Forbidden_Function.hStat == NULL)
		{
			_DBGetLastError(Forbidden_Function.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Forbidden_Function.hName = _DBBindPlaceHolderVariable(Forbidden_Function.hStat, ":Name", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Forbidden_Function.hName == NULL)
		{
			_DBGetLastError(Forbidden_Function.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Forbidden_Function() => Error: %u, bind Name : %s", dwErr, szErr);
			break;
		}

		Forbidden_Function.hFuncId = _DBBindPlaceHolderVariable(Forbidden_Function.hStat, ":FuncId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_USER_FUNC);
		if (Forbidden_Function.hFuncId == NULL)
		{
			_DBGetLastError(Forbidden_Function.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => Error: %u, bind FuncId : %s", dwErr, szErr);
			break;
		}

		Forbidden_Function.hFuncMode = _DBBindPlaceHolderVariable(Forbidden_Function.hStat, ":FuncMode", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_USER_FUNC);
		if (Forbidden_Function.hFuncMode == NULL)
		{
			_DBGetLastError(Forbidden_Function.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => Error: %u, bind FuncMode : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Forbidden_Function.hStat))
		{
			_DBGetLastError(Forbidden_Function.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Forbidden_Function.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Forbidden_Function.hFuncId, Forbidden_Function.dwConfIndex, func_id, NULL);
		if (pvResult != func_id)
			break;

		pvResult = _DBGetVariableItemValue(Forbidden_Function.hFuncMode, Forbidden_Function.dwConfIndex, func_mode, NULL);
		if (pvResult != func_mode)
			break;

		NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function(), funcid = %d funcmode = %d", *func_id, *func_mode);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Forbidden_Function.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Forbidden_Function() => disconnect from database!" );
		_DBCloseStatement(Forbidden_Function.hStat);
		DB_CONNECTION_Close();
		Forbidden_Function.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Forbidden_Function(OUT DWORD *func_id,
															   OUT DWORD *func_mode)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Forbidden_Function.dwConfIndex++;

		pvResult = _DBGetVariableItemValue(Forbidden_Function.hFuncId, Forbidden_Function.dwConfIndex, func_id, NULL);
		if (pvResult != func_id)
			break;

		pvResult = _DBGetVariableItemValue(Forbidden_Function.hFuncMode, Forbidden_Function.dwConfIndex, func_mode, NULL);

		if (pvResult != func_mode)
			break;

		NTSVCInfo("CONF_PV_Get_Next_User_Forbidden_Function(), funcid = %d funcmode = %d", *func_id, *func_mode);

		bRet = TRUE;
	} while (FALSE);

	if (!bRet && Forbidden_Function.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Forbidden_Function() => disconnect from database!" );
		_DBCloseStatement(Forbidden_Function.hStat);
		DB_CONNECTION_Close();
		Forbidden_Function.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/