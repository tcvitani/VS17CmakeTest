/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Display Unit
* FICHIER: AFF_ANI.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Declarations de constantes et variables globales a la tache
*         AFF_ANI.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Aff/AFF/Includes/Aff_ani.h_v  $
 * 
 *    Rev 1.2   Jul 19 2001 16:54:58   sbatiot
 *  
 * 
 *    Rev 1.1   Mar 31 2000 14:43:50   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.3   Nov 17 1999 10:54:36   nbl
 *  
 * 
 *    Rev 1.2   Nov 15 1999 09:37:12   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:24   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:22   nbl
 *  
 * 
 *    Rev 1.0   Mar 27 1998 11:13:04   pgg
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   19 Jan 1998 10:39:40   ANA
 *  
 * 
 *    Rev 1.1   02 Dec 1997 14:29:34   ANA
 * Gestion de la liste chainee et de la section #STRINGS
 * 
 *    Rev 1.0   21 Nov 1997 17:01:48   ANA
 * Version initiale
 *
 * Renamed from SGM to AFF 21.07.1999. sasa - ecsat
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef AFF_ANI_H
#define AFF_ANI_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>
/*----------------DEFINES:-----------------*/

/*--------------- TYPEDEF: ----------------*/

/*--------------- FUNCTIONS: ---------------*/
PROTECTED void InitAffAni(short int ident_aff);

PROTECTED void AFFReceptionService(short int ident_aff,
                                   struct_neutre *p_neutre);

PROTECTED void AFFReceptionIos (short int ident_aff,
                                struct_neutre *p_neutre);

/*-------------- VARIABLES: ---------------*/

/*------------------------------------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif


