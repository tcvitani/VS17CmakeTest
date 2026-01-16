/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_hosts.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des hosts par l'intermédiaire des librairie
 *              d'extension, et définition des comportements par
 *              défaut (equivalent d'une libraire d'extension pour les
 *              fichiers locaux).
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <ntsvc.h>
#include <col.h>
#include <acom.h>
#include <reg.h>

#include <cmhost.h>
#include <cmwork.h>
#include <cm_glob.h>
#include <cm_utl.h>
#include <cm_text.h>

#define LOC_DEF
#include <cm_hosts.h>
#undef LOC_DEF

#include <memclass.h>




//
// PROTOTYPES PRIVES
//

PRIVATE HANDLE WINAPI _CMDllHostOpen( char * szName, HKEY hKeyConfig );
PRIVATE void WINAPI _CMDllHostClose( HANDLE hCnx );
PRIVATE BOOL WINAPI _CMDllHostStatus( HANDLE hCnx );
PRIVATE BOOL WINAPI _CMDllHostPut( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath );
PRIVATE BOOL WINAPI _CMDllHostGet( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath );
PRIVATE BOOL WINAPI _CMDllHostMove( HANDLE hCnx, char * pcSrc , char * pcDst, BOOL bCopy );
PRIVATE BOOL WINAPI _CMDllHostEnum( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFiles, DWORD * pdwCount );
PRIVATE void WINAPI _CMDllHostFree( WIN32_FIND_DATA * psFiles );




//
// CODE DES FONCTION PROTEGEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED  BOOL CMHostInit( CM_HOST * psHost, char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion de l'host à initialiser
 *             szName     : Pointe sur le nom attribué à l'host
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres de l'host
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure de gestion d'un host
 * --------------------------------------------------------------------
 */
PROTECTED  BOOL CMHostInit( CM_HOST * psHost, char * szName, HKEY hKeyConfig )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;

#define GET_PROC(p,name) \
    if ( ((FARPROC)(p) = GetProcAddress( psHost->hDll, (name) )) == NULL ) \
    { \
        dwErr = GetLastError(); \
        SVC_ERR_SS( dwErr, "ERR_HOST_GETPROC", szName, (name) ); \
        __leave; \
    } \
    else
        
#define LIRE_CHAINE(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        SVC_ERR_SS( dwErr, "ERR_HOST_PARAM", szName, (nom) );\
        __leave;\
    }\
    else

    __try
    {
        ZeroMemory( psHost, sizeof(psHost[0]) );
        InitializeCriticalSection( &psHost->sProtect );

        CM_STRNCPY( psHost->szName, szName, sizeof(psHost->szName) );
        CMTrim( psHost->szName );

        if ( hKeyConfig != NULL )
        {
            //
            // Récupérer la valeur du paramètre DLL et s'assurer
            // que le nom de la DLL n'est pas vide
            //
            LIRE_CHAINE( psHost->szDll, "Dll" );
            CMTrim( psHost->szDll );
            if ( psHost->szDll[0] == 0 )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                SVC_ERR_SS( dwErr, "ERR_HOST_PARAM", szName, "Dll" );
                __leave;
            }
        }

        psHost->bLocal = ( psHost->szDll[0] == 0 );
        if ( ! psHost->bLocal )
        {

            psHost->hDll = LoadLibrary( psHost->szDll );
            if ( psHost->hDll == NULL )
            {
                dwErr = GetLastError();
                SVC_ERR_SS( dwErr, "ERR_HOST_LOADLIB", szName, psHost->szDll );
                __leave;
            }

            GET_PROC( psHost->pfOpen  , "HostOpen" );
            GET_PROC( psHost->pfClose , "HostClose" );
            GET_PROC( psHost->pfStatus, "HostQueryStatus" );
            GET_PROC( psHost->pfGet   , "HostGetFile" );
            GET_PROC( psHost->pfPut   , "HostPutFile" );
            GET_PROC( psHost->pfMove  , "HostMoveFile" );
            GET_PROC( psHost->pfEnum  , "HostEnumFiles" );
            GET_PROC( psHost->pfFree  , "HostFreeEnum" );
        }
        else
        {
            psHost->pfOpen   = _CMDllHostOpen;
            psHost->pfClose  = _CMDllHostClose;
            psHost->pfStatus = _CMDllHostStatus;
            psHost->pfGet    = _CMDllHostGet;
            psHost->pfPut    = _CMDllHostPut;
            psHost->pfMove   = _CMDllHostMove;
            psHost->pfEnum   = _CMDllHostEnum;
            psHost->pfFree   = _CMDllHostFree;
        }

        if ( ! CMHostCnxOpen( psHost, hKeyConfig ) )
        {
            dwErr = GetLastError();
            SVC_ERR_S( dwErr, "ERR_HOST_OPEN", szName );
            __leave;
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( psHost->hDll != NULL )
                FreeLibrary( psHost->hDll );
            DeleteCriticalSection( &psHost->sProtect );
            ZeroMemory( psHost, sizeof(psHost[0]) );
        }

        SetLastError( dwErr );
    }

#undef GET_PROC
#undef LIRE_CHAINE

	return (dwErr == NO_ERROR);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostTerminate( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostTerminate( CM_HOST * psHost )
{
    CMHostCnxClose( psHost );
    if ( psHost->hDll != NULL )
        FreeLibrary( psHost->hDll );
    DeleteCriticalSection( &psHost->sProtect );
    ZeroMemory( psHost, sizeof(psHost[0]) );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostLock( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostLock( CM_HOST * psHost )
{
    EnterCriticalSection( &psHost->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostLock( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostUnlock( CM_HOST * psHost )
{
    LeaveCriticalSection( &psHost->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxOpen( CM_HOST * psHost, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres de l'host
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostOpen dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxOpen( CM_HOST * psHost, HKEY hKeyConfig )
{
    DWORD dwErr;

    CMHostLock( psHost );

    if ( psHost->hCnx == NULL )
    {
        psHost->hCnx = psHost->pfOpen( psHost->szName, hKeyConfig );
        dwErr = GetLastError();
    }
    else
        dwErr = NO_ERROR;

    CMHostUnlock( psHost );

    SetLastError( dwErr );
    return ( psHost->hCnx != NULL );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxStatus( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'host est connecté, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de la non connexion.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostQueryStatus dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxStatus( CM_HOST * psHost )
{
    DWORD dwErr;
    BOOL bResult;

    CMHostLock( psHost );

    if ( psHost->hCnx == NULL )
    {
        bResult = FALSE;
        dwErr = ERROR_NOT_READY;
    }
    else
    {
        bResult = psHost->pfStatus( psHost->hCnx );
        dwErr = GetLastError();
    }

    CMHostUnlock( psHost );

    SetLastError( dwErr );
    return ( bResult );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostCnxClose( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostClose dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostCnxClose( CM_HOST * psHost )
{
    CMHostLock( psHost );

    if ( psHost->hCnx != NULL )
    {
        psHost->pfClose( psHost->hCnx );
        psHost->hCnx = NULL;
    }

    CMHostUnlock( psHost );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxPut( CM_HOST * psHost, char * pcLocalPath , char * pcRemotePath )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcLocalPath : Chemin local du fichier source
 *             pcRemotePath: Chemin distant du fichier destination
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostPutFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxPut( CM_HOST * psHost, char * pcLocalPath , char * pcRemotePath )
{
    DWORD dwErr;
    BOOL bResult;

	NTSVCInfo("START: CM_HOSTS::CMHostCnxPut() : HostName:[%s]:pcLocalPath: %s, pcRemotePath:%s", psHost->szName, pcLocalPath, pcRemotePath);
    CMHostLock( psHost );

    if ( psHost->hCnx != NULL )
    {
        bResult = psHost->pfPut( psHost->hCnx, pcLocalPath, pcRemotePath );
        dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_NOT_READY;
        bResult = FALSE;
    }

    CMHostUnlock( psHost );
	NTSVCInfo("END: CM_HOSTS::CMHostCnxPut() : HostName:[%s]:pcLocalPath: %s, pcRemotePath:%s", psHost->szName, pcLocalPath, pcRemotePath);

    SetLastError( dwErr );
    return ( bResult );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxGet( CM_HOST * psHost, char * pcRemotePath, char * pcLocalPath )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemotePath: Chemin distant du fichier source
 *             pcLocalPath : Chemin local du fichier destination
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostGetFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxGet( CM_HOST * psHost, char * pcRemotePath, char * pcLocalPath )
{
    DWORD dwErr;
    BOOL bResult;

	NTSVCInfo("START: CM_HOSTS::CMHostCnxGet() : HostName:[%s]:pcRemotePath: %s, pcLocalPath:%s", psHost->szName, pcRemotePath, pcLocalPath);

    CMHostLock( psHost );

    if ( psHost->hCnx != NULL )
    {
        bResult = psHost->pfGet( psHost->hCnx, pcRemotePath, pcLocalPath );
        dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_NOT_READY;
        bResult = FALSE;
    }

    CMHostUnlock( psHost );

	NTSVCInfo("END: CM_HOSTS::CMHostCnxGet() : HostName:[%s]:pcRemotePath: %s, pcLocalPath:%s", psHost->szName, pcRemotePath, pcLocalPath);

    SetLastError( dwErr );
    return ( bResult );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxMove( CM_HOST * psHost, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemoteSrc : Chemin distant du fichier source
 *             pcRemoteDst : Chemin distant du fichier destination (si NULL, il s'agit d'un effacement)
 *             bCopy       : Si TRUE, effectue une copie.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostMoveFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxMove( CM_HOST * psHost, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
{
    DWORD dwErr;
    BOOL bResult;

	NTSVCInfo("START: CM_HOSTS::CMHostCnxMove() : HostName:[%s]:pcRemoteSrc: %s, pcRemoteDst:%s", psHost->szName, pcRemoteSrc, pcRemoteDst);


    CMHostLock( psHost );

    if ( psHost->hCnx != NULL )
    {
        bResult = psHost->pfMove( psHost->hCnx, pcRemoteSrc, pcRemoteDst, bCopy );
        dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_NOT_READY;
        bResult = FALSE;
    }

    CMHostUnlock( psHost );

	NTSVCInfo("END: CM_HOSTS::CMHostCnxMove() : HostName:[%s]:pcRemoteSrc: %s, pcRemoteDst:%s", psHost->szName, pcRemoteSrc, pcRemoteDst);

    SetLastError( dwErr );
    return ( bResult );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxEnum( CM_HOST * psHost, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemoteMask: Chemin distant du masque à utiliser pour l'énumération.
 *             ppsFound    : En cas de succés, retourne un pointeur sur un tableau de
 *                           structures de type WIN32_FIND_DATA contenant les informations
 *                           sur les fichiers énumérés.
 *             pdwCount    : En cas de succés, retourne le nombre de fichiers énumérés.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostEnumFiles dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxEnum( CM_HOST * psHost, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
{
    DWORD dwErr;
    BOOL bResult;

	NTSVCInfo("START: CM_HOSTS::CMHostCnxEnum() : HostName:[%s]:%s", psHost->szName, pcRemoteMask);

    CMHostLock( psHost );

    if ( psHost->hCnx != NULL )
    {
        bResult = psHost->pfEnum( psHost->hCnx, pcRemoteMask, ppsFound, pdwCount );
        dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_NOT_READY;
        bResult = FALSE;
    }

    CMHostUnlock( psHost );

	NTSVCInfo("END: CM_HOSTS::CMHostCnxEnum() : HostName:[%s]:%s", psHost->szName, pcRemoteMask);

    SetLastError( dwErr );
    return ( bResult );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostCnxFree( CM_HOST * psHost, WIN32_FIND_DATA * psFind )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             psFind : Pointe sur un tableau d'énumération de fichiers
 *                      retourné par HostEnumFiles.
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostFreeEnum dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostCnxFree( CM_HOST * psHost, WIN32_FIND_DATA * psFind )
{
    psHost->pfFree( psFind );
}
    



//
// CODE DES FONCTION PROTEGEES
// Ces fonctions sont la version locale de la DLL d'extension
// pour accéder à un host.
//



PRIVATE HANDLE WINAPI _CMDllHostOpen( char * szName, HKEY hKeyConfig )
{
    SetLastError( NO_ERROR );
    return (VOID*)0x00000CEC;
}




PRIVATE void WINAPI _CMDllHostClose( HANDLE hCnx )
{
    return;
}




PRIVATE BOOL WINAPI _CMDllHostStatus( HANDLE hCnx )
{
    SetLastError( NO_ERROR );
    return TRUE;
}




PRIVATE BOOL WINAPI _CMDllHostPut( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
{
    BOOL bResult;
    DWORD dwErr = NO_ERROR;

    bResult = CopyFile( pcLocalPath, pcRemotePath, TRUE );
    if ( ! bResult )
        dwErr = GetLastError();
    else
        dwErr = NO_ERROR;

    SetLastError( dwErr );
    return bResult;
}




PRIVATE BOOL WINAPI _CMDllHostGet( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
{
    BOOL bResult;
    DWORD dwErr = NO_ERROR;

    bResult = CopyFile( pcRemotePath, pcLocalPath, TRUE );
    if ( ! bResult )
        dwErr = GetLastError();
    else
        dwErr = NO_ERROR;

    SetLastError( dwErr );
    return bResult;
}




PRIVATE BOOL WINAPI _CMDllHostMove( HANDLE hCnx, char * pcSrc , char * pcDst, BOOL bCopy )
{
    BOOL bResult;
    DWORD dwErr = NO_ERROR;

    if ( pcDst == NULL )
        bResult = DeleteFile( pcSrc );

    else if ( bCopy )
        bResult = CopyFile( pcSrc, pcDst, TRUE );

    else
        bResult = MoveFile( pcSrc, pcDst );
        

    if ( ! bResult )
        dwErr = GetLastError();
    else
        dwErr = NO_ERROR;

    SetLastError( dwErr );
    return bResult;
}




PRIVATE BOOL WINAPI _CMDllHostEnum( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFiles, DWORD * pdwCount )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwIndex;
    DWORD dwErr = NO_ERROR;
    DWORD dwAllocated = 0;
    DWORD dwFiles = 0;
    BOOL bResult;
    WIN32_FIND_DATA sFile;
    WIN32_FIND_DATA * psFiles = NULL;
    WIN32_FIND_DATA * psNewFiles;

    __try
    {
        dwIndex = 0;

        bResult = ( ( hFind = FindFirstFile( pcRemoteMask, &sFile ) ) != INVALID_HANDLE_VALUE );
        dwErr = GetLastError();
        while ( bResult )
        {
            if ( ( sFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            {
                while ( dwIndex >= dwAllocated )
                {
                    dwAllocated += CM_LIST_INCREMENT;
                    if ( psFiles == NULL )
                        psNewFiles = HeapAlloc( GetProcessHeap(), 0, sizeof(psFiles[0]) * dwAllocated );
                    else
                        psNewFiles = HeapReAlloc( GetProcessHeap(), 0, psFiles, sizeof(psFiles[0]) * dwAllocated );
                    if ( psNewFiles == NULL )
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                    psFiles = psNewFiles;
                }
                psFiles[dwIndex] = sFile;
                dwIndex++;
            }

            bResult = FindNextFile( hFind, &sFile );
            dwErr = GetLastError();
        }

        if ( ( dwErr != ERROR_NO_MORE_FILES  ) && 
             ( dwErr != ERROR_FILE_NOT_FOUND ) &&
             ( dwErr != NO_ERROR )
           )
            __leave;

        (*ppsFiles) = psFiles;
        (*pdwCount) = dwIndex;
        psFiles = NULL;

        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psFiles != NULL )
            HeapFree( GetProcessHeap(), 0, psFiles );
        if ( hFind != INVALID_HANDLE_VALUE )
            FindClose( hFind );

        SetLastError( dwErr );
    }

	return (dwErr == NO_ERROR);
}




PRIVATE void WINAPI _CMDllHostFree( WIN32_FIND_DATA * psFiles )
{
    HeapFree( GetProcessHeap(), 0, psFiles );
}

