/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE PRINCIPALE DES ICONES
* FICHIER: icon.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'afficher l etat du serveur de pv
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/Icon.c_v  $
 * 
 *    Rev 1.3   26 Apr 2002 11:44:50   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.1   13 Dec 2000 16:34:42   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
* 
*    Rev 1.1   Dec 08 1999 16:38:52   pgg
*  
* 
*    Rev 1.0   Nov 22 1999 11:00:30   PGG
* Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/ 

#include <windows.h>
#include <stdio.h>
#include <ntsvc.h>
#include <icon.h>
#include <srvpv_main.h>

#include <resource.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

/*--------------- TYPEDEFS: ----------------*/

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

PRIVATE struct
{
	HICON hIconOk;
    HICON hIconKo;
    HICON hIconOkRun;

    // infos sur le thread avec sa file de "window messages"
    HANDLE hThread;
    DWORD  ThreadId;
} 
ICO = {0};

/*--------------- FUNCTIONS: ---------------*/

DWORD WINAPI ICO_WM_Dispatch (PVOID param)
{

	while ( TRUE )
        {
            if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT ) break;

            NTSVCRefreshTrayIcon(  ICO.hIconOk, "SVC_RUNNING" );

            if ( NTSVCWaitForEnd( 1000 ) != WAIT_TIMEOUT ) break;

            NTSVCRefreshTrayIcon(  ICO.hIconOkRun, "SVC_RUNNING" );
        }
    
    NTSVCInfo ("ICO_WM_Dispatch() => thread stop!");
    
    return 0;
}

PROTECTED VOID ICO_Init (void)
{
    ICO.hIconOk    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_SRVPVSVC_OK ) );
    ICO.hIconKo    = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_SRVPVSVC_KO ) );
    ICO.hIconOkRun = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_SRVPVSVC_OK_RUN ) );

	NTSVCRefreshTrayIcon( ICO.hIconKo, "SVC_INITIALIZING" );
}

PROTECTED BOOL ICO_Start (void)
{
    // création du thread du module
    ICO.hThread = NTSVCCreateThread (NULL, 0, ICO_WM_Dispatch, NULL, 0, &ICO.ThreadId, "SRVPVSVC ICO Thread");
    if (ICO.hThread == NULL)
        return FALSE;
    
    return TRUE;
}

PROTECTED BOOL ICO_Stop (DWORD TimeOut)
{	
    // on attend la disparition du thread avec délai
    if (WaitForSingleObject(ICO.hThread, TimeOut) != WAIT_OBJECT_0)
        return FALSE;
    
	NTSVCRefreshTrayIcon(NULL, NULL);

    // libération du handle de thread
    CloseHandle(ICO.hThread);
    
	DestroyIcon(ICO.hIconOk);
    DestroyIcon(ICO.hIconKo);
    DestroyIcon(ICO.hIconOkRun);

    return TRUE;
}
