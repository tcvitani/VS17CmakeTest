/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : trc
 * FILE       : htrc.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Handle
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion de "handles" pour la librairie de trace
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */


#ifndef HTRC_H
#define HTRC_H

// Pour pouvoir utiliser les types DWORD, etc.
#ifndef _WINDOWS_
	#include <windows.h>
#endif

/*---------------------------- MACROS et TYPES -------------------------*/


typedef enum _HTRC_HANDLE_STATE
{
    HTRC_FREE_HANDLE = 0,
    HTRC_VALID_HANDLE,
    HTRC_INVALID_HANDLE,
    HTRC_NOT_HANDLE
}
HTRC_HANDLE_STATE;


typedef BOOL WINAPI HTRC_ENUM_PROC( void * pvHandle, void * pvContext );


/*---------------------------- PROTOTYPES -------------------------*/


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void HTRC_Initialise (  )
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialiser la gestion des handles
 * --------------------------------------------------------------------
 */
void HTRC_Initialise();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void HTRC_Termine (  )
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Terminer la gestion des handles. Tous les handles
 *             alloués sont automatiquement désalloués.
 * --------------------------------------------------------------------
 */
void HTRC_Termine();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void * HTRC_Creer_Handle ( DWORD dwSize, DWORD dwType )
 * PARAMETERS: DWORD dwSize : Taille de la structure du handle
 *             DWORD dwType : Identifiant de type de handle
 * RETURN    : un handle ou NULL si erreur (utiliser alors GetLastError())
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Créer un nouveau handle dans l'état HTRC_VALID_HANDLE
 * --------------------------------------------------------------------
 */
void * HTRC_Creer_Handle( DWORD dwSize, DWORD dwType );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : HTRC_HANDLE_STATE HTRC_Etat_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle dont on souhaite connaitre l'état
 * RETURN    : Etat du handle. C'est à dire HTRC_VALID_HANDLE si ce
 *             le handle est alloué et qu'il est marqué comme valide,
 *             HTRC_INVALID_HANDLE si il est alloué, mais marqué comme
 *             non valide, HTRC_NOT_HANDLE si ce n'est pas un handle.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Connaitre l'état d'un handle, tester si un pointeur est
 *             bien un handle.
 * --------------------------------------------------------------------
 */
HTRC_HANDLE_STATE HTRC_Etat_Handle( void * pvHandle );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Invalider_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle à marquer comme non valide
 * RETURN    : Code d'erreur win32, NO_ERROR si marquage ok.
 *             Si le handle est déja marqué invalide, on obtient
 *             une erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Marque un handle comme non valide (il reste alloué et
 *             les données qu'il contient ne sont pas altérées).
 * --------------------------------------------------------------------
 */
DWORD HTRC_Invalider_Handle( void * pvHandle );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Valider_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle à marquer comme valide
 * RETURN    : Code d'erreur win32, NO_ERROR si marquage ok.
 *             Si le handle est déja marqué valide, on obtient
 *             une erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Marque un handle comme valide (il reste alloué et
 *             les données qu'il contient ne sont pas altérées).
 * --------------------------------------------------------------------
 */
DWORD HTRC_Valider_Handle( void * pvHandle );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Detruire_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle a detruire
 * RETURN    : Un code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Supprime un handle, c'est à dire le désalloue, qu'il
 *             soit marqué non valide ou valide
 * --------------------------------------------------------------------
 */
DWORD HTRC_Detruire_Handle( void * pvHandle );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void * HTRC_Trouver_Handle ( HTRC_HANDLE_STATE eState, DWORD dwType )
 * PARAMETERS: HTRC_HANDLE_STATE eState : Etat du handle a trouver
 *             DWORD dwType             : Type du handle a trouver
 * RETURN    : Handle trouvé (NULL si non trouvé)
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recherche le premier handle dans un état donné d'un type donné
 * --------------------------------------------------------------------
 */
void * HTRC_Trouver_Handle( HTRC_HANDLE_STATE eState, DWORD dwType );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL HTRC_Enum_Handle ( DWORD dwType, HTRC_ENUM_PROC * pfEnum, void * pvContext )
 * PARAMETERS: DWORD dwType             : Type des handles a trouver
 *             HTRC_ENUM_PROC * pfEnum  : Callback fonction pour l'énumération
 *             void * pvContext         : Pour passer des paramètres à la callback fonction
 * RETURN    : TRUE si complet, FALSE si arrété par la callback procedure
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Enumére les handles par callback procedure
 * --------------------------------------------------------------------
 */
BOOL HTRC_Enum_Handle ( DWORD dwType, HTRC_ENUM_PROC * pfEnum, void * pvContext );


#endif
/*---------------------------- FIN DU FICHIER -------------------------*/
