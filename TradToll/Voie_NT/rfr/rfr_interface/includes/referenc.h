/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Declaration externe de la tache REFERENCE
* FICHIER: REFERENC.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
*             La tache REFERENCE s'appelle exactement 'Reception du
*             fichier de R‚f‚rence'.
*             Elle recoit des messages en provenance
*             - de la tache de com
*             - de Horodate
*             - de la tache R‚seau
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/referenc.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:46   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.11   Jan 22 1999 15:22:10   FR
 * Mise à jour du numéro de version
 * 
 *    Rev 1.10   19 Mar 1998 14:31:58   DPI
 * - sur MESSAGE_MANUEL, le fichier manuel est vérifié
 * - MESSAGE_MANUEL_ACK est envoyé à la tache appelante
 * - sur Cold Start, tous les repertoires applicatifs
 *   sont effacés
 * 
 *    Rev 1.9   11 Feb 1998 16:33:14   DPI
 * Version
 * 
 *    Rev 1.8   03 Dec 1997 08:20:54   DPI
 * Evolution de version pour TCPx.LIB
 * 
 *    Rev 1.7   03 Nov 1997 16:07:04   DPI
 * Version 4.00
 * 
 *    Rev 1.6   29 Oct 1997 19:28:42   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 *    Rev 1.5   29 Oct 1997 19:14:20   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.4   Aug 06 1997 15:50:52   DPI
 * Version 3.00 de TCPx.LIB
 * 
 *    Rev 1.3   Jul 29 1997 15:31:18   DPI
 * Correction bug dans comparaison des 
 * fichiers de reference
 * 
 *    Rev 1.2   Jul 28 1997 18:44:02   DPI
 * Gestion de la version
 * 
 *    Rev 1.1   Jun 03 1997 11:48:58   DPI
 * Ajout du service M_RFR_APPLICATIF
 * 
 *    Rev 1.0   Apr 24 1997 14:59:42   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef REFERENC_H
#define REFERENC_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#include <csrlc32.h>
#include <run.h>

#ifdef REFERENC_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include "rc_struc.h"

/*--------------- VERSION: ---------------*/

#define REFERENC_VERSION                "6.00"

/*--------------- DEFINES: ---------------*/


/*--------------- TYPES ---------------*/
/* erreurs possibles pour les retour des fonctions du module  */

/* ces valeurs sont celles emises en mai 97 vers le LS */
typedef enum
{
   ALREADY_CHECKED_FILE = 0,
   INVALID_FILE = 1,
   CHECKED_FILE = 2,
   ACTIVATED_FILE = 3,
   PARTIAL_ACTIVATION_FILE = 4,
   UNLOAD_FILE = 5,
   TRASHING_FILE = 6,
} enum_rfr_update;

typedef enum
{
   PREMIER_RFR_SERVICE = 0,

   M_RFR_ARRET = PREMIER_RFR_SERVICE,
   M_RFR_REFERENCE,
   M_RFR_MANUEL,
   M_RFR_APPLICATIF,

   BUTEE_RFR_SERVICE,
   DERNIER_RFR_SERVICE = BUTEE_RFR_SERVICE - 1,
   NB_RFR_SERVICE = BUTEE_RFR_SERVICE - PREMIER_RFR_SERVICE

} enum_rfr_service;

typedef enum
{
   PREMIER_RFR_TYPE = 0,

   /*types de message du service M_RFR_ARRET*/
   RFR_DEMANDE = PREMIER_RFR_TYPE,
   RFR_ARRET_ACQ,

   /*types de message du service M_RFR_REFERENCE */
   RFR_RECEPTION_REFERENCE,                    /* in */
   RFR_EMISSION_UPDATE,                        /* out */

   /*types de message du service M_RFR_MANUEL */
   RFR_FICHIER_MANUEL,                         /* in */
   RFR_FICHIER_MANUEL_ACK,                     /* out */
   RFR_FICHIER_MANUEL_NACK,                    /* out */

   BUTEE_RFR_TYPE,
   DERNIER_RFR_TYPE = BUTEE_RFR_TYPE - 1,
   NB_RFR_TYPE = BUTEE_RFR_TYPE - PREMIER_RFR_TYPE
} enum_rfr_type;

typedef struct
{
   char              id       [ RFR_MAX_ID];
   char              fichier  [ RFR_MAX_FIC];
} struct_rfr_message_manuel;

typedef struct
{
   char              id       [ RFR_MAX_ID];
   char              reference[ RFR_MAX_REF];
   enum_rfr_update   update;
} struct_rfr_message_update;

typedef struct
{
   char        fichier  [ RFR_MAX_FIC];
   char        reference[ RFR_MAX_REF];
} struct_rfr_message_ref;

typedef struct
{
   struct_neutre        neutre;
   enum_rfr_service     service;
   enum_rfr_type        type_message;
} struct_rfr_entete;

/* si le service est M_RFR_APPLICATIF, il n'y a pas de champ dans l'union */
typedef struct
{
   struct_rfr_entete entete;
   union
   {
      struct_rfr_message_manuel  message_manuel;   /* in/out */
      struct_rfr_message_ref     message_ref;      /* in */
      struct_rfr_message_update  message_update;   /* out */
   }u;
} struct_rfr_message;

/* parametres de configuration du module */
typedef struct
{
     short int       priorite_init_max;
     short int       priorite_tache_max;
     noyau_pool_id   num_pool;

     noyau_bal_id    bal_fichier;
     noyau_bal_id    bal_message;
     noyau_bal_id    bal_horodate;

     boolean         cold_start;
} struct_rfr_config;

/*--------------- FONCTIONS ---------------*/
EXPORT enum_instance_result WINAPI ReferenceLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI ReferenceArret(noyau_bal_id iBalId);

#undef PUBLIC
#undef I
#undef INIT
#endif
