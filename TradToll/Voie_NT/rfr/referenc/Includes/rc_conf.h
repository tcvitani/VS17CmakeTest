/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Gestion du fichier de configuration
* FICHIER: rc_conf.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_conf.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:07:00   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:34   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RC_CONF_H
#define RC_CONF_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#include <protect.h>

PROTECTED void RC_LireConfiguration( TpTTacheContext Context);
PROTECTED void RC_EcrireNouvelleConfiguration( TpTTacheContext Context);


/*--------------- FONCTIONS ---------------*/

#endif
