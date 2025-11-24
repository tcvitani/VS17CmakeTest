/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: AUT
* FICHIER: aut.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE: AUTOMATE
* --------------------------------------------------------------------
* RESUME: Interface du moteur d'automate.
* --------------------------------------------------------------------
* DESCRIPTION: Structures utilisees par le moteur d'automate et
*              interface d'utilisation.
* --------------------------------------------------------------------
* HISTORIQUE: 
 * $Log:   T:/MODULO/Commun_NT/Aut/Includes/csr_aut.h_v  $
 * 
 *    Rev 1.2   Apr 25 2001 18:30:08   pboutele
 *  
 *
 *    Rev 1.0   28 Oct 1997 17:33:18   BPH
 * Initial revision
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef AUT_H
#define AUT_H

/*--------------- INCLUDES: ---------------*/

#include <windows.h>

/*--------------- RESERVED: ---------------*/

#ifdef AUT_DEF
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define AUT_ARGS "@"
#define AUT_NO_ARGS "_"

#define AUT_EVT_INIT(event, args) args event

/*--------------- TYPEDEFS: ---------------*/

/* référence vers une instance du module automate */
typedef void * aut_automate_id;

/* a utiliser pour declarer statiquement un evenement en l'initialisant */
typedef const char aut_event[];

/* pour referencer un evenement */
typedef const char * aut_event_id;

/* pour referencer un etat */
typedef int aut_etat_id;
   
/* pour referencer une action */
typedef BOOL ( WINAPI * aut_action )(void *data, void *args);

/* une transition entre etats <=> event associe à { etat_suivant, fonction } */
typedef struct
{
   aut_event_id event;
   aut_etat_id etat_suivant;
   aut_action action;
} aut_transition;

/* un etat est un ensemble de transitions vers d'autres etats */
typedef const aut_transition aut_etat[];

/* un automate est un ensemble d'etats */
typedef struct
{
    const aut_transition *etat;
    const aut_transition *en_entree;
    const aut_transition *en_sortie;
}
aut_automate[];

/* EVENEMENTS PREDEFINIS */
PUBLIC aut_event AUT_EVT_NULL;
PUBLIC aut_event AUT_EVT_PAR_DEFAUT;
PUBLIC aut_event AUT_EVT_EN_ENTREE;
PUBLIC aut_event AUT_EVT_EN_SORTIE;
PUBLIC aut_event AUT_EVT_TIMEOUT;

#define AUT_ACTION_NULL NULL

#define AUT_ETAT_NULL -1
#define AUT_ETAT_COURANT -2

typedef enum
{
   AUT_OK,
   AUT_TABLE_NOK,
   AUT_ETAT_INITIAL_NOK,
   AUT_NB_ETATS_NOK,
   AUT_ALLOC_NOK,
   AUT_LIBERE_NOK,
   AUT_AUTOMATE_ID_NOK,
   AUT_ARGS_NULL,
   AUT_EVT_INCONNU,
   AUT_ETAT_SUIVANT_NOK,
   AUT_TRANSITION_NOK,
   AUT_NOM_AUTOMATE_NOK,
   AUT_DEBUG_NOK
}
aut_enum_retour;

/*--------------- FUNCTIONS: ---------------*/

PUBLIC aut_enum_retour WINAPI AUT_Lance (OUT aut_automate_id *id,
                                         IN void *data,
                                         IN aut_etat_id etat_initial,
                                         IN aut_etat_id nb_etats,
                                         IN aut_automate table,
                                         IN aut_etat extension,
                                         IN char nom[_MAX_PATH]);

PUBLIC aut_enum_retour WINAPI AUT_Arret (IN OUT aut_automate_id *id);

PUBLIC aut_enum_retour WINAPI AUT_Envoie (IN aut_automate_id id, IN aut_event_id event, IN void *args);

PUBLIC aut_enum_retour WINAPI AUT_RetourEtatPrecedent( aut_automate_id id );

PUBLIC aut_enum_retour WINAPI AUT_Reset(IN aut_automate_id id);

PUBLIC aut_etat_id WINAPI AUT_DonneEtatCourant (IN aut_automate_id id);

PUBLIC aut_event_id WINAPI AUT_DonneEventCourant (IN aut_automate_id id);

PUBLIC boolean WINAPI AUT_EnRecursion (IN aut_automate_id id);

PUBLIC char * WINAPI AUT_DonneNom (aut_automate_id id);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* AUT_H */

