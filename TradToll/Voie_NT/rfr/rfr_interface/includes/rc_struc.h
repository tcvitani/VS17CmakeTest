/*------   (v) 1997 CS-Route   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Definition des types/structures
* FICHIER: RC_STRUC.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Definition des types et structures utilis‚es dans la
*         r‚ception de fichiers et export‚es vers l'application.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_struc.h_v  $
 * 
 *    Rev 1.1   31 Jan 2001 17:16:26   FR
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:46   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.6   03 Nov 1997 16:07:02   DPI
 * Version 4.00
 * 
 *    Rev 1.5   29 Oct 1997 19:14:18   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.4   Jun 27 1997 10:46:56   DPI
 * Sur iteration absente, mettre une valeur 
 * dans la reference pour emission vers le LS
 * 
 *    Rev 1.3   May 14 1997 14:02:50   DPI
 * AJout de la Bal et du Pool dans l'IdContext
 * 
 *    Rev 1.2   May 12 1997 13:43:44   DPI
 * 1- Gerer les actions obligatoires
 * 2- Sur Manuel, gérer comme cas standard
 * 
 *    Rev 1.1   Apr 25 1997 14:57:08   DPI
 * Intégration de Horodate 2.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:38   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RC_STRUC_H
#define RC_STRUC_H

/*--------------- INCLUDES: ---------------*/


/* pour inclure horodate, il faut inclure le noyau */
#include "noyau.h"
/* il faut inclure  horodate, car tous les modules qui ne s'en servent pas
 * ne compilent pas.
 */
#include "horodate.h"

/*--------------- INCLUDES: ---------------*/
#ifdef RC_DEF_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*----------------------------------------------------------*/
/* Constantes   */

#include "rfr_cnst.h"

#define           RFR_MAX_BUFFER_PARAM  ( 400)

//#define           PATH_TRANSFERT        RFR.path_transfert
//#define           PATH_REFERENCE        RFR.path_reference
//#define           DISQUE                RFR.disque
//#define           DISQUE_VIRTUEL        RFR.disque_virtuel

/* ce define ci-dessous est utilis‚ pour emettre vers le LS */
#define           ID_FICHIER_REFERENCE  "001"
#define           ID_REFERENCE_MANUELLE  "0000000000000000" /* 16 fois '0' */
#define           ID_REFERENCE_ABSENTE   "----------------" /* 16 fois '-' */

/*--------------- TYPEDEF: ---------------*/

typedef enum { NO_REASON, VERIFY, START, MANUAL, DO_IT, RC_ERROR, ABORT}    TEnum_Reason;
/* NO_REASON est utilis‚ en interne
 * VERIFY est utilise lors de l'appel d'une fonction TFcn_Verify
 * START  est utilise lors de l'appel d'une fonction TDoIt et
 *        signifie que l'application est en demarrage
 * MANUAL est utilise lors de l'appel d'une fonction TDoIt et
 *        signifie que l'iteration … traiter vient d'ˆtre fabriqu‚
 *        manuellement par l'op‚rateur. Dans ce cas, il n'existe pas
 *        d'iteration BEFORE et AFTER meme si l'it‚ration est non nulle
 * DO_IT  est utilise lors de l'appel d'une fonction TDoIt et
 *        signifie que tout va bien, et que le traitement continue
 * RC_ERROR  est utilise lors de l'appel d'une fonction TDoIt et
 *        signifie que l'‚vŠnement attendu ne se produira pas.
 * ABORT  est utilise lors de l'appel d'une fonction TDoIt et
 *        signifie que le fichier a change, et que le traitement est
 *        abandonne.
 */

typedef enum { ACK, NACK}     TEnum_Verified;
/* ACK    signifie que le fichier est valide
 * NACK   signifie que le fichier est invalide, mais on continue l'analyse
 *        du fichier de reference
 */

typedef enum { DONE, NOT_DONE}         TEnum_Done;
/* DONE       signifie que le traitement de la fonction est r‚ussi,
 * NOT_DONE   signifie que le traitement de la fonction a ‚chou‚,
 *            et qu'elle doit ˆtre rappel‚e au cycle suivant.
 */

/* Define g‚n‚raliste, pour les index sur tableaux ou fichiers
 * Il DOIT ˆtre utilis‚ pour tous les index
 */
#define NO_ITEM               -1

/* Tous ces identifiants sont en faits des index sur des tableaux */
#define     NO_ALARME         NO_ITEM
#define     NO_MESSAGE        NO_ITEM
#define     NO_ID_RECEPTION   NO_ITEM

/* ceci est une encapsulation de l'identifiant de Horodate avec changement
 * de format
 */
typedef t_hrd_num_reveil       TAlarmeExterne;
#define     NO_ALARME_EXTERNE NO_ITEM

typedef long                  TIdAlarme;
typedef long                  TIdMessage;
typedef long                  TIdReception;

/* ceci identifie une iteration comme un entier croissant commencant … 0
 * - 0 signifie pas d'it‚ration
 * - 1 … n pour les it‚rations non vides
 * - BEFORE_ITERATIONS avant le traitement de la premiŠre it‚ration
 * - AFTER_ITERATIONS aprŠs le traitement de la derniŠre it‚ration
 * - MISSING_ITERATION lorsqu'il n'y a aucune it‚ration, donc aucun fichier
 */
#define BEFORE_ITERATIONS     -1
#define AFTER_ITERATIONS      -2
#define MISSING_ITERATION     -3
typedef int                   TIteration;

/* ceci permet d'identifier un message d'une tache */
typedef enum { QUELCONQUE}    TTacheMessage;


typedef struct {
                  /* donn‚es de Temps Reel */
                  noyau_pool_id   NumPool;
                  noyau_bal_id    BalLocale;

                  /* donn‚es applicatives */
                  char          Id[ RFR_MAX_ID];
                  TIteration    Iteration;
                  /* valeur de l'iteration en cours de traitement*/
                  TIdReception  IdReception;
                  /* identifiant interne */
                  TEnum_Reason  AppelRaison;
                  /* raison de l'appel … la fonction */
                  boolean      Change;
                  /* est-ce que le fichier vient de changer, ou est-ce
                   * du … un nouveau fichier de r‚f‚rence
                   */
                  char          NouveauFichier[ RFR_MAX_FIC];
                  char          NouvelleReference[ RFR_MAX_REF];
                  /* nom et reference du nouveau fichier */
                  char          AncienFichier[ RFR_MAX_FIC];
                  char          AncienneReference[ RFR_MAX_REF];
                  /* nom et reference de l'ancien fichier */
                  boolean       EstEffacableAncienFichier;
                  /* indicateur autorisant l'effacement du fichier */
                  char          BufferParam[ RFR_MAX_BUFFER_PARAM];
                  /* zone allou‚e pour stocker les informations n‚cessaires
                   * au traitement applicatif de l'iteration
                   */

                  /*------------------------------------------------*/
                  /* champs utilis‚s en interne */
                  long           IndiceIdContext;
                  /* sert uniquement aux chainages des IdContext
                   * qui sont libres
                   */
                  int            NbActions;
                  /* sert … savoir combien d'actions partagent la
                   * structure
                   */
                  long           Position;
                  /* indique la position de la structure elle-mˆme */
                  void           *TacheContext;
                  /* Fournit le contexte de la tache */
               } TIdContext, *TpTIdContext;

typedef struct
        {
            struct_hrd_date_entree_hex    Date;
            struct_hrd_heure_hex          Heure;
        } TDateHeure, *TpTDateHeure;

/* signature des methodes */
typedef struct struct_rfr_record *TpTRfrRecord;

typedef void      (* TFcn_Recorded)(void);
typedef void      (* TFcn_RecordedEx)(TpTRfrRecord p_record);

typedef TEnum_Verified      (* TFcn_Verify)( TpTIdContext   IdContext,
                                             char           *Commentaire
                                             /* RFR_MAX_COMMENTAIRE */);

// FR 31/01/01 : gestion des acquittements de tables
// (define utilise pour le parametre "Commentaire" des fonctions de prototype TFcn_DoIt)
#define  RFR_FILE_ALREADY_PRESENT   "RFR_FILE_ALREADY_PRESENT"

typedef TEnum_Done          (* TFcn_DoIt)( TpTIdContext   IdContext,
                                           char           *Commentaire
                                             /* RFR_MAX_COMMENTAIRE */);

typedef struct struct_rfr_record
{
	char          IdName[MAX_PATH];
	char          Path[MAX_PATH];
	boolean       Iterable;
	boolean       Obligatoire;
	TFcn_Recorded Enregistree;
	TFcn_Verify   Verifier;
	TFcn_DoIt     Faire;
	int           TailleArgs;
	TFcn_RecordedEx EnregistreeEx;
	
}struct_rfr_record, TRfrRecord, *TpTRfrRecord;

/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
/* Fonctions de DoIt */

EXPORT boolean WINAPI AttendreAlarme( TpTIdContext    IdContext,
                                     TpTDateHeure    DateHeure,
                                     TIdAlarme       *IdAlarme,
                                     TFcn_DoIt       Faire);
EXPORT boolean WINAPI OublierAlarme( TpTIdContext     IdContext,
                                    TIdAlarme        *IdAlarme);


EXPORT boolean WINAPI AttendreMessage( TpTIdContext       IdContext,
                                      noyau_bal_id       Bal,
                                      TTacheMessage      Message,
                                      TIdMessage         *IdMessage,
                                      TFcn_DoIt          Faire);
EXPORT boolean WINAPI OublierMessage( TpTIdContext        IdContext,
                                     TIdMessage          *IdMessage);

/*----------------------------------------------------------*/
/* Fonction d'initialisation */

EXPORT boolean WINAPI RFRNewTableauId(TpTRfrRecord p_record);



#endif
