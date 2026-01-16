/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAIMG                                                    */
/* FILE:     cmworkgeaimg.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             converts the GEA image file to JPG file format renaming the   */
/*             image file. The file name is composed of the data stored in   */
/*             the GEA image file header and it uses the following           */
/*             convension: PPPPLLLYYYYMMDDHHMISEJJTTTTTI.JPG                 */
/*                PPPP  - plaza number                                       */
/*                LLL   - lane number                                        */
/*                YYYY  - year of the image acquisition (trs. generation)    */
/*                MM    - month of the image acquisition (trs. generation)   */
/*                DD    - day of the image acquisition (trs. generation)     */
/*                HH    - hour of the image acquisition (trs. generation)    */
/*                MI    - minute of the image acquisition (trs. generation)  */
/*                SE    - second of the image acquisition (trs. generation)  */
/*                JJ    - job number                                         */
/*                TTTTT - transaction number                                 */
/*                I     - picture index                                      */
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
#include <msg_gea_lc_image_header.h>

// Module interface
#define LOC_DEF
#include <cmworkgeaimg.h>
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
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_PARAMS
{
	char  szOutDir[CMWORK_MAX_PARAM_STR];
	DWORD dwNetMult;
	DWORD dwPurgeHour;
	DWORD dwPurgeMinute;
	DWORD dwPurgePeriod;
	SYSTEMTIME  sLastPurgeDTime;
	char  szWorkerName[MAX_PATH];
	TRC_EMETTEUR hTrc;
}CMW_PARAMS;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE DWORD SaveImage( IN char * pcBuffer,
						 IN DWORD dwBuffSize,
						 IN char *szFullPath );
PRIVATE DWORD DateToDays( IN SYSTEMTIME * sTime );
PRIVATE void PurgeOldFiles( IN char * pDir, IN DWORD dwPurgePeriod );
PRIVATE void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... );
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
	char szBuffer[MAX_PATH];

    __try
    {
        pcParams = HeapAlloc( GetProcessHeap(), 0, sizeof(CMW_PARAMS) );
        if ( pcParams == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

		// Save worker name
		strncpy_s(pcParams->szWorkerName, sizeof(pcParams->szWorkerName), szName, MAX_PATH);

		// Out directory (contains the JPG pictures)
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "OutDirectory", pcParams->szOutDir, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;
		// Network multiplier (GEA network number can be the part of the CSSI plaza ID)
		if ( REG_Lire_Entier( hKeyConfig, NULL, "NetworkMultiplier", &pcParams->dwNetMult ) != NO_ERROR )
			pcParams->dwNetMult = 0;
		// Purge time
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "PurgeTime", szBuffer, &dwSize );
        if ( dwErr != NO_ERROR )
		{
			pcParams->dwPurgeHour = 0;
			pcParams->dwPurgeMinute = 0;
		}
		else
		{
			if ( sscanf_s( szBuffer, "%d:%d", &pcParams->dwPurgeHour, &pcParams->dwPurgeMinute ) != 2 )
			{
				pcParams->dwPurgeHour = 0;
				pcParams->dwPurgeMinute = 0;
			}
		}
		// Purge period
		if ( REG_Lire_Entier( hKeyConfig, NULL, "PurgePeriod", &pcParams->dwPurgePeriod ) != NO_ERROR )
			pcParams->dwPurgePeriod = 0;
		// Last purge date time
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "LastPurgeDateTime", szBuffer, &dwSize );
        if ( dwErr != NO_ERROR )
		{
			memset( &pcParams->sLastPurgeDTime, 0, sizeof( SYSTEMTIME ) );
		}
		else
		{
			if ( sscanf_s( szBuffer,
						 "%d.%d.%d %d:%d",
						 &pcParams->sLastPurgeDTime.wDay,
						 &pcParams->sLastPurgeDTime.wMonth,
						 &pcParams->sLastPurgeDTime.wYear,
						 &pcParams->sLastPurgeDTime.wHour,
						 &pcParams->sLastPurgeDTime.wMinute ) != 5 )
			{
				memset( &pcParams->sLastPurgeDTime, 0, sizeof( SYSTEMTIME ) );
			}
		}

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
/*            extracts the picture(s) from the GEA image file.               */
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

	DWORD dwSize;
	DWORD dwRead;
	HANDLE * hFile;
	char * pcBuffer = NULL;
	DWORD dwMsgSize;
	char  szFullPath[MAX_PATH], szSubDir[MAX_PATH], szBuffer[MAX_PATH], szKey[MAX_PATH];
	struct MSG_GEA_IMAGE_HEADER *p_header = NULL;
	SYSTEMTIME sCurrentTime;
 
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
			WorkTrace( hWrk, "WORKGEAIMG - Error openning file (%d)", dwErr );
			__leave;
		}

		// Allocate the memory to store the message header
		pcBuffer = HeapAlloc( GetProcessHeap(), 0, MSG_GEA_IMAGE_HEADER_SIZE );
		if ( pcBuffer == NULL )
		{
			dwErr = GetLastError();
			WorkTrace( hWrk, "WORKGEAIMG - Error allocating the file header memory block (%d)", dwErr );
			__leave;
		}

        // Read the file header
        if ( ! ReadFile( hFile, pcBuffer, MSG_GEA_IMAGE_HEADER_SIZE, &dwRead, NULL ) )
        {
             dwErr = GetLastError();
			 WorkTrace( hWrk, "WORKGEAIMG - Error reaging file (%d)", dwErr );
             __leave;
        }
        if ( dwRead != MSG_GEA_IMAGE_HEADER_SIZE )
        {
             dwErr = ERROR_BAD_FILE;
			 WorkTrace( hWrk, "WORKGEAIMG - Error reading file (%d) (the file is not image)", dwErr );
             __leave;
        }

		p_header = MSG_GEA_IMAGE_HEADER_New();
		if ( p_header != NULL )
		{
		 	if ( MSG_GEA_IMAGE_HEADER_Read( p_header, (BYTE *)pcBuffer, MSG_GEA_IMAGE_HEADER_SIZE, &dwMsgSize ) )
			{
				if ( ( p_header->picture_size == 0) || ( p_header->num_pictures == 0 ) )
				{
					dwErr = ERROR_BAD_FILE;
					WorkTrace( hWrk, "WORKGEAIMG - Error (%d) picture size is 0", dwErr );
					__leave;
				}

				// Extract the first image
				HeapFree( GetProcessHeap(), 0, pcBuffer );
				pcBuffer = NULL;

				pcBuffer = HeapAlloc( GetProcessHeap(), 0, p_header->picture_size );
                if ( pcBuffer == NULL )
				{
                   dwErr = GetLastError();
				   WorkTrace( hWrk, "WORKGEAIMG - Error allocating memory (%d)", dwErr );
			       __leave;
				}
				if ( SetFilePointer( hFile, MSG_GEA_IMAGE_HEADER_SIZE, NULL, FILE_BEGIN ) != 0 )
				{
					if ( ! ReadFile( hFile, pcBuffer, p_header->picture_size, &dwRead, NULL ) )
					{
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKGEAIMG - Error reading file (%d)", dwErr );
						__leave;
					}
					if ( dwRead != p_header->picture_size )
					{
						dwErr = ERROR_BAD_FILE;
						WorkTrace( hWrk, "WORKGEAIMG - Error reading file (%d)", dwErr );
						__leave;
					}

					sprintf_s( szSubDir, 
						sizeof(szSubDir),
						"%s\\%03s\\%02d\\%02d\\",
						pcParams->szOutDir,
						p_header->lane_number,
						p_header->time_of_message.wMonth,
						p_header->time_of_message.wDay );

					MakeSureDirectoryPathExists( szSubDir );

					sprintf_s(szFullPath,
						sizeof(szFullPath),
						"%s%04d%03s%04d%02d%02d%02d%02d%02d%02d%05d1.jpg",
						szSubDir,
						atol( p_header->plaza_number ) + atol( p_header->network_number ) * pcParams->dwNetMult,
						p_header->lane_number,
						p_header->time_of_message.wYear,
						p_header->time_of_message.wMonth,
						p_header->time_of_message.wDay,
						p_header->time_of_message.wHour,
						p_header->time_of_message.wMinute,
						p_header->time_of_message.wSecond,
						p_header->shift_number,
						p_header->transaction_number);

					if ( SaveImage( pcBuffer, p_header->picture_size, szFullPath ) != NO_ERROR)
					{
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKGEAIMG - Error saving the first picture (%d)", dwErr );
						__leave;
					}

					HeapFree( GetProcessHeap(), 0, pcBuffer );
					pcBuffer = NULL;
				}
				else
				{
					dwErr = GetLastError();
					WorkTrace( hWrk, "WORKGEAIMG - Error setting the file pointer (%d)", dwErr );
					__leave;
				}

				// Extract the second image
				if( p_header->num_pictures == 2 )
				{
					dwSize = GetFileSize( hFile, NULL );
					dwSize = dwSize - MSG_GEA_IMAGE_HEADER_SIZE - p_header->picture_size;
					if ( dwSize <= 0 )
					{
						dwErr = ERROR_BAD_FILE;
						WorkTrace( hWrk, "WORKGEAIMG - Error (%d) picture size is 0", dwErr );
						__leave;
					}

					pcBuffer = HeapAlloc( GetProcessHeap(), 0, dwSize );
					if ( pcBuffer == NULL )
					{
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKGEAIMG - Error allocating the memory (%d)", dwErr );
						__leave;
					}

					if ( SetFilePointer( hFile, MSG_GEA_IMAGE_HEADER_SIZE + p_header->picture_size, NULL, FILE_BEGIN ) != 0 )
					{
						if ( ! ReadFile( hFile, pcBuffer, dwSize, &dwRead, NULL ) )
						{
							dwErr = GetLastError();
							WorkTrace( hWrk, "WORKGEAIMG - Error reading file (%d)", dwErr );
							__leave;
						}
						if ( dwRead != dwSize )
						{
							dwErr = ERROR_BAD_FILE;
							WorkTrace( hWrk, "WORKGEAIMG - Error reading file (%d)", dwErr );
							__leave;
						}

						sprintf_s(szFullPath, 
							sizeof(szFullPath),
							"%s%04d%03s%04d%02d%02d%02d%02d%02d%02d%05d2.jpg",
							szSubDir,
							atol( p_header->plaza_number ) + atol( p_header->network_number) * pcParams->dwNetMult,
							p_header->lane_number,
							p_header->time_of_message.wYear,
							p_header->time_of_message.wMonth,
							p_header->time_of_message.wDay,
							p_header->time_of_message.wHour,
							p_header->time_of_message.wMinute,
							p_header->time_of_message.wSecond,
							p_header->shift_number,
							p_header->transaction_number);

						if ( SaveImage( pcBuffer, dwSize, szFullPath ) != NO_ERROR )
						{
							dwErr = GetLastError();
							WorkTrace( hWrk, "WORKGEAIMG - Error saving the second picture (%d)", dwErr );
							__leave;
						}

						HeapFree( GetProcessHeap(), 0, pcBuffer );
						pcBuffer = NULL;
					}
					else
					{
						dwErr = GetLastError();
						WorkTrace( hWrk, "WORKGEAIMG - Error setting the file pointer (%d)", dwErr );
						__leave;
					}
				}
			}
			else
			{
				// Cannot read header
				dwErr = ERROR_BAD_FILE;
				WorkTrace( hWrk, "WORKGEAIMG - Error reading file header (%d)", dwErr );
				__leave;
			}
		}
		else
		{
			dwErr = GetLastError();
			WorkTrace( hWrk, "WORKGEAIMG - Error allocating the memory (%d)", dwErr );
            __leave;
		}

		// Purge directories
		if ( pcParams->dwPurgePeriod != 0 )
		{
			GetLocalTime( &sCurrentTime );

			if ( ( pcParams->sLastPurgeDTime.wYear == 0 ) ||
				 ( ( ( DateToDays( &sCurrentTime ) - DateToDays( &pcParams->sLastPurgeDTime ) ) >= 1 ) && 
				 ( sCurrentTime.wHour >= pcParams->dwPurgeHour ) &&
				 ( sCurrentTime.wMinute >= pcParams->dwPurgeMinute ) ) )
			{
				PurgeOldFiles( pcParams->szOutDir, pcParams->dwPurgePeriod );

				// Save the last purge date and time
				sprintf_s( szBuffer,
						sizeof(szBuffer),
						 "%d.%d.%d %d:%d",
						 sCurrentTime.wDay,
						 sCurrentTime.wMonth,
						 sCurrentTime.wYear,
						 sCurrentTime.wHour,
						 sCurrentTime.wMinute );

				sprintf_s( szKey,
						sizeof(szKey),
						 "%s%s\\",
						 "SYSTEM\\CurrentControlSet\\Services\\CommocSvc\\Parameters\\Workers\\",
						 pcParams->szWorkerName );

				REG_Ecrire_Chaine ( HKEY_LOCAL_MACHINE,
									szKey,
									"LastPurgeDateTime",
									szBuffer ); 
			}
		}
	}

	__finally
	{
		if ( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );

		if (pcBuffer != NULL)
		{
			HeapFree(GetProcessHeap(), 0, pcBuffer);
			pcBuffer = NULL;
		}

		if (p_header != NULL)
		{
			MSG_GEA_IMAGE_HEADER_Delete_All(p_header);
			p_header = NULL;
		}

		SetLastError( dwErr );

		if( dwErr == NO_ERROR )
		{
			// Backup file
			dwRet = CM_WORKER_ACK_OK | CM_WORKER_ACT_MOVE_ACK;
		}
		else
		{
			// Move file to error directory
			dwRet = CM_WORKER_ACK_ABANDON | CM_WORKER_ACT_MOVE_ERR;
		}
	}

	return dwRet;
}

/*****************************************************************************/
/*                     END OF THE EXPORTED FUNCTIONS                         */
/*****************************************************************************/

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE DWORD SaveImage( IN char * pcBuffer,                       */
/*                                 IN DWORD dwBuffSize,                      */
/*                                 IN char * szFullPath )                    */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service when it     */
/*            detects the file in the appropriate directory. The function    */
/*            extracts the picture(s) from the GEA image file.               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN char * pcBuffer   - Memory buffer that contains the extracted     */
/*                             picture                                       */
/*      IN DWORD dwBuffSize  - Memory buffer size in bytes                   */
/*      IN char * szFullPath - File name in which the picture will be        */
/*                             stored (contains the full path)               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              The picture is successfully saved.                 */
/*  !NO_ERROR             Error saving the file.                             */
/*****************************************************************************/

PRIVATE DWORD SaveImage( IN char * pcBuffer,
						 IN DWORD dwBuffSize,
						 IN char * szFullPath )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwWrite;
	HANDLE * hFile;

	hFile = CreateFile( szFullPath, 
                        GENERIC_WRITE, 
                        0, 
                        NULL, 
                        CREATE_ALWAYS, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL );

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		dwErr = GetLastError();
	}
	else
	{
		if ( ! WriteFile( hFile, pcBuffer, dwBuffSize, &dwWrite, NULL ) )
			dwErr = GetLastError();
		if ( dwWrite != dwBuffSize )
			dwErr = GetLastError();

		CloseHandle( hFile );
	}

	return dwErr;
}

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE DWORD DateToDays( IN SYSTEMTIME * sTime )                  */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function calculates the number of days from 1.1.0000.     */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN SYSTEMTIME * sTime - Pointer to the date and time structure.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 This function returns the number of days.          */
/*****************************************************************************/

PRIVATE DWORD DateToDays( IN SYSTEMTIME * sTime )
{
	DWORD dwDays;
	DWORD dwDays1[12] = { 0,
				/* +31 */ 31,
				/* +28 */ 59,
				/* +31 */ 90,
				/* +30 */ 120,
				/* +31 */ 151,
				/* +30 */ 181,
				/* +31 */ 212,
				/* +31 */ 243,
				/* +30 */ 273,
				/* +31 */ 304,
				/* +30 */ 334 };

	DWORD dwDays2[12] = { 0,
				/* +31 */ 31,
				/* +29 */ 60,
				/* +31 */ 91,
				/* +30 */ 121,
				/* +31 */ 152,
				/* +30 */ 182,
				/* +31 */ 213,
				/* +31 */ 244,
				/* +30 */ 274,
				/* +31 */ 305,
				/* +30 */ 335 };

	dwDays = sTime->wYear*365 +
			 ( DWORD )( sTime->wYear/4 ) -
			 ( DWORD )( sTime->wYear/100 ) +
			 ( DWORD )( sTime->wYear/400 );

	if ( ( sTime->wYear%400 == 0 ) ||
		 ( ( sTime->wYear%4 == 0 ) && ( sTime->wYear%100 !=0 ) ) )
	{
		dwDays = dwDays + dwDays2[sTime->wMonth-1];
	}
	else
	{
		dwDays = dwDays + dwDays1[sTime->wMonth-1];
	}

	dwDays = dwDays + sTime->wDay;

	return dwDays;
}

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE void PurgeOldFiles( IN char * pDir,                        */
/*                                    IN DWORD dwPurgePeriod )               */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function purges the pictures older than the date          */
/*            calculated by substracting the dwPurgeParameter parameter from */ 
/*            the system date.                                               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN char * pDir         - Directory that will be purged. The          */
/*                               procedure will purge sub-directories too.   */
/*      IN DWORD dwPurgePeriod - After number of days determined by this     */
/*                               parameter the picture will be deleted.      */ 
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 This function returns the number of days.          */
/*****************************************************************************/

PRIVATE void PurgeOldFiles( IN char * pDir, IN DWORD dwPurgePeriod )
{
	HANDLE hFindFile;
	WIN32_FIND_DATA sFileData;
	char szDir[MAX_PATH], szSubDir[MAX_PATH], szFileName[MAX_PATH];
	SYSTEMTIME sFileTime, sCurrentTime;
	DWORD dwTemp1, dwTemp2, dwScaned;

	GetLocalTime( &sCurrentTime );
	sprintf_s(szDir, sizeof(szDir), "%s\\*", pDir);
	hFindFile = FindFirstFile( szDir, &sFileData );

	if ( hFindFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( sFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( ( strcmp( sFileData.cFileName, "." ) != 0 ) &&
					 ( strcmp( sFileData.cFileName, ".." ) != 0 ) )
				{
					sprintf_s(szSubDir, sizeof(szSubDir), "%s\\%s", pDir, sFileData.cFileName);
					PurgeOldFiles( szSubDir, dwPurgePeriod );
					RemoveDirectory( szSubDir );
				}
			}
			else
			{
				dwScaned = sscanf_s( sFileData.cFileName,
								   "%04d%03d%04d%02d%02d%02d%02d%02d",
								   &dwTemp1,
								   &dwTemp2,
								   &sFileTime.wYear,
								   &sFileTime.wMonth,
								   &sFileTime.wDay,
								   &sFileTime.wHour,
								   &sFileTime.wMinute,
								   &sFileTime.wSecond );
				if ( dwScaned !=8 )
					DeleteFile( szFileName );

				if ( DateToDays( &sCurrentTime ) - DateToDays( &sFileTime ) > dwPurgePeriod )
				{
					sprintf_s(szFileName, sizeof(szFileName), "%s\\%s", pDir, sFileData.cFileName);
					DeleteFile( szFileName );
				}
			}

			Sleep( 10 );
		}
		while( FindNextFile( hFindFile, &sFileData ) != 0 );
	}

	FindClose( hFindFile );
}

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... ) */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the trace file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN HANDLE hWork    - Pointer to module parameters                    */
/*      IN char * szFormat - Pointer to the format string                    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PRIVATE void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... )
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