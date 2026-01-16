/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     srvpv_dbif.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef SRVPV_DBIF_H
#define SRVPV_DBIF_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/

#ifdef SRVPV_DBIF_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif

/*-------------------------------- DEFINES:   -------------------------------*/

#define DB_TYPE_INT 0
#define DB_TYPE_STR 1 

#define DB_TYPE_TEXT(x) \
    ( (x) == DB_TYPE_INT ? "DB_TYPE_INT" : \
    ( (x) == DB_TYPE_STR ? "DB_TYPE_STR"  : \
                           "DB_TYPE_UNKNOWN" ) )

#define DB_ERRCLASS_NONE 0
#define DB_ERRCLASS_OS   1
#define DB_ERRCLASS_DB   2

#define DB_ERRCLASS_TEXT(x) \
    ( (x) == DB_ERRCLASS_NONE ? "DB_ERRCLASS_NONE" : \
    ( (x) == DB_ERRCLASS_OS   ? "DB_ERRCLASS_OS"   : \
    ( (x) == DB_ERRCLASS_DB   ? "DB_ERRCLASS_DB"   : \
                                "DB_ERRCLASS_UNKNOWN" ) ) )

#define DB_VALUE_NULL          ((void*)0)
#define DB_VALUE_TOO_SMALL     ((void*)1)
#define DB_VALUE_TOO_BIG       ((void*)2)
#define DB_VALUE_OUT_OF_RANGE  ((void*)3)
#define DB_VALUE_ERROR         ((void*)0xFFFFFFFF)


//#define DB_VAR_NAME_SIZE 32

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct DB_VAR DB_VAR;

typedef struct DB_STMT DB_STMT;

typedef struct DB_CNX DB_CNX;

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT char * WINAPI _DBGetLastError(DB_CNX *psDb, DWORD *pdwType, DWORD *pdwCode, char *pcText, DWORD dwSize);

EXPORT DB_CNX * WINAPI _DBConnectCallback(void *pvCallbackContext);

EXPORT DB_CNX * WINAPI _DBConnect(char *pcInst, char *pcUsr, char *pcPwd);

EXPORT void WINAPI _DBDisconnect(DB_CNX *psDb);

EXPORT void WINAPI _DBReinitVariable(DB_VAR *psVar);

EXPORT DWORD WINAPI _DBGetCurrentItemCount(DB_VAR * psVar);

EXPORT void * WINAPI _DBGetVariableItemValue(DB_VAR *psVar, DWORD dwIndex, void *pvBuffer, DWORD *pdwLen);

EXPORT void * WINAPI _DBGetVariableItemValueTruncate(DB_VAR *psVar, DWORD dwIndex, void * pvBuffer, DWORD *pdwLen);

EXPORT void * WINAPI _DBSetVariableItemValue(DB_VAR *psVar, DWORD dwIndex, void *pvBuffer, DWORD dwLen);

EXPORT DB_VAR * WINAPI _DBBindPlaceHolderVariable(DB_STMT *psStmt, char *pcVarName, DWORD dwDataType, DWORD dwItemSize, DWORD dwItemCount);

EXPORT DB_VAR * WINAPI _DBBindRowItemVariable(DB_STMT *psStmt, DWORD dwPosition, DWORD dwDataType, DWORD dwItemSize);

EXPORT void WINAPI _DBClearVariables(DB_STMT *psStmt);

EXPORT DB_STMT * WINAPI _DBOpenStatement(DB_CNX *psDb, char *pcStatement);

EXPORT void WINAPI _DBCloseStatement(DB_STMT *psStmt);

EXPORT BOOL WINAPI _DBExecuteStatement(DB_STMT *psStmt);

EXPORT BOOL WINAPI _DBGetNextRow(DB_STMT *psStmt, BOOL *pbEnd);

#endif

/*-------------------------------- END OF FILE ------------------------------*/

