/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: GETFILEVERSION
* FICHIER: CSR_Files_Tools.c
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <CSR_Files_Tools.h>
#include <string.h>
/* Outils_C */
#include <str.h>
/* Commun */
#include<csrlc32.h>
/* Registry */
#include <reg.h>
/* Files managing*/
#include <fic.h>



/*--------------- RESERVED: ---------------*/

#include "memclass.h"
 
/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- GLOBALS: ----------------*/

/*--------------- FUNCTIONS: ---------------*/



PROTECTED enum_files_return WINAPI FileWriteToList ( char *pFileId, char *pFileName);
PROTECTED enum_files_return WINAPI FileExtractSubkey(char *p_FileName, char *p_Subkey, size_t sSubkeySize);
PRIVATE enum_files_return WINAPI FileUpdateCurrentWithNew_Multi( char *pFileId, BOOL delete_current_file);

/*--------------- CODE: ---------------*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetRootPath (OUT char *pFilesRootPath)
* PARAMETRES:	*pFilesRootPath
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY)
*				
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Returns common path for data files
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetRootPath (char *pFilesRootPath)
{
    DWORD dwLen				= 0;
	char pcKey[MAX_PATH]	= {0};
	char reading[MAX_PATH]	= {0};
	
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles);
	
	dwLen = sizeof(reading);
	
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_PATH, reading, &dwLen ) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	if (STR_strlen( MAX_PATH, reading) == 0)
		return (FILE_ERROR_REGISTRY);
	
	STR_strcpy(MAX_PATH, pFilesRootPath, reading);
	return(FILE_OK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetSpecificPath (IN char *pFileId, OUT char *pFilePath)
* PARAMETRES:	*pFileId	- string identifier of the file (<<TFT>>, <<TCI>>, ...)
*				*pFilePath
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA) 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Returns specific path for data file
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetSpecificPath ( char *pFileId, char *pFilePath)
{
    DWORD dwLen				= 0;
	char pcKey[MAX_PATH]	= {0};
	char RootPath[MAX_PATH] = {0};
	char Subdir[MAX_PATH]	= {0};
   	enum_files_return	file_get_root_path_return;

	file_get_root_path_return = FileGetRootPath(RootPath);
	if (file_get_root_path_return == FILE_OK)
		STR_strcpy(MAX_PATH, pFilePath, RootPath);
	else
		return (file_get_root_path_return);
		
	
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	dwLen = sizeof(Subdir);
	
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_SUBDIR, Subdir, &dwLen ) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	if (STR_strlen( MAX_PATH, Subdir) == 0)
		return (FILE_BLANK_DATA);

	FIC_makepath(pFilePath, RootPath, Subdir, NULL, NULL);

	if (_access(pFilePath, 0) == -1)
		FIC_CreatePath(pFilePath);

	return(FILE_OK);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetVersion ( char *pFileId, char *pVersion)
* PARAMETRES:	*pFileId	- string identifier of the file (<<TFT>>, <<TCI>>, ...)
*				*pVersion	- extracted file version
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY,
*				FILE_ERROR_MISSING, FILE_NO_FILE)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Retrieves file version from file name
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC enum_files_return WINAPI FileGetVersion(char *pFileId, char *pVersion, size_t sVersionSize)
{
	char FileName[FILE_LIST_MAX_PATH] = {0};
	enum_files_return	file_get_current_return;
	enum_files_return	file_extract_version;

    BOOL multi = FALSE;
	
    FileGetMultiFilesBoolean (pFileId, &multi);

    if (multi)
    {
		strcpy_s(pVersion, sVersionSize, "MultiFiles");
        return FILE_OK;
    }

	file_get_current_return = FileGetCurrent(pFileId, FileName, sizeof(FileName));
	if (file_get_current_return != FILE_OK)
		return(file_get_current_return);

	file_extract_version = FileExtractVersion(FileName, pVersion, sVersionSize);
	return(file_extract_version);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileSetCurrent ( IN char *pFileId, IN char *pFileName)
* PARAMETRES:	*pFileId	- string identifier of the file (<<TFT>>, <<TCI>>, ...)
*				*pFileName	-	file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_BLANK_DATA, FILE_ERROR_REGISTRY,FILE_ERROR_MISSING)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Modifies the registry (<<current>> field), and automaticly
*		updates the field <<current version>>
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileSetCurrent ( char *pFileId, char *pFileName)
{
	char pcKey[MAX_PATH]			= {0};
	char writing_current[MAX_PATH]	= {0};
	char SpecificPath[MAX_PATH]		= {0};
	char CurrentName[MAX_PATH]		= {0};
	enum_files_return	file_get_specific_path_return;
	
	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);


	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	STR_strcpy(MAX_PATH, writing_current, pFileName);


	if (_stricmp( writing_current, "\0")!=0)
	{
		FIC_makepath(CurrentName, SpecificPath, NULL, writing_current, NULL);
		// Check if the current file exists on specified path
		if (_access(CurrentName, 0) == -1)
			return(FILE_ERROR_MISSING);
	}

	if ( REG_Ecrire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_CURRENT, writing_current) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);

	
	return(FILE_OK);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetCurrent (IN char *pFileId, OUT char *pFileName)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* 				*pFileName	-	file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Reads the registry to return the <<current>> registry field
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetCurrent(char *pFileId, char *pFileName, size_t sFileNameSize)
{
    DWORD dwLen				= 0;
	char pcKey[MAX_PATH]	= { 0 };
	char reading[MAX_PATH]	= { 0 };
	char Path[MAX_PATH]		= { 0 };
	char fullname[MAX_PATH] = { 0 };
	enum_files_return	file_set_current_return;
    BOOL multi = FALSE;
	
    FileGetMultiFilesBoolean (pFileId, &multi);

    if (multi)
		return FileGetList(pFileId, pFileName, sFileNameSize);

	strcpy_s(pFileName, sFileNameSize, "");

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	dwLen = sizeof(reading);
	
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_CURRENT, reading, &dwLen ) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	if (STR_strlen( MAX_PATH, reading) == 0)
		return (FILE_BLANK_DATA);
	
	// Find path for file
	if (FileGetSpecificPath( pFileId, Path) != FILE_OK)
		return(FILE_ERROR_MISSING);

	FIC_makepath(fullname, Path, NULL, reading, NULL);

	// Check if file exists on specified path
	if (_access(fullname, 0) == -1)
	{
		file_set_current_return = FileSetCurrent( pFileId, "\0");
		if (file_set_current_return != FILE_OK)
			return (file_set_current_return);
	
		return(FILE_ERROR_MISSING);
	}

	STR_strcpy( MAX_PATH, pFileName, reading);
	return(FILE_OK);

}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileSetNew (IN char *pFileId,IN char *pFileName)
* PARAMETRES:	*pFileName	-	file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_ERROR_MISSING) 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Modifies the registry <<new>> field
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileSetNew ( char *pFileId, char *pFileName)
{
	char pcKey[MAX_PATH]		= {0};
	char writing_new[MAX_PATH]	= {0};
	char NewName[MAX_PATH]		= {0};
	char SpecificPath[MAX_PATH] = {0};
	char New[MAX_PATH]			= {0};
	enum_files_return	file_get_specific_path_return;
    BOOL multi					= FALSE;	
	
    FileGetMultiFilesBoolean (pFileId, &multi);

	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);
	
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	STR_strcpy(MAX_PATH, writing_new, pFileName);
	FIC_makepath(NewName, SpecificPath, NULL, writing_new, NULL);

	// check if file exists
	if (_stricmp(writing_new, "\0") != 0 && _access(NewName, 0) == -1 && !multi)
		return(FILE_ERROR_MISSING);

    if (multi)
    {
		if (_stricmp(writing_new, "\0") != 0)
		{
			FileGetNew(pFileId, New, sizeof(New));
			if (strlen(pFileName) != 0)
			{
				strcat_s(New, sizeof(New), pFileName);
			}
			strcpy_s(writing_new, sizeof(writing_new), New);
		}
    }
	
	if ( REG_Ecrire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_NEW, writing_new) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	return(FILE_OK);

}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetNew (IN char *pFileId, OUT char *pFileName)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
*				*pFileName	-	file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Reads the registry to return the <<new>> registry field
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetNew(char *pFileId, char *pFileName, size_t sFileNameSize)
{
    DWORD	dwLen				= 0;
	char	pcKey[MAX_PATH]		= {0};
	char	reading[MAX_PATH]	= {0};
	char	Path[MAX_PATH]		= {0};
	char	fullname[MAX_PATH]	= {0};
	enum_files_return	file_set_new_return = 0;
	BOOL	multi				= FALSE;
	
    FileGetMultiFilesBoolean (pFileId, &multi);

	strcpy_s(pFileName, sFileNameSize, "");

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	dwLen = sizeof(reading);
	
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_NEW, reading, &dwLen ) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	if (STR_strlen( MAX_PATH, reading) == 0)
		return (FILE_BLANK_DATA);
	
    if (multi)
    {
		//Check for file size to avoid access violation on strcat_s
		if (dwLen >= sFileNameSize)
			return(FILE_ERROR_REGISTRY);

	    STR_strcpy( MAX_PATH, pFileName, reading);
		//append separator to end
		strcat_s(pFileName, sFileNameSize, FILES_TOOLS_SEP_CHAR);

	    return(FILE_OK);
    }

	// Find path for file
	if (FileGetSpecificPath( pFileId, Path) != FILE_OK)
		return(FILE_ERROR_MISSING);

	FIC_makepath(fullname, Path, NULL, reading, NULL);

	// Check if file exists on specified path
	if (_access(fullname, 0) == -1)
	{
		// File doesn't exist, delete name in registry
		file_set_new_return = FileSetNew( pFileId, "\0");
		if (file_set_new_return != FILE_OK)
			return (file_set_new_return);
	
		return(FILE_ERROR_MISSING);
	}


	STR_strcpy( MAX_PATH, pFileName, reading);

	return(FILE_OK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileDeleteCurrent ( char *pFileId)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* RETOUR:		enum_files_return value (FILE_OK, 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Deletes the current file from the disk and automaticly updates the registry.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileDeleteCurrent ( char *pFileId)
{
	char FullName[MAX_PATH]={0};
	enum_files_return	file_set_current_return;
	enum_files_return	file_get_full_name_return;
	enum_files_return	file_get_current_return;
	enum_files_return	file_get_specific_path_return;
	BOOL multi = FALSE;
	char pcKey[MAX_PATH + 1]	= {0};
	char FileCurrent[FILE_LIST_MAX_PATH] = { 0 };
	char SpecificPath[MAX_PATH] = { 0 };
	char file_path[MAX_PATH]	 = { 0 };
	intptr_t hfind;
	struct _finddata_t ffblk;
	long done;
	char TempStr[FILE_LIST_MAX_PATH + 1] = { 0 };
	char *ptr = NULL, *ptr_old = NULL;

    FileGetMultiFilesBoolean (pFileId, &multi);

    if (multi)
    {
		file_get_current_return = FileGetCurrent(pFileId, FileCurrent, sizeof(FileCurrent));
		file_get_specific_path_return = FileGetSpecificPath(pFileId, SpecificPath);

		if (file_get_current_return != FILE_OK && file_get_specific_path_return != FILE_OK)
		{
			return (FILE_ERROR_DELETING_CURRENT);
		}

		// find files in specific directory
		FIC_makepath(file_path, SpecificPath, NULL, "*", "*");

		if ((hfind = FIC_FindFirst(file_path, &ffblk)) >= 0)
		{
			done = 0;
			while (done >= 0)
			{
				// If object found is not a directory and is not read only
				if (!(ffblk.attrib & (_A_SUBDIR | _A_RDONLY)))
				{		
					ptr_old = FileCurrent;

					while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
					{
						memcpy(TempStr, ptr_old, ptr - ptr_old);
						TempStr[(int)(ptr - ptr_old)] = '\0';
						ptr_old = ptr;
						ptr_old++;

						if (_stricmp(ffblk.name, TempStr) == 0) //Same
						{
							FIC_makepath(file_path, SpecificPath, NULL, ffblk.name, NULL);
							if (FIC_remove(file_path) != 0)
								return (FILE_ERROR_DELETING_CURRENT);
						}
					}
	
				}
				done = FIC_FindNext(hfind, &ffblk);
			}
			FIC_FindClose(hfind);
		}

		sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s\\%s", CSR_REG_KEYn_CSRBASE,
            CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS,
            CSR_REG_KEYn_PCSFiles, pFileId, CSR_REG_KEYn_LIST);

        REG_Efface_Cle (CSR_REG_KEYi_ROOT, pcKey, NULL);

	    return FILE_OK;
    }
	

	file_get_full_name_return = FileGetFullNameCurrent ( pFileId, FullName);
	if (file_get_full_name_return == FILE_ERROR_MISSING)
		return (FILE_OK);

	file_set_current_return = FileSetCurrent( pFileId, "\0");
	if (file_set_current_return != FILE_OK)
		return (file_set_current_return);

	if( FIC_remove(FullName) != 0)
		return (FILE_ERROR_DELETING_NEW);


	return (FILE_OK);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileDeleteNew ( char *pFileId)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, 
*				FILE_ERROR_DELETING_NEW, FILE_BLANK_DATA, FILE_ERROR_MISSING)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Deletes the new file from the disk and automaticly updates the registry.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileDeleteNew ( char *pFileId)
{
	char				FullName[MAX_PATH] = {0};
	enum_files_return	file_set_new_return;
	enum_files_return	file_get_full_name_return;
	enum_files_return	file_get_new_return;
	enum_files_return	file_get_specific_path_return;
	char				pcKey[MAX_PATH + 1] = {0};
	char				FileNew[MAX_PATH] = { 0 };
	char				SpecificPath[MAX_PATH] = { 0 };
	char				file_path[MAX_PATH] = { 0 };
    BOOL                multi = FALSE;
	intptr_t			hfind;
	struct _finddata_t	ffblk;
	long				done;
	char				TempStr[MAX_PATH + 1] = { 0 };
	char				*ptr = NULL, *ptr_old = NULL;

    FileGetMultiFilesBoolean (pFileId, &multi);

    if (multi)
    {
		file_get_new_return = FileGetNew(pFileId, FileNew, sizeof(FileNew));
		file_get_specific_path_return = FileGetSpecificPath(pFileId, SpecificPath);

		if (file_get_new_return != FILE_OK && file_get_specific_path_return != FILE_OK)
		{
			return (FILE_ERROR_DELETING_NEW);
		}

		// find files in specific directory
		FIC_makepath(file_path, SpecificPath, NULL, "*", "*");

		if ((hfind = FIC_FindFirst(file_path, &ffblk)) >= 0)
		{
			done = 0;
			while (done >= 0)
			{
				// If object found is not a directory and is not read only
				if (!(ffblk.attrib & (_A_SUBDIR | _A_RDONLY)))
				{
					ptr_old = FileNew;

					while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
					{
						memcpy(TempStr, ptr_old, ptr - ptr_old);
						TempStr[(int)(ptr - ptr_old)] = '\0';
						ptr_old = ptr;
						ptr_old++;

						if (_stricmp(ffblk.name, TempStr) == 0) //Same
						{
							FIC_makepath(file_path, SpecificPath, NULL, ffblk.name, NULL);
							if (FIC_remove(file_path) != 0)
								return (FILE_ERROR_DELETING_NEW);
						}
					}

				}
				done = FIC_FindNext(hfind, &ffblk);
			}
			FIC_FindClose(hfind);
		}


		sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);

	    if ( REG_Ecrire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_NEW, "") != ERROR_SUCCESS )
		    return (FILE_ERROR_REGISTRY);
	
	    return FILE_OK;
    }
	

	

	file_get_full_name_return = FileGetFullNameNew ( pFileId, FullName);
	if (file_get_full_name_return == FILE_ERROR_MISSING)
		return (FILE_OK);

	file_set_new_return = FileSetNew( pFileId, "\0");
	if (file_set_new_return != FILE_OK)
		return(file_set_new_return);
	
	if( FIC_remove(FullName) != 0)
		return(FILE_ERROR_DELETING_NEW);

	return(FILE_OK);
	
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileUpdateCurrentWithNew ( char *pFileId, BOOL delete_current_file)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
*				delete_current_file	-	is TRUE if the current file must be deleted (FALSE if not)
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA, FILE_ERROR_MISSING)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Updates the registry (current and current_version) through the function FileSetCurrent
*		and tests if the new file exists before updating the registry
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileUpdateCurrentWithNew ( char *pFileId, BOOL delete_current_file)
{

	char FileNewName[MAX_PATH]		= { 0 };
	char FullNameNew[MAX_PATH]		= { 0 };
	char FullNameCurrent[MAX_PATH]	= { 0 };
	enum_files_return	file_get_new_return;
	enum_files_return	file_get_full_name_new_return;
	enum_files_return	file_get_full_name_current_return;
	enum_files_return	file_set_current_return;
	BOOL multi = FALSE;

    FileGetMultiFilesBoolean (pFileId, &multi);

    if (multi)
        return FileUpdateCurrentWithNew_Multi (pFileId, delete_current_file);

	// get full filename for current file
	file_get_full_name_current_return =	FileGetFullNameCurrent ( pFileId, FullNameCurrent);
//	if (file_get_full_name_current_return != FILE_OK)
//		return(file_get_full_name_current_return);

	// get full filename for new file
	file_get_full_name_new_return =	FileGetFullNameNew ( pFileId, FullNameNew);
	if (file_get_full_name_new_return != FILE_OK)
		return(file_get_full_name_new_return);
	
	// Get "new" filename from registry
	file_get_new_return = FileGetNew(pFileId, FileNewName, sizeof(FileNewName));
	if (file_get_new_return != FILE_OK)
		return(file_get_new_return);

	
	// Check if the new file exists on specified path
	if (_access(FullNameNew, 0) == -1)
		return(FILE_ERROR_MISSING);
	
	file_set_current_return = FileSetCurrent( pFileId, FileNewName);
	if (file_set_current_return != FILE_OK)
		return(file_set_current_return);

	// Check if the old current file exists on specified path and delete it if
	// delete_current_file = TRUE
	if (file_get_full_name_current_return == FILE_OK)
	{
		if ((_access(FullNameCurrent, 0) != -1) && (delete_current_file==TRUE))
			FIC_remove(FullNameCurrent);
	}

	FileSetNew (pFileId, ""); // Clear 'New' entry in the registry
	
	return(FILE_OK);	
	
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileCompareCurrentWithNew ( char *pFileId)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* RETOUR:		enum_files_return value (FILE_OK, 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Compares the <<current>> and <<new>> entries in the registry.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileCompareCurrentWithNew ( char *pFileId)
{
	char				current_reg[FILE_LIST_MAX_PATH] = { 0 };
	char				new_reg[MAX_PATH]		= { 0 };
	char				FileId[MAX_PATH]		= { 0 };
	enum_files_return	file_get_new_return;
	enum_files_return	file_get_current_return;
	BOOL				multi = FALSE;
	char				*ptr = NULL, *ptr_old = NULL;
	char				*ptr_2 = NULL, *ptr_old_2 = NULL;
	char				TempStr[MAX_PATH + 1]	= { 0 };
	char				TempStr2[FILE_LIST_MAX_PATH + 1] = { 0 };
	BOOL				bMultiDifferent			= FALSE;
	BOOL				bCurrentEqual			= FALSE;

	FileGetMultiFilesBoolean(pFileId, &multi);

	STR_strcpy(MAX_PATH, FileId, pFileId);

	file_get_new_return = FileGetNew(pFileId, new_reg, sizeof(new_reg));
	if (file_get_new_return!=FILE_OK)
		return(file_get_new_return);
	
	file_get_current_return = FileGetCurrent(pFileId, current_reg, sizeof(current_reg));
	if (file_get_current_return!=FILE_OK)
		return(file_get_current_return);
	
	if (multi)
	{
		ptr_old = new_reg;

		while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
		{
			memcpy(TempStr, ptr_old, ptr - ptr_old);
			TempStr[(int)(ptr - ptr_old)] = '\0';
			ptr_old = ptr;
			ptr_old++;

			ptr_old_2 = current_reg;

			bCurrentEqual = FALSE;

			while ((ptr_2 = strstr(ptr_old_2, FILES_TOOLS_SEP_CHAR)) != NULL)
			{
				
				memcpy(TempStr2, ptr_old_2, ptr_2 - ptr_old_2);
				TempStr2[(int)(ptr_2 - ptr_old_2)] = '\0';
				ptr_old_2 = ptr_2;
				ptr_old_2++;

				if (_stricmp(TempStr2, TempStr) == 0)
				{
					bCurrentEqual = TRUE;
				}

			}

			//If any of new is not equal, then whole function is different
			if (!bCurrentEqual)
				bMultiDifferent = TRUE;
			
		}

		if (bMultiDifferent)
			return (FILE_DIFFERENT);
		else
			return (FILE_EQUAL);
	}
	else
	{
		if (_stricmp(new_reg, current_reg) == 0)
			return (FILE_EQUAL);
		else
			return (FILE_DIFFERENT);
	}

} 

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileExtractVersion ( char *p_pathfile, char *p_version)
* PARAMETRES:	*p_pathfile	- file name and path
*				*p_version	- extracted file version
* RETOUR:		enum_files_return value (FILE_OK, 
*				FILE_ERROR_EXTRACTING_VERSION)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Retrieves file version from file name
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC enum_files_return WINAPI FileExtractVersion(char *p_pathfile, char *p_version, size_t sVersionSize)
{
	#define FILE_VERS_SEP		"."
	#define FILE_VERS_NB_SEP	3
	#define FILE_VERS_OFFSET	1

	char *p_path, *p_file;
	int count;

	p_path = p_pathfile;
	if (p_path == NULL)
		return(FILE_ERROR_EXTRACTING_VERSION);

	// Extract the file name from an optional full path
	p_file = p_path;
	while ((p_path = strstr (p_path, "\\")) != NULL)
	{
		p_path ++;
		p_file = p_path;
	}

	if (p_file == NULL)
		return(FILE_ERROR_EXTRACTING_VERSION);

	count = 0;
	while (((p_file = strstr (p_file, FILE_VERS_SEP)) != NULL) && (count < (FILE_VERS_NB_SEP-1)))
	{
		p_file ++;
		count ++;
	}
	
	// version number is placed after FILE_VERS_NB_SEP '.' character in filename
	if (count==(FILE_VERS_NB_SEP - 1))
	{
		if (p_file != NULL)
		{
			p_file ++;
			p_file += FILE_VERS_OFFSET;
			strcpy_s(p_version, sVersionSize, p_file);
			if ((p_file = strstr (p_version, FILE_VERS_SEP)) != NULL)
				*p_file = '\x0';
		}
		else
		{
			strcpy_s(p_version, sVersionSize, "");
			return(FILE_ERROR_EXTRACTING_VERSION);
		}
	}
	else
		return(FILE_ERROR_EXTRACTING_VERSION);

	return(FILE_OK);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC int FileCompareDates ( SYSTEMTIME *p_t1, SYSTEMTIME *p_t2 )
 * PARAMETERS: SYSTEMTIME *p_t1 : 
 *             SYSTEMTIME *p_t2 : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PUBLIC int FileCompareDates(SYSTEMTIME *p_t1, SYSTEMTIME *p_t2)
{
   int temp;

   temp = p_t2->wYear - p_t1->wYear;
   if (temp != 0)
	   return temp;

   temp = p_t2->wMonth - p_t1->wMonth;
   if (temp != 0)
	   return temp;

   temp = p_t2->wDay - p_t1->wDay;
   if (temp != 0)
	   return temp;

   temp = p_t2->wHour - p_t1->wHour;
   if (temp != 0)
	   return temp;

   temp = p_t2->wMinute - p_t1->wMinute;
   return temp;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetFullNameCurrent ( char *pFileId, char *ppFullName)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* 				*pFullName	-	full file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Returns full name (path+filename of current file)
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetFullNameCurrent ( char *pFileId, char *pFullName)
{
	char NameCurrent[MAX_PATH]	= { 0 };
	char FileName[FILE_LIST_MAX_PATH] = { 0 };
	char SpecificPath[MAX_PATH]	= { 0 };
	enum_files_return	file_get_specific_path_return;
	enum_files_return	file_get_current_return;


	// Get "current" filename from registry
	file_get_current_return = FileGetCurrent(pFileId, FileName, sizeof(FileName));
	if (file_get_current_return != FILE_OK)
		return(file_get_current_return);

	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);

	// Create full file name
	FIC_makepath(NameCurrent, SpecificPath, FileName, NULL, NULL);
	
	STR_strcpy(MAX_PATH, pFullName, NameCurrent);

	return(FILE_OK);
	
}

PUBLIC enum_files_return WINAPI FileGetFullNameCurrentByType ( IN char *pFileId, IN char *pSubType, IN char *pFullName)
{
	char NameCurrent[MAX_PATH]	= { 0 };
	char FileName[FILE_LIST_MAX_PATH] = { 0 };
	char SpecificPath[MAX_PATH] = { 0 };
	enum_files_return	file_get_specific_path_return;
	enum_files_return	file_get_current_return;


	// Get "current" filename from registry
	file_get_current_return = FileGetCurrentByType(pFileId, pSubType, FileName, sizeof(FileName));
	if (file_get_current_return != FILE_OK)
		return(file_get_current_return);

	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);

	// Create full file name
	FIC_makepath(NameCurrent, SpecificPath, FileName, NULL, NULL);
	
	STR_strcpy(MAX_PATH, pFullName, NameCurrent);

	return(FILE_OK);
	
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileGetFullNameNew ( char *pFileId, char *pFullName)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* 				*pFullName	-	file name
* RETOUR:		enum_files_return value (FILE_OK, FILE_ERROR_REGISTRY, FILE_BLANK_DATA)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Returns full name (path+filename) of new file
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileGetFullNameNew ( char *pFileId, char *pFullName)
{
	char NameNew[MAX_PATH]		= { 0 };
	char FileName[MAX_PATH]		= { 0 };
	char SpecificPath[MAX_PATH] = { 0 };
	enum_files_return	file_get_specific_path_return;
	enum_files_return	file_get_new_return;


	// Get "new" filename from registry
	file_get_new_return = FileGetNew(pFileId, FileName, sizeof(FileName));
	if (file_get_new_return != FILE_OK)
		return(file_get_new_return);

	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);

	// Create full file name
	FIC_makepath(NameNew, SpecificPath, FileName, NULL, NULL);
	
	STR_strcpy(MAX_PATH, pFullName, NameNew);

	return(FILE_OK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_files_return WINAPI FileDeleteNotUsedFiles ( char *pFileId)
* PARAMETRES:	*pFileId	-	string identifier of the file (<<TFT>>, <<TCI>>, ...)
* RETOUR:		enum_files_return value (FILE_OK, 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: FilesTools Function
* ROLE: Deletes the new file from the disk and automaticly updates the registry.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_files_return WINAPI FileDeleteNotUsedFiles ( char *pFileId)
{
	char   file_path[MAX_PATH]	= { 0 };
	struct _finddata_t ffblk;
	long done;
	intptr_t hfind;
	char FileNew[MAX_PATH]		= { 0 };
	char FileCurrent[FILE_LIST_MAX_PATH] = { 0 };
	char SpecificPath[MAX_PATH] = { 0 };
    boolean report = FALSE;
	BOOL multi = FALSE;
	char TempStr[FILE_LIST_MAX_PATH + 1] = { 0 };
	char *ptr = NULL, *ptr_old = NULL;
	BOOL bCurrentFileSame = FALSE;

	enum_files_return	file_get_new_return;
	enum_files_return	file_get_specific_path_return;
	enum_files_return	file_get_current_return;
	
	// Find full path for specific file
	file_get_specific_path_return = FileGetSpecificPath( pFileId, SpecificPath);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);

	// Get "new" filename from registry
	file_get_new_return = FileGetNew(pFileId, FileNew, sizeof(FileNew));

	// Get "current" filename from registry
	file_get_current_return = FileGetCurrent(pFileId, FileCurrent, sizeof(FileCurrent));

	FileGetMultiFilesBoolean(pFileId, &multi);

	// find files in specific directory
	FIC_makepath(file_path, SpecificPath, NULL,"*","*");
	if ((hfind = FIC_FindFirst(file_path,&ffblk)) >= 0)
	{
	   done = 0;
	   while( done >= 0 )
	   {
		   // If object found is not a directory and is not read only
		   if (!(ffblk.attrib & (_A_SUBDIR|_A_RDONLY)))
		   {
			   if (multi)
			   {
				   ptr_old = FileCurrent;

				   while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
				   {
					   memcpy(TempStr, ptr_old, ptr - ptr_old);
					   TempStr[(int)(ptr - ptr_old)] = '\0';
					   ptr_old = ptr;
					   ptr_old++;

					   if (_stricmp(ffblk.name, TempStr) == 0) //Same
					   {
						   bCurrentFileSame = TRUE;
						   break;
					   }
				   }

				   if (bCurrentFileSame == FALSE)
				   {
					   if (_stricmp(ffblk.name, FileNew) != 0)
					   {
						   FIC_makepath(file_path, SpecificPath, NULL, ffblk.name, NULL);
						   if (FIC_remove(file_path) == 0)
							   report = TRUE;
					   }
				   }

			   }
			   else
			   {
				   if ((_stricmp(ffblk.name, FileNew) != 0) && (_stricmp(ffblk.name, FileCurrent) != 0))
				   {
					   FIC_makepath(file_path, SpecificPath, NULL, ffblk.name, NULL);
					   if (FIC_remove(file_path) == 0)
						   report = TRUE;
				   }
			   }
		   }
			done = FIC_FindNext(hfind, &ffblk);
	   }
	   FIC_FindClose (hfind);
	}
	
	return(FILE_OK);
	
}

PUBLIC enum_files_return WINAPI FileGetMultiFilesBoolean(char *pFileId, BOOL *pMultiFiles)
{
    DWORD i = 0;
	char pcKey[MAX_PATH] = { 0 };
	
    *pMultiFiles = FALSE;

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId);
	
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_MULTIFILES, &i) != ERROR_SUCCESS )
		return (FILE_OK); // Champ non obligatoire
	
	if (i == 1)
        *pMultiFiles = TRUE;

	return(FILE_OK);
}

PUBLIC enum_files_return FileGetList(IN char *pFileId, OUT char *pFilesList, size_t sFilesListSize)
{
	char				pvValeur[MAX_PATH + 1]	= { 0 };
	DWORD				size;
	int					i;
	char				pcKey[MAX_PATH + 1]		= { 0 };
	DWORD				val_type, name_size;
	char				pName[MAX_PATH + 1]		= { 0 };
	char				Path[MAX_PATH]			= { 0 };
	char				fullname[MAX_PATH]		= { 0 };
	BOOL				bFileIsMissing = FALSE;
	
    if ((pFileId == NULL) || (pFilesList == NULL))
	    return FILE_BAD_PARAMETER;

	strcpy_s(pFilesList, sFilesListSize, "");
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s\\%s", CSR_REG_KEYn_CSRBASE,
        CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS,
        CSR_REG_KEYn_PCSFiles, pFileId, CSR_REG_KEYn_LIST);


	// Find path for file
	if (FileGetSpecificPath(pFileId, Path) != FILE_OK)
		return(FILE_ERROR_MISSING);

    size = sizeof(pvValeur);
    name_size = sizeof(pName);
    i = 0;
	while (REG_Enum_Valeurs(CSR_REG_KEYi_ROOT,pcKey, i, pName, &name_size, &val_type, pvValeur, &size ) == ERROR_SUCCESS)
	{
        if (val_type == REG_SZ)
        {
			if (size >= sFilesListSize)
				return FILE_ERROR_LIST_OVERLOADED;

			strncat_s(pFilesList, sFilesListSize, pvValeur, _TRUNCATE);
			if ( (size>0))
				strncat_s(pFilesList, sFilesListSize, FILES_TOOLS_SEP_CHAR, _TRUNCATE);
        }

        size = sizeof(pvValeur);
        name_size = sizeof(pName);
		i++;

		FIC_makepath(fullname, Path, NULL, pvValeur, NULL);

		// Check if file exists on specified path
		if (_access(fullname, 0) == -1)
		{
			bFileIsMissing = TRUE;
		}

	}

    if (strlen (pFilesList) == 0)
        return FILE_NO_FILE;
	else if (bFileIsMissing)
		return(FILE_ERROR_MISSING);
	else
        return FILE_OK;
}

PRIVATE enum_files_return WINAPI FileUpdateCurrentWithNew_Multi( char *pFileId, BOOL delete_current_file)
{
	char				NewFiles[MAX_PATH + 1]	= { 0 };
	char				Path[MAX_PATH + 1]		= { 0 };
	char				TempStr[MAX_PATH + 1]	= { 0 };
	char				RegFile[FILE_LIST_MAX_PATH + 1] = { 0 };
	char				FileType[20 + 1]		= { 0 };
	char				*ptr = NULL, *ptr_old = NULL;
	struct _finddata_t	file_info;
	intptr_t			hFind;
    enum_files_return   ret,file_get_specific_path_return;
	

	if ((ret = FileGetNew(pFileId, NewFiles, sizeof(NewFiles)) != FILE_OK))
		return ret; //retrieve NewFiles value from registry

	file_get_specific_path_return = FileGetSpecificPath( pFileId, Path);
	if (file_get_specific_path_return != FILE_OK)
		return(file_get_specific_path_return);

	// write all values to its appropriate places so application can read 
	// from these files
	ptr_old = NewFiles;

	while((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
	{
		memcpy(TempStr, ptr_old, ptr - ptr_old);
		TempStr[(int)(ptr - ptr_old)] = '\0';
		ptr_old = ptr;
		ptr_old++;
		
		FileWriteToList(pFileId, TempStr);
	}

	strcpy_s(TempStr,sizeof(TempStr), ptr_old);
	

	FileWriteToList(pFileId, TempStr);

	// reset NewFiles value in registry
	FileSetNew(pFileId, "");

    if (delete_current_file)
    {
	    FIC_makepath(TempStr, Path, NULL, "*.*", NULL);
	    // deleting files that are not referenced in registry
	    if ((hFind = FIC_FindFirst(TempStr, &file_info)) >= 0)
	    {
            do
            {
				FileExtractSubkey(file_info.name, FileType, sizeof(FileType));
				FileGetCurrentByType(pFileId, FileType, RegFile, sizeof(RegFile));
		        if (_stricmp(file_info.name, RegFile) != 0)
		        {
			        FIC_makepath(TempStr, Path, NULL, file_info.name, NULL);
					// Check if the file exists on specified path
					if (_access(TempStr, 0) != -1)
					{
						FIC_unlink(TempStr);
					}
						   
		        }
            }
		    while(FIC_FindNext(hFind, &file_info) == 0);

		    FIC_FindClose(hFind);
	    }
    }

    return FILE_OK;
}

PRIVATE enum_files_return WINAPI FileWriteToList ( char *pFileId, char *pFileName)
{
	char pcKey[MAX_PATH]	= { 0 };
	char subkey[MAX_PATH]	= { 0 };
		
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s\\%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE,
        CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles,
        pFileId, CSR_REG_KEYn_LIST);

	// Extract the subkey value from filename
	if (FileExtractSubkey(pFileName, subkey, sizeof(subkey)) != FILE_OK)
		return(FILE_TYPE_EXTRACT_ERROR);

	if ( REG_Ecrire_Chaine( CSR_REG_KEYi_ROOT, pcKey, subkey, pFileName) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	return(FILE_OK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED enum_files_return WINAPI TftExtractSubkey (char *p_TftFileName, char *p_TftSubkey)
* PARAMETRES:	*p_TftFileName - file name 
*				*p_TftSubkey - extracted subkey for tft file
* RETOUR:		enum_files_return value 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: CsrTft Function
* ROLE: Retrieves subkey name from file name
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_files_return WINAPI FileExtractSubkey(char *p_FileName, char *p_Subkey, size_t sSubkeySize)
{
	#define FILE_SUBKEY_SEP		"."
	#define FILE_SUBKEY_NB_SEP	3
	char	*p_filename; 
	char	*p_ptr1;
	char	*p_ptr2;
	int		count;

	p_filename = p_FileName;

	count = 0;
	while (((p_filename = strstr (p_filename, FILE_SUBKEY_SEP)) != NULL) && (count < (FILE_SUBKEY_NB_SEP-2)))
	{
		p_filename ++;
		count ++;
	}
	
	// subkey name is placed between second and third point character ('.') of the filename
	if (p_filename != NULL)
	{
		p_filename ++;
		p_ptr1 = p_filename;
		count=0;
		p_filename = p_FileName;
		while (((p_filename = strstr (p_filename, FILE_SUBKEY_SEP)) != NULL) && (count < FILE_SUBKEY_NB_SEP-1))
		{
			p_filename ++;
			count ++;
		}

		p_ptr2 = p_filename;

		memcpy(p_Subkey, p_ptr1, p_ptr2-p_ptr1);
		p_Subkey[p_ptr2-p_ptr1]='\x0';
	}
	else
	{
		strcpy_s(p_Subkey, sSubkeySize, "");
		return(FILE_TYPE_EXTRACT_ERROR);
	}
	
	return(FILE_OK);
}

PUBLIC enum_files_return WINAPI FileGetCurrentByType(IN char *pFileId, IN char *pSubType, OUT char *pFileName, size_t sFileNameSize)
{
    DWORD dwLen				= 0;
	char pcKey[MAX_PATH]	= { 0 };
	char reading[MAX_PATH]	= { 0 };
	char Path[MAX_PATH]		= { 0 };
	char fullname[MAX_PATH] = { 0 };
	enum_files_return	file_set_current_return;
    BOOL multi = FALSE;
	
    FileGetMultiFilesBoolean (pFileId, &multi);

    if (!multi)
        return FileGetCurrent (pFileId, pFileName, sFileNameSize);

	strcpy_s(pFileName, sFileNameSize, "");

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s\\%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE,
        CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS, CSR_REG_KEYn_PCSFiles, pFileId,
        CSR_REG_KEYn_LIST);
	
	dwLen = sizeof(reading);
	
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, pSubType, reading, &dwLen ) != ERROR_SUCCESS )
		return (FILE_ERROR_REGISTRY);
	
	if (STR_strlen( MAX_PATH, reading) == 0)
		return (FILE_BLANK_DATA);
	
	// Find path for file
	if (FileGetSpecificPath( pFileId, Path) != FILE_OK)
		return(FILE_ERROR_MISSING);

	FIC_makepath(fullname, Path, NULL, reading, NULL);

	// Check if file exists on specified path
	if (_access(fullname, 0) == -1)
	{
		file_set_current_return = FileSetCurrent( pFileId, "\0");
		if (file_set_current_return != FILE_OK)
			return (file_set_current_return);
	
		return(FILE_ERROR_MISSING);
	}

	STR_strcpy( MAX_PATH, pFileName, reading);
	return(FILE_OK);

}

EXPORT enum_files_return WINAPI FileImport(IN char *pFileId, IN char *pFullName, IN BOOL remove_src, OUT char *pSubType, size_t sSubTypeSize)
{
	char			    drive[MAX_PATH]			= { 0 };
	char			    dir[MAX_PATH]			= { 0 };
	char			    file[MAX_PATH]			= { 0 };
	char			    ext[MAX_PATH]			= { 0 };
	char                SpecificPath[MAX_PATH]	= { 0 };
	char			    file_ext[MAX_PATH]		= { 0 };
	char                TftDestFile[MAX_PATH]	= { 0 };
    enum_files_return	file_return;
    BOOL multi = FALSE;
    
    
   if (_access(pFullName, 0) != 0)	
        return(FILE_NO_FILE);
    
    FileGetMultiFilesBoolean (pFileId, &multi);
    
    // Find full path for specific file
    file_return = FileGetSpecificPath( pFileId, SpecificPath);
    if (file_return != FILE_OK)
        return(file_return);
    
    FIC_splitpath(pFullName, drive, dir, file, ext);
    FIC_makepath(TftDestFile, SpecificPath, NULL, file, ext);
    FIC_makepath(file_ext, NULL, NULL, file, ext);
    
    if (FIC_copy(TftDestFile, pFullName, FIC_COPY_WITH_DATE))
    {
        if (multi)
        {
            file_return = FileWriteToList (pFileId, file_ext);
            if (file_return != FILE_OK)
                return(file_return);
        }
        else
        {
            file_return = FileSetCurrent (pFileId, file_ext);
            if (file_return != FILE_OK)
                return(file_return);
        }
        if (pSubType != NULL)
	        FileExtractSubkey (file_ext, pSubType, sSubTypeSize);
        
        
        if (remove_src)
            FIC_remove(pFullName);
    }
    else
        return (FILE_COPY_ERROR);
    
    return FILE_OK;
    
}


