/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     DBIF.H                                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DBIF_H
#define DBIF_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef DBIF_EXPORTS
	#include <public.h>
#else
	#include <export.h>
#endif

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#ifndef DBIF_MAP

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

#define DB_VAR_NAME_SIZE 255    // Variable name can be file name with full path (used by ComMocSvc)


/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct DB_VAR DB_VAR;

typedef struct DB_STMT DB_STMT;

typedef struct DB_CNX DB_CNX;

#define DBDECLARE(type,name)    EXPORT type name

#else

#define DB_VAR void
#define DB_STMT void
#define DB_CNX void

#define DBDECLARE(type,name)    typedef type tp##name

#endif


/*-------------------------------- FUNCTIONS: -------------------------------*/

DBDECLARE (char * WINAPI, DBGetLastError) (DB_CNX *psDb, 
										   DWORD *pdwType, 
										   DWORD *pdwCode, 
										   char *pcText, 
										   DWORD dwSize);

DBDECLARE (DB_CNX * WINAPI, DBConnectCallback) (void * pvCallbackContext);

DBDECLARE (DB_CNX * WINAPI, DBConnect) (char *pcInst, 
										char *pcUsr, 
										char *pcPwd);

DBDECLARE (void WINAPI, DBDisconnect) (DB_CNX *psDb);

DBDECLARE (void WINAPI, DBReinitVariable) (DB_VAR *psVar);

DBDECLARE (DWORD WINAPI, DBGetCurrentItemCount) (DB_VAR *psVar);

DBDECLARE (void * WINAPI, DBGetVariableItemValue) (DB_VAR *psVar, 
												   DWORD dwIndex, 
											 	   void *pvBuffer, 
												   DWORD *pdwLen);

DBDECLARE (void * WINAPI, DBGetVariableItemValueTruncate) (DB_VAR *psVar, 
														   DWORD dwIndex, 
														   void *pvBuffer, 
														   DWORD *pdwLen);

DBDECLARE (void * WINAPI, DBSetVariableItemValue) (DB_VAR *psVar, 
												   DWORD dwIndex, 
												   void *pvBuffer, 
												   DWORD dwLen);

DBDECLARE (DB_VAR * WINAPI, DBBindPlaceHolderVariable) (DB_STMT * psStmt,
														char *pcVarName,
														DWORD dwDataType, 
														DWORD dwItemSize, 
														DWORD dwItemCount);

DBDECLARE (DB_VAR * WINAPI, DBBindRowItemVariable) (DB_STMT *psStmt,
													DWORD dwPosition,
													DWORD dwDataType, 
													DWORD dwItemSize);

DBDECLARE (void WINAPI, DBClearVariables) (DB_STMT *psStmt);

DBDECLARE (DB_STMT * WINAPI, DBOpenStatement) (DB_CNX *psDb, char *pcStatement);

DBDECLARE (void WINAPI, DBCloseStatement) (DB_STMT *psStmt);

DBDECLARE (BOOL WINAPI, DBExecuteStatement) (DB_STMT *psStmt);

DBDECLARE (BOOL WINAPI, DBGetNextRow) (DB_STMT *psStmt, BOOL *pbEnd);

#endif

/*-------------------------------- END OF FILE ------------------------------*/

