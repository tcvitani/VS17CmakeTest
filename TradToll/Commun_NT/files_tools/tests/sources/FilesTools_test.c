/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DES
 * FILE       : DES_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for DES
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include <CSR_Files_tools.h>

#define LOC_DEF
#include <FilesTools_test.h>
#undef LOC_DEF

#include <memclass.h>






int __cdecl main( int iArgc, char ** ppcArgv )
{
	enum_files_return eErr;
	char szRootPath[MAX_PATH] = { 0 };
	char szFilePath[MAX_PATH] = { 0 };

	//-----------------------------------------------------------
	eErr = FileGetRootPath((char*)szRootPath);

	if(eErr == FILE_OK)
		printf("FileGetRootPath OK Root:[%s] ", szRootPath);
	else
		printf("FileGetRootPath error:[%d] ", eErr);

	//-----------------------------------------------------------
	eErr = FileGetSpecificPath("VRN",(char*)szFilePath);

	if (eErr == FILE_OK)
		printf("FileGetSpecificPath OK Root:[%s] ", szFilePath);
	else
		printf("FileGetSpecificPath error:[%d] ", eErr);

	//-----------------------------------------------------------









    return NO_ERROR;

}


