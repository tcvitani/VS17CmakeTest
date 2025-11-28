/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES IdContext
* FICHIER: FIC_IDCN.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les IdContext que
*         qui existent pour chque it‚ration et sont utilis‚s par les
*         m‚thodes Verifier et Faire de l'applicatif
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_idcn.h_v  $
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

#ifndef FIC_IDCN_H
#define FIC_IDCN_H

/*--------------- DEFINES: ----------------*/


/*--------------- INCLUDES: ---------------*/
#ifdef FIC_IDCN_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterIdContext)( TGereFic        *GereFic,
                                              FIC_enum_retour Etat,
                                              long            Position,
                                              TParam          *Param,
                                              TVal            *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean InitFichierIdContext( TGereFic         *GereFic,
                                     char*            Attributs);
PUBLIC boolean FermerFichierIdContext( TGereFic       *GereFic);

PUBLIC FIC_enum_retour LireIdIdContext( TGereFic      *GereFic,
                                        TIdReception  Id,
                                        TIdContext    **Item);
PUBLIC FIC_enum_retour LirePosIdContext( TGereFic     *GereFic,
                                         long           Position,
                                         TIdContext    **Item);
PUBLIC FIC_enum_retour AjouterPosIdContext( TGereFic      *GereFic,
                                            TIdContext    *Item,
                                            long          *Position);
PUBLIC FIC_enum_retour LibererPosIdContext( TGereFic      *GereFic,
                                            long          Position);
PUBLIC FIC_enum_retour EcrirePosIdContext( TGereFic      *GereFic,
                                           long           Position,
                                           TIdContext    *Item);
PUBLIC FIC_enum_retour TraverseIdContextFichier(
                                          TGereFic          *GereFic,
                                          TTraiterIdContext Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
