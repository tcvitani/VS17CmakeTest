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

// Module interface
#define LOC_DEF
#include <cmworkchkexp.h>
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
	DWORD dwFooterLength;
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

		if ( REG_Lire_Entier( hKeyConfig, NULL, "FooterLength", &pcParams->dwFooterLength ) != NO_ERROR )
			pcParams->dwFooterLength = 0;







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

		if ( pcParams->dwFooterLength == 0 )
		{
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

			if ( strncmp( szTerminator, "#ERF", 4 ) != 0 )
			{
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKORA - File copying not finished (%d) %s", dwErr, szTerminator );
				CloseHandle( hFile );
				__leave;
			}

		}
		else
		{
			// Check the beginning of the footer line (0x0D 0x0A)
			SetFilePointer( hFile, -4-pcParams->dwFooterLength, NULL, FILE_END );
			dwErr = GetLastError();
			if ( dwErr != 0 )
			{
				WorkTrace( hWrk, "WORKORA - Error setting the file pointer (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			memset( szTerminator, 0, MAX_PATH );
			if ( ! ReadFile( hFile, szTerminator, 2, &dwRead, NULL ) )
			{
				dwErr = GetLastError();
				WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			if ( dwRead != 2 )
			{
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			if ( strncmp( szTerminator, "\r\n", 2 ) != 0 )
			{
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKORA - File copying not finished (%d) %s", dwErr, szTerminator );
				CloseHandle( hFile );
				__leave;
			}

			// Check the end of the footer line (0x0D 0x0A)
			SetFilePointer( hFile, -2, NULL, FILE_END );
			dwErr = GetLastError();
			if ( dwErr != 0 )
			{
				WorkTrace( hWrk, "WORKORA - Error setting the file pointer (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			memset( szTerminator, 0, MAX_PATH );
			if ( ! ReadFile( hFile, szTerminator, 2, &dwRead, NULL ) )
			{
				dwErr = GetLastError();
				WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			if ( dwRead != 2 )
			{
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKORA - Error reading from the file (%d)", dwErr );
				CloseHandle( hFile );
				__leave;
			}

			if ( strncmp( szTerminator, "\r\n", 2 ) != 0 )
			{
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKORA - File copying not finished (%d) %s", dwErr, szTerminator );
				CloseHandle( hFile );
				__leave;
			}
		}

		CloseHandle( hFile );
		// File copying is finished
	}

	__finally
	{
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