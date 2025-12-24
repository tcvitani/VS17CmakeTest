/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU0.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Includes/simu0.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:06   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   May 27 1997 16:32:20   ANA
 *  
 * 
 *    Rev 1.2   May 13 1997 17:12:12   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 * 
 *    Rev 1.1   Apr 15 1997 10:07:06   ANA
 * Correction bug sur le service FICHIER de TCP/IP
 * 
 *    Rev 1.0   Mar 21 1997 09:29:36   ANA
 * Creation
 * 
  *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef SIMU_H
#define SIMU_H

/*--------------- INCLUDES: ---------------*/
//#include "stdcsee.h"

#ifdef INIT_DEF
#include "public.h"
#else
#include "export.h"
#endif

/*---------------- DEFINE:------------------*/

#define POOL_SIMU NULL

enum
{
   LOGICIEL_FINI,
   LOGICIEL_EN_COURS
};


/*---------------- TYPEDEF:-------------------*/
typedef enum
{
  FIRST_BAL = 0,
  BAL_A = FIRST_BAL,
  BAL_B,
  BAL_C,
  BAL_D,

  BUTEE_BAL,
  NB_BAL = BUTEE_BAL - FIRST_BAL
} enum_bal;


typedef enum
{
  FIRST_TACHE = 0,
  TACHE_SIMU = FIRST_TACHE,
  TACHE_RECOIT,

  BUTEE_TACHE,
  NB_TACHE = BUTEE_TACHE - FIRST_TACHE
} enum_tache;

/*typedef enum
{
  FIRST_POOL = 0,
  EMI_POOL_SYSTEME = FIRST_POOL,
  POOL_MODULE,
  POOL_SIMU,

  BUTEE_POOL,
  NB_POOL = BUTEE_POOL - FIRST_POOL
} enum_pool;*/

typedef enum
{
  FIRST_CHRONO = 0,

  BUTEE_CHRONO,
  NB_CHRONO = BUTEE_CHRONO - FIRST_CHRONO
} enum_chrono;

typedef struct
{
   short int init;
   short int courant;
} struct_simu_priorite;

/*--------------------FONCTIONS: ---------------*/

PUBLIC noyau_enum_retour SimuLance(struct_simu_priorite /*priorite*/);
PUBLIC noyau_enum_retour SimuArret(void) ;

/*------------------VARIABLES:------------------------*/
PUBLIC short int etat_logiciel;

#undef PUBLIC
#undef I
#undef INIT
#endif