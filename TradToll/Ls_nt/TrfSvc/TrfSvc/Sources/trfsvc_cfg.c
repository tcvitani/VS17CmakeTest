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
#include <acom.h>
#include <col.h>
#include <ntsvc.h>
#include <reg.h>
#include <csr_rfa.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>
#include <trfsvc_main.h>

#define LOC_DEF
#include <trfsvc_cfg.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

typedef LPVOID WINAPI RUDEInitialize( LPCTSTR lpParameters );
typedef BOOL WINAPI RUDEGetStatus( LPVOID pvContext );
typedef VOID WINAPI RUDETerminate( LPVOID pvContext );


PRIVATE unsigned char * CfgTrim( unsigned char * pcStr );


// --------------- CODE ----------------------


PROTECTED DWORD CfgLoad()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwIdSize;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwPos;
    HANDLE hFile;
    HCOLLECTIONITEM hItem;
    char * pcScan;
    TRFSVC_FILE sFile;
    char szTemp[MAX_PATH*2];
    char szMode[MAX_PATH*2];
    char szServer[MAX_PATH*2];

    dwIndex = 0;

    do
    {
        ZeroMemory( &sFile, sizeof(sFile) );
		
		sFile.bIsReceivingFileInProgress = FALSE;

        dwIdSize = sizeof( sFile.szFileKey );
        dwSize = sizeof( szTemp );

        dwErr = REG_Enum_Valeurs( 
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\" TRFSVC_SERVICE_NAME "\\Parameters\\Files",
            dwIndex,
            sFile.szFileKey,
            &dwIdSize,
            &dwType,
            szTemp,
            &dwSize );
        if ( dwErr == ERROR_NO_MORE_ITEMS ) { dwErr = NO_ERROR; break; }
        if ( dwErr != NO_ERROR ) break;
        if ( dwType != REG_SZ ) { dwIndex++ ; continue; }

        pcScan = szTemp;
        
        dwPos = 0;
        while ( ( (*pcScan) != '\0' ) && ( (*pcScan) != '(' ) )
            szMode[dwPos++] = *(pcScan++);
        szMode[dwPos] = '\0';
        CfgTrim( szMode );
        if ( (*pcScan) == '\0' ) { dwErr = ERROR_INVALID_DATA; break; }

        pcScan ++;

        dwPos = 0;
        while ( ( (*pcScan) != '\0' ) && ( (*pcScan) != ')' ) ) 
            szServer[dwPos++] = *(pcScan++);
        szServer[dwPos] = '\0';
        CfgTrim( szServer );
        if ( (*pcScan) == '\0' ) { dwErr = ERROR_INVALID_DATA; break; }

        pcScan ++;

        while ( ( (*pcScan) == ' ' ) && ( (*pcScan) == '\t' ) )
            pcScan++;
        if ( (*pcScan) != '=' ) { dwErr = ERROR_INVALID_DATA; break; }

        pcScan++;
        CfgTrim( pcScan );

        if      ( _stricmp( szMode, "IMPORT" ) == 0 )
        { 
            sFile.bImport = TRUE;
            sFile.bExport = FALSE;
        }
        else if ( _stricmp( szMode, "EXPORT" ) == 0 )
        {
            sFile.bImport = FALSE;
            sFile.bExport = TRUE;
        }
        else if ( ( _stricmp( szMode, "EXPORTIMPORT" ) == 0 ) || ( _stricmp( szMode, "IMPORTEXPORT" ) == 0 ) )
        {
            sFile.bImport = TRUE;
            sFile.bExport = TRUE;
        }
        else 
        { 
            dwErr = ERROR_INVALID_DATA; 
            break; 
        }

        if ( ( strlen( szServer ) > sizeof( sFile.szServer ) ) ||
             ( strlen( pcScan ) > sizeof( sFile.szFile ) ) )
             { dwErr = ERROR_INVALID_DATA; break; }

		strcpy_s(sFile.szFile, sizeof(sFile.szFile), pcScan);
		strcpy_s(sFile.szServer, sizeof(sFile.szServer), szServer);
		sprintf_s(sFile.szTemp, sizeof(sFile.szTemp), "%s.$$$", sFile.szFile);

        hFile = RFACreateFile( sFile.szFileKey, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
            sFile.ullLastTime = 0;
        else
        {
			FILETIME sftLastWriteTime;

			if (!RFAGetFileTime(hFile, NULL, NULL, &sftLastWriteTime))
			{
				sFile.ullLastTime = 0;
			}
			else
			{
				sFile.ullLastTime = convertFILETIME(&sftLastWriteTime);
			}

			RFACloseHandle( hFile );
        }

        hItem = ColItemAdd( gsSvcWork.hColFiles, &sFile.szFileKey, &sFile, sizeof(sFile) );
        if ( hItem == NULL ) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        dwIndex ++;
    }
    while ( TRUE );

    return dwErr;
}


PROTECTED void CfgUnload()
{
    HCOLLECTIONITEM hItem;

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColExtensions, &hItem ) )
    {
        ColItemRemove( hItem );
        hItem = COL_SCAN_BEGIN;
    }
}


PROTECTED DWORD CfgExtensionsStart()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwDllSize;
    DWORD dwType;
    HCOLLECTIONITEM hItem;
    TRFSVC_EXTENSION sExt;

	NTSVCError("CfgExtensionsStart..");

    dwIndex = 0;

    do
    {
        ZeroMemory( &sExt, sizeof(sExt) );

        dwNameSize = sizeof( sExt.szName );
        dwDllSize = sizeof( sExt.szFileDll );

        dwErr = REG_Enum_Valeurs(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\" TRFSVC_SERVICE_NAME "\\Parameters\\Extensions",
            dwIndex,
            sExt.szName,
            &dwNameSize,
            &dwType,
            sExt.szFileDll,
            &dwDllSize );
        if ( dwErr == ERROR_NO_MORE_ITEMS ) { dwErr = NO_ERROR; break; }
        if ( dwErr != NO_ERROR ) break;
        if ( dwType != REG_SZ ) { dwIndex++ ; continue; }

        sExt.hInst = LoadLibrary( sExt.szFileDll );
        if ( sExt.hInst == NULL ) { dwErr = GetLastError(); break; }

        if ( ( sExt.pfInitialize = GetProcAddress( sExt.hInst, "RUDEInitialize" ) ) == NULL ) 
            dwErr = GetLastError();
        else if ( ( sExt.pfTeminate = GetProcAddress( sExt.hInst, "RUDETerminate" ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( sExt.pfGetStatus = GetProcAddress( sExt.hInst, "RUDEGetStatus" ) ) == NULL )
            dwErr = GetLastError();
        if ( dwErr != NO_ERROR ) { FreeLibrary( sExt.hInst ); break; }

        sExt.pvContext = ((RUDEInitialize*)sExt.pfInitialize)( sExt.szName );
        if ( sExt.pvContext == NULL ) { dwErr = ERROR_INVALID_DATA; FreeLibrary( sExt.hInst ); break; }

        hItem = ColItemAdd( gsSvcWork.hColExtensions, &sExt.szName, &sExt, sizeof(sExt) );
        if ( hItem == NULL ) 
        { 
            ((RUDEInitialize*)sExt.pfTeminate)( sExt.pvContext );
            dwErr = ERROR_INVALID_DATA; 
            FreeLibrary( sExt.hInst ); 
            break; 
        }

        dwIndex ++;
    }
    while ( TRUE );

    if ( dwErr != NO_ERROR )
        CfgExtensionsStop();

    return dwErr;
}




PROTECTED void CfgExtensionsStop()
{
    HCOLLECTIONITEM hItem;
    TRFSVC_EXTENSION * psExt;

	NTSVCError("CfgExtensionsStop..");

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColExtensions, &hItem ) )
    {
        psExt = ColItemData( hItem );
        ((RUDETerminate*)psExt->pfTeminate)( psExt->pvContext );
        FreeLibrary( psExt->hInst ); 
        ColItemRemove( hItem );
        hItem = COL_SCAN_BEGIN;
    }
}


PROTECTED BOOL CfgExtensionTestStatus( char * szFaulty, DWORD dwBytes )
{
    HCOLLECTIONITEM hItem;
    TRFSVC_EXTENSION * psExt;
    BOOL bStatus = TRUE;

    if ( gsSvcWork.hColExtensions != NULL )
    {
        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hColExtensions, &hItem ) )
        {
            psExt = ColItemData( hItem );
            bStatus = ((RUDEGetStatus*)psExt->pfGetStatus)( psExt->pvContext );
            if ( ! bStatus )
            {
				strncpy_s(szFaulty, sizeof(szFaulty), psExt->szName, dwBytes);
                szFaulty[dwBytes-1] = '\0';
                break;       
            }
        }
    }
    return bStatus;
}


PRIVATE unsigned char * CfgTrim( unsigned char * pcStr )
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

