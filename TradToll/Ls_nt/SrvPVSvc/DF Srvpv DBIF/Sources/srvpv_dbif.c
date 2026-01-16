/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     srvpv_dbif.c													 */
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
#include <srvpv_dbif.h>
#include <dbif.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

PRIVATE struct DB_CNX
{
	int dummy;
}
DB_CNX_fake;

PRIVATE struct DB_VAR
{
	int dummy;
}
DB_VAR_fake;

PRIVATE struct DB_STMT
{
	int dummy;
}
DB_STMT_fake;

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT char * WINAPI _DBGetLastError(DB_CNX *psDb, DWORD *pdwType, DWORD *pdwCode, char *pcText, DWORD dwSize)
{
	return pcText;
}

EXPORT DB_CNX * WINAPI _DBConnectCallback(void *pvCallbackContext)
{
	return &DB_CNX_fake;
}

EXPORT DB_CNX * WINAPI _DBConnect(char *pcInst, char *pcUsr, char *pcPwd)
{
	return &DB_CNX_fake;
}

EXPORT void WINAPI _DBDisconnect(DB_CNX *psDb)
{

}

EXPORT void WINAPI _DBReinitVariable(DB_VAR *psVar)
{

}

EXPORT DWORD WINAPI _DBGetCurrentItemCount(DB_VAR *psVar)
{
	return 0;
}

EXPORT void * WINAPI _DBGetVariableItemValue(DB_VAR *psVar, DWORD dwIndex, void *pvBuffer, DWORD *pdwLen)
{
	return DB_VALUE_ERROR;
}

EXPORT void * WINAPI _DBGetVariableItemValueTruncate(DB_VAR *psVar, DWORD dwIndex, void *pvBuffer, DWORD *pdwLen)
{
	return DB_VALUE_ERROR;
}

EXPORT void * WINAPI _DBSetVariableItemValue(DB_VAR *psVar, DWORD dwIndex, void *pvBuffer, DWORD dwLen)
{
	return pvBuffer;
}

EXPORT DB_VAR * WINAPI _DBBindPlaceHolderVariable(DB_STMT *psStmt, char *pcVarName, DWORD dwDataType, DWORD dwItemSize, DWORD dwItemCount)
{
	return &DB_VAR_fake;
}

EXPORT DB_VAR * WINAPI _DBBindRowItemVariable(DB_STMT *psStmt, DWORD dwPosition, DWORD dwDataType, DWORD dwItemSize)
{
	return &DB_VAR_fake;
}

EXPORT void WINAPI _DBClearVariables(DB_STMT *psStmt)
{

}

EXPORT DB_STMT * WINAPI _DBOpenStatement(DB_CNX *psDb, char *pcStatement)
{
	return &DB_STMT_fake;
}

EXPORT void WINAPI _DBCloseStatement(DB_STMT *psStmt)
{

}

EXPORT BOOL WINAPI _DBExecuteStatement(DB_STMT *psStmt)
{
	return TRUE;
}

EXPORT BOOL WINAPI _DBGetNextRow(DB_STMT *psStmt, BOOL *pbEnd)
{
	return TRUE;
}

/*-------------------------------- END OF FILE ------------------------------*/

