/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement d'un fichier item du fichier de reference
* FICHIER: item_fic.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/item_fic.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   19 Mar 1998 14:31:58   DPI
 * - sur MESSAGE_MANUEL, le fichier manuel est vérifié
 * - MESSAGE_MANUEL_ACK est envoyé à la tache appelante
 * - sur Cold Start, tous les repertoires applicatifs
 *   sont effacés
 * 
 *    Rev 1.1   03 Nov 1997 16:06:58   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:30   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ITEM_FIC_H
#define ITEM_FIC_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef ITEM_FIC_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/

PUBLIC void    RecevoirUnFichier( TpTTacheContext   Context,
                                  char              *NouveauFichier,
                                  boolean           Charge);
PUBLIC boolean VerifierFichierManuel( TpTTacheContext  Context);



#endif
