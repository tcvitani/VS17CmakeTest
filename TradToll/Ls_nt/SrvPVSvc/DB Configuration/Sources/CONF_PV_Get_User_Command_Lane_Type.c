/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Command_Lane_Type.c							 */
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
	DB_VAR	*hTypeId;
	DB_CNX	*hDbCnx;
} 
Command_Lane_Type = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Command_Lane_Type(IN CHAR name[MSG_PV_MAX_USER_NAME],
															   IN DWORD matricule,
															   IN DWORD cmd_id,
															   OUT DWORD *type_id)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Command_Lane_Type.hDbCnx = DB_CONNECTION_Open();
	if (Command_Lane_Type.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_User_Command_Lane_Type() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PV.PV_DB_Get_Command_Lane_Type(:Name, %d, %d, :TypeId ); END;",
					matricule, 
					cmd_id);

		Command_Lane_Type.hStat = _DBOpenStatement(Command_Lane_Type.hDbCnx, szStatement);
		if (Command_Lane_Type.hStat == NULL)
		{
			_DBGetLastError(Command_Lane_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Command_Lane_Type() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Command_Lane_Type.hName = _DBBindPlaceHolderVariable(Command_Lane_Type.hStat, ":Name", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Command_Lane_Type.hName == NULL)
		{
			_DBGetLastError(Command_Lane_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Command_Lane_Type() => Error: %u, bind Name : %s", dwErr, szErr);
			break;
		}

		Command_Lane_Type.hTypeId = _DBBindPlaceHolderVariable(Command_Lane_Type.hStat, ":TypeId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_CMD_LANE_TYPE);
		if (Command_Lane_Type.hTypeId == NULL)
		{
			_DBGetLastError(Command_Lane_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Command_Lane_Type() => Error: %u, bind TypeId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Command_Lane_Type.hStat))
		{
			_DBGetLastError(Command_Lane_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Command_Lane_Type() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Command_Lane_Type.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Command_Lane_Type.hTypeId, Command_Lane_Type.dwConfIndex, type_id, NULL);
		if (pvResult != type_id)
			break;

		NTSVCInfo("CONF_PV_Get_First_User_Command_Lane_Type(), typeid = %d", *type_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Command_Lane_Type.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Command_Lane_Type() => disconnect from database!" );
		_DBCloseStatement(Command_Lane_Type.hStat);
		DB_CONNECTION_Close();
		Command_Lane_Type.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Command_Lane_Type(OUT DWORD *type_id)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Command_Lane_Type.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Command_Lane_Type.hTypeId, Command_Lane_Type.dwConfIndex, type_id, NULL);
		if (pvResult != type_id)
			break;

		NTSVCInfo("CONF_PV_Get_Next_User_Command_Lane_Type(), typeid = %d ", *type_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Command_Lane_Type.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Command_Lane_Type() => disconnect from database!" );
		_DBCloseStatement(Command_Lane_Type.hStat);
		DB_CONNECTION_Close();
		Command_Lane_Type.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/