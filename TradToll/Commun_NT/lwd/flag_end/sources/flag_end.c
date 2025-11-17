/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : LWD (Launcher/Watch Dog)
 * FICHIER    : FLAG_END.C
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : arret, application
 * -----------------------------------------------------------------
 * RESUME     : Signale au process startup qu'un arret application va
 *              etre effectue.
 * --------------------------------------------------------------------
 * DESCRIPTION: Fournit une fonction qui permet de signaler un arret volontaire
 *              de l'applicatif au process startup. Ainsi 'startup' laissera 
 *              l'application s'arreter et ne la relancera pas !
 * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * Revision   : 1.0.0
 * Date       : 09/1998
 * Auteur     : AFX
 * Evolution  : creation
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */


#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include <csrlc32.h>

#include <lwd.h>
#include <loc_lwd.h>

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
 * PARAMETRES: cf Win32 API
 * RETOUR    : cf Win32 API
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Initialiser la DLL à son chargement, et terminer proprement à son
 *             déchargement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
	BOOL bOK = TRUE;

	switch ( dwWhy )
	{
		// Lorsque le process fait le premier LoadLibrary concernant cette DLL
		case DLL_PROCESS_ATTACH :
		{
			break;
		}

		// Lorsque le process fait le dernier FreeLibrary concernant cette DLL
		case DLL_PROCESS_DETACH :
		{
			break;
		}

	}

	return bOK;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI LWD_Signale_Fin_Application (void)
 * PARAMETRES: aucun
 * RETOUR    : ERROR_SUCCES = l'evenement fin a pu etre signale a 'startup'
 *			   Sinon (l'evenement fin n'a pu etre signale a 'startup') code 
 *			   renvoye par GetLastError().
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Signale au process 'Startup' que l'applicatif qu'il controle
 *			   va s'arreter volontairement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */

DWORD WINAPI LWD_Signale_Fin_Application (void)
{
	HANDLE hEvent;

    BOOL fResult; // NBL 17/05/2002
    DWORD dwErr;  // NBL 17/05/2002


	/* Recuperer un handle sur l'evenement */
	hEvent = OpenEvent (EVENT_MODIFY_STATE, FALSE, LWD_EVT_FLAG_END_NAME);

	/* Si on a un acces sur l'evenement ... */
	if (hEvent != NULL)
	{
        // NBL 17/05/2002 /*... et si le changement d'etat reussit ... */
		// NBL 17/05/2002 if (SetEvent (hEvent) != 0)
        // NBL 17/05/2002
	    // NBL 17/05/2002 /* Tout s'est bien passe, le process 'Startup' est prevenu de la fin de l'application */
		// NBL 17/05/2002       return ERROR_SUCCESS;
		// NBL 17/05/2002 else
		// NBL 17/05/2002       return GetLastError();

        fResult = SetEvent(hEvent);             // NBL 17/05/2002
        dwErr = ( fResult ? ERROR_SUCCESS :     // NBL 17/05/2002
                            GetLastError() );   // NBL 17/05/2002
        CloseHandle( hEvent );                  // NBL 17/05/2002
	}
	else
	{
		// NBL 17/05/2002 return GetLastError();
        dwErr = GetLastError();                 // NBL 17/05/2002
	}
    return  dwErr;                              // NBL 17/05/2002
}