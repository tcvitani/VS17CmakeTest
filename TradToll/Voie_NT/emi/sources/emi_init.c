/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions utiles pour l'application
* --------------------------------------------------------------------
* DESCRIPTION: Fonctions EmiRessource(), EmiLance() et EmiArret()
*              qui permettent a une application utilisatrice du module
*              de l'initialiser, de le lancer et de l'arreter.
*              Fonctions EmiDebutDebug() et EmiFinDebug() pour ecrire
*              dans un fichier les erreurs survenues.
*              Fonctions EmiDebutTrace() et EmiFinTrace() pour ecrire
*              dans un fichier les traces utiles lors de l'integration
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emi_init.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:04   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.20   Jan 22 1999 15:19:28   FR
 * Mise à jour du numéro de version
 * 
 *    Rev 1.19   08 Dec 1997 11:23:12   DPI
 * Correction bug printf
 *
 *    Rev 1.18   03 Dec 1997 15:51:14   DPI
 * Evolution pour TCPx.LIB 5.00
 * 
 *    Rev 1.17   06 Nov 1997 15:18:28   DPI
 * Integration DUT 9.00
 * 
 *    Rev 1.16   03 Nov 1997 16:10:14   DPI
 * Version 4.00
 * 
 *    Rev 1.15   30 Oct 1997 15:53:52   DPI
 * Suppression du chrono, remplacer par HRD
 * Formule de calcul de l'heure
 * 
 *    Rev 1.14   29 Oct 1997 19:42:10   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 * 
 *    Rev 1.13   25 Sep 1997 16:02:00   HMO
 * Corection erreurs
 *
 *    Rev 1.12   23 Sep 1997 16:15:18   HMO
 * Modifications effectuées apres site pour MAJ
 *
 *    Rev 1.11   23 Sep 1997 14:43:04   HMO
 * Modifications effectuées sur site
 * 
 *    Rev 1.10   Aug 06 1997 15:38:26   DPI
 * Correction emission en backup numero
 * de fichier sans extension
 *
 *    Rev 1.9   Aug 01 1997 17:41:48   HMO
 *  
 * 
 *    Rev 1.8   Aug 01 1997 17:29:32   HMO
 * Gestion de la version
 * 
 *    Rev 1.7   Jun 13 1997 10:34:16   HMO
 *  
 * 
 *    Rev 1.6   May 29 1997 17:36:12   ANA
 *  
 * 
 *    Rev 1.5   May 27 1997 09:30:50   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.4   May 16 1997 13:32:58   ANA
 * Remplacement du fichier emi_act.c par msg_fic.c
 * 
 * 
 *    Rev 1.3   May 13 1997 16:51:34   ANA
 * Gestion de la sauvegarde sur disque et de la purge,
 * Nouvel algorithme de recherche du  fichier de départ.
 * 
 *
 *    Rev 1.2   Apr 15 1997 10:03:36   ANA
 * Correction du bug sur le service FICHIER deTCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:48   ANA
 * Nouvelle gestion du "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:24:56   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <stdarg.h>
#include <io.h>
#include <conio.h>
#include <string.h>

#include <csrlc32.h>
#include <run.h>
#include <reg.h>

#include "err.h"
#include "fic.h"

/* module NOYAU */
#include <noyau.h>
#include <debug.h>

//#include <tcp_ip.h>

#include <csr_lan.h>

#include "str.h"
#include "fic_conf.h"

#include "horodate.h"
#include "emi_fic.h"

#include "msg_fic.h"

#define LOC_DEF
#include "emi_glob.h"
#undef LOC_DEF

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/
#define MAXCHAINE 127

/*--------------- TYPEDEFS: ---------------*/

/*#define NOM_PERIODE_EMISSION   "PERIODE_EMISSION"
#define NOM_PATH_DISTANT       "PATH_DISTANT"
#define NOM_PERIODE_PURGE      "PERIODE_PURGE"
#define NOM_HEURE_PURGE        "HEURE_PURGE"
#define NOM_DELTA_FERMER       "DELTA_FERMER_FICHIER"
#define NOM_DELTA_EMETTRE      "DELTA_EMETTRE_FICHIER"
#define NOM_NUM_VOIE           "NUM_VOIE"
#define NOM_DATE_HRD           "DATE_HRD"*/

#define MOD_REG_KEYv_DISK				"disk_path"
#define MOD_REG_KEYv_BACKUP				"backup_path"
#define MOD_REG_KEYv_HOSTNAME			"hostname"
#define MOD_REG_KEYv_DISTANT			"distant_path"
#define MOD_REG_KEYv_LOCAL_DRIVE		"local_root"	
#define MOD_REG_KEYv_LOCAL_DIRECTORY	"local_directory"
#define MOD_REG_KEYv_PURGE_PER			"purge_period"
#define MOD_REG_KEYv_EMISSION_PER		"emission_period"
#define MOD_REG_KEYv_PURGE_TIME			"purge_time"
#define MOD_REG_KEYv_DELTA_EMISSION		"delta_emission"
#define MOD_REG_KEYv_DELTA_CLOSING		"delta_closing"
#define MOD_REG_KEYv_LANE_NO			"lane_number"
#define MOD_REG_KEYv_BL_FILE			"bl_file_name"
#define MOD_REG_KEYv_BL_HRD				"bl_hrd_name"
#define MOD_REG_KEYv_COLD_START			"cold_start"

/*--------------- FUNCTIONS: --------------*/

//PRIVATE   void LectureFichierConfiguration( char *fic_buffer);
PRIVATE int CalculerHeureDebutChrono( int delta_fermer,
                                      int delta_emettre,
                                      int num_voie);

/*--------------- VARIABLES: --------------*/

PRIVATE boolean emi_flag_trace = FALSE;
PRIVATE boolean emi_flag_ecran = FALSE;
PRIVATE boolean emi_flag_fichier_ouvert = FALSE;

PROTECTED FILE *emi_fic_trace;
PROTECTED FILE *emi_fic_debug;


/*--------------- CODE: -------------------*/
extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:

			/* raz structure */
			memset (&EMI,0,sizeof(EMI));
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  
            break;          

         default:             
            break;     
    }      
    
    return TRUE; 

    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI MODLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
 * PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
 *                                         de lancement de l'instance.
 *             IN char * pcBalName       : Nom de la boite à lettre à creer pour l'instance
 *             OUT noyau_bal_id * piBalId: Pointe sur une variable recevant l'id de la boite à lettre
 *                                         créée pour l'instance (et servant d'identifiant d'instance.
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour démarrage
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{
	return EmiLance (pcKey, pcBalNam, piBalId);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI MODArret ( IN noyau_bal_id iBalId )
 * PARAMETERS: IN noyau_bal_id iBalId : Id de la bal identifiant l'instance
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour arret
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
	return EmiArret (iBalId);
}
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI EmiLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
 * PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
 *                                         de lancement de l'instance.
 *             IN char * pcBalName       : Nom de la boite à lettre à creer pour l'instance
 *             OUT noyau_bal_id * piBalId: Pointe sur une variable recevant l'id de la boite à lettre
 *                                         créée pour l'instance (et servant d'identifiant d'instance.
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour démarrage
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI EmiLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{
	noyau_enum_retour	cr_lance;
    char                  NomTache[MAX_PATH + 1];
    char				  pcPoolId[MAX_PATH + 1];			// REG : Chaine de config du pool
	DWORD                 dwPrioMax;                        // REG : Priorité max taches
	DWORD                 dwPrioInitMax;                    // REG : Priorité max taches initiale
	DWORD                 dwLen;
	DWORD               dwColdStart;                    // REG : Booleen 'cold start'
	DWORD				dwLaneNo;
	DWORD				dwDeltaEmission;
	DWORD				dwDeltaClosing;
	char				pcBalFileName[MAX_PATH + 1];
	char				pcBalHrdName[MAX_PATH + 1];

	struct_hrd_config config_hrd = { THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_NORMAL,
		(noyau_pool_id)"Reserved",
		10 };	// nbre d'alarmes max


//	char *field_cold_start = "cold_start";
//	char *field_bl_file_name = "bl_file_name";
//	char *field_bl_hrd_name = "bl_hrd_name";
	int purge_hour, purge_minute;

//	char pcFileName[MAX_PATH + 1];
//	char *field_fichier_config = "fichier_config";
	char pcPurgeTime[MAX_PATH + 1];
//	char *field_purge_time = "purge_time";

    /* initialisation du mode trace */
    if( EmiInitTrace( pcBalNam ) != INST_INIT_OK )
        return INST_INIT_ERR_FICHIER_DEBUG;

	// Vérification de la taille du nom de la BAL ANI
	if ( STR_strlen( MAX_PATH, pcBalNam ) > MAX_PATH )
	{
		EmiFichierDebug("EMI_INIT ***** EmiLance() => Nom de BAL trop long : %s", pcBalNam );
		return INST_INIT_ERR_LANCE;
	}

	EMI.config_received = FALSE;
	EMI.authorized = FALSE;
	EMI.cold_start = FALSE;


	/*****************************************************************/
	/* [debut] Lecture des entrées de configuration dans le registre */
	/*****************************************************************/
	// Pool
	
	dwLen = sizeof(pcPoolId);

	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolId, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_POOL );
		return INST_INIT_ERR_REGISTRE;
	}

	// Priorité initiale max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &dwPrioInitMax ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);
		
	// Priorité max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &dwPrioMax ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority(dwPrioMax);

	
	// Booleen 'cold start'
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_COLD_START, &dwColdStart ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_COLD_START );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (dwColdStart > 1) /* 'cold start' doit etre 0 ou 1 */
		{
			EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur valeur registre : [%s]:%s", pcKey, MOD_REG_KEYv_COLD_START );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// BAL module File (LAN)
	dwLen = sizeof( pcBalFileName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_BL_FILE, pcBalFileName, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_BL_FILE );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// BAL module Hrd (Horodate)
	dwLen = sizeof( pcBalHrdName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_BL_HRD, pcBalHrdName, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_BL_HRD );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Fichier de configuration 
	/*dwLen = sizeof( pcFileName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_fichier_config, pcFileName, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, field_fichier_config );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (pcFileName) == 0)
		{
			EmiFichierDebug ("RC_INIT ***** EmiLance() => Entree registre vide : [%s]:%s", pcKey, field_fichier_config );
			return INST_INIT_ERR_REGISTRE;
		}
		else
		{
			/7 Chemin/nom fichier reference manuelle 
			strcpy (EMI.file_config, pcFileName);
		}
	}*/

	// Heure de la purge (format = HH:MM) */
	dwLen = sizeof( pcPurgeTime );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PURGE_TIME, pcPurgeTime, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PURGE_TIME );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (STR_strlen( MAX_PATH, pcPurgeTime) == 0)
		{
			EmiFichierDebug ("RC_INIT ***** EmiLance() => Entree registre vide : [%s]:%s", pcKey, MOD_REG_KEYv_PURGE_TIME );
			return INST_INIT_ERR_REGISTRE;
		}
		else
		{
			if (sscanf_s (pcPurgeTime, "%d:%d", &purge_hour, &purge_minute) != 2)
			{
				EmiFichierDebug ("RC_INIT ***** EmiLance() => Entree incoherente : [%s]:%s", pcKey, MOD_REG_KEYv_PURGE_TIME );
				return INST_INIT_ERR_REGISTRE;
			}
			else
				EMI.heure_purge.wHour = purge_hour;
				EMI.heure_purge.wMinute = purge_minute;
				EMI.heure_purge.wSecond = 0;
				EMI.heure_purge.wMilliseconds = 0;
		}
	}

	// Purge period
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PURGE_PER, &EMI.periode_purge) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PURGE_PER );
		return INST_INIT_ERR_REGISTRE;
	}

	// Emission period
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_EMISSION_PER, &EMI.util.duree_sommeil) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_EMISSION_PER );
		return INST_INIT_ERR_REGISTRE;
	}

	// Backup path
	dwLen = sizeof(EMI.util.DIR.path_backup);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_BACKUP, EMI.util.DIR.path_backup, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_BACKUP );
		return INST_INIT_ERR_REGISTRE;
	}


	// Distant hostname
	dwLen = sizeof(EMI.util.DIR.hostname);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_HOSTNAME, EMI.util.DIR.hostname, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_HOSTNAME );
		return INST_INIT_ERR_REGISTRE;
	}

	
	// Distant path
	dwLen = sizeof(EMI.util.DIR.path_distant);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_DISTANT, EMI.util.DIR.path_distant, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_DISTANT );
		return INST_INIT_ERR_REGISTRE;

		if (STR_strlen( MAX_PATH, EMI.util.DIR.path_distant) == 0)
		{
			EmiFichierDebug ("RC_INIT ***** EmiLance() => Entree registre vide : [%s]:%s", pcKey, MOD_REG_KEYv_DISTANT );
			return INST_INIT_ERR_REGISTRE;
		}
	}

	// Disk path
	dwLen = sizeof(EMI.util.DIR.path_disk);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_DISK, EMI.util.DIR.path_disk, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_DISK );
		return INST_INIT_ERR_REGISTRE;
	}

	// Delta emission
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_DELTA_EMISSION, &dwDeltaEmission ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_DELTA_EMISSION );
		return INST_INIT_ERR_REGISTRE;
	}

	// Delta closing
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_DELTA_CLOSING, &dwDeltaClosing ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_DELTA_CLOSING );
		return INST_INIT_ERR_REGISTRE;
	}

	// Lane number
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_LANE_NO, &dwLaneNo ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_LANE_NO );
		return INST_INIT_ERR_REGISTRE;
	}

	// Local drive

	dwLen = sizeof(EMI.util.DIR.local_drive);

	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_LOCAL_DRIVE, EMI.util.DIR.local_drive, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_LOCAL_DRIVE );
		return INST_INIT_ERR_REGISTRE;
		if (STR_strlen( MAX_PATH, EMI.util.DIR.local_drive) == 0)
		{
			EmiFichierDebug ("RC_INIT ***** EmiLance() => Entree registre vide : [%s]:%s", pcKey, MOD_REG_KEYv_LOCAL_DRIVE);
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	
	// Local directory

	dwLen = sizeof(EMI.util.DIR.local_directory);

	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_LOCAL_DIRECTORY, EMI.util.DIR.local_directory, &dwLen ) != ERROR_SUCCESS )
	{
		EmiFichierDebug ("RC_INIT ***** EmiLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_LOCAL_DIRECTORY );
		return INST_INIT_ERR_REGISTRE;
	}

	if (HRDLance(config_hrd, &EMI.bl_hrd) != HRD_LANCE_OK)
	{
		//ProjectErrorFile("**** APP INIT **** Error starting HRD module");
		ExitBad();
	}

	/***************************************************************/
	/* [fin] Lecture des entrées de configuration dans le registre */
	/***************************************************************/
	
	sprintf_s(NomTache, sizeof(NomTache), "EMI_FIC EMISSION (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
		EMI.taches[EMI_TACHE_EMISSION],
		TRUE,
		dwPrioInitMax,
		4048,
		(LPTHREAD_START_ROUTINE)(Emission),
		(PVOID)NULL,
		NULL,
		NomTache);
	
	sprintf_s(NomTache, sizeof(NomTache), "EMI_FIC BACKUP (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
		EMI.taches[EMI_TACHE_BACKUP],
		TRUE,
		dwPrioInitMax,
		4048,
		(LPTHREAD_START_ROUTINE)(EmiBackup),
		(PVOID)NULL,
		NULL,
		NomTache);
	
	// Plus d'autre tache
	NOYAU_VIDE_TACHE( EMI.taches[EMI_NB_TACHE] );
	

   /* flag indiquant le cold start */
   EMI.cold_start = (boolean)dwColdStart;

   /* Priorite max du module EMI */
   EMI.priorite_max = (short)dwPrioMax;

   /* Numero de pool */
   EMI.util.pool = NOYAU_GetPoolId(pcPoolId);

   // Start timer  
   EMI.util.heure_debut_reveil = CalculerHeureDebutChrono( dwDeltaClosing,
                                                           dwDeltaEmission,
                                                           dwLaneNo);

   STR_strcpy(MAX_PATH, EMI.util.BalNam, pcBalNam);
   STR_strcpy(MAX_PATH, EMI.util.BackupBalNam, "BL_BKP");

   /* bal d'interface avec le module Horodate (HRD) */
   EMI.util.bal_hrd = AttendBAL( pcBalHrdName );
   if( EMI.util.bal_hrd <= 0 )
   {
	   EmiFichierDebug ("EmiLance: Pb Init Bal Horodate()");
	   return INST_INIT_ERR_LANCE;
   }

   /* bal d'interface avec la tache de chargement des fichiers (module LAN) */
   EMI.util.bal_fic = AttendBAL( pcBalFileName );
	if( EMI.util.bal_fic <= 0 )
	{
		EmiFichierDebug ("EmiLance: Pb Init Bal File (LAN)");
		return INST_INIT_ERR_LANCE;
	}

   //LectureFichierConfiguration (EMI.file_config);

   /* repertoire de backup distant par date */
   STR_strcpy(MAX_PATH,EMI.util.path_backup_date,EMI.util.DIR.path_distant);
//   STR_strcat(MAX_PATH,EMI.util.path_backup_date,".bkp");
	
   /* repertoire de backup distant par numero */
   STR_strcpy(MAX_PATH,EMI.util.path_backup_numero,EMI.util.DIR.path_distant);

   /* lancement des taches */
   cr_lance = LanceTache(EMI.taches);
   if( cr_lance != NOYAU_OK )
		return INST_INIT_ERR_LANCE;

	
   *piBalId = AttendBAL( EMI.util.BalNam );
   AttendBAL(EMI.util.BackupBalNam);
    
   if( *piBalId <= 0 )
   {
		EmiFichierDebug("EmiLance: Pb InitBal()");
		return INST_INIT_ERR_LANCE;
   }

   /* Tracer succes init */
	EmiFichierTrace ("EMI_INIT: EmiLance() => init OK" );

	if (AlloueEvent(&EMI.event,NULL) != NOYAU_OK)
				ExitBad();
	
	return INST_INIT_OK;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_emi_retour EmiArret(short int num_bal)
* PARAMETRES:
*     entree: numero de boite aux lettres (celui retourne par EmiRessources())
*     retour: compte-rendu de l'arret des taches
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale
* ROLE: fonction qui permet d'arreter les taches du module EMISSION
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT enum_instance_result EmiArret(noyau_bal_id iBalId)
{
	noyau_enum_retour cr_arret ;

   /* arret des taches associees au socket trouve */
   cr_arret = ArretTaches(EMI.taches);

   /* si l'arret s'est mal passe */
   if (cr_arret == NOYAU_ARRET_TACHE_NOK)
		return INST_ARRET_NOK;

     LibereEvent(&EMI.event);

    /* Arret du mode DEBUG */
	if (DBG_Arret(&EMI.dbg) != DBG_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;
	 
	/* retourner la valeur de l'arret des taches */
	return INST_ARRET_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean LectureFichierConfiguration( void)
* PARAMETRES: aucun
* RETOUR: parametres lus dans le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Lecture des parametres de configuration de la VM : config.ini
* --------------------------------------------------------------------
* $F_FCTN
*/
/*PRIVATE void LectureFichierConfiguration( char *fichier_conf)
{
   TGereFic configuration;
   char     fic_buffer_item[CONF_MAX_ITEM];
   TValeur  fic_buffer_id;
   TValeur  fic_buffer_val;
   TValeur  valeur_retour;
   int      delta_fermer;
   int      delta_emettre;
   int      num_voie;
   int		hour, minute;

   // ouvrir le fichier 
   if( ! InitFichierConfig( &configuration, fichier_conf,"r+t",
                          fic_buffer_item, &fic_buffer_id, &fic_buffer_val))
      ERR_ErreurFatale();

   // acquerir les donnees et les mettre en forme 
   //-------------------------------------
   if( LireFichierConfig( &configuration, NOM_PERIODE_EMISSION,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();

   // periode d'emission en seconde 
   EMI.util.duree_sommeil  = atoi( valeur_retour.Contenu);
   EmiFichierTrace("duree_chrono  = '%d' secondes\n", EMI.util.duree_sommeil);

   //-------------------------------------
   if( LireFichierConfig( &configuration, NOM_PATH_DISTANT,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();
   STR_strcpy(MAX_PATH, EMI.util.path_distant, valeur_retour.Contenu);
   EmiFichierTrace("path_distant  = '%s'\n", EMI.util.path_distant);

   //-------------------------------------
   if( LireFichierConfig( &configuration, NOM_PERIODE_PURGE,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();

   // periode de purge 
   EMI.periode_purge = atoi( valeur_retour.Contenu);
   EmiFichierTrace("periode_purge  = '%d' jours\n", EMI.periode_purge);

   //-------------------------------------
   if( LireFichierConfig( &configuration, NOM_HEURE_PURGE,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();

   sscanf( valeur_retour.Contenu, "%02d : %02d", &hour, &minute);
   EMI.heure_purge.wHour = hour;
   EMI.heure_purge.wMinute = minute;
   EMI.heure_purge.wSecond = 0;
   EMI.heure_purge.wMilliseconds = 0;
   EmiFichierTrace( "heure_purge  = '%d:%d'\n",
                    EMI.heure_purge.wHour, EMI.heure_purge.wMinute);

   //-------------------------------------
   if( LireFichierConfig( &configuration, NOM_DELTA_FERMER,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();
   delta_fermer = atoi( valeur_retour.Contenu);

   if( LireFichierConfig( &configuration, NOM_DELTA_EMETTRE,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();
   delta_emettre = atoi( valeur_retour.Contenu);

   if( LireFichierConfig( &configuration, NOM_NUM_VOIE,
                          0, &valeur_retour) != FIC_OK)
      ERR_ErreurFatale();
   num_voie = atoi( valeur_retour.Contenu);

   EMI.util.heure_debut_reveil = CalculerHeureDebutChrono( delta_fermer,
                                                           delta_emettre,
                                                           num_voie);
   EmiFichierTrace( "heure_debut_reveil  = '%d' secondes\n",
                    EMI.util.heure_debut_reveil);

   // fermer le fichier 
   if( ! FermerFichierConfig( &configuration))
      ERR_ErreurFatale();
}
*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int CalculerHeureDebutChrono
* PARAMETRES:
*            - delta_fermer
*            - delta_voie
*            - num_voie
* RETOUR: L'heure de debut du chrono
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Calcul de l'heure de d‚but du chrono
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE int CalculerHeureDebutChrono( int delta_fermer,
                                      int delta_emettre,
                                      int num_voie)
{
   int debut_chrono;

   debut_chrono = delta_fermer * num_voie + delta_emettre + num_voie;
   EmiFichierTrace( "delta  = '%d * %dV + %d +%dV' secondes\n",
                     delta_fermer, num_voie, delta_emettre, num_voie);

   return( debut_chrono);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result EmiInitTrace (char * pcBal)
* PARAMETRES: 
*     pcBal : nom de la bal d'interface
* RETOUR: compte-rendu
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui initialise le mode trace
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_instance_result EmiInitTrace (char * pcBal)
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
    dbg_struct_debug *debug;
    dbg_struct_trace *tab_traces;

    debug = &EMI.dbg;
    tab_traces = &EMI.tab_traces[EMI_TRC];

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);

  	// chemin traces
    dwLen = sizeof( debug->rep_fichiers_traces );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, debug->rep_fichiers_traces, &dwLen ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;

  	// chemin erreurs
    dwLen = sizeof( debug->rep_fichier_erreurs );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, debug->rep_fichier_erreurs, &dwLen ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
    // Taille max des fichiers
    if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;
	debug->taille_limite = dwTailleMax;
	    
	// pour initialiser les traces à l'écran
	// chaque instance du module aura son propre
    // fichier donné par son nom de bal (unique)
   STR_strcpy (MAX_PATH,debug->nom_fichier_traces_ecran, pcBal );

	// chaque instance du module aura également son propre
    // fichier d'erreurs donné par son nom de bal (unique)
   STR_strcpy (MAX_PATH,debug->nom_fichier_erreurs, pcBal );

   /* chaque instance du module aura son propre fichier de trace */
   STR_strcpy(MAX_PATH,tab_traces->nom, pcBal);

   debug->tab_traces = tab_traces;
   debug->nb_fichiers_traces = EMI_NB_TRACES;

   if( DBG_Lance(debug) != DBG_OK )
      return INST_INIT_ERR_FICHIER_DEBUG;

   return INST_INIT_OK;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void EmiFichierTrace (char *fmt,...)
* PARAMETRES: chaine contenant le message a ecrire
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier de trace
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiFichierTrace (char *fmt,...)
{
   va_list args;
   char string[1024];

   va_start (args,fmt);
   vsprintf_s(string, sizeof(string), fmt, args);

   DBG_EcritFichierTraces(EMI_TRC, &EMI.dbg, string);

   va_end (args);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void DEFINE_EmiFichierDebug (char *fmt,...)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier d'erreurs
*       et dans le fichier de traces
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DEFINE_EmiFichierDebug (char *fmt,...)
{
   va_list args;
   char string[200];

   va_start (args,fmt);
   vsprintf_s(string, sizeof(string), fmt, args);

   DBG_FILE = EMI_FILE;
   DBG_LINE = EMI_LINE;

   DEFINE_DBG_EcritFichierErreurs(EMI_TRC, &EMI.dbg, string);

   va_end (args);
}
