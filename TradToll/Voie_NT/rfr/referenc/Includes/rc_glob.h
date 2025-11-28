/*------   (v) 1997 CS-Route  ------------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: REFERENCE
* FICHIER: rc_glob.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier interne au module
*         Tache Reception du fichier de reference
* --------------------------------------------------------------------
* DESCRIPTION: Declarations de constantes et variables globales
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_glob.h_v  $
 * 
 *    Rev 1.1   24 Sep 2001 15:37:22   FROUGIET
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:02   DPI
 * Version 4.00
 * 
 *    Rev 1.2   29 Oct 1997 19:14:18   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.1   29 Oct 1997 18:53:00   DPI
 * Creation du deuxieme time-out de tache
 * 
 *    Rev 1.0   Apr 24 1997 14:59:36   DPI
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef RC_GLOB_H
#define RC_GLOB_H

/*--------------- INCLUDES: ---------------*/
#include <noyau.h>
#include <debug.h>


#include <referenc.h>
/* inclusion des constantes de r‚f‚rence */
#include "rfr_cnst.h"

#include <protect.h>
/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

/*--------------- CONSTANTES ---------------*/
/* pour les traces du module */
enum index_traces
{
   RFR_TRC,
   RFR_NB_TRACES
};
#define ReferenceFichierDebug RFR_FILE=__FILE__,\
                        RFR_LINE=__LINE__,\
                        DEFINE_ReferenceFichierDebug

/*-----Fonctionnel-------------*/
#define  FICHIER_REFERENCE_EXT        "txt"
#define  FICHIER_REFERENCE_TMP        "ref_cree""."FICHIER_REFERENCE_EXT
#define  FICHIER_REFERENCE_COURANT    "ref_cour""."FICHIER_REFERENCE_EXT

/*
#define  FICHIER_CONFIG               DISQUE":"PATH_REFERENCE"\\conf_rfr"
#define  FICHIER_CONFIG_COURANT       FICHIER_CONFIG".cur"
#define  FICHIER_CONFIG_TMP           FICHIER_CONFIG".tmp"
#define  FICHIER_REFERENCE_MANUELLE   DISQUE":"PATH_REFERENCE"\\manuel.txt"
*/

#define  FICHIER_CONFIG				RFR.fichier_config
#define  FICHIER_CONFIG_COURANT		RFR.fichier_config_courant
#define  FICHIER_CONFIG_TMP			RFR.fichier_config_tmp
#define  FICHIER_REFERENCE_MANUELLE	RFR.fichier_reference_manuelle

/*-----Temps r‚el-------------*/

//#define RFR_TACHE_HORLOGE                 -2
#define RFR_NB_BAL                        1
#define RFR_NB_CHRONO                     0

#define RFR_TIMEOUT_RAPIDE                9
#define RFR_TIMEOUT_LENT                180

//#define RFR_DUREE_PREMIER_CHRONO          18  //(1s)
//#define RFR_DUREE_CHRONO_RAPIDE         RFR.util.duree_chrono / 10

//#define RFR_EVT_REPONSE_FTP              1

enum
{
   RFR_NON_VALIDE = 0,
   RFR_VALIDE
};

enum
{
   RFR_LIBRE = 0,
   RFR_AUCUN = RFR_LIBRE,
   RFR_EN_COURS
};


/*----------------TYPEDEF:----------------*/
/*--------------- STRUCTURE DE TACHE ---------------*/

typedef enum
{
   ACTION_START,
   NO_ACTION_TO_DO,
   ACTION_MSG_SERVICE_FICHIER,
   ACTION_MSG_REFERENCE,
   ACTION_MSG_MANUEL,
   ACTION_MSG_FICHIER_CHARGER,
   ACTION_MSG_FICHIER_NON_CHARGER,
   ACTION_MSG_APPLICATIF,
   ACTION_MSG_ALARME,
   ACTION_MSG_TERMINATE,
   ACTION_TIME
} TEnum_Action;

typedef enum
{
   START_REFERENCE,

   REFERENCE_MESSAGE,
   REFERENCE_MESSAGE_FOR_FUTURE,

   MANUAL_CHANGE,

   FAILURE_LOAD_REFERENCE_FILE,
   FAILURE_LOAD_FILE,
   REFERENCE_FILE_LOADED,
   FILE_LOADED,
   NO_FILE_LOADED,
   COMPARE_REFERENCE_FILE,

   DO_RECEPTION_ACTION,
   CLOSE_RECEPTION_ACTIONS,

   RECEIVE_ALARM_ACTION,
   DO_ALARM_ACTION,
   CLOSE_ALARM_ACTIONS,

   RECEIVE_MESSAGE_ACTION,
   DO_MESSAGE_ACTION,
   CLOSE_MESSAGE_ACTIONS,

   TERMINATE_REFERENCE,
   NOTHING
} TEnum_Event;
/* ces events sont ceux qui se produisent pour la tache
 * START_REFERENCE              - d‚marrage de la tache
 * REFERENCE_MESSAGE            - reception du message de r‚f‚rence
 * REFERENCE_MESSAGE_FOR_FUTURE - reception du message de r‚f‚rence
 *                                alors qu'on en traite un autre
 * FAILURE_LOAD_REFERENCE_FILE  - echec au chargement du fichier de r‚f‚rence
 * FAILURE_LOAD_FILE            - echec au chargement d'un fichier item
 * REFERENCE_FILE_LOADED        - fichier de r‚f‚rence charg‚
 * FILE_LOADED                  - fichier item charg‚
 * NO_FILE_LOADED               - cycle effectu‚ sans r‚ception de message
 *                                alors qu'on attends le chargement de fichier item
 * COMPARE_REFERENCE_FILE       - il faut comparer le nouveau fichier de
 *                                r‚f‚rence et l'ancien
 * DO_RECEPTION_ACTION          - il faut executer une action de reception
 * CLOSE_RECEPTION_ACTION       - il faut clore le traitement des actions
 *                                de r‚ception
 * RECEIVE_ALARM_ACTION         - reception d'une alarme
 * DO_ALARM_ACTION              - il faut executer une action sur alarme
 * CLOSE_ALARM_ACTIONS          - il faut clore le traitement des actions
 *                                sur alarme
 * RECEIVE_MESSAGE_ACTION       - reception d'une alarme
 * DO_MESSAGE_ACTION            - il faut executer une action sur message
 * CLOSE_MESSAGE_ACTIONS        - il faut clore le traitement des actions
 *                                sur message
 * TERMINATE_REFERENCE          - fin de tache
 */

typedef enum
{
   NO_STEP,
   WAITING_SERVICE_FILES,
   STARTING,

   WAITING_REFERENCE_FILE,
   LOADING_REFERENCE_FILE,
   COMPARING_REFERENCE_FILES,
   WAITING_FILES,

   DOING_RECEPTION_ACTIONS,
   CLOSING_RECEPTION_ACTIONS,

   DOING_ALARM_ACTIONS,
   CLOSING_ALARM_ACTIONS,

   DOING_MESSAGE_ACTIONS,
   CLOSING_MESSAGE_ACTIONS,
}  TEnum_Step;
/* ces ‚tapes sont utilis‚s pour s'y retrouver entre chaque boucle
 * dans le main
 *  WAITING_REFERENCE_FILE    - aucune action en cours
 *  LOADING_REFERENCE_FILE    - en attente du t‚l‚chargement du fichier
 *  COMPARING_REFERENCE_FILES - comparer les fichiers de r‚f‚rence
 *  WAITING_FILES             - en attente de fichier item
 *  DOING_RECEPTION_ACTIONS   - plus rien n'est attendu, on effectue les
 *                              actions de reception
 *  CLOSING_RECEPTION_ACTIONS - toutes les actions de r‚ception sont faites
 *  DOING_ALARM_ACTIONS       - on a recu une alarme, on ex‚cute une action
 *                              sur cette alarme
 *  CLOSING_ALARM_ACTIONS     - toutes les actions sur 1 alarme sont faites
 *  DOING_MESSAGE_ACTIONS     - on a recu un message, on ex‚cute une action
 *                              sur ce message
 *  CLOSING_MESSAGE_ACTIONS   - toutes les actions sur 1 message sont faites
 */

typedef enum
{
   RFR_PREMIER_TACHE = 0,

   RFR_TACHE = RFR_PREMIER_TACHE,

   RFR_BUTEE_TACHE,
   RFR_DERNIER_TACHE = RFR_BUTEE_TACHE - 1,
   RFR_NB_TACHE = RFR_BUTEE_TACHE - RFR_PREMIER_TACHE
}
enum_rfr_type_tache;

/* structure caracterisant un service */
typedef struct
{
   short int     demandeur;
   unsigned char etat;
} struct_rfr_service;


typedef struct
{
   char             Fichier[ RFR_MAX_FIC];
   char             Reference[ RFR_MAX_REF];
} struct_rfr_reference;

typedef struct
{
   char             Id[ RFR_MAX_ID];
   char             Fichier[ RFR_MAX_FIC];
} struct_rfr_manuel;

typedef struct
{
   TAlarmeExterne    Id;
} struct_rfr_alarme;

typedef struct
{
   noyau_bal_id     Bal;
   TTacheMessage    TypeMessage;
} struct_rfr_applicatif;

typedef struct
{
   char             Fichier[ RFR_MAX_FIC];
   char             Path[ RFR_MAX_PATH];
} struct_rfr_charger_fichier;

typedef struct
{
   TEnum_Action     ActionId;

   union
   {
      struct_rfr_reference              reference;
      struct_rfr_manuel                 manuel;
      struct_rfr_charger_fichier        charger;
      struct_rfr_alarme                 alarme;
      struct_rfr_applicatif             applicatif;
   }u;
}struct_rfr_action;

typedef struct
{
   short int       PrioriteMax;
   noyau_pool_id   NumeroPool;
   noyau_bal_id    TacheId;
   noyau_bal_id      BalLocale;
   noyau_bal_id      BalFichier;
   noyau_bal_id      BalHorodate;
   noyau_bal_id      BalMessage;
   noyau_bal_id      BalRetour;
   boolean           TemoinArret;
   boolean           ColdStart;
   boolean           Demarrage;
}struct_rfr_utile;

typedef struct
        {
			boolean init;
			noyau_event_id wait_init_evt;
			DWORD 		sleep_duration_after_init_sec;
			struct_tache               taches[RFR_NB_TACHE+1];

           /* donn‚es de gestion du temps r‚el */
           struct_rfr_service service[NB_RFR_SERVICE];
           struct_rfr_utile   util;

           /* donn‚es de gestion de l'action qui se produit */
           struct_rfr_action   action;

           /* donn‚es de gestion des fonctionnalit‚s */
           TEnum_Step  PhaseEnCours;
           boolean     ServiceFichierOuvert;
           int         NbFichiersReclames;
           boolean     TousFichiersValides;

           char        IdFichierReference     [ RFR_MAX_ID];

           char        NomFichierCourant      [ RFR_MAX_PATH];
           char        ReferenceFichierCourant[ RFR_MAX_REF];

           char        NomFichierNouveau      [ RFR_MAX_PATH];
           char        ReferenceFichierNouveau[ RFR_MAX_REF];

           char        NomFichierFutur        [ RFR_MAX_PATH];
           char        ReferenceFichierFutur  [ RFR_MAX_REF];

           long        AlarmeEnCours;
           long        MessageEnCours;

		   char		   hostname[MAX_PATH + 1];
		   char		   distant_path[MAX_PATH + 1];

           /* gestionnaire du fichier de configuration */
           TGereFic    *GereConfig;

           /* gestionnaires de fichiers de reference */
           TGereFic    *GereAncien;
           TGereFic    *GereNouveau;
           TGereFic    *GereCreer;

           /* gestionnaire des ID */
           TGereFic    *GereId;

           /* gestionnaire des ACTIONS */
           TGereFic    *GereActions;
           long        PremiereActionReception;
           long        DerniereActionReception;

           /* gestionnaire des IdContext */
           TGereFic    *GereIdContext;

           /* gestionnaire des Alarmes */
           TGereFic    *GereAlarmes;

           /* gestionnaire des Messages */
           TGereFic    *GereMessages;

			char path_transfert[RFR_MAX_PATH];
			char path_reference[RFR_MAX_PATH];
			char disque[RFR_MAX_PATH];
			char disque_virtuel[RFR_MAX_PATH];
			char fichier_config[RFR_MAX_PATH];
			char fichier_config_courant[RFR_MAX_PATH];
			char fichier_config_tmp[RFR_MAX_PATH];
			char fichier_reference_manuelle[RFR_MAX_PATH];

			dbg_struct_debug dbg;
			dbg_struct_trace tab_traces[RFR_NB_TRACES];
			// MFR start 2007/5/24
			char	szTftMailboxName[RFR_MAX_PATH];
			// MFR end 2007/5/24

			char	RefFileToDelete[ RFR_MAX_PATH];
			char	RefFileCurrent[ RFR_MAX_PATH];
			char	szFileNamesForComparingWithReg[4096];
			char	szMultiFilesNames[RFR_MAX_PATH];
			char	szOldFileId[RFR_MAX_PATH];
			char	szCurrentFileId[RFR_MAX_PATH];


        } struct_globale_rfr, TTacheContext, *TpTTacheContext;

/*-------------------- variables ---------------- */
/* tableau de structure */
PROTECTED TTacheContext          RFR;

/* pour le mode DEBUG */
PROTECTED char *RFR_FILE;
PROTECTED int RFR_LINE;

/* -------------- FONCTIONS: --------------*/

PROTECTED enum_instance_result ReferenceInitTrace (char * pcBal);
PROTECTED enum_instance_result ReferenceDeinitTrace(void);
PROTECTED void ReferenceFichierTrace (char * fmt,...);
PROTECTED void DEFINE_ReferenceFichierDebug (char *fmt,...);

/*------------------VARIABLES:-----------------*/


#undef PROTECTED
#undef I
#undef INIT
#endif