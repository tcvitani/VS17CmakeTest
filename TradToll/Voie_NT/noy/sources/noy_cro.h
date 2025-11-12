/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LIBRAIRIES
* FICHIER: NOY_CRO.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE: 
* --------------------------------------------------------------------
* RESUME: Fonctions globales qui concernent le noyau temps-reel.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d'interface
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Noyau/Includes/NOY_CRO.H_v  $
 * 
 *    Rev 1.2   Jan 13 2000 15:05:06   bph
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:40:06   bph
 *  
 * 
 *    Rev 1.3   28 Oct 1997 14:54:12   BPH
 * 1ere archive v6.00 - cf fiche de version
 * 
 *    Rev 1.2   Feb 28 1997 10:26:12   BPH
 *  
 * 
 *    Rev 1.1   Feb 21 1997 15:47:30   BPH
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef NOY_CRO_H
#define NOY_CRO_H

/*--------------- EXTERNALS: ---------------*/
#include <protect.h>

/*--------------- DEFINES: ---------------*/
/*** Nombre maximum de chronomètres ***/
#define  MAX_CHRONO           256


/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

PROTECTED noyau_enum_retour Noyau_Init_Chrono (void);

/*--------------- VARIABLES: ---------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif
