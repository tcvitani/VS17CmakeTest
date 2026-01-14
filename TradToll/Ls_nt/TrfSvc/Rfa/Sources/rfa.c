/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
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

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <reg.h>
#include <trc.h>

#include <createdir.h>

#define LOC_DEF
#include <csr_rfa_svc.h>
#include <csr_rfa.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)


#define RFA_ENTER_EXCLUSIVE_ACCESS \
    if ( TRUE )\
    { \
        if ( WaitForSingleObject( psHandle->hMutex, INFINITE ) != WAIT_OBJECT_0 )\
        {\
            dwErr = ERROR_INVALID_DATA;\
            __leave;\
        }\
        __try\
        {

#define RFA_LEAVE_EXCLUSIVE_ACCESS(onTerm) \
        }\
        __finally\
        {\
            {\
                onTerm\
            }\
            ReleaseMutex( psHandle->hMutex );\
        }\
        if ( dwErr != NO_ERROR )\
            __leave;\
    }\
    else;




PRIVATE struct
{
    BOOL bInService;
    RFA_WRITE_HOOK * pfServiceWriteHook;
    char szKeyBase[MAX_PATH];
    char szKeyFiles[MAX_PATH];
}
    gsRFA = 
{
    FALSE,
    NULL,
    "SYSTEM\\CurrentControlSet\\Services\\TrfSvc\\Parameters",
    "SYSTEM\\CurrentControlSet\\Services\\TrfSvc\\Parameters\\Files"
};



#define LIBINIT_UNLOCKED  0
#define LIBINIT_LOCKED    1


PRIVATE DWORD gdwLibLock = LIBINIT_UNLOCKED;
PRIVATE DWORD gdwLibCount = 0;
PRIVATE TRC_EMETTEUR ghRfaTrace = NULL;



PRIVATE BOOL WINAPI rfaIsValidHandle( RFA_STRUCT * psHandle );
PRIVATE DWORD WINAPI rfaRefreshInfo( RFA_STRUCT * psHandle, HANDLE hFile );
PRIVATE unsigned char * rfaTrim( unsigned char * pcStr );

PRIVATE BOOL rfaLibLock();
PRIVATE void rfaLibUnlock();
PRIVATE void rfaLibInit();
PRIVATE void rfaTrace( char * pcFormat, ... );





EXPORT BOOL WINAPI DllMain(
	IN HINSTANCE hInstDLL,
	IN DWORD dwReason,
	IN LPVOID pvReserved)
{
	BOOL bRes = TRUE;
    char szPath[MAX_PATH];

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH : 
        if ( GetModuleFileName( NULL, szPath, sizeof(szPath) ) )
        {
			_strlwr_s(szPath, sizeof(szPath));

			if (strstr(szPath, "trfsvc") != NULL)
			{
				gsRFA.bInService = TRUE;
			}
		}
		break;

	case DLL_THREAD_ATTACH :  
		break;

	case DLL_THREAD_DETACH :  
		break;

	case DLL_PROCESS_DETACH : 
		break;

	default : 
		break;
    }

    return bRes;
}


EXPORT BOOL WINAPI RFASetHook(
        void * pfWriteHook )
{
    DWORD dwErr = NO_ERROR;

    rfaLibInit();

    rfaTrace( "Appel  RFASetHook( 0x%08X )", pfWriteHook );

    if ( gsRFA.bInService )
        gsRFA.pfServiceWriteHook = pfWriteHook;
    else
        dwErr = ERROR_ACCESS_DENIED;

    rfaTrace( "RFASetHook() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

    SetLastError( dwErr );
    return ( dwErr == NO_ERROR );
}


EXPORT HANDLE WINAPI RFACreateFile(
        LPCTSTR lpFileKey,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    RFA_STRUCT * psHandle = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwMode;
    DWORD dwPos;
    BOOL bExist;
    BOOL bCreateDirectory;
    char * pcScan;
    char szMode[MAX_PATH];
    char szServer[MAX_PATH];
    char szTemp[MAX_PATH*2];
	char szFileKey[MAX_PATH] = {0};

    __try
    {
        rfaLibInit();

		strcpy_s(szFileKey, sizeof(szFileKey), lpFileKey);

		_strupr_s(szFileKey, sizeof(szFileKey));
		rfaTrace("RFACreateFile(%s)...", szFileKey);

        psHandle = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psHandle) );
        if ( psHandle == NULL ) { dwErr = ERROR_NOT_ENOUGH_MEMORY; __leave; }

		if (strlen(szFileKey) >= sizeof(psHandle->szKey)) { dwErr = ERROR_INVALID_PARAMETER; __leave; }
		strcpy_s(psHandle->szKey, sizeof(psHandle->szKey), szFileKey);

        dwSize = sizeof(szTemp);
		dwErr = REG_Lire_Chaine(HKEY_LOCAL_MACHINE, gsRFA.szKeyFiles, (char*)szFileKey, szTemp, &dwSize);
        if ( dwErr != NO_ERROR ) __leave;

        pcScan = szTemp;
        
        dwPos = 0;
        while ( ( (*pcScan) != '\0' ) && ( (*pcScan) != '(' ) )
            szMode[dwPos++] = *(pcScan++);
        szMode[dwPos] = '\0';
        rfaTrim( szMode );
        if ( (*pcScan) == '\0' ) { dwErr = ERROR_INVALID_DATA; __leave; }

        pcScan ++;

        dwPos = 0;
        while ( ( (*pcScan) != '\0' ) && ( (*pcScan) != ')' ) ) 
            szServer[dwPos++] = *(pcScan++);
        szServer[dwPos] = '\0';
        rfaTrim( szServer );
        if ( (*pcScan) == '\0' ) { dwErr = ERROR_INVALID_DATA; __leave; }

        pcScan ++;

        while ( ( (*pcScan) == ' ' ) && ( (*pcScan) == '\t' ) )
            pcScan++;
        if ( (*pcScan) != '=' ) { dwErr = ERROR_INVALID_DATA; __leave; }

        pcScan++;
        rfaTrim( pcScan );

        if      ( _stricmp( szMode, "IMPORT" ) == 0 )
        { 
            psHandle->bImport = TRUE;
            psHandle->bExport = FALSE;
        }
        else if ( _stricmp( szMode, "EXPORT" ) == 0 )
        {
            psHandle->bImport = FALSE;
            psHandle->bExport = TRUE;
        }
        else if ( ( _stricmp( szMode, "EXPORTIMPORT" ) == 0 ) || ( _stricmp( szMode, "IMPORTEXPORT" ) == 0 ) )
        {
            psHandle->bImport = TRUE;
            psHandle->bExport = TRUE;
        }
        else 
        { 
            dwErr = ERROR_INVALID_DATA; 
            __leave; 
        }

        if ( strlen( pcScan ) > sizeof(psHandle->szFile) ) { dwErr = ERROR_INVALID_DATA; __leave; }
		strcpy_s(psHandle->szFile, sizeof(psHandle->szFile), pcScan);

        psHandle->pfWriteHook = ( gsRFA.bInService ? gsRFA.pfServiceWriteHook : NULL );
        psHandle->bWriteAccess = gsRFA.bInService && ( ( dwDesiredAccess == RFA_SERVICE_ACCESS ) || psHandle->bExport );
        psHandle->bFailMode = ( gsRFA.bInService && ( hTemplateFile == (HANDLE)RFA_SERVICE_ACCESS ) );

		strcpy_s(psHandle->szMutex, sizeof(psHandle->szMutex), "CSR_RFA_");
		strncat_s(psHandle->szMutex, sizeof(psHandle->szMutex), szFileKey, sizeof(psHandle->szMutex));
        psHandle->szMutex[ sizeof(psHandle->szMutex) ] = '\0';
        
        for ( pcScan = psHandle->szMutex ; (*pcScan) != '\0'; pcScan ++ )
            if      ( (*pcScan) == '\\' ) (*pcScan) = '_';
            else if ( (*pcScan) == ':' ) (*pcScan) = '_';
            else if ( (*pcScan) == ' ' ) (*pcScan) = '_';

        psHandle->hMutex = CreateMutex( NULL, FALSE, psHandle->szMutex );
        if ( psHandle->hMutex == NULL ) { dwErr = GetLastError(); __leave; }

        if ( ( dwCreationDisposition == CREATE_NEW          ) ||
             ( dwCreationDisposition == CREATE_ALWAYS       ) ||
             ( dwCreationDisposition == OPEN_ALWAYS         ) ||
             ( dwCreationDisposition == TRUNCATE_EXISTING   ) )
        {
            if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }
            dwMode = GENERIC_READ|GENERIC_WRITE;
            bCreateDirectory = TRUE;
        }
        else
        {
            if ( dwCreationDisposition != OPEN_EXISTING ) { dwErr = ERROR_INVALID_PARAMETER; __leave; }
            dwMode = GENERIC_READ;
            bCreateDirectory = FALSE; 
        }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

		rfaTrace( "RFACreateFile() - psHandle->szFile : %s, dwCreationDisposition : %d, dwMode : %d", psHandle->szFile, dwCreationDisposition, dwMode);

        hFile = CreateFile(
            psHandle->szFile,
            dwMode,
            0,
            NULL,
            dwCreationDisposition,
            FILE_ATTRIBUTE_NORMAL,
            NULL );

		rfaTrace( "RFACreateFile() - TMP");

        if ( ( hFile == INVALID_HANDLE_VALUE ) && bCreateDirectory )
        {
            CreateFileDirectories( psHandle->szFile );
            hFile = CreateFile(
                psHandle->szFile,
                dwMode,
                0,
                NULL,
                dwCreationDisposition,
                FILE_ATTRIBUTE_NORMAL,
                NULL );
        }
        dwErr = GetLastError();
       
		if ( ! psHandle->bFailMode )
            if ( hFile == INVALID_HANDLE_VALUE ) __leave;
        
		bExist = ( dwErr == ERROR_ALREADY_EXISTS );
        dwErr = NO_ERROR;

        rfaRefreshInfo( psHandle, &hFile );

        if ( ( dwCreationDisposition == CREATE_NEW                ) ||
             ( dwCreationDisposition == CREATE_ALWAYS             ) ||
             ( ( dwCreationDisposition == OPEN_ALWAYS ) && bExist ) ||
             ( dwCreationDisposition == TRUNCATE_EXISTING         ) )
            if ( psHandle->pfWriteHook != NULL ) psHandle->pfWriteHook( psHandle, RFA_CALLBACK_NEW, 0, NULL );

	
		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}
		
			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;         
           
        
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( psHandle != NULL ) 
            {
                if ( psHandle->hMutex != NULL ) CloseHandle( psHandle->hMutex );
                HeapFree( GetProcessHeap(), 0, psHandle );
            }
            psHandle = INVALID_HANDLE_VALUE;
            rfaTrace( "RFACreateFile() -> Erreur = %u", dwErr );
        }
        else
        {
            psHandle->dwMark = RFA_SERVICE_ACCESS;
            rfaTrace( "RFACreateFile() -> Succes = 0x%08X", psHandle );
        }

        SetLastError( dwErr );
    }

	return psHandle;
}


EXPORT BOOL WINAPI RFACloseHandle(
        HANDLE hObject )
{
    DWORD dwErr = NO_ERROR;
    RFA_STRUCT * psHandle;

    __try
    {
        rfaTrace( "Appel à RFACloseHandle(0x%08X)", hObject );

        psHandle = hObject;
        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }

        CloseHandle( psHandle->hMutex );
        HeapFree( GetProcessHeap(), 0, psHandle );
    }
    __finally
    {
        rfaTrace( "RFACloseHandle() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );
    }

	return ( dwErr == NO_ERROR );
}


EXPORT BOOL WINAPI RFAEnter( HANDLE hFile )
{
    DWORD dwErr = NO_ERROR;
    RFA_STRUCT * psHandle = hFile;

    __try
    {
        if ( psHandle == NULL ) { dwErr = ERROR_INVALID_HANDLE; __leave; }

        if ( WaitForSingleObject( psHandle->hMutex, INFINITE ) != WAIT_OBJECT_0 )
            { dwErr = ERROR_INVALID_DATA; __leave; }
    }
    __finally
    {
        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}

EXPORT BOOL WINAPI RFALeave( HANDLE hFile )
{
    DWORD dwErr = NO_ERROR;
    RFA_STRUCT * psHandle = hFile;

    __try
    {
        if ( psHandle == NULL ) { dwErr = ERROR_INVALID_HANDLE; __leave; }

        if ( ! ReleaseMutex( psHandle->hMutex ) )
            { dwErr = GetLastError(); __leave; }
    }
    __finally
    {
        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}


EXPORT BOOL WINAPI RFADeleteAndCloseHandle(
        HANDLE hObject )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    
    __try
    {
        psHandle = hObject;

        rfaTrace( "Appel à RFADeleteAndCloseHandle(0x%08X)", hObject );

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }

        bResult = DeleteFile( psHandle->szFile );
        if ( ! bResult ) 
            dwErr = GetLastError();
        else
        {
            psHandle->ullSize = (ULONGLONG)(-1);
            if ( psHandle->pfWriteHook != NULL )
                psHandle->pfWriteHook( psHandle, RFA_CALLBACK_DEL, 0, NULL );
        }

		}
		__finally
		{
			{
				// onTerm 
				RFACloseHandle(psHandle);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;                

    } 
    __finally
    {
        rfaTrace( "RFADeleteAndCloseHandle() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );
    }

	return ( dwErr == NO_ERROR );
}



EXPORT BOOL WINAPI RFAWriteFile(
		HANDLE hRFA_STRUCT,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    ULONGLONG ullPos;
	HANDLE hFile = INVALID_HANDLE_VALUE;

    __try
    {
        rfaTrace( "Appel à RFAWriteFile( 0x%08X )", hFile );

		psHandle = hRFA_STRUCT;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }


        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ|GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        ullPos = psHandle->ullPointer;

        RFA_LL_LO(ullPos) = SetFilePointer( hFile, RFA_LL_LO(ullPos), &RFA_LL_HI(ullPos), FILE_BEGIN );
        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullPos) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) ) __leave;
        dwErr = NO_ERROR;

        bResult = WriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, &hFile );

        if ( psHandle->pfWriteHook != NULL )
        {
            rfaTrace( "RFAWriteFile() -> Appel du hook 0x%08X pour %s", psHandle->pfWriteHook, psHandle->szKey );
            psHandle->pfWriteHook( psHandle, ullPos, (*lpNumberOfBytesWritten), (BYTE*)lpBuffer );
        }

        psHandle->ullPointer = ullPos + (*lpNumberOfBytesWritten);

		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;         
    }
    __finally
    {
        rfaTrace( "RFAWriteFile() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );
        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}




EXPORT BOOL WINAPI RFAReadFile(
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    ULONGLONG ullPos;
    
    __try
    {
        rfaTrace( "Appel à RFAReadFile( 0x%08X )", hFile );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        ullPos = psHandle->ullPointer;

        RFA_LL_LO(ullPos) = SetFilePointer( hFile, RFA_LL_LO(ullPos), &RFA_LL_HI(ullPos), FILE_BEGIN );
        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullPos) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) ) __leave;
        dwErr = NO_ERROR;

        bResult = ReadFile( hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        psHandle->ullPointer = ullPos + (*lpNumberOfBytesRead);

		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;         
    }
    __finally
    {
        rfaTrace( "RFAReadFile() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );
        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}



EXPORT BOOL WINAPI RFAGetFileSize(
        HANDLE hFile,
        LPDWORD lpFileSizeHigh )
{
    DWORD dwErr = NO_ERROR;
    RFA_STRUCT * psHandle;
    DWORD dwSizeLo = 0xFFFFFFFF;
    
    __try
    {
        rfaTrace( "Appel à RFAGetFileSize( 0x%08X )", hFile );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, &hFile );

        if ( lpFileSizeHigh == NULL )
        {
            if ( psHandle->ullSize > (ULONGLONG)0xFFFFFFFE )
            {
                dwErr = ERROR_INVALID_DATA;
                dwSizeLo = 0xFFFFFFFF;
                __leave;
            }
        }
        else
            (*lpFileSizeHigh) = RFA_LL_HI(psHandle->ullSize);        
        dwSizeLo = RFA_LL_LO(psHandle->ullSize);

		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;         
    }
    __finally
    {
        rfaTrace( "RFAGetFileSize() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur",
                                                ( dwErr == NO_ERROR ) ? dwSizeLo : dwErr );

        SetLastError( dwErr );
    }

	return dwSizeLo;
}




EXPORT BOOL WINAPI RFASetFileTime(
        HANDLE hFile,
        CONST FILETIME * lpCreationTime,
        CONST FILETIME * lpLastAccessTime,
        CONST FILETIME * lpLastWriteTime )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    
    __try
    {
        rfaTrace( "Appel à RFASetFileTime( 0x%08X )", hFile );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ|GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        bResult = SetFileTime( hFile, NULL, NULL, lpLastWriteTime );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, &hFile );

        if ( psHandle->pfWriteHook != NULL )
            psHandle->pfWriteHook( psHandle, RFA_CALLBACK_DATE, 0, NULL );

		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;

    }
    __finally
    {
        rfaTrace( "RFASetFileTime() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}



EXPORT BOOL WINAPI RFAGetFileTime(
        HANDLE hFile,
        FILETIME * lpCreationTime,
        FILETIME * lpLastAccessTime,
        FILETIME * lpLastWriteTime )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    
    __try
    {
        rfaTrace( "Appel à RFAGetFileTime( 0x%08X )", hFile );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );

		dwErr = GetLastError();
		rfaTrace("RFAGetFileTime() - hFile = 0x%08X , dwErr %d", hFile, dwErr);

        if ( hFile == INVALID_HANDLE_VALUE ) {  __leave; }

        bResult = GetFileTime( hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }
		
		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;
    }
    __finally
    {
        rfaTrace( "RFAGetFileTime() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Success" : "Error", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}




EXPORT DWORD WINAPI RFASetFilePointer(
        HANDLE hFile,
        LONG lDistanceToMove,
        PLONG lpDistanceToMoveHigh,
        DWORD dwMoveMethod )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwPosLow = INVALID_SET_FILE_POINTER;
    RFA_STRUCT * psHandle;
    ULONGLONG ullPos;
    
    __try
    {
        rfaTrace( "Appel à RFASetFilePointer( 0x%08X, %u, 0x%08X, %u )",
                  hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( lpDistanceToMoveHigh == NULL )
            (LONGLONG)ullPos = lDistanceToMove;
        else
        {
            RFA_LL_LO(ullPos) = (DWORD)lDistanceToMove;
            RFA_LL_HI(ullPos) = *(DWORD*)lpDistanceToMoveHigh;
        }

        if      ( dwMoveMethod == FILE_BEGIN   ) ;
        else if ( dwMoveMethod == FILE_CURRENT ) ullPos = psHandle->ullPointer + ullPos;
        else if ( dwMoveMethod == FILE_END     ) ullPos = psHandle->ullPointer + ullPos;

        if ( ullPos > psHandle->ullSize ) { dwErr = ERROR_INVALID_PARAMETER; __leave; }
        
        psHandle->ullPointer = ullPos;
        
        dwPosLow = RFA_LL_LO(ullPos);
        if ( lpDistanceToMoveHigh != NULL )
            (*lpDistanceToMoveHigh) = RFA_LL_HI(ullPos);
        
		}
		__finally
		{
			{
				// onTerm 
				;
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;

    }
    __finally
    {
        rfaTrace( "RFASetFilePointer() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return dwPosLow;
}




EXPORT BOOL WINAPI RFASetEndOfFile(
        HANDLE hFile )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle;
    ULONGLONG ullPos;
    
    __try
    {
        rfaTrace( "Appel à RFASetEndOfFile( 0x%08X )", hFile );

        psHandle = hFile;
        hFile = INVALID_HANDLE_VALUE;

        if ( ! rfaIsValidHandle( psHandle ) ) { dwErr = ERROR_INVALID_HANDLE; __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ|GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        ullPos = psHandle->ullPointer;

        RFA_LL_LO(ullPos) = SetFilePointer( hFile, RFA_LL_LO(ullPos), &RFA_LL_HI(ullPos), FILE_BEGIN );
        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullPos) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) ) __leave;
        dwErr = NO_ERROR;

        bResult = SetEndOfFile( hFile );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, &hFile );

        if ( psHandle->pfWriteHook != NULL )
            psHandle->pfWriteHook( psHandle, RFA_CALLBACK_CUT, 0, NULL );

		}
		__finally
		{
			{
				// onTerm 
				if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;
    }
    __finally
    {
        rfaTrace( "RFASetEndOfFile() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}


EXPORT BOOL WINAPI RFADeleteFile(
        LPCTSTR lpFileKey )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle = INVALID_HANDLE_VALUE;

    __try
    {
        rfaTrace( "Appel à RFADeleteFile( %s )", lpFileKey );

        psHandle = RFACreateFile(
            lpFileKey,
            0,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( psHandle == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        if ( ! psHandle->bWriteAccess ) { dwErr = ERROR_ACCESS_DENIED; __leave; }

        bResult = DeleteFile( psHandle->szFile );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        psHandle->ullSize = (ULONGLONG)(-1);
        if ( psHandle->pfWriteHook != NULL )
            psHandle->pfWriteHook( psHandle, RFA_CALLBACK_DEL, 0, NULL );

		}
		__finally
		{
			{
				// onTerm 
				;
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;

    }
    __finally
    {
        if ( psHandle != NULL ) RFACloseHandle( psHandle );

        rfaTrace( "RFADeleteFile() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}

EXPORT BOOL WINAPI RFAMoveFileEx(
        LPCTSTR lpExistingFileName,
        LPCTSTR lpFileKey,
        DWORD dwFlags )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle = INVALID_HANDLE_VALUE;
    __try
    {
        rfaTrace( "Appel à RFAMoveFileEx( %s, %s, 0x%08X )", lpExistingFileName, lpFileKey, dwFlags );

        dwFlags = ( dwFlags & 
                    ( MOVEFILE_COPY_ALLOWED     |
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_WRITE_THROUGH
                    )
                  );
        psHandle = RFACreateFile(
            lpFileKey,
            RFA_SERVICE_ACCESS,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            (HANDLE)(RFA_SERVICE_ACCESS) );
        if ( psHandle == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        bResult = MoveFileEx( lpExistingFileName, psHandle->szFile, dwFlags );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, NULL );

		if (psHandle->pfWriteHook != NULL)
		{
			//FILETIME sftLastWriteTime;

			//if (!RFAGetFileTime(psHandle, NULL, NULL, &sftLastWriteTime))
			//{
			//	psHandle->ullLastWrite = 0;
			//}
			//else
			//{
			//	psHandle->ullLastWrite = convertFILETIME(&sftLastWriteTime);
			//}

			psHandle->pfWriteHook(psHandle, RFA_CALLBACK_REPLACE, 0, NULL);
		}

		}
		__finally
		{
			{
				// onTerm 
				;
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;

    }
    __finally
    {
        if ( psHandle != NULL )
            RFACloseHandle( psHandle );

        rfaTrace( "RFAMoveFileEx() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}

EXPORT BOOL WINAPI RFACopyFile(
        LPCTSTR lpExistingFileName,
        LPCTSTR lpFileKey,
        BOOL bFailIfExists )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle = INVALID_HANDLE_VALUE;
    __try
    {
        rfaTrace( "Appel à RFACopyFile( %s, %s, %s )", lpExistingFileName, lpFileKey, bFailIfExists ? "TRUE" : "FALSE" );

        psHandle = RFACreateFile(
            lpFileKey,
            RFA_SERVICE_ACCESS,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            (HANDLE)(RFA_SERVICE_ACCESS) );
        if ( psHandle == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        bResult = CopyFile( lpExistingFileName, psHandle->szFile, bFailIfExists );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

        rfaRefreshInfo( psHandle, NULL );

        if ( psHandle->pfWriteHook != NULL ) psHandle->pfWriteHook( psHandle, RFA_CALLBACK_REPLACE, 0, NULL );

		}
		__finally
		{
			{
				// onTerm 
				;
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;

    }
    __finally
    {
        if ( psHandle != NULL )
            RFACloseHandle( psHandle );

        rfaTrace( "RFACopyFile() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succès" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}





EXPORT BOOL WINAPI RFACopyFileKey(
        LPCTSTR lpExistingFileKey,
        LPCTSTR lpNewFileName,
        BOOL bFailIfExists )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    RFA_STRUCT * psHandle = INVALID_HANDLE_VALUE;
    __try
    {
        rfaTrace( "Appel à RFACopyFileKey( %s, %s, %s )", lpExistingFileKey, lpNewFileName, bFailIfExists ? "TRUE" : "FALSE" );

        psHandle = RFACreateFile(
            lpExistingFileKey,
            0,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( psHandle == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }


		if (WaitForSingleObject(psHandle->hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			dwErr = ERROR_INVALID_DATA;
			__leave;
		}
		__try
		{

        bResult = CopyFile( psHandle->szFile, lpNewFileName, bFailIfExists );
        if ( ! bResult ) { dwErr = GetLastError(); __leave; }

		}
		__finally
		{
			{
				// onTerm 
				;
			}

			ReleaseMutex(psHandle->hMutex);
		}
		if (dwErr != NO_ERROR)
			__leave;


    }
    __finally
    {
        if ( psHandle != NULL )
            RFACloseHandle( psHandle );

        rfaTrace( "RFACopyFileKey() -> %s(%u)", ( dwErr == NO_ERROR ) ? "Succes" : "Erreur", dwErr );

        SetLastError( dwErr );
    }

	return ( dwErr == NO_ERROR );
}



PRIVATE BOOL WINAPI rfaIsValidHandle( RFA_STRUCT * psHandle )
{
    if ( IsBadReadPtr( psHandle, sizeof(psHandle) ) ||
         IsBadWritePtr( psHandle, sizeof(psHandle) ) )
         return FALSE;
    if ( psHandle->dwMark != RFA_SERVICE_ACCESS )
         return FALSE;
    return TRUE;
}



PRIVATE DWORD WINAPI rfaRefreshInfo( RFA_STRUCT * psHandle, HANDLE * phFile )
{
    BOOL bClose = FALSE;
    DWORD dwErr = NO_ERROR;
    HANDLE hFile = NULL;
    ULONGLONG ullSize;
    ULONGLONG ullLastWrite;

    __try
    {
        if ( phFile != NULL )
        {
            hFile = (*phFile);
            if ( ( hFile == INVALID_HANDLE_VALUE ) || ( hFile == NULL ) )
            {
                dwErr = ERROR_FILE_NOT_FOUND; 
                __leave; 
            }
            CloseHandle( hFile );
            bClose = FALSE;
        }
        else
        {
            bClose = TRUE;
        }

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        RFA_LL_LO(ullSize) = GetFileSize( hFile, &RFA_LL_HI(ullSize) );
        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullSize) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) ) __leave;
        dwErr = NO_ERROR;

        if ( ! GetFileTime( hFile, NULL, NULL, (LPFILETIME)&ullLastWrite ) ) { dwErr = GetLastError(); __leave; }

        psHandle->ullLastWrite = ullLastWrite;
        psHandle->ullSize = ullSize;
    }
    __finally
    {
        if ( bClose )
        {
            CloseHandle( hFile );
            hFile = NULL;
        }
        if ( dwErr != NO_ERROR ) 
        {
            psHandle->ullSize = (ULONGLONG)(-1);
            psHandle->ullPointer = 0;
        }
        if ( phFile != NULL )
            (*phFile) = hFile;
    }

	return dwErr;
}


PRIVATE unsigned char * rfaTrim( unsigned char * pcStr )
{
    unsigned char * pcSrc = pcStr;
    unsigned char * pcDst = pcStr;
    unsigned char * pcStop = NULL;

    while ( ( *pcSrc < (unsigned char)' ' ) && ( *pcSrc != '\0' ) ) pcSrc ++;
    while ( *pcSrc != '\0' )
    {
        *pcDst = *pcSrc;
        if ( ( pcStop == NULL ) && ( *pcDst < (unsigned char)' ' ) )
            pcStop = pcDst;
        else if ( *pcDst >= (unsigned char)' ' )
            pcStop = NULL;
        pcDst ++;
        pcSrc ++;
    }
    if ( pcStop != NULL ) 
        *pcStop = '\0';
    else
        *pcDst = '\0';
    return pcStr;
}





// Declaration of this function in SDK is different. If compiling of the next function reports warnings
// C4047 and C4022 and if you have SDK tools installed please goto Tools/Options and select Directories tab.
// On this tab move ..\Microsoft visual Studio\VC98\Include to the first position and rebuild this project



PRIVATE BOOL rfaLibLock()
{
    DWORD dwOld;

    while ( TRUE )
    {
		#if (_MSC_VER == 1200) && !defined(_WIN64)
			dwOld = (DWORD)InterlockedCompareExchange( (PVOID*)&gdwLibLock, 
													   (PVOID)LIBINIT_LOCKED, 
													   (PVOID)LIBINIT_UNLOCKED );
		#else
			dwOld = (DWORD)InterlockedCompareExchange( ((volatile LONG *)&gdwLibLock), 
													   (LONG)LIBINIT_LOCKED, 
													   (LONG)LIBINIT_UNLOCKED );
		#endif
        if ( dwOld == LIBINIT_UNLOCKED )
            break;

        Sleep(1);
    }
    
    return TRUE;
}



PRIVATE void rfaLibUnlock()
{
	#if (_MSC_VER == 1200) && !defined(_WIN64)
		InterlockedCompareExchange( (PVOID*)&gdwLibLock, 
                                    (PVOID)LIBINIT_LOCKED, 
                                    (PVOID)LIBINIT_UNLOCKED );
	#else
		InterlockedCompareExchange( ((volatile LONG *)&gdwLibLock), 
									(LONG)LIBINIT_UNLOCKED,
									(LONG)LIBINIT_LOCKED );
	#endif
}



PRIVATE void rfaLibInit()
{
    DWORD dwErr;
    DWORD dwLen;
    DWORD dwConsole;
    DWORD dwMask;
    char szTrcPath[MAX_PATH];

    if ( rfaLibLock() )
    {
        if ( gdwLibCount == 0 )
        {
            dwLen = sizeof(szTrcPath);
            dwErr = REG_Defaut_Chaine( HKEY_LOCAL_MACHINE,
                                       gsRFA.szKeyBase,
                                       "RfaTraceFile",
                                       szTrcPath,
                                       &dwLen,
                                       "\\RFA_SVC.TRC" );
            if ( dwErr != NO_ERROR )
                szTrcPath[0] = 0;

            dwErr = REG_Defaut_Entier( HKEY_LOCAL_MACHINE,
                                       gsRFA.szKeyBase,
                                       "RfaConsoleTrace",
                                       &dwConsole,
                                       0 );
            if ( dwErr != NO_ERROR )
                dwConsole = 0;

            if ( dwConsole != 0 )
                dwMask = TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_IMMEDIAT | TRC_OPT_CONSOLE;
            else
                dwMask = TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_IMMEDIAT;

            if ( strlen(szTrcPath) > 0 )
            {

                dwErr = TRC_Initialise_Trace( "RFADLL",
                                              szTrcPath,
                                              dwMask,
                                              &ghRfaTrace );
                if ( dwErr != NO_ERROR )
                    ghRfaTrace = NULL;
            }
            else
                ghRfaTrace = NULL;
        }

        gdwLibCount ++;

        rfaLibUnlock();
    }
}

PRIVATE void rfaTrace( char * pcFormat, ... )
{
    va_list pMark;
    int iPos;
	char szText[1024] = {0};

    if ( ghRfaTrace != NULL )
    {
		iPos = sprintf_s(szText, 
						sizeof(szText),
						"pid=%u tid=%u (%s) ", 
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        gsRFA.bInService ? "SVC" : "USR" );

        va_start( pMark, pcFormat );
		_vsnprintf_s(szText + iPos, sizeof(szText) - iPos, _TRUNCATE, pcFormat, pMark);
        szText[sizeof(szText)-1] = '\0';
		va_end(pMark);
        //TRC_Trace_Simple( ghRfaTrace, TRC_OPT_MASK, NULL, 0, szText );

		TRC_Trace_V(ghRfaTrace, TRC_OPT_MASK, NULL, 0, szText, NULL);
		

    }
}