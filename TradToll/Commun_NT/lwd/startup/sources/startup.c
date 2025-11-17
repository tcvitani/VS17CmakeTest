/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : LWD (Launcher/Watch Dog)
 * FICHIER    : STARTUP.C
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

#define _WIN32_WINNT 0x0501

#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <stdlib.h>
#include <errno.h>
#include <noyau.h>

#include <csrlc32.h>

#include <reg.h>
#include <trc.h>
#include <lwd.h>

#include "loc_lwd.h"

#include <psapi.h>
#include <Windows.h>


/**********/
/* Define */
/**********/

#define MAX_PROGRAM	32

#define FILE_TRCERR	"\\_LWD.trc"
#define FILE_TRCRES	"\\_LWD_RES.trc"

#define MERROR(code, code_attendu, err_win32) Error(__FILE__, __LINE__, code, code_attendu, err_win32)
#define KEY_STARTUP_CONFIG	CSR_REG_KEYn_CSRBASE \
							CSR_REG_KEYn_LANE_BASE \
							CSR_REG_KEYn_CONFIG \
							LWD_REG_KEYn_STARTUP

#define CSR_KEY_DYNAMIC	CSR_REG_KEYn_CSRBASE \
							CSR_REG_KEYn_LANE_BASE \
							CSR_REG_KEYn_DYNAMIC

// Use to convert bytes to MB
#define DIV				1048576
#define DIV2			1024
#define WIDTH			4
#define WIDTH2			8
#define LINE_LEN		40

typedef struct  
{
	MEMORYSTATUSEX			statex;
	PERFORMACE_INFORMATION	pi;
}
struct_sysres;

/**************/
/* Prototypes */
/**************/
DWORD UpdateRegisterWithPid (void);
DWORD ReadStartupKey (void);
DWORD LaunchApplication (DWORD nb_exe);
DWORD WatchApplication (DWORD nb_exe);
DWORD StopApplication (DWORD nb_exe);
HANDLE LaunchProgram (CHAR *program, DWORD priority );
void KillProcess (DWORD index);
HANDLE CreateEvt (char *);
void ResetAndCloseEvt (HANDLE hEvt);
BOOL Error(char *source_file, DWORD line, DWORD code, DWORD code_attendu, BOOL err_win32);
BOOL LWD_FullCreatePath (char *path);

void LWD_TraceResources(DWORD nb_exe);
void LWD_TraceTotalMemoryInfo(void);
void LWD_TraceProcessInfo (DWORD dwNbProcesses);
BOOL AlignTotalTraceLine(char *pszDescription, DWORDLONG dwlValue, char *pszValueUnit, DWORD dwLineLen, OUT char *pszLine, DWORD dwLineSize, BOOL bDelta, DWORDLONG dwlOldValue);
BOOL AlignProcessTraceLine(char *pszName, DWORD dwMaxLen, char *pszUnit, OUT char *pszLine, DWORD dwLineSize, PROCESS_MEMORY_COUNTERS *pPmc, PROCESS_MEMORY_COUNTERS *pOld);
BOOL AddNewMonitoringItem(DWORD dwMaxLen, char *pszUnit, OUT char *pszLine, DWORD dwLineSize, int iValue, DWORD dwItemDist);
DWORD GetLongestProcessName(DWORD dwNbProcesses);
void TraceProcessTableHeader(DWORD dwMax);
void TraceTotalTableHeader(void);



/**********************/
/* Variables globales */
/**********************/

TRC_EMETTEUR trc; /* Gestion des traces */
TRC_EMETTEUR trc_res; /* resource trace */

/* Tableaux stockant les programmes(et handles) a executer et a surveiller */
char program[MAX_PROGRAM][256];
BOOL exit_code_test[MAX_PROGRAM];
DWORD priority_class[MAX_PROGRAM]; // NBL 17/05/2002
DWORD affinity_bitmask[MAX_PROGRAM];

/* '+1' reserved for evenement 'END' */
// '+2' : 1st for EvtEND, 2nd for waitable timer
HANDLE hprogram[MAX_PROGRAM+2], hEvtEnd, hEvtStop, hTimer1;

BOOL fin; /* Pour signaler fin programme */
DWORD end_timeout, sleep_between_2_launch, delay_between_processes;
DWORD resource_delay_sec;

char szAppPath[MAX_PATH];

BOOL bFirst	= TRUE;
PROCESS_MEMORY_COUNTERS	sProcMemCntOld[MAX_PROGRAM+2] = {0};
struct_sysres sysResOld = {0};


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

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : int main (INT Argc, CHAR *Argv[])
 * PARAMETRES: Argc : nombre d'argument(s) passe(s) sur la ligne de commande
 *             Argv[] : Argument(s) passe(s) sur la ligne de commande
 * RETOUR    : code de sortie
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cette fonction est la fonction principale du  process.
 *			   Elle est chargee :
 *				- d'initialiser les traces
 *				- de lire dans la cle 'startup' les parametres et programmes a lancer
 *				- de lancer ces programmes
 *				- de surveiller ces programmes
 *				- de relancer ces programmes si l'un deux s'arrete et que 
 *				  l'evenement 'fin normale' n'a pas ete signale.
 *				- de s'arreter lorsque l'evenenement 'fin normale' lui a ete donne et que
 *				  tous les process aient ete arretes
 * --------------------------------------------------------------------
 * $F_FCTN
 */


int __cdecl main (INT Argc, CHAR *Argv[])
{
	DWORD res_trc, res_pid, res_key, nb_programs, result, nb_launched;
    HANDLE event;
    DWORD dwLen;
    char TrcErrFilePath[_MAX_PATH];
	char TrcResFilePath[_MAX_PATH];
    char szTmp[MAX_PATH];
	// MFR start 2011/2/4
	char	szKey[MAX_PATH] = { 0 };
	// MFR end


	char *field_no_console="CloseConsole";
	DWORD no_console;

	// get install folder
	// MFR start 2011/2/4
//	NoyGetInstallPath(szAppPath);
	
	sprintf_s(szKey, sizeof(szKey)-1, "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);
	
	dwLen = sizeof(szAppPath);
	if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, szKey, NOYAU_REPERTOIRE_APP, szAppPath, &dwLen ) != ERROR_SUCCESS )
	{
		strcpy_s(szAppPath, sizeof(szAppPath) - 1, "c:");
	}
	// MFR end

	/*******************************************/
	/* Mise en place du fichier traces/erreurs */
	/*******************************************/


	/* Fermeture ou non de la console  */
	/* Connexion sur la cle "Startup" (configuration) */
	no_console=0;
	result = REG_Lire_Entier (CSR_REG_KEYi_ROOT, KEY_STARTUP_CONFIG, field_no_console, &no_console);
	if(MERROR(result, ERROR_SUCCESS, TRUE))
		no_console=0;

	if(no_console)
		FreeConsole();

    // chemin traces
    dwLen = sizeof( TrcErrFilePath );
    if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, KEY_STARTUP_CONFIG, CSR_REG_KEYv_TRACEPATH, 
        szTmp, &dwLen ) != ERROR_SUCCESS )
	{
		printf ("\nLWD : Parametre chemin traces/erreurs [%s] manquant ou inaccessible dans registre.",
            CSR_REG_KEYv_TRACEPATH);
		_getch();
		exit (-1);
	}

	// build full trace path
	strcpy_s(TrcErrFilePath, sizeof(TrcErrFilePath)-1, szTmp);
	
	if (strchr(szTmp, ':') == NULL)
		sprintf_s(TrcErrFilePath,  sizeof(TrcErrFilePath) - 1, "%s%s", szAppPath, szTmp);

	strcpy_s(TrcResFilePath, sizeof(TrcResFilePath) - 1, TrcErrFilePath);
 
	/* Verifier existence du repertoire de traces/erreurs */
	if (_access (TrcErrFilePath, 0) != 0)
		if (errno == ENOENT)
			if (!LWD_FullCreatePath (TrcErrFilePath))
			{
				printf ("\nProbleme pour creer le repertoire de traces/erreurs.");
				_getch();
				exit (-1);
			}

	/* Initialisation des traces */
	strcat_s(TrcErrFilePath, sizeof(TrcErrFilePath) - 1, FILE_TRCERR);
	res_trc = TRC_Initialise_Trace ("LWD", TrcErrFilePath, 
        TRC_OPT_FICHIER|TRC_OPT_CREER_FICHIER|TRC_OPT_CONSOLE, &trc);

	if (res_trc != NO_ERROR)
	{
		printf ("\nProbleme pour initialiser les traces.");
		_getch();
		exit (-1);
	}

	// initialization of resource trace
	strcat_s(TrcResFilePath, sizeof(TrcResFilePath) - 1, FILE_TRCRES);
	res_trc = TRC_Initialise_Trace ("LWD_RES", TrcResFilePath, 
        TRC_OPT_FICHIER|TRC_OPT_CREER_FICHIER, &trc_res);
	
	if (res_trc != NO_ERROR)
	{
		printf ("\nProbleme pour initialiser les traces(res).");
		_getch();
		exit (-1);
	}
	

    /* Tester si une instance de STARTUP n'est pas deja chargee */
	if ((event = OpenEvent (EVENT_MODIFY_STATE, FALSE, LWD_EVT_APPLICATION_STOP)) != NULL)
    {
        CloseHandle (event);

		res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK,
			"Startup deja en execution !");

	    /* Mettre a jour les traces */
	    res_trc = TRC_Vider_Traces (1000);
	    /* Arret des traces */
	    res_trc = TRC_Termine_Trace (trc);
		res_trc = TRC_Termine_Trace (trc_res);


		exit (-1);
    }

	/* Enregistrer le PID dans le registre (pour les lanceurs de modules) */
	res_pid = UpdateRegisterWithPid ();
	if (res_pid != ERROR_SUCCESS)
	{
		res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK,
			"Erreur %lu lors de l'enregistrement du PID dans le registre.", res_pid);

	    /* Mettre a jour les traces */
	    res_trc = TRC_Vider_Traces (1000);
	    /* Arret des traces */
	    res_trc = TRC_Termine_Trace (trc);
		res_trc = TRC_Termine_Trace (trc_res);

		exit (-1);
	}

	/*********************/
	/* Lecture de la cle */
	/*********************/

	/* (recuperation des programmes (a lancer et a surveiller) ou erreur) */
	res_key = ReadStartupKey ();

	/* Si le nombre de programme(s) lu(s) dans la cle est coherent */
	if ((res_key > 0) && (res_key <= MAX_PROGRAM))
	{
		nb_programs = res_key;
		fin = FALSE;
		while (!fin)
		{
			/****************************/
			/* Lancement des programmes */
			/****************************/

			nb_launched = LaunchApplication (nb_programs);

			if (nb_launched > 0)

				/*******************************/
				/* Surveillance des programmes */
				/*******************************/

				while ((result = WatchApplication (nb_launched)) == 0);
			else
				/* Fin du programme */
				fin = TRUE;
		}


		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Fin de 'Startup'!");
	}
	else
	{
		if ((LONG)res_key >= 0)
			if (res_key == 0)
				res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, "Aucun programme a executer !");
			else
				res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, "Trop de programmes a executer (%lu/%lu) !", 
					res_key, (DWORD)MAX_PROGRAM);
		else
			switch ((LONG)res_key)
			{
				case -1 :
					res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, 
						"Probleme acces cle !");
					break;

				case -2 :
					res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, 
						"Parametre obligatoire pour startup absent ou incoherent!");
					break;

				case -3 :
					res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, 
						"Parametre programme absent ou incoherent!");
					break;

				default :
					res_trc = TRC_Trace_Texte (trc, TRC_OPT_MASK, 
						"Erreur inconnue renvoyee par ReadStartupKey()");
					break;
			}
//		_getch();
	}	

	/*******/
	/* Fin */
	/*******/

	/* Mettre a jour les traces */
	res_trc = TRC_Vider_Traces (1000);
	/* Arret des traces */
	res_trc = TRC_Termine_Trace (trc);
	res_trc = TRC_Termine_Trace (trc_res);

	return 0;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD UpdateRegisterWithPid  ( void )
 * PARAMETERS: void : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Enregistrer le Process ID de 'Startup' dans la cle dynamique
 *			   de CSR.
 *			   Ce PID servira aux lanceurs de modules pour surveiller une fin
 *			   anormale de 'Startup'.
 * --------------------------------------------------------------------
 */
DWORD UpdateRegisterWithPid (void)
{
	char *PidValue_name="PID";
	DWORD result, pid;
	char key[MAX_PATH] = {0};

	sprintf_s(key, sizeof(key)-1, "%s%s", CSR_KEY_DYNAMIC, LWD_REG_KEYn_STARTUP);

	/* Ecriture du PID dans une sous-cle 'Startup' */
	result = REG_Ecrire_Entier (CSR_REG_KEYi_ROOT, key,
		PidValue_name, pid=GetCurrentProcessId());

	if (!MERROR(result, ERROR_SUCCESS, TRUE))
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "PID Startup = %lu.", pid);

	return result;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD ReadStartupKey (void)
 * PARAMETRES: aucun
 * RETOUR    : nombre d'entrees(sous-cles) lues dans la cle ou 
 *             -1 si probleme acces cle
 *             -2 si parametre de config 'startup' absent ou incoherent
 *			   -3 si parametre de programme a lancer absent ou incoherent
 * --------------------------------------------------------------------
 * VARIABLES : index, hkey_startup, program
 * --------------------------------------------------------------------
 * ROLE      : Lit dans la cle 'startup' les process a lancer via des sous-cles
 *				- Connexion sur la cle 'startup'
 *				- Recuperation des parametres propre a 'startup'
 *				- Lecture et stockage des entrees de chaque sous-cle
 *				- Deconnexion de la cle 'startup'
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD ReadStartupKey (void)
{
	DWORD result, index, surplus, ignore;
	HKEY hkey_startup;
	char souscle[256];
	DWORD ExitCodeTest, DoNotLaunch, dwAffinity;
	DWORD ssize;
    BOOL flag_launch;
	char *field_end_timeout="end_timeout_ms";
    char *field_sleep_between_2_launch="sleep_between_2_launch_ms";
	char *field_exe="exe";
	char *field_exit_code_test = "exit_code_test";
	char *field_flag_do_not_launch = "do_not_launch";
	char *field_priority_class = "priority_class";  // NBL 17/05/2002
	UCHAR szTmp[MAX_PATH];
	char *field_delay_between_processes="delay_between_processes_ms";
	char *pszAffinityBitmask="affinity_bit_mask";
	char *field_resource_delay_sec = "resource_delay_sec";

	index = 0;
	surplus = 0; /* Nb de programmes au-dessus de MAX_PROGRAM */
    ignore = 0;  /* Nb de programmes a ignorer (a ne pas lancer) */

	/* Connexion sur la cle "Startup" (configuration) */
	result = REG_Ouvrir (NULL, CSR_REG_KEYi_ROOT, KEY_STARTUP_CONFIG, &hkey_startup);

	if (!MERROR(result, ERROR_SUCCESS, TRUE))
	{
		/* Succes connection */


		/* Recuperation parametre(s) */
		/* Timeout pour fin application */
		result = REG_Lire_Entier (hkey_startup, NULL, field_end_timeout, &end_timeout);

		if (MERROR(result, ERROR_SUCCESS, TRUE))
			index = -2;

		/* Delai entre 2 lancements d'apllication  */
		result = REG_Lire_Entier (hkey_startup, NULL, field_sleep_between_2_launch, &sleep_between_2_launch);

		if (MERROR(result, ERROR_SUCCESS, TRUE))
			index = -2;

		if (REG_Lire_Entier (hkey_startup, NULL, field_delay_between_processes, &delay_between_processes) != ERROR_SUCCESS)
		{
			delay_between_processes = 1;
//			TRC_Trace_Texte (trc, TRC_OPT_MASK, "Missing Reg value [%s] : %s. The value is set to 1 by default", hkey_startup, field_delay_between_processes);	// MFR Trying to trace HKEY instead of key name
			TRC_Trace_Texte (trc, TRC_OPT_MASK, "Missing Reg value [%s] : %s. The value is set to 1 by default", KEY_STARTUP_CONFIG, field_delay_between_processes);
		}

		/* delay between taking system resources */
		if (REG_Lire_Entier (hkey_startup, NULL, field_resource_delay_sec, &resource_delay_sec) != ERROR_SUCCESS)
		{
			resource_delay_sec = 0;
			TRC_Trace_Texte (trc_res, TRC_OPT_MASK, "Missing Reg value [%s] : %s. The value is set to 0 by default", KEY_STARTUP_CONFIG, field_resource_delay_sec);
		}
		else
		{
			TRC_Trace_Texte (trc_res, TRC_OPT_MASK, "Resource delay set to %d sec", resource_delay_sec);
		}
		
		/* Recuperation des donnees de chaque entree de la cle */
		/* Ces donnees (chaines) representent un nom d'executable a charger en memoire */
		/* (avec eventuellement des parametres) */
		
		while (result == ERROR_SUCCESS)
		{
			ssize = sizeof(souscle);
			result = REG_Enum_Cles (hkey_startup, NULL, index+surplus+ignore, souscle, &ssize);

			/* Si sous-cle recuperee */
			if (result != ERROR_NO_MORE_ITEMS)
			{
				if (!MERROR(result, ERROR_SUCCESS, TRUE))
				{
					TRC_Trace_Texte (trc, TRC_OPT_MASK, "Sous-cle %s (%lu)", souscle, index+surplus+ignore+1);

					ssize = sizeof(program[index]);
					/* Recuperer l'entree (info exe) dans la sous-cle */
					result = REG_Lire_Chaine (hkey_startup, souscle, field_exe, 
						szTmp, &ssize);

					sprintf_s(program[index], sizeof(program[index]) - 1, "\"%s%s", szAppPath, szTmp);

					if (result == ERROR_SUCCESS)
					{
						TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : %s", field_exe, program[index]);

						// getting affinity bitmask
						if (REG_Lire_Entier (hkey_startup, souscle, pszAffinityBitmask, &dwAffinity) != ERROR_SUCCESS)
						{
							TRC_Trace_Texte (trc, TRC_OPT_MASK, "Registry value %s is invalid or missing for %s ", pszAffinityBitmask, souscle);
							affinity_bitmask[index] = 0;
						}
						else
						{
							affinity_bitmask[index] = dwAffinity;
							TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : %lu", pszAffinityBitmask, dwAffinity);
						}

						/* Recuperer l'entree (booleen 'exit_code_test') dans la sous-cle */
						result = REG_Lire_Entier (hkey_startup, souscle, field_exit_code_test,
							&ExitCodeTest);

						if (result == ERROR_SUCCESS)
						{
						    TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : %lu", field_exit_code_test, ExitCodeTest);

						    /* Recuperer l'entree (booleen 'do_not_launch') dans la sous-cle */
							result = REG_Lire_Entier (hkey_startup, souscle, field_flag_do_not_launch,
							&DoNotLaunch);

                            if (result == ERROR_SUCCESS)
                            {

							    /* Information 'do_not_launch' : doit valoir TRUE ou FALSE */
							    if (DoNotLaunch == 1)
                                {
								    flag_launch = FALSE;
                                }
							    else
								    if (DoNotLaunch == 0)
									    flag_launch = TRUE;
								    else /* ni FALSE ni TRUE ! */
									    flag_launch = TRUE;
                            }
                            else /* Parametre non trouve mais non obligatoire */
                            {
                                result = ERROR_SUCCESS; /* Ignorer l'erreur */
							    flag_launch = TRUE;     /* Valeur par defaut */
                                DoNotLaunch = 0; /* Default (=FALSE) */
                            }

							TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : %lu", field_flag_do_not_launch, DoNotLaunch);

                            // NBL 17/05/2002: Début
                            if ( REG_Lire_Entier( hkey_startup, souscle, field_priority_class, &priority_class[index] ) != ERROR_SUCCESS )
                            {
                                TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : Absente, utilisation de la priorité par défaut", field_priority_class );
                                priority_class[index] = 0;
                            }
                            else
                            {
                                if ( ( priority_class[index] != NORMAL_PRIORITY_CLASS   ) &&
                                     ( priority_class[index] != HIGH_PRIORITY_CLASS     ) &&
                                     ( priority_class[index] != IDLE_PRIORITY_CLASS     ) &&
                                     ( priority_class[index] != REALTIME_PRIORITY_CLASS ) )
                                {
                                    TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : Erreur de valeur, utilisation de la priorité par défaut", field_priority_class );
                                    priority_class[index] = 0;
                                }
                                else
                                {
                                    TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s : %s", 
                                        priority_class[index] == NORMAL_PRIORITY_CLASS   ? "NORMAL_PRIORITY_CLASS"   :
                                        priority_class[index] == HIGH_PRIORITY_CLASS     ? "HIGH_PRIORITY_CLASS"     :
                                        priority_class[index] == IDLE_PRIORITY_CLASS     ? "IDLE_PRIORITY_CLASS"     :
                                        priority_class[index] == REALTIME_PRIORITY_CLASS ? "REALTIME_PRIORITY_CLASS" :
                                                                                           "ERREUR" );
                                }
                            }

							/*******************************************************/
							/* Verification des informations lues dans la sous-cle */
							/*******************************************************/

							/* Information 'exe' */
							/* Si c'est une entree invalide (elle doit contenir au moins un caractere */
							/* encadre par "" */
							if (strlen (program[index]) < 3)
							{
								/* Signaler erreur */
								index = -3;
								/* Arreter de lire la cle */
								break;
							}
							/* Information 'exit_code_test' : doit valoir TRUE ou FALSE */
							if (ExitCodeTest == 1)
								exit_code_test[index] = TRUE;
							else
								if (ExitCodeTest == 0)
									exit_code_test[index] = FALSE;
								else /* ni FALSE ni TRUE ! */
								{
									/* Signaler erreur */
									index = -3;
									/* Arreter de lire la cle */
									break;
								}

							if (index < (MAX_PROGRAM-1))
                            {
                                if (flag_launch)
								    index ++;
                                else
                                    ignore ++;
                            }
							else
								/* On continue de lire uniquement pour savoir combien d'entrees */
								/* sont en trop (les entrees supplementaires sont stockees dans */
								/* le dernier element du tableau de programmes */
								surplus ++;
						}
						else
						{
							TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s introuvable", field_exit_code_test);
							/* Signaler erreur */
							index = -3;
							/* Arreter de lire la cle */
							break;
						}
					}
					else
					{
						TRC_Trace_Texte (trc, TRC_OPT_MASK, "Entree %s introuvable", field_exe);
						/* Signaler erreur */
						index = -3;
						/* Arreter de lire la cle */
						break;
					}
				}
			}
		}

		/* Deconnexion de la cle "Startup" */
		result = REG_Fermer (hkey_startup);

		if (MERROR(result, ERROR_SUCCESS, TRUE))
			index = -1;

        if (ignore)
		    TRC_Trace_Texte (trc, TRC_OPT_MASK, "  ==> %u process a ne pas lancer", ignore);

	}
	else
		index = -1;

	return (index+surplus);
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD LaunchApplication (DWORD nb_exe)
 * PARAMETRES: nb_exe : nb executables a lancer
 * RETOUR    : nb executables lances
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Essaie de lancer tous les programmes qui ont ete lus dans
 *			   la cle 'startup'. Si un d'eux ne demarre pas, les autres
 *			   sont arretes et la fonction retourne.
 * 
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD LaunchApplication (DWORD nb_exe)
{
	DWORD			index;
	HANDLE			h;
	LARGE_INTEGER	liDueTime = {0};
	LONG			lPeriod = 0;

	/* Create event object nommed "Stop application". This event will be signaled by : */
	/*		- startup when application request his end */
	/*      - a process before it terminate */
	if ((hEvtStop = CreateEvt(LWD_EVT_APPLICATION_STOP)) == NULL)
	{
		/*********************/
		/** Sortie fonction **/
		/*********************/
		return 0;
	}

	/* Create event object nommed "Flag end". This event must be signaled by application */
	/* to inform that the end (of application) is volontary ! */
	if ((hEvtEnd = CreateEvt(LWD_EVT_FLAG_END_NAME)) == NULL)
	{
		/*********************/
		/** Sortie fonction **/
		/*********************/
		return 0;
	}
	else
	{
		/* Memoriser le handle associe a l'evenement "arret application" */
		hprogram[nb_exe] = hEvtEnd;
	}

	// Creating waitabletimer object
	if ((hTimer1 = CreateWaitableTimer(NULL, FALSE, NULL)) == NULL)
		return 0;
	else
	{
		hprogram[nb_exe+1] = hTimer1;

		//liDueTime.QuadPart = -(int)resource_delay_sec * 10000000; // resource_delay_sec seconds in 100 nano seconds
		liDueTime.QuadPart = -20 * 10000000;
		lPeriod = resource_delay_sec * 1000;
		
		if(resource_delay_sec > 0)
			SetWaitableTimer(hTimer1, &liDueTime, lPeriod, NULL, NULL, TRUE);
	}

	/* Lancer tous les programmes lus dans la cle */
	index = 0;
	while (index < nb_exe)
	{
		if ((h=LaunchProgram (program[index], priority_class[index])) != (HANDLE)ERROR_INVALID_HANDLE)
		{
			hprogram[index] = h;
			// set process affinity 
			if (affinity_bitmask[index])
			{
				if (!SetProcessAffinityMask(h, affinity_bitmask[index]))
					TRC_Trace_Texte (trc, TRC_OPT_MASK, "SetProcessAffinityMask() : Error = %d, for process %s", GetLastError(), program[index]);
			}
		}
		else
			break;
		index ++;

		Sleep(delay_between_processes);
	}

	/* Au moins un exe n'a pu se lancer */
	if (index < nb_exe)
	{
		/* Terminer les autres exe deja lances */
		while (index >= 1) /* '1' car on decremente l'index avant l'appel de la fonction ... */ 
		{
			index --;
			KillProcess (index); /* ... ceci permet d'ignorer le process qui n'a pu se lancer */
		}

		/* Destruction de l'evenement "arret application" */
		ResetAndCloseEvt (hEvtEnd); 

		/* Destruction de l'evenement "stop application" */
		ResetAndCloseEvt (hEvtStop);

		/*********************/
		/** Sortie fonction **/
		/*********************/
		return 0;
	}
	
	return index;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WatchApplication (DWORD nb_exe)
 * PARAMETRES: nb_exe : nb executables a surveiller
 * RETOUR    : nb executables qui etaient surveilles ou 0 si probleme
 *			   sur WaitForMultipleObjects()
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Surveille la fin anormale d'un des programmes lances.
 *			   Si un programme s'arrete et que le signal 'fin normale'
 *			   n'a pas ete donne, alors cette fonction arrete tous les
 *             autres process (TerminateProcess()).
 *			   Si le signal 'fin normale' a ete donne, cette fonction
 *			   ne fait aucune action sur les process et retourne.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WatchApplication (DWORD nb_exe)
{
	DWORD index,object_signaled, nb_handle, code_sortie, libelle_code_sortie, last_error,EventStopState;
	BOOL get_exit_code_ok;
	char * fin_process="*** Process %s ARRETE. Code de sortie = ";
	char sortie_process[257];

	nb_handle = nb_exe + 2; // '+1' pour l'evenement "arret application"; +2 FOR TIMER 
	index = 0;

	if(bFirst)
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Attente de la fin d'un process (ou de l'arret application) ...");



	/*******************************************************************/
	/* Attente sur la fin d'un des process lances ou fin d'application */
	/*******************************************************************/
	object_signaled = WaitForMultipleObjects (nb_handle, hprogram, FALSE, INFINITE);

	if ((object_signaled >= WAIT_OBJECT_0) && (object_signaled < (WAIT_OBJECT_0+nb_handle)))
	{
		/* Retablir offset a partir de 0 */
		object_signaled -= WAIT_OBJECT_0;

		if (hprogram[object_signaled] == hTimer1)
		{
			LWD_TraceResources(nb_exe);
		}
		/* Si ce n'est pas une fin application */
		else if (hprogram[object_signaled] != hEvtEnd)
		{

			/* Recuperer code de sortie du process arrete */
			if ((get_exit_code_ok=GetExitCodeProcess (hprogram[object_signaled], &code_sortie))
				== FALSE)
				MERROR (GetLastError(), ERROR_SUCCESS, TRUE);

			strcpy_s(sortie_process, sizeof(sortie_process) - 1, fin_process);
			/* Tracer process arrete sans signal 'fin application' */
			if (get_exit_code_ok)
			{
				strcat_s(sortie_process, sizeof(sortie_process) - 1, "0x%X (%lu/%lu) : %s. ***");
                libelle_code_sortie = code_sortie;
                if (libelle_code_sortie > LAST_EXIT_CODE)
                    libelle_code_sortie = EXIT_BUTEE;
				TRC_Trace_Texte (trc, TRC_OPT_MASK, sortie_process,
					program[object_signaled], code_sortie, code_sortie, LAST_EXIT_CODE, 
                    lib_exit_codes[libelle_code_sortie-FIRST_EXIT_CODE]);

				/* Si le code de sortie du programme concerne doit etre teste */
				if (exit_code_test[object_signaled])
				{
					/* Recuperer l'etat de l'evenement 'Stop' */
					EventStopState = WaitForSingleObject (hEvtStop, 0);
					/* Si evt 'Stop' positionne ... */
					if (EventStopState == WAIT_OBJECT_0)
					{
						/* ...le programme concerne est alors sorti volontairement */
						/* Signaler la fin du process 'Startup' */
						fin = TRUE;
						TRC_Trace_Texte (trc, TRC_OPT_MASK, "Le Process s'est arrete volontairement.");
					}
					else
					{
						last_error = GetLastError ();
						TRC_Trace_Texte (trc, TRC_OPT_MASK, "Resultat test evt Stop = %lu.", object_signaled);
						if (object_signaled == WAIT_FAILED)
							MERROR (last_error, ERROR_SUCCESS, TRUE);
					}
				}
			}
			else
			{
				strcat_s(sortie_process, sizeof(sortie_process) - 1, "irrecuperable. ***");
				TRC_Trace_Texte (trc, TRC_OPT_MASK, sortie_process);
			}

			/* Fermeture du handle associe au process */
			if (CloseHandle (hprogram[object_signaled]) == 0)
				MERROR (GetLastError(), ERROR_SUCCESS, TRUE);

			hprogram[object_signaled] = (HANDLE)ERROR_INVALID_HANDLE;


			/* Arreter tous les autres process */
			while (index < nb_exe)
			{
				KillProcess (index);
				index ++;
			}

            /* Laisser une tempo entre 2 lancements d'application (si fin 'Startup' non memorisee) */
            if (!fin)
            {
	            TRC_Trace_Texte (trc, TRC_OPT_MASK, "Delai de %lu ms avant de relancer l'application ...", sleep_between_2_launch);
                Sleep (sleep_between_2_launch);
            }
		}
		/* C'est une fin application */
		else
		{
			TRC_Trace_Texte (trc, TRC_OPT_MASK, "Arret application normal.");
			
			//TIC 2013/01/10 - to assure that the startup will exit on AppStop event
			index = nb_exe;
			fin = TRUE;
			StopApplication (nb_exe);
			//End TIC 2013/01/10 
				
		}
	}
	else /* Sortie du WaitForMultipleObjects() sans objet signale */
	{
		MERROR (object_signaled, WAIT_OBJECT_0, FALSE);
	}

	if (index != 0) /* Si on a du arreter les process surveilles ou fin application */
	{
		ResetAndCloseEvt (hEvtEnd); 
		ResetAndCloseEvt (hEvtStop); 
	}

	return index;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD StopApplication (DWORD nb_exe)
 * PARAMETRES: nb_exe : nb executables a stopper
 * RETOUR    : nb executables stoppes ou <0 si probleme
 *			   (sur WaitForMultipleObjects(), ...)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Signale le stop application et attend la fin des process
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD StopApplication (DWORD nb_exe)
{
	DWORD index, nb_handle, code_sortie;
	BOOL get_exit_code_ok;
	char * fin_process="*** Process %s ARRETE. Code de sortie = ";
	char sortie_process[257];

	/* Signaler l'evenement 'stop application' */
	if (SetEvent (hEvtStop) == 0)
		return -1;

	nb_handle = nb_exe;
	index = 0;

	TRC_Trace_Texte (trc, TRC_OPT_MASK, "Attente (%lu ms) de la fin des process ...", end_timeout);

	/*********************************/
	/* Attente de la fin des process */
	/*********************************/
// 	object_signaled = WaitForMultipleObjects (nb_handle, hprogram, TRUE, end_timeout);
// 
// 	if (object_signaled == WAIT_TIMEOUT)
// 	{
		Sleep(end_timeout);

		index = 0;

		// stop timer
		if(hTimer1)
			CancelWaitableTimer(hTimer1);

		if(hTimer1)
			CloseHandle(hTimer1);

		/* Arreter tous les process qui ne se sont pas arretes */
		while (index < nb_exe)
		{
			/* Recuperer code de sortie du process arrete */
			if ((get_exit_code_ok=GetExitCodeProcess (hprogram[index], &code_sortie))
				== FALSE)
				MERROR (GetLastError(), ERROR_SUCCESS, TRUE);

			strcpy_s(sortie_process, sizeof(sortie_process) - 1, fin_process);
			/* Tracer process arrete sans signal 'fin application' */
			if (get_exit_code_ok)
			{
				strcat_s(sortie_process, sizeof(sortie_process) - 1 , "%lu. ***");
				TRC_Trace_Texte (trc, TRC_OPT_MASK, sortie_process,
					program[index], code_sortie);

				/* Si process toujours actif ... */
				if (code_sortie == STILL_ACTIVE)
				{
					/* ... l'arreter */
					KillProcess (index);
				}
			}
			else
			{
				strcat_s(sortie_process, sizeof(sortie_process) - 1 , "irrecuperable. ***");
				TRC_Trace_Texte (trc, TRC_OPT_MASK, sortie_process);

				/* Arreter le process */
				KillProcess (index);
			}

			index ++;
		}
// 	}

	/* Signaler la fin du process 'Startup' */
	fin = TRUE;

	return index;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE LaunchProgram (CHAR *program, DWORD priority )
 * PARAMETRES: *program : chaine contenant le programme (chemin+nom+parametres)
 *						  a executer
 *             priority : classe de priorité du nouveau process
 * RETOUR    : Handle sur le process cree
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cree un process avec le parametre passe
 * 
 * --------------------------------------------------------------------
 * $F_FCTN
 */
HANDLE LaunchProgram (CHAR *program, DWORD priority )
{
	CHAR lpzCurDir[MAX_PATH] = {0}, lpzCmdLine[MAX_PATH] = { 0 };
	BOOL blRc;
	DWORD dwCreatFlags, last_error;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION lpPInfos;
	SECURITY_ATTRIBUTES sAttr;
	
	/*----------------------------------------------------------------*/
	/*        Récupération du répertoire courrant                     */
	/*----------------------------------------------------------------*/
	GetCurrentDirectory (MAX_PATH, lpzCurDir);
	
	/*----------------------------------------------------------------*/
	/*        Création du processus PROCESSUS1                        */
	/*----------------------------------------------------------------*/
	dwCreatFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | priority;        
	
	/* ligne de commande */
	strcpy_s(lpzCmdLine, sizeof(lpzCmdLine) - 1, program);
	
	/* initialise la structure STARTUPINFO */
	memset (&StartupInfo, 0, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USECOUNTCHARS | STARTF_USEFILLATTRIBUTE |
		/*STARTF_USEPOSITION |*/ STARTF_USESIZE;
	StartupInfo.dwXCountChars = 80;
	StartupInfo.dwYCountChars = 25;
	StartupInfo.dwX = 10;
	StartupInfo.dwY = 10;
	/* Data size window for 80x25 characters (with police=8x12) */
	StartupInfo.dwXSize = 640;
	StartupInfo.dwYSize = 300;
	/* Background GRAY, Text BLACK */
	StartupInfo.dwFillAttribute = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	
	/* Positionne les attributs de securité de sorte a rendre le */
	/* nouveau processus héritable                               */
	sAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	sAttr.lpSecurityDescriptor = NULL;
	sAttr.bInheritHandle = TRUE;
	
	/* création du processus P1 */
	blRc = CreateProcess (NULL,    /* pointe sur le nom du programme */
		lpzCmdLine,  /* pointe la ligne de commande */
		&sAttr,      /* pointe sur les attributs de sécurité du processus */ 
		NULL,        /* pointe sur les attributs de sécurité du thread */ 
		FALSE,       /* drapeau d'héritage */ 
		dwCreatFlags,/* drapeau de creation */ 
		NULL,        /* nouveau block d'environnement */
		lpzCurDir,   /* pointe sur le nom du répertoire courrant */ 
		&StartupInfo,/* pointe sur STARTINFO */ 
		&lpPInfos);  /* pointe sur PROCESS_INFORMATION */ 
	if (blRc == FALSE)
	{
		last_error = GetLastError ();
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Erreur CreateProcess : %s",program);
		MERROR (last_error, ERROR_SUCCESS, TRUE);
		return (HANDLE)ERROR_INVALID_HANDLE;
	}
	else
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Lancement du process %s OK ... ", program);
	
	/* Fermeture du handle thread (non utilise) */
	/* (Le handle process reste ouvert car utilise pour surveiller process) */
	blRc = CloseHandle (lpPInfos.hThread);
	
//	_getch ();
	
	return lpPInfos.hProcess;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE CreateEvt (char *) 
 * PARAMETRES: pointeur sur nom de l'evenement
 * RETOUR    : Handle sur l'evenement (objet nomme) cree
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Creation d'un objet evenement nomme 
 * --------------------------------------------------------------------
 * $F_FCTN
 */
HANDLE CreateEvt (char *p_evt_name) 
{
	HANDLE hEvt;

	/* Create event object nommed <p_evt_name> */
	hEvt = CreateEvent (NULL, TRUE, FALSE, p_evt_name);

	if (hEvt == NULL)
	{
		MERROR (GetLastError(), ERROR_SUCCESS, TRUE);
	}

	return hEvt;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE CreateEvt (char *) 
 * PARAMETRES: pointeur sur nom de l'evenement
 * RETOUR    : Handle sur l'evenement (objet nomme) cree
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Creation d'un objet evenement nomme 
 * --------------------------------------------------------------------
 * $F_FCTN
 */
void ResetAndCloseEvt (HANDLE hEvt) 
{
	if (ResetEvent (hEvt) == 0)
	{
		MERROR (GetLastError(), ERROR_SUCCESS, TRUE);
	}
	if (CloseHandle (hEvt) == 0)
	{
		MERROR (GetLastError(), ERROR_SUCCESS, TRUE);
	}
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : void KillProcess  ( DWORD index )
 * PARAMETERS: DWORD index : index dans le tableau des handles de process
 * RETURN    : sans objet
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Terminer un process et fermer le handle associe
 * --------------------------------------------------------------------
 */

BOOL SafeTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    DWORD dwTID, dwCode, dwErr = 0;
    HANDLE hProcessDup = INVALID_HANDLE_VALUE;
    HANDLE hRT = NULL;
    HINSTANCE hKernel = GetModuleHandle("Kernel32");
    BOOL bSuccess = FALSE;

    BOOL bDup = DuplicateHandle(GetCurrentProcess(), 
                                hProcess, 
                                GetCurrentProcess(), 
                                &hProcessDup, 
                                PROCESS_ALL_ACCESS, 
                                FALSE, 
                                0);

	if(!bDup)
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "DuplicateHandleFails");

    // Detect the special case where the process is 
    // already dead...
    if ( GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && 
         (dwCode == STILL_ACTIVE) ) 
    {
        FARPROC pfnExitProc;
           
        pfnExitProc = GetProcAddress(hKernel, "ExitProcess");

        hRT = CreateRemoteThread((bDup) ? hProcessDup : hProcess, 
                                 NULL, 
                                 0, 
                                 (LPTHREAD_START_ROUTINE)pfnExitProc,
                                 (PVOID)uExitCode, 0, &dwTID);

        if ( hRT == NULL )
            dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_PROCESS_ABORTED;
    }


    if ( hRT )
    {
        // Must wait process to terminate to 
        // guarantee that it has exited...
        
		if(WaitForSingleObject((bDup) ? hProcessDup : hProcess, 
			3000)==WAIT_TIMEOUT){
			TerminateProcess((bDup) ? hProcessDup : hProcess,0xFADE);
			}
		
        CloseHandle(hRT);
        bSuccess = TRUE;
    }

    if ( bDup )
        CloseHandle(hProcessDup);

    if ( !bSuccess )
        SetLastError(dwErr);

    return bSuccess;
}


void KillProcess (DWORD index)
{
	if (hprogram[index] != (HANDLE)ERROR_INVALID_HANDLE)
	{
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Le handle de %s est encore valide", program[index]);
		if(SafeTerminateProcess(hprogram[index], 0xDEADBEEF) )
		{
			DWORD dwP;
			
			GetExitCodeProcess(hprogram[index], &dwP);
		
			TRC_Trace_Texte (trc, TRC_OPT_MASK, "Process %s ProcessExitCode %X",program[index],dwP);
		
			CloseHandle(hprogram[index]);
			hprogram[index] = (HANDLE)ERROR_INVALID_HANDLE;
		}
		else
		{
			DWORD dwErr = GetLastError();
			TRC_Trace_Texte (trc, TRC_OPT_MASK, "SafeTerminate LastError %X", dwErr);
		}
	}
	else
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Le handle du process %s n'est plus valide", program[index]);
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
		TRC_Trace_Texte (trc, TRC_OPT_MASK, "Error %s(%lu).", source_file, line);
		if (err_win32)
			TRC_Trace_Erreur_Win32 (trc, TRC_OPT_MASK, code);

		return TRUE;
	}

	return FALSE;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL LWD_FullCreatePath  ( char *path )
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
 * EXEMPLES  : result = LWD_FullCreatePath ("..\dir1\dir2");
 *			   result = LWD_FullCreatePath ("c:\dir1\dir2\dir3");
 *			   result = LWD_FullCreatePath ("\\ordinateur_distant\partage\dir1\dir2");
 *			   result = LWD_FullCreatePath ("\\ordinateur_distant\partage\nom long ok\dir");
 * REMARQUE 2 : Cette fonction est une copie de FIC_FullCreatePath() [Fonct_c.dll de OUTILS_C]
 *				C'est une copie car startup doit rester independant du noyau (OUTILS_C utilise 
 *              le noyau pour ExitBad()).
 * --------------------------------------------------------------------
 */
BOOL LWD_FullCreatePath (char *path)
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

void LWD_TraceResources(DWORD nb_exe)
{
	if(resource_delay_sec == 0)
		return;

	if(nb_exe == 0)
	{
		TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "nb_exe = 0");
		return;
	}

	TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "==========================================================================================");
	TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "");
	LWD_TraceTotalMemoryInfo();
	TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "");
	LWD_TraceProcessInfo(nb_exe);
	TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "");
	bFirst = FALSE;
}

void LWD_TraceTotalMemoryInfo(void)
{
	char					*divisor	= "K";
	MEMORYSTATUSEX			statex		= {0};
	PERFORMACE_INFORMATION	pi			= {0};
	char					szTmp[1024]	= {0};
	BOOL					bGms		= FALSE;
	BOOL					bGpi		= FALSE;
	struct_sysres			sysRes		= {0};

	statex.dwLength = sizeof(statex);

	if(GlobalMemoryStatusEx(&statex))
	{
		bGms = TRUE;
		memcpy(&sysRes.statex, &statex, sizeof(statex));
	}
	else
	{
		TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "Unable to get system's current usage of physical and virtual memory");
	}

	pi.cb = sizeof(pi);
	if(GetPerformanceInfo(&pi, sizeof(pi)))
	{
		bGpi = TRUE;
		memcpy(&sysRes.pi, &pi, sizeof(pi));
	}
	else
	{
		TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "Unable to get performance info");
	}

	if((bGms == TRUE) && (bGpi == TRUE))
	{
		if(AlignTotalTraceLine("Total physical memory:", statex.ullTotalPhys/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), FALSE, 0))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

		if(AlignTotalTraceLine("Total paging file:", statex.ullTotalPageFile/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), FALSE, 0))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

		if(AlignTotalTraceLine("Total virtual memory:", statex.ullTotalVirtual/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), FALSE, 0))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

		TRC_Trace_Texte(trc_res, TRC_OPT_MASK, "");
		TraceTotalTableHeader();

 		if(AlignTotalTraceLine("Memory in use:", statex.dwMemoryLoad, " %%", LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.statex.dwMemoryLoad))
 			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);
		
		if(AlignTotalTraceLine("Free physical memory:", statex.ullAvailPhys/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.statex.ullAvailPhys/DIV2))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);
		
		if(AlignTotalTraceLine("Free paging file:", statex.ullAvailPageFile/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.statex.ullAvailPageFile/DIV2))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);
		
		if(AlignTotalTraceLine("Free virtual memory:", statex.ullAvailVirtual/DIV2, " Kb", LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.statex.ullAvailVirtual/DIV2))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

		if(AlignTotalTraceLine("Total handle count:", pi.HandleCount, NULL, LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.pi.HandleCount))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);
		
		if(AlignTotalTraceLine("Total process count:", pi.ProcessCount, NULL, LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.pi.ProcessCount))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);
		
		if(AlignTotalTraceLine("Total thread count:", pi.ThreadCount, NULL, LINE_LEN, szTmp, sizeof(szTmp), TRUE, sysResOld.pi.ThreadCount))
			TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

		memcpy(&sysResOld.statex, &statex, sizeof(statex));
		memcpy(&sysResOld.pi, &pi, sizeof(pi));
	}
}        



void LWD_TraceProcessInfo (DWORD dwNbProcesses)
{
	DWORD					dwIdx		= 0;
	DWORD					dwMax		= 0;
	PROCESS_MEMORY_COUNTERS	sProcMemCnt = {0};
	char					szTmp[1024]	= {0};

	dwMax = GetLongestProcessName(dwNbProcesses);

	if(dwNbProcesses > 0)
		TraceProcessTableHeader(dwMax);

	for (dwIdx = 0; dwIdx < dwNbProcesses; dwIdx++)
	{
		if (hprogram[dwIdx])
		{
			if (GetProcessMemoryInfo(hprogram[dwIdx], &sProcMemCnt, sizeof (sProcMemCnt)))
			{
				if(AlignProcessTraceLine(program[dwIdx], dwMax, " Kb", szTmp, sizeof(szTmp), &sProcMemCnt, &sProcMemCntOld[dwIdx]))
					TRC_Trace_Texte(trc_res, TRC_OPT_MASK, szTmp);

				memcpy(&sProcMemCntOld[dwIdx], &sProcMemCnt, sizeof(sProcMemCnt));
			}
		}
	}
}

DWORD GetLongestProcessName(DWORD dwNbProcesses)
{
	DWORD	i		= 0;
	DWORD	dwMax	= 0;
	DWORD	dwLen	= 0;

	for (i = 0; i < dwNbProcesses; i++)
	{
		dwLen = (DWORD)strlen(program[i]);

		if(dwLen > dwMax)
			dwMax = dwLen;
	}

	return dwMax;
}

void TraceProcessTableHeader(DWORD dwMax)
{
	DWORD	i				= 0;
	DWORD	dwLen1			= 0;
	char	szHeader[1024]	= {0};
	
	strcpy_s(szHeader, sizeof(szHeader) -1, "Process");
	
	dwLen1 = (DWORD)strlen(szHeader);
	
	for(i = dwLen1; i <= dwMax + dwLen1; i++)
		szHeader[i] = ' ';
	
	if(bFirst)
		strcat_s(szHeader, sizeof(szHeader) - 1, "  W.Set        P.W.Set         P.Pool");
	else
		strcat_s(szHeader, sizeof(szHeader) - 1, "  W.Set        P.W.Set         P.Pool     Delta W.S.     Delta P.P.");
	
	TRC_Trace_Texte (trc_res, TRC_OPT_MASK, szHeader);
}

void TraceTotalTableHeader(void)
{
	char	szHeader[1024]	= {0};

	if(bFirst)
		strcat_s(szHeader, sizeof(szHeader) - 1, "                                     Now");
	else
		strcat_s(szHeader, sizeof(szHeader) - 1, "                                     Now              Delta");

	TRC_Trace_Texte (trc_res, TRC_OPT_MASK, szHeader);
}

BOOL AlignTotalTraceLine(char *pszDescription, DWORDLONG dwlValue, char *pszValueUnit, DWORD dwLineLen, OUT char *pszLine, DWORD dwLineSize, BOOL bDelta, DWORDLONG dwlOldValue)
{
	DWORD		i				= 0;
	DWORD		len1			= 0;
	DWORD		len2			= 0;
	DWORD		len3			= 0;
	int			iDotCount		= 0;
	ULONGLONG	ullDiff			= 0;
	char		szTmp[MAX_PATH]	= {0};
	
	memset(pszLine, 0, dwLineSize);
	
	if(pszValueUnit != NULL)
		sprintf_s(szTmp, sizeof(szTmp) - 1, "%I64d%s", dwlValue, pszValueUnit);
	else
		sprintf_s(szTmp, sizeof(szTmp) - 1, "%I64d", dwlValue);
	
	len1 = (DWORD)strlen(pszDescription);
	len2 = (DWORD)strlen(pszLine);
	
	if((len1 < dwLineSize) && ((dwLineSize - len2) > len1))
		strcpy_s(pszLine, dwLineSize - 1, pszDescription);
	else
		return FALSE;
	
	len2 = (DWORD)strlen(pszLine);
	len3 = (DWORD)strlen(szTmp);

	if(pszValueUnit != NULL)
		if(strchr(pszValueUnit, '%'))
			len3--;

	iDotCount = dwLineLen - (len2 + len3);
	
	if(iDotCount > 0)
	{
		for(i = len2; i < (iDotCount + len2); i++)
			pszLine[i] = '.';
	}
	else
		return FALSE;
	
	len2 = (DWORD)strlen(pszLine);
	sprintf_s(&pszLine[len2], dwLineSize - len2-1, "%s", szTmp);

	if(bDelta)
	{
		if(!bFirst)
		{
			ullDiff = dwlValue - dwlOldValue;

			AddNewMonitoringItem(0, pszValueUnit, pszLine, dwLineSize, (int)ullDiff, 20);
		}
	}

	
	return TRUE;
}


BOOL AlignProcessTraceLine(char *pszName, DWORD dwMaxLen, char *pszUnit, OUT char *pszLine, DWORD dwLineSize, PROCESS_MEMORY_COUNTERS *pPmc, PROCESS_MEMORY_COUNTERS *pOld)
{
	DWORD	i				= 0;
	DWORD	len1			= 0;
	DWORD	len2			= 0;
	DWORD	len3			= 0;
	DWORD	dwLineLen		= 0;
	DWORD	dwOld			= 0;
	int		iDotCount		= 0;
	DWORD	dwWSet			= 0;
	DWORD	dwPWSet			= 0;
	DWORD	dwPPool			= 0;
	char	szTmp[MAX_PATH]	= {0};

	memset(pszLine, 0, dwLineSize);

	dwWSet = (DWORD)(pPmc->WorkingSetSize/DIV2);
	dwPWSet = (DWORD)(pPmc->PeakWorkingSetSize / DIV2);
	dwPPool = (DWORD)(pPmc->QuotaPagedPoolUsage / DIV2);

	strcpy_s(pszLine, dwLineSize-1, pszName);
	AddNewMonitoringItem(dwMaxLen, pszUnit, pszLine, dwLineSize, dwWSet, 0);
	AddNewMonitoringItem(0, pszUnit, pszLine, dwLineSize, dwPWSet, 0);
	AddNewMonitoringItem(0, pszUnit, pszLine, dwLineSize, dwPPool, 0);

	if(!bFirst)
	{
		dwOld = (DWORD)(pOld->WorkingSetSize / DIV2);
		AddNewMonitoringItem(0, pszUnit, pszLine, dwLineSize, dwWSet - dwOld, 0);

		dwOld = (DWORD)(pOld->QuotaPagedPoolUsage / DIV2);
		AddNewMonitoringItem(0, pszUnit, pszLine, dwLineSize, dwPPool - dwOld, 0);
	}

	return TRUE;
}


BOOL AddNewMonitoringItem(DWORD dwMaxLen, char *pszUnit, OUT char *pszLine, DWORD dwLineSize, int iValue, DWORD dwItemDist)
{
	DWORD	i				= 0;
	DWORD	len1			= 0;
	DWORD	len2			= 0;
	DWORD	len3			= 0;
	DWORD	dwOld			= 0;
	DWORD	dwLineLen		= 0;
	DWORD	dwDist			= 0;
	int		iDotCount		= 0;
	char	szTmp[MAX_PATH]	= {0};
	
	len1 = (DWORD)strlen(pszLine);

	if(dwItemDist > 0)
		dwDist = dwItemDist;
	else
		dwDist = 15;
	
	if(pszUnit != NULL)
		sprintf_s(szTmp, sizeof(szTmp) - 1, "%d%s", iValue, pszUnit);
	else
		sprintf_s(szTmp, sizeof(szTmp) - 1, "%d", iValue);
	
	len2 = (DWORD)strlen(szTmp);
	
	if(dwMaxLen > 0)
		dwLineLen = dwMaxLen + dwDist;
	else
		dwLineLen = len1 + dwDist;
	
	iDotCount = dwLineLen - (len1 + len2);
	
	if(iDotCount > 0)
	{
		for(i = len1; i < (iDotCount + len1); i++)
			pszLine[i] = '.';
	}
	else
		return FALSE;
	
	len3 = (DWORD)strlen(pszLine);
	sprintf_s(&pszLine[len3], dwLineSize - len3 - 1, "%s", szTmp);
	
	return TRUE;
}