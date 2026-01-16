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
	DB_VAR	*hCoinId;
	DB_VAR	*hCurrId;
	DB_VAR	*hCoinLabel;
	DB_VAR	*hValue;
	DB_VAR	*hBag;
	DB_CNX	*hDbCnx;
}
Coin = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Coin(OUT DWORD *coin_id,
												OUT DWORD *currency_id,
												OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
												OUT DOUBLE *value,
												OUT DWORD *bag)
{
	void	*pvResult					= NULL;
	DWORD	dwErr						= 0;
	CHAR	szErr[200]					= { 0 };
	char	szStatement[1024]			= { 0 };
	char	StrValue[CONF_MAX_VALUE]	= { 0 };
	BOOL	bRet						= FALSE;
	DWORD	StrSize						= 0;

	Coin.hDbCnx = DB_CONNECTION_Open();
	if (Coin.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Coin( :CoinId, :CurrId, :CoinLabel, :Value, :Bag ); END;");

		Coin.hStat = _DBOpenStatement(Coin.hDbCnx, szStatement);
		if (Coin.hStat == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Coin.hCoinId = _DBBindPlaceHolderVariable(Coin.hStat, ":CoinId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_COIN);
		if (Coin.hCoinId == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: %u, bind CoinId : %s", dwErr, szErr);
			break;
		}

		Coin.hCurrId = _DBBindPlaceHolderVariable(Coin.hStat, ":CurrId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_COIN);
		if (Coin.hCurrId == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: %u, bind CurrId : %s", dwErr, szErr);
			break;
		}

		Coin.hCoinLabel = _DBBindPlaceHolderVariable(Coin.hStat, ":CoinLabel", DB_TYPE_STR, CONF_MAX_COIN_LABEL, CONF_MAX_COIN);
		if (Coin.hCoinLabel == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: %u, bind CoinLabel : %s", dwErr, szErr);
			break;
		}

		Coin.hValue = _DBBindPlaceHolderVariable(Coin.hStat, ":Value", DB_TYPE_STR, CONF_MAX_VALUE, CONF_MAX_COIN);
		if (Coin.hValue == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: %u, bind CoinValue : %s", dwErr, szErr);
			break;
		}

		Coin.hBag = _DBBindPlaceHolderVariable(Coin.hStat, ":Bag", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_COIN);
		if (Coin.hBag == NULL)
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => Error: %u, bind Bag : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Coin.hStat))
		{
			_DBGetLastError(Coin.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Coin() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Coin.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Coin.hCoinId, Coin.dwConfIndex, coin_id, NULL);
		if (pvResult != coin_id)
			break;

		pvResult = _DBGetVariableItemValue(Coin.hCurrId, Coin.dwConfIndex, currency_id, NULL);
		if (pvResult != currency_id)
			break;

		StrSize = MSG_PV_MAX_COIN_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Coin.hCoinLabel, Coin.dwConfIndex, coin_label, &StrSize);
		if (pvResult != coin_label)
			break;

		pvResult = _DBGetVariableItemValue(Coin.hValue, Coin.dwConfIndex, StrValue, NULL);
		if (pvResult != StrValue)
			break;

		*value = atof(StrValue);
		pvResult = _DBGetVariableItemValue(Coin.hBag, Coin.dwConfIndex, bag, NULL);
		if (pvResult != bag)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Coin(), CoinId = %lu, CurrencyId = %lu, Coin = %s, Value = %f, Bag = %lu",
			*coin_id, *currency_id, coin_label, *value, *bag);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Coin.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Coin() => disconnect from database!" );
		_DBCloseStatement(Coin.hStat);
		DB_CONNECTION_Close();
		Coin.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Coin(OUT DWORD *coin_id,
											   OUT DWORD *currency_id,
											   OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
											   OUT DOUBLE * value,
											   OUT DWORD * bag)
{
	void	*pvResult					= NULL;
	BOOL	bRet						= FALSE;
	CHAR	StrValue[CONF_MAX_VALUE]	= { 0 };
	CHAR	szErr[200]					= { 0 };
	DWORD	StrSize						= 0;

	bRet = FALSE;
	do
	{
		Coin.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Coin.hCoinId, Coin.dwConfIndex, coin_id, NULL);
		if (pvResult != coin_id)
			break;

		pvResult = _DBGetVariableItemValue(Coin.hCurrId, Coin.dwConfIndex, currency_id, NULL);
		if (pvResult != currency_id)
			break;

		StrSize = MSG_PV_MAX_COIN_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Coin.hCoinLabel, Coin.dwConfIndex, coin_label, &StrSize);
		if (pvResult != coin_label)
			break;

		pvResult = _DBGetVariableItemValue(Coin.hValue, Coin.dwConfIndex, StrValue, NULL);
		if (pvResult != StrValue)
			break;

		*value = atof(StrValue);
		pvResult = _DBGetVariableItemValue(Coin.hBag, Coin.dwConfIndex, bag, NULL);
		if (pvResult != bag)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Coin(), CoinId = %lu, CurrencyId = %lu, Coin = %s, Value = %f, Bag = %lu",
			*coin_id, *currency_id, coin_label, *value, *bag);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Coin.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Coin() => disconnect from database!" );
		_DBCloseStatement(Coin.hStat);
		DB_CONNECTION_Close();
		Coin.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
