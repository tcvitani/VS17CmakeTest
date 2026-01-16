/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile.c                                                 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             Extension module of the ComMOCSvc service. This module        */
/*             converts the GEA PAQ file to CSSI files.                      */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

// VC++ interface
#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>
// CSSI interface
#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>
#include <csr_msg.h>
#include <commoc.h>

// Module interface
#define LOC_DEF
#include <cmworkgeafile.h>
#undef LOC_DEF
#include <cmworkgeafile_convert.h>
#include <cmworkgeafile_file.h>
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/*****************************************************************************/
/*                     CODE OF THE EXPORTED FUNCTIONS                        */
/*****************************************************************************/

/**/
/*****************************************************************************/
/*SYNTAX: EXPORT HANDLE WINAPI WorkerOpen( IN char * szName,                 */
/*                                         IN HKEY hKeyConfig )              */
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
/*      IN char * szName   - Module name defined by the registry sub-key     */
/*                           name                                            */
/*      IN HKEY hKeyConfig - Handle to the registry key that contains the    */
/*                           module parameters                               */
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
EXPORT HANDLE WINAPI WorkerOpen( IN char * szName, IN HKEY hKeyConfig )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwSize;
	CMW_PARAMS * pcParams = NULL;
	char szTrace[CMWORK_MAX_PARAM_STR];

    __try
    {
        pcParams = HeapAlloc( GetProcessHeap(), 0, sizeof( CMW_PARAMS ) );
        if ( pcParams == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

		strcpy_s(pcParams->szWrkName, sizeof(pcParams->szWrkName), szName);
		// Out directory (contains the CSSI messages)
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "OutDirectory", pcParams->szOutDir, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;
		// Context directory
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "LocalContext", pcParams->szCtxDir, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;
		// Temp directory
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "LocalTemp", pcParams->szTempDir, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		// This parameter forces the file processing by the order defined in the SortBy parameter
		if ( REG_Lire_Entier( hKeyConfig, NULL, "EnsureOrder", &pcParams->dwEnsureOrder ) != NO_ERROR )
			pcParams->dwEnsureOrder = 0;
		// If this parameter is set to one all the converted CSSI files will be named as backup files
		if ( REG_Lire_Entier( hKeyConfig, NULL, "StoreInBakFiles", &pcParams->dwStoreInBakFiles ) != NO_ERROR )
			pcParams->dwStoreInBakFiles = 0;
		// Databse name
		dwSize=CMWORK_MAX_PARAM_STR;
		if ( REG_Lire_Chaine( hKeyConfig, NULL, "DBName", pcParams->szDbInst, &dwSize ) != NO_ERROR )
			sprintf_s(pcParams->szDbInst, sizeof(pcParams->szDbInst), "");
		// User name
		dwSize=CMWORK_MAX_PARAM_STR;
		if ( REG_Lire_Chaine( hKeyConfig, NULL, "UserName", pcParams->szDbUsr, &dwSize ) != NO_ERROR )
			sprintf_s(pcParams->szDbUsr, sizeof(pcParams->szDbUsr), "");
		// Password
		dwSize=CMWORK_MAX_PARAM_STR;
		if ( REG_Lire_Chaine( hKeyConfig, NULL, "Password", pcParams->szDbPwd, &dwSize ) != NO_ERROR )
			sprintf_s(pcParams->szDbPwd, sizeof(pcParams->szDbPwd), "");
		// List of lanes (only lanes in the list will be processed)
		dwSize=CMWORK_MAX_PARAM_STR;
		if ( REG_Lire_Chaine( hKeyConfig, NULL, "LaneList", pcParams->szLaneList, &dwSize ) != NO_ERROR )
			sprintf_s(pcParams->szLaneList, sizeof(pcParams->szLaneList), "");
		// Network multiplier (GEA network number can be the part of the CSSI plaza ID)
		if ( REG_Lire_Entier( hKeyConfig, NULL, "NetworkMultiplier", &pcParams->dwNetMult ) != NO_ERROR )
			pcParams->dwNetMult = 0;

		// Name of the trace file
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

		// Name of the error file
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "ErrorFile", szTrace, &dwSize );
        if ( dwErr != NO_ERROR )
            szTrace[0] = 0;

		if ( szTrace[0] != 0 )
		{
			dwErr = TRC_Initialise_Trace( 
                szName, 
                szTrace, 
                TRC_OPT_FICHIER | TRC_OPT_CONSOLE | TRC_OPT_NUMEROTATION,
                &pcParams->hErr );
			if ( dwErr != NO_ERROR )
			{
				__leave;
			}
		}
		else
			pcParams->hErr = NULL;

		// Delete temporary directory
		DeleteDirContent( pcParams->szTempDir );
		// Load context files
		dwErr = LoadContexts( pcParams );
		if ( dwErr != NO_ERROR )
			__leave;
	}

    __finally
    {
        if ( ( dwErr != NO_ERROR ) && ( pcParams != NULL ) )
        {
			if ( pcParams->hTrc != NULL )
                TRC_Termine_Trace( pcParams->hTrc );

			if ( pcParams->hErr != NULL )
                TRC_Termine_Trace( pcParams->hErr );

            HeapFree( GetProcessHeap(), 0, pcParams );
            pcParams = NULL;
        }

        SetLastError( dwErr );
    }

	return (VOID*)pcParams;
}
//401 1
/**/
/*****************************************************************************/
/*SYNTAX: EXPORT void WINAPI WorkerClose( IN HANDLE hWrk )                   */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at the      */
/*            service exit.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN HANDLE hWrk  - Pointer to the module handle.                      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerClose=_WorkerClose@4" )
#endif
EXPORT void WINAPI WorkerClose( IN HANDLE hWrk )
{
	CMW_PARAMS * pcParams = hWrk;

	if ( pcParams->hTrc != NULL )
		TRC_Termine_Trace( pcParams->hTrc );

	if ( pcParams->hTrc != NULL )
		TRC_Termine_Trace( pcParams->hErr );

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
/*            converts the GEA PAQ file to CSSI message files.               */
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
DWORD WINAPI WorkerProcessFileEx( IN HANDLE hWrk,
								  IN char * szFilePath,
								  IN void * pvReserved )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwRet = NO_ERROR;
	CMW_PARAMS * pcParams = hWrk;
	HANDLE * hFile;
 
	__try
	{
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
			WorkError( hWrk, "WORKGEAFILE - Error openning file (%d)", dwErr );
			__leave;
		}

		dwErr = ConvertFile( pcParams, hFile );
		if ( dwErr != NO_ERROR )
		{
			WorkError( hWrk, "WORKGEAFILE - Error converting the file (%d)", dwErr );
			__leave;
		}

		dwErr = MoveDirContent( pcParams->szTempDir, pcParams->szOutDir );
		if ( dwErr != NO_ERROR )
		{
			WorkError( hWrk, "WORKGEAFILE - Error moving files (%d)", dwErr );
			__leave;
		}

		dwErr = SaveContexts( pcParams );
		if ( dwErr != NO_ERROR )
		{
			WorkError( hWrk, "WORKGEAFILE - Error saving contexts (%d)", dwErr );
			__leave;
		}

	}

	__finally
	{
		if ( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );

		SetLastError( dwErr );

		if( dwErr == NO_ERROR )
		{
			// Backup file
			dwRet = CM_WORKER_ACK_OK | CM_WORKER_ACT_MOVE_ACK;
		}
		else
		{
			// Delete temporary directory
			DeleteDirContent( pcParams->szTempDir );
			// Load context - return to previous counters
			LoadContexts( pcParams );
			// Move file to error directory
			if( pcParams->dwEnsureOrder == 0 )
				dwRet = CM_WORKER_ACK_ABANDON | CM_WORKER_ACT_MOVE_ERR;
			else
				dwRet = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_NONE;
		}
	}

	return dwRet;
}

/*****************************************************************************/
/*                     END OF THE EXPORTED FUNCTIONS                         */
/*****************************************************************************/

/*-------------------------------- END OF FILE ------------------------------*/