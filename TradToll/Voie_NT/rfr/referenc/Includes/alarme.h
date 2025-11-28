/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des alarmes
* FICHIER: alarme.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/alarme.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:40   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:06:54   DPI
 * Version 4.00
 * 
 *    Rev 1.2   May 13 1997 09:52:22   DPI
 * Blocage du temps sur alarme
 * 
 *    Rev 1.1   Apr 24 1997 16:44:06   DPI
 * Ajout de EffacerAlarmeUsee
 * 
 *    Rev 1.0   Apr 24 1997 14:59:22   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ALARME_H
#define ALARME_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef ALARME_DEF
	#include <public.h>
#else
	#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/
PUBLIC boolean       RecevoirAlarme( TpTTacheContext  Context,
                                     TAlarmeExterne   *AlarmeExterne);
PUBLIC boolean       EffacerAlarmeUsee( TpTTacheContext  Context,
                                        TAlarmeExterne   *AlarmeExterne);
#undef PUBLIC
#undef I
#undef INIT
#endif
