/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : trctst
 * FILE       : trcdbg.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <reg.h> // ZBR

// *************************************************************************
// La partie suivante permet de tester les problèmes liées à l'allocation et
// la désallocation en mode debug si la macro TRC_DEBUG_HEAP est définie
#if defined(_DEBUG) && defined(TRC_DEBUG)

#pragma message( "ATTENTION : ACTIVATION TRACE DE DEBUG" )

#ifdef TRC_DEBUG_LOC
    #pragma message( "ATTENTION : TRACE DE DEBUG LOC" )
    DWORD _gdwAllocCount;
    DWORD _gdwFreeCount;
    DWORD _gdwAllocBytes;
    DWORD _gdwFreeBytes;
    CRITICAL_SECTION _gsDbgCritical;
    BOOL _gbInit = FALSE;
	// ZBR
	CHAR szTrc[MAX_PATH] = {0}; 
	CHAR szTrcPath[MAX_PATH] = {0}; 
	CHAR szKey[MAX_PATH] = {0};
	DWORD dwLen;

	sprintf(szKey, "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);

	dwLen = MAX_PATH;
	if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, 
		szKey, 
		CSR_REG_KEYv_TRACEPATH, 
		szTrcPath,
		&dwLen ) != ERROR_SUCCESS )
	{
		strcpy( szTrc, "C:\\exploit\\logs\\_traces\\TRC.TXT" );
	}
	else
	{
		strcat(szTrcPath, "\\TRC.TXT");
		strcpy(szTrc, szTrcPath);
	}


	//RegGetApplicationPath(szTrc);
	//FIC_makepath(szTrc, NULL, szTrc, "TRC", "TXT");
	// end of ZBR

    void TRCDBGTrace( char * pcFile, char * pcFormat, ... )
    {
        FILE * pfFile;
    	va_list lParm;
        if ( ! _gbInit )
        {
            InitializeCriticalSection( &_gsDbgCritical );
            _gbInit = TRUE;
        }
        EnterCriticalSection( &_gsDbgCritical );
    	va_start( lParm, pcFormat );
        pfFile = fopen( pcFile, "a+" );
        if ( pfFile != NULL )
        {
            vfprintf( pfFile, pcFormat, lParm );
            fprintf( pfFile, "\n" );
            fclose( pfFile );
        }
        LeaveCriticalSection( &_gsDbgCritical );
    }
    void * TRCDBGHeapAlloc( HANDLE hHeap, DWORD dwFlags, DWORD dwBytes  )
    {
        LPVOID lpMem;
        lpMem = HeapAlloc( hHeap, dwFlags, dwBytes );
        if ( lpMem == NULL )
            TRCDBGTrace( szTrc, "*HEAP* HeapAlloc(%d) -> ERREUR", dwBytes ); // ZBR
        else
        {
            _gdwAllocCount ++;
            _gdwAllocBytes += HeapSize(hHeap,0,lpMem);
            TRCDBGTrace( szTrc, "*HEAP* HeapAlloc(%d) -> ptr=0x%08X bloc=%d", dwBytes, lpMem, HeapSize(hHeap,0,lpMem) ); // ZBR
        }
        TRCDBGTrace( 
            szTrc, // ZBR
            "*HEAP* ALOC:%d(%d) FREE:%d(%d) DIFF:%d(%d)",
            _gdwAllocCount, 
            _gdwAllocBytes, 
            _gdwFreeCount, 
            _gdwFreeBytes,
            _gdwAllocCount - _gdwFreeCount,
            _gdwAllocBytes - _gdwFreeBytes );
        return lpMem;
    }
    BOOL TRCDBGHeapFree( HANDLE hHeap, DWORD dwFlags, LPVOID lpMem )
    {
        DWORD dwBytes = HeapSize(hHeap,0,lpMem);
        BOOL bResult;
        bResult = HeapFree( hHeap, dwFlags, lpMem );
        if ( ! bResult )
            TRCDBGTrace( szTrc, "*HEAP* HeapFree(0x%08X) -> ERREUR", lpMem ); // ZBR
        else
        {
            _gdwFreeCount ++;
            _gdwFreeBytes += dwBytes;
            TRCDBGTrace( szTrc, "*HEAP* HeapFree(0x%08X) -> bloc=%d", lpMem, dwBytes ); // ZBR
        }
        TRCDBGTrace( 
            szTrc, // ZBR 
            "*HEAP* ALOC:%d(%d) FREE:%d(%d) DIFF:%d(%d)",
            _gdwAllocCount, 
            _gdwAllocBytes, 
            _gdwFreeCount, 
            _gdwFreeBytes,
            _gdwAllocCount - _gdwFreeCount,
            _gdwAllocBytes - _gdwFreeBytes );
        return bResult;
    }
#else
    #pragma message( "ATTENTION : TRACE DE DEBUG NLOC" )
    extern DWORD _gdwAllocCount;
    extern DWORD _gdwFreeCount;
    extern DWORD _gdwAllocBytes;
    extern DWORD _gdwFreeBytes;
    extern CRITICAL_SECTION _gsDbgCritical;
    extern BOOL _gbInit;
    void TRCDBGTrace( char * pcFile, char * pcFormat, ... );
    void * TRCDBGHeapAlloc( HANDLE hHeap, DWORD dwFlags, DWORD dwBytes  );
    BOOL TRCDBGHeapFree( HANDLE hHeap, DWORD dwFlags, LPVOID lpMem );
#endif

#define HeapAlloc TRCDBGHeapAlloc
#define HeapFree TRCDBGHeapFree

#endif
// Fin test des problèmes liées à l'allocation et la désallocation en mode debug
// *************************************************************************


