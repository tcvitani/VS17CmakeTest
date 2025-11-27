/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HRD_SERV.H
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Gestion des messages recus par HORODATE.
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Hrd/Includes/HRD_SERV.H_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:08:56   bph
 *  
 * 
 *    Rev 1.14   03 Jun 1998 10:38:58   DPI
*
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef HRD_SERV_H
#define HRD_SERV_H

/*--------------- INCLUDES: ---------------*/
#include <protect.h>

/* version */
#define HRD_VERSION           "4.20"
#define HRD_COMMENT           "NOYAU 6.12"


#define HRD_NB_TACHE   1

typedef enum {
   HRD_TRACE_INACTIVE,
   HRD_TRACE_ACTIVE
}
enum_hrd_trace;

/*--------------- CONSTANTES: ---------------*/

/* Timeout when waiting for access to a semaphore (in tickcounts 55ms) */
#define HRD_SEM_TO 1000

/* Maximum number of alarms before  */
#define MAX_ALARMES 100

/* Number of time an alarm structure can be rearmed before it have the same id.
   MAX_ID_INCREMENT * MAX_ALARMES must be less than the maximum 
   value of t_hrd_num_reveil */
#define MAX_ID_INCREMENT 100

/*--------------------- types -------------------*/

typedef enum
{
    PREMIER_TYPE_ALARME = 0,

    ALARME_INIT = PREMIER_TYPE_ALARME, 
    ALARME_PONCT ,
    ALARME_JOUR ,
    ALARME_HOR,
    ALARME_REGUL,

    BUTEE_TYPE_ALARME,
    DERNIER_TYPE_ALARME = BUTEE_TYPE_ALARME -1,
    NB_TYPE_ALARME = BUTEE_TYPE_ALARME - PREMIER_TYPE_ALARME
}
enum_type_alarme;


typedef struct
{
   noyau_bal_id bl_retour; /*BAL de retour*/
   enum_type_alarme type_alarme; /*type : horaire,ponctuelle,reguliere,journaliere*/
   struct_hrd_date_entree_hex date; /*Date du reveil*/
   struct_hrd_heure_hex heure;/*Heure du reveil*/
   long int periode;/*Periode du reveil exprimee en secondes*/
   unsigned long int secondes_depuis_80;/*secondes de 1980 a date de reveil*/
   t_hrd_num_reveil id;
}
struct_reservation_alarme;

/*-------------------- fonctions----------------- */

/*Date et heure du PC*/
PROTECTED struct_hrd_date_entree_hex DatePC(void);
PROTECTED struct_hrd_heure_hex HeurePC(void);

PROTECTED DWORD WINAPI Hrd ( LPVOID ); 

/*Fonctions de gestion des services */
PROTECTED void InitHorodate (void);

PROTECTED void ReceptionArret(struct_hrd_message * /*p_struct_hrd_message*/);

PROTECTED void ReceptionGestionAlarme(struct_hrd_message *p_struct_hrd_message);

PROTECTED void ReceptionEvolHeure(struct_hrd_message *p_struct_hrd_message);

PROTECTED void ReceptionGestionDateHeure(struct_hrd_message *p_struct_hrd_message);

/*Fonctions de gestion des messages de type M_HRD_ARRET*/
PROTECTED void Demande(struct_hrd_message *p_struct_hrd_message);

/*Fonctions de gestion des messages de type M_HRD_GESTION_ALARME*/
PROTECTED void ArmePonct(struct_hrd_message *p_struct_hrd_message);
PROTECTED void ArmeHor(struct_hrd_message *p_struct_hrd_message);
PROTECTED void ArmeJour(struct_hrd_message *p_struct_hrd_message);
PROTECTED void ArmeRegul(struct_hrd_message *p_struct_hrd_message);
PROTECTED void Desarme(struct_hrd_message *p_struct_hrd_message);
PROTECTED void Reveil(void);
PROTECTED void Reveil_Figeant(void);

/*Fonctions de gestion des messages de type M_HRD_EVOL_HEURE*/
PROTECTED void ArretEvol(noyau_bal_id bl_retour);
PROTECTED void DepartEvol(noyau_bal_id bl_retour);

/*Fonctions de gestion des messages de type M_HRD_GESTION_DATE_HEURE*/
PROTECTED void NouvelleDateHeure(struct_hrd_message *p_struct_hrd_message);

/*Fonctions de reveil des taches*/
PROTECTED void Reveil(void);
PROTECTED void MAJReveils(void);

/*Envoie d'un message a une tache qui a demande un service M_HRD_GESTION_ALARME*/
PROTECTED void EnvoiAcqGestionAlarme
                 (noyau_bal_id bal_dest,
                 short int service_id,
                 short int type_message,
                 struct_hrd_gestion_alarme msg_gestion_alarme);
PROTECTED int bissextile(short int annee);

/* gestion d'attribution des num‚ros d'alarmes */
PROTECTED struct_reservation_alarme* AllocateAlarm (struct_hrd_date_entree_hex date,
                                                    struct_hrd_heure_hex heure,
                                                    long int periode,
                                                    noyau_bal_id bl_retour,
                                                    unsigned long int secondes_depuis_80);

PROTECTED t_hrd_num_reveil GenerateId(struct_reservation_alarme* alarm);
PROTECTED struct_reservation_alarme* FindAlarm(t_hrd_num_reveil id);

PROTECTED void DumpHrd();

PROTECTED enum_hrd_reveil RequestHrdSemaphore (noyau_delai timeout);

/*-------------------- variables ---------------- */

PROTECTED struct_tache _HRD_tache[HRD_NB_TACHE + 1]
#ifdef LOC_DEF
= {
   { 1, 0 , 2048 , Hrd },
   { 0, 0, 0, NULL }
  };
#else
;
#endif

typedef struct
{
   /*Numero de la boite a lettres de HRD*/
   noyau_bal_id bal;

   /*Priorite de HRD*/
   noyau_priorite_tache priorite;

   /*Pool de HRD*/
   noyau_pool_id pool;

   /* num‚ro dans _HRD_.tab_alarme[] de l'unique r‚veil figeant le temps */
   t_hrd_num_reveil num_reveil_figeant;

   /*etat de l'evolution du temps*/
   enum_hrd_evolution_temps evol;

   /*BAL de retour de la tache qui a demande une suspension du temps*/
   noyau_bal_id evol_bl;

   /*Date et heure de reference de la tache HORODATE*/
   struct_hrd_date_entree_hex date;
   struct_hrd_heure_hex heure;

   /*Flag qui permet de savoir si HRD est initialisee*/
   short int etat_init;

  /*Tableau de reservation du numero d'alarme et de memorisation du message*/
  struct_reservation_alarme tab_alarme[MAX_ALARMES];

}
struct_module_hrd;

typedef struct
{
  /* Main instance (the one starting the Hrd thread) */
  boolean main_instance;

  /* Semaphore used as a mutex to access _HRD_ */
  noyau_semaphore_id HRD_sem;

  /*Flag de trace */
  short int flag_trace;

  /*Fichier de trace*/
  FILE * fic_trace;
}
struct_local_hrd;

// début de section de données partagées initialisées
#include "global.h"

   PROTECTED struct_module_hrd _HRD_ INIT(0);
   PROTECTED WORD HRD_NB_INSTANCES INIT(0);

// fin de section de données partagées initialisées
#include "global.h"


PROTECTED struct_local_hrd _hrd_ INIT(0);

#undef PROTECTED
#undef I
#undef INIT
#endif