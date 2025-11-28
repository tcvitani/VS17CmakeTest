/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Traitement de la reception Inter tache
* FICHIER: RC_RECPT.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_recpt.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:07:02   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jun 03 1997 11:48:22   DPI
 * Suppression de ReceptionRfrApplicatif
 * 
 *    Rev 1.0   Apr 24 1997 14:59:38   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RC_RECPT_H
#define RC_RECPT_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#include <protect.h>

/*--------------- TYPES ---------------*/

/*--------------- FONCTIONS ---------------*/

PROTECTED void        ReceptionRfrService( TpTTacheContext     Context,
                                           struct_rfr_message *p_message);
PROTECTED void        ReceptionRfrFichier( TpTTacheContext     Context,
                                           struct_lan_message *p_message);
PROTECTED void        ReceptionRfrAlarme( TpTTacheContext     Context,
                                          struct_hrd_message *p_message);
PROTECTED void        ReceptionRfrTime( TpTTacheContext     Context);

#endif
