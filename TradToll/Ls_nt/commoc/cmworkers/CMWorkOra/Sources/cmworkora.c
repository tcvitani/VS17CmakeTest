/******************* (v) 2002 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKORA                                                       */
/* FILE:     cmworkora.c                                                     */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             calls the ORACLE stored procedure that imports the file into  */
/*             the database. The ORACLE procedure call is the module         */
/*             parameter. The parameter of the oracle procedure can be the   */
/*             file name (with full path) that has to be imported.           */
/*             It uses the new worker procedure that enables additional      */
/*             options (delete bad file, retry bad file processing etc.      */
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
#include <csr_excpt.h>
#include <dbif.h>

// Module interface
#define LOC_DEF
#include <cmworkora.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define CMWORK_MAX_STR        512
#define CMWORK_MAX_PARAM_STR  50
#define CMWORK_MAX_FILE_NAME  MAX_PATH
#define CMWORK_STRNCPY(x,y,n)      (strncpy(x,y,n),(x)[n-1]=0,x)
#define CMWORK_STRNCAT(x,y,n)      (strncat(x,y,n),(x)[n-1]=0,x)

#define ERROR_BAD_FILE        1001
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_PARAMS
{
	char szCommand[CMWORK_MAX_STR];
	char szDBName[CMWORK_MAX_PARAM_STR];
	char szUserName[CMWORK_MAX_PARAM_STR];
	char szPassword[CMWORK_MAX_PARAM_STR];
	TRC_EMETTEUR hTrc;
}CMW_PARAMS;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE void WorkTrace( HANDLE hWork, char * szFormat, ... );
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/*****************************************************************************/
/*                     CODE OF THE EXPORTED FUNCTIONS                        */
/*****************************************************************************/

/**/
/*****************************************************************************/
/*SYNTAX: EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )  */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at service  */
/*            startup. The function initializes the module. It reads the     */
/*            module parameters from the registry and initializes the trace  */
/*            file.                                                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) PVOID param  - Pointer to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  HANDLE                If the initialization is successful the function   */
/*                        returns the module handle (pointer to module       */
/*                        parameters).                                       */
/*  NULL                  The function returns NULL if the initialization is */
/*                        not successful.                                    */
/*****************************************************************************/
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerOpen=_WorkerOpen@8" )
#endif
EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    CMW_PARAMS * pcParams = NULL;
	char szTrace[CMWORK_MAX_PARAM_STR];

    __try
    {
        pcParams = HeapAlloc( GetProcessHeap(), 0, sizeof(CMW_PARAMS) );
        if ( pcParams == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

		dwSize=CMWORK_MAX_STR;
        dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "Command", pcParams->szCommand, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "DBName", pcParams->szDBName, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "UserName", pcParams->szUserName, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "Password", pcParams->szPassword, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "TraceFile", szTrace, &dwSize );
        if ( dwErr != NO_ERROR )
            szTrace[0] = 0;

		if ( szTrace[0] != 0 )
		{
			dwErr = TRC_Initialise_Trace( 
                szName, 
                szTrace, 
                TRC_OPT_FICHIER | TRC_OPT_CONSOLE | TRC_OPT_NUMEROTATION,
                &pcParams->hTrc );
			if ( dwErr != NO_ERROR )
			{
				__leave;
			}
		}
		else
			pcParams->hTrc = NULL;
	}

    __finally
    {
        if ( ( dwErr != NO_ERROR ) && ( pcParams != NULL ) )
        {
			if ( pcParams->hTrc != NULL )
                TRC_Termine_Trace( pcParams->hTrc );

            HeapFree( GetProcessHeap(), 0, pcParams );
            pcParams = NULL;
        }

        SetLastError( dwErr );
    }
	return (VOID*)pcParams;
}

/**/
/*****************************************************************************/
/*SYNTAX: EXPORT void WINAPI WorkerClose( HANDLE hWrk )                      */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at the      */
/*            service exit.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk  - Pointer to the module handle.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerClose=_WorkerClose@4" )
#endif
EXPORT void WINAPI WorkerClose( HANDLE hWrk )
{
	CMW_PARAMS * pcParams = hWrk;

	if ( pcParams->hTrc != NULL )
		TRC_Termine_Trace( pcParams->hTrc );

    HeapFree( GetProcessHeap(), 0, hWrk );
    return;
}

/**/
/*****************************************************************************/
/*SYNTAX: EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk,                 */
/*                                              char * szFilePath )          */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service when it     */
/*            detects the file in the appropriate directory. The function    */
/*            calls the ORACLE procedure that imports the file into the      */
/*            database.                                                      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk       - Worker handle (pointer to module           */
/*                                parameters).                               */
/*      (IN ) char * szFilePath - File name (full path)                      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  The file is successfully converted.                */
/*  FALSE                 Error converting the file.                         */
/*****************************************************************************/
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerProcessFile=_WorkerProcessFile@8" )
#endif
EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath )
{
	char szErr[200] = "";
    DWORD dwErr = NO_ERROR;
    CMW_PARAMS * pcParams = hWrk;
	HANDLE * hFile;
	char szTerminator[MAX_PATH];
	DWORD dwRead;
	DB_CNX * hDbCnx = NULL;
	DB_STMT * hStmt = NULL;
	DB_VAR * hFileName;

    __try
    {
		// Check if the file copying is finished
		hFile = CreateFile( szFilePath, 
							GENERIC_READ, 
							0, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			dwErr = GetLastError();
			WorkTrace( hWrk, "WORKORA - Error openning file (%d)", dwErr );
			__leave;
		}

		SetFilePointer( hFile, -6, NULL, FILE_END );
		dwErr = GetLastError();
		if ( dwErr != 0 )
		{
			WorkTrace( hWrk, "WORKORA - Error setting the file pointer (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		memset( szTerminator, 0, MAX_PATH );
		if ( ! ReadFile( hFile, szTerminator, 4, &dwRead, NULL ) )
		{
			dwErr = GetLastError();
			WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		if ( dwRead != 4 )
		{
			dwErr = ERROR_BAD_FILE;
			WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		CloseHandle( hFile );
		if ( strncmp( szTerminator, "#ERF", 4 ) != 0 )
		{
			dwErr = ERROR_BAD_FILE;
			WorkTrace( hWrk, "WORKORA - File copying not finished (%d) %s", dwErr, szTerminator );
			__leave;
		}

		// File copying is finished 
		hDbCnx = DBConnect( pcParams->szDBName, pcParams->szUserName, pcParams->szPassword );
		if ( hDbCnx == NULL )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
			WorkTrace( hWrk, "WORKORA - Error connect database (%d) %s", dwErr, szErr);
			__leave;
		}

		hStmt = DBOpenStatement( hDbCnx, pcParams->szCommand );
        if ( hStmt == NULL )
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
			WorkTrace( hWrk, "WORKORA - Error open statement (%d) %s", dwErr, szErr);
			__leave;
        }

		hFileName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":FileName", 
                    DB_TYPE_STR, 
                    CMWORK_MAX_FILE_NAME, 
                    0 );
		if ( hFileName == NULL )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
			WorkTrace( hWrk, "WORKORA - Error bind variable (%d) %s", dwErr, szErr );
			__leave;
		}

		if ( DBSetVariableItemValue( hFileName, 0, szFilePath, (DWORD)strlen(szFilePath) + 1 ) != szFilePath )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
			WorkTrace( hWrk, "WORKORA - Error set variable (%d) %s", dwErr, szErr);
			__leave;
		}

		if ( ! DBExecuteStatement( hStmt ) )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
			WorkTrace( hWrk, "WORKORA - Error execute statement (%d) %s", dwErr, szErr);
			__leave;
		}
	}

	__finally
	{
		if ( ( hStmt != NULL ) && ( hDbCnx != NULL ) )
			DBCloseStatement( hStmt );

		if ( hDbCnx != NULL )
		{
			DBDisconnect( hDbCnx );
			hDbCnx = NULL;
		}

		SetLastError( dwErr );
	}
	return ( dwErr == NO_ERROR );
}

/*****************************************************************************/
/*                     END OF THE EXPORTED FUNCTIONS                         */
/*****************************************************************************/

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE void WorkTrace( HANDLE hWork, char * szFormat, ... )       */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the trace file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWork    - Pointer to module parameters                 */
/*      (IN ) char * szFormat - Pointer to the format string                 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PRIVATE void WorkTrace( HANDLE hWork, char * szFormat, ... )
{
    va_list pMark;
	CMW_PARAMS * pcParams = hWork;

    if ( pcParams->hTrc != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Trace_Texte_V( pcParams->hTrc, TRC_OPT_MASK, szFormat, pMark );
    }
}
/*-------------------------------- END OF FILE ------------------------------*/