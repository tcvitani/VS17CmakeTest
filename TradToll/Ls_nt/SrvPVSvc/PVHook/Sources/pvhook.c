/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     pvhook.c														 */
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if (_MSC_VER == 1200) && !defined(_WIN64)
#include <fstream>
#else if (_MSC_VER == 1600)
#include <math.h>
#endif

#include <io.h>
#include <errno.h>
#include <share.h>

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

#include <direct.h>
#include <stdlib.h>

#define LOC_DEF
#include <pvhook.h>
#undef LOC_DEF

#include "ntsvc.h"

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define PVHOOK_REG_VAL_RETENTION    "PvHookDaysOfRetention"
#define PVHOOK_REG_VAL_FILEPATH     "PvHookFilesPath"
#define PVHOOK_REG_VAL_ENABLE       "PvHookEnable"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

PRIVATE struct
{
	DWORD dwRetention;
	char szFilePath[MAX_PATH];
	DWORD dwEnable;
}
PVHOOK = { 0 };

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PvhookError Pvhook_SetFileName(unsigned char* pName);
PvhookError Pvhook_DeleteFile(void);
PvhookError Pvhook_GetPath(unsigned char* pPath);

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
EXPORT PvhookError WINAPI Pvhook_NTSVC_Inherit_Handle(void *pvContext)
{
	DWORD dwErr = 0;
	NTSVC_PARAMETER_DEF * psParams; // paramètres du registre

	// on récupère un context de paramétrage de service
	NTSVCSetContext(pvContext);

	// récupération des paramètres dans le registre
	psParams = NTSVCOpenParameters(
		PVHOOK_REG_VAL_RETENTION, REG_DWORD, 4, 20, &PVHOOK.dwRetention,
		PVHOOK_REG_VAL_FILEPATH, REG_SZ, MAX_PATH, "C:\\CSR\\PvHook", &PVHOOK.szFilePath,
		PVHOOK_REG_VAL_ENABLE, REG_DWORD, 4, FALSE, &PVHOOK.dwEnable,
		NULL);

	if (psParams == NULL)
		return Pvhook_CannotOpenReg;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return Pvhook_CannotReadReg;

	NTSVCCloseParameters(psParams);

	return Pvhook_Ok;
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
EXPORT PvhookError WINAPI Pvhook_DataWrite(DWORD DataSize, BYTE *pDataBuffer)
{
	unsigned char	FileName[_MAX_PATH] = { 0 };
	unsigned char	PathName[_MAX_PATH] = { 0 };
	PvhookError		Err					= Pvhook_Ok;
	int				fh					= 0;	
	BOOL			YaErr				= FALSE;	
	errno_t			err					= 0;

	if (PVHOOK.dwEnable)
	{
		if ((Err = Pvhook_SetFileName(&FileName[0])) == Pvhook_Ok)
		{
			if ((Err = Pvhook_GetPath(&PathName[0])) == Pvhook_Ok)
			{
				strcat_s(PathName, sizeof(PathName), "\\\\");
				_chdir(PathName);
				printf("Goto %s\n", PathName);

				err = _sopen_s(&fh, FileName,
					_O_WRONLY | _O_BINARY | _O_APPEND,
					_SH_DENYNO,
					_S_IREAD | _S_IWRITE);

				if (fh == -1)
				{
					switch (err)
					{
						// Tried to open read-only file for writing, or
						// file’s sharing mode does not allow specified operations, or
						// given path is directory
						case EACCES:	
							YaErr = TRUE;
							Err = Pvhook_NoAcces;
							break;

						// _O_CREAT and _O_EXCL flags specified, but filename already exists
						case EEXIST:
							YaErr = TRUE;
							Err = Pvhook_GrosBug;
							break;

						// Invalid flag or pmode argument 
						case EINVAL:
							YaErr = TRUE;
							Err = Pvhook_GrosBug;
							break;

						// No more file handles available (too many open files)
						case EMFILE:
							YaErr = TRUE;
							Err = Pvhook_OpenLimit;
							break;

						// File or path not found
						case ENOENT:	
							// File not found, create it
							err = _sopen_s(&fh,
										FileName,
										_O_WRONLY | _O_CREAT | _O_BINARY | _O_APPEND,
										_SH_DENYNO,
										_S_IREAD | _S_IWRITE);
							printf("File not exist :( -> Create %s\n", FileName);
							Err = Pvhook_DeleteFile();		// Delete file if any...
							// Pas 'YaErr' because on va tenter d'ecrire kan meme
							break;

						default:
							YaErr = TRUE;
							Err = Pvhook_GrosBug;
							break;
					}
				}

				if (!YaErr)
				{
					printf("Fichier ready  :) -> Write %s\n", FileName);
					_write(fh, &DataSize, sizeof(DWORD));
					_write(fh, pDataBuffer, DataSize);
					_close(fh);
				}
			}
		}
	}

	return(Err);
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
PvhookError Pvhook_SetFileName(unsigned char* pName)
{
	struct tm		newtime;
	time_t			ltime;
	int				LgLocName;
	BYTE			LocName[30];
	unsigned char	*pLoc = &LocName[0];
	errno_t			err;

	time(&ltime);
	err = localtime_s(&newtime, &ltime);
	// Size:
	LgLocName = sprintf_s(pLoc, sizeof(LocName), "%04d", (newtime.tm_year) + 1900);							//	 4
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, "%02d", (newtime.tm_mon) + 1);	//	 6
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, "%02d", (newtime.tm_mday));		//   8
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, "%02d", (newtime.tm_hour));		//  10
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, ".");								//  11
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, "00");							//  13
	LgLocName += sprintf_s(pLoc + LgLocName, sizeof(LocName) - LgLocName, "hor");							//  16
	pLoc[LgLocName] = 0x00;																					//  17

	if (strcpy_s(pName, _MAX_PATH, pLoc) == 0)
		return Pvhook_Ok;

	return Pvhook_NoName;
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
PvhookError Pvhook_DeleteFile(void)
{
	int				DureeRetentionEnJours		= PVHOOK.dwRetention;
	double			DureeRetentionEnSecondes	= 0;
	double			AgeFichierEnSecondes		= 0;
	HANDLE			fh							= NULL;
	WIN32_FIND_DATA	fd							= { 0 };
	BOOL			Found						= TRUE;
	BOOL			StopScan					= FALSE;
	struct tm		tmFile						= { 0 };
	time_t			ltFile;
	time_t			ltNow;
	char			tm_year[4]					= { 0 };
	char			tm_mon[2]					= { 0 };
	char			tm_mday[2]					= { 0 };
	char			tm_hour[2]					= { 0 };
	unsigned char	PathName[_MAX_PATH]			= { 0 };	// Path where to write the file
	PvhookError		Err							= Pvhook_Ok;

	DureeRetentionEnSecondes = DureeRetentionEnJours * 60 * 60 * 24;

	Pvhook_GetPath(&PathName[0]);
	strcat_s(PathName, sizeof(PathName), "\\\\");
	_chdir(PathName);
	printf("Goto %s\n", PathName);

	fh = FindFirstFile((LPCTSTR)"*.00hor", &fd);
	if (fh != INVALID_HANDLE_VALUE)
	{
		for (; Found & !StopScan; Found = FindNextFile(fh, &fd))
		{
			printf("found %s\t", fd.cFileName);
			strncpy_s(tm_year, sizeof(tm_year), &((fd.cFileName)[0]), 4);
			strncpy_s(tm_mon, sizeof(tm_mon), &((fd.cFileName)[4]), 2);
			strncpy_s(tm_mday, sizeof(tm_mday), &((fd.cFileName)[6]), 2);
			strncpy_s(tm_hour, sizeof(tm_hour), &((fd.cFileName)[8]), 2);
			tm_year[3] = tm_mon[1] = tm_mday[1] = tm_hour[1] = 0x00;

			time(&ltNow);
			localtime_s(&tmFile, &ltNow);
			tmFile.tm_year = atoi(tm_year) - 1900;
			tmFile.tm_mon = atoi(tm_mon) - 1;
			tmFile.tm_mday = atoi(tm_mday);
			tmFile.tm_hour = atoi(tm_hour);
			tmFile.tm_min = 0;
			tmFile.tm_sec = 0;
			tmFile.tm_isdst = 0;
			ltFile = mktime(&tmFile);	

			time(&ltNow);
			AgeFichierEnSecondes = difftime(ltNow, ltFile);
			if (AgeFichierEnSecondes > DureeRetentionEnSecondes)
			{
				printf("...Delete %s\n", fd.cFileName);
				if (!DeleteFile(fd.cFileName))
				{
					printf("Arg peux po deleter %s\n", fd.cFileName);
					Err = Pvhook_CannotDelete;
				}
			}
			else
			{
				printf("...Keep it\n");
			}
		}
	}

	return (Err);
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
PvhookError Pvhook_GetPath(unsigned char* StrPath)
{
	PvhookError Err = Pvhook_Ok;

	strcpy_s(StrPath, _MAX_PATH, PVHOOK.szFilePath);
	return (Err);
}

/*-------------------------------- END OF FILE ------------------------------*/
