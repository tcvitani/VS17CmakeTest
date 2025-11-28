/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base comparant 2 fichiers de ref‚rence
* FICHIER: COMP_REF.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Compare 2 fichiers de r‚f‚rence, en construit un troisiŠme
*         et fait des actions.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/comp_ref.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:40   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:06:54   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jul 29 1997 15:31:12   DPI
 * Correction bug dans comparaison des 
 * fichiers de reference
 * 
 *    Rev 1.0   Apr 24 1997 14:59:24   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef COMP_REF_H
#define COMP_REF_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDE 2: ----------------*/
/*--------------- INCLUDES: ---------------*/
#ifdef COMP_REF_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean EstCorrecteReference( TpTGereFic   GereFic);

PUBLIC void    ComparerReferences( TpTGereFic           GereAncien,
                                   TpTGereFic           GereNouveau,
                                   TTraiterReference    TraiterItemDifferent,
                                   TTraiterReference    TraiterItemIdentique,
                                   boolean              ParReference,
                                   void                 *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
