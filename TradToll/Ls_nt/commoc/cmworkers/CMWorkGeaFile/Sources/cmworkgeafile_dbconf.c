/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_dbconf.c                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage conversion from  */
/*             the GEA beginning of job message to the CSSI start of shift   */
/*             message.                                                      */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>

// CSSI interface
#include <trc.h>
#include <reg.h>
#include <dbif.h>

// Module interface
#include <cmworkgeafile.h>
#include <cmworkgeafile_convert.h>
#define LOC_DEF
#include <cmworkgeafile_dbconf.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define MAX_CURRENCY_LABEL_LENGTH		40
#define MAX_CURRENCIES					30
// Selects the currency IDs and the currency labels from the database
#define CURR_STATEMENT \
	"DECLARE\n \
		TYPE tab_entier_type IS TABLE OF INTEGER \
			INDEX BY BINARY_INTEGER;\n \
		TYPE tab_string_type IS TABLE OF VARCHAR2(40) \
			INDEX BY BINARY_INTEGER;\n \
		PROCEDURE Get_Curr(curr_id OUT tab_entier_type, curr_label OUT tab_string_type) IS\n \
			i INTEGER:=1;\n \
		BEGIN\n \
			FOR line IN (SELECT ID_CURRENCY, SHORT_LABEL_1 FROM T_CURRENCY) LOOP\n \
				curr_id(i):=line.id_currency; \
				curr_label(i):=line.short_label_1; \
				i:=i+1;\n \
			END LOOP;\n \
		END;\n \
	BEGIN\n \
		Get_Curr(:IdCurr, :CurrLabel);\n \
	END;" \
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE DWORD CurrDBToReg( IN CMW_PARAMS * pcParams, IN DB_CNX * hDbCnx );
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD RefreshCurrTable( IN CMW_PARAMS * pcParams )                 */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/
PROTECTED DWORD RefreshCurrTable( IN CMW_PARAMS * pcParams )
{
	DWORD dwErr= NO_ERROR;
#ifdef CMWORK_DB_CONNECTION
	DB_CNX * hDbCnx;

	if( strlen( pcParams->szDbInst ) == 0 )
		return NO_ERROR;

	// Connect to database
	hDbCnx  = DBConnect( pcParams->szDbInst, pcParams->szDbUsr, pcParams->szDbPwd );
	if ( hDbCnx == NULL )
		return ERROR_NO_DB_CONNECTION;

	dwErr = CurrDBToReg( pcParams, hDbCnx );

	if ( hDbCnx != NULL )
	{
		DBDisconnect( hDbCnx );
		hDbCnx = NULL;
	}
#endif
	return dwErr;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD CurrDBToReg( IN CMW_PARAMS * pcParams,                       */
/*                           IN DB_CNX * hDbCnx )                            */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN DB_CNX * hDbCnx       - Database connection identifier            */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PRIVATE DWORD CurrDBToReg( IN CMW_PARAMS * pcParams, IN DB_CNX * hDbCnx )
{
#ifdef CMWORK_DB_CONNECTION
	DWORD dwErr, dwConfIndex, dwCurrId, dwStrSize;
	char szErr[200] = "";
	DB_STMT * hStat;
    DB_VAR * hCurrId;
    DB_VAR * hCurrLabel;
	void * pvResult;
	CHAR szCurrLabel[MAX_CURRENCY_LABEL_LENGTH] = {0};
	CHAR szKey[MAX_PATH];

	sprintf_s(szKey,
			sizeof(szKey),
			 "%s\\%s\\%s\\",
			 "SYSTEM\\CurrentControlSet\\Services\\CommocSvc\\Parameters\\Workers",
			 pcParams->szWrkName,
			 "CurrencyTable");

	hStat = DBOpenStatement( hDbCnx, CURR_STATEMENT );

	if ( hStat == NULL )
	{
		DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof( szErr ) );
		WorkError( pcParams , "(CurrDBToReg) Open statement error: %s", szErr );

		return dwErr;
	}

	hCurrId = DBBindPlaceHolderVariable( hStat, 
										 ":IdCurr", 
										 DB_TYPE_INT, 
										 sizeof(DWORD), 
										 MAX_CURRENCIES );

	if ( hCurrId== NULL )
	{
		DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
		WorkError( pcParams , "(CurrDBToReg) Bind variable error: %s", szErr );
		DBCloseStatement(hStat);

		return dwErr;
	}

	hCurrLabel = DBBindPlaceHolderVariable( hStat, 
											":CurrLabel", 
											DB_TYPE_STR, 
											MAX_CURRENCY_LABEL_LENGTH, 
											MAX_CURRENCIES );

	if ( hCurrLabel == NULL )
    {
		DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
		WorkError( pcParams , "(CurrDBToReg) Bind variable error: %s", szErr );
		DBCloseStatement( hStat );

		return dwErr;
	}

	if ( !DBExecuteStatement( hStat ) )
	{
		DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
		WorkError( pcParams , "(CurrDBToReg) Execute statement error: %s", szErr );
		DBCloseStatement( hStat );

		return dwErr;
	}

	for ( dwConfIndex = 0; dwConfIndex < MAX_CURRENCIES; dwConfIndex ++ )
	{
		pvResult = DBGetVariableItemValue( hCurrId, dwConfIndex, &dwCurrId, NULL );
		if ( pvResult != &dwCurrId )
			break;

		dwStrSize = MAX_CURRENCY_LABEL_LENGTH;
		pvResult = DBGetVariableItemValueTruncate( hCurrLabel, dwConfIndex, szCurrLabel, &dwStrSize );
        
        if ( pvResult != szCurrLabel )
            break;

		// Save it in the registry
		sscanf_s(szCurrLabel, "%s", szCurrLabel, _countof(szCurrLabel));
		REG_Ecrire_Entier( HKEY_LOCAL_MACHINE, szKey, szCurrLabel, dwCurrId ); 
	}

	DBCloseStatement( hStat );
#endif
	return NO_ERROR;
}
/*-------------------------------- END OF FILE ------------------------------*/