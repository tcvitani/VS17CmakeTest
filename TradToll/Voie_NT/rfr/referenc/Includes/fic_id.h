/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES ID
* FICHIER: FIC_ID.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les ID que
*         reconnait l'application dans un fichier de r‚f‚rence.
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_id.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:06:56   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:26   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_ID_H
#define FIC_ID_H

/*--------------- DEFINES: ----------------*/


/*--------------- INCLUDES: ---------------*/
#ifdef FIC_ID_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterIdReception)( TGereFic        *GereFic,
                                              FIC_enum_retour Etat,
                                              long            Position,
                                              TParam          *Param,
                                              TVal            *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean InitFichierIdReception( TGereFic         *GereFic,
                                       char*            Attributs);
PUBLIC boolean FermerFichierIdReception( TGereFic       *GereFic);

PUBLIC FIC_enum_retour LireIdIdReception( TGereFic      *GereFic,
                                          char          *Id,
                                          TReception    **Item);
PUBLIC FIC_enum_retour LirePosIdReception( TGereFic     *GereFic,
                                           long           Position,
                                           TReception    **Item);
PUBLIC FIC_enum_retour AjouterPosIdReception( TGereFic      *GereFic,
                                              TReception    *Item);
PUBLIC FIC_enum_retour EcrirePosIdReception( TGereFic      *GereFic,
                                             long           Position,
                                             TReception    *Item);
PUBLIC FIC_enum_retour TraverseIdReceptionFichier(
                                          TGereFic          *GereFic,
                                          TTraiterIdReception Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
