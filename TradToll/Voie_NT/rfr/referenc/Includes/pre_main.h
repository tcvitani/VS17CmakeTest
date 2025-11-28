/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Definition
* FICHIER: PRE_MAIN.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/pre_main.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:00   DPI
 * Version 4.00
 * 
 *    Rev 1.2   May 14 1997 15:42:32   DPI
 *  
 * 
 *    Rev 1.1   May 12 1997 13:43:42   DPI
 * 1- Gerer les actions obligatoires
 * 2- Sur Manuel, gérer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 14:59:34   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef PRE_MAIN_H
#define PRE_MAIN_H

/*--------------- INCLUDES: ---------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef PRE_MAIN_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*----------------------------------------------------------*/
/* Fonction d'initialisation */

//EXPORT boolean WINAPI RFRNewTableauId(TpTRfrRecord p_record);
PUBLIC void          CreerRepertoires( void);


#endif
