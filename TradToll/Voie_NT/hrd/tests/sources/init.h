/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: INIT
* FICHIER: INIT.H
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
* Le travail effectue par ce module est le suivant :
*     1) verifier que le repertoire d'execution existe sur le disque
*        ( le logiciel ne demarre pas si ce test echoue)
*     2) Choisir ce repertoire comme repertoire courant.
*     3) Verifier que le repertoire de demarrage existe et le creer
*        le cas echeant.
*     4) Verifier que le disque virtuel existe.
*        ( le logiciel ne demarre pas si ce test echoue)
*     5) Ouvrir le fichier d'initialisation du logiciel et verifier
*        que son contenu est coherant.
*        ( le logiciel ne demarre pas si ce test echoue)
*     6) Tenter d'ouvrir le fichier context qui doit se trouver dans le
*        repertoire de demarrage et en d‚duire les valeurs des
*        parametres de contexte. Creer ce fichier si il n'existe pas.
*     7) Initialiser l'ecran de la VM.
*        ( le logiciel ne demarre pas si cet init echoue)
*     8) Initialiser les cartes de communications
*        ( le logiciel ne demarre pas si cet init echoue)
*     9) Initialiser RTC et IOS
*        ( le logiciel ne demarre pas si cet init echoue)
*    10) Initialisation des chronometres
*        ( le logiciel ne demarre pas si cet init echoue)
*    11) Lancer toutes les taches temps reel
*        ( le logiciel ne demarre pas si cet init echoue)
*    12) Attendre dans un while(1) que les taches s'arretent.
*    13) Gerer une eventuelle sauvegarde des fichiers de donnees
*        sur une disquette.
*    14) Effacer l'ecran de la VM et rendre la main a DOS.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/modulo/voies/horodate/simu/init.h_v  
 * 
 *    Rev 1.0   Aug 05 1996 09:00:26   NHA
 *  
*
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef INIT_H
#define INIT_H

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>


#include <memclass.h>


/*--------------- RESERVED: ---------------*/



/*--------------- EXTERNALS: ---------------*/



/*--------------- DEFINES: ---------------*/

/* codes de retour de l'application */
#define EXIT_OK                        0
#define EXIT_KO                      100
#define EXIT_NO_EXE_REP              101
#define EXIT_NO_VIRTUEL              102
#define EXIT_NO_INIT                 103
#define EXIT_PB_CONTEXT              104
#define EXIT_NO_ECRAN                105
/*#define EXIT_NO_COMM                 106*/
#define EXIT_NO_RTC                  107
#define EXIT_NO_TACHES               108
#define EXIT_NO_BAL                  109

#define PRIORITE_MAX                 8

/* Nombre de messages pouvant etre alloues en meme temps */
/* cette valeur est utilise dans le calcul de DATA_SEGMENT_SIZE */
#define NB_NUM_SEGMENTS              600

/* Taille reservee a RTC dans le data segment     */
/* cette valeur doit etre conforme a ce qui a ete */
/* saisi lors de la configuration du loader.      */
#define DATA_SEGMENT_SIZE            12288

/* Taille reservee a la pile de la tache horloge  */
#define TAILLE_PILE_CLOCK            1024

/* touches utilisees pour faire la sauvegarde */
#define NB_BL                        5
#define NB_TACHE                     2
#define NB_POOL                      2
#define LOGICIEL_EN_COURS 1
#define LOGICIEL_FINI 0

enum
{
  FIRST_POOL,   /* POOL1 */
  POOL_HRD,
  POOL_DO,
  NID_POOL,
  LAST_POOL     /* POOL3 */
};

enum
{
  FIRST_TACHE,  /* TACHE1 */

  LAST_TACHE    /* TACHE3 */
};

enum
{
  FIRST_BL,     /* BL_TACHE1 */
  LAST_BL       /* BL_TACHE2 */
};

enum
{
  FIRST_CHRONO, /* CHRONO1 */
  LAST_CHRONO   /* CHRONO2 */
};



/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/


/*--------------- VARIABLES: ---------------*/


PUBLIC short int etat_logiciel;
PUBLIC short int acq_fin;

#endif
