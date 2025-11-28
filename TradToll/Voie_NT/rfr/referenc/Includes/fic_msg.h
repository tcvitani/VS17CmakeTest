/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES MESSAGES
* FICHIER: FIC_MSG.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les messages attendues
*         d'autres taches
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_msg.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:06:56   DPI
 * Version 4.00
 * 
 *    Rev 1.2   29 Oct 1997 19:14:14   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.1   Jul 18 1997 17:57:46   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 14:59:28   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_MSG_H
#define FIC_MSG_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef FIC_MSG_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterMessage)( TGereFic        *GereFic,
                                              FIC_enum_retour Etat,
                                              long            Position,
                                              TParam          *Param,
                                              TVal            *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean InitFichierMessage( TGereFic *GereFic, char* Attributs);
PUBLIC boolean FermerFichierMessage( TGereFic *GereFic);

PUBLIC FIC_enum_retour LireIdMessage( TGereFic             *GereFic,
                                      noyau_bal_id         Bal,
                                      TTacheMessage        Message,
                                      TReceptionMessage    **Item);
PUBLIC FIC_enum_retour LirePosMessage( TGereFic             *GereFic,
                                       long                 Position,
                                       TReceptionMessage    **Item);
PUBLIC FIC_enum_retour AjouterPosMessage( TGereFic          *GereFic,
                                          TReceptionMessage *Item,
                                          long              *Position);
PUBLIC FIC_enum_retour LibererPosMessage( TGereFic      *GereFic,
                                          long          Position);
PUBLIC FIC_enum_retour EcrirePosMessage( TGereFic          *GereFic,
                                         long               Position,
                                         TReceptionMessage  *Item);
PUBLIC FIC_enum_retour TraverseMessageFichier(
                                          TGereFic          *GereFic,
                                          TTraiterMessage Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
