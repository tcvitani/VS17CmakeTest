/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMWORK
 * FILE       : CMWORK.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, worker
 * --------------------------------------------------------------------
 * SUMMARY    : Extension traitement pour commoc.
 * --------------------------------------------------------------------
 * DESCRIPTION: Extension de traitement pour commoc. Cette librairie
 *              fournie l'ensemble des fonctions requises par Commoc
 *              pour mettre en oeuvre un traitement fichiers.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>

#if defined(CMWORK_EXPORTS) || defined(CMWORKEX_EXPORTS)
#    include <public.h>
#else
#    include <export.h>
#endif

#include <cmwork.h>


#include <memclass.h>



#define CMWORK_MAX_STR  512
#define CMWORK_STRNCPY(x,y,n)      (strncpy_s(x,n,y,_TRUNCATE),(x)[n-1]=0,x)
#define CMWORK_STRNCAT(x,y,n)      (strncat_s(x,n,y,_TRUNCATE),(x)[n-1]=0,x)



PRIVATE char * _CMWorkReplaceStrings( char * pcSrc, char * pcDst, DWORD dwDstSize, char * pcFind, char * pcReplace );
PRIVATE DWORD WINAPI _SortFileNbr( char * szNbrMask, char * szFileName );



//
// CODE DES FONCTIONS EXPORTEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: szName     : Nom donné à l'instance créée.
 *             hKeyConfig : Handle de clé de registre dans laquelle la
 *                          fonction va aller chercher les informations
 *                          dont elle à besoin pour configurer l'instance.
 * --------------------------------------------------------------------
 * RETURN    : Un handle de l'instance en cas de succés d'initialisation.
 *             NULL en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de traitement de fichier.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerOpen=_WorkerOpen@8" )
#endif
EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )
{
    DWORD dwErr = NO_ERROR;
	DWORD dwSize = CMWORK_MAX_STR;
    char * pcCommand = NULL;

    __try
    {
        pcCommand = HeapAlloc( GetProcessHeap(), 0, CMWORK_MAX_STR );
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




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI WorkerClose( HANDLE hWrk )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libére les resources allouées par WorkerOpen.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerClose=_WorkerClose@4" )
#endif
EXPORT void WINAPI WorkerClose( HANDLE hWrk )
{
    HeapFree( GetProcessHeap(), 0, hWrk );
    return;
}




#ifdef CMWORK_EXPORTS

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 *             szFileName : Chemin complet du fichier à traiter
 * --------------------------------------------------------------------
 * RETURN    : TRUE lorsque le fichier a été traité. FALSE sinon.
 *             GetLastError() donnant un code d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le traitement d'un fichier
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerProcessFile=_WorkerProcessFile@8" )
#endif
EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    BOOL bResult;
    char * pcCommand = hWrk;
    STARTUPINFO sStartup;
    PROCESS_INFORMATION  sProcess;
    char szExpanded[CMWORK_MAX_STR];
    char szCommand[CMWORK_MAX_STR];

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

        _CMWorkReplaceStrings( szExpanded, szCommand, sizeof(szCommand), "$(FILEPATH)", szFilePath );

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

#endif


#ifdef CMWORKEX_EXPORTS

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath, void * pvReserved )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 *             szFileName : Chemin complet du fichier à traiter
 *             pvReserved : Reservé pour de futures évolutions
 * --------------------------------------------------------------------
 * RETURN    : Un (et un seul) flag parmi :
 * RETURN    : Un (et un seul) flag parmi :
 *                     CM_WORKER_ACK_NONE
 *                     CM_WORKER_ACK_OK
 *                     CM_WORKER_ACK_RETRY_ERR
 *                     CM_WORKER_ACK_RETRY_BUSY
 *                     CM_WORKER_ACK_ABANDON
 *             indiquant le résultat du traitement, combiné avec
 *             un (et un seul) flag parmi :
 *                     CM_WORKER_ACT_NONE
 *                     CM_WORKER_ACT_MOVE_ACK
 *                     CM_WORKER_ACT_COPY_ERR
 *                     CM_WORKER_ACT_MOVE_ERR
 *                     CM_WORKER_ACT_DELETE
 *             indiquant l'action à mener en retour.
 *             GetLastError() donne un code d'erreur lorsque c'est
 *             applicable et NO_ERROR sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le traitement étendu d'un fichier
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:WorkerProcessFileEx=_WorkerProcessFileEx@12" )
#endif
EXPORT DWORD WINAPI WorkerProcessFileEx( HANDLE hWrk, char * szFilePath, void * pvReserved )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    DWORD dwStatus;
    BOOL bResult;
    char * pcCommand = hWrk;
    STARTUPINFO sStartup;
    PROCESS_INFORMATION  sProcess;
    char szExpanded[CMWORK_MAX_STR];
    char szCommand[CMWORK_MAX_STR];

    __try
    {
        ZeroMemory( &sStartup, sizeof(sStartup) );
        ZeroMemory( &sProcess, sizeof(sProcess) );

        bResult = ExpandEnvironmentStrings( pcCommand, szExpanded, sizeof(szExpanded) );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            dwStatus = CM_WORKER_ACK_RETRY_BUSY | CM_WORKER_ACT_NONE;
            __leave;
        }

        _CMWorkReplaceStrings( szExpanded, szCommand, sizeof(szCommand), "$(FILEPATH)", szFilePath );

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
            dwStatus = CM_WORKER_ACK_RETRY_BUSY | CM_WORKER_ACT_NONE;
            __leave;
        }

        dwRes = WaitForSingleObject( sProcess.hProcess, INFINITE );
        if ( dwRes != WAIT_OBJECT_0 )
        {
            if ( dwRes == WAIT_FAILED )
                dwErr = GetLastError();
            else
                dwErr = ERROR_INVALID_DATA;
            dwStatus = CM_WORKER_ACK_RETRY_BUSY | CM_WORKER_ACT_NONE;
            __leave;
        }

        if ( ! GetExitCodeProcess( sProcess.hProcess, &dwRes ) )
        {
            dwErr = GetLastError();
            dwStatus = CM_WORKER_ACK_RETRY_BUSY | CM_WORKER_ACT_NONE;
            __leave;
        }

        dwErr = NO_ERROR;
        if ( dwRes == 0 )
            dwStatus = CM_WORKER_ACK_OK | CM_WORKER_ACT_MOVE_ACK;
        else
            dwStatus = CM_WORKER_ACK_RETRY_ERR | CM_WORKER_ACT_COPY_ERR;
    }
    __finally
    {
        if ( sProcess.hThread != NULL )
            CloseHandle( sProcess.hThread );
        if ( sProcess.hProcess != NULL )
            CloseHandle( sProcess.hProcess );
        SetLastError( dwErr );
    }

	return dwStatus;
}


#endif


PRIVATE char * _CMWorkReplaceStrings( char * pcSrc, char * pcDst, DWORD dwDstSize, char * pcFind, char * pcReplace )
{
	size_t iLen = strlen(pcFind);
    char * pcCur;
    char * pcNext;
    char szBuffer[CMWORK_MAX_STR];

    (*pcDst) = 0;
    CMWORK_STRNCPY( szBuffer, pcSrc, sizeof(szBuffer) );
    pcCur = szBuffer;

    while ( ( pcNext = strstr( pcCur, pcFind ) ) != NULL )
    {
        (*pcNext) = 0;
        CMWORK_STRNCAT( pcDst, pcCur, dwDstSize );
        CMWORK_STRNCAT( pcDst, pcReplace, dwDstSize );
		pcCur = pcNext + iLen;
    }
    CMWORK_STRNCAT( pcDst, pcCur, dwDstSize );

    return pcDst;
}



