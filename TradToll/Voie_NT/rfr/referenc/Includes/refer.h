/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement du fichier de r‚f‚rence
* FICHIER: refer.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION: Ce module traite le fichier de r‚f‚rence.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/refer.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:46   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:07:04   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:40   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef REFER_H
#define REFER_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef REFER_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- INCLUDES: ---------------*/

PUBLIC boolean ComparerFichiersReference( TpTTacheContext       Context);
PUBLIC void    RecevoirFichierReference( TpTTacheContext       Context);
PUBLIC boolean DemanderFichierReference( TpTTacheContext  Context);
PUBLIC void    AcquitterFichierReference( TpTTacheContext       Context,
                                          boolean               Acquittement);
PUBLIC void    CreerFichierReferenceManuel( TpTTacheContext  Context);

PUBLIC void DeleteUnusedRegKey(TpTTacheContext Context);

#endif
