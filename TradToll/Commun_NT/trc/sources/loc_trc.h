/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : TRACE (TRC)
 * FICHIER    : LOC_TRC.H
 * LANGAGE    : C (VC++ 5.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Trace, log
 * -----------------------------------------------------------------
 * RESUME     : Module de trace assynchrone permettant la trace distante
 * --------------------------------------------------------------------
 * DESCRIPTION: Trace assynchrone placant en file d'attente les messages
 *              de trace, traités en différé par une tache de priorité
 *              basse.
 *              ATTENTION, l'assynchronisme étant géré par un thread
 *              lancé lors de la création de la premiere trace (terminé
 *              lors de la destruction de la derniere trace). La création
 *              d'un trace, son utilisation ou sa destruction dans
 *              un DllMain pose problème.
 * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * $Log : $
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */



#ifndef LOC_TRC_H
#define LOC_TRC_H


// Pour pouvoir utiliser les types DWORD, etc.
#ifndef _WINDOWS_
	#include <windows.h>
#endif


#ifdef TRC_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


/*---------------------------- MACROS et TYPES -------------------------*/

#define TRC_ERREUR_TEXTE "Texte associé à l'erreur introuvable"
#define TRC_EVENTLOG_TEXTE "TRACE"

// Prefixes pour le nommage des mailslots
#define TRC_PREFIXE_MAILSLOT_EMISSION_DISTANTE      "\\\\*\\mailslot\\TRCCSR_"
#define TRC_PREFIXE_MAILSLOT_EMISSION_LOCALE      "\\\\.\\mailslot\\TRCCSR_"
#define TRC_PREFIXE_MAILSLOT_RECEPTION      "\\\\.\\mailslot\\TRCCSR_"

#define TRC_PREFIXE_EVENTLOG "TRACE_"

#define TRC_SUFFIXE_FICHIER ".OLD"

// Prefixes for backup folder move
#define BACKUP_MOVING_ACTIVE 2
#define FILE_EXTENSIONS 8


// Nombre de messages buffurisé par le pipe interne
#define TRC_TAILLE_QUEUE         128

// Delai d'arret autorisé au thread d'écriture / émission (ms)
#define TRC_DELAI_ARRET          10000


#define TRC_LIBERER_INSTANCE   0
#define TRC_SIGNALER_EVENEMENT 1

#define TRC_MAX_TAILLE_LIGNE_TEXTE 60000

#define TRC_TYPE_EMISSION 0
#define TRC_TYPE_RECEPTION 1


// Prototypes des fonctions non exportées (absentes de "trace.h")


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


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI TRC_Thread_Ecriture( void * pvParm )
 * PARAMETRES: Cf Win32 spec.
 * RETOUR    : Cf Win32 spec.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Thread en boucle sur la lecture dans le pipe interne.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Thread_Ecriture( void * pvParm );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Ecrire_Message( TRC_MESSAGE psMsg, char * pcTrace, DWORD dwTrace )
 * PARAMETRES: psMsg     : Message à ecrire sous sa forme texte
 *             pcTrace   : Chaine où écrire le message
 *             dwTrace   : Taille du buffer pointé par pcTrace
 * RETOUR    : Taille du texte (sans le '\0')
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ecriture d'un message sous sa forme texte dans un fichier par i/o win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD TRC_Ecrire_Message( TRC_MESSAGE psMsg, char * pcTrace, DWORD dwTrace );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void TRC_Envoyer_Message( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg     : Message à envoyer
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Envoie d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
void TRC_Envoyer_Message( TRC_MESSAGE psMsg );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL TRC_Option_Valide( DWORD dwOptions, BOOL bFichier )
 * PARAMETRES: dwOptions : Masque d'option d'une instance de trace
 *             bFichier  : Indique si un nom de fichier a été défini
 * RETOUR    : TRUE si les options sont valides, FALSE sinon.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Test les options pour une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL TRC_Option_Valide( DWORD dwOptions, BOOL bFichier );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : int TRC_Ferme_Tout()
 * PARAMETRES: Aucun
 * RETOUR    : 0
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Procédure appelée lors de la fin du thread principal
 * --------------------------------------------------------------------
 * $F_FCTN
 */
int TRC_Ferme_Tout();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL WINAPI TRC_Ferme_Proc ( void * pvHandle, void * pvContext )
 * PARAMETERS: void * pvHandle  : 
 *             void * pvContext : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
BOOL WINAPI TRC_Ferme_Proc( void * pvHandle, void * pvContext );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void TRC_Change_Fichier ( char * pcFichier, char * pcFichierOld )
 * PARAMETERS: char * pcFichier    : nom du fichier à fermer complètement
 *             char * pcFichierOld : nom du fichier de backup
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme toutes les instances ouvertes d'un fichier donné
 * --------------------------------------------------------------------
 */
void TRC_Change_Fichier( char * pcFichier, char * pcFichierOld );

 
 
/*
 * --------------------------------------------------------------------
 * SYNTAX    : HINSTANCE TRC_Incrementer_Usage_DLL ( HINSTANCE hinst, int nCount )
 * PARAMETERS: HINSTANCE hinst : Instance de la DLL a recharger
 *             int nCount      : Nombre de fois qu'elle doit être rechargée
 * RETURN    : Instance de la DLL rechargée
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Incrémente l'usage d'une DLL
 * --------------------------------------------------------------------
 */
HINSTANCE TRC_Incrementer_Usage_DLL(HINSTANCE hinst);


#endif
/*---------------------------- FIN DU FICHIER -------------------------*/


