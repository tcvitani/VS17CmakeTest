/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : RUN
 * FILE       : run.c
 * LANGAGE    : C
 * --------------------------------------------------------------------
 * KEYWORDS   : Class = type de module
 * --------------------------------------------------------------------
 * SUMMARY    : Lanceur de modules pour les applications voie NT.
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

// Standard Microsoft definitions for types like DWORD, WORD etc...
// for macros like WINAPI etc...
// Because this modules verifies if it has already been included,
// there is no danger of macro redefinition
#include <WINDOWS.H>
// ...
// ..


// Standard and CS Route specific includes 
#include <CSRLC32.H>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <direct.h>
#include <errno.h>

#include <noyau.h>

#include <reg.h>
#include <trc.h>
#include <lwd.h>

#define MODULE_LAUNCHER
#include <run.h>
#undef MODULE_LAUNCHER

// Project specific includes
#include <loc_run.h>


#include <memclass.h>

/*--------------- FUNCTIONS: ---------------*/

/* SBA 17/04/01 */
typedef enum {
	LAUNCH_ABORTED,
	LAUNCH_SUCCESSFUL,
	LAUNCH_IGNORING_ERRORS
} launch_result;
char INST_RUNNER[MAX_PATH] = {0};
void RunFichierTrace (char *fmt, ...);
/* Fin SBA 17/04/01 */

void InitHandles (void);
DWORD TestStopEvent (void);
DWORD GetStartupHandle (void);
LONG GetInstancesModules (char *key_name);
LONG RunInstances (void);
LONG StopInstances (void);
LONG WaitEndEvents (void);

LONG ReadRunnerKey (char *name_key);
LONG GetInstances (DWORD index_module);

void CloseAllHandles (BOOL flag_trace);
void IncrementeProcessWorkingSetSize(unsigned long taille_increment);

char * StrTrimLeft( char * string );
char * StrTrimRight( char * string );
char * StrTrim( char * string );
BOOL Error(char *source_file, DWORD line, DWORD code, DWORD code_attendu, BOOL err_win32);
BOOL RUN_FullCreatePath (char *path);

/*------------- DEFINE & MACRO -------------*/

#define MERROR(code, code_attendu, err_win32) Error(__FILE__, __LINE__, code, code_attendu, err_win32)

#define KEY_RUNNER	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					RUN_REG_KEYn_RUNNER

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					MOD_REG_KEYn_MODULES

#define CSR_KEY_DYNAMIC	CSR_REG_KEYn_CSRBASE \
						CSR_REG_KEYn_LANE_BASE \
						CSR_REG_KEYn_DYNAMIC

#define RUN_MAX_INSTANCE	16
#define RUN_MAX_MODULES		30

typedef enum
{
	EVT_STOP,
	EVT_END_STARTUP,

	EVT_NB
}enum_evts;

/*--------------- TYPEDEF: -----------------*/

typedef struct
{
	char mailbox_name[MAX_PATH];
	noyau_bal_id bal_id;
	BOOL running;
}struct_instance;

typedef struct
{
	char name[MAX_PATH];
	char dll_entry_point[MAX_PATH];
	DWORD ignore_run_error;	// SBA 19/01/01
	PMOD_PROC_LANCE	pMODLance;
	PMOD_PROC_ARRET	pMODArret;
	DWORD nb_instance;
	struct_instance	instances[RUN_MAX_INSTANCE];
}struct_module;

/*--------------- VARIABLES: ---------------*/

PRIVATE TRC_EMETTEUR trc; /* Gestion des traces */
PRIVATE struct_module modules[RUN_MAX_MODULES] = {0}; /* Tableau d'instances par module */
PRIVATE DWORD nb_modules, nb_instances;
PRIVATE HANDLE handles[EVT_NB];
PRIVATE BOOL test;

// Libelles codes de sortie des processes (RESPECTER ORDRE DEFINI DANS enum_exit_codes de LWD.H)
char *lib_exit_codes[NB_EXIT_CODE + 1] =
{
	"EXIT_OK",
	"EXIT_KO",
	"EXIT_NO_EXE_REP",
	"EXIT_NO_VIRTUEL",
	"EXIT_NO_INIT",
	"EXIT_PB_CONTEXT",
	"EXIT_NO_ECRAN",
	"EXIT_NO_COMM",
	"EXIT_NO_RTC",
	"EXIT_NO_BAL",
	"EXIT_NO_TACHES",
	"EXIT_PB_SYSTEME",
	"EXIT_NO_LANE_INI",
	"EXIT_NO_DEN_MESS",
	"EXIT_BAD_NUM_CP",
	"EXIT_PB_RESSOURCE",
	"EXIT_NO_RESEAU",

	"EXIT_RUN_ERR_REP_TRACE",
	"EXIT_RUN_ERR_INIT_TRACE",
	"EXIT_RUN_ERR_PARAM",
	"EXIT_RUN_ERR_STOP_EVT",
	"EXIT_RUN_ERR_STARTUP_HANDLE",
	"EXIT_RUN_ERR_INSTANCE_LOAD",
	"EXIT_RUN_ERR_INSTANCE_START",
	"EXIT_RUN_ERR_INSTANCE_STOP",

    "EXIT_CODE_HORS_LIMITE" /* Laisser cette valeur a la fin */
};

// Libelles codes de sortie de l'init des modules (RESPECTER ORDRE DEFINI DANS enum_instance_result de RUN.H)
PRIVATE char *libelle_init[NB_INST_INIT_CODE + 1] = 
{
    "INST_INIT_OK",
    "INST_INIT_ERR_LANCE",
    "INST_INIT_ERR_FICHIER_PARAM",
    "INST_INIT_ERR_PORT",
    "INST_INIT_ERR_FICHIER_DEBUG",
    "INST_INIT_ERR_MAX_INSTANCE",
    "INST_INIT_ERR_REGISTRE",
    "INST_INIT_ERR_CHRONO",
    "INST_INIT_ERR_AUTOMATE",
    "INST_INIT_ERR_EVENT",
    "INST_INIT_ERR_BAL",

    "INST_INIT_ERR_HORS_LIMITE" /* Laisser cette valeur a la fin */
};

// Libelles codes de sortie de l'arret des modules (RESPECTER ORDRE DEFINI DANS enum_instance_result de RUN.H)
PRIVATE char *libelle_arret[NB_INST_ARRET_CODE + 1] = 
{
    "INST_ARRET_OK",
    "INST_ARRET_NOK",
    "INST_ARRET_ERR_RESSOURCE",
    "INST_ARRET_ERR_AUTOMATE",
    "INST_ARRET_ERR_PORT",
    "INST_ARRET_ERR_FICHIER_DEBUG",
    "INST_ARRET_ERR_TACHE",

    "INST_ARRET_ERR_HORS_LIMITE" /* Laisser cette valeur a la fin */
};

/*
 * --------------------------------------------------------------------
 * SYNTAX    : int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
 * PARAMETERS: HINSTANCE hInstance     : 
 *             HINSTANCE hPrevInstance : 
 *             LPSTR lpCmdLine         : 
 *             int nCmdShow            : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
int WINAPI WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow )
{
	DWORD res_trc,i;
	char *p_test, *par_test="/TEST";
	char *p_wsize_incr, *p, *par_wsize_incr="/processworkingsetsizeincr=";
	char param[MAX_PATH] = { 0 };
	unsigned long size_incr;
	char key_name[MAX_PATH] = { 0 };
	LONG result;
	enum_exit_codes exit_code;
    int libelle_code_sortie;
    char *format_sortie = "Module RUN arrete. Code de sortie = 0x%X (%u/%u) : %s";
    DWORD dwLen;
	char TrcErrFilePath[_MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = { 0 };
	char szTmp[MAX_PATH] = { 0 };

	exit_code = EXIT_OK;
    res_trc = ~NO_ERROR;

	NoyGetInstallPath(szAppPath);

	/* Initialiser les handles */
	InitHandles ();

	/*******************************************/
	/* Mise en place du fichier traces/erreurs */
	/*******************************************/

    // chemin traces
    dwLen = sizeof( szTmp );
    if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, KEY_RUNNER, CSR_REG_KEYv_TRACEPATH, 
        szTmp, &dwLen ) != ERROR_SUCCESS )
	{
		printf ("\nRUN : Parametre chemin traces/erreurs [%s] manquant ou inaccessible dans registre.",
            CSR_REG_KEYv_TRACEPATH);
		_getch();
		exit (-1);
	}
	else
	{
		strcpy_s(TrcErrFilePath, sizeof(TrcErrFilePath)-1, szTmp);
		if (strchr(szTmp, ':') == NULL)
			sprintf_s(TrcErrFilePath, sizeof(TrcErrFilePath) - 1, "%s%s", szAppPath, szTmp);

		/* Verifier existence du repertoire de traces/erreurs */
	    if (_access (TrcErrFilePath, 0) != 0)
		    if (errno == ENOENT)
			    if (!RUN_FullCreatePath (TrcErrFilePath))
					exit_code = EXIT_RUN_ERR_REP_TRACE;
	}

	if (exit_code == EXIT_OK)
	{
		/* Initialisation des traces */
		strcat_s(TrcErrFilePath, sizeof(TrcErrFilePath) - 1, FILE_TRCERR);
		res_trc = TRC_Initialise_Trace ("RUN", TrcErrFilePath, TRC_OPT_FICHIER|TRC_OPT_CREER_FICHIER, &trc);

		if (res_trc != NO_ERROR)
			exit_code = EXIT_RUN_ERR_INIT_TRACE;
	}

	if (exit_code == EXIT_OK)
	{
		/*******************************************************/
		/* Verification des parametres de la ligne de commande */
		/*******************************************************/

		/* Verifier si TEST precise en ligne de commande */
		if ((p_test = strstr (lpCmdLine, par_test)) != NULL)
		{
			test = TRUE; /* Memoriser TEST en cours */
			/* supprimer le parametre (TEST) de la ligne de commande */
			i = 0;
			while (i < strlen(par_test))
				p_test[i++] = ' ';
		}
		else
			test = FALSE;

		/* Verifier si WorkingSizeIncrement est precise en ligne de commande */
		strcpy_s(param, sizeof(param) - 1, lpCmdLine);
		_strlwr_s(param, sizeof(param) - 1);		// passage en minuscule pour la recherche
		if ((p_wsize_incr = strstr (param, par_wsize_incr)) != NULL)
		{
			sscanf_s(p_wsize_incr, "%s", param, sizeof(param));
			p = param + strlen(par_wsize_incr);	
			if (sscanf_s(p, "%ld", &size_incr) == 1 )
			{
				// MAJ du ProcessWorkingSetSize du process
				IncrementeProcessWorkingSetSize(size_incr);
				RunFichierTrace ("Parametre %s accepte",param);
			}
			else
				RunFichierTrace ("Parametre %s incorrect",param);

			/* supprimer le parametre (ProcessWorkingSetSize) de la ligne de commande */
			memset(lpCmdLine +(p_wsize_incr-param), ' ', strlen(param));
		}

		/* Supprimer les blancs (espace/tabulation) avant et apres le parametre */
		strcpy_s(key_name, sizeof(key_name) - 1, StrTrim(lpCmdLine));
		strcpy_s(INST_RUNNER, sizeof(INST_RUNNER) - 1, key_name);	// SBA 17/04/01
		if (strlen (key_name) == 0)
		{
			RunFichierTrace ("Parametre obligatoire absent (sous-cle lanceur).");
			exit_code = EXIT_RUN_ERR_PARAM;
		}
	}

	if (exit_code == EXIT_OK)
	{
		/* Test si l'evenement de 'stop application' a ete cree par le lanceur general */
		if (TestStopEvent () != ERROR_SUCCESS)
			exit_code = EXIT_RUN_ERR_STOP_EVT;
	}

	if (exit_code == EXIT_OK)
	{
		/* Recuperer un handle sur le lanceur general ('Startup') */
		if (GetStartupHandle () != ERROR_SUCCESS)
			exit_code = EXIT_RUN_ERR_STARTUP_HANDLE;
	}

	if (exit_code == EXIT_OK)
	{
		/************************************************/
		/* Recuperer les instances des modules a lancer */
		/************************************************/
		result = GetInstancesModules (key_name);

		if (result > 0) /* Si recuperation instances OK */
		{
			/************************/
			/* Lancer les instances */
			/************************/
			if (RunInstances () != LAUNCH_ABORTED)	// SBA 17/04/01
			{
				/*******************************/
				/* Attendre la fin application */
				/*******************************/
				WaitEndEvents ();

				/*************************/
				/* Arreter les instances */
				/*************************/
				if (StopInstances () <= 0)
					exit_code = EXIT_RUN_ERR_INSTANCE_STOP;
			}
			else
			{
				/* Arreter les instances */
				StopInstances ();
				exit_code = EXIT_RUN_ERR_INSTANCE_START;
			}
		}
		else
			exit_code = EXIT_RUN_ERR_INSTANCE_LOAD;
	}

	/*******/
	/* Fin */
	/*******/

	if (!test)
		/* Signaler au lanceur general que l'on s'arrete volontairement */
		if (SetEvent (handles[EVT_STOP]))
        {
            if (res_trc == NO_ERROR) /* Module trace actif */
			    RunFichierTrace ("Fin signalee a 'Startup'.");
        }
		else
			MERROR (GetLastError (), 0, TRUE);

    libelle_code_sortie = exit_code;
    if (libelle_code_sortie > LAST_EXIT_CODE)
        libelle_code_sortie = EXIT_BUTEE;

    if (res_trc == NO_ERROR) /* Module trace actif */
    {
	    RunFichierTrace (format_sortie,
		    exit_code, exit_code, LAST_EXIT_CODE, 
            lib_exit_codes[libelle_code_sortie-FIRST_EXIT_CODE]);

	    /* Fermer tous les handles utilises (trace autorisee) */
	    CloseAllHandles (TRUE);

	    /* Mettre a jour les traces */
	    res_trc = TRC_Vider_Traces (1000);
	    /* Arret des traces */
	    res_trc = TRC_Termine_Trace (trc);
    }
    else /* Module trace en echec ==> afficher sur console (si presente !!) */
    {
        printf (format_sortie,
		    exit_code, exit_code, LAST_EXIT_CODE, 
            lib_exit_codes[libelle_code_sortie-FIRST_EXIT_CODE]);

	    /* Fermer tous les handles utilises (trace interdite) */
	    CloseAllHandles (FALSE);
    }

	return (int)exit_code;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : void InitHandles  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise les handles.
 * --------------------------------------------------------------------
 */
void InitHandles (void)
{
    int i;

    for (i=0; i < EVT_NB; i++)
	    handles[i] = (void *)ERROR_INVALID_HANDLE;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD TestStopEvent  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Teste si l'evenement de Stop est accessible
 * --------------------------------------------------------------------
 */
DWORD TestStopEvent (void)
{
	if (test)
		return ERROR_SUCCESS;

	/* Recuperer un handle sur l'evenement */
	handles[EVT_STOP] = OpenEvent (EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, LWD_EVT_APPLICATION_STOP);

	/* Si on a un acces sur l'evenement ... */
	if (handles[EVT_STOP] != NULL)
		return ERROR_SUCCESS;
	else
		return GetLastError();
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD GetStartupHandle  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recuperer dans le registre (dynamique) le PID de 'Startup'
 *			   Ce PID est necessaire pour obtenir un handle sur ce process 'Startup'.
 *			   Ce handle est necessaire pour surveiller la fin anormale de 'Startup'.
 * --------------------------------------------------------------------
 */
DWORD GetStartupHandle (void)
{
	char *PidValue_name="PID";
	HKEY hkey_dynamic;
	DWORD result, res_key, startup_pid;

	if (test)
		return ERROR_SUCCESS;

	/* Connexion sur la cle dynamique */
	result = REG_Ouvrir (NULL, CSR_REG_KEYi_ROOT, CSR_KEY_DYNAMIC, &hkey_dynamic);

	if (!MERROR(result, ERROR_SUCCESS, TRUE))
	{
		/* Succes connection */

		/* Lecture du PID dans une sous-cle 'Startup' */
		result = REG_Lire_Entier (hkey_dynamic, LWD_REG_KEYn_STARTUP,
			PidValue_name, &startup_pid);

		if (!MERROR(result, ERROR_SUCCESS, TRUE))
		{
			RunFichierTrace ("PID Startup = %lu.", startup_pid);

			/******************************************************************/
			/* Recuperer un handle sur le process 'Startup'                   */
			/* 'SYNCHRONIZE' necessaire pour les fonctions WaitFor...Object() */
			/******************************************************************/

			handles[EVT_END_STARTUP] = OpenProcess (SYNCHRONIZE, TRUE, startup_pid);
			if (handles[EVT_END_STARTUP] == NULL)
			{
				result = GetLastError ();
				MERROR(result, ERROR_SUCCESS, TRUE);
			}

		}

		/* Deconnexion de la cle dynamique */
		res_key = REG_Fermer (hkey_dynamic);

		MERROR(res_key, ERROR_SUCCESS, TRUE);

		/* Si aucun probleme n'a ete detecte avant la fermeture de cle */
		if (result == ERROR_SUCCESS)
			/* On retourne le resultat de la fermeture de cle */
			result = res_key;
		/* Sinon on retourne le probleme detecte avant la fermeture de cle */
	}

	return result;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG GetInstancesModules  ( char *key_name )
 * PARAMETERS: char *key_name : nom de la sous-cle a lire dans cle 'runner'
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
LONG GetInstancesModules (char *key_name)
{
	LONG res_key, result;
	/* Analyse de la cle (passee en parametre) */

	res_key = ReadRunnerKey (key_name);

	RunFichierTrace ("%ld instance(s) repartie(s) sur %ld module(s)",
		nb_instances, nb_modules);

	result = 0;

	if (res_key > RUN_MAX_MODULES)
		RunFichierTrace ("%ld instance(s) repartie(s) sur %ld module(s)",
			nb_instances, nb_modules);
	else
		switch (res_key)
		{
			case 0 :
				RunFichierTrace (
					"Aucune instance a lancer !");
				break;

			case -1 :
				RunFichierTrace (
					"Probleme acces cle !");
				break;

			case -2 :
				RunFichierTrace (
					"Parametre obligatoire pour runner absent ou incoherent!");
				break;

			case -3 :
				RunFichierTrace (
					"Probleme sur une instance!");
				break;

			default :
				if (res_key < 0)
					RunFichierTrace (
						"Erreur inconnue renvoyee par ReadRunnerKey()");
				else
					result = 1;
				break;
		}

	return result;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG ReadRunnerKey  ( char *key_name )
 * PARAMETERS: char *key_name : nom de la cle
 * RETURN    : code d'erreur	-1 = probleme avec cle
 *								-2 = probleme parametre
 *								-3 = probleme avec instances
 *
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Analyser la cle contenant les modules a lancer
 * --------------------------------------------------------------------
 */
LONG ReadRunnerKey (char *key_name)
{
	char key_fullname[MAX_PATH] = {0};
	HKEY hkey_runner;
	DWORD result, res_inst;
	DWORD sname, svalue;
	LONG index, surplus;
	char module_param[MAX_PATH] = { 0 };	// SBA 19/01/01

	strcpy_s(key_fullname, sizeof(key_fullname) - 1, KEY_RUNNER);
	strcat_s(key_fullname, sizeof(key_fullname) - 1, key_name);

	/* Connexion sur la cle du lanceur */
	result = REG_Ouvrir (NULL, CSR_REG_KEYi_ROOT, key_fullname, &hkey_runner);

	if (!MERROR(result, ERROR_SUCCESS, TRUE))
	{
		/* Succes connection */

		/* Recuperation des donnees de chaque entree de la cle */
		/* Ces donnees (chaines) representent une classe de module (AFF, DCP, ...) */
		/* avec leur DLL associee */
/* SBA 19/01/01 : ... ainsi qu'un parametre indiquant si le runner doit s'arreter
					ou bien continuer si une erreur a lieu au lancement de l'un des
					modules de la classe en question */

		index = -1;
		surplus = 0;
		nb_modules = 0;
		nb_instances = 0;
		while (result == ERROR_SUCCESS)
		{
			if (index < (RUN_MAX_MODULES-1))
			{
				index ++;
			}
			else
				surplus ++;
			/* Initialiser nom du module a rien */
			strcpy_s(modules[index].name, sizeof(modules[index].name) - 1, "");
			sname = sizeof(modules[index].name);
			svalue = sizeof(module_param);	// SBA 19/01/01 : ex modules[index].dll_entry_point
			result = REG_Enum_Valeurs_Chaine (hkey_runner, NULL, index+surplus,
				modules[index].name, &sname,
				module_param, &svalue);	// SBA 19/01/01 : ex modules[index].dll_entry_point

// SBA 19/01/01
			strcpy_s(modules[index].dll_entry_point, sizeof(modules[index].dll_entry_point) - 1, "");
			modules[index].ignore_run_error = 0;
			sscanf_s(module_param, "%s | %d", modules[index].dll_entry_point, sizeof(modules[index].dll_entry_point), &(modules[index].ignore_run_error));

			/* Si valeur recuperee */
			if (result != ERROR_NO_MORE_ITEMS)
			{
				if (!MERROR(result, ERROR_SUCCESS, TRUE))
				{
					nb_modules ++; /* MAJ compteur global de modules */
					/* Eliminer les blancs superflus et tracer */
					StrTrim (modules[index].name);
					StrTrim (modules[index].dll_entry_point);

					RunFichierTrace ("Module %lu -> %s (dll=%s) -> %s",
						index+surplus+1,
						modules[index].name, modules[index].dll_entry_point,
						modules[index].ignore_run_error?"ignore_err":"stop_on_err");	// SBA 19/01/01

					if ((strlen (modules[index].name) > 0) && 
						(strlen (modules[index].dll_entry_point) > 0))
					{
						/******************************************/
						/* Recuperer les instances pour ce module */
						/******************************************/

						res_inst = GetInstances (index);

						if ((res_inst < 0) || (res_inst > RUN_MAX_INSTANCE))
						{
							index = -3;
							break;
						}
					}
					else /* Mauvais parametre */
					{
						index = -2;
						break;
					}
				}
				else
				{
					index = -1;
					break;
				}
			}
		}


		/* Deconnexion de la cle du lanceur */
		result = REG_Fermer (hkey_runner);

		if (MERROR(result, ERROR_SUCCESS, TRUE))
			index = -1;
	}
	else
		index = -1;

	return (index+surplus);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG GetInstances  ( DWORD index_module )
 * PARAMETERS: DWORD index_module : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recuperer les informations d'instances a lancer pour 
 *			   un type de module.
 * --------------------------------------------------------------------
 */
LONG GetInstances (DWORD index_module)
{
	char key_name[MAX_PATH] = {0};
	char instance_name[MAX_PATH] = { 0 };
	HKEY hkey_module;
	DWORD result;
	LONG index, surplus;
	DWORD ssize;
	HINSTANCE hinstance;
	DWORD last_error;
	BOOL key_connection = FALSE;

	strcpy_s(key_name, sizeof(key_name) - 1, KEY_MODULE);
	strcat_s(key_name, sizeof(key_name) - 1, modules[index_module].name);

	modules[index_module].nb_instance = 0;

	index = 0;
	surplus = 0; /* Nb d'entrees au-dessus d'un MAX */

	/* Connexion sur une sous-cle de "Modules" */
	result = REG_Ouvrir (NULL, CSR_REG_KEYi_ROOT, key_name, &hkey_module);

	/* Si on ne trouve pas d'entree dans la cle 'module' du registre, alors seule la */
	/* DLL trouvee dans la cle 'runner' est a lancer (il n'y a pas d'autre instance a */
	/* lancer. Ex : Module sans BAL) */
	
	if (result == ERROR_SUCCESS)
		key_connection = TRUE;
	else /* On ignore donc l'erreur de connexion sur la sous-cle 'module' */
		result = ERROR_SUCCESS;

	if (!MERROR(result, ERROR_SUCCESS, TRUE))
	{
		/* Succes connection */

		/* Recuperation des donnees de chaque entree de la cle */
		/* Ces donnees (sous-cles) representent un nom de BAL associe a une instance du module */
		
		while (result == ERROR_SUCCESS)
		{
			/* Init du nom de BAL d'interface (qui est optionnel) */
			strcpy_s(modules[index_module].instances[index].mailbox_name, 
						sizeof(modules[index_module].instances[index].mailbox_name) - 1, "");

			if (key_connection) /* Connexion sur la sous-cle 'module' valide */
			{
				/* Recuperer instance du module */
				ssize = sizeof(modules[index_module].instances[index].mailbox_name);
				result = REG_Enum_Cles (hkey_module, NULL, index+surplus, 
					modules[index_module].instances[index].mailbox_name, &ssize);

				/* Si aucune instance trouvee ... */
				if (result == ERROR_NO_MORE_ITEMS)
				{
					if (index == 0)
					{
						result = ERROR_SUCCESS; /* Ignorer l'erreur car DLL a charger quand meme */
						strcpy_s(instance_name, sizeof(instance_name) - 1, modules[index_module].name);
					}
				}
				else
					strcpy_s(instance_name, sizeof(instance_name) - 1, modules[index_module].instances[index].mailbox_name);
			}
			else /* Pas de connexion sur la cle */
				if (index > 0) /*  et DLL deja memorisee */
					result = ERROR_NO_MORE_ITEMS; /* Simuler plus d'instance a chercher */
				else /* DLL a memoriser */
					/* Nom instance = nom module associe a la DLL */
					strcpy_s(instance_name, sizeof(instance_name) - 1, modules[index_module].name);

			/* Si sous-cle recuperee */
			if (result != ERROR_NO_MORE_ITEMS)
			{
				if (!MERROR(result, ERROR_SUCCESS, TRUE))
				{
					RunFichierTrace ("Instance %lu -> %s",
						index+surplus+1, instance_name);

					modules[index_module].instances[index].running = FALSE;

					/* Une instance de plus pour ce module */
					modules[index_module].nb_instance ++;
					nb_instances ++; /* MAJ compteur global d'instances */
				}

				if (index < (RUN_MAX_INSTANCE-1))
					index ++;
				else
					/* On continue de lire uniquement pour savoir combien d'entrees */
					/* sont en trop (les entrees supplementaires sont stockees dans */
					/* le dernier element du tableau d'instances) */
					surplus ++;
			}
		}

		/* Deconnexion de la cle "Module" */
		if (key_connection)
		{
			result = REG_Fermer (hkey_module);

			if (MERROR(result, ERROR_SUCCESS, TRUE))
				index = -1;
		}

	}
	else
		index = -1;

	/* Si aucune erreur */
	if ((index > 0) && (surplus == 0))
	{
		/* Recuperer les adresses des fonctions de lancement et d'arret */
		if ((hinstance = LoadLibrary (modules[index_module].dll_entry_point)) != NULL)
		{
			modules[index_module].pMODLance = 
				(PMOD_PROC_LANCE)GetProcAddress (hinstance, MOD_PROC_LANCE);
			if (modules[index_module].pMODLance == NULL)
			{
				last_error = GetLastError ();
				index = -2;
			}

			modules[index_module].pMODArret = 
				(PMOD_PROC_ARRET)GetProcAddress (hinstance, MOD_PROC_ARRET);
			if (modules[index_module].pMODArret == NULL)
			{
				last_error = GetLastError ();
				index = -2;
			}
		}
		else // Impossible de charger la DLL
		{
			last_error = GetLastError ();
			index = -2;
		}
	}

	return (index+surplus);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG RunInstances  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lancer les instances lues dans le registre
 * --------------------------------------------------------------------
 */
LONG RunInstances (void)
{
	char key_name[MAX_PATH] = {0};
	LONG result;
	DWORD index_module, index_instance, cpt_instances;
	enum_instance_result res_inst;
	int lib_init_inst;
	BOOL error;
	DWORD cpt_errors;

	cpt_instances = 0;
	index_module = 0;
	error = FALSE;
	cpt_errors = 0;

	while (!error && (cpt_instances + cpt_errors < nb_instances))	// SBA 17/04/01
	{
		index_instance = 0;

		while (!error && (index_instance < modules[index_module].nb_instance))
		{
			/* Cle modules */
			strcpy_s(key_name, sizeof(key_name) - 1, KEY_MODULE);
			/* Sous-cle du module concerne */
			strcat_s(key_name, sizeof(key_name) - 1, modules[index_module].name);
			if (strlen(modules[index_module].instances[index_instance].mailbox_name))
			{
				/* Separateur */
				strcat_s(key_name, sizeof(key_name) - 1, "\\");
				/* Sous-cle ou sont stockes les parametres pour cette instance */
				strcat_s(key_name, sizeof(key_name) - 1, modules[index_module].instances[index_instance].mailbox_name);
			}

			/***************************/
			/* Lancement de l'instance */
			/***************************/
			res_inst = modules[index_module].pMODLance (key_name,
				modules[index_module].instances[index_instance].mailbox_name,
				&modules[index_module].instances[index_instance].bal_id);

			if (res_inst == INST_INIT_OK)
			{
				RunFichierTrace ("Lancement instance %s OK.", key_name);

				modules[index_module].instances[index_instance].running = TRUE;
				cpt_instances ++;
			}
			else
			{
				lib_init_inst = res_inst;
				if (lib_init_inst > LAST_INST_INIT_CODE)
					lib_init_inst = BUTEE_INST_INIT_CODE;

				RunFichierTrace ("RUN_ERREUR : Instance %s non lancee. Erreur = %ld/%u : %s.", 
						key_name, res_inst, LAST_INST_INIT_CODE, libelle_init[lib_init_inst - FIRST_INST_INIT_CODE]);

				cpt_errors ++;

				if (!modules[index_module].ignore_run_error)	// SBA 19/01/01 + 17/04/01
					error = TRUE;
			}

			index_instance ++;	/* Passage a l'instance suivante */
		}

		index_module ++;	/* Passage au module suivant */
	}

	if (cpt_errors == 0)
	{
		result = LAUNCH_SUCCESSFUL;
		RunFichierTrace ("Lancement totalement reussi");
	}
	else if (!error)
	{
		result = LAUNCH_IGNORING_ERRORS;
		RunFichierTrace ("Lancement reussi ... en ignorant certaines erreurs");
	}
	else
	{
		result = LAUNCH_ABORTED;
		RunFichierTrace ("Lancement abandonne !");
	}

	return result;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG WaitEndEvents  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Attend les evenements de fin (Stop application, fin 'Startup)
 * --------------------------------------------------------------------
 */
LONG WaitEndEvents (void)
{
	DWORD object_signaled, evt_signaled;
	LONG result;

	if (!test)
	{
		RunFichierTrace ("Attente evenements ... ");

		object_signaled = WaitForMultipleObjects (EVT_NB, handles, FALSE, INFINITE);

		result = 1;

		if ((object_signaled >= WAIT_OBJECT_0) && (object_signaled < (WAIT_OBJECT_0+EVT_NB)))
		{
			/* Retablir offset a partir de 0 */
			evt_signaled = object_signaled - WAIT_OBJECT_0;

			switch (evt_signaled)
			{
				case EVT_STOP : /* Evenement fin application */

					RunFichierTrace ("Detection evenement 'EVT_STOP'.");
					break;

				case EVT_END_STARTUP : /* Startup a ete ou s'est arrete violemment */

					RunFichierTrace ("Detection evenement 'EVT_END_STARTUP'.");
					break;

				default :

					RunFichierTrace ("Detection evenement inconnu [%lu].", evt_signaled);
					result = 0;
					break;
			}
		}
		else
		{
			if (object_signaled == WAIT_FAILED)
				MERROR (GetLastError (), ERROR_SUCCESS, TRUE);
			RunFichierTrace ("Probleme attente evenements (retour = %lu).", object_signaled);

			result = 0;
		}
	}
	else
	{
		while (1) Sleep (5000);
		result = 1;
	}

	return result;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : LONG StopInstances  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Arreter toutes les instances lancees
 * --------------------------------------------------------------------
 */
LONG StopInstances (void)
{
	LONG result;
	DWORD index_module, index_instance, cpt_instances;
	enum_instance_result res_inst;
	BOOL error;
	int lib_arret_inst;

	result = 0;
	cpt_instances = 0;
	index_module = 0;
	error = FALSE;

	while (cpt_instances < nb_instances)	// SBA 17/04/01
	{
		index_instance = 0;

		while (index_instance < modules[index_module].nb_instance)
		{
			if (modules[index_module].instances[index_instance].running)
			{
				RunFichierTrace ("Demande arret instance %s (module %s)...", 
                    modules[index_module].instances[index_instance].mailbox_name,
                    modules[index_module].name);

				/***********************/
				/* Arret de l'instance */
				/***********************/
				res_inst = modules[index_module].pMODArret (
					modules[index_module].instances[index_instance].bal_id);

				if (res_inst == INST_ARRET_OK)
				{
				    RunFichierTrace ("Arret instance %s OK.",
					    modules[index_module].instances[index_instance].mailbox_name);

					modules[index_module].instances[index_instance].running = FALSE;
				}
				else
				{
					error = TRUE;
					lib_arret_inst = res_inst;
					if (lib_arret_inst > LAST_INST_ARRET_CODE)
						lib_arret_inst = BUTEE_INST_ARRET_CODE;
						
					RunFichierTrace ("RUN_ERREUR : Instance %s mal arretee. Erreur = %ld/%u : %s.",
							modules[index_module].instances[index_instance].mailbox_name,
							res_inst, LAST_INST_ARRET_CODE, libelle_arret[lib_arret_inst - FIRST_INST_ARRET_CODE]);
				}
			}

			cpt_instances ++;
			index_instance ++;	/* Passage a l'instance suivante */
		}

		index_module ++;	/* Passage au module suivant */
	}

	if ((cpt_instances == nb_instances) && !error)
		result = 1;

	return result;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void CloseAllHandles  ( BOOL flag_trace )
 * PARAMETERS: flag_trace : TRUE = trace autorisee, FALSE = trace interdite
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme tous les handles.
 * --------------------------------------------------------------------
 */
void CloseAllHandles (BOOL flag_trace)
{
	int i;

	for (i=0;i<EVT_NB;i++)
		if (handles[i] != (HANDLE)ERROR_INVALID_HANDLE)
			if (!CloseHandle (handles[i]))
            {
                if (flag_trace)
				    RunFichierTrace (
					    "Probleme pour fermer handles[%lu]. Erreur = %lu", i, GetLastError ());
            }
			else
				handles[i] = (HANDLE)ERROR_INVALID_HANDLE;
}


/*
 * -----------------------------------------------------------------
 * SYNTAX    : char * StrTrimLeft( char * string )
 * PARAMETERS: string : the string to modify
 * RETURN    : string
 * -----------------------------------------------------------------
 * OVERVIEW  : Remove all beginning characters
 *             smaller than the space.
 * -----------------------------------------------------------------
 */
char * StrTrimLeft( char * string )
{
	DWORD i,j;
	i = 0;
	j = 0;
	while ( string[i] != '\0' && string[i] <= ' ')
		i++;
	while ( string[i] != '\0' )
	{
		string[j] = string[i];
		i++;
		j++;
	}
	string[j] = '\0';

	return string;
}

/*
 * -----------------------------------------------------------------
 * SYNTAX    : char * StrTrimRight( char * string )
 * PARAMETERS: string : the string to modify
 * RETURN    : string
 * -----------------------------------------------------------------
 * OVERVIEW  : Remove all ending characters
 *             smaller than the space.
 * -----------------------------------------------------------------
 */
char * StrTrimRight( char * string )
{
	DWORD j;
	j = (DWORD)strlen( string );

	while ( j > 0 )
	{
		j--;
		if ( string[j] > ' ' )
		{
			j++;
			break;
		}
		string[j] = '\0';
	}
	string[j]='\0';
	return string;
}


/*
 * -----------------------------------------------------------------
 * SYNTAX    : char * StrTrim( char * string )
 * PARAMETERS: string : the string to modify
 * RETURN    : string
 * -----------------------------------------------------------------
 * OVERVIEW  : Remove all ending characters
 *             smaller than the space.
 * -----------------------------------------------------------------
 */
char * StrTrim( char * string )
{
	return StrTrimRight( StrTrimLeft( string ) );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL RUN_FullCreatePath  ( char *path )
 * PARAMETERS: char *path   : arborescence a creer : CE PARAMETRE NE PEUT EXCEDER _MAX_PATH
 * RETURN    : TRUE si OK, FALSE sinon
 *			   Le parametre <path> n'est pas modifie par cette fonction
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une arborescence complete
 * REMARQUE  : Cette fonction accepte les creations sur machine distante
 *			   a condition evidemment que les droits soient autorises sur
 *			   cette machine distante.
 * EXEMPLES  : result = RUN_FullCreatePath ("..\dir1\dir2");
 *			   result = RUN_FullCreatePath ("c:\dir1\dir2\dir3");
 *			   result = RUN_FullCreatePath ("\\ordinateur_distant\partage\dir1\dir2");
 *			   result = RUN_FullCreatePath ("\\ordinateur_distant\partage\nom long ok\dir");
 * REMARQUE 2 : Cette fonction est une copie de FIC_FullCreatePath() [Fonct_c.dll de OUTILS_C]
 *				C'est une copie car startup doit rester independant du noyau (OUTILS_C utilise 
 *              le noyau pour ExitBad()).
 * --------------------------------------------------------------------
 */
BOOL RUN_FullCreatePath (char *path)
{
	char *token;
	char *next_token;
	char full_path[_MAX_PATH] = { 0 }, copy_path[_MAX_PATH] = { 0 }, build_path[_MAX_PATH] = { 0 };
	char *slash="/", *backslash = "\\";
	char *disk_sep = ":";
	char *machine_sep = "\\\\";
	char *sep = "\\/";

	/* Test coherence parametre */
	if (path == NULL)
		return FALSE;
	if (strlen (path) > _MAX_PATH)
		return FALSE;

	/* Recopier la chaine passee en parametre pour ne pas la modifier */
	strcpy_s(copy_path, sizeof(copy_path) - 1, path);

	/* Remplacer les eventuels '/' par des '\' */
	/* Le plupart des fonctions de manipulation de repertoires interpretent correctement le '/' */
	/* mais pour garantir un bon comportement, on effectue le remplacement */
	token = copy_path;
	while ((token = strstr (copy_path, slash)) != NULL)
	{
		*token = *backslash;
		token ++;
	}

	/* Transformer l'arborescence passee en parametre en chemin absolu */
	/* C'est-a-dire avec nom de machine ou unite de disque en premier */
    if (_fullpath(full_path, copy_path, _MAX_PATH) == NULL)
        return FALSE;

	/* Initialiser l'arborescence a creer */
	strcpy_s(build_path, sizeof(build_path) - 1, "");
	/* Verifier si nom de machine inclus dans l'arborescence a construire */
	token = strstr (copy_path, machine_sep);
	/* Si ce n'est pas le cas ... */
	if (token == NULL)
		/* Verifier si unite de disque inclus dans l'arborescence a construire */
		token = strstr (copy_path, disk_sep);
	else /* Arborescence avec nom de machine */
		/* Completer l'arborescence a creer (avec identifiant machine ("\\")) */
		strcat_s(build_path, sizeof(build_path) - 1, machine_sep);

	/* Si on a trouve le nom de machine ou l'unite de disque */
	/* Ce qui doit etre le cas apres l'appel a _fullpath(...) */
	if (token != NULL)
	{
		/*****************************************************************************************/
		/* Attention on utilise la fonctionnalite de strtok() qui ignore les caracteres de dedut */
		/* de chaine s'ils font partis des caracteres recherches                                 */
		/* Exemple avec nom de machine  : strtok ("\\\\machine_name\\shared_dir\\dir1", "\\")==> */
		/*		strtok() renvoie "machine_name" (elle ignore les 2 '\' de debut de chaine        */
		/* Exemple avec unite de disque : strtok ("c:\\dir1\\dir2", "\\") ==>                    */
		/*		strtok() renvoie "c:"                                                            */
		/* Ainsi dans notre cas, un seul appel a strtok() nous donne soit :                      */
		/*	- le nom de machine                                                                  */
		/*	- l'unite de disque                                                                  */
		/*****************************************************************************************/
		token = strtok_s(copy_path, backslash, &next_token);

		/* Si aucun nom de machine ou unite de disque */
		if (token == NULL)
			/* Rien a creer */
			return FALSE;
	}
	else /* Ni nom de machine, ni unite de disque */
		return FALSE;

	/* Completer l'arborescence a creer (avec nom machine ou unite de disque) suivi de '\' */
	strcat_s(build_path, sizeof(build_path) - 1, token);
	strcat_s(build_path, sizeof(build_path) - 1, backslash);

    // Boucle de création des sous-répertoires (si inexistant) : critere de recherche = '\' */
	while ((token = strtok_s(NULL, backslash, &next_token)) != NULL)
    {
		/* Completer l'arborescence a creer (avec sous-repertoire trouve) */
		strcat_s(build_path, sizeof(build_path) - 1, token);
		/* Si ce repertoire n'existe pas deja */
        if (_access(build_path, 0) != 0)
			/* Tenter de le creer */
            if(!CreateDirectory(build_path, NULL))
                return FALSE; /* Erreur creation */

		/* Completer l'arborescence a creer (avec '\' entre chaque sous-repertoire trouve) */
		strcat_s(build_path, sizeof(build_path) - 1, backslash);
    }

    return TRUE;
}
        
/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void IncrementeProcessWorkingSetSize(unsigned long taille_increment)
 * PARAMETRES: taille_increment : increment (en octet) du working set size du process
 * RETOUR    : aucun
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cette fonction incrémente la taille du working set du process
 * --------------------------------------------------------------------
 * $F_FCTN
 */
void IncrementeProcessWorkingSetSize(unsigned long taille_increment)
{
    HANDLE hprocess;
    HANDLE htoken;
    TOKEN_PRIVILEGES PreviousState;
    TOKEN_PRIVILEGES NewState;
    LUID pLuid;
    long ReturnLength;
	SIZE_T ProcessWorkingSetSizeMin;
	SIZE_T ProcessWorkingSetSizeMax;

    if ( taille_increment == 0 )
        return;

    // Recuperation du handle du process
    hprocess = GetCurrentProcess();

    // Ouverture d'un handle sur token, avec les droits de modification
    // des privileges
    OpenProcessToken( hprocess,
                      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                      &htoken );

    // Recuperation du LUID du privilege SE_INC_BASE_PRIORITY_NAME
    LookupPrivilegeValue( NULL,
                          SE_INC_BASE_PRIORITY_NAME,
                          &pLuid );

    // Initialisation des parametres pour pouvoir modifier l'etat du
    // privilege dont on vient de recuperer le LUID
    NewState.PrivilegeCount = 1;
    NewState.Privileges->Luid = pLuid;
    NewState.Privileges->Attributes = SE_PRIVILEGE_ENABLED;

    // Modification de l'etat du privilege, et recuperation de l'ancien
    // etat du privilege
    AdjustTokenPrivileges( htoken,
                           FALSE,
                           &NewState,
                           sizeof(PreviousState),
                           &PreviousState,
                           &ReturnLength );

    // Recuperation des parametres actuels du process working set
    GetProcessWorkingSetSize( hprocess,
                              &ProcessWorkingSetSizeMin,
                              &ProcessWorkingSetSizeMax );

    // Modification des parametres actuels du process working set
    SetProcessWorkingSetSize( hprocess,
                              ProcessWorkingSetSizeMin+taille_increment,
                              ProcessWorkingSetSizeMax+taille_increment );

    // Restauration de l'ancien etat du privilege
    AdjustTokenPrivileges( htoken,
                           FALSE,
                           &PreviousState,
                           0,
                           NULL,
                           NULL );

    // Fermeture du handle
    CloseHandle( htoken );
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL Error(char *source_file, DWORD line, DWORD code, DWORD code_attendu,
 *                        BOOL err_win32)
 * PARAMETRES: source_file : nom du fichier source
 *			   line : numero de ligne du source ou est appelee cette fonction
 *			   code : code d'erreur 
 *			   code_attendu : code attendu par l'appelant
 *			   err_win32 : TRUE = erreur WIN32, FALSE = autre
 * RETOUR    : Booleen : FALSE = pas d'erreur, TRUE = erreur
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cette fonction compare le code d'erreur avec le code attendu
 *			   et enregistre des traces si les codes ne sont pas egaux.
 *			   Si erreur Win32 indiquee, alors formatage traces a partir du 
 *			   code d'erreur.
 *			   Cette fonction doit etre appelee via la macro MERROR(...).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL Error(char *source_file, DWORD line, DWORD code, DWORD code_attendu, BOOL err_win32)
{
	if (code != code_attendu)
	{
		RunFichierTrace ("Error %s(%lu).", source_file, line);
		if (err_win32)
			TRC_Direct_Trace_Erreur_Win32 (trc, TRC_OPT_MASK, code);

		return TRUE;
	}

	return FALSE;
}


// SBA 17/04/01
PRIVATE void RunFichierTrace (char *fmt, ...)
{
	va_list args;
	char string[500] = {0};

	va_start (args,fmt);
	vsprintf_s(string, sizeof(string)-1, fmt, args);

	TRC_Direct_Trace_Texte (trc, TRC_OPT_MASK, "[%s] %s", INST_RUNNER, string);

	va_end (args);
}

/*---------------------------- END OF FILE -------------------------*/
