/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Alarm.c											 */
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
	DB_VAR	*hType;
	DB_VAR	*hSubType;
	DB_CNX	*hDbCnx;
}
Alarm = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_ALARM_Get_First_Alarm(OUT DWORD *alarm_type, OUT DWORD *alarm_sub_type)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	CHAR	szStatement[1024]	= { 0 };
	DWORD	dwErr				= 0;
	BOOL	bRet				= FALSE;

	Alarm.hDbCnx = DB_CONNECTION_Open();
	if (Alarm.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_ALARM_Get_First_Alarm() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN ALARM.ALARM_DB_Get_Alarm(:Type, :SubType); END;");

		Alarm.hStat = _DBOpenStatement(Alarm.hDbCnx, szStatement);
		if (Alarm.hStat == NULL)
		{
			_DBGetLastError(Alarm.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Alarm() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Alarm.hType = _DBBindPlaceHolderVariable(Alarm.hStat, ":Type", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_ALARM);
		if (Alarm.hType == NULL)
		{
			_DBGetLastError(Alarm.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Alarm() => Error: %u, bind AlarmType: %s", dwErr, szErr);
			break;
		}

		Alarm.hSubType = _DBBindPlaceHolderVariable(Alarm.hStat, ":SubType", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_ALARM);
		if (Alarm.hSubType == NULL)
		{
			_DBGetLastError(Alarm.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Alarm() => Error: %u, bind AlarmSubType: %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Alarm.hStat))
		{
			_DBGetLastError(Alarm.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Alarm() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Alarm.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Alarm.hType, Alarm.dwConfIndex, alarm_type, NULL);
		if (pvResult != alarm_type)
			break;

		pvResult = _DBGetVariableItemValue(Alarm.hSubType, Alarm.dwConfIndex, alarm_sub_type, NULL);
		if (pvResult != alarm_sub_type)
			break;

		NTSVCInfo("CONF_ALARM_Get_First_Alarm(), AlarmType = %d, AlarmSubType = %d", *alarm_type, *alarm_sub_type);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Alarm.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_ALARM_Get_First_Alarm() => disconnect from database!" );
		_DBCloseStatement(Alarm.hStat);
		DB_CONNECTION_Close();
		Alarm.hDbCnx = NULL;
	}

	return bRet;
}


PROTECTED BOOL WINAPI CONF_ALARM_Get_Next_Alarm(OUT DWORD * alarm_type, OUT DWORD * alarm_sub_type)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	bRet = FALSE;
	do
	{
		Alarm.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Alarm.hType, Alarm.dwConfIndex, alarm_type, NULL);
		if (pvResult != alarm_type)
			break;

		pvResult = _DBGetVariableItemValue(Alarm.hSubType, Alarm.dwConfIndex, alarm_sub_type, NULL);
		if (pvResult != alarm_sub_type)
			break;

		NTSVCInfo("CONF_ALARM_Get_Next_Alarm(), AlarmType = %d, AlarmSubType = %d", *alarm_type, *alarm_sub_type);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Alarm.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_ALARM_Get_Next_Alarm() => disconnect from database!" );
		_DBCloseStatement(Alarm.hStat);
		DB_CONNECTION_Close();
		Alarm.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/