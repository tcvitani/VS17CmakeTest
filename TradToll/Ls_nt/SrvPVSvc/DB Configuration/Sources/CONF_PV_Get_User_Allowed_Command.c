/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Allowed_Command.c								 */
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
	DB_VAR	*hName;
	DB_VAR	*hCmdId;
	DB_VAR	*hCmdMode;
	DB_VAR	*hCmdLabel;
	DB_CNX	*hDbCnx;
}
Allowed_Command = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Allowed_Command(IN CHAR name[MSG_PV_MAX_USER_NAME],
															 IN DWORD matricule,
															 OUT DWORD *cmd_id,
															 OUT DWORD * cmd_mode,
															 OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Allowed_Command.hDbCnx = DB_CONNECTION_Open();
	if (Allowed_Command.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PV.PV_DB_Get_Allowed_Command(:Name, %d, :CmdId, :CmdLabel, :CmdMode); END;", 
					matricule);

		Allowed_Command.hStat = _DBOpenStatement(Allowed_Command.hDbCnx, szStatement);
		if (Allowed_Command.hStat == NULL)
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Allowed_Command.hName = _DBBindPlaceHolderVariable(Allowed_Command.hStat, ":Name", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Allowed_Command.hName == NULL)
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Allowed_Command() => Error: %u, bind Name : %s", dwErr, szErr);
			break;
		}

		Allowed_Command.hCmdId = _DBBindPlaceHolderVariable(Allowed_Command.hStat, ":CmdId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_USER_CMD);
		if (Allowed_Command.hCmdId == NULL)
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => Error: %u, bind CmdId : %s", dwErr, szErr);
			break;
		}

		Allowed_Command.hCmdMode = _DBBindPlaceHolderVariable(Allowed_Command.hStat, ":CmdMode", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_USER_CMD);
		if (Allowed_Command.hCmdMode == NULL)
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => Error: %u, bind CmdMode : %s", dwErr, szErr);
			break;
		}

		Allowed_Command.hCmdLabel = _DBBindPlaceHolderVariable(Allowed_Command.hStat, ":CmdLabel", DB_TYPE_STR, CONF_MAX_CMD_NAME, CONF_MAX_USER_CMD);
		if (Allowed_Command.hCmdLabel == NULL)
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => Error: %u, bind CmdLabel : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Allowed_Command.hStat))
		{
			_DBGetLastError(Allowed_Command.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Allowed_Command.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Allowed_Command.hCmdId, Allowed_Command.dwConfIndex, cmd_id, NULL);
		if (pvResult != cmd_id)
			break;

		pvResult = _DBGetVariableItemValue(Allowed_Command.hCmdMode, Allowed_Command.dwConfIndex, cmd_mode, NULL);
		if (pvResult != cmd_mode)
			break;

		StrSize = MSG_PV_MAX_COMMAND_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Allowed_Command.hCmdLabel, Allowed_Command.dwConfIndex, cmd_label, &StrSize);
		if (pvResult != cmd_label)
			break;

		NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command(), cmdid = %d cmdmode = %d cmdlabel = %s", *cmd_id, *cmd_mode, cmd_label);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Allowed_Command.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Allowed_Command() => disconnect from database!" );
		_DBCloseStatement(Allowed_Command.hStat);
		DB_CONNECTION_Close();
		Allowed_Command.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Allowed_Command(OUT DWORD *cmd_id,
															OUT DWORD * cmd_mode,
															OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		 = 0;

	bRet = FALSE;
	do
	{
		Allowed_Command.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Allowed_Command.hCmdId, Allowed_Command.dwConfIndex, cmd_id, NULL);
		if (pvResult != cmd_id)
			break;

		pvResult = _DBGetVariableItemValue(Allowed_Command.hCmdMode, Allowed_Command.dwConfIndex, cmd_mode, NULL);
		if (pvResult != cmd_mode)
			break;

		StrSize = MSG_PV_MAX_COMMAND_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Allowed_Command.hCmdLabel, Allowed_Command.dwConfIndex, cmd_label, &StrSize);
		if (pvResult != cmd_label)
			break;

		NTSVCInfo("CONF_PV_Get_Next_User_Allowed_Command(), cmdid = %d cmdmode = %d cmdlabel = %s", *cmd_id, *cmd_mode, cmd_label);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Allowed_Command.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_User_Allowed_Command() => disconnect from database!" );
		_DBCloseStatement(Allowed_Command.hStat);
		DB_CONNECTION_Close();
		Allowed_Command.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/