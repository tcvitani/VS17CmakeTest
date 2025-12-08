/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV.C                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project						 */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>

#include "ntsvc.h"

#include <memclass.h>

void WINAPI MyIconHandler( UINT uiId );
                                         
char gszServiceName[] = "TestSvc";


/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
 *                        OUT char ** ppcServiceName,
 *                        OUT NTSVCCommandMain ** ppfCommand )
 * PARAMETERS: Cf doc NTSVC
 * RETURN    : Cf doc NTSVC
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cf doc NTSVC
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
        OUT     char ** ppcServiceName,
        OUT     NTSVCCommandMain ** ppfCommand )
{
    (*ppcServiceName) = gszServiceName;
    (*ppfCommand) = NULL;
    return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
 *                          IN      DWORD    dwArgc,
 *                          IN      char  ** ppcArgv )
 * PARAMETERS: dwArgc  : nombre de params.
 *             ppcArgv : liste des arguments
 * RETURN    : paramètre, ligne de commande, initialisation
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré pour le service
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
        IN      DWORD    dwArgc,
        IN      char  ** ppcArgv )
{
    DWORD dwIndex;
    HICON thIcon[4];

    NTSVC_ERR( "Avant SERVICE_START_PENDING" );
    NTSVCSetCurrentState( SERVICE_START_PENDING, 10000, NO_ERROR );
    NTSVC_ERR( "Aprés SERVICE_START_PENDING" );

    NTSVC_ERR( "Avant SERVICE_RUNNING" );
    NTSVCSetCurrentState( SERVICE_RUNNING, 0, NO_ERROR );
    NTSVC_ERR( "Aprés SERVICE_RUNNING" );

    thIcon[0] = LoadIcon( NULL, IDI_INFORMATION );
    thIcon[1] = LoadIcon( NULL, IDI_QUESTION );
    thIcon[2] = LoadIcon( NULL, IDI_WARNING );
    thIcon[3] = LoadIcon( NULL, IDI_WINLOGO );

    Sleep( 1000 );

    NTSVCSetTrayIconHandler( &MyIconHandler );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 1, "ZZZZ" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_REMOVE, 1, NULL );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 1, "&Re-create icon" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 100, "" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_LAST, 2, "&Exception" );
    NTSVCDefineTrayIconMenu( NTSVC_MENU_INSERT_FIRST, 3, "&Stop service" );

    for ( dwIndex = 1 ; TRUE ; dwIndex ++ )
    {
        if ( NTSVCWaitForEnd( 200 ) != WAIT_TIMEOUT )
            break;

        NTSVCRefreshTrayIcon( thIcon[dwIndex%4], "0123456789012345678901234567890123456789012345678901234567890123456789COUCOU %u", dwIndex );
    }

    NTSVC_ERR( "Avant SERVICE_STOP_PENDING" );
    NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );
    NTSVC_ERR( "Aprés SERVICE_STOP_PENDING" );

    Sleep( 1000 );

    NTSVC_ERR( "Avant SERVICE_STOPPED" );
    NTSVCSetCurrentState( SERVICE_STOPPED, 0, NO_ERROR );
    NTSVC_ERR( "Aprés SERVICE_STOPPED" );

    // Tout est terminé
}

void WINAPI MyIconHandler( UINT uiId )
{
    if ( ( uiId == 0 ) || ( uiId == 3 ) )
        NTSVCSignalEnd();

    if ( uiId == 1 )
        NTSVCRefreshTrayIcon( NULL, NULL );

    if ( uiId == 2 )
        *(DWORD*)(NULL) = 0;
}

/*-------------------------------- END OF FILE ------------------------------*/