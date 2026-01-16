/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_workers.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des workers par l'intermédiaire des librairie
 *              d'extension, et définition des comportements par
 *              défaut (equivalent d'une libraire d'extension).
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
#include <cm_dirs.h>

#define LOC_DEF
#include <cm_workers.h>
#undef LOC_DEF

#include <memclass.h>




//
// PROTOTYPES PRIVES
//

PRIVATE HANDLE WINAPI _CMDllWorkerOpen( char * szName, HKEY hKeyConfig );
PRIVATE void WINAPI _CMDllWorkerClose( HANDLE hWrk );
PRIVATE BOOL WINAPI _CMDllWorkerProcessFile( HANDLE hWrk, char * szFilePath );
PRIVATE char * _CMReplaceStrings( char * pcSrc, char * pcDst, DWORD dwDstSize, char * pcFind, char * pcReplace );



//
// CODE DES FONCTION PROTEGEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED  BOOL CMWorkerInit( CM_WORKER * psWorker, char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker   : Pointe sur la structure de gestion du worker à initialiser
 *             szName     : Pointe sur le nom attribué au worker
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres du worker
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure de gestion d'un worker
 * --------------------------------------------------------------------
 */
PROTECTED  BOOL CMWorkerInit( CM_WORKER * psWorker, char * szName, HKEY hKeyConfig )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    char szSort[CM_MAX_STR];
    char szPath[CM_MAX_STR];
    char szPathWork[CM_MAX_STR];
    CM_SORT sSortParams;

#define GET_PROC(p,name) \
    if ( ((FARPROC)(p) = GetProcAddress( psWorker->hDll, (name) )) == NULL ) \
    { \
        dwErr = GetLastError(); \
        SVC_ERR_SS( dwErr, "ERR_WORKER_GETPROC", szName, (name) ); \
        __leave; \
    } \
    else

#define OPT_PROC(p,name) \
    (FARPROC)(p) = GetProcAddress( psWorker->hDll, (name) )
    
#define LIRE_CHAINE(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        SVC_ERR_SS( dwErr, "ERR_WORKER_PARAM", szName, (nom) );\
        __leave;\
    }\
    else

#define DEF_CHAINE(var,nom,def) \
    dwSize = sizeof(var);\
    dwErr = REG_Defaut_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize, (def) );\
    if ( dwErr != NO_ERROR )\
    {\
        dwErr = NO_ERROR;\
        strcpy_s( (var), dwSize, (def) );\
    }\
    else

#define LIRE_ENTIER(var,nom) \
    dwErr = REG_Lire_Entier( hKeyConfig, NULL, (nom), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        SVC_ERR_SS( dwErr, "ERR_WORKER_PARAM", szName, (nom) );\
        __leave;\
    }\
    (var) = dwSize

#define DEF_ENTIER(var,nom,def) \
    dwErr = REG_Defaut_Entier( hKeyConfig, NULL, (nom), &dwSize, def );\
    if ( dwErr != NO_ERROR )\
    {\
        NTSVCInfo( "REG_Defaut_Entier retourne %u", dwErr );\
        dwErr = NO_ERROR;\
        (var) = (def);\
    }\
    else\
        (var) = dwSize

    __try
    {
        //
        // Initialiser la structure
        //
        ZeroMemory( psWorker, sizeof(psWorker[0]) );
        InitializeCriticalSection( &psWorker->sProtect );

        //
        // Récupérer le nom du worker
        //
        CM_STRNCPY( psWorker->szName, szName, sizeof(psWorker->szName) );
        CMTrim( psWorker->szName );

        //
        // Récupérer la DLL de gestion du worker,
        // la charger et l'initialiser
        //
        LIRE_CHAINE( psWorker->szDll, "Dll" );
        CMTrim( psWorker->szDll );
        if ( psWorker->szDll[0] != 0 )
        {
            //
            // La chaine n'est pas vide, il s'agit réellement d'une DLL
            // Il faut donc la charger et obtenir un pointeur sur chacune
            // des fonctions nécessaire à la gestion d'un worker.
            //
            psWorker->hDll = LoadLibrary( psWorker->szDll );
            if ( psWorker->hDll == NULL )
            {
                dwErr = GetLastError();
                SVC_ERR_SS( dwErr, "ERR_WORKER_LOADLIB", szName, psWorker->szDll );
                __leave;
            }
            GET_PROC( psWorker->pfOpen   , "WorkerOpen" );
            GET_PROC( psWorker->pfClose  , "WorkerClose" );
            OPT_PROC( psWorker->pfProcess, "WorkerProcessFile" );
            if ( psWorker->pfProcess == NULL )
            {
                GET_PROC( psWorker->pfProcessEx, "WorkerProcessFileEx" );
            }
        }
        else
        {
            //
            // La chaine est vide, il s'agit des fonctions par défaut
            // de gestion d'un worker. On fait directement sur les
            // fonction du module.
            //
            psWorker->pfOpen      = _CMDllWorkerOpen;
            psWorker->pfClose     = _CMDllWorkerClose;
            psWorker->pfProcess   = _CMDllWorkerProcessFile;
            psWorker->pfProcessEx = NULL;
        }

        //
        // Récupérer les paramètres de fonctionnement courant
        //
        LIRE_ENTIER( psWorker->dwPollingPeriod , "PollingPeriod" );
        DEF_ENTIER ( psWorker->dwInAndAckDelete, "InAndAckDelete", 1 );
        DEF_ENTIER ( psWorker->dwInAndErrDelete, "InAndErrDelete", 0 );
        DEF_CHAINE ( psWorker->szNameMask      , "NameMask"      , "" );
        DEF_ENTIER ( psWorker->dwMaxAckFiles   , "MaxAckFiles"   , 0 );
        DEF_ENTIER ( psWorker->dwMaxAckKB      , "MaxAckKB"      , 0 );
        DEF_ENTIER ( psWorker->dwMaxErrFiles   , "MaxErrFiles"   , 0 );
        DEF_ENTIER ( psWorker->dwMaxErrKB      , "MaxErrKB"      , 0 );

        DEF_ENTIER ( psWorker->dwEnsureOrder   , "EnsureOrder"   , 0 );

        DEF_CHAINE ( szSort                    , "SortBy"  , "INC:NAME" );
        CMTrim( szSort );
        if ( ! CMAnalyseSortParams( szSort, &sSortParams ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            SVC_ERR_SS( dwErr, "ERR_WORKER_PARAM", szName, "SortBy" );
            __leave;
        }

        //
        // Récupérer les infos spécifiques à chaque répertoires
        // de travail et les initialiser.
        //
        
        LIRE_CHAINE( szPath, "LocalMailIn" );
        CMDirInit( &psWorker->sDirRAB, gsSvcWork.psLocalHost, szPath, psWorker->szNameMask );
        psWorker->sDirRAB.sSort = sSortParams;

        LIRE_CHAINE( szPath, "LocalMailInAck" );
        CMDirInit( &psWorker->sDirRABS, gsSvcWork.psLocalHost, szPath, psWorker->szNameMask );
        psWorker->sDirRABS.sSort = sSortParams;

        DEF_CHAINE( szPathWork, "LocalMailInErr", "" );
        if ( szPathWork[0] == 0 )
			strcpy_s(szPathWork, sizeof(szPathWork), szPath);
        psWorker->fSXIdentical = ( _stricmp( szPathWork, szPath ) == 0 );
        if ( ! psWorker->fSXIdentical )
        {
            CMDirInit( &psWorker->sDirRABX, gsSvcWork.psLocalHost, szPathWork, psWorker->szNameMask );
            psWorker->sDirRABX.sSort = sSortParams;
        }

        //
        // Initialiser le worker
        //
        psWorker->hWrk = psWorker->pfOpen( psWorker->szName, hKeyConfig );
        if ( psWorker->hWrk == NULL )
        {
            dwErr = GetLastError();
            SVC_ERR_S( dwErr, "ERR_WORKER_OPEN", szName );
            __leave;
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( psWorker->hDll != NULL )
                FreeLibrary( psWorker->hDll );
            DeleteCriticalSection( &psWorker->sProtect );
            ZeroMemory( psWorker, sizeof(psWorker[0]) );
        }

        SetLastError( dwErr );   
    }

#undef GET_PROC
#undef LIRE_CHAINE

	return (dwErr == NO_ERROR);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerTerminate( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un worker
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerTerminate( CM_WORKER * psWorker )
{
    psWorker->pfClose( psWorker->hWrk );
    CMDirTerminate( &psWorker->sDirRAB );
    CMDirTerminate( &psWorker->sDirRABS );
    if ( psWorker->hDll != NULL )
        FreeLibrary( psWorker->hDll );
    DeleteCriticalSection( &psWorker->sProtect );
    ZeroMemory( psWorker, sizeof(psWorker[0]) );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerLock( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerLock( CM_WORKER * psWorker )
{
    EnterCriticalSection( &psWorker->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerLock( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerUnlock( CM_WORKER * psWorker )
{
    LeaveCriticalSection( &psWorker->sProtect );
}





//
// CODE DES FONCTION PROTEGEES
// Ces fonctions sont la version locale de la DLL d'extension
// pour accéder à un host.
//



PRIVATE HANDLE WINAPI _CMDllWorkerOpen( char * szName, HKEY hKeyConfig )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    char * pcCommand = NULL;

    __try
    {
        pcCommand = HeapAlloc( GetProcessHeap(), 0, CM_MAX_STR );
        if ( pcCommand == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        dwErr = REG_Lire_Chaine( hKeyConfig, NULL, "Command", pcCommand, &dwSize );
        if ( dwErr != NO_ERROR )
            __leave;
    }
    __finally
    {
        if ( ( dwErr != NO_ERROR ) && ( pcCommand != NULL ) )
        {
            HeapFree( GetProcessHeap(), 0, pcCommand );
            pcCommand = NULL;
        }

        SetLastError( dwErr );
    }

	return (VOID*)pcCommand;
}




PRIVATE void WINAPI _CMDllWorkerClose( HANDLE hWrk )
{
    HeapFree( GetProcessHeap(), 0, hWrk );

    return;
}




PRIVATE BOOL WINAPI _CMDllWorkerProcessFile( HANDLE hWrk, char * szFilePath )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    BOOL bResult;
    char * pcCommand = hWrk;
    STARTUPINFO sStartup;
    PROCESS_INFORMATION  sProcess;
    char szExpanded[CM_MAX_STR];
    char szCommand[CM_MAX_STR];

    __try
    {
        ZeroMemory( &sStartup, sizeof(sStartup) );
        ZeroMemory( &sProcess, sizeof(sProcess) );

        bResult = ExpandEnvironmentStrings( pcCommand, szExpanded, sizeof(szExpanded) );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        _CMReplaceStrings( szExpanded, szCommand, sizeof(szCommand), "$(FILEPATH)", szFilePath );

        sStartup.cb = sizeof( sStartup );
        sStartup.dwFlags = STARTF_USESHOWWINDOW;
        sStartup.wShowWindow = SW_HIDE;
        bResult = CreateProcess(
            NULL, 
            szCommand, 
            NULL, 
            NULL,
            FALSE,
            CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS,
            NULL,
            NULL,
            &sStartup,
            &sProcess );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            __leave;
        }

        dwRes = WaitForSingleObject( sProcess.hProcess, INFINITE );
        if ( dwRes != WAIT_OBJECT_0 )
        {
            if ( dwRes == WAIT_FAILED )
                dwErr = GetLastError();
            else
                dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        bResult = GetExitCodeProcess( sProcess.hProcess, &dwRes );
        if ( dwRes != NO_ERROR )
        {
            dwErr = ERROR_BAD_COMMAND;
            __leave;
        }
    }
    __finally
    {
        if ( sProcess.hThread != NULL )
            CloseHandle( sProcess.hThread );
        if ( sProcess.hProcess != NULL )
            CloseHandle( sProcess.hProcess );
        SetLastError( dwErr );
    }

	return (dwErr == NO_ERROR);
}





PRIVATE char * _CMReplaceStrings( char * pcSrc, char * pcDst, DWORD dwDstSize, char * pcFind, char * pcReplace )
{
    DWORD dwLen = (DWORD)strlen( pcFind );
    char * pcCur;
    char * pcNext;
    char szBuffer[CM_MAX_STR];

    (*pcDst) = 0;
    CM_STRNCPY( szBuffer, pcSrc, sizeof(szBuffer) );
    pcCur = szBuffer;

    while ( ( pcNext = strstr( pcCur, pcFind ) ) != NULL )
    {
        (*pcNext) = 0;
        CM_STRNCAT( pcDst, pcCur, dwDstSize );
        CM_STRNCAT( pcDst, pcReplace, dwDstSize );
        pcCur = pcNext + dwLen;
    }
    CM_STRNCAT( pcDst, pcCur, dwDstSize );

    return pcDst;
}

