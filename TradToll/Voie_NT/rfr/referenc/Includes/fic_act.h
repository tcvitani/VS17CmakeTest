/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES ACTIONS
* FICHIER: FIC_ACT.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les actions qui
*         sont encore a faire
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_act.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:40   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:06:54   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jul 18 1997 17:57:46   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 14:59:24   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_ACT_H
#define FIC_ACT_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef FIC_ACT_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterAction)( TGereFic              *GereFic,
                                           FIC_enum_retour       Etat,
                                           long                  Position,
                                           TParam                *Param,
                                           TReceptionAction      *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean InitFichierAction( TGereFic *GereFic, char *Attributs);
PUBLIC boolean FermerFichierAction( TGereFic *GereFic);

PUBLIC FIC_enum_retour LireIdAction( TGereFic          *GereFic,
                                     TIdReception      *Id,
                                     TReceptionAction  **Item);
PUBLIC FIC_enum_retour LirePosAction( TGereFic         *GereFic,
                                      long               Position,
                                      TReceptionAction **Item);
PUBLIC FIC_enum_retour AjouterPosAction( TGereFic              *GereFic,
                                         TReceptionAction      *Item,
                                         long          *Position);
PUBLIC FIC_enum_retour LibererPosAction( TGereFic      *GereFic,
                                         long          Position);
PUBLIC FIC_enum_retour EcrirePosAction( TGereFic               *GereFic,
                                        long                    Position,
                                        TReceptionAction       *Item);
PUBLIC FIC_enum_retour TraverseActionFichier(
                                          TGereFic          *GereFic,
                                          TTraiterAction Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
