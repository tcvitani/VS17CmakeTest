/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Payment.c										 */
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
#include <Msg_Pv_define.h>

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
Payment = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Payment(OUT DWORD *pay_id,
												   OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Payment.hDbCnx = DB_CONNECTION_Open();
	if (Payment.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Payment() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Payment(:PaymentId,:PaymentName); END;");

		Payment.hStat = _DBOpenStatement(Payment.hDbCnx, szStatement);
		if (Payment.hStat == NULL)
		{
			_DBGetLastError(Payment.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Payment.hId = _DBBindPlaceHolderVariable(Payment.hStat, ":PaymentId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_PAYMENT);
		if (Payment.hId == NULL)
		{
			_DBGetLastError(Payment.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment() => Error: %u, bind PaymentId : %s", dwErr, szErr);
			break;
		}

		Payment.hName = _DBBindPlaceHolderVariable(Payment.hStat, ":PaymentName", DB_TYPE_STR, CONF_MAX_PAYMENT_NAME, CONF_MAX_PAYMENT);
		if (Payment.hName == NULL)
		{
			_DBGetLastError(Payment.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment() => Error: %u, bind PaymentName : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Payment.hStat))
		{
			_DBGetLastError(Payment.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Payment.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Payment.hId, Payment.dwConfIndex, pay_id, NULL);
		if (pvResult != pay_id)
			break;

		StrSize = MSG_PV_MAX_PAYMENT_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Payment.hName, Payment.dwConfIndex, pay_name, &StrSize);
		if (pvResult != pay_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Payment(), %s = %d", pay_name, *pay_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Payment.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Payment() => disconnect from database!" );
		_DBCloseStatement(Payment.hStat);
		DB_CONNECTION_Close();
		Payment.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Payment(OUT DWORD *pay_id,
												  OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;
	do
	{
		Payment.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Payment.hId, Payment.dwConfIndex, pay_id, NULL);
		if (pvResult != pay_id)
			break;

		StrSize = MSG_PV_MAX_PAYMENT_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Payment.hName, Payment.dwConfIndex, pay_name, &StrSize);
		if (pvResult != pay_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Payment(), %s = %d", pay_name, *pay_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Payment.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Payment() => disconnect from database!" );
		_DBCloseStatement(Payment.hStat);
		DB_CONNECTION_Close();
		Payment.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

