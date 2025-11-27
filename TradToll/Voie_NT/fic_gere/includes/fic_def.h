/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FIC_GERE (Gestion de fichier quelconques).
* FICHIER: FIC_DEF.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ficher d'entete interne du module FIC_GERE
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface interne
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fic_gere/Includes/fic_def.h_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:38   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:41:50   bph
 *  
 * 
 *    Rev 1.2   Mar 04 1997 14:13:26   HMO
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include "fic_gere.h"

#ifdef FIC_DEF_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- INCLUDES: ---------------*/


/*--------------- RESERVED: ---------------*/


/*--------------- EXTERNALS: --------------*/


/*--------------- DEFINES: ----------------*/

#define FIC_BUFFER_SIZE 512


/*--------------- FUNCTIONS: --------------*/

EXPORT FIC_enum_retour WINAPI Fichier_LireIdPos( TGereFic *config,
                                                 long      position_depart,
                                                 TId      *Id,
                                                 TParam   *param,
                                                 long      *position,
                                                 TItem    **Item,
                                                 TVal     **Val);


/* Cette fonction servira si l'utilisateur sait comment se deplacer */
/* dans son fichier son utilisation n'est pas impl‚ment‚e actuellement */
/*
typedef FIC_enum_retour (*TCherchePos)( FILE   *Fichier,
                                        int    position_recherche,
                                        int    position_courante,
                                        size_t *offset);*/

#undef PUBLIC
#undef I
#undef INIT
