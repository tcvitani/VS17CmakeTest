/************** (v) 2017 EMOVIS - All rights reserved ***************/
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
