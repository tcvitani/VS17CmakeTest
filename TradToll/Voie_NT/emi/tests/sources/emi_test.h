/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: REFERENCE
* FICHIER: RFR_TEST.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Module contenant la routine main par laquelle le
* programme demarre. Ce module est charge de verifier la coherance
* de l'environnement logiciel et de positionner les variables
* globales avant de lancer les autres taches. Ce module devient
* ensuite la tache de fond.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Includes/emi_test.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:06   afx
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
*
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef RFR_TEST_H
#define RFR_TEST_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>


/*--------------- EXTERNALS: ---------------*/



/*--------------- DEFINES: ---------------*/
/* codes de retour de l'application */
#define EXIT_OK                        0
#define EXIT_NOK                      100
#define EXIT_NO_INIT                 101
#define EXIT_NO_RTC                  102
#define EXIT_NO_TACHES               103
#define EXIT_NO_BAL                  104
#define EXIT_NO_CONFIG               105
#define EXIT_NO_RESEAU               106
#define EXIT_PB_ARRET_TACHES         107
#define EXIT_PB_RESSOURCE            108

#define PRIORITE_MAX                 THREAD_PRIORITY_HIGHEST
#define PRIORITE_TACHE               THREAD_PRIORITY_NORMAL


/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/


/*--------------- VARIABLES: ---------------*/


#undef PROTECTED
#undef I
#undef INIT
#endif

