/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Class.c											 */
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
#include <Msg_Pv_define.h>

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
Class = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Class(OUT DWORD *cla_id,
												 OUT CHAR cla_name[MSG_PV_MAX_CLASS_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Class.hDbCnx = DB_CONNECTION_Open();
	if (Class.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Class() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Class(:ClassId,:ClassName); END;");

		Class.hStat = _DBOpenStatement(Class.hDbCnx, szStatement);
		if (Class.hStat == NULL)
		{
			_DBGetLastError(Class.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Class() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Class.hId = _DBBindPlaceHolderVariable(Class.hStat, ":ClassId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_CLASS);
		if (Class.hId == NULL)
		{
			_DBGetLastError(Class.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Class() => Error: %u, bind ClassId : %s", dwErr, szErr);
			break;
		}

		Class.hName = _DBBindPlaceHolderVariable(Class.hStat, ":ClassName", DB_TYPE_STR, CONF_MAX_CLASS_NAME, CONF_MAX_CLASS);
		if (Class.hName == NULL)
		{
			_DBGetLastError(Class.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Class() => Error: %u, bind ClassName : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Class.hStat))
		{
			_DBGetLastError(Class.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Class() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Class.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Class.hId, Class.dwConfIndex, cla_id, NULL);
		if (pvResult != cla_id)
			break;

		StrSize = MSG_PV_MAX_LANE_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Class.hName, Class.dwConfIndex, cla_name, &StrSize);
		if (pvResult != cla_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Class(), %s = %d", cla_name, *cla_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Class.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Class() => disconnect from database!" );
		_DBCloseStatement(Class.hStat);
		DB_CONNECTION_Close();
		Class.hDbCnx = NULL;
	}

	return bRet;
}


PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Class(OUT DWORD *cla_id,
												OUT CHAR cla_name[MSG_PV_MAX_CLASS_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;
	do
	{
		Class.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Class.hId, Class.dwConfIndex, cla_id, NULL);
		if (pvResult != cla_id)
			break;

		StrSize = MSG_PV_MAX_LANE_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Class.hName, Class.dwConfIndex, cla_name, &StrSize);
		if (pvResult != cla_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Class(), %s = %d", cla_name, *cla_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Class.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Class() => disconnect from database!" );
		_DBCloseStatement(Class.hStat);
		DB_CONNECTION_Close();
		Class.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/