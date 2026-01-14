/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : extfilessvc
 * FILE       : extfilessvc_files.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des fichiers
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
#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <col.h>
#include <reg.h>
#include <csr_evtlog.h>

#include <extfilessvc_glob.h>


#define LOC_DEF
#include <extfilessvc_files.h>
#undef LOC_DEF

#include <memclass.h>

PRIVATE int __cdecl FileItemCmp( const void * pvFile1, const void * pvFile2 ) 
{
    WIN32_FIND_DATA * psFile1 = (WIN32_FIND_DATA*)pvFile1;
    WIN32_FIND_DATA * psFile2 = (WIN32_FIND_DATA*)pvFile2;

    if ( gsSvcWork.sParmWork.dwSortByName != 0 )
    {
        return strcmp( psFile1->cFileName, psFile2->cFileName );
    }
    else
    {
        if ( *(ULONGLONG*)(&psFile1->ftLastWriteTime) < *(ULONGLONG*)(&psFile2->ftLastWriteTime) )
            return - 1;
        else if ( *(ULONGLONG*)(&psFile1->ftLastWriteTime) == *(ULONGLONG*)(&psFile2->ftLastWriteTime) )
            return 0;
        else
            return 1;
    }
}

PROTECTED WIN32_FIND_DATA * FileListOpen( char * szPath, char * szMask, DWORD * pdwCount )
{
    DWORD dwAllocated = 0;
    DWORD dwCount = 0;
    HANDLE hFind;
    WIN32_FIND_DATA sFind;
    WIN32_FIND_DATA * psList = NULL;
    WIN32_FIND_DATA * psNewList = NULL;
    char szFullMask[EXTFILESSVC_MAXPATH];

    *pdwCount = 0;

    _snprintf_s( szFullMask, sizeof(szFullMask), sizeof(szFullMask), "%s\\%s", szPath, szMask );
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
            memcpy( &psList[dwCount], &sFind, sizeof(psList[dwCount]) );
            dwCount ++;
        }

        if ( ! FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
    }
    if ( psList != NULL )
        qsort( psList, dwCount, sizeof( *psList ), FileItemCmp );

    *pdwCount = dwCount;
 
    return psList;
}


PROTECTED void FileListClose( WIN32_FIND_DATA * psList )
{
    if ( psList != NULL )
        HeapFree( GetProcessHeap(), 0, psList );
}



