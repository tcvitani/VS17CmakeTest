/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Currency.c										 */
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
#include <MSG_PV_Define.h>

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
	DB_VAR	*hLabel;
	DB_CNX	*hDbCnx;
}
Currency = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Currency(OUT DWORD *curr_id,
													OUT CHAR curr_label[MSG_PV_MAX_CURRENCY_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Currency.hDbCnx = DB_CONNECTION_Open();
	if (Currency.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Currency() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Currency(:CurrencyId,:CurrencyLabel ); END;");

		Currency.hStat = _DBOpenStatement(Currency.hDbCnx, szStatement);
		if (Currency.hStat == NULL)
		{
			_DBGetLastError(Currency.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Currency() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Currency.hId = _DBBindPlaceHolderVariable(Currency.hStat, ":CurrencyId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_CURRENCY);
		if (Currency.hId == NULL)
		{
			_DBGetLastError(Currency.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Currency() => Error: %u, bind CurrencyId : %s", dwErr, szErr);
			break;
		}

		Currency.hLabel = _DBBindPlaceHolderVariable(Currency.hStat, ":CurrencyLabel", DB_TYPE_STR, CONF_MAX_CURRENCY_LABEL, CONF_MAX_CURRENCY);
		if (Currency.hLabel == NULL)
		{
			_DBGetLastError(Currency.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Currency() => Error: %u, bind CurrencyLabel : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Currency.hStat))
		{
			_DBGetLastError(Currency.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Currency() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Currency.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Currency.hId, Currency.dwConfIndex, curr_id, NULL);
		if (pvResult != curr_id)
			break;

		StrSize = MSG_PV_MAX_CURRENCY_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Currency.hLabel, Currency.dwConfIndex, curr_label, &StrSize);
		if (pvResult != curr_label)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Currency(), %s = %d", curr_label, *curr_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Currency.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Currency() => disconnect from database!" );
		_DBCloseStatement(Currency.hStat);
		DB_CONNECTION_Close();
		Currency.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Currency(OUT DWORD *curr_id,
												   OUT CHAR curr_label[MSG_PV_MAX_CURRENCY_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;
	do
	{
		Currency.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Currency.hId, Currency.dwConfIndex, curr_id, NULL);
		if (pvResult != curr_id)
			break;

		StrSize = MSG_PV_MAX_CURRENCY_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Currency.hLabel, Currency.dwConfIndex, curr_label, &StrSize);
		if (pvResult != curr_label)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Currency(), %s = %d", curr_label, *curr_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Currency.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Currency() => disconnect from database!" );
		_DBCloseStatement(Currency.hStat);
		DB_CONNECTION_Close();
		Currency.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/