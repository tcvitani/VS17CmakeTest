/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Login.c										 */
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
	DB_STMT	*hStat;
	DB_VAR	*hName_in;
	DB_VAR	*hPassword;
	DB_VAR	*hName_out;
	DB_VAR	*hMatricule;
	DB_VAR	*hFirstname;
	DB_VAR	*hOk;
	DB_VAR	*hMax;
	DB_VAR	*hProfileId;
	DB_VAR	*hProfileName;
	DB_CNX	*hDbCnx;
} 
Login = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_User_Login(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
											 IN DWORD matricule_in,
											 IN CHAR *password,
											 IN DWORD dwPasswordSize,
											 OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
											 OUT DWORD *matricule_out,
											 OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
											 OUT DWORD *user_ok,
											 OUT DWORD *profile_max_connections,
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
	name_out[0]					= '\0';
	matricule_out[0]			= '\0';
	firstname[0]				= '\0';
	profile_name[0]				= '\0';
	*user_ok					= 0;
	*profile_max_connections	= 0;
	*profile_id					= 0;

	Login.hDbCnx = DB_CONNECTION_Open();
	if (Login.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_User_Login() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PV.PV_DB_Get_User_Login( :Name_in, %d, :Password, :Name_out, :Matricule, :Firstname, :LoginOk, :LoginMax, :ProfileId, :ProfileName ); END;", 
					matricule_in);


		Login.hStat = _DBOpenStatement(Login.hDbCnx, szStatement);
		if (Login.hStat == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Login.hName_in = _DBBindPlaceHolderVariable(Login.hStat, ":Name_in", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Login.hName_in == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind Name_in : %s", dwErr, szErr);
			break;
		}

		Login.hPassword = _DBBindPlaceHolderVariable(Login.hStat, ":Password", DB_TYPE_STR, CONF_MAX_USER_PASSWORD, 0);
		if (Login.hPassword == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind Password : %s", dwErr, szErr);
			break;
		}

		Login.hName_out = _DBBindPlaceHolderVariable(Login.hStat, ":Name_out", DB_TYPE_STR, CONF_MAX_USER_NAME, 0);
		if (Login.hName_out == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind Name_out : %s", dwErr, szErr);
			break;
		}

		Login.hMatricule = _DBBindPlaceHolderVariable(Login.hStat, ":Matricule", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Login.hMatricule == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind Matricule : %s", dwErr, szErr);
			break;
		}

		Login.hFirstname = _DBBindPlaceHolderVariable(Login.hStat, ":Firstname", DB_TYPE_STR, CONF_MAX_USER_FIRSTNAME, 0);
		if (Login.hFirstname == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind Firstname : %s", dwErr, szErr);
			break;
		}

		Login.hOk = _DBBindPlaceHolderVariable(Login.hStat, ":LoginOk", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Login.hOk == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind LoginOk : %s", dwErr, szErr);
			break;
		}

		Login.hMax = _DBBindPlaceHolderVariable(Login.hStat, ":LoginMax", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Login.hMax == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind LoginMax : %s", dwErr, szErr);
			break;
		}

		Login.hProfileId = _DBBindPlaceHolderVariable(Login.hStat, ":ProfileId", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Login.hProfileId == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind ProfileId : %s", dwErr, szErr);
			break;
		}

		Login.hProfileName = _DBBindPlaceHolderVariable(Login.hStat, ":ProfileName", DB_TYPE_STR, CONF_MAX_USER_PROFILE_NAME, 0);
		if (Login.hProfileName == NULL)
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => Error: %u, bind ProfileName : %s", dwErr, szErr);
			break;
		}

		Login.dwConfIndex = 0;
		pvResult = _DBSetVariableItemValue(Login.hName_in, Login.dwConfIndex, name_in, (DWORD)strlen(name_in) + 1);
		if (pvResult != name_in)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != name_in");
			break;
		}

		pvResult = _DBSetVariableItemValue(Login.hPassword, Login.dwConfIndex, password, dwPasswordSize);
		if (pvResult != password)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != password");
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Login.hStat))
		{
			_DBGetLastError(Login.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_User_Login() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Login.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Login.hOk, Login.dwConfIndex, user_ok, NULL);
		if (pvResult != user_ok)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != user_ok");
			break;
		}

		pvResult = _DBGetVariableItemValue(Login.hMax, Login.dwConfIndex, profile_max_connections, NULL);
		if (pvResult != profile_max_connections)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != profile_max_connections");
			break;
		}

		StrSize = MSG_PV_MAX_USER_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Login.hName_out, Login.dwConfIndex, name_out, &StrSize);
		if (pvResult != name_out)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != name_out");
			break;
		}

		pvResult = _DBGetVariableItemValue(Login.hMatricule, Login.dwConfIndex, matricule_out, NULL);
		if (pvResult != matricule_out)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != matricule_out");
			break;
		}

		StrSize = MSG_PV_MAX_USER_FIRSTNAME;
		pvResult = _DBGetVariableItemValueTruncate(Login.hFirstname, Login.dwConfIndex, firstname, &StrSize);
		if (pvResult != firstname)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != firstname");
			break;
		}

		pvResult = _DBGetVariableItemValue(Login.hProfileId, Login.dwConfIndex, profile_id, NULL);
		if (pvResult != profile_id)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != profile_id");
			break;
		}

		StrSize = MSG_PV_MAX_USER_PROFILE_NAME;
		pvResult = _DBGetVariableItemValueTruncate(Login.hProfileName, Login.dwConfIndex, profile_name, &StrSize);
		if (pvResult != profile_name)
		{
			NTSVCInfo("CONF_PV_Get_User_Login(),pvResult != name_out");
			break;
		}

		NTSVCInfo("CONF_PV_Get_User_Login(), Name = %s, Matricule = %d, Firstname = %s, Profile = %s, Userok = %d, Usermax = %d",
			name_out, *matricule_out, firstname, profile_name, *user_ok, *profile_max_connections);

		bRet = TRUE;
	}
	while (FALSE);

	if (Login.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_User_Login() => disconnect from database!" );
		_DBCloseStatement(Login.hStat);
		DB_CONNECTION_Close();
		Login.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
