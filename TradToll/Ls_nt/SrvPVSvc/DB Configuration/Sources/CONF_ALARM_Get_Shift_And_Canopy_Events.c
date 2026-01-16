/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Shift_Mode_And_Vault_Events.c					 */
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
	DB_VAR	*hTP;
	DB_VAR	*hSTPId;
	DB_VAR	*hSTPMode;
	DB_VAR	*hTC;
	DB_VAR	*hSTCMarquise;
	DB_CNX	*hDbCnx;
}
Shift_Mode_Vult_Events = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_ALARM_Get_Shift_Mode_And_Vault_Events(OUT DWORD *pShiftAndModeEvtType,
																 OUT DWORD *pShiftEvtSubType,
																 OUT DWORD *pModeEvtSubType,
																 OUT DWORD *pVault1EvtType,
																 OUT DWORD *pVault2EvtType)
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	CHAR	szStatement[1024]	= { 0 };
	DWORD	dwErr				= 0;
	BOOL	bRet				= FALSE;

	Shift_Mode_Vult_Events.hDbCnx = DB_CONNECTION_Open();
	if (Shift_Mode_Vult_Events.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN ALARM.ALARM_DB_Get_Shift_Canopy_Evts(:TP, :STPId, :STPMode, :TC, :STCMarquise); END;");

		Shift_Mode_Vult_Events.hStat = _DBOpenStatement(Shift_Mode_Vult_Events.hDbCnx, szStatement);
		if (Shift_Mode_Vult_Events.hStat == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_First_Shift_And_Canopy_Events() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Shift_Mode_Vult_Events.hTP = _DBBindPlaceHolderVariable(Shift_Mode_Vult_Events.hStat, ":TP", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Shift_Mode_Vult_Events.hTP == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: %u, bind TP : %s", dwErr, szErr);
			break;
		}

		Shift_Mode_Vult_Events.hSTPId = _DBBindPlaceHolderVariable(Shift_Mode_Vult_Events.hStat, ":STPId", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Shift_Mode_Vult_Events.hSTPId == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: %u, bind STPId : %s", dwErr, szErr);
			break;
		}

		Shift_Mode_Vult_Events.hSTPMode = _DBBindPlaceHolderVariable(Shift_Mode_Vult_Events.hStat, ":STPMode", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Shift_Mode_Vult_Events.hSTPMode == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: %u, bind STPMode : %s", dwErr, szErr);
			break;
		}

		Shift_Mode_Vult_Events.hTC = _DBBindPlaceHolderVariable(Shift_Mode_Vult_Events.hStat, ":TC", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Shift_Mode_Vult_Events.hTC == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: %u, bind TC : %s", dwErr, szErr);
			break;
		}

		Shift_Mode_Vult_Events.hSTCMarquise = _DBBindPlaceHolderVariable(Shift_Mode_Vult_Events.hStat, ":STCMarquise", DB_TYPE_INT, sizeof(DWORD), 0);
		if (Shift_Mode_Vult_Events.hSTCMarquise == NULL)
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => Error: %u, bind STCMarquise : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Shift_Mode_Vult_Events.hStat))
		{
			_DBGetLastError(Shift_Mode_Vult_Events.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Shift_Mode_Vult_Events.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Shift_Mode_Vult_Events.hTP, Shift_Mode_Vult_Events.dwConfIndex, pShiftAndModeEvtType, NULL);
		if (pvResult != pShiftAndModeEvtType)
			break;

		pvResult = _DBGetVariableItemValue(Shift_Mode_Vult_Events.hSTPId, Shift_Mode_Vult_Events.dwConfIndex, pShiftEvtSubType, NULL);
		if (pvResult != pShiftEvtSubType)
			break;

		pvResult = _DBGetVariableItemValue(Shift_Mode_Vult_Events.hSTPMode, Shift_Mode_Vult_Events.dwConfIndex, pModeEvtSubType, NULL);
		if (pvResult != pModeEvtSubType)
			break;

		pvResult = _DBGetVariableItemValue(Shift_Mode_Vult_Events.hTC, Shift_Mode_Vult_Events.dwConfIndex, pVault1EvtType, NULL);
		if (pvResult != pVault1EvtType)
			break;

		pvResult = _DBGetVariableItemValue(Shift_Mode_Vult_Events.hSTCMarquise, Shift_Mode_Vult_Events.dwConfIndex, pVault2EvtType, NULL);
		if (pvResult != pVault2EvtType)
			break;

		NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events(), poste=%d id=%d mode=%d coffre1=%d coffre2=%d ", 
			*pShiftAndModeEvtType, *pShiftEvtSubType, *pModeEvtSubType, *pVault1EvtType, *pVault2EvtType);

		bRet = TRUE;
	} 
	while (FALSE);

	if (Shift_Mode_Vult_Events.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_ALARM_Get_First_Shift_And_Canopy_Events() => disconnect from database!" );
		_DBCloseStatement(Shift_Mode_Vult_Events.hStat);
		DB_CONNECTION_Close();
		Shift_Mode_Vult_Events.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/