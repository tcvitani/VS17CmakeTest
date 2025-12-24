/*------   (v) 1997 CS-Route  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION de FICHIER
* FICHIER: emi_fic.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module EMISSION DE FICHIERS
* --------------------------------------------------------------------
*                    ***  A NOTER ***
*  Pour les sauvegardes manuelles sur disquettes,les fichiers sont copi‚s
*  dans un repertoire sp‚cifique : C:\DISK\BACKUP, … chaque cycle.
*  C'est … l'application d'effectuer la copie sur disquette et DE VIDER
*  CE REPERTORIE une fois la copie termin‚e.
*  ATTENTION, un repertoire ne peut contenir qu'un nombre limit‚ de fichiers,
*  la demande de sauvegarde doit donc rester compatible :
*  exemple : … raison d'un fichier toutes les 5 minutes, et de 4096 fichiers
*            par repertoire, la demande de sauvegarde ne doit pas d‚passer
*            14 jours (environ) !!!!!


* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Includes/emi_fic.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:02   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.13   Jan 22 1999 15:19:00   FR
 * Mise à jour du numéro de version
 * 
 *    Rev 1.12   03 Dec 1997 15:48:36   DPI
 * Evolution pour TCPx.LIB 5.00
 * 
 *    Rev 1.11   31 Oct 1997 11:44:08   DPI
 * Modif param EMI_MAX_DATE
 * 
 *    Rev 1.10   29 Oct 1997 19:44:54   DPI
 *  
 * 
 *    Rev 1.9   29 Oct 1997 19:42:08   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 * 
 *    Rev 1.8   14 Oct 1997 17:23:14   HMO
 * Version 3.10
 * 
 *    Rev 1.7   23 Sep 1997 16:13:08   HMO
 * Modifications effectuées apres site pour MAJ
 * 
 *    Rev 1.7   23 Sep 1997 16:10:40   HMO
 * Modifications effectuées sur site
 * 
 *    Rev 1.6   Aug 06 1997 15:37:54   DPI
 * Nouvelle version
 * 
 *    Rev 1.5   Aug 01 1997 17:26:00   HMO
 * Gestion de la version
 * 
 *    Rev 1.4   Jun 10 1997 09:06:50   ANA
 *  
 * 
 *    Rev 1.3   May 27 1997 09:31:48   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.2   May 16 1997 13:33:44   ANA
 *  
 *
 *    Rev 1.1   May 13 1997 17:09:16   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 *
 *    Rev 1.0   Apr 15 1997 09:43:54   ANA
 * Renommage de emission.h par emi_fic.h
 * 
 *    Rev 1.1   Apr 07 1997 11:50:44   ANA
 * Nouvelle Gestion des "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:14:30   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef EMI_FIC_H
#define EMI_FIC_H

/*--------------- INCLUDES: ---------------*/
#include <dos.h>

/*--------------- RESERVED: ---------------*/
#include <csrlc32.h>
#include <run.h>


#ifdef EMI_DEF
#include "public.h"
#else
#include "export.h"
#endif

/*--------------- EXTERNALS: --------------*/


/*--------------- DEFINES: ----------------*/
#define EMI_VERSION                 "6.00"

/* fichiers de trace du module */
//#define EMI_FICHIER_TRACE           "EMITRACE.FIC"
//#define EMI_FICHIER_TRACE_ECRAN     "EMITRACE.ECR"

/* repertoire de sauvegarde sur disque */
//#define REP_SAUVEGARDE_LOCAL       "C:\\DISK\\BACKUP"

#define FICHIER_COLD_START      "CLD_STRT"

/* taille de stockage de la date */
//#define EMI_MAX_DATE            25

/* nombre de jetons pour le service ETAT */
#define EMI_NB_JETONS               2

/*--------------- TYPEDEFS: ---------------*/

/* erreurs possibles pour les retour des fonctions du module  */

/* liste des services disponibles */
typedef enum
{
   PREMIER_EMI_SERVICE = 0,

   M_EMI_ARRET  = PREMIER_EMI_SERVICE,
   M_EMI_ETAT,
   M_EMI_RESTITUTION,
   M_EMI_PURGE,
   M_EMI_CONFIG,

   BUTEE_EMI_SERVICE,
   DERNIER_EMI_SERVICE = BUTEE_EMI_SERVICE - 1,
   NB_EMI_SERVICE = BUTEE_EMI_SERVICE - PREMIER_EMI_SERVICE
} enum_emi_service;

/* liste des types de messages */
typedef enum
{
   PREMIER_EMI_TYPE = 0,

   EMI_DEBUT = PREMIER_EMI_TYPE,    /***************************************/
   EMI_DEBUT_ACQ,                   /* pour les services ETAT et PURGE     */
   EMI_DEBUT_NACQ,                  /*                                     */
   EMI_FIN,                         /*                                     */
   EMI_FIN_ACQ,                     /*                                     */
   EMI_FIN_NACQ,                    /***************************************/
   EMI_DEMANDE,                     /***************************************/
   EMI_DEMANDE_ACQ,                 /* pour les services ARRET, RESTITUTION*/
   EMI_DEMANDE_NACQ,                /***************************************/
   EMI_NOUVEL_ETAT,

   BUTEE_EMI_TYPE,
   DERNIER_EMI_TYPE = BUTEE_EMI_TYPE - 1,
   NB_EMI_TYPE = BUTEE_EMI_TYPE - PREMIER_EMI_TYPE
} enum_emi_type;

/******************************************************
*                  SERVICE ETAT                       *
******************************************************/
/*       etat possibles pour la liaison FIC
*            et les demandes de backup */
typedef enum
{
   EMI_LIAISON_HS = 0,
   EMI_LIAISON_OK,
   EMI_INACTIF,
   EMI_BACKUP_EN_COURS,
   EMI_PURGE_EN_COURS,
   EMI_BACKUP_TERMINE_COMPLET,
   EMI_BACKUP_TERMINE_INCOMPLET,
   EMI_PURGE_TERMINE,
   EMI_BACKUP_ERR_NUMERO,
   EMI_BACKUP_ERR_DATE,
   EMI_AUCUN_FICHIER

}enum_emi_etat;

/* structure des messages */
typedef struct
{
   enum_emi_etat liaison;
   enum_emi_etat backup;
}struct_emi_gestion_etat;

/******************************************************
*               SERVICE RESTITUTION                   *
*******************************************************/
/* support de restitution  */
typedef enum
{
   RESTIT_FIC = 1,
   RESTIT_DISQUE,
   RESTIT_PURGE
}enum_emi_action;

/* type de restitution possibles */
typedef enum
{
   RESTIT_DATE = 1,
   RESTIT_NUMERO
}enum_emi_type_restit;

typedef enum
{
	EMI_RESTIT_AUTOMATIQUE,
	EMI_RESTIT_MANUAL
}enum_emi_restit_mode;

typedef struct
{
   long debut;
   long fin;
}struct_restit_numero;

typedef struct
{
   char debut[9];
   char fin[9];
}struct_restit_date;

/* messages */
typedef struct
{
   enum_emi_action			action;
   enum_emi_restit_mode		mode;
   enum_emi_type_restit		type;
   union
   {
      struct_restit_numero numero;
      struct_restit_date   date;
   }t;
}struct_emi_gestion_restit;

/* structure des messages pour le service PURGE */
typedef struct_emi_gestion_restit struct_emi_gestion_purge;

typedef struct
{
	BOOL    authorization;
	BOOL	cold_start;
	DWORD	plaza_number;
	DWORD	lane_number;
}struct_emi_srv_config_data;


/* structure de l'entete des messages */
typedef struct
{
   struct_neutre       neutre;
   enum_emi_service    service;
   enum_emi_type       type_message;
} struct_emi_entete;

/* structure des messages echang‚s */
typedef struct
{
   struct_emi_entete    entete;
   union
    {
	 struct_emi_gestion_etat			msg_etat;
	 struct_emi_gestion_restit			msg_restit;
	 struct_emi_gestion_purge			msg_purge;
	 struct_emi_srv_config_data			msg_config_data;
    }u;
}struct_emi_message;

/* parametres de configuration du module */
typedef struct
{
     boolean       flag_cold_start;
     short int     priorite_init_max;
     short int     priorite_tache_max;
     short int     num_pool;
     short int     bal_fic;
     short int     bal_hrd;
     char          fichier_conf[MAX_PATH];
     SYSTEMTIME		time_purge;

} struct_emi_config;

/********************************************************************
*                     REMARQUE IMPORTANTE                           *
*  La periode d'emission courante des fichiers messages vers le LS  *
*  doit ˆtre SUPERIEURE … la periode de cr‚ation de ces fichiers    *
*  pour assurer l'emission de plusieurs fichiers existants          *
*  si du retard a ‚t‚ pris lors de l'‚mission courante              *
*********************************************************************/

/*--------------- FUNCTIONS: ---------------*/
EXPORT enum_instance_result WINAPI EmiLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI EmiArret(noyau_bal_id iBalId);


/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* EMI_FIC_H */


