
/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Gestion du path d'un fichier
* FICHIER: path.h
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
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/path.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:44   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:07:00   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:32   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef PATH_H
#define PATH_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef PATH_DEF
#include <public.h>
#else
#include <export.h>
#endif



/*--------------- FONCTIONS ---------------*/

PUBLIC void ChangerPathTransfert( char *FichierDest, char *FichierSource);
PUBLIC void ChangerPathReference( char *FichierDest, char *FichierSource);
PUBLIC void ChangerPathVirtuel( char *FichierDest, char *FichierSource);
PUBLIC void ChangerPath( char *FichierDest, char *FichierSource, char *Path);


#endif
