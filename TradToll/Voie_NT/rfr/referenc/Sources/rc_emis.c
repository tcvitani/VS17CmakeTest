/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Traitement de l'emission Inter Tache
* FICHIER: RC_EMIS.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Referenc/Sources/rc_emis.c_v  $
 * 
 *    Rev 1.2   Sep 03 2001 13:11:24   sbatiot
 *  
 * 
 *    Rev 1.1   31 Jan 2001 17:16:32   FR
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.8   03 Nov 1997 16:07:30   DPI
 * Version 4.00
 * 
 *    Rev 1.7   29 Oct 1997 19:13:54   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.6   29 Oct 1997 18:24:16   DPI
 * Evolution ExitAlloue... du noyau 6.00
 * 
 *    Rev 1.5   Jul 09 1997 10:09:44   DPI
 * Changement de l'ordre des includes du
 * a evolution dans 'err.h'
 * 
 *    Rev 1.4   Jul 02 1997 11:54:40   DPI
 * Bug de type mineure
 * 
 *    Rev 1.3   Jun 30 1997 14:27:36   DPI
 * Evolution de 'tcp.h'
 * 
 *    Rev 1.2   Jun 11 1997 17:50:58   DPI
 * Amelioration des traces
 * 
 *    Rev 1.1   Jun 04 1997 19:13:14   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:22   DPI
 *  
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>

/* outils_C */
#include "str.h"
#include "err.h"

/* module noyau */
#include "noyau.h"
//#include "tcp_ip.h"
#include "csr_lan.h"
#include "horodate.h"
#include <fic.h>

#define RC_EMIS_DEF

#include "rc_def.h"
#include "fic_gere.h"

#include "referenc.h"

#include "rc_glob.h"

#include "rc_emis.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RC_DemanderChargementFichier
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi des messages vers le module teste
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_DemanderChargementFichier (noyau_bal_id      BalDest,
                                             noyau_bal_id      BalSource,
                                             noyau_pool_id     NumeroPool,
                                             char              *FichierSource,
                                             char              *PathDest/*,
                                             char              *Hostname*/)
{
   struct_lan_message  *p_message = NULL;
//   char                szSourceName[MAX_PATH];

   ReferenceFichierTrace("=> download file '%s'\n", FichierSource);

   ExitAlloue ((struct_neutre **)(&p_message),
                      sizeof(struct_lan_message),
                      NumeroPool);

   p_message->entete.service = M_LAN_FICHIER;
   p_message->entete.type_message = LAN_RECEPTION;

   strcpy_s(p_message->u.param_fic.hostname, sizeof(p_message->u.param_fic.hostname), RFR.hostname);

   // NP 27/04/01 : if the first character of the FichierSource is '\'
   // don't merge distant path with the source, use the absolute path
   if ( FichierSource[0] == '\\' )
      FIC_makepath(p_message->u.param_fic.fichier, NULL, NULL, FichierSource, NULL);
   else
      FIC_makepath(p_message->u.param_fic.fichier, NULL, RFR.distant_path, FichierSource, NULL);

   strcpy_s(p_message->u.param_fic.path, sizeof(p_message->u.param_fic.path), PathDest);

   ReferenceFichierTrace ("RC_DemanderChargementFichier(): download file from %s to %s",
                          p_message->u.param_fic.fichier, p_message->u.param_fic.path);

   ExitEnvoie (BalDest, BalSource, (struct_neutre *)p_message);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RC_OuvrirServiceFichier
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Ouverture du service de Fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_OuvrirServiceFichier (noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool)
{
/*   short int                   code_rtc;
   struct_lan_message_externe  message;

   message.entete.service = M_LAN_FICHIER;
   message.entete.type_message = LAN_DEBUT;

   code_rtc = EnvoiLan( BalDest, BalSource, NumeroPool,
                        &message);
   ERR_EstVrai( code_rtc == NOYAU_OK );	*/

   struct_lan_message  *p_message = NULL;

   ExitAlloue ((struct_neutre **)(&p_message),
                      sizeof(struct_lan_message),
                      NumeroPool);

   p_message->entete.service = M_LAN_FICHIER;
   p_message->entete.type_message = LAN_DEBUT;
   
   ExitEnvoie (BalDest, BalSource, (struct_neutre *)p_message);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RC_EmettreMessageUpdate
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi des messages vers le module d'emission de message vers le LS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_EmettreMessageUpdate (noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool,
                                        char              *Id,
                                        char              *Reference,
                                        enum_rfr_update   Update)
{
   struct_rfr_message  *p_message = NULL;

   ExitAlloue ((struct_neutre **)(&p_message),
                      sizeof(struct_rfr_message),
                      NumeroPool);

   p_message->entete.service = M_RFR_REFERENCE;
   p_message->entete.type_message = RFR_EMISSION_UPDATE;

   STR_strcpy( MAX_PATH, p_message->u.message_update.id, Id);
   STR_strcpy( MAX_PATH, p_message->u.message_update.reference, Reference);
   p_message->u.message_update.update = Update;

   ReferenceFichierTrace ("RC_EmettreMessageUpdate():BalDest:%d, Id '%s' Ref '%s' update=%d\n",
                          BalDest, Id, Reference, Update);

   ExitEnvoie (BalDest, BalSource, (struct_neutre *)p_message);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RC_EmettreMessageManuel
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi du message d'acquittement d'un changement manuel
*       Ce message va directement dans l'applicatif
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_EmettreMessageManuel (noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool,
                                        char              *Id,
                                        char              *Fichier,
                                        boolean           Fait)
{
   struct_rfr_message  *p_message = NULL;

   ExitAlloue ((struct_neutre **)(&p_message),
                      sizeof(struct_rfr_message),
                      NumeroPool);

   p_message->entete.service = M_RFR_MANUEL;
   if( Fait)
      p_message->entete.type_message = RFR_FICHIER_MANUEL_ACK;
   else
      p_message->entete.type_message = RFR_FICHIER_MANUEL_NACK;

   STR_strcpy( MAX_PATH, p_message->u.message_manuel.id, Id);
   STR_strcpy( MAX_PATH, p_message->u.message_manuel.fichier, Fichier);

   ExitEnvoie (BalDest, BalSource, (struct_neutre *)p_message);
}


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RC_AcquitterArret
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Ouverture du service de Fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RC_AcquitterArret (noyau_bal_id      BalDest,
                                  noyau_bal_id      BalSource,
                                  noyau_pool_id     NumeroPool)
{
   struct_rfr_message  *p_message = NULL;

   ExitAlloue ((struct_neutre **)(&p_message),
                      sizeof(struct_rfr_message),
                      NumeroPool);

   p_message->entete.service = M_RFR_ARRET;
   p_message->entete.type_message = RFR_ARRET_ACQ;

   ExitEnvoie (BalDest, BalSource, (struct_neutre *)p_message);

}
