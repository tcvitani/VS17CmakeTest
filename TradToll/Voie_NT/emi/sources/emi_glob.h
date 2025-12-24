/*------   (v) 1997 CS-Route  ------------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: emi_glob.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier interne au module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations de constantes et variables globales
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Includes/emi_glob.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:02   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.9   06 Nov 1997 15:18:22   DPI
 * Integration DUT 9.00
 * 
 *    Rev 1.8   03 Nov 1997 16:10:12   DPI
 * Version 4.00
 * 
 *    Rev 1.7   Aug 01 1997 17:27:08   HMO
 * Passage en protected de la fonction EmiGestionEtat
 * 
 *    Rev 1.6   Jun 20 1997 17:35:00   HMO
 *  
 * 
 *    Rev 1.5   Jun 13 1997 10:30:30   HMO
 *  
 * 
 *    Rev 1.4   May 27 1997 09:31:50   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.3   May 13 1997 17:09:16   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 * 
 *    Rev 1.2   Apr 15 1997 10:01:56   ANA
 * Correction dur bug sur le service FICHIER
 * 
 *    Rev 1.1   Apr 07 1997 11:50:42   ANA
 * Nouvelle Gestion des "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:14:30   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef EMI_GLOB_H
#define EMI_GLOB_H

/*--------------- INCLUDES: ---------------*/
#include <noyau.h>
#include <debug.h>

#include <protect.h>

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/
/* pour les traces du module */
enum index_traces
{
   EMI_TRC,
   EMI_NB_TRACES
};
#define EmiFichierDebug EMI_FILE=__FILE__,\
                        EMI_LINE=__LINE__,\
                        DEFINE_EmiFichierDebug

//#define EMI_TACHE_HORLOGE                 -2
//#define EMI_NB_BAL                        2
//#define EMI_NB_CHRONO                     0

#define EMI_TIMEOUT_EMISSION              INFINITE  //infini
#define EMI_TIMEOUT_BACKUP                18 // 1s

//#define EMI_EVT_REPONSE_FIC              1

enum
{
   EMI_NON_VALIDE = 0,
   EMI_VALIDE
};

enum
{
   EMI_AUCUN = -1,
   EMI_LIBRE,
   EMI_EN_COURS
};


/*----------------TYPEDEF:----------------*/
typedef enum
{
   EMI_PREMIER_TACHE = 0,

   EMI_TACHE_EMISSION = EMI_PREMIER_TACHE,
   EMI_TACHE_BACKUP,

   EMI_BUTEE_TACHE,
   EMI_DERNIER_TACHE = EMI_BUTEE_TACHE - 1,
   EMI_NB_TACHE = EMI_BUTEE_TACHE - EMI_PREMIER_TACHE
}
enum_emi_type_tache;

/* structure caracterisant un service */
typedef struct
{
   noyau_bal_id  demandeur[EMI_NB_JETONS];
   unsigned char etat;
} struct_emi_service;

/* structure regroupant les numeros de fichiers */
typedef struct
{
   long  courant;
   long  backup;
   long  premier_backup;
   long  dernier_backup;
}struct_emi_num_fichier;

typedef struct
{
   char						hostname[MAX_PATH];
   char						path_distant[MAX_PATH];
   char						path_disk[MAX_PATH];
   char						path_backup[MAX_PATH];
   char						local_directory[MAX_PATH];
   char						local_drive[MAX_PATH];
}struct_emi_directories;

typedef struct
{
   noyau_pool_id			pool;
   char						BalNam[MAX_PATH];
   char						BackupBalNam[MAX_PATH];
   noyau_bal_id				bal_id;
   noyau_bal_id				bal_backup;
   noyau_bal_id				bal_fic;
   noyau_bal_id				bal_hrd;
   int						heure_debut_reveil;
   t_hrd_num_reveil			reveil;
   int						duree_sommeil;
//   char						path_distant[MAX_PATH];
//   char						path_disk[MAX_PATH];
//   char						path_backup[MAX_PATH];
//   char						disk_backup[MAX_PATH];
   enum_emi_type_restit		restit_type;
   enum_emi_restit_mode		mode;
   char						path_backup_date[MAX_PATH];
   char						path_backup_numero[MAX_PATH];
   struct_emi_directories	DIR;
}struct_emi_utile;

/* structure globale associee */
typedef struct
{
   struct_tache               taches[EMI_NB_TACHE+1];
   boolean                 temoin_arret_emission;
   //char						file_config[MAX_PATH];
   short int               priorite_max;
   //short int               tache_id;
   //short int               tache_backup;
   noyau_event_id		   event;	
   boolean                 cold_start;
   struct_emi_utile        util;
   struct_emi_num_fichier  num_fichier;
   boolean                 flag_chrono;
   boolean                 flag_emission;
   boolean                 flag_restitution;
   int                     periode_purge; /* en jours */
   SYSTEMTIME				heure_purge;
   struct_emi_gestion_etat msg_etat;
   struct_emi_gestion_etat ancien_etat;
   struct_emi_service      service[NB_EMI_SERVICE];

   DWORD					lane_number;
   DWORD					plaza_number;
   BOOL						authorized;
   BOOL						config_received;

	dbg_struct_debug dbg;
	dbg_struct_trace tab_traces[EMI_NB_TRACES];

	noyau_bal_id bl_hrd;
}struct_globale_emi;

/* -------------- FONCTIONS: --------------*/
PROTECTED DWORD WINAPI Emission (PVOID param);
PROTECTED DWORD WINAPI EmiBackup (PVOID param);
//PROTECTED void SuppressionFichierColdStart( void);

PROTECTED enum_instance_result EmiInitTrace (char * pcBal);
PROTECTED void EmiFichierTrace (char * fmt,...);
PROTECTED void DEFINE_EmiFichierDebug (char *fmt,...);
PROTECTED void EmiGestionEtat(struct_emi_gestion_etat *,
                              struct_emi_gestion_etat *);

/*------------------VARIABLES:-----------------*/
/* tableau de structure */
PROTECTED struct_globale_emi EMI;

/* pour le mode DEBUG */
PROTECTED char *EMI_FILE;
PROTECTED int EMI_LINE;


#undef PROTECTED
#undef I
#undef INIT
#endif