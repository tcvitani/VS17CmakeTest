/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : Launcher/WhatchDog (LWD)
 * FICHIER    : LOC_LWD.H
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : end
 * -----------------------------------------------------------------
 * RESUME     : Module de lanceur d'applicatif et de chien de garde
 *              (surveillance de process)
 * --------------------------------------------------------------------
 * DESCRIPTION: 
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



#ifndef LOC_LWD_H
#define LOC_LWD_H


#ifdef LWD_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


// Pour pouvoir utiliser les types DWORD, etc.
#ifndef _WINDOWS_
	#include <windows.h>
#endif


/*---------------------------- MACROS et TYPES -------------------------*/

/* Nom de l'evenement utilise pour signaler la fin applicative */
#define LWD_EVT_FLAG_END_NAME	"CSR_Evt_Flag_End"

/* Prototypes des fonctions non exportées (absentes de "lwd.h") */


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
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk );


#endif
/*---------------------------- FIN DU FICHIER -------------------------*/


