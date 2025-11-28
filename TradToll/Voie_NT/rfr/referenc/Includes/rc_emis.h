/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Traitement de la emission Inter tache
* FICHIER: RC_EMIS.H
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rc_emis.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:07:02   DPI
 * Version 4.00
 * 
 *    Rev 1.1   29 Oct 1997 19:14:18   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.0   Apr 24 1997 14:59:36   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RC_EMIS_H
#define RC_EMIS_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#include <protect.h>

/*--------------- TYPES ---------------*/

/*--------------- FONCTIONS ---------------*/

PROTECTED void RC_DemanderChargementFichier( noyau_bal_id      BalDest,
                                             noyau_bal_id      BalSource,
                                             noyau_pool_id     NumeroPool,
                                             char              *FichierSource,
                                             char              *PathDest);
                                             //,char              *Hostname);
PROTECTED void RC_OuvrirServiceFichier( noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool);
PROTECTED void RC_EmettreMessageUpdate( noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool,
                                        char              *Id,
                                        char              *Reference,
                                        enum_rfr_update   Update);
PROTECTED void RC_EmettreMessageManuel( noyau_bal_id      BalDest,
                                        noyau_bal_id      BalSource,
                                        noyau_pool_id     NumeroPool,
                                        char              *Id,
                                        char              *Reference,
                                        boolean           Fait);
PROTECTED void RC_AcquitterArret( noyau_bal_id      BalDest,
                                  noyau_bal_id      BalSource,
                                  noyau_pool_id     NumeroPool);

#endif
