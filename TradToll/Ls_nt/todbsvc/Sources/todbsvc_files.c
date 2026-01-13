/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_files.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des fichier
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
#include <stdlib.h>

#define LOC_DEF
#include <todbsvc_files.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

PRIVATE int __cdecl FileNameCmp( const void * pcFile1, const void * pcFile2 ) 
{
    return _strnicmp( pcFile1, pcFile2, MAX_PATH );
}

PROTECTED FILE_LIST_ITEM * FileListOpen( char * szPath, char * szMask, DWORD * pdwCount )
{
    DWORD dwAllocated = 0;
    DWORD dwCount = 0;
    HANDLE hFind;
    WIN32_FIND_DATA sFind;
    FILE_LIST_ITEM * psList = NULL;
    FILE_LIST_ITEM * psNewList = NULL;
    char szFullMask[MAX_PATH];

    *pdwCount = 0;

    _snprintf_s( szFullMask, _countof(szFullMask), sizeof(szFullMask), "%s\\%s", szPath, szMask );
    szFullMask[sizeof(szFullMask)-1] = '\0';

    hFind = FindFirstFile( szFullMask, &sFind );
    while ( hFind != INVALID_HANDLE_VALUE )
    {
        if ( ( sFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
        {
            while ( dwCount >= dwAllocated )
            {
                if ( dwAllocated == 0 )
                    psNewList = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, 32 * sizeof( *psNewList ) );
                else
                    psNewList = HeapReAlloc( GetProcessHeap(), 0, psList, ( dwAllocated + 16 ) * sizeof( *psNewList ) );

                if ( psNewList == NULL )
                {
                    if ( psList != NULL )
                    {
                        HeapFree( GetProcessHeap(), 0, psList );
                        psList = NULL;
                        dwCount = 0;
                    }
                    FindClose( hFind );
                    break;
                }

                psList = psNewList;
                dwAllocated += 16;
            }
            memcpy( psList[dwCount], sFind.cFileName, sizeof(psList[dwCount]) );
            dwCount ++;
        }

        if ( ! FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
    }
    if ( psList != NULL )
        qsort( psList, dwCount, sizeof( *psList ), FileNameCmp );

    *pdwCount = dwCount;
 
    return psList;
}


PROTECTED void FileListClose( FILE_LIST_ITEM * psList )
{
    if ( psList != NULL )
        HeapFree( GetProcessHeap(), 0, psList );
}


PROTECTED BOOL FileSplitName( char * pcFile, DWORD * pdwFile )
{
    DWORD dwPlaza;
    DWORD dwLane;

    return ( sscanf_s( pcFile, "MSG.PCS%04lu.L%04lu.S%08lu", &dwPlaza, &dwLane, pdwFile ) == 3 );
}


PROTECTED BOOL FileAccessCheck( char * pcFile )
{
    HANDLE hFile;
    BOOL bOpen;

    hFile = CreateFile(
            pcFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
    bOpen = ( hFile != INVALID_HANDLE_VALUE );
    if ( bOpen )
        CloseHandle( hFile );
    return bOpen;
}
