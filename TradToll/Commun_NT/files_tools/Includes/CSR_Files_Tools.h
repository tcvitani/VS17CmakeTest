/*------   (v) 1999 CS-Route  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL MSG
* FICHIER: CSR_Files_Tools.h
* MSGGAGE: C
* --------------------------------------------------------------------
* RESUME: Get file version
* --------------------------------------------------------------------
* DESCRIPTION: Extracts file version from file name
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef CSR_FILE_GET_VERSION_H
#define CSR_FILE_GET_VERSION_H

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <stdio.h>


/*--------------- RESERVED: ---------------*/
#ifdef CSR_FILES_TOOLS_EXPORTS
#   include "public.h"
#else
#   include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/
#define CSR_REG_KEYn_PCS		"PCS communication\\"
#define CSR_REG_KEYn_PCSFiles	"Files\\"
#define CSR_REG_KEYn_LIST	    "List\\"
#define CSR_REG_KEYv_PATH		"path"
#define CSR_REG_KEYv_SUBDIR		"subdir"
#define CSR_REG_KEYv_CURRENT	"current"
#define CSR_REG_KEYv_NEW		"new"
#define CSR_REG_KEYv_VERSION	"current_version"
#define CSR_REG_KEYv_MULTIFILES	"MultiFiles"


#define FILES_TOOLS_SEP_CHAR    ";"

#define FILE_LIST_MAX_PATH		4096

/*--------------- TYPEDEFS: ---------------*/

typedef enum
{
	FILE_OK,
	FILE_ERROR_MISSING,
	FILE_ERROR_REGISTRY,
	FILE_NO_FILE,
	FILE_ERROR_EXTRACTING_VERSION,
	FILE_BLANK_DATA,
	FILE_ERROR_DELETING_NEW,
	FILE_ERROR_DELETING_CURRENT,
	FILE_EQUAL,
	FILE_DIFFERENT,
    FILE_BAD_PARAMETER,
    FILE_TYPE_EXTRACT_ERROR,
    FILE_COPY_ERROR,
	FILE_ERROR_LIST_OVERLOADED,
}
enum_files_return;


/*--------------- FUNCTIONS: ---------------*/


// FILE VERSION EXTRACTION FUNCTION

EXPORT enum_files_return WINAPI FileGetVersion(char *pFileId, char *pVersion, size_t sVersionSize);

EXPORT enum_files_return WINAPI FileGetRootPath ( char *pFilesRootPath);

EXPORT enum_files_return WINAPI FileGetSpecificPath ( char *pFileId, char *pFilePath);

EXPORT enum_files_return WINAPI FileSetCurrent ( char *pFileId, char *pFileName);

EXPORT enum_files_return WINAPI FileGetCurrent(char *pFileId, char *pFileName, size_t sFileNameSize);

EXPORT enum_files_return WINAPI FileSetNew ( char *pFileId, char *pFileName);

EXPORT enum_files_return WINAPI FileGetNew(char *pFileId, char *pFileName, size_t sFileNameSize);

//EXPORT enum_files_return WINAPI FileUpdateCurrent ( char *pFileId, char *pNewCurrentPath, char *pNewName);

//EXPORT enum_files_return WINAPI FileUpdateNew ( char *pFileId, char *pNewNewPath, char *pNewName);

EXPORT enum_files_return WINAPI FileUpdateCurrentWithNew ( char *pFileId, BOOL delete_current_file);

EXPORT enum_files_return WINAPI FileCompareCurrentWithNew ( char *pFileId);

EXPORT enum_files_return WINAPI FileDeleteCurrent ( char *pFileId);

EXPORT enum_files_return WINAPI FileDeleteNew ( char *pFileId);

EXPORT enum_files_return WINAPI FileExtractVersion(char *p_pathfile, char *p_version, size_t sVersionSize);

EXPORT int FileCompareDates(SYSTEMTIME *p_t1, SYSTEMTIME *p_t2);

EXPORT enum_files_return WINAPI FileGetFullNameCurrent ( char *pFileId, char *pFilePath);

EXPORT enum_files_return WINAPI FileGetFullNameNew ( char *pFileId, char *pFilePath);

EXPORT enum_files_return WINAPI FileDeleteNotUsedFiles ( char *pFileId);

/* Retourne le nom du fichier courant correspondant au sous-type */
EXPORT enum_files_return WINAPI FileGetCurrentByType(IN char *pFileId, IN char *pSubType, OUT char *pFileName, size_t sFileNameSize);

/* Retourne les chemins et nom complet du fichier courant correspondant au sous-type */
EXPORT enum_files_return WINAPI FileGetFullNameCurrentByType ( IN char *pFileId, IN char *pSubType, OUT char *pFullName);

EXPORT enum_files_return WINAPI FileImport(IN char *pFileId, IN char *pFullName, IN BOOL remove_src, OUT char *pSubType, size_t sSubTypeSize);

EXPORT enum_files_return FileGetList(IN char *pFileId, OUT char *pFilesList, size_t sFilesListSize);

EXPORT enum_files_return WINAPI FileGetMultiFilesBoolean(IN char *pFileId, OUT BOOL *pMultiFiles);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* CSR_FILES_TOOLS_H */
