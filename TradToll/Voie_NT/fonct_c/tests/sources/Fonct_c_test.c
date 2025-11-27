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

#include <fic.h>

#define LOC_DEF
#include <Fonct_c_test.h>
#undef LOC_DEF

#include <memclass.h>






int __cdecl main( int iArgc, char ** ppcArgv )
{
	//char Subdir[MAX_PATH] = { 0 };
	//char RootPath[MAX_PATH] = { 0 };
	char pFilePath[MAX_PATH] = { 0 };
	int	iErr;
	//-----------------------------------------------------------
	FIC_makepath(pFilePath, "c:\\csr\\files", "TEST", NULL, NULL);
	printf("FIC_makepath %s", pFilePath);

	//-----------------------------------------------------------

	FIC_CreatePath("c:\\csr\\files\\test\\test1\\test2");

	FIC_deltree("c:\\csr\\files\\test");




    return NO_ERROR;

}


