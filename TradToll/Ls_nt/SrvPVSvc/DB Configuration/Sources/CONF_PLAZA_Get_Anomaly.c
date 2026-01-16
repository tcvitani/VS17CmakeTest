/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Anomaly.c										 */
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
Anomaly = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Anomaly(OUT DWORD *ano_id,
												   OUT CHAR ano_name[MSG_PV_MAX_ANOMALY_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Anomaly.hDbCnx = DB_CONNECTION_Open();
	if (Anomaly.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Anomaly() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Anomaly(:AnomalyId,:AnomalyName); END;");

		Anomaly.hStat = _DBOpenStatement(Anomaly.hDbCnx, szStatement);
		if (Anomaly.hStat == NULL)
		{
			_DBGetLastError(Anomaly.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Anomaly() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Anomaly.hId = _DBBindPlaceHolderVariable(Anomaly.hStat, ":AnomalyId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_ANOMALY);
		if (Anomaly.hId == NULL)
		{
			_DBGetLastError(Anomaly.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Anomaly() => Error: %u, bind AnomalyId : %s", dwErr, szErr);
			break;
		}

		Anomaly.hName = _DBBindPlaceHolderVariable(Anomaly.hStat, ":AnomalyName", DB_TYPE_STR, CONF_MAX_ANOMALY_NAME, CONF_MAX_ANOMALY);
		if (Anomaly.hName == NULL)
		{
			_DBGetLastError(Anomaly.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Anomaly() => Error: %u, bind AnomalyName : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Anomaly.hStat))
		{
			_DBGetLastError(Anomaly.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Anomaly() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Anomaly.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Anomaly.hId, Anomaly.dwConfIndex, ano_id, NULL);
		if (pvResult != ano_id)
			break;

		StrSize = MSG_PV_MAX_ANOMALY_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Anomaly.hName, Anomaly.dwConfIndex, ano_name, &StrSize);
		if (pvResult != ano_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Anomaly(), %s = %d", ano_name, *ano_id);

		bRet = TRUE;
	}
	while (FALSE);

	if (!bRet && Anomaly.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Anomaly() => disconnect from database!" );
		_DBCloseStatement(Anomaly.hStat);
		DB_CONNECTION_Close();
		Anomaly.hDbCnx = NULL;
	}

	return bRet;
}


PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Anomaly(OUT DWORD *ano_id,
												  OUT CHAR ano_name[MSG_PV_MAX_ANOMALY_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;
	do
	{
		Anomaly.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Anomaly.hId, Anomaly.dwConfIndex, ano_id, NULL);
		if (pvResult != ano_id)
			break;

		StrSize = MSG_PV_MAX_ANOMALY_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Anomaly.hName, Anomaly.dwConfIndex, ano_name, &StrSize);
		if (pvResult != ano_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Anomaly(), %s = %d", ano_name, *ano_id);

		bRet = TRUE;
	}
	while (FALSE);

	if (!bRet && Anomaly.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Anomaly() => disconnect from database!" );
		_DBCloseStatement(Anomaly.hStat);
		DB_CONNECTION_Close();
		Anomaly.hDbCnx = NULL;
	}

	return bRet;
}


/*-------------------------------- END OF FILE ------------------------------*/
