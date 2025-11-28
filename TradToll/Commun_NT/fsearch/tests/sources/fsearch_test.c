/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : FSEARCH
 * FILE       : FSEARCH_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for FSEARCH
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

#include <CSR_FSEARCH.h>

#define LOC_DEF
#include <FSEARCH_test.h>
#undef LOC_DEF

#include <memclass.h>




PROTECTED int __cdecl main(
    IN int iArgc,
    IN char ** ppcArgc )
{
    int iErr = NO_ERROR;
    BOOL fResult;
    DWORD dwErr;
    SEARCH_PARAMS sParams;
    DWORD dwFound;
    DWORD dwIndex;
    BYTE tbFound[1000];
    
    __try
    {
        sParams.fAsciiSearch = TRUE;
        sParams.fBinarySearch = FALSE;
        sParams.fBCDSearch = FALSE;

        sParams.fFromFile = TRUE;
        sParams.fFromMemory = FALSE;
        sParams.fMultiJokerAllowed = TRUE;
        sParams.fSingleJokerAllowed = TRUE;
        sParams.fSortedRecords = TRUE;
        sParams.fHeaderPresent = FALSE;
        sParams.fCaseSensitive = TRUE;
        sParams.fFixedLength = TRUE;
		strcpy_s(sParams.szPath, sizeof(sParams.szPath), ".\\EXL.txt");
        sParams.pbBufferBytes = NULL;
        sParams.dwBufferBytes = 0;

        dwFound = sizeof(tbFound);
        fResult = FSearchKeyEx(
            SEARCH_FILE_RAW(20),
            &sParams,
            "    300023140004",
            4,
            12,
            &dwFound,
            tbFound );
        if ( fResult )
        {
            printf( "\n Search OK!" );
            printf( "\n Size of the found record : %u ", dwFound );

            if ( dwFound > 0 )
            {
                printf( "\n  [" );
                for ( dwIndex = 0 ; dwIndex < dwFound ; dwIndex ++ )
                    printf( " %02X", (DWORD)tbFound[dwIndex] );
                printf( " ]" );
                printf( "\n  \"" );
                for ( dwIndex = 0 ; dwIndex < dwFound ; dwIndex ++ )
                    if ( tbFound[dwIndex] > 32 )
                        printf( "%c", tbFound[dwIndex] );
                    else
                        printf( "." );
                printf( "\"" );
            }
        }
        else
        {
            dwErr = GetLastError();
            printf( "\n Search error!" );
            printf( "\n Erreur : %u ", dwErr );
        }
    }
    __finally
    {
        MessageBox( NULL, "OK", "OK", MB_OK );
    }

	return iErr;
}
