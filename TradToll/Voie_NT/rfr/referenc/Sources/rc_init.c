/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Tache REFERENCE
* FICHIER: RC_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Reception du fichier de reference
*         Code des fonctions utiles pour l'application
* --------------------------------------------------------------------
* DESCRIPTION: Fonctions
*              - ReferenceRessource(),
*              - ReferenceLance()
*              - ReferenceArret()
*              qui permettent a une application utilisatrice du module
*              de l'initialiser, de le lancer et de l'arreter.
*
*              NON IMPLENTE
*              Fonctions ReferenceDebutDebug() et ReferenceFinDebug()
*              pour ecrire
*              dans un fichier les erreurs survenues.
*              Fonctions ReferenceDebutTrace() et ReferenceFinTrace()
*               pour ecrire
*              dans un fichier les traces utiles lors de l'integration
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Referenc/Sources/rc_init.c_v  $
 * 
 *    Rev 1.1   24 Sep 2001 15:39:42   FROUGIET
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:52   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.5   08 Dec 1997 11:26:08   DPI
 * Correction bug printf
 * 
 *    Rev 1.4   03 Nov 1997 16:07:30   DPI
 * Version 4.00
 * 
 *    Rev 1.3   29 Oct 1997 19:28:48   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 *    Rev 1.2   29 Oct 1997 19:13:54   DPI
 * Suppression de l'include "noyau2.h"
 *
 *    Rev 1.1   Jun 04 1997 19:13:14   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:22   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <dos.h>
#include <stdarg.h>
#include <io.h>
#include <conio.h>
#include <string.h>

/* module NOYAU */
#include <csrlc32.h>
#include "noyau.h"
#include <debug.h>
#include <run.h>
#include <reg.h>
#include <trc.h>

#include "fic.h"
#include "rc_def.h"
#include "fic_gere.h"

#include "referenc.h"

#define LOC_DEF
#include "rc_glob.h"
#undef LOC_DEF
#include "rc_loc.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

#define MAXCHAINE 127
#define RFR_WAIT_INIT_EVT_NAME "EVT_RFR_WAIT_INIT"

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:

			/* raz structure */
			memset (&RFR,0,sizeof(RFR));

			/* Initialisation non effectuee (faite par ReferenceLance()) */
			RFR.init = FALSE;

			// Creation de l'evt pour gestion initialisation du module
			if (AlloueEvent (&RFR.wait_init_evt, RFR_WAIT_INIT_EVT_NAME) != NOYAU_OK)
				ExitBad();
			
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  

			LibereEvent (&RFR.wait_init_evt);

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
	return ReferenceLance (pcKey, pcBalNam, piBalId);
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
	return ReferenceArret (iBalId);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC enum_rfr_retour ReferenceLance(struct_rfr_congig *p_config)
* PARAMETRES:
*     entree: config regroupe :
*           :     - le numero de pool du module
*           :     - la priorite max d'initialisation des taches du module
*           :     - la priorite max que peuvent prendre les taches ensuite
*     retour: compte-rendu du lancement des taches
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale
* ROLE: fonction qui permet d'initialiser et de lancer les taches
*       relatives au module REFERENCE
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT enum_instance_result WINAPI ReferenceLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{
	noyau_enum_retour	cr_lance;
    char				NomTache[MAX_PATH + 1];
	
    DWORD               dwLen;
	
	char				pcPoolId[MAX_PATH + 1];			// REG : Chaine de config du pool
	DWORD               dwPrioMax;                      // REG : Priorité max taches
	DWORD               dwPrioInitMax;                  // REG : Priorité max taches initiale
	DWORD               dwColdStart;                    // REG : Booleen 'cold start'
	char				pcBalFileName[MAX_PATH + 1];
	char				pcBalMsgName[MAX_PATH + 1];
	char				pcBalHrdName[MAX_PATH + 1];
	char *field_cold_start = "cold_start";
	char *field_bl_file_name = "bl_file_name";
	char *field_bl_msg_name = "bl_msg_name";
	char *field_bl_hrd_name = "bl_hrd_name";
	char *field_hostname	= "hostname";
	char *field_distant_path = "distant_path";
	char *sleep_duration_after_init_sec = "sleep_duration_after_init_sec";
	

/* ###
#define           PATH_TRANSFERT        "D:\\transfer"
#define           PATH_REFERENCE        "\\referenc"
#define           DISQUE                "C"
#define           DISQUE_VIRTUEL        "D"
*/
	char *field_path_transfert = "path_transfert";
	char *field_path_reference = "path_reference";
	char *field_local_disk = "local_disk";
	char *field_virtual_disk = "virtual_disk";

/* ###
#define  FICHIER_CONFIG               DISQUE":"PATH_REFERENCE"\\conf_rfr"
#define  FICHIER_CONFIG_COURANT       FICHIER_CONFIG".cur"
#define  FICHIER_CONFIG_TMP           FICHIER_CONFIG".tmp"
#define  FICHIER_REFERENCE_MANUELLE   DISQUE":"PATH_REFERENCE"\\manuel.txt"
*/
	char pcFileName[MAX_PATH + 1];
	char *field_fichier_config = "fichier_config";
	char *field_fichier_reference_manuelle = "fichier_reference_manuelle";
	// MFR start 2007/5/24
	char *szTftMailboxName = "bl_tft";
	// MFR end 2007/5/24

	/* initialisation du mode trace */
	if( ReferenceInitTrace (pcBalNam) != INST_INIT_OK )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	// Vérification de la taille du nom de la BAL
	if ( strlen( pcBalNam ) > MAX_PATH )
	{
		ReferenceFichierDebug("RC_INIT ***** ReferenceLance() => Nom de BAL trop long : %s", pcBalNam );
		return INST_INIT_ERR_LANCE;
	}

	/*****************************************************************/
	/* [debut] Lecture des entrées de configuration dans le registre */
	/*****************************************************************/
	// Pool
	dwLen = sizeof( pcPoolId );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolId, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_POOL );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Priorité initiale max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &dwPrioInitMax ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);
	
	// Priorité max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &dwPrioMax ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority(dwPrioMax);
	
	// Booleen 'cold start'
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, field_cold_start, &dwColdStart ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_cold_start );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (dwColdStart > 1) /* 'cold start' doit etre 0 ou 1 */
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur valeur registre : [%s]:%s", pcKey, field_cold_start );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// BAL module File (LAN)
	dwLen = sizeof( pcBalFileName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_bl_file_name, pcBalFileName, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_bl_file_name );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// BAL module Msg (message = COM (LS))
	dwLen = sizeof( pcBalMsgName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_bl_msg_name, pcBalMsgName, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_bl_msg_name );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// BAL module Hrd (Horodate)
	dwLen = sizeof( pcBalHrdName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_bl_hrd_name, pcBalHrdName, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_bl_hrd_name );
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Repertoire temporaire pour les transferts de fichiers */
	dwLen = sizeof( RFR.path_transfert );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_path_transfert, RFR.path_transfert, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_path_transfert );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (RFR.path_transfert) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_path_transfert );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// Repertoire de travail pour le module Reference */
	dwLen = sizeof( RFR.path_reference );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_path_reference, RFR.path_reference, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_path_reference );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (RFR.path_reference) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_path_reference );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// Unite de disque locale */
	dwLen = sizeof( RFR.disque );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_local_disk, RFR.disque, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_local_disk );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (RFR.disque) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_local_disk );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// Disque virtuel */
	dwLen = sizeof( RFR.disque_virtuel );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_virtual_disk, RFR.disque_virtuel, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_virtual_disk );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (RFR.disque_virtuel) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_virtual_disk );
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// Fichier de config (nom + extensions pour en cours et temporaire */
	dwLen = sizeof( pcFileName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_fichier_config, pcFileName, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_fichier_config );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (pcFileName) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_fichier_config );
			return INST_INIT_ERR_REGISTRE;
		}
		else
		{
			/* Chemin/nom fichier config (sans extension) */
			FIC_makepath (RFR.fichier_config , RFR.disque, RFR.path_reference, pcFileName, NULL);
//			sprintf (RFR.fichier_config, "%s:%s\\%s", RFR.disque, RFR.path_reference, pcFileName);
			/* Chemin/nom fichier config en cours */
			sprintf_s(RFR.fichier_config_courant, sizeof(RFR.fichier_config_courant), "%s.cur", RFR.fichier_config);
			/* Chemin/nom fichier config temporaire */
			sprintf_s(RFR.fichier_config_tmp, sizeof(RFR.fichier_config_tmp), "%s.tmp", RFR.fichier_config);
		}
	}
	
	// Fichier de reference manuelle */
	dwLen = sizeof( pcFileName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_fichier_reference_manuelle, pcFileName, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_fichier_reference_manuelle );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (strlen (pcFileName) == 0)
		{
			ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Entree registre vide : [%s]:%s", pcKey, field_fichier_reference_manuelle );
			return INST_INIT_ERR_REGISTRE;
		}
		else
		{
			/* Chemin/nom fichier reference manuelle */
//			sprintf (RFR.fichier_reference_manuelle, "%s:%s\\%s",
//				RFR.disque, RFR.path_reference, pcFileName);
			FIC_makepath(RFR.fichier_reference_manuelle, RFR.disque, RFR.path_reference, pcFileName, NULL);
		}
	}

	// Host
	dwLen = sizeof( RFR.hostname );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_hostname, RFR.hostname, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_hostname );
		return INST_INIT_ERR_REGISTRE;
	}

	//Distant path
	dwLen = sizeof( RFR.distant_path );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, field_distant_path, RFR.distant_path, &dwLen ) != ERROR_SUCCESS )
	{
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s", pcKey, field_distant_path );
		return INST_INIT_ERR_REGISTRE;
	}

	// Temps d'endormissement apres init et avant prise en compte des messages dans la BAL
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, sleep_duration_after_init_sec, &RFR.sleep_duration_after_init_sec ) != ERROR_SUCCESS )
	{
		RFR.sleep_duration_after_init_sec = 2;
		ReferenceFichierDebug ("RC_INIT ***** ReferenceLance() => Erreur registre : [%s]:%s (Valeur par défaut=%d sec)", pcKey, sleep_duration_after_init_sec,RFR.sleep_duration_after_init_sec );
	}
	if ( RFR.sleep_duration_after_init_sec < 2 )
		RFR.sleep_duration_after_init_sec = 2;

	// MFR start 2007/5/24
	// Get name for TFT maibox key
	dwLen = RFR_MAX_PATH;
	if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, szTftMailboxName, RFR.szTftMailboxName, &dwLen ) != ERROR_SUCCESS )
	{
		sprintf_s(RFR.szTftMailboxName, sizeof(RFR.szTftMailboxName), "BL_TFT");
	}
	else
	{
		if( strlen( RFR.szTftMailboxName ) == 0 )
			sprintf_s(RFR.szTftMailboxName, sizeof(RFR.szTftMailboxName), "BL_TFT");
	}
	// MFR end 2007/5/24

	
	/***************************************************************/
	/* [fin] Lecture des entrées de configuration dans le registre */
	/***************************************************************/
	
	/* Initialisation thread REFERENCE */
	
	sprintf_s(NomTache, sizeof(NomTache), "REFERENCE (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
		RFR.taches[0],
		TRUE,
		dwPrioInitMax,
		8096,
		(LPTHREAD_START_ROUTINE)(Reference),
		(PVOID)NULL,
		NULL,
		NomTache);
	
	// Plus d'autre tache
	NOYAU_VIDE_TACHE( RFR.taches[RFR_NB_TACHE] );
	
     /* Publication du numero BAL de REFERENCE */
   RFR.util.BalLocale = PublieBAL (pcBalNam, 0);
   if (RFR.util.BalLocale <= 0)
      ExitBad();
   else
	   *piBalId = RFR.util.BalLocale;

	/* Initialisation effectuee */
	RFR.init = TRUE;

   /* Priorite max du module RFR */
   RFR.util.PrioriteMax = (short)dwPrioMax;

   /* Numero de pool */
   RFR.util.NumeroPool = NOYAU_GetPoolId(pcPoolId);

   /* bal d'interface avec le module Horodate (HRD) */
   RFR.util.BalHorodate = AttendBAL( pcBalHrdName );
   if( RFR.util.BalHorodate <= 0 )
   {
	   ReferenceFichierDebug ("ReferenceLance: Pb Init Bal Horodate()");
	   return INST_INIT_ERR_LANCE;
   }

   /* bal d'interface avec la tache de chargement des fichiers (module LAN) */
   RFR.util.BalFichier = AttendBAL( pcBalFileName );
	if( RFR.util.BalFichier <= 0 )
	{
		ReferenceFichierDebug ("ReferenceLance: Pb Init Bal File()");
		return INST_INIT_ERR_LANCE;
	}

   /* bal d'interface avec la tache de transfert des fichiers vers le LS (module COM_LS) */
   RFR.util.BalMessage = AttendBAL( pcBalMsgName );
	if( RFR.util.BalMessage <= 0 )
	{
		ReferenceFichierDebug ("ReferenceLance: Pb Init Bal Msg()");
		return INST_INIT_ERR_LANCE;
	}

   /* sommes-nous en cold start */
   RFR.util.ColdStart = (boolean)dwColdStart;

   /* lancement de la tache */
   cr_lance = LanceTache(RFR.taches);
   if( cr_lance != NOYAU_OK )
		return INST_INIT_ERR_LANCE;

	/* Tracer succes init */
	ReferenceFichierTrace ("RC_INIT: ReferenceLance() => init OK ");
	
	return INST_INIT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: EXPORT enum_instance_result WINAPI ReferenceArret(noyau_bal_id iBalId)
* PARAMETRES:
*     entree: numero de boite aux lettres
*     retour: compte-rendu de l'arret des taches
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale
* ROLE: fonction qui permet d'arreter les taches du module REFERENCE
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT enum_instance_result WINAPI ReferenceArret(noyau_bal_id iBalId)
{
   noyau_enum_retour cr_arret ;

   /* arret des taches associees au module */
   cr_arret = ArretTaches(RFR.taches);

   /* si l'arret s'est mal passe */
   if (cr_arret == NOYAU_ARRET_TACHE_NOK)
		return INST_ARRET_NOK;

	/* Arret du mode DEBUG */
   if (ReferenceDeinitTrace() != INST_INIT_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;
	 
	/* retourner la valeur de l'arret des taches */
	return INST_ARRET_OK;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT boolean WINAPI RFRInitOK ( void )
 * PARAMETERS: void : 
 * RETURN    : TRUE = RFR est initialise, FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Indiquer si RFR est initialisee
 *				L'initialisation se fait par ReferenceLance() qui met
 *				a jour des variables via le registre
 * --------------------------------------------------------------------
 */
EXPORT boolean WINAPI RFRInitOK( void )
{
	return RFR.init;
}
//------------------------------------------------------------------------------------



PRIVATE int CreatePath(char *path)
{
	char *token;
	char full_path[_MAX_PATH];


	if (_fullpath(full_path, path, _MAX_PATH) == NULL)
		return FALSE;

	// recherche du premier '\' normalement juste apres le ':' avec un path absolu
	token = strchr(full_path, '\\');
	token++;

	// création des sous-répertoires si inexistant
	while ((token = strchr(token, '\\')) != NULL)
	{
		*token = '\0';

		if (_access(full_path, 0) != 0)
			if (!CreateDirectoryA(full_path, NULL))
			{
				*token = '\\';
				return FALSE;
			}

		*token = '\\';
		token++;
	}

	// création du répertoire final si inexistant
	if (_access(full_path, 0) != 0)
		if (!CreateDirectoryA(full_path, NULL))
			return FALSE;

	return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result ReferenceInitTrace (char * pcBal)
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
PROTECTED enum_instance_result ReferenceInitTrace (char * pcBal)
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
    dbg_struct_debug *debug;
    dbg_struct_trace *tab_traces;
	char				path[MAX_PATH] = { 0 };

    debug = &RFR.dbg;
    tab_traces = &RFR.tab_traces[RFR_TRC];

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
	    
	/* creation du chemin complet vers le fichier d'erreurs */
	_snprintf_s(path, sizeof(path), sizeof(path), "%s\\%s" NOYAU_EXTENSION_ERREURS, debug->rep_fichier_erreurs, pcBal);
	strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), path);

	/* creation du chemin complet vers le fichier de traces avec ajout de l'extension par defaut */
	_snprintf_s(tab_traces->nom, sizeof(tab_traces->nom), sizeof(tab_traces->nom), "%s\\%s" NOYAU_EXTENSION_TRACES, debug->rep_fichiers_traces, pcBal);

	/* creation du repertoire d'erreurs */
	if (_access(debug->rep_fichier_erreurs, 0) != 0)
	{
		if (!CreatePath(debug->rep_fichier_erreurs))
			return INST_INIT_ERR_FICHIER_DEBUG;
	}

	/* creation du repertoire de traces */
	if (_access(debug->rep_fichiers_traces, 0) != 0)
	{
		if (!CreatePath(debug->rep_fichiers_traces))
			return INST_INIT_ERR_FICHIER_DEBUG;
	}

	/* initialise le fichier de debug */
	TRC_Initialise_Trace("DEBUG", debug->nom_fichier_erreurs, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_TEXTE_SEUL, (TRC_EMETTEUR *)&debug->priv.emet);
	TRC_Taille_Max_Fichier(debug->priv.emet, debug->taille_limite);

	/* teste le fichier de debug */
	TRC_Trace_Texte(debug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, " => Debut du debug");

	TRC_Initialise_Trace("TRACE", tab_traces->nom, TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *)&tab_traces->emet);
	TRC_Taille_Max_Fichier(tab_traces->emet, debug->taille_limite);

   return INST_INIT_OK;
}


PROTECTED enum_instance_result ReferenceDeinitTrace(void)
{
	TRC_Termine_Trace(RFR.dbg.priv.emet);

	TRC_Termine_Trace(RFR.tab_traces[RFR_TRC].emet);

	return INST_INIT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ReferenceFichierTrace (char *fmt,...)
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
PROTECTED void ReferenceFichierTrace (char *fmt,...)
{
	va_list args;

	va_start(args, fmt);

	TRC_Trace_V(RFR.tab_traces[RFR_TRC].emet, TRC_OPT_MASK, NULL, 0, fmt, args);

	va_end(args);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ReferenceFichierDebug (char *fmt,...)
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
PROTECTED void DEFINE_ReferenceFichierDebug (char *fmt,...)
{
	va_list				args;
	dbg_struct_debug	*pDebug;

	pDebug = &RFR.dbg;

	va_start(args, fmt);

	TRC_Trace_V(RFR.tab_traces[RFR_TRC].emet, TRC_OPT_MASK, NULL, 0, fmt, args);
	TRC_Direct_Trace_V(pDebug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, NULL, 0, fmt, args);

	va_end(args);
}
