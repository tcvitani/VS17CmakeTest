/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : TRACE (TRC)
 * FICHIER    : TRACE.H
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


/*---------------------------- DEFINITION DU TYPE DE LIBRAIRIE -------------------------*/

#ifndef TRC_H
#define TRC_H


// Pour pouvoir utiliser les types DWORD, HKEY et les
// fonctions RegXXXX. et la macro WINAPI. Windows.H vérifiant
// qu'il n'a pas déjà été inclus, on peut l'insérer sans pb.
#include <windows.h>


// Definition des modificateur déclaratifs (PUBLIC etc.)
#ifdef TRC_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


// Codes d'options pour l'instance d'écoute
#define TRC_OPT_MAILSLOT_LOCAL        ((DWORD)0x00000001)
#define TRC_OPT_MAILSLOT_DISTANT      ((DWORD)0x00000002)
#define TRC_OPT_FICHIER               ((DWORD)0x00000004)
#define TRC_OPT_CONSOLE               ((DWORD)0x00000008)
#define TRC_OPT_EVENTLOG              ((DWORD)0x00000010)
#define TRC_OPT_NUMEROTATION          ((DWORD)0x00000020)
#define TRC_OPT_IMMEDIAT              ((DWORD)0x00000040)
#define TRC_OPT_CREER_FICHIER         ((DWORD)0x00000080)
#define TRC_OPT_TEXTE_SEUL            ((DWORD)0x00000100)
#define TRC_OPT_TOUJOURS_OUVERT       ((DWORD)0x00000200)



// Pour controler la validité d'une option ou pour
// toutes les choisir
#define TRC_OPT_MASK                  ((DWORD)0x000003FF)



// Taille maximum du texte pour un message de trace
#define TRC_MAX_TEXTE           60000


// Taille maximum du fichier de trace
#define TRC_TAILLE_FICHIER_DEFAUT ((LONGLONG)(1400000))



// Structure définissant une instance d'un émetteur de traces
typedef struct _TRC_EMETTEUR_STRUCT
{
	BOOL bActive;
    BOOL bFileMissing;
    
    DWORD dwOptions;

	DWORD dwNum;

	HANDLE hMailSlot;
	HANDLE hFichier;
	HANDLE hConsole;
	HANDLE hEventLog;

	LONGLONG llTailleFichier;

	char pcFichier[ MAX_PATH+1 ];
	char pcFichierOld[ MAX_PATH+5 ];
	char pcMailSlot[ MAX_PATH+1 ];
	char pcEventLog[ MAX_PATH+1 ];
	
} TRC_EMETTEUR_STRUCT, * TRC_EMETTEUR;



// Structure définissant un message de trace
typedef struct _TRC_MESSAGE_STRUCT
{
	DWORD dwTaille;

    BOOL bDirect;

	TRC_EMETTEUR psEmetteur;

	DWORD dwOptions;

	DWORD dwTick;
	SYSTEMTIME sDate;
	DWORD dwNum;

	DWORD dwDonnees;
	DWORD dwTexte;
	BYTE pbDonnees[1];

} TRC_MESSAGE_STRUCT, * TRC_MESSAGE;






// Structure définissant une instance d'un recepteur de traces
typedef struct _TRC_RECEVEUR_STRUCT
{
	CRITICAL_SECTION sCritical;
	HANDLE hMailSlot;
} TRC_RECEVEUR_STRUCT, *TRC_RECEVEUR;




/*---------------------------- PROTOTYPES -------------------------*/




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Vider_Traces( DWORD dwDelai)
 * PARAMETRES: dwDelai : Delai d'attente maxi pour le vidage de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Attendre que toutes les trace en traitement assynchrone
 *             soient émise
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Vider_Traces( DWORD dwDelai);




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Initialise_Trace( char * pcCle, char * pcFichier, DWORD dwOptions, TRC_EMETTEUR * ppsEmetteur )
 * PARAMETRES: pcCle    : Clé servant a identifier l'instance de trace
 *             pcFichier: Nom du fichier de trace.
 *             dwOptions: Options de trace.
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             ppsEmetteur: Pointe sur un "handle" qui recevra les données de l'instance
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Créer une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Initialise_Trace( char * pcCle, char * pcFichier, DWORD dwOptions, TRC_EMETTEUR * ppsEmetteur );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL TRC_Fichier_Actif ( TRC_EMETTEUR psEmetteur )
 * PARAMETERS: psEmetteur   : valeur obtenue avec TRC_Initialise. Doit avoir l'option
 *                     TRC_OPT_FICHIER active.
 * RETURN    : TRUE : le fichier existe, est ouvert et en cours d'utilisation pour
 *               l'instance de trace testée.
 *             FALSE: soit le fichier n'existe pas, soit il n'a pas pu être ouvert, soit
 *               l'option TRC_OPT_FICHIER n'est pas active.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si le fichier précisé à l'initilisation de l'instance de trace
 *             reçoit effectivement les trace émises.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI TRC_Fichier_Actif( TRC_EMETTEUR psEmetteur );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Taille_Max_Fichier( TRC_EMETTEUR psEmetteur, LONGLONG llTailleFichier )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise. Doit avoir l'option
 *                     TRC_OPT_FICHIER active.
 *             llTailleFichier : Taille maximum du fichier de trace.
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Definir la taille maximum d'un fichier de trace.
 *             La valeur par défaut étant TRC_TAILLE_FICHIER_DEFAUT
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Taille_Max_Fichier( TRC_EMETTEUR psEmetteur, LONGLONG llTailleFichier );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
 * PARAMETRES: psEmetteur   : pointe sur la structure obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat : comme vprintf
 *             lParam   : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat : comme vprintf
 *             lParam   : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace directement : au retour de
 *             la fonction, le système garantie que les données de la
 *             trace ont effectivement été traitées.
 *             Attention, la seule restriction est de ne pas appeler
 *             TRC_Termine_Trace() pour cette instance dans un autre thread
 *             en même temps que TRC_Direct_Trace_V() (possibilité
 *             d'exception).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : pointe sur la structure obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcTexte  : texte de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcTexte  : texte de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm ))
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pcFormat, lParm : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm ))
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pcFormat, lParm : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : pointe sur la structure obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
 * PARAMETRES: psEmetteur   : pointe sur la structure obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwErreur )
 * PARAMETRES: psEmetteur   : pointe sur la structure obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             dwErreur    : Code d'erreur win32
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwCodeWin32 );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwErreur )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *             dwErreur    : Code d'erreur win32
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Direct_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwCodeWin32 );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Trace( TRC_EMETTEUR psEmetteur )
 * PARAMETRES: psEmetteur : instance de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK). Dans le cas
 *             ou il s'agit de la dernière instance. La fonction
 *             renvoie ERROR_NO_MORE_ITEMS
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Terminer une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Termine_Trace( TRC_EMETTEUR psEmetteur );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Tout( )
 * PARAMETRES: 
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Terminer toutes les instances de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Termine_Tout();



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Initialise_Ecoute( char * pcCle, TRC_RECEVEUR * psReceveur )
 * PARAMETRES: pcCle    : Clé de création de la trace
 *             psReceveur : Pointeur sur une structre qui recevra les infos de
 *                        l'écoute.
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Créer un instance d'écoute
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Initialise_Ecoute( char * pcCle, TRC_RECEVEUR * psReceveur );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Ecoute_Message( TRC_RECEVEUR psReceveur, TRC_MESSAGE * ppsMsg )
 * PARAMETRES: psReceveur : Instance d'écoute
 *             ppsMsg   : Récupère un pointeur sur un message. Si NULL, pas de message
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Receptionne des messages de trace.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Ecoute_Message( TRC_RECEVEUR psReceveur, TRC_MESSAGE * ppsMsg);



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Libere_Message( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Libère la mémoire allouée pour la réception d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Libere_Message( TRC_MESSAGE psMsg );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Ecoute( TRC_RECEVEUR psReceveur )
 * PARAMETRES: pcCle    : Clé de création de la trace
 *             psReceveur : Pointeur sur une l'instance d'écoute
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Termine une instance d'écoute
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Termine_Ecoute( TRC_RECEVEUR psReceveur );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void TRC_Message_Date_GMT( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
 *             void TRC_Message_Date_Locale( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
 * PARAMETRES: psMsg   : Message reçu
 *             psDate  : Pointe sur une structure recevant la date GMT ou local
 *                       du message
 * RETOUR    : 
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère la date du message.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI TRC_Message_Date_GMT( TRC_MESSAGE psMsg, SYSTEMTIME * psDate );
EXPORT void WINAPI TRC_Message_Date_Locale( TRC_MESSAGE psMsg, SYSTEMTIME * psDate );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL TRC_Message_Direct( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : TRUE si le message a été généré par envoie direct
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le mode d'envoie du message de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI TRC_Message_Direct( TRC_MESSAGE psMsg );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Num( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Numéro d'ordre du message, 0 si pas de numérotation.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le numéro d'ordre d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Message_Num( TRC_MESSAGE psMsg );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Tick( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Nombre de ms écoulées depuis dernier reboot.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le nombre de ms depuis dernier reboot
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Message_Tick( TRC_MESSAGE psMsg );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BYTE * TRC_Message_Donnees( TRC_MESSAGE psMsg )
 *             char * TRC_Message_Texte( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : un pointeur sur les données / le texte d'un message
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère un pointeur sur les données / le texte d'un message.
 *             Attention, si le message est libéré, la valeur retournée
 *             n'est plus utilisable.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BYTE * WINAPI TRC_Message_Donnees( TRC_MESSAGE psMsg );
EXPORT char * WINAPI TRC_Message_Texte( TRC_MESSAGE psMsg );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Taille_Donnees( TRC_MESSAGE psMsg )
 *             DWORD TRC_Message_Taille_Texte( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Taille des données / du texte d'un message
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère la taille des données / du texte d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI TRC_Message_Taille_Donnees( TRC_MESSAGE psMsg );
EXPORT DWORD WINAPI TRC_Message_Taille_Texte( TRC_MESSAGE psMsg );



#endif
/*---------------------------- FIN DU FICHIER -------------------------*/

