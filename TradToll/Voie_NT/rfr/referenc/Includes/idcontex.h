/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement d'un id context
* FICHIER: idcontex.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/idcontex.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:06:58   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jun 03 1997 15:26:20   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.0   Apr 24 1997 14:59:30   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef IDCONTEX_H
#define IDCONTEX_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef IDCONTEX_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/

PUBLIC void    InitialiserIdContext( TpTTacheContext  Context,
                                     TFichier         *RefFichier,
                                     TFichier         *RefAncienFichier,
                                     boolean          EffacableAncienFichier,
                                     TReception       *Id,
                                     TIdContext       *IdContext,
                                     boolean          Change,
                                     boolean          Manuel);
PUBLIC void    EnregistrerIdContext( TpTTacheContext  Context,
                                     TIdContext       *IdContext,
                                     long             *Position);
PUBLIC boolean RetirerIdContext( TpTTacheContext  Context,
                                 long             Position);



#endif
