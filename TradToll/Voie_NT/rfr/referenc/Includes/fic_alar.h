/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES ALARMES
* FICHIER: FIC_ALAR.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les alarmes
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_alar.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:06:54   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jul 18 1997 17:57:46   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 14:59:26   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_ALAR_H
#define FIC_ALAR_H

/*--------------- DEFINES: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef FIC_ALAR_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterAlarme)( TGereFic         *GereFic,
                                           FIC_enum_retour  Etat,
                                           long             Position,
                                           TParam           *Param,
                                           TReceptionAlarme *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean InitFichierAlarme( TGereFic *GereFic, char *Attributs);
PUBLIC boolean FermerFichierAlarme( TGereFic *GereFic);

PUBLIC FIC_enum_retour LireIdAlarme( TGereFic          *GereFic,
                                     TDateHeure        *DateHeure,
                                     TAlarmeExterne    *AlarmeExterne,
                                     TReceptionAlarme  **Item);
PUBLIC FIC_enum_retour LirePosAlarme( TGereFic *GereFic,
                                      long     Position,
                                      TReceptionAlarme    **Item);
PUBLIC FIC_enum_retour AjouterPosAlarme( TGereFic *GereFic,
                                         TReceptionAlarme    *Item,
                                         long          *Position);
PUBLIC FIC_enum_retour LibererPosAlarme( TGereFic      *GereFic,
                                         long          Position);
PUBLIC FIC_enum_retour EcrirePosAlarme( TGereFic *GereFic,
                                        long     Position,
                                        TReceptionAlarme    *Item);
PUBLIC FIC_enum_retour TraverseAlarmeFichier(
                                          TGereFic          *GereFic,
                                          TTraiterAlarme Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
