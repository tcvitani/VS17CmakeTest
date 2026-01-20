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
#include <cmwork.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define CMWORK_MAX_STR        512
#define CMWORK_MAX_PARAM_STR  MAX_PATH
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
	DWORD dwEnsureOrder;
	TRC_EMETTEUR hTrc;
	char szTargetFileTerminator[CMWORK_MAX_PARAM_STR];
	DWORD dwFileTerminatorBckPosition;
	char szErrorCodeList[CMWORK_MAX_PARAM_STR];
	DWORD dwCopyToProcessedFolder;
	char szProcessedFolder[CMWORK_MAX_PARAM_STR];

}CMW_PARAMS;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE void WorkTrace( HANDLE hWork, char * szFormat, ... );
PRIVATE BOOL foundInErrorCodeList(DWORD dwErr, char * szErrCodeList);

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

		// This parameter forces the file processing by the order defined in the SortBy parameter
		if ( REG_Lire_Entier( hKeyConfig, NULL, "EnsureOrder", &pcParams->dwEnsureOrder ) != NO_ERROR )
			pcParams->dwEnsureOrder = 0;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "Password", pcParams->szPassword, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;


		if ( REG_Lire_Entier( hKeyConfig, NULL, "FileTerminatorBckPosition", &pcParams->dwFileTerminatorBckPosition ) != NO_ERROR )
			pcParams->dwFileTerminatorBckPosition = 2;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "TargetFileTerminator", pcParams->szTargetFileTerminator, &dwSize );
        if ( dwErr != NO_ERROR )
			strcpy_s(pcParams->szTargetFileTerminator, sizeof(pcParams->szTargetFileTerminator), "#ERF");


		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "MoveToErrIfCode", pcParams->szErrorCodeList, &dwSize );
		if (dwErr != NO_ERROR)
			strcpy_s(pcParams->szErrorCodeList, sizeof(pcParams->szErrorCodeList), "");

		//on success copy to processed folder	
		if ( REG_Lire_Entier( hKeyConfig, NULL, "CopyToProcessedFolder", &pcParams->dwCopyToProcessedFolder ) != NO_ERROR )
			pcParams->dwCopyToProcessedFolder = 0;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "ProcessedFolder", pcParams->szProcessedFolder, &dwSize );
        if ( dwErr != NO_ERROR )
        {
			strcpy_s(pcParams->szProcessedFolder, sizeof(pcParams->szProcessedFolder), "c:\\");
			pcParams->dwCopyToProcessedFolder = 0; //if not both parameters defined do not copy to processed folder
		}



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
/*SYNTAX: EXPORT DWORD WINAPI WorkerProcessFileEx( IN HANDLE hWrk,           */
/*                                                 IN char * szFilePath,     */
/*                                                 IN void * pvReserved )    */
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
/*      IN HANDLE hWrk       - Worker handle (pointer to module              */
/*                             parameters).                                  */
/*      IN char * szFilePath - File name (full path)                         */
/*      IN void * pvReserved - Reserved                                      */
/*===========================================================================*/
/*  Return                   Description                                     */
/*---------------------------------------------------------------------------*/
/* Result of the processing:                                                 */
/*  CM_WORKER_ACK_NONE       (0x00000000) : processing canceled/file ignored */
/*  CM_WORKER_ACK_OK         (0x00000001) : processing done successfully     */
/*  CM_WORKER_ACK_RETRY_ERR  (0x00000002) : error during processing,         */
/*                                          retry later                      */
/*  CM_WORKER_ACK_RETRY_BUSY (0x00000003) : processing busy, retry later     */
/*  CM_WORKER_ACK_ABANDON    (0x00000004) : error during processing, abandon */
/*                                          file                             */
/* Combined with the service action after processing:                        */
/*  CM_WORKER_ACT_NONE       (0x00000000) : nothing to do                    */
/*  CM_WORKER_ACT_MOVE_ACK   (0x00000100) : move file to acknowledgement     */
/*                                          directory                        */
/*  CM_WORKER_ACT_COPY_ERR   (0x00000200) : copy file to the error           */
/*                                          directory and move it to         */
/*                                          the acknowledgement directory    */
/*  CM_WORKER_ACT_MOVE_ERR   (0x00000300) : move the file to the             */
/*                                          error directory                  */
/*  CM_WORKER_ACT_DELETE     (0x00000400) : simply delete the file           */
/*****************************************************************************/
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerProcessFileEx=_WorkerProcessFileEx@12" )
#endif
EXPORT DWORD WINAPI WorkerProcessFileEx( IN HANDLE hWrk,
								  IN char * szFilePath,
								  IN void * pvReserved )
{
	char szErr[200] = "";
	char szUnexpectedErr[200] = "";
    DWORD dwErr = NO_ERROR;
    CMW_PARAMS * pcParams = hWrk;
	HANDLE * hFile;
	char szTerminator[MAX_PATH];
	char szFile[MAX_PATH];
	char szTargetPath[MAX_PATH];
	char *pCh;
	DWORD dwRead;
	DB_CNX * hDbCnx = NULL;
	DB_STMT * hStmt = NULL;
	DB_VAR * hFileName;
	int iRewFileOffset; 
	BOOL bDBError;
	DWORD dwRet = 0;

    __try
    {
		bDBError = TRUE;
		WorkTrace( hWrk, "Processing file %s", szFilePath );

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "Checking file %s!", szFilePath); //set the text so we can trace what has happened

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

		//add -1 to cover the 0A 0D ftp text mode transfer problem
		iRewFileOffset = -(int)pcParams->dwFileTerminatorBckPosition -(int)strlen(pcParams->szTargetFileTerminator) - 1;
		//default iRewFileOffset = -6

		SetFilePointer( hFile, iRewFileOffset, NULL, FILE_END );
		dwErr = GetLastError();
		if ( dwErr != 0 )
		{
			WorkTrace( hWrk, "WORKORA - Error setting the file pointer (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		memset( szTerminator, 0, MAX_PATH );
		if ( ! ReadFile( hFile, szTerminator, (DWORD)strlen(pcParams->szTargetFileTerminator) + 2, &dwRead, NULL ) )
		{
			dwErr = GetLastError();
			WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		if ( dwRead < strlen(pcParams->szTargetFileTerminator) )
		{
			dwErr = ERROR_BAD_FILE;
			WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
			CloseHandle( hFile );
			__leave;
		}

		CloseHandle( hFile );
		if ( strstr( szTerminator, pcParams->szTargetFileTerminator) == NULL )
		{
			dwErr = ERROR_BAD_FILE;
			WorkTrace( hWrk, "WORKORA - File copying not finished (%d) %s or end of file does not match", dwErr, szTerminator );
			__leave;
		}

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "Connecting to database %s!", pcParams->szDBName); //set the text so we can trace what has happened
		
		// File copying is finished 
		hDbCnx = DBConnect( pcParams->szDBName, pcParams->szUserName, pcParams->szPassword );
		if ( hDbCnx == NULL )
		{
			dwErr = ERROR_INVALID_HANDLE;
			sprintf_s(szErr, sizeof(szErr), "Unable to connect to database %s!", pcParams->szDBName);
			WorkTrace( hWrk, "WORKORA - Error connect database (%d) %s", dwErr, szErr);
			__leave;
		}

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "DBOpenStatement: %s!", pcParams->szCommand); //set the text so we can trace what has happened

		hStmt = DBOpenStatement( hDbCnx, pcParams->szCommand );
        if ( hStmt == NULL )
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
			WorkTrace( hWrk, "WORKORA - Error open statement (%d) %s", dwErr, szErr);
			__leave;
        }

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "DBBindPlaceHolderVariable:%s!", pcParams->szCommand); //set the text so we can trace what has happened
		
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

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "DBSetVariableItemValue:%s!", szFilePath); //set the text so we can trace what has happened
		
		if ( DBSetVariableItemValue( hFileName, 0, szFilePath, (DWORD)strlen(szFilePath) + 1 ) != szFilePath )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
			WorkTrace( hWrk, "WORKORA - Error set variable (%d) %s", dwErr, szErr);
			__leave;
		}

		sprintf_s(szUnexpectedErr, sizeof(szUnexpectedErr), "DBExecuteStatement:%s!", pcParams->szCommand); //set the text so we can trace what has happened
		
		if ( ! DBExecuteStatement( hStmt ) )
		{
			DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr ) );
			WorkTrace( hWrk, "WORKORA - Error execute statement (%d) %s", dwErr, szErr);
			__leave;
		}

		bDBError = FALSE;

	}
	__finally
	{
		if(bDBError == TRUE)
		{
			WorkTrace( hWrk, "Unexpected error raised! Error while: %s",szUnexpectedErr);
		}
		
		if ( ( hStmt != NULL ) && ( hDbCnx != NULL ) )
			DBCloseStatement( hStmt );

		if ( hDbCnx != NULL )
		{
			DBDisconnect( hDbCnx );
			hDbCnx = NULL;
		}

		SetLastError( dwErr );
		
		if( dwErr == NO_ERROR && bDBError == FALSE)
		{
			if(pcParams->dwCopyToProcessedFolder)
			{	
				pCh = strrchr(szFilePath,'\\');
				if(pCh!=NULL)
				{
					strcpy_s(szFile, sizeof(szFile), pCh);
					strcpy_s(szTargetPath, sizeof(szTargetPath), pcParams->szProcessedFolder);
					strcat_s(szTargetPath, sizeof(szTargetPath), szFile);

					if(CopyFile(szFilePath,szTargetPath, FALSE) == 0)
					{
						WorkTrace( hWrk, "WORKORA - Error CopyFile source: %s target:%s ", szFilePath, szTargetPath);
					}
				}				
				else
				{
					WorkTrace( hWrk, "WORKORA - Error extracting file name from %s ", szFilePath);
					dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
				}

			}
			
			WorkTrace( hWrk, "File %s succesfully processed!", szFilePath);
			// Backup file
			dwRet = CM_WORKER_ACK_OK | CM_WORKER_ACT_MOVE_ACK;
		}
		else
		{
			// Move file to error directory
			if( pcParams->dwEnsureOrder == 0 )
			{
				
				if(dwErr != NO_ERROR)
					dwRet = CM_WORKER_ACK_ABANDON | CM_WORKER_ACT_MOVE_ERR;
				else
					dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
			}
			else
			{
				// Move to error folder if the error number found in the list of errors (parameter szErrorCodeList)
				if(foundInErrorCodeList(dwErr, pcParams->szErrorCodeList) && dwErr != NO_ERROR)
					dwRet = CM_WORKER_ACK_ABANDON | CM_WORKER_ACT_COPY_ERR;
				else 
					dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
			}
		}
	}

	return dwRet;
}



BOOL foundInErrorCodeList(DWORD dwErr, char * szErrCodeList)
{
	char szTargetErrorCode[CMWORK_MAX_PARAM_STR];
	char *token;
	char *next_token1 = NULL;

	sprintf_s(szTargetErrorCode, sizeof(szTargetErrorCode), "%d", dwErr);

	token = strtok_s(szErrCodeList, ",;", &next_token1);
	
	while( token != NULL )
	{
		if(strcmp(szTargetErrorCode,token) == 0)
			return TRUE;
	  /* Get next token: */
		token = strtok_s(NULL, ",;", &next_token1);
	}

	return FALSE;

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