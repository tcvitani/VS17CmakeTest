/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKCHKEXP                                                    */
/* FILE:     cmworkchkexp.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             checks if the file is completely exported (the file has the   */
/*             #ERF terminator).                                             */
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
#include <imagehlp.h>
#include <commoc.h>

// Module interface
#define LOC_DEF
#include <cmworkchkexpex.h>
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
	TRC_EMETTEUR hTrc;
	char szTargetFileTerminator[CMWORK_MAX_PARAM_STR];
	DWORD dwFileTerminatorBckPosition;
	DWORD dwCopyToProcessedFolder;
	char szProcessedFolder[CMWORK_MAX_PARAM_STR];
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

		if ( REG_Lire_Entier( hKeyConfig, NULL, "FileTerminatorBckPosition", &pcParams->dwFileTerminatorBckPosition ) != NO_ERROR )
			pcParams->dwFileTerminatorBckPosition = 2;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "TargetFileTerminator", pcParams->szTargetFileTerminator, &dwSize );
        if ( dwErr != NO_ERROR )
			strcpy_s(pcParams->szTargetFileTerminator, sizeof(pcParams->szTargetFileTerminator), "#ERF");

		//on success copy to processed folder	
		if ( REG_Lire_Entier( hKeyConfig, NULL, "CopyToOutputFolder", &pcParams->dwCopyToProcessedFolder ) != NO_ERROR )
			pcParams->dwCopyToProcessedFolder = 0;

		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "OutputFolder", pcParams->szProcessedFolder, &dwSize );
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
/*SYNTAX: EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk,                 */
/*                                              char * szFilePath )          */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service when it     */
/*            detects the file in the appropriate directory.                 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk       - Worker handle (pointer to module           */
/*                                parameters).                               */
/*      (IN ) char * szFilePath - File name (full path)                      */
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
    DWORD dwErr = ERROR_BAD_FILE;
    CMW_PARAMS * pcParams = hWrk;
	HANDLE * hFile;
	char szTerminator[MAX_PATH];
	char szFile[MAX_PATH];
	char szTargetPath[MAX_PATH];
	char szTargetTempFilePath[MAX_PATH];
	char szTempFileName[MAX_PATH];
	char *pCh;
	DWORD dwRead;
	int iRewFileOffset; 
	DWORD dwRet = 0;

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


	}

	__finally
	{
		SetLastError( dwErr );
		
		if( dwErr == NO_ERROR )
		{
			if(pcParams->dwCopyToProcessedFolder)
			{	
				pCh = strrchr(szFilePath,'\\');
				if(pCh!=NULL)
				{
					strcpy_s(szFile, sizeof(szFile), pCh + 1);
					strcpy_s(szTargetPath, sizeof(szTargetPath), pcParams->szProcessedFolder);
					MakeSureDirectoryPathExists( szTargetPath );
					
					pCh = strrchr(szTargetPath,'\\');
					if(pCh != szTargetPath + strlen(szTargetPath))
					{
						strcat_s(szTargetPath, sizeof(szTargetPath), "\\");
					}

					//create temp file full path
					strcpy_s(szTempFileName, sizeof(szTempFileName), szFile);
					strcat_s(szTempFileName, sizeof(szTempFileName), ".tmp");
					strcpy_s(szTargetTempFilePath, sizeof(szTargetTempFilePath), szTargetPath);

					//create final file full path
					strcat_s(szTargetTempFilePath, sizeof(szTargetTempFilePath), szTempFileName);
					strcat_s(szTargetPath, sizeof(szTargetPath), szFile);

					if(CopyFile(szFilePath,szTargetTempFilePath, FALSE) == 0)
					{
						
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKORA - Error CopyFile source: %s target:%s ERR %d", szFilePath, szTargetTempFilePath,dwErr);
						dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
					}


					if(MoveFileEx(szTargetTempFilePath,szTargetPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0)
					{
						
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKORA - Error MoveFileEx source: %s target:%s ERR %d", szTargetTempFilePath, szTargetPath,dwErr);
						dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
					}
				}
				else
				{
					WorkTrace( hWrk, "WORKORA - Error extracting file name from %s ", szFilePath);
					dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
				}


			}

			// Copy file to processed folder and ACK so commoc will delete it
			dwRet = CM_WORKER_ACK_OK | CM_WORKER_ACT_MOVE_ACK;
		}
		else
		{
			dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
		}
	}

	return dwRet;
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