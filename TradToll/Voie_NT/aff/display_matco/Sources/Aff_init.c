/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions utiles pour l'application
* --------------------------------------------------------------------
* DESCRIPTION: Fonctions AffRessource(), AffLance() et AffArret()
*              qui permettent … une application utilisatrice du module
*              de l'initialiser, de le lancer et de l'arreter.
*              Fonctions permettant l'analse du fichier de formattage
*              et la construction de la liste chain‚e.
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Aff/AFF/SOURCES/Aff_init.c_v  $
 * 
 *    Rev 1.3   Jul 19 2001 16:55:54   sbatiot
 *  
 * 
 *    Rev 1.2   Oct 06 2000 11:55:30   CL
 * Ajout "unit_address" dans le registre (pour GIT dans un premier temps)
 * Premières versions des modules ERIdan et AAU
 * 
 *    Rev 1.1   Mar 31 2000 14:47:04   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.4   Nov 22 1999 10:03:46   nbl
 *  
 * 
 *    Rev 1.3   Nov 17 1999 10:54:42   nbl
 *  
 * 
 *    Rev 1.2   Nov 15 1999 09:37:28   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:40   nbl
 *  
 * 
 *    Rev 1.2   Feb 15 1999 13:54:24   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:44   nbl
 *  
* 
*    Rev 1.3   09 Apr 1998 11:39:04   HMO
*  
* 
*    Rev 1.2   09 Apr 1998 11:32:44   HMO
* - Correction init des services
* - Ajout du nettoyage des services a l'arret
* 
*    Rev 1.1   06 Apr 1998 10:44:40   HMO
*  
* 
*    Rev 1.0   Mar 27 1998 11:13:18   pgg
* Checked in from initial workfile by PVCS Version Manager Project Assistant.
*
* Renamed from SGM to AFF 21.07.1999. sasa - ecsat
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <csrlc32.h>


#include <noyau.h>
#include <trc.h>
#include <debug.h>
#include <serie.h>
#include <err.h>
#include <reg.h>
#include <run.h>
#include <fic_gere.h>

#include <aff_ext.h>
#include <aff_util.h>
#define LOC_DEF
#include <aff_glob.h>
#undef LOC_DEF
#include <aff_serv.h>


/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*
/*--------------- DEFINES: ---------------*/

#define ESPACE         0x20
#define TABULATION     0x09


/*--------------- TYPEDEFS: ---------------*/
/*--------------- VARIABLES:---------------*/

PRIVATE unsigned char compteur_ressource=0;

/*--------------- FUNCTIONS: ---------------*/


// Point d'entrée standard DLL
EXPORT BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk );
PRIVATE enum_instance_result SetCharConversionArray(short int ident_aff, char * pcKey);
PRIVATE BOOL EnvoiArretModule(short int ident_aff);
PRIVATE BOOL ReadMailbox(short int ident_aff, noyau_bal_id m_ArretBalId);
/*--------------- CODE: ---------------*/



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
EXPORT BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
	BOOL bOK = TRUE;

	switch ( dwWhy )
	{

	// Lorsque le process fait le premier LoadLibrary consernant cette DLL
	case DLL_PROCESS_ATTACH :
		break;

	// Lorsque le process fait le dernier FreeLibrary consernant cett DLL
	case DLL_PROCESS_DETACH :
			break;

	}

	return bOK;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AFFLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
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
EXPORT enum_instance_result WINAPI AFFLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{
	short int             ident_aff;
	long                  ligne;
	noyau_enum_retour     cr_lance;
	Ser_enum_cpt_erreur_t cr_port;
    DWORD				  dwNumPort;                        // REG : Numéro de port
	short int             index;
	
	DWORD                 dwLen;
	DWORD                 dwPrioMax;                        // REG : Priorité max taches
	DWORD                 dwPrioInitMax;                    // REG : Priorité max taches initiale

	char                  pcFichierFormat[ MAX_PATH + 1 ];  // REG : Chemin du fichier de format
	char 				  pcPoolName[ MAX_PATH + 1 ];       // REG : Id de pool
	char				  pcPortCfg[ MAX_PATH + 1 ];        // REG : Chaine de config du port serie
    char                  pcProtocol[MAX_PATH];				// Protocol layer
    char                  pcNomTache[MAX_PATH];
	
	// Determiner le numéro de l'instance
	DebutRegion();
	if ( compteur_ressource >= NB_AFFICHEURS_MAX )
	{
		FinRegion();
		return INST_INIT_ERR_MAX_INSTANCE;
	}
	ident_aff = compteur_ressource;
	compteur_ressource ++ ;
	FinRegion();

	/* RAZ structure */
	memset (&AFF[ident_aff],0,sizeof(struct_globale_aff));

	strcpy_s(AFF[ident_aff].pcKey, sizeof(AFF[ident_aff].pcKey), pcKey);
	/* RAZ liste des taches */
	for( index = 0; index < NB_MAX_AFF_PRIO; index++)
		AFF[ident_aff].sScreen.tab_request[index].bal = NO_BAL;
	
	/* initialisation du mode trace */
	if( AffInitTrace( pcBalNam, ident_aff ) != INST_INIT_OK )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	AffFichierTrace(ident_aff,"AFF_INIT pcKey : %s", AFF[ident_aff].pcKey );

	// Vérification de la taille du nom de la BAL ANI
	if ( strlen( pcBalNam ) > MAX_PATH )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Nom de BAL trop long : %s", pcBalNam );
		return INST_INIT_ERR_LANCE;
	}

	// Lecture des entrées de configuration dans le registre
	// Numéro du port série
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PORT, &dwNumPort ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PORT );
		return INST_INIT_ERR_REGISTRE;
	}
    if ( dwNumPort == 0 )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s (la valeur doit être non nulle)", pcKey, MOD_REG_KEYv_PORT );
		return INST_INIT_ERR_REGISTRE;
	}

	// Paramètres du port série
	dwLen = sizeof( pcPortCfg );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PARAMPORT, pcPortCfg, &dwLen ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PARAMPORT );
		return INST_INIT_ERR_REGISTRE;
	}

    // Protocol layer
    dwLen = sizeof( pcProtocol );
    if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PROTOCOL, pcProtocol, &dwLen ) != ERROR_SUCCESS )
    {
        AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PROTOCOL );
		return INST_INIT_ERR_REGISTRE;
    }
    if ( strlen( pcProtocol ) > 0 )
    {
        AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s (la valeur doit être vide)", pcKey, MOD_REG_KEYv_PROTOCOL );
        return INST_INIT_ERR_REGISTRE;
    }

    // Pool
    dwLen = sizeof( pcPoolName );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolName, &dwLen ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_POOL );
		return INST_INIT_ERR_REGISTRE;
	}

	// Priorité initiale max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &dwPrioInitMax ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority( dwPrioInitMax );

	// Priorité max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &dwPrioMax ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX );
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority( dwPrioMax );

	// Chemin du fichier format
	dwLen = sizeof( pcFichierFormat );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_FICHIER_FORMAT, pcFichierFormat, &dwLen ) != ERROR_SUCCESS )
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur registre : [%s]:%s", pcKey, AFF_REG_KEYv_FICHIER_FORMAT );
		return INST_INIT_ERR_REGISTRE;
	}

	// Unit address
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_UNIT_ADDRESS, &AFF[ident_aff].unit_address ) != ERROR_SUCCESS )
	{
		AFF[ident_aff].unit_address = 0;
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Unit address inexistent, set to 0");
	}

	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_DELAY_BETWEEN_CMD, &dwLen ) != ERROR_SUCCESS )
	{
		AFF[ident_aff].delay_command = 0;
	}
	else
		AFF[ident_aff].delay_command = dwLen / 55;

	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_TEXT_FONT, &dwLen ) != ERROR_SUCCESS )
		AFF[ident_aff].dwTextFormat = 2;
	else
		AFF[ident_aff].dwTextFormat = dwLen;

	if(SetCharConversionArray(ident_aff,pcKey)==INST_INIT_ERR_REGISTRE)
	{
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Error with characters conversion table");
		return INST_INIT_ERR_REGISTRE;
	}
	
	if(REG_Lire_Entier(	CSR_REG_KEYi_ROOT,
						pcKey,
						MOD_REG_KEYv_STOP_MAILBOX_TIMEOUT_MS,
						&AFF[ident_aff].stopModuleReadMailboxTimeoutMs) != ERROR_SUCCESS)
	{
		AFF[ident_aff].stopModuleReadMailboxTimeoutMs = 5000;

		AffFichierDebug(ident_aff,
						"AFF_INIT ***** AFFLance() => Error registry : [%s]:%s , value will be set to default %d [ms]",
						pcKey,
						MOD_REG_KEYv_STOP_MAILBOX_TIMEOUT_MS,
						AFF[ident_aff].stopModuleReadMailboxTimeoutMs);
	}

	/* mise a jour des priorites des taches a l'init */

	// Tache IOS
	sprintf_s(pcNomTache, sizeof(pcNomTache), "AFF IOS (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			AFF[ident_aff].taches[0],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(AffIos),
			(PVOID)(ident_aff),
			NULL,
            pcNomTache );

	// Tache ANI
	sprintf_s(pcNomTache, sizeof(pcNomTache), "AFF ANI (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			AFF[ident_aff].taches[1],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(AffAni),
			(PVOID)(ident_aff),
            NULL,
			pcNomTache );

	// Plus d'autre tache
	NOYAU_VIDE_TACHE( AFF[ident_aff].taches[2] );

	// Setting arret_bal to default value
	AFF[ident_aff].arret_bal = -1;

	/* recuperation de la priorite max du module AFF */
	AFF[ident_aff].priorite_max = dwPrioMax;
	
	/* recuperation du numero de pool */
	AFF[ident_aff].pool = NOYAU_GetPoolId( pcPoolName );

	/* recuperation du numero de port */
	AFF[ident_aff].gestion_peripherique.port = (short int)dwNumPort;

	// Semaphores des services initialises
	AFF[ident_aff].sem_service[M_SRV_ARRET]     = SEM_ARRET;
	AFF[ident_aff].sem_service[M_SRV_ETAT]      = SEM_ETAT;
	AFF[ident_aff].sem_service[M_SRV_ESPION]    = SEM_ESPION;
	AFF[ident_aff].sem_service[M_AFF_AFFICHAGE] = SEM_AFFICHAGE;
	AFF[ident_aff].sem_service[M_AFF_VISU]      = SEM_VISU;

	// Nom de la bal ANI
	strcpy_s(AFF[ident_aff].nom_bal_ani, sizeof(AFF[ident_aff].nom_bal_ani), pcBalNam);
	sprintf_s(AFF[ident_aff].nom_bal_ios, sizeof(AFF[ident_aff].nom_bal_ios), "iBAL_AFF_IOS_%d_%d", ident_aff, GetCurrentProcessId());

	/***********MSU********/
	strcpy_s(AFF[ident_aff].pcFichierFormat, sizeof(AFF[ident_aff].pcFichierFormat), pcFichierFormat);
	AFF[ident_aff].position = 1;
	/*********************/

	// Lecture du fichier des formats
	if( AFF_ReadFile(pcFichierFormat,
		AFF[ident_aff].tab_label,
		NB_MAX_LABEL,
		&ligne,
		&AFF[ident_aff].position) == FALSE)
	{
		FreeTabLabel( AFF[ident_aff].tab_label, NB_MAX_LABEL);
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Erreur dans le fichier %s, ligne %ld *****",
			pcFichierFormat,
			ligne);
	    return( INST_INIT_ERR_FICHIER_PARAM );
	}

	// Initialisation du port série :
	cr_port = SerOuvrePort( (short)dwNumPort, pcPortCfg );
	if (cr_port != SER_CPT_OK)
	{ 
		/* Pb d'ouverture du port, redirection dans le fichier d'erreur */
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Echec SerOuvrePort() pour port '%d' *****",
			dwNumPort);
		FreeTabLabel( AFF[ident_aff].tab_label, NB_MAX_LABEL);
		return INST_INIT_ERR_PORT;
	}

    /* initialiser tous les services */
	if ((AFF[ident_aff].service[M_SRV_ETAT] = SrvLance (SRV_ILLIMITE)) == NULL)
        ExitBad();
	if ((AFF[ident_aff].service[M_SRV_ESPION] = SrvLance (SRV_ILLIMITE)) == NULL)
        ExitBad();
	if ((AFF[ident_aff].service[M_SRV_ARRET] = SrvLance (1)) == NULL)
		ExitBad();
	if ((AFF[ident_aff].service[M_AFF_AFFICHAGE] = SrvLance (SRV_ILLIMITE)) == NULL)
        ExitBad();
	if ((AFF[ident_aff].service[M_AFF_VISU] = SrvLance (SRV_ILLIMITE)) == NULL)
        ExitBad();
	if ((AFF[ident_aff].service[M_SRV_DEVICE_INFO] = SrvLance (SRV_ILLIMITE)) == NULL)
		ExitBad();

	// Timer initialization
	if (AlloueChrono(&AFF[ident_aff].uiChronoTest,
					"TEST DISPLAY") != NOYAU_OK)
		return INST_INIT_ERR_LANCE;

	/* lancement des 2 taches */
	cr_lance = LanceTache( AFF[ident_aff].taches );
	if (cr_lance != NOYAU_OK)
	{
		FreeTabLabel( AFF[ident_aff].tab_label, NB_MAX_LABEL);
		return INST_INIT_ERR_LANCE;
	}
	
    /* Initialisation des BAL du module */
    *piBalId = AttendBAL( pcBalNam );
    if( *piBalId <= 0 )
    {
		AffFichierDebug(ident_aff, "AffLance: Pb InitBal() pour AFF");
		return INST_INIT_ERR_LANCE;
    }
	
	/* redirection trace */
	AffFichierTrace(ident_aff,"AFF_INIT: AffLance() => init OK ");
	

	return INST_INIT_OK;
	
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
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
{
    return AFFLance( pcKey, pcBalName, piBalId );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI AFFArret ( IN noyau_bal_id iBalId )
 * PARAMETERS: IN noyau_bal_id iBalId : Id de la bal identifiant l'instance
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour arret
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI AFFArret(noyau_bal_id iBalId)
{
    short int             ident_aff;
    Ser_enum_cpt_erreur_t cpt;

    /* recherche en fonction du nombre de lecteur lance */
    for (ident_aff = 0; ident_aff < NB_AFFICHEURS_MAX; ident_aff++)
    {
		if(AFF[ident_aff].ani_bal == iBalId)
			break;
    }
	
    if( ident_aff > NB_AFFICHEURS_MAX )
		/* Imprimante non trouve */
		return  INST_ARRET_NOK;

	// Stop thread(s). If stopping thread(s) through mailbox fails, stop it immediately.
	if (!EnvoiArretModule(ident_aff))
	{
		AffFichierDebug(ident_aff, "AFF_INIT ***** AFFArret() => EnvoiArretModule failed! *****");

		if (ArretTaches(AFF[ident_aff].taches) == NOYAU_ARRET_TACHE_NOK)
			return INST_ARRET_NOK;
	}

	// Free timer
	LibereChrono(&AFF[ident_aff].uiChronoTest);

	/* on ferme le port s‚rie utilis‚ */
	cpt = SerFermePort (AFF[ident_aff].gestion_peripherique.port);
	if( cpt != SER_CPT_OK )
		AffFichierDebug(ident_aff,"AFF_INIT ***** AffArret() => Echec FermePort() port '%d' *****",
		AFF[ident_aff].gestion_peripherique.port);
	// lib‚rer la m‚moire occup‚e par la liste chain‚e
	FreeTabLabel(AFF[ident_aff].tab_label, NB_MAX_LABEL);
	AFF_MsgRequestFreeTab(&AFF[ident_aff].sScreen);
	
	// Remove Mailboxes
    //SupprimeBAL (AFF[ident_aff].nom_bal_ani);	// will be done in ANI thread
    //SupprimeBAL (AFF[ident_aff].nom_bal_ios);	// will be done in IOS thread

	// liberer les services
    if (SrvArret(&AFF[ident_aff].service[M_SRV_ETAT]) == FALSE)
        ExitBad();
    if (SrvArret(&AFF[ident_aff].service[M_SRV_ESPION]) == FALSE)
        ExitBad();
	if (SrvArret(&AFF[ident_aff].service[M_SRV_ARRET]) == FALSE)
		ExitBad();
    if (SrvArret(&AFF[ident_aff].service[M_AFF_AFFICHAGE]) == FALSE)
        ExitBad();
    if (SrvArret(&AFF[ident_aff].service[M_AFF_VISU]) == FALSE)
        ExitBad();
	if (SrvArret(&AFF[ident_aff].service[M_SRV_DEVICE_INFO]) == FALSE)
		ExitBad();

	
	/* Arret du mode DEBUG */
	if (DBG_Arret(&AFF[ident_aff].dbg) != DBG_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	/* retourner la valeur de l'arret des taches */
	return INST_ARRET_OK;
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
    return AFFArret( iBalId );
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result AffInitTrace (char * pcBal, short int cpt_ressource)
* PARAMETRES:
*     cpt_ressource : index de l'instance
*     pcBal : nom de la bal d'interface
*     retour: compte-rendu
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: fonction qui initialise le mode DEBUG associ‚ au noyau 6.00 :
*       un fichier sp‚cifique de trace pour chaque instance
*       du module doit etre present dans le r‚pertoire C:\TRACES !!!
*       un fichier sp‚cifique d'erreurs pour chaque instance
*       du module doit etre present dans le r‚pertoire C:\ERREURS !!!
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_instance_result AffInitTrace (char * pcBal, short int cpt_ressource )
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
	dbg_struct_debug *debug;
	dbg_struct_trace *tab_traces;

	debug = &AFF[cpt_ressource].dbg;
	tab_traces = &AFF[cpt_ressource].tab_traces[AFF_TRC];

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
	// chaque instance du module AFF aura son propre
    // fichier donné par son nom de bal (unique)
	strcpy_s(debug->nom_fichier_traces_ecran, sizeof(debug->nom_fichier_traces_ecran), pcBal);

	// chaque instance du module AFF aura également son propre
    // fichier d'erreurs donné par son nom de bal (unique)
	strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), pcBal);

	// chaque instance du module AFF aura son propre fichier de trace
	strcpy_s(tab_traces->nom, sizeof(tab_traces->nom), pcBal);

	debug->tab_traces = tab_traces;
	debug->nb_fichiers_traces = AFF_NB_TRACES;
	
	if( DBG_Lance(debug) != DBG_OK )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	return INST_INIT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AffFichierTrace (short int ident_aff,char *fmt,...)
* PARAMETRES:
*    entree : identificateur de l'imprimante
*           : texte … ecrire dans le fichier
*    retour : rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier de TRACES
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AffFichierTrace (short int ident_aff,char *fmt,...)
{
	va_list args;
	CHAR szString[1024] = {0};

	va_start(args, fmt);
	_vsnprintf_s(szString, sizeof(szString), sizeof(szString), fmt, args);

	TRC_Trace_V(AFF[ident_aff].dbg.tab_traces->emet, TRC_OPT_MASK, NULL, 0, fmt, args);

	va_end(args);	
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AffFichierDebug (short int ident_aff,char *fmt,...)
* PARAMETRES:
*    entree : identificateur de l'imprimante
*           : texte … ecrire dans le fichier
*    retour : rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier d'erreurs
*       et dans le fichier de traces
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DEFINE_AffFichierDebug (short int ident_aff,char *fmt,...)
{
	va_list args;
	char string[1024];
	
	va_start (args,fmt);
	vsprintf_s(string, sizeof(string), fmt, args);
	
	DBG_FILE = AFF_FILE;
	DBG_LINE = AFF_LINE;
	
	DEFINE_DBG_EcritFichierErreurs(AFF_TRC, &AFF[ident_aff].dbg, string);
	
	va_end (args);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result SetCharConversionArray(short int ident_aff, char * pcKey)
* PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
*           : 
*    retour : 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: 
* ROLE: Sets charset conversion array from registry
*       
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE enum_instance_result SetCharConversionArray(short int ident_aff, char * pcKey)
{


	char SubKey[MAX_PATH];
	char KeyFound[MAX_PATH];
	char Valeur[MAX_PATH];
	DWORD result;
	DWORD Index;
	DWORD KeySize,SizeVal;
	int nAsciFrom;
	int nAsciTo;

	int n;

	// initialize conversion_array to default valuees

	for( n = 0; n <= 255; n++)
	{
       AFF[ident_aff].char_conv_array[n]=n;
	   AFF[ident_aff].inv_char_conv_array[n]=n;
	}

	// generate subkey
	sprintf_s(SubKey, sizeof(SubKey), "%s\\%s",pcKey,AFF_REG_KEYv_CHARSET_CONVERSION);

	Index=0;

	while(TRUE){
		
		SizeVal=sizeof(Valeur);
		KeySize=sizeof(KeyFound);
		
		result=REG_Enum_Valeurs_Chaine(CSR_REG_KEYi_ROOT,SubKey,Index,KeyFound,&KeySize,Valeur,&SizeVal);
	        		
	
		if(result!=ERROR_SUCCESS)break;
		
		/* Le periph est inactif passe au suivant */
		if(!KeySize){
			Index++;
			continue;
		}
		
		nAsciFrom=atoi(KeyFound);
		nAsciTo=atoi(Valeur);
		
		if(nAsciFrom>0 && nAsciFrom<256 && nAsciTo>0 && nAsciTo<256)
		{
			AFF[ident_aff].char_conv_array[nAsciFrom]=nAsciTo;
		}
		else
		{	
			AffFichierDebug(ident_aff,"AFF_INIT ***** SetCharConversionArray() => Bad character conversion table");
			return INST_INIT_ERR_REGISTRE;

		}
		
		Index++;
	}

	// building inverse charset conversion array
	for( n = 0; n <= 255; n++)
	{
	   AFF[ident_aff].inv_char_conv_array[AFF[ident_aff].char_conv_array[n]]=n;
	}


	return INST_INIT_OK;
}

/**/
/*******************************************************************/
/*SYNTAX: BOOL EnvoiArretModule(short int ident_aff)			   */
/*=================================================================*/
/*TYPE:   Local function.	                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function opens mailbox, sends ARRET message to  */
/*			  ANI thread and receives response from it.			   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int ident_aff			   - Instance ID.              */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  BOOL                  If module stops properly function		   */
/*						  returns TRUE, otherwise returns FALSE.   */
/*******************************************************************/
PRIVATE BOOL EnvoiArretModule(short int ident_aff)
{
	BOOL				bRet					= FALSE;
	char				m_ArretBalName[MAX_PATH]= { 0 };
	noyau_bal_id		m_ArretBalId			= 0;
	struct_aff_message  *p_msg_emis				= NULL;
	
	// ********** OPENING MAILBOX TO SIMULATE APPLICATION **********
	// Make ArretBalName according to iInstanceIdx and thread ID that duplicate can't be made.
	sprintf_s(m_ArretBalName, _countof(m_ArretBalName), "AFFArret_iIdx%d_thread%d", ident_aff, GetCurrentThreadId());
	m_ArretBalId = PublieBAL(m_ArretBalName, NOYAU_BAL_ILLIMITEE);

	// ****** ALLOCATING MEMORY AND FILLING MESSAGE STRUCTURE ******
	ExitAlloue((struct_neutre **)(&p_msg_emis),
				sizeof(struct_aff_message),
				NOYAU_GetPoolId(NULL));

	p_msg_emis->entete.service = M_SRV_ARRET;
	p_msg_emis->entete.type_message = SRV_TYP_DEMANDE;
	// Setting message to 0 because it's not used. For security only.
	memset(&p_msg_emis->u.srv_arret, 0, sizeof(p_msg_emis->u.srv_arret));

	if(Envoie(AFF[ident_aff].ani_bal, m_ArretBalId, (struct_neutre *)(p_msg_emis))!=NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
	else
	{
		bRet = ReadMailbox(ident_aff, m_ArretBalId);
	}

	SupprimeBAL(m_ArretBalName);

	return bRet;
}

/**/
/*******************************************************************/
/*SYNTAX: BOOL ReadMailbox(IN dcp_inst_id iInstanceIdx,			   */
/*						   IN noyau_bal_id m_ArretBalId)		   */
/*=================================================================*/
/*TYPE:   Local function.	                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function waits for response to ARRET message	   */
/*			  that has been sent before calling this function.	   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int ident_aff			   - Instance ID.              */
/*		noyau_bal_id m_ArretBalId	   - Bal ID which ARRET message*/
/*										 has been sent from		   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  BOOL                  If module stops properly function		   */
/*						  returns TRUE, otherwise returns FALSE.   */
/*******************************************************************/
PRIVATE BOOL ReadMailbox(short int ident_aff, noyau_bal_id m_ArretBalId)
{
	BOOL				bRet		= FALSE;
	noyau_enum_retour	code_rtc	= 0;
	struct_neutre		*p_neutre	= NULL;
	struct_aff_message  *p_msg_rec	= NULL;

	code_rtc = RecoitMs(m_ArretBalId,
						(struct_neutre **)(&p_neutre),
						AFF[ident_aff].stopModuleReadMailboxTimeoutMs);

	if (code_rtc == NOYAU_BAL_MESS)
	{
		if (p_neutre->bl_retour == AFF[ident_aff].ani_bal)
		{
			p_msg_rec = (struct_aff_message*)p_neutre;
			switch (p_msg_rec->entete.type_message)
			{
				case SRV_TYP_ARRET_ACQ:
					bRet = ReadMailbox(ident_aff, m_ArretBalId);
					break;

				case SRV_TYP_ARRET_EFFECTUE:
					bRet = TRUE;
					break;

				case SRV_TYP_ARRET_NACQ:
				default:
					bRet = FALSE;
					break;
			}				
		}
		else
		{
			// Undefined message
			bRet = FALSE;

			AffFichierDebug(ident_aff, "*** ReadMailbox() - message received from undefined mail box %d ***",
				p_neutre->bl_retour);
		}
		// Free the message in the mailbox
		ExitLibere((struct_neutre **)(&p_neutre));
	}

	return bRet;
}
/*--------------------------- END OF FILE -------------------------*/