/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DE REFERENCE
* FICHIER: FIC_REF.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ficher d'entete du fichier de REFERENCE VM
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Includes/fic_ref.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:42   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   03 Nov 1997 16:06:56   DPI
 * Version 4.00
 * 
 *    Rev 1.0   Apr 24 1997 14:59:28   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_REF_H
#define FIC_REF_H

/*--------------- DEFINES: ----------------*/

#define           FORMAT_REF_1   " R["
#define           FORMAT_REF_2   "]"
#define           FORMAT_HOST    " @"

#define           FORMAT_1       " %[_0-9a-zA-Z-] [ %d ]"
#define           SEPARATEUR_1   '='
#define           FORMAT_2       " %s" FORMAT_REF_1 " %[^] " FORMAT_REF_2
#define           SEPARATEUR_2   ']'
#define           FORMAT_3       FORMAT_HOST "%s"

#define           FORMAT_ECRIT_1A  "%s = "
#define           FORMAT_ECRIT_1B  "%s[%d] = "

#define           COMMENTS       '#'

/*--------------- INCLUDE 2: ----------------*/

/*--------------- INCLUDES: ---------------*/
#ifdef FIC_REF_DEF
#include <public.h>
#else
#include <export.h>
#endif


/*--------------- TYPEDEF ---------------*/

typedef FIC_enum_retour (*TTraiterReference)( TGereFic        *GereFic,
                                              FIC_enum_retour Etat,
                                              long            Position,
                                              TParam          *Param,
                                              TFichier        *Val);

/*--------------- FUNCTIONS: --------------*/

PUBLIC boolean EchangerFichierReference( TGereFic *GereFic,
                                         char     *NomFichier,
                                         char     *Attributs);
PUBLIC boolean InitFichierReference( TGereFic *GereFic,
                                     char     *NomFichier,
                                     char     *Attributs,
                                     TFichier *Id,
                                     TFichier *Val);
PUBLIC boolean FermerFichierReference( TGereFic *GereFic);

PUBLIC FIC_enum_retour LireIdReference( TGereFic *GereFic,
                                        char     *Id,
                                        int      Iteration,
                                        char     *Reference,
                                        char     *Fichier,
                                        TFichier **Val);
PUBLIC FIC_enum_retour LirePosReference( TGereFic *GereFic,
                                         int      Position,
                                         TFichier **Val);
PUBLIC FIC_enum_retour EcrirePosReference( TGereFic *GereFic,
                                           char     *Id,
                                           int      Iteration,
                                           char     *Fichier,
                                           char     *Reference,
                                           char     *Hostname);
PUBLIC FIC_enum_retour TraverseReferenceFichier(
                                          TGereFic          *GereFic,
                                          TTraiterReference Fonction,
                                          void              *Param);


#undef PUBLIC
#undef I
#undef INIT
#endif
