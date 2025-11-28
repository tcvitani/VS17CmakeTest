/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Definition des types/structures
* FICHIER: FIC_REC.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Definition des types et structures utilis‚es dans la
*         r‚ception de fichiers
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_def.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:00   DPI
 * Version 4.00
 * 
 *    Rev 1.2   29 Oct 1997 19:14:16   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.1   May 12 1997 13:43:42   DPI
 * 1- Gerer les actions obligatoires
 * 2- Sur Manuel, gérer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 14:59:36   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RC_DEF_H
#define RC_DEF_H

#include "rc_struc.h"

/*--------------------------------------------------------------------*/
/* Gestion des fichiers de taille variable */

/* ATTENTION, il ne faut pas changer ce define, car on exploite
 * le fait que le premier element du fichier est reserv‚
 */
#define           INDICE_ITEM_LIBRE           0
#define           DECALAGE_ITEM_LIBRE         1
#define           ITEM_OCCUPE                 -1

/*--------------------------------------------------------------------*/
/* Gestion des actions */

#define NO_ACTION        NO_ITEM
/* NOTA: on utilise ‚galement NO_ALARME au lieu de NO_ACTION
 *       ainsi que NO_MESSAGE
 */
typedef enum { ALARME, MESSAGE, RECEPTION}  TEnum_TypeAction;

typedef struct Struct_ReceptionAction
               {
                  /* identification */
                  TIdReception        IdReception;
                  TIteration          Iteration;
                  long                IndiceIdContext;

                  /* identification de l'alarme pour Horodate */
                  TAlarmeExterne      AlarmeExterne;

                  /* traitement … faire */
                  TFcn_DoIt           Faire;

                  /* type d'evenement declenchant */
                  TEnum_TypeAction    TypeAction;
                  long                IndiceType;

                  /* Chainage par IdReception identique */
                  long                    ActionPrecedente_ID;
                  long                    ActionSuivante_ID;

                  /* Chainage par evenement identique */
                  long                   ActionPrecedente_EVENT;
                  long                   ActionSuivante_EVENT;

                  long                   IndiceAction;
                  /* sert uniquement aux chainages des IdContext
                   * qui sont libres
                   */
               } TReceptionAction, *TpTReceptionAction;
/* Cette structure identifie une action … faire.
 * Elle contient ‚galement toutes les cl‚s ‚trangŠres pour acc‚der
 * aux autres tables
 */


/*--------------------------------------------------------------------*/
/* Gestion des alarmes */

typedef enum { NO_ALARM, WAITING_ACK, WAITING_ALARM } TEnum_EtatAlarme;
typedef struct {
                  TEnum_EtatAlarme       Etat;
                  TDateHeure             DateHeure;

                  TAlarmeExterne         AlarmeExterne;

                  /* chainage des actions … faire */
                  long                   PremiereAction;
                  long                   DerniereAction;

                  /* chainage des alarmes vides */
                  long                    IndiceAlarme;
                  /* indique la position de la structure elle-mˆme */
                  long                    Position;
               } TReceptionAlarme, *TptReceptionAlarme;
/* Cette structure identifie une alarme attendue
 * Elle contient ‚galement deux cl‚s ‚trangŠres pour acc‚der
 * … la table des actions.
 * N'oublions pas qu'une mˆme alarme peut ˆtre attendu par plusieurs
 * ‚vŠnements ( mˆme date)
 */

/* Le type TIdAlarme d‚clar‚ dans le fichier RC_STRUC.H est utilis‚
 * pour identifier une alarme par l'applicatif.
 * Le contenu de ce type est la position de l'action qui est appel‚
 * par cette alarme pour cet ID/It‚ration donn‚e
 */


/*--------------------------------------------------------------------*/
/* Gestion des messages des taches */

typedef enum { NO_WAITING, WAITING_MESSAGE } TEnum_EtatMessage;
typedef struct {
                  TEnum_EtatMessage      Etat;
                  noyau_bal_id           Bal;
                  TTacheMessage          Message;

                  /* chainage des actions … faire */
                  long                   PremiereAction;
                  long                   DerniereAction;
                  /* chainage des messages vides */
                  long                    IndiceMessage;
                  /* indique la position de la structure elle-mˆme */
                  long                    Position;
               } TReceptionMessage, *TptReceptionMessage;
/* Cette structure identifie un message attendu
 * Elle contient ‚galement deux cl‚s ‚trangŠres pour acc‚der
 * … la table des actions.
 * N'oublions pas qu'un mˆme message peut ˆtre attendu par plusieurs
 * ‚vŠnements
 */


/*--------------------------------------------------------------------*/
/* Gestion des identifiants de fichiers */

typedef struct {
                  /* cette partie est initialis‚e au d‚marrage */
                  char             Path[ RFR_MAX_PATH];
                  char             Id  [ RFR_MAX_ID];
                  TIdReception     IdReception;

                  boolean          Iterable;
                  boolean          Obligatoire;
                  TFcn_Verify      Verifier;
                  TFcn_DoIt        Faire;

                  /* chainage des actions … faire */
                  /* cette partie est modifi‚e dynamiquement */
                  long                   PremiereAction;
                  long                   DerniereAction;
               } TReception, *TpTReception;
/* Cette structure identifie un type de fichier attendu, pour sa partie
 * statique.
 * La partie dynamique contient le lien vers les diverses actions en cours
 */

/*--------------------------------------------------------------------*/
/* Contenu du fichier de reference */

/* le define suivant est utilis‚ pour identifier le fichier de r‚f‚rence
 * transmis par le LS depuis le fichier courant.
 * L'identifation est rajout‚ dans le fichier courant par la Voie, et pas
 * par le LS, qui s'engage … ne pas utilis‚ cette valeur
 */
#define    REFERENCE_ID            "FILE_REFERENCE"
typedef struct {
                  char              Id[ RFR_MAX_ID];
                  unsigned int      Iteration;
                  char              Fichier[ RFR_MAX_FIC];
                  char              Reference[ RFR_MAX_REF];
                  char              Hostname[ RFR_MAX_HOST];
               } TFichier;
/* cette structure d‚crit les donn‚es disponibles dans le fichier de
 * r‚f‚rence
 */
#endif