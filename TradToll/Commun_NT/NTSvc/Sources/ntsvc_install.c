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

#include <ntsvc.h>

#define LOC_DEF
	#include <ntsvc_install.h>
#undef LOC_DEF

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED DWORD WINAPI NTSVCInstall(
        IN      char * pcFullPath,
        IN      char * pcServiceName,
        IN      DWORD  dwStartType,
        IN      char * pcDependencies,
        IN      char * pcUser,
        IN      char * pcPassword,
        IN      BOOL   bInteract )
{
	DWORD dwErr = NO_ERROR;
	SC_HANDLE hSvc = NULL;
	SC_HANDLE hMng = NULL;

    // Ouverture du service control manager avec accès complet : il faut etre administrateur
	hMng = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( hMng == NULL )
        dwErr = GetLastError();
    else
    {
        // Installation du service en fonction du contenu des paramètres
		hSvc = CreateService(
			hMng,
			pcServiceName,
			pcServiceName,
			SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS|(bInteract?SERVICE_INTERACTIVE_PROCESS:0),
			dwStartType,
			SERVICE_ERROR_NORMAL,
			pcFullPath,
			NULL,
			NULL,
			pcDependencies,
			pcUser,
			pcPassword );
		if ( hSvc == NULL )
            dwErr = GetLastError();
    }

    if ( hSvc != NULL )
        CloseServiceHandle( hSvc );
	if ( hMng != NULL )
		CloseServiceHandle( hMng );

	return dwErr;
}



PROTECTED DWORD WINAPI NTSVCRemove( 
        IN      char * pcServiceName )
{
	DWORD dwErr = NO_ERROR;
	SC_HANDLE hSvc = NULL;
	SC_HANDLE hMng = NULL;

    // Ouverture du service control manager avec accès complet : il faut etre administrateur
    hMng = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( hMng == NULL )
        dwErr = GetLastError();
    else
    {
        // Ouverture du service avec accès complet : il faut toujours etre administrateur
        hSvc = OpenService( hMng, pcServiceName, SERVICE_ALL_ACCESS );
	    if ( hSvc == NULL )
		    dwErr = GetLastError();
        else
        {
            if ( ! DeleteService(hSvc) )
		        dwErr = GetLastError();
        }
    }

    if ( hSvc != NULL )
        CloseServiceHandle( hSvc );
    if ( hMng != NULL )
        CloseServiceHandle( hMng );

    return dwErr;
}

/*-------------------------------- END OF FILE ------------------------------*/

