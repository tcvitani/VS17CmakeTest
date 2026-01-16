/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Payment_Sub_Type.c								 */
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
Pmt_Sub_Type = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Payment_Sub_Type(IN DWORD pay_id,
															OUT DWORD *pay_sub_id,
															OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	void	*pvResult			= NULL;
	CHAR	szErr[200]			= { 0 };
	DWORD	dwErr				= 0;
	CHAR	szStatement[1024]	= { 0 };
	BOOL	bRet				= FALSE;
	DWORD	StrSize				= 0;

	Pmt_Sub_Type.hDbCnx = DB_CONNECTION_Open();
	if (Pmt_Sub_Type.hDbCnx == NULL)
	{
		NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type() => Error: connection to database impossible!");
		return FALSE;
	}

	bRet = FALSE;
	do
	{
		// Prepare the request
		_snprintf_s(szStatement, _countof(szStatement), sizeof(szStatement),
			"BEGIN PLAZA.PLAZA_DB_Get_Payment_Sub_Type( %d, :PaymentId,:PaymentName); END;", pay_id);

		Pmt_Sub_Type.hStat = _DBOpenStatement(Pmt_Sub_Type.hDbCnx, szStatement);
		if (Pmt_Sub_Type.hStat == NULL)
		{
			_DBGetLastError(Pmt_Sub_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type() => _DBOpenStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Associate variables
		Pmt_Sub_Type.hId = _DBBindPlaceHolderVariable(Pmt_Sub_Type.hStat, ":PaymentId", DB_TYPE_INT, sizeof(DWORD), CONF_MAX_PAYMENT);

		if (Pmt_Sub_Type.hId == NULL)
		{
			_DBGetLastError(Pmt_Sub_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type() => Error: %u, bind PaymentId : %s", dwErr, szErr);
			break;
		}

		Pmt_Sub_Type.hName = _DBBindPlaceHolderVariable(Pmt_Sub_Type.hStat, ":PaymentName", DB_TYPE_STR, CONF_MAX_PAYMENT_NAME, CONF_MAX_PAYMENT);
		if (Pmt_Sub_Type.hName == NULL)
		{
			_DBGetLastError(Pmt_Sub_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type() => Error: %u, bind PaymentName : %s", dwErr, szErr);
			break;
		}

		// Execute the request
		if (!_DBExecuteStatement(Pmt_Sub_Type.hStat))
		{
			_DBGetLastError(Pmt_Sub_Type.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type() => _DBExecuteStatement(), Error: %u, %s", dwErr, szErr);
			break;
		}

		// Retrieve results
		Pmt_Sub_Type.dwConfIndex = 0;
		pvResult = _DBGetVariableItemValue(Pmt_Sub_Type.hId, Pmt_Sub_Type.dwConfIndex, pay_sub_id, NULL);
		if (pvResult != pay_sub_id)
			break;

		StrSize = MSG_PV_MAX_PAYMENT_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Pmt_Sub_Type.hName, Pmt_Sub_Type.dwConfIndex, pay_name, &StrSize);
		if (pvResult != pay_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type(), %s = %d", pay_name, *pay_sub_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Pmt_Sub_Type.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_First_Payment_Sub_Type() => disconnect from database!" );
		_DBCloseStatement(Pmt_Sub_Type.hStat);
		DB_CONNECTION_Close();
		Pmt_Sub_Type.hDbCnx = NULL;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Payment_Sub_Type(OUT DWORD *pay_sub_id,
														   OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	void	*pvResult	= NULL;
	CHAR	szErr[200]	= { 0 };
	BOOL	bRet		= FALSE;
	DWORD	StrSize		= 0;

	bRet = FALSE;
	do
	{
		Pmt_Sub_Type.dwConfIndex++;
		pvResult = _DBGetVariableItemValue(Pmt_Sub_Type.hId, Pmt_Sub_Type.dwConfIndex, pay_sub_id, NULL);
		if (pvResult != pay_sub_id)
			break;

		StrSize = MSG_PV_MAX_PAYMENT_LABEL;
		pvResult = _DBGetVariableItemValueTruncate(Pmt_Sub_Type.hName, Pmt_Sub_Type.dwConfIndex, pay_name, &StrSize);
		if (pvResult != pay_name)
			break;

		NTSVCInfo("CONF_PLAZA_Get_Next_Payment_Sub_Type(), %s = %d", pay_name, *pay_sub_id);

		bRet = TRUE;
	} 
	while (FALSE);

	if (!bRet && Pmt_Sub_Type.hDbCnx != NULL)
	{
		// NTSVCInfo( "CONF_PLAZA_Get_Next_Payment_Sub_Type() => disconnect from database!" );
		_DBCloseStatement(Pmt_Sub_Type.hStat);
		DB_CONNECTION_Close();
		Pmt_Sub_Type.hDbCnx = NULL;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
