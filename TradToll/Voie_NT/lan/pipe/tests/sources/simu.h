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
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/simu.h_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   02 Oct 1998 11:46:12   bph
 *  
 * 
 *    Rev 1.9   Aug 04 1997 17:34:42   ANA
 * Transfert de fichiers avec renommage
 * 
 *    Rev 1.8   Jul 02 1997 15:22:26   ANA
 * Mise à jour pour PEMM et noyau 5.00
 * Nouvelle gestion du DOP
 * 
 *    Rev 1.6   Apr 05 1997 16:59:26   ANA
 * Modifications pour l'arret des taches
 * 
 *    Rev 1.5   Mar 07 1997 18:33:34   ANA
 * Evolution du message de vie et Ajout de la tache
 * de gestion du service HORAIRE
 *
 *    Rev 1.4   Mar 03 1997 14:09:40   ANA
 * Integration du module NOYAU gerant la com TCP/IP
 *
 *    Rev 1.3   Feb 26 1997 16:41:06   ANA
 * Configuration reseau par fichier
 *
 *    Rev 1.2   Feb 20 1997 15:57:00   ANA
 * Version 0
 * 
 *    Rev 1.1   Feb 10 1997 17:15:38   ANA
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
#define FALSE    0
#define TRUE     1

#define SIMU_PRIORITE_MAX           7

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
  TACHE_SRV_PIPE,
  
  BUTEE_TACHE,
  NB_TACHE = BUTEE_TACHE - FIRST_TACHE
} enum_tache;


#define POOL_SIMU NULL


typedef struct
{
   noyau_priorite_tache init;
   noyau_priorite_tache courant;
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