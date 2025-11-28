/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: TCP/IP
* FICHIER: TCP_INIT.H
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
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/test_lan.h_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:18   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:36:22   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:46:16   bph
 *  
 * 
 *    Rev 1.4   Apr 14 1997 16:41:52   BPH
 *  
 * 
 *    Rev 1.3   Mar 07 1997 18:33:38   ANA
 * Evolution du message de vie et Ajout de la tache
 * de gestion du service HORAIRE
 *
 *    Rev 1.2   Mar 03 1997 14:09:40   ANA
 * Integration du module NOYAU gerant la com TCP/IP
 *
 *    Rev 1.1   Feb 10 1997 17:15:40   ANA
 *  
 *
*
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef TCP_TEST_H
#define TCP_TEST_H

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
#define PRIORITE_RESEAU              THREAD_PRIORITY_NORMAL
#define PRIORITE_TACHE               THREAD_PRIORITY_NORMAL

/* Nombre de messages pouvant etre alloues en meme temps */
/* cette valeur est utilise dans le calcul de DATA_SEGMENT_SIZE */
#define NB_NUM_SEGMENTS              200

/* Taille reservee a RTC dans le data segment     */
/* cette valeur doit etre conforme a ce qui a ete */
/* saisi lors de la configuration du loader.      */
#define DATA_SEGMENT_SIZE            12000

/* Taille reservee a la pile de la tache horloge  */
#define TAILLE_PILE_CLOCK            1024


/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

/* Tableau donnant le nombre de messages pouvant attendre  */
/* dans chaque boite aux lettres : valeur par defaut       */
static short int NumSegment [NB_BAL+1] =
{
   -1,      /* BL_     */
   -1,      /* BL_    */
   -1,
   -1,
   0        /* Fin du tableau */
};


#undef PROTECTED
#undef I
#undef INIT
#endif

