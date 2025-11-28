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
 * $Log:   T:/MODULO/VoieNt/Referenc/test/Includes/simu0.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:54   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jun 04 1997 19:11:40   DPI
 *  
 * 
 *    Rev 1.1   Jun 03 1997 17:38:38   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:44:54   DPI
 *  
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

#ifdef INIT_DEF
#include "public.h"
#else
#include "export.h"
#endif

/*---------------- DEFINE:------------------*/

enum
{
   LOGICIEL_FINI,
   LOGICIEL_EN_COURS
};


/*---------------- TYPEDEF:-------------------*/
/*
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
*/

typedef enum
{
  FIRST_TACHE = 0,
  TACHE_SIMU = FIRST_TACHE,
  TACHE_RECOIT,

  BUTEE_TACHE,
  NB_TACHE = BUTEE_TACHE - FIRST_TACHE
} enum_tache;

#define POOL_SIMU NULL

typedef struct
{
   short int init;
   short int courant;
} struct_simu_priorite;

/*--------------------FONCTIONS: ---------------*/

PUBLIC short int SimuLance(struct_simu_priorite /* priorite_max*/);
PUBLIC short int SimuArret(void) ;

/*------------------VARIABLES:------------------------*/
PUBLIC short int etat_logiciel;

#undef PUBLIC
#undef I
#undef INIT
#endif