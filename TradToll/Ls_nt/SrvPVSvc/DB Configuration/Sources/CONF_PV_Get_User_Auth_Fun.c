/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Auth_Fun.c									 */
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
	DB_VAR	*hName_in;
	DB_VAR	*hPassword;
	DB_VAR	*hName_out;
	DB_VAR	*hMatricule;
	DB_VAR	*hFirstname;
	DB_VAR	*hOk;
	DB_VAR	*hProfileId;
	DB_VAR	*hProfileName;
	DB_CNX	*hDbCnx;
}
Auth = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_User_Auth_Fun(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
												IN DWORD matricule_in,
												IN CHAR *password,
												IN DWORD dwPasswordSize,
												IN DWORD func_id,
												OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
												OUT DWORD *matricule_out,
												OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
												OUT DWORD *user_ok,
												OUT DWORD *profile_id,
												OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	// default
	name_out[0]			= '\0';
	matricule_out[0]	= '\0';
	firstname[0]		= '\0';
	profile_name[0]		= '\0';
	*user_ok			= 0;
	*profile_id			= 0;

	Auth.hDbCnx = DB_CONNECTION_Open();
	if (Auth.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PV.PV_DB_Get_User_Auth_Fun( :Name_in, %d, :Password, %d, :Name_out, :Matricule, :FirstName, :UserOk, :ProfileId, :ProfileName ); END;", 
					matricule_in, 
					func_id);

		Auth.hStat = _DBOpenStatement(Auth.hDbCnx, szStatement);
		if (Auth.hStat == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Auth.hName_in = _DBBindPlaceHolderVariable(Auth.hStat, ":Name_in", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Auth.hName_in == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind Name_in : %s", dwErr, szErr);
			break;
		}

		Auth.hPassword = _DBBindPlaceHolderVariable(Auth.hStat, ":Password", DB_TYPE_STR, CONF_MAX_USER_PASSWORD, 0);
		if (Auth.hPassword == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind Password : %s", dwErr, szErr);
			break;
		}

		Auth.hName_out = _DBBindPlaceHolderVariable(Auth.hStat, ":Name_out", DB_TYPE_STR, CONF_MAX_USER_FIRSTNAME, 0);
		if (Auth.hName_out == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind Name_out : %s", dwErr, szErr);
			break;
		}

		Auth.hMatricule = _DBBindPlaceHolderVariable(Auth.hStat, ":Matricule", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Auth.hMatricule == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind Matricule : %s", dwErr, szErr);
			break;
		}

		Auth.hFirstname = _DBBindPlaceHolderVariable(Auth.hStat, ":Firstname", DB_TYPE_STR, CONF_MAX_USER_FIRSTNAME, 0);
		if (Auth.hFirstname == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind Firstname : %s", dwErr, szErr);
			break;
		}

		Auth.hOk = _DBBindPlaceHolderVariable(Auth.hStat, ":UserOk", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Auth.hOk == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind User_ok : %s", dwErr, szErr);
			break;
		}

		Auth.hProfileId = _DBBindPlaceHolderVariable(Auth.hStat, ":ProfileId", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Auth.hProfileId == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind ProfileId : %s", dwErr, szErr);
			break;
		}

		Auth.hProfileName = _DBBindPlaceHolderVariable(Auth.hStat, ":ProfileName", DB_TYPE_STR, CONF_MAX_USER_PROFILE_NAME, 0);
		if (Auth.hProfileName == NULL)
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => Error: %u, bind ProfileName : %s", dwErr, szErr);
			break;
		}

		Auth.dwConfIndex = 0;
		pvResult = _DBSetVariableItemValue(Auth.hName_in, Auth.dwConfIndex, name_in, (DWORD)strlen(name_in) + 1);
		if (pvResult != name_in)
			break;

		pvResult = _DBSetVariableItemValue(Auth.hPassword, Auth.dwConfIndex, password, dwPasswordSize);
		if (pvResult != password)
			break;

		// Execute the request
		if (!_DBExecuteStatement(Auth.hStat))
		{
			_DBGetLastError(Auth.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Auth_Fun() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Auth.dwConfIndex = 0;
		StrSize = MSG_PV_MAX_USER_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Auth.hName_out, Auth.dwConfIndex, name_out, &StrSize);
		if (pvResult != name_out)
			break;

		pvResult = _DBGetVariableItemValue(Auth.hMatricule, Auth.dwConfIndex, matricule_out, NULL);
		if (pvResult != matricule_out)
			break;

		StrSize = MSG_PV_MAX_USER_FIRSTNAME;
		pvResult = _DBGetVariableItemValueTruncate(Auth.hFirstname, Auth.dwConfIndex, firstname, &StrSize);
		if (pvResult != firstname)
			break;

		pvResult = _DBGetVariableItemValue(Auth.hOk, Auth.dwConfIndex, user_ok, NULL);
		if (pvResult != user_ok)
			break;

		pvResult = _DBGetVariableItemValue(Auth.hProfileId, Auth.dwConfIndex, profile_id, NULL);
		if (pvResult != profile_id)
			break;

		StrSize = MSG_PV_MAX_USER_PROFILE_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Auth.hProfileName, Auth.dwConfIndex, profile_name, &StrSize);
		if (pvResult != profile_name)
			break;

		NTSVCInfo("CONF_PV_Get_User_Auth_Fun(),  name = %s, matricule = %d, firstname = %s, userok = %d", name_out, *matricule_out, firstname, *user_ok);

		bRet = TRUE;
	} 
	while (FALSE);

	if (Auth.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_User_Auth_Fun() => disconnect from database!" );
		_DBCloseStatement(Auth.hStat);
		DB_CONNECTION_Close();
		Auth.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/