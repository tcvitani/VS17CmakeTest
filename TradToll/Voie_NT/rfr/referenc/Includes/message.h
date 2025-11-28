/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des messages
* FICHIER: messages.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/message.h_v  $
 * 
 *    Rev 1.1   Sep 03 2001 13:11:16   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:06:58   DPI
 * Version 4.00
 * 
 *    Rev 1.1   29 Oct 1997 19:14:16   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.0   Apr 24 1997 14:59:32   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef MESSAGE_H
#define MESSAGE_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef MESSAGE_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/
/*
EXPORT boolean WINAPI AttendreMessage( TpTIdContext     IdContext,
                                      noyau_bal_id     Bal,
                                      TTacheMessage    Message,
                                      TIdMessage       *IdMessage,
                                      TFcn_DoIt        Faire);
									  */
PUBLIC boolean       RecevoirMessage( TpTTacheContext  Context,
                                      noyau_bal_id           Bal,
                                      TTacheMessage    Message);
/*
EXPORT boolean WINAPI OublierMessage( TpTIdContext      IdContext,
                                     TIdMessage        *IdMessage);
*/
#undef PUBLIC
#undef I
#undef INIT
#endif
