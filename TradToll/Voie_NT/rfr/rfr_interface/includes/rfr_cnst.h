/*------   (v) 1997 CS-Route   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Definition des constantes utilis‚s pour la gestion des
*          r‚f‚rences
* FICHIER: RFR_CNST.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Definition des constantes utilis‚es dans la
*         r‚ception de fichiers et export‚es vers l'application.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
*
*              Ces constantes sont celles utilis‚es entre le LS et la
*              VM pour toutes les donn‚es du fichier de configuration,
*              et pour certaines donn‚es des fichiers appartenant au
*              fichiers de configuration
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/rfr_cnst.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:46   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:07:04   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:42   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef RFR_CNST_H
#define RFR_CNST_H

/*----------------------------------------------------------*/
/* Constantes   */

#define           RFR_MAX_PATH     MAX_PATH
#define           RFR_MAX_ID       ( 20+1)
#define           RFR_MAX_FIC      RFR_MAX_PATH
#define           RFR_MAX_REF      RFR_MAX_PATH
#define           RFR_MAX_HOST     ( 16+1)

#define           RFR_MAX_COMMENTAIRE   ( 30+1)

#endif
