/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : BITBL
 * FILE       : BITBL_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for BITBL
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

#include <CSR_BITBL.h>

#define LOC_DEF
#include <BITBL_test.h>
#undef LOC_DEF

#include <memclass.h>




PROTECTED int __cdecl main(IN int iArgc,  IN char ** ppcArgc )
{
    int iErr = NO_ERROR;
	DWORD dwVal = 0;
	unsigned char pSrc[] = { 0x15, 0xA8, 0x2D, 0xB9 };

	DWORD dwCurrentBitPos = 8;
	DWORD dwAvailableBits = 24;

    __try
    {
        // Insert test code here
		dwVal = 0; 
		if (BITBLGetNumber(&dwVal, 8, pSrc, &dwCurrentBitPos, &dwAvailableBits))
		{
			printf("BITBLGetNumber returned TRUE: Val:%X", dwVal);
		}
		else
		{
			printf("BITBLGetNumber returned FALSE: ");

		}

        
    }
    __finally
    {
		;
    }

 	return iErr;
}
