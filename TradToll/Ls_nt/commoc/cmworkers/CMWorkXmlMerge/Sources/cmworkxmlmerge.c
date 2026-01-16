/******************* (v) 2018 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKXMLMERGE                                                  */
/* FILE:     cmworkxmlmerge.c                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             merges the lsit of users exported from the toll system to the */
/*             XML file and list of users stored in the XML file on the Scan */
/*             Coin PC.                                                      */
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
#include <xml_tools.h>
#include <cmworkxmlmerge_parser.h>

// Module interface
#define LOC_DEF
#include <cmworkxmlmerge.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define CMWORK_MAX_STR              512
#define CMWORK_MAX_PARAM_STR        100
#define CMWORK_MAX_FILE_NAME        MAX_PATH

#define USER_FILE_RECORD_SIZE      400
#define USER_FILE_HEADER_SIZE      100
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_PARAMS
{
	char                   szOutDir[CMWORK_MAX_PARAM_STR];
	char                   szBackupDir[CMWORK_MAX_PARAM_STR];
	char                   szTmpDir[CMWORK_MAX_PARAM_STR];
	char                   szDestinationFileName[CMWORK_MAX_FILE_NAME];

	DWORD                  dwPurgeHour;
	DWORD                  dwPurgeMinute;
	DWORD                  dwPurgePeriod;
	SYSTEMTIME             sLastPurgeDTime;
	char                   szWorkerName[MAX_PATH];
	TRC_EMETTEUR           hTrc;

	struct_PARSER_CONTEXT *psParserContext;
	struct_PARSER_CONTEXT *psParserContextOrig;
}CMW_PARAMS;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE DWORD DateToDays( IN SYSTEMTIME * sTime );
PRIVATE void PurgeOldFiles(IN HANDLE hWrk, IN char * pDir, IN DWORD dwPurgePeriod);
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
	char szTrace[CMWORK_MAX_PARAM_STR] = { 0 };
	char szBuffer[MAX_PATH] = { 0 };

    __try
    {
		pcParams = HeapAlloc(GetProcessHeap(), 0, sizeof(CMW_PARAMS));
        if ( pcParams == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

		// Save worker name
		strncpy_s(pcParams->szWorkerName, sizeof(pcParams->szWorkerName), szName, MAX_PATH);

		// Out directory (contains the final XML file used by Scan Coin)
		dwSize=CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "OutDirectory", pcParams->szOutDir, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;

		// Backup directory (contains original XML saved before merging)
		dwSize = CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine(hKeyConfig, NULL, "BackupDirectory", pcParams->szBackupDir, &dwSize);
		if (dwErr != NO_ERROR)
			__leave;

		// Temporary directory (file merging occurs in this foder)
		dwSize = CMWORK_MAX_PARAM_STR;
		dwErr = REG_Lire_Chaine(hKeyConfig, NULL, "TmpDirectory", pcParams->szTmpDir, &dwSize);
		if (dwErr != NO_ERROR)
			__leave;

		// File name used by Scan Coin system 
		dwSize = CMWORK_MAX_FILE_NAME;
		dwErr = REG_Lire_Chaine(hKeyConfig, NULL, "DestinationFileName", pcParams->szDestinationFileName, &dwSize);
		if (dwErr != NO_ERROR)
			__leave;

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
/*            merges the XML file generated by the toll system with the file */
/*            used/managed by the Scan Coin system. File is merged by        */
/*            keeping password of existing users unchanged.                  */
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

	DWORD dwWritten = 0;
	HANDLE hFileDest = NULL, hFileTmp = NULL;
	char  szFullTmpFileName[MAX_PATH] = { 0 }, szBuffer[MAX_PATH] = { 0 }, szKey[MAX_PATH] = { 0 };
	char  szFullOutFileName[MAX_PATH] = { 0 }, szFullBackupFileName[MAX_PATH] = { 0 };
	SYSTEMTIME sCurrentTime = { 0 };
	char szLastErrorMessage[MAX_PATH] = { 0 };
	int iLastErrorLineNumber;
	struct_FILE_DATA_XML sSourceXmlElement = { 0 }, sOriginalXmlElement = { 0 };
	enum_xml_tools_result	eStatus = enuXML_TOOLS_OK;
	DWORD dwNewFileSize = 0;
	unsigned char *pcXmlData = NULL;
	XML_TOOLS_Attribute	sAtrList = { 0 };
 
	__try
	{
		// Create full file name with the path
		if (pcParams->szOutDir[strlen(pcParams->szOutDir) - 1] == '\\')
			sprintf_s(szFullOutFileName, MAX_PATH, "%s%s", pcParams->szOutDir, pcParams->szDestinationFileName);
		else
			sprintf_s(szFullOutFileName, MAX_PATH, "%s\\%s", pcParams->szOutDir, pcParams->szDestinationFileName);

		if (pcParams->szTmpDir[strlen(pcParams->szTmpDir) - 1] == '\\')
			sprintf_s(szFullTmpFileName, MAX_PATH, "%s%s", pcParams->szTmpDir, pcParams->szDestinationFileName);
		else
			sprintf_s(szFullTmpFileName, MAX_PATH, "%s\\%s", pcParams->szTmpDir, pcParams->szDestinationFileName);

		// Check if the file in the destination folder exists
		hFileDest = CreateFile( szFullOutFileName,
								GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
		if (hFileDest == INVALID_HANDLE_VALUE)
		{
			// Move new file to the destination folder
			WorkTrace(hWrk, "WORKXMLMERGE - Original file does not exist. System init");

			// Copy to temp
			if (CopyFile(szFilePath, szFullTmpFileName, FALSE) == FALSE)
			{
				dwErr = GetLastError();
				WorkTrace(hWrk, "WORKXMLMERGE - Error coping file %s to %s (Error code: %d)", szFilePath, szFullTmpFileName, dwErr);
				__leave;
			}

			// Move to destination
			if(MoveFileEx(szFullTmpFileName, szFullOutFileName, MOVEFILE_REPLACE_EXISTING) == FALSE)
			{
				dwErr = GetLastError();
				WorkTrace(hWrk, "WORKXMLMERGE - Error moving file %s to %s (Error code: %d)", szFullTmpFileName, szFullOutFileName, dwErr);
				__leave;
			}
		}
		else
		{
			// File exists
			WorkTrace(hWrk, "WORKXMLMERGE - Merge file %s and %s", szFilePath, szFullOutFileName);

			CloseHandle(hFileDest);

			// Backup original file
			GetLocalTime(&sCurrentTime);
			if (pcParams->szBackupDir[strlen(pcParams->szBackupDir) - 1] == '\\')
			{
				sprintf_s(	szFullBackupFileName,
							MAX_PATH,
							"%s%04d%02d%02d_%02d%02d%02d%03d_%s",
							pcParams->szBackupDir,
							sCurrentTime.wYear,
							sCurrentTime.wMonth,
							sCurrentTime.wDay,
							sCurrentTime.wHour,
							sCurrentTime.wMinute,
							sCurrentTime.wSecond,
							sCurrentTime.wMilliseconds,
							pcParams->szDestinationFileName);
			}
			else
			{
				sprintf_s(  szFullBackupFileName,
							MAX_PATH,
							"%s\\%04d%02d%02d_%02d%02d%02d%03d_%s",
							pcParams->szBackupDir,
							sCurrentTime.wYear,
							sCurrentTime.wMonth,
							sCurrentTime.wDay,
							sCurrentTime.wHour,
							sCurrentTime.wMinute,
							sCurrentTime.wSecond,
							sCurrentTime.wMilliseconds,
							pcParams->szDestinationFileName);
			}

			if (CopyFile(szFullOutFileName, szFullBackupFileName, TRUE) == FALSE)
			{
				dwErr = GetLastError();
				WorkTrace(hWrk, "WORKXMLMERGE - Error coping file %s to %s (Error code: %d)", szFullOutFileName, szFullBackupFileName, dwErr);
				__leave;
			}

			// Merge two files
			// Init coontext for the source file 
			pcParams->psParserContext = XML_PARSER_CONTEXT_New();

			if (pcParams->psParserContext != NULL)
			{
				eStatus = XML_InitXmlParser(pcParams->psParserContext->pXmlCtx, TRUE);
			}

			if (eStatus != enuXML_TOOLS_OK)
			{
				WorkTrace(pcParams, "Error Initializing parser fro the new received file! XML tools error: %d", eStatus);

				dwErr = eStatus;
				__leave;
			}

			// Init coontext for the original file 
			pcParams->psParserContextOrig = XML_PARSER_CONTEXT_New();

			if (pcParams->psParserContextOrig != NULL)
			{
				eStatus = XML_InitXmlParser(pcParams->psParserContextOrig->pXmlCtx, TRUE);
			}

			if (eStatus != enuXML_TOOLS_OK)
			{
				WorkTrace(pcParams, "Error Initializing parser fro the original file! XML tools error: %d", eStatus);

				dwErr = eStatus;
				__leave;
			}

			// Load source file (received from toll system)
			if (FILE_XML_Load(pcParams->psParserContext, szFilePath) != enuXML_TOOLS_OK)
			{
				if (XML_GetLastError(pcParams->psParserContext->pXmlCtx,
									szLastErrorMessage,
									MAX_PATH,
									szLastErrorMessage,
									MAX_PATH,
									&iLastErrorLineNumber) == TRUE)
				{
					WorkTrace(hWrk, "WORKXMLMERGE - Error %s parsing source XML file in line no. %d", szLastErrorMessage, iLastErrorLineNumber);
				}
				else
				{
					WorkTrace(hWrk, "WORKXMLMERGE - Error parsing source XML file (unknown error)");
				}

				dwErr = ERROR_BAD_FILE;
				__leave;
			}
			else
			{
				// Load the original file (used by Scan Coin)
				if (FILE_XML_Load(pcParams->psParserContextOrig, szFullOutFileName) != enuXML_TOOLS_OK)
				{
					if (XML_GetLastError(pcParams->psParserContextOrig->pXmlCtx,
										szLastErrorMessage,
										MAX_PATH,
										szLastErrorMessage,
										MAX_PATH,
										&iLastErrorLineNumber) == TRUE)
					{
						WorkTrace(hWrk, "WORKXMLMERGE - Error %s parsing original XML file in line no. %d", szLastErrorMessage, iLastErrorLineNumber);
					}
					else
					{
						WorkTrace(hWrk, "WORKXMLMERGE - Error parsing original XML file (unknown error)");
					}

					dwErr = ERROR_BAD_FILE;
					__leave;
				}

				// Get number of elements and calculate the new file size
				dwNewFileSize = GetNumberOfElements(pcParams->psParserContext) * USER_FILE_RECORD_SIZE + USER_FILE_HEADER_SIZE;
				// Allocate corresponding memory block
				pcXmlData = HeapAlloc(GetProcessHeap(), 0, dwNewFileSize);
				if (pcXmlData == NULL)
				{
					dwErr = GetLastError();
					__leave;
				}
				memset(pcXmlData, 0, dwNewFileSize);

				// <?xml version="1.0" encoding="UTF-8"?>
				XML_AddXmlVerAndEncodingAndStandalone(pcXmlData, dwNewFileSize, XML_TOOLS_XML_VER_1_0, XML_TOOLS_UTF8_ENCODING, NULL, ADD_NEW_LINE_AT_THE_END);
				// <users>
				XML_AddRootElement(pcXmlData, dwNewFileSize, "users", NULL, NULL, ADD_NEW_LINE_AT_THE_END);

				// For every element in the source file
				while (GetNextElement(pcParams->psParserContext, &sSourceXmlElement) == TRUE)
				{
					if (FindElementByIdentifier(pcParams->psParserContextOrig, sSourceXmlElement.szLogin, &sOriginalXmlElement) == TRUE)
					{
						strncpy_s(sSourceXmlElement.szIdentifier, MAX_LOCAL_ORDER_LEN + 1, sOriginalXmlElement.szIdentifier, MAX_LOCAL_ORDER_LEN);
						if (strcmp(sOriginalXmlElement.szRole, sSourceXmlElement.szRole) == 0)
						{
							strncpy_s(sSourceXmlElement.szPassword, MAX_LOCAL_ORDER_LEN + 1, sOriginalXmlElement.szPassword, MAX_LOCAL_ORDER_LEN);
							strncpy_s(sSourceXmlElement.szChangePinOnLogin, MAX_LOCAL_ORDER_LEN + 1, sOriginalXmlElement.szChangePinOnLogin, MAX_LOCAL_ORDER_LEN);
						}
					}
					// WorkTrace(hWrk, "WORKXMLMERGE - Login: %s, old pwd: %s, new pwd %s", sSourceXmlElement.szLogin, sSourceXmlElement.szPassword, sOriginalXmlElement.szPassword);

					XML_ResetAttributeList(&sAtrList);

					XML_AddStringAttributeToList(&sAtrList, "login", sSourceXmlElement.szLogin, (DWORD)strnlen_s(sSourceXmlElement.szLogin, MAX_LOCAL_ORDER_LEN));
					XML_AddStringAttributeToList(&sAtrList, "password", sSourceXmlElement.szPassword, (DWORD)strnlen_s(sSourceXmlElement.szPassword, MAX_LOCAL_ORDER_LEN));
					XML_AddStringAttributeToList(&sAtrList, "name", sSourceXmlElement.szName, (DWORD)strnlen_s(sSourceXmlElement.szName, MAX_LOCAL_ORDER_LEN));
					XML_AddStringAttributeToList(&sAtrList, "identifier", sSourceXmlElement.szIdentifier, (DWORD)strnlen_s(sSourceXmlElement.szIdentifier, MAX_LOCAL_ORDER_LEN));
					XML_AddStringAttributeToList(&sAtrList, "role", sSourceXmlElement.szRole, (DWORD)strnlen_s(sSourceXmlElement.szRole, MAX_LOCAL_ORDER_LEN));

					if (strlen(sSourceXmlElement.szPassword) > 0)
					{
						if (strlen(sSourceXmlElement.szChangePinOnLogin) > 0) // || strcmp(sOriginalXmlElement.szRole, sSourceXmlElement.szRole))
							XML_AddStringAttributeToList(&sAtrList, "changePinOnLogin", sSourceXmlElement.szChangePinOnLogin, (DWORD)strnlen_s(sSourceXmlElement.szChangePinOnLogin, MAX_LOCAL_ORDER_LEN));
					}
					else
						XML_AddStringAttributeToList(&sAtrList, "noPin", sSourceXmlElement.szNoPin, (DWORD)strnlen_s(sSourceXmlElement.szNoPin, MAX_LOCAL_ORDER_LEN));

					XML_AddEmptyElement(pcXmlData, dwNewFileSize, "user", &sAtrList, ADD_NEW_LINE_AT_THE_END);
				}

				// </users>
				XML_AddEndElement(pcXmlData, dwNewFileSize, "users", ADD_NEW_LINE_AT_THE_END);

				// Create temp file
				hFileTmp = CreateFile(  szFullTmpFileName,
										GENERIC_WRITE,
										0,
										NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL);
				if (hFileTmp == INVALID_HANDLE_VALUE)
				{
					dwErr = GetLastError();
					WorkTrace(hWrk, "WORKXMLMERGE - Error creating file %s (Error code: %d)", szFullTmpFileName, dwErr);
					__leave;
				}

				WriteFile(hFileTmp, pcXmlData, (DWORD)strnlen_s(pcXmlData, dwNewFileSize), &dwWritten, NULL);
				CloseHandle(hFileTmp);

				// Move temp file to final destination, overwrite the original file
				if (MoveFileEx(szFullTmpFileName, szFullOutFileName, MOVEFILE_REPLACE_EXISTING) == FALSE)
				{
					dwErr = GetLastError();
					WorkTrace(hWrk, "WORKXMLMERGE - Error moving file %s to %s (Error code: %d)", szFullTmpFileName, szFullOutFileName, dwErr);
					__leave;
				}
			}
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
				PurgeOldFiles(hWrk, pcParams->szBackupDir, pcParams->dwPurgePeriod);

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
		if ( hFileDest != INVALID_HANDLE_VALUE )
			CloseHandle(hFileDest);

		if ( hFileTmp != INVALID_HANDLE_VALUE )
			CloseHandle(hFileTmp);

		if (pcParams->psParserContext != NULL)
		{
			XML_PARSER_CONTEXT_Delete_All(&(pcParams->psParserContext));
			pcParams->psParserContext = NULL;
		}

		if (pcParams->psParserContextOrig != NULL)
		{
			XML_PARSER_CONTEXT_Delete_All(&(pcParams->psParserContextOrig));
			pcParams->psParserContextOrig = NULL;
		}

		if (pcXmlData != NULL)
			HeapFree(GetProcessHeap(), 0, pcXmlData);

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
/*SYNTAX: PRIVATE void PurgeOldFiles( IN HANDLE hWrk,                        */
/*                                    IN char * pDir,                        */
/*                                    IN DWORD dwPurgePeriod )               */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function purges the files older than the date             */
/*            calculated by substracting the dwPurgeParameter parameter from */ 
/*            the system date.                                               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN HANDLE hWrk         - Pointer to the module handle.               */
/*      IN char * pDir         - Directory that will be purged. The          */
/*                               procedure will purge sub-directories too.   */
/*      IN DWORD dwPurgePeriod - After number of days determined by this     */
/*                               parameter the picture will be deleted.      */ 
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 This function returns the number of days.          */
/*****************************************************************************/

PRIVATE void PurgeOldFiles(IN HANDLE hWrk, IN char * pDir, IN DWORD dwPurgePeriod)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA sFileData;
	char szDir[MAX_PATH], szSubDir[MAX_PATH], szFileName[MAX_PATH], szFileSufix[MAX_PATH];
	SYSTEMTIME sFileTime, sCurrentTime;
	DWORD dwScaned;

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
					PurgeOldFiles(hWrk, szSubDir, dwPurgePeriod);
					RemoveDirectory( szSubDir );
				}
			}
			else
			{
				dwScaned = sscanf_s( sFileData.cFileName,
								   "%04d%02d%02d_%02d%02d%02d%s",
								   &sFileTime.wYear,
								   &sFileTime.wMonth,
								   &sFileTime.wDay,
								   &sFileTime.wHour,
								   &sFileTime.wMinute,
								   &sFileTime.wSecond,
								   szFileSufix );
				if ( dwScaned != 7 )
					DeleteFile( szFileName );

				if ( DateToDays( &sCurrentTime ) - DateToDays( &sFileTime ) > dwPurgePeriod )
				{
					sprintf_s(szFileName, sizeof(szFileName), "%s\\%s", pDir, sFileData.cFileName);
					DeleteFile( szFileName );

					WorkTrace(hWrk, "WORKXMLMERGE - Purge file %s from the backup folder", sFileData.cFileName);
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