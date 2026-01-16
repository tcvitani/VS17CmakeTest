/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_file.c                                            */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage reading and      */
/*             writing to the GEA and CSSI files.                            */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>
// CSSI interface
#include <trc.h>

// Module interface
#include <cmworkgeafile.h>
#include <cmworkgeafile_convert.h>
#define LOC_DEF
#include <cmworkgeafile_file.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define	MSG_GEA_FILE_LINE_LENGTH_SIZE		8
#define MSG_GEA_MAX_FILE_LINE_LENGTH		5000
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEAReadNextMessage( IN CMW_PARAMS * pcParams,                */
/*                                  IN HANDLE * hFile,                       */
/*                                  OUT char ** pcMessage )                  */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD GEAReadNextMessage( IN CMW_PARAMS * pcParams, 
									IN HANDLE * hFile,
									OUT char ** pcMessage )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwRead, dwMsgLength;
	char szMsgLength[10];

	memset( szMsgLength, 0, 10);
	if ( ! ReadFile( hFile, szMsgLength, MSG_GEA_FILE_LINE_LENGTH_SIZE, &dwRead, NULL ) )
	{
		dwErr = GetLastError();
		return dwErr;
	}

	if ( dwRead == 0 )
		return ERROR_HANDLE_EOF;
	else if ( dwRead != MSG_GEA_FILE_LINE_LENGTH_SIZE )
		return ERROR_BAD_FILE;

	dwMsgLength = atol( szMsgLength ) + 2;

	WorkTrace( pcParams, "\r\n==> Message length = %d", dwMsgLength-2 );

	if ( dwMsgLength <= 2 )
		return ERROR_HANDLE_EOF;

	if ( *pcMessage != NULL )
		HeapFree( GetProcessHeap(), 0, *pcMessage );

	// MFR start
//	*pcMessage = HeapAlloc( GetProcessHeap(), 0, dwMsgLength );
	*pcMessage = HeapAlloc( GetProcessHeap(), 0, dwMsgLength + 1 );
	// MFR end

	if ( *pcMessage == NULL )
		return GetLastError();

	if ( ! ReadFile( hFile, *pcMessage, dwMsgLength, &dwRead, NULL ) )
	{
		dwErr = GetLastError();
		HeapFree( GetProcessHeap(), 0, *pcMessage );
		*pcMessage = NULL;

		return dwErr;
	}

	if ( dwRead != dwMsgLength )
	{
		dwErr = ERROR_BAD_FILE;
		HeapFree( GetProcessHeap(), 0, *pcMessage );
		*pcMessage = NULL;

		return dwErr;
	}

	strncpy_s(*pcMessage, dwMsgLength, *pcMessage + 1, dwMsgLength - 2);
	( *pcMessage )[dwMsgLength - 2] = '\0';
	( *pcMessage )[dwMsgLength - 1] = '\0';

	return dwErr;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEAAppendMessage( IN HANDLE * hFile, IN char * szBuffer )    */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD GEAAppendMessage( IN HANDLE * hFile, IN char * szBuffer )
{
	char szLine[MSG_GEA_MAX_FILE_LINE_LENGTH];
	DWORD dwWrite, dwError;
	size_t iLength = 0;

	iLength = strlen(szBuffer);
	sprintf_s(szLine, sizeof(szLine), "%8d %s\n", iLength, szBuffer);

	if (!WriteFile(hFile, szLine, (DWORD)iLength + 10, &dwWrite, NULL))
		dwError = GetLastError();
	if (dwWrite != iLength + 10)
		dwError = GetLastError();

	return dwError;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD AppendMessage( IN HANDLE * hFile,                            */
/*                             IN char * szBuffer,                           */
/*                             IN DWORD dwLength )                           */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD AppendMessage( IN HANDLE * hFile,
							   IN char * szBuffer,
							   IN DWORD dwLength )
{
	DWORD dwWrite, dwError;

	if ( ! WriteFile( hFile, szBuffer, dwLength, &dwWrite, NULL ) )
		dwError = GetLastError();
	if ( dwWrite != dwLength )
		dwError = GetLastError();

	return dwError;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD MoveDirContent( IN char * szSourceDir, IN char * szDestDir ) */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD MoveDirContent( IN char * szSourceDir, IN char * szDestDir )
{
	HANDLE hFind;
	WIN32_FIND_DATA sFind;
	char szFullMask[MAX_PATH];
	char szSourceFile[MAX_PATH], szDestFile[MAX_PATH];

	sprintf_s(szFullMask, sizeof(szFullMask), "%s\\*", szSourceDir);
    szFullMask[sizeof(szFullMask)-1] = '\0';

	hFind = FindFirstFile( szFullMask, &sFind );
    while ( hFind != INVALID_HANDLE_VALUE )
    {
		if ( ( sFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
        {
			sprintf_s(szSourceFile, sizeof(szSourceFile), "%s\\%s", szSourceDir, sFind.cFileName);
			sprintf_s(szDestFile, sizeof(szDestFile), "%s\\%s", szDestDir, sFind.cFileName);
			if ( MoveFileEx( szSourceFile, szDestFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) == 0 )
			{
				FindClose( hFind );
				return GetLastError();
			}
		}

		if ( ! FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD DeleteDirContent( IN char * szDir )                          */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD DeleteDirContent( IN char * szDir )
{
	HANDLE hFind;
	WIN32_FIND_DATA sFind;
	char szFullMask[MAX_PATH];
	char szFile[MAX_PATH];

	sprintf_s(szFullMask, sizeof(szFullMask), "%s\\*", szDir);
    szFullMask[sizeof(szFullMask)-1] = '\0';

	hFind = FindFirstFile( szFullMask, &sFind );
    while ( hFind != INVALID_HANDLE_VALUE )
    {
		if ( ( sFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
        {
			sprintf_s(szFile, sizeof(szFile), "%s\\%s", szDir, sFind.cFileName);
			if ( DeleteFile( szFile ) == 0 )
			{
				FindClose( hFind );
				return GetLastError();
			}
		}

		if ( ! FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
	}

	return NO_ERROR;
}
/*-------------------------------- END OF FILE ------------------------------*/