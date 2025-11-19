/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : LWD (Lauch/Watch Dog)
 * FICHIER    : STARTUP.H
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : process, program, key
 * -----------------------------------------------------------------
 * RESUME     : Lanceur et surveillance d'application (de programmes)
 * --------------------------------------------------------------------
 * DESCRIPTION: Ce programme va lire dans la base de registre la cle 'startup' 
 *				et lance tous les  programmes qui se trouvent dans cette cle.
 *				Il surveille ensuite si l'un des programmes s'arrete. Si c'est le cas et que 
 *				l'arret n'est pas volontaire par l'application, il arrete alors 
 *				tous les programmes et les relance a nouveau.
 *				Ce module fournit une fonction (DLL) qui permet a une application de 
 *              signaler son arret, afin d'eviter que ce module ne redemarre l'application!
 *				Si le module detecte la fin d'un process(programme) alors que le signal d'arret
 *				n'a pas ete communique, le module arrete (violemment) les autres process et les
 *              relance tous.
 *
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

#ifndef LWD_H
#define LWD_H


#ifdef LWD_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


// Pour pouvoir utiliser les types DWORD, etc.
#ifndef _WINDOWS_
	#include <windows.h>
#endif

// Sous-Clé de Config
// ====================
#define LWD_REG_KEYn_STARTUP	"Startup\\"

// Noms de l'evenement 'Stop application'
#define	LWD_EVT_APPLICATION_STOP	"LWD_EVT_APPLICATION_STOP"


// Codes de sortie des processes
typedef enum
{
	EXIT_OK				= CSR_OK,

    FIRST_EXIT_CODE = EXIT_OK,

	EXIT_KO,
	EXIT_NO_EXE_REP,
	EXIT_NO_VIRTUEL,
	EXIT_NO_INIT,
	EXIT_PB_CONTEXT,
	EXIT_NO_ECRAN,
	EXIT_NO_COMM,
	EXIT_NO_RTC,
	EXIT_NO_BAL,
	EXIT_NO_TACHES,
	EXIT_PB_SYSTEME,
	EXIT_NO_LANE_INI,
	EXIT_NO_DEN_MESS,
	EXIT_BAD_NUM_CP,
	EXIT_PB_RESSOURCE,
	EXIT_NO_RESEAU,

	EXIT_RUN_ERR_REP_TRACE,
	EXIT_RUN_ERR_INIT_TRACE,
	EXIT_RUN_ERR_PARAM,
	EXIT_RUN_ERR_STOP_EVT,
	EXIT_RUN_ERR_STARTUP_HANDLE,
	EXIT_RUN_ERR_INSTANCE_LOAD,
	EXIT_RUN_ERR_INSTANCE_START,
	EXIT_RUN_ERR_INSTANCE_STOP,

	EXIT_BUTEE, /* Laisser cette constante en fin d'enumeration */
    LAST_EXIT_CODE = EXIT_BUTEE - 1,
    NB_EXIT_CODE = LAST_EXIT_CODE - FIRST_EXIT_CODE + 1
}
enum_exit_codes ;

/*---------------------------- PROTOTYPES -------------------------*/

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

EXPORT DWORD WINAPI LWD_Signale_Fin_Application (void);

#endif