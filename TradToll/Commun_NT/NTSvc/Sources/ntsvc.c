/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV.C                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>

#include <reg.h>
#include <trc.h>

#define LOC_DEF
	#include <ntsvc.h>
#undef LOC_DEF

#include <ntsvc_mgmt.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void * WINAPI NTSVCGetContext()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Retourne un handle sur le contexte du service.
 * --------------------------------------------------------------------
 */
PUBLIC void * WINAPI NTSVCGetContext()
{
    return gpsMain;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCSetContext( void * pvContext )
 * PARAMETERS: pvContext : Handle de contexte du service
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Définit le contexte du service.
 *             A utiliser pour pouvoir utiliser des fonctions de la
 *             librairie NTSVC depuis des DLL (Appeler NTSVCGetContext()
 *             depuis le code de l'EXE, puis NTSVCSetContext() avec la
 *             valeur obtenue depuis le code d'une DLL linkée avec NTSVC.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCSetContext( void * pvContext )
{
    gpsMain = pvContext;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCSignalEnd()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie une demande d'arrêt du service.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCSignalEnd()
{
    SetEvent( gpsMain->sWork.hEndEvent );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCResetEnd()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Réarme l'évènement de demande de fin.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCResetEnd()
{
    ResetEvent( gpsMain->sWork.hEndEvent );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCWaitForEnd(
 *                      IN      DWORD dwTimeout )
 * PARAMETERS: IN      DWORD dwTimeout  : Temps maximum d'attente de la demande d'arrêt
 *                                        exprimé en millisecondes (INFINITE pour attendre
 *                                        indéfiniment.
 * RETURN    : WAIT_OBJECT_0 si la demande d'arrêt est arrivée
 *             WAIT_TIMEOUT si la demande d'arrêt n'est pas arrivée dans le délai demandé.
 *             WAIT_FAILED en cas d'erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test / attend une demande d'arrêt du service.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCWaitForEnd(
        IN      DWORD dwTimeout )
{
    DWORD dwErr = WaitForSingleObject( gpsMain->sWork.hEndEvent, dwTimeout );

    if ( dwErr == WAIT_TIMEOUT )
        return WAIT_TIMEOUT;
    else if ( dwErr == WAIT_OBJECT_0 )
        return WAIT_OBJECT_0;
    else
        return WAIT_FAILED;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCWaitForEndOrMultipleObjects(
 *                      IN      DWORD    dwObjCount,
 *                      IN      HANDLE * phObjects,
 *                      IN      DWORD    dwTimeout )
 * PARAMETERS: IN      DWORD    dwObjCount : Nombre d'objet sur lesquels attendre, en plus de la
 *                                           demande d'arret.
 *             IN      HANDLE * phObjects  : Tableau des handles d'objets sur lesquels attendre.
 *             IN      DWORD dwTimeout     : Temps maximum d'attente de la demande d'arrêt ou du
 *                                           signal d'un des objet du tableau. Le délai est
 *                                           exprimé en millisecondes (INFINITE pour attendre
 *                                           indéfiniment.
 * RETURN    : WAIT_OBJECT_0 si la demande d'arrêt est arrivée
 *             WAIT_OBJECT_0 + 1 + n si l'objet d'index n est passé à l'état signalé.
 *             WAIT_TIMEOUT si la demande d'arrêt ou le signal d'un objet ne sont pas arrivées
 *             dans le délai demandé
 *             WAIT_FAILED en cas d'erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test / attend une demande d'arrêt du service et test / attend le signal d'un
 *             objet. Ceci permet d'éviter les boucles de scrutation pour tester tour à tour
 *             la demande d'arret et d'autres objets.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCWaitForEndOrMultipleObjects(
        IN      DWORD    dwObjCount,
        IN      HANDLE * phObjects,
        IN      DWORD    dwTimeout )
{
    DWORD dwErr;
    HANDLE * phList = HeapAlloc( GetProcessHeap(), 0, (dwObjCount + 1) * sizeof( HANDLE ) );

    if ( phList == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    phList[0] = gpsMain->sWork.hEndEvent;
    CopyMemory( &phList[1], phObjects, dwObjCount * sizeof( HANDLE ) );

    dwErr = WaitForMultipleObjects(
            dwObjCount + 1,
            phList,
            FALSE,
            dwTimeout );

    HeapFree( GetProcessHeap(), 0, phList );

    if ( dwErr == WAIT_TIMEOUT )
        return WAIT_TIMEOUT;
    else if ( 
                ( dwErr >= ( WAIT_OBJECT_0 ) ) &&
                ( dwErr <= ( WAIT_OBJECT_0 + dwObjCount  ) )
            )
        return dwErr - WAIT_OBJECT_0;
    else
        return WAIT_FAILED;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCSetCurrentState(
 *                      IN      DWORD dwState, 
 *                      IN      DWORD dwWait, 
 *                      IN      DWORD dwError )
 * PARAMETERS: IN      DWORD dwState : Nouvel état du service parmi les valeurs suivantes :
 *                                           SERVICE_STOPPED
 *                                           SERVICE_START_PENDING (transitoire)
 *                                           SERVICE_STOP_PENDING (transitoire)
 *                                           SERVICE_RUNNING
 *                                           SERVICE_CONTINUE_PENDING (transitoire)
 *                                           SERVICE_PAUSE_PENDING (transitoire)
 *                                           SERVICE_PAUSED
 *             IN      DWORD dwWait  : Délai transitoire
 *             IN      DWORD dwError : Code d'erreur associé à l'état
 * RETURN    : Code d'erreur Win32 ou NO_ERROR.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Change l'état courant du service. S'il s'agit d'un état transitoire, le
 *             paramêtre dwWait est utilisé. Pour ne pas provoquer d'erreur, lorsqu'un
 *             service est en état transitoire, il doit être changé d'état dans les
 *             dwWait millisecondes ou le même état doit être réactivé.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCSetCurrentState(
        IN      DWORD dwState, 
        IN      DWORD dwWait, 
        IN      DWORD dwError )
{
    DWORD dwErr = NO_ERROR;
	static DWORD dwCheckPoint = 0;

    if ( gpsMain->sWork.bInitialized )
        if ( ! gpsMain->sFlags.bSimulated )
            if ( gpsMain->sWork.hStatus != 0 ) 
            {
                gpsMain->sWork.sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	            gpsMain->sWork.sStatus.dwCurrentState = dwState ;
	            gpsMain->sWork.sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	            gpsMain->sWork.sStatus.dwWin32ExitCode = dwError;
                gpsMain->sWork.sStatus.dwServiceSpecificExitCode = 0;
	            gpsMain->sWork.sStatus.dwWaitHint = dwWait;
	            if ( ( dwState == SERVICE_START_PENDING    ) ||
                     ( dwState == SERVICE_STOP_PENDING     ) ||
                     ( dwState == SERVICE_CONTINUE_PENDING ) ||
                     ( dwState == SERVICE_PAUSE_PENDING    ) )
                    gpsMain->sWork.sStatus.dwCheckPoint = ++dwCheckPoint;
                else
                    gpsMain->sWork.sStatus.dwCheckPoint = dwCheckPoint;

	            if ( ! SetServiceStatus( gpsMain->sWork.hStatus, &gpsMain->sWork.sStatus ) )
		            dwErr = GetLastError();
            }
            else
                dwErr = ERROR_INVALID_DATA;
        else
            dwErr = NO_ERROR;
    else
        dwErr = ERROR_SERVICE_NOT_ACTIVE;

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC BOOL WINAPI NTSVCIsDebugMode()
 * PARAMETERS: Aucun
 * RETURN    : TRUE si service simulé, FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Détermine si le service est simulé ou non
 * --------------------------------------------------------------------
 */
PUBLIC BOOL WINAPI NTSVCIsDebugMode()
{
    // return gpsMain->sFlags.bConsole;
    return gpsMain->sFlags.bSimulated;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCInfo( char * pcFormat, ... )
 * PARAMETERS: pcFormat, ... : Comme printf
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace informative
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCInfo( char * pcFormat, ... )
{
    va_list pMark;
    va_start( pMark, pcFormat );

    if ( gpsMain->sWork.hInfo != NULL )
        TRC_Trace_Texte_V( gpsMain->sWork.hInfo, TRC_OPT_MASK, pcFormat, pMark );


	va_end(pMark);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCError( char * pcFormat, ... )
 * PARAMETERS: pcFormat, ... : Comme printf
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace erreur (critique)
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCError( char * pcFormat, ... )
{
    va_list pMark;
    va_start( pMark, pcFormat );

    if ( gpsMain->sWork.hError != NULL )
        TRC_Direct_Trace_Texte_V( gpsMain->sWork.hError, TRC_OPT_MASK, pcFormat, pMark );

	if (gpsMain->sWork.hInfo != NULL)
		TRC_Trace_Texte_V(gpsMain->sWork.hInfo, TRC_OPT_MASK, pcFormat, pMark);

	va_end(pMark);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC NTSVC_PARAMETER_DEF * WINAPI NTSVCOpenParameters( char * pcName, ... )
 * PARAMETERS: ...     : Pour chaque paramètre 
 *                NOM    : de type char *, pointer sur le nom
 *                TYPE   : REG_SZ ou REG_DWORD
 *                SIZE   : Ignoré pour un entier, taille du buffer pour une chaine
 *                DEFAULT: Si chaine, de type char *, pointe sur la valeur par défaut,
 *                         Si entier, de type DWORD, donne la valeur par défaut,
 *                VALEUR : Si chaine, de type char *, pointe sur le buffer récupérant la valeur lue
 *                         Si entier, de type DWORD *, pointe sur le buffer récupérant la valeur lue
 *              FINIR PAR NOM = NULL
 * RETURN    : NULL si erreur, un pointeur sur un tableau de définition de paramètre,
 *             qui doit être fermé une fois utilisé.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Défini un ensemble de paramètres de la base de registres, pour le
 *             service. Une fois utilisé, à fermer avec NTSVCCloseParameters().
 * --------------------------------------------------------------------
 */
PUBLIC NTSVC_PARAMETER_DEF * WINAPI NTSVCOpenParameters(
        char * pcName,
        ... )
{
    va_list lParm;      // Pour les paramètres
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD  dwType;
    DWORD  dwSize;
    void * pvDef;
    void * pvVal; 
    char * pcSave;
    DWORD dwCount = 0;
    DWORD * pdwParams;
    NTSVC_PARAMETER_DEF * psParams;

    pcSave = pcName;
    va_start( lParm, pcName );
    while ( pcName != NULL )
    {
        dwCount ++;
        dwType = va_arg( lParm, DWORD );
        dwSize = va_arg( lParm, DWORD );
        pvDef  = va_arg( lParm, void * );
        pvVal  = va_arg( lParm, void * );
        pcName = va_arg( lParm, char * );
    }
    pcName = pcSave;
    
    pdwParams = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwCount * sizeof(NTSVC_PARAMETER_DEF) + sizeof(DWORD) );
    if ( pdwParams != NULL )
    {

        *pdwParams = dwCount;
        psParams = (NTSVC_PARAMETER_DEF *)(pdwParams+1);

	    // Placer le curseur des paramètres au premier
	    va_start( lParm, pcName );

        for ( dwIndex = 0 ; pcName != NULL ; dwIndex ++ )
        {
            dwType = va_arg( lParm, DWORD );
            dwSize = va_arg( lParm, DWORD );
            pvDef  = va_arg( lParm, void * );
            pvVal  = va_arg( lParm, void * );
            if ( ( dwType == REG_SZ ) || ( dwType == REG_DWORD ) )
            {
                strncpy_s( psParams[dwIndex].szName, MAX_PATH, pcName, sizeof(psParams[dwIndex].szName) );
                psParams[dwIndex].szName[sizeof(psParams[dwIndex].szName)-1] = '\0';
                psParams[dwIndex].dwSize = dwSize;
                psParams[dwIndex].dwType = dwType;
                psParams[dwIndex].pvDefault = pvDef;
                psParams[dwIndex].pvValue = pvVal;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            pcName = va_arg( lParm, char * );
        }
        if ( dwErr != NO_ERROR )
        {
            HeapFree( GetProcessHeap(), 0, pdwParams );
            psParams = NULL;
        }
    }

    return psParams;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCCloseParameters( NTSVC_PARAMETER_DEF * psParams )
 * PARAMETERS: psParams : Pointeur renvoyé par NTSVCOpenParameters().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme un ensemble de paramètres de la base de registres, ouvert
 *             avec NTSVCOpenParameters.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCCloseParameters( NTSVC_PARAMETER_DEF * psParams )
{
    DWORD * pdwParams = ((DWORD*)psParams)-1;
    HeapFree( GetProcessHeap(), 0, pdwParams );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCLoadParameters( NTSVC_PARAMETER_DEF * psParams, DWORD * pdwErrPos )
 * PARAMETERS: psParams  : Pointeur renvoyé par NTSVCOpenParameters().
 *             pdwErrPos : En cas d'erreur, récupère l'index du paramètre en erreur.
 * RETURN    : NO_ERROR si tout lu, Sinon un code Win32.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les valeurs du registre correspondant à un ensemble de paramètres 
 *             ouvert avec NTSVCOpenParameters().
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCLoadParameters( NTSVC_PARAMETER_DEF * psParams, DWORD * pdwErrPos )
{
    DWORD * pdwParams = ((DWORD*)psParams)-1;
    DWORD dwCount = *pdwParams;
    DWORD dwIndex;
    HKEY hKey;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwSize;
    char szKey[MAX_PATH];

    *pdwErrPos = 0xFFFFFFFF;

    if ( gpsMain->sFlags.pcServiceName == NULL )
        return ERROR_INVALID_DATA;

    _snprintf_s( szKey, MAX_PATH, sizeof(szKey)-1, "%s\\%s", gpsMain->sFlags.pcServiceName, NTSVC_REG_KEY_PARAM );

    dwErr = REG_Ouvrir( NULL, HKEY_LOCAL_MACHINE, NTSVC_REG_KEY_SERVICE, &hKey );
    if ( dwErr != ERROR_SUCCESS )
        return dwErr;

    for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
    {
        if ( psParams[dwIndex].dwType == REG_SZ ) 
        {
            dwSize = psParams[dwIndex].dwSize;
            dwErr = REG_Defaut_Chaine( 
                    hKey, 
                    szKey, 
                    psParams[dwIndex].szName, 
                    (char*)psParams[dwIndex].pvValue,
                    &dwSize,
                    (char*)psParams[dwIndex].pvDefault );
        }
        else if ( psParams[dwIndex].dwType == REG_DWORD ) 
        {
            dwErr = REG_Defaut_Entier( 
                    hKey, 
                    szKey, 
                    psParams[dwIndex].szName, 
                    (DWORD*)psParams[dwIndex].pvValue,
                    (DWORD)(psParams[dwIndex].pvDefault) );
        }
        else
            dwErr = ERROR_INVALID_DATA;
        if ( dwErr != ERROR_SUCCESS )
        {
            *pdwErrPos = dwIndex;
            break;
        }
    }

    REG_Fermer( hKey );
    return dwErr;

}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCSaveParameters( NTSVC_PARAMETER_DEF * psParams, DWORD * pdwErrPos )
 * PARAMETERS: psParams  : Pointeur renvoyé par NTSVCOpenParameters().
 *             pdwErrPos : En cas d'erreur, récupère l'index du paramètre en erreur.
 * RETURN    : NO_ERROR si tout sauvé, Sinon un code Win32.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Enregistre les valeurs correspondant à un ensemble de paramètres 
 *             ouvert avec NTSVCOpenParameters() dans le registre.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCSaveParameters( NTSVC_PARAMETER_DEF * psParams, DWORD * pdwErrPos )
{
    DWORD * pdwParams = ((DWORD*)psParams)-1;
    DWORD dwCount = *pdwParams;
    DWORD dwIndex;
    HKEY hKey;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwSize;
    char szKey[MAX_PATH];

    *pdwErrPos = 0xFFFFFFFF;

    if ( gpsMain->sFlags.pcServiceName == NULL )
        return ERROR_INVALID_DATA;

    _snprintf_s( szKey, MAX_PATH, sizeof(szKey)-1, "%s\\%s", gpsMain->sFlags.pcServiceName, NTSVC_REG_KEY_PARAM );

    dwErr = REG_Ouvrir( NULL, HKEY_LOCAL_MACHINE, NTSVC_REG_KEY_SERVICE, &hKey );
    if ( dwErr != ERROR_SUCCESS )
        return dwErr;

    for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
    {
        if ( psParams[dwIndex].dwType == REG_SZ ) 
        {
            dwSize = psParams[dwIndex].dwSize;
            dwErr = REG_Ecrire_Chaine(
                    hKey, 
                    szKey, 
                    psParams[dwIndex].szName, 
                    (char*)psParams[dwIndex].pvValue );
        }
        else if ( psParams[dwIndex].dwType == REG_DWORD ) 
        {
            dwErr = REG_Ecrire_Entier( 
                    hKey, 
                    szKey, 
                    psParams[dwIndex].szName, 
                    *(DWORD*)psParams[dwIndex].pvValue );
        }
        else
            dwErr = ERROR_INVALID_DATA;
        if ( dwErr != ERROR_SUCCESS )
        {
            *pdwErrPos = dwIndex;
            break;
        }
    }

    REG_Fermer( hKey );
    return dwErr;

}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCGetParametersCount( NTSVC_PARAMETER_DEF * psParams)
 * PARAMETERS: psParams  : Pointeur renvoyé par NTSVCOpenParameters().
 * RETURN    : Nombre de paramètres dans la liste
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Détermine le nombre de paramètres dans une liste
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCGetParametersCount( NTSVC_PARAMETER_DEF * psParams)
{
    DWORD * pdwParams = ((DWORD*)psParams)-1;
    return *pdwParams;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC HANDLE WINAPI NTSVCCreateThread( 
 *                      LPSECURITY_ATTRIBUTES lpThreadAttributes,
 *                      DWORD dwStackSize,
 *                      LPTHREAD_START_ROUTINE lpStartAddress,
 *                      LPVOID lpParameter,
 *                      DWORD dwCreationFlags,
 *                      LPDWORD lpThreadId,
 *                      char * pcThreadName )
 * PARAMETERS: Cf. définition de la fonction Win32 CreateThread
 *             pcThreadName : Name of the thread (can be null)
 * RETURN    : Cf. définition de la fonction Win32 CreateThread
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lance un thread avec encapsulation pour gestion d'exceptions
 * --------------------------------------------------------------------
 */
PUBLIC HANDLE WINAPI NTSVCCreateThread( 
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        DWORD dwStackSize,
        LPTHREAD_START_ROUTINE lpStartAddress,
        LPVOID lpParameter,
        DWORD dwCreationFlags,
        LPDWORD lpThreadId,
        char * pcThreadName )
{
    return ExcptCreateThreadWithHandler( 
            lpThreadAttributes,
            dwStackSize,
            lpStartAddress,
            lpParameter,
            dwCreationFlags,
            lpThreadId,
            pcThreadName,
            NTSVCThreadExceptionHandler );
}

#define INIT_ICON(not,ico,str) \
        {\
            ZeroMemory( &(not), sizeof((not)) );\
            (not).cbSize = sizeof((not));\
            (not).hWnd = gpsMain->sWork.hWnd;\
            (not).uID = GetCurrentProcessId();\
            (not).uFlags = NIF_MESSAGE | ( ( (ico) != NULL ? NIF_ICON  : 0 ) | ( (str) != NULL ? NIF_TIP  : 0 ) );\
            (not).uCallbackMessage = gpsMain->sWork.uiWndNotify;\
            (not).hIcon = (ico);\
            if ( (str) != NULL ) \
                _snprintf_s( (not).szTip, sizeof((not).szTip), 64, "%s (%s)", gpsMain->sWork.szServiceName, szText );\
            else\
                _snprintf_s( (not).szTip, sizeof((not).szTip), 64, "%s", gpsMain->sWork.szServiceName );\
            (not).szTip[sizeof((not).szTip)-1] = '\0';\
        }

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCRefreshTrayIcon( HICON hIcon, char * pcFormat, ... )
 * PARAMETERS: hIcon   : Handle de l'icone à mettre dans le system tray de
 *                     la window station principale.
 *             pcFormat: Pointe sur une chaine définissant un format prinft
 *                     pour remplir le tool tip text. Attention, limité à 63 caractères.
 *             ...
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Si premier appel, ajoute un icone au system tray.
 *             hIcon et pcFormat sont optionnel (NULL si non renseigné).
 *             Si hIcon et pcFormat sont NULL tous les deux, l'icone est
 *             détruit.
 * REMARQUE  : La fontion est sérialisée par une section critique, c'est
 *             à dire que si deux threads appellent la fonction en même
 *             il n'y a pas de risque de conflit d'accés au bureau.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCRefreshTrayIcon( HICON hIcon, char * pcFormat, ... )
{
    DWORD           dwErr = NO_ERROR;
    NOTIFYICONDATA  sNot;
    va_list         pMark;
    char            szText[sizeof(sNot.szTip)];

    if ( ! gpsMain->sWork.bInitialized ) 
        return;
    if ( gpsMain->sWork.hWnd == NULL )
        return;

    __try
    {
        EnterCriticalSection( &gpsMain->sWork.sIconSerializer );

        if ( pcFormat != NULL )
        {
            va_start( pMark, pcFormat );
            _vsnprintf_s( szText, sizeof(szText), 64, pcFormat, pMark );
            szText[sizeof(szText)-1] = '\0';
        }

        if ( ( hIcon == NULL ) && ( pcFormat == NULL ) )
        {
            INIT_ICON( sNot, NULL, NULL )
            Shell_NotifyIcon( NIM_DELETE, &sNot );
        }
        else
        {
            INIT_ICON( sNot, hIcon, pcFormat )
            if ( ! Shell_NotifyIcon( NIM_MODIFY, &sNot ) )
            {
                INIT_ICON( sNot, NULL, NULL )
                Shell_NotifyIcon( NIM_DELETE, &sNot );

                INIT_ICON( sNot, hIcon, pcFormat )
                Shell_NotifyIcon( NIM_ADD, &sNot );
            }
        }
    }
    __finally
    {
        LeaveCriticalSection( &gpsMain->sWork.sIconSerializer );
    }
}    




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI NTSVCSetTrayIconHandler( NTSVCTrayIconHandler * pfHandler )
 * PARAMETERS: pfHandler : Pointeur sur la fonction de callback de notification
 *                         d'événement sur l'icone de la barre de taches.
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Défini un handler pour les événement de l'icone de la barre de taches.
 *             Si un élément du menu est choisi par l'utilisateur, son identifiant sera
 *             communiqué dans le paramètre "uiId" de la fonction de callback.
 *             Si l'utilisateur effectue un double click, l'événement uiId vaudra 0.
 * REMARQUE  : Attention, ce handle doit être le plus court possible. Si l'action qu'il
 *             traite doit se prolonger, il faut créer un nouveau thread et rendre la
 *             main immédiatement.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI NTSVCSetTrayIconHandler( NTSVCTrayIconHandler * pfHandler )
{
    if ( ! gpsMain->sWork.bInitialized ) 
        return;
    EnterCriticalSection( &gpsMain->sWork.sIconSerializer );

    gpsMain->sWork.pfWndHandler = pfHandler;

    LeaveCriticalSection( &gpsMain->sWork.sIconSerializer );
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI NTSVCDefineTrayIconMenu( DWORD dwAction, UINT uiId, char * szText )
 * PARAMETERS: dwAction : Action a effectuer (parmi NTSVC_MENU_INSERT_FIRST, NTSVC_MENU_INSERT_LAST,
 *                        NTSVC_MENU_REMOVE ou NTSVC_MENU_CLEAR_ALL).
 *             uiId     : Identifiant de l'élément (0 et 0xFFFFFFFF interdits).
 *             szText   : Texte du menu (ou NULL pour les actions d'effacement).
 * RETURN    : Code d'erreur WIN32 ou NO_ERROR.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute ou enlève un élément au menu contextuel de l'icone de
 *             la barre de tache.
 * REMARQUE  : Insérer un élement dont le texte est vide a pour effet d'inserer un
 *             séparateur.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI NTSVCDefineTrayIconMenu( DWORD dwAction, UINT uiId, char * szText )
{
    BOOL  fFound = FALSE;
    DWORD dwIndex;
    DWORD dwCopy;
    DWORD dwErr = ERROR_INVALID_DATA;

    __try
    {
        EnterCriticalSection( &gpsMain->sWork.sIconSerializer );

        if ( szText == NULL )
            szText = "";

        if ( ( dwAction == NTSVC_MENU_INSERT_FIRST ) ||
             ( dwAction == NTSVC_MENU_INSERT_LAST  ) ||
             ( dwAction == NTSVC_MENU_REMOVE       ) )
        {
            if ( ( uiId == 0 ) || ( uiId == 0xFFFFFFFF ) )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            for ( dwIndex = 0 ; dwIndex < gpsMain->sWork.dwWndItems ; dwIndex ++ )
                if ( gpsMain->sWork.tsWndItem[dwIndex].uiId == uiId )
                {
                    fFound = TRUE;
                    break;
                }
        }

        if ( dwAction == NTSVC_MENU_INSERT_FIRST )
        {
            if ( fFound )
            {
                dwErr = ERROR_ALREADY_EXISTS;
                __leave;
            }

            if ( gpsMain->sWork.dwWndItems >= NTSVC_MAX_WND_ITEMS )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            for ( dwCopy = gpsMain->sWork.dwWndItems ; dwCopy > 0 ; dwCopy -- )
                gpsMain->sWork.tsWndItem[dwCopy] = gpsMain->sWork.tsWndItem[dwCopy-1];
            gpsMain->sWork.tsWndItem[0].uiId = uiId;
            strncpy_s( gpsMain->sWork.tsWndItem[0].szText, 
				       64,
                       szText, 
                       sizeof(gpsMain->sWork.tsWndItem[0].szText) );
            gpsMain->sWork.tsWndItem[0].szText[sizeof(gpsMain->sWork.tsWndItem[0].szText)-1] = 0;
            gpsMain->sWork.dwWndItems ++;
        }
        else if ( dwAction == NTSVC_MENU_INSERT_LAST )
        {
            if ( fFound )
            {
                dwErr = ERROR_ALREADY_EXISTS;
                __leave;
            }

            if ( gpsMain->sWork.dwWndItems >= NTSVC_MAX_WND_ITEMS )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            dwCopy = gpsMain->sWork.dwWndItems;
            gpsMain->sWork.tsWndItem[dwCopy].uiId = uiId;
            strncpy_s( gpsMain->sWork.tsWndItem[dwCopy].szText, 
				       64,
                       szText, 
                       sizeof(gpsMain->sWork.tsWndItem[dwCopy].szText) );
            gpsMain->sWork.tsWndItem[dwCopy].szText[sizeof(gpsMain->sWork.tsWndItem[dwCopy].szText)-1] = 0;
            gpsMain->sWork.dwWndItems ++;
        }
        else if ( dwAction == NTSVC_MENU_REMOVE )
        {
            if ( ! fFound )
            {
                dwErr = ERROR_FILE_NOT_FOUND;
                __leave;
            }

            for ( dwCopy = dwIndex + 1 ; dwCopy < gpsMain->sWork.dwWndItems ; dwCopy ++ )
                gpsMain->sWork.tsWndItem[dwCopy-1] = gpsMain->sWork.tsWndItem[dwCopy];
            gpsMain->sWork.dwWndItems --;
        }
        else if ( dwAction == NTSVC_MENU_CLEAR_ALL )
        {
            gpsMain->sWork.dwWndItems = 0;
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &gpsMain->sWork.sIconSerializer );
    }
	return dwErr;
}

/*-------------------------------- END OF FILE ------------------------------*/
