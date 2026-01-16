/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Lane_Device.c										 */
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

PRIVATE  struct
{
	DWORD	dwConfIndex;
	DB_STMT	*hStat;
	DB_VAR	*hId;
	DB_CNX	*hDbCnx;
}
Device = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Lane_Device(IN DWORD plazanum,
													IN DWORD lanenum,
													OUT DWORD *dev_id)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;

	Device.hDbCnx = DB_CONNECTION_Open();
	if (Device.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PV_Get_First_Lane_Device() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, 
					_countof(szStatement), 
					sizeof(szStatement),
					"BEGIN PLAZA.PLAZA_DB_PV_Get_Lane_Device(%d, %d, :DeviceId); END;", 
					plazanum, 
					lanenum);

		Device.hStat = _DBOpenStatement(Device.hDbCnx, szStatement);
		if (Device.hStat == NULL)
		{
			_DBGetLastError(Device.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Lane_Device() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Device.hId = _DBBindPlaceHolderVariable(Device.hStat, ":DeviceId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_DEVICES);
		if (Device.hId == NULL)
		{
			_DBGetLastError(Device.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Lane_Device() => Error: %u, bind DeviceId : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Device.hStat))
		{
			_DBGetLastError(Device.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PV_Get_First_Lane_Device() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		Device.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Device.hId, Device.dwConfIndex, dev_id, NULL);
		if (pvResult != dev_id)
			break;

		NTSVCInfo("CONF_PV_Get_First_Lane_Device(), %d = %d", lanenum, *dev_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Device.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_First_Lane_Device() => disconnect from database" );
		_DBCloseStatement(Device.hStat);
		DB_CONNECTION_Close();
		Device.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Lane_Device(OUT DWORD *dev_id)
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;

	// 

	bRet = FALSE;
	do
	{
		Device.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Device.hId, Device.dwConfIndex, dev_id, NULL);
		if (pvResult != dev_id)
			break;

		NTSVCInfo("CONF_PV_Get_Next_Lane_Device(), %d", *dev_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Device.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PV_Get_Next_Lane_Device() => disconnect from database!" );
		_DBCloseStatement(Device.hStat);
		DB_CONNECTION_Close();
		Device.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/