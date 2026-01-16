/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     DB_connection.c												 */
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
#include <ntsvc.h>

#include <Db_Connection.h>
#include <srvpv_dbif.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define DB_REG_VAL_DBUSR	"DbUser"
#define DB_REG_VAL_DBPWD	"DbPassword"
#define DB_REG_VAL_DBINST	"DbInstance"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

PRIVATE struct
{
	char	szDbUsr[MAX_PATH];
	char	szDbPwd[MAX_PATH];
	char	szDbInst[MAX_PATH];
	DB_CNX	*hDbCnx;
	int		counter;
}
DB = { 0 };

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED DB_CNX * WINAPI DB_CONNECTION_Open(VOID)
{
	DWORD				dwErr = 0;
	NTSVC_PARAMETER_DEF	*psParams = NULL;

	if (DB.hDbCnx == NULL)
	{
		// get registry parameter
		psParams = NTSVCOpenParameters(
			DB_REG_VAL_DBUSR, REG_SZ, MAX_PATH,		"PVUSER",						&DB.szDbUsr,
			DB_REG_VAL_DBPWD, REG_SZ, MAX_PATH,		"PVPWD",						&DB.szDbPwd,
			DB_REG_VAL_DBINST, REG_SZ, MAX_PATH,	"ENTER DATA BASE NAME HERE !",	&DB.szDbInst,
			NULL);

		if (psParams == NULL)
			return NULL;

		if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
			return NULL;

		NTSVCCloseParameters(psParams);

		// connect to database
		DB.hDbCnx = _DBConnect(DB.szDbInst, DB.szDbUsr, DB.szDbPwd);

		if (DB.hDbCnx == NULL)
			NTSVC_ERR2("DB_CONNECTION_Open() => Error connection to database: %s, user: %s", DB.szDbInst, DB.szDbUsr);
		else
			NTSVCInfo("DB_CONNECTION_Open() => connected to database: %s, user: %s", DB.szDbInst, DB.szDbUsr);
	}

	// if connected, increase counter for one 
	if (DB.hDbCnx != NULL)
		DB.counter = DB.counter++;

	return DB.hDbCnx;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED DB_CNX * WINAPI DB_CONNECTION_Close(VOID)
{
	if (DB.counter <= 1 && DB.hDbCnx != NULL)
	{
		NTSVCInfo("DB_CONNECTION_Close() => disconnect from database");
		_DBDisconnect(DB.hDbCnx);
		DB.hDbCnx = NULL;
	}

	if (DB.counter > 0)
		DB.counter = DB.counter--;

	return DB.hDbCnx;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PUBLIC void WINAPI DB_CONNECTION_NTSVC_Inherit_Handle(void *pvContext)
{
	NTSVCSetContext(pvContext);
}

/*-------------------------------- END OF FILE ------------------------------*/