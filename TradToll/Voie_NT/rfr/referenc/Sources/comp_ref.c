/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base comparant 2 fichiers de ref‚rence
* FICHIER: COMP_REF.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
*         - Verifie la correctitude d'un fichier de r‚f‚rence.
*         - Compare 2 fichiers de r‚f‚rence.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/comp_ref.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:48   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:22   DPI
 * Version 4.00
 * 
 *    Rev 1.2   Jul 29 1997 15:30:08   DPI
 * Correction bug sur comparaison des fichiers
 * de reference
 * 
 *    Rev 1.1   Jun 04 1997 19:13:04   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:08   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#define COMP_REF_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "rc_def.h"
#include "fic_gere.h"

#include "fic_ref.h"
#include "comp_ref.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- TYPEDEF: ---------------*/

typedef struct
        {
           TpTGereFic            GereFic2;
           TTraiterReference     TraiterItemIdentique;
           TTraiterReference     TraiterItemDifferent;
           void                  *Param;
           boolean               ParReference;
        } TComparerItemsParam;

/*--------------- FUNCTIONS: ---------------*/
PRIVATE FIC_enum_retour ComparerItems( TGereFic          *GereFic1,
                                       FIC_enum_retour   Etat,
                                       long              Position,
                                       TParam            *Param,
                                       TFichier          *Val);

PRIVATE FIC_enum_retour EstCorrectItem( TGereFic          *GereFic1,
                                        FIC_enum_retour   Etat,
                                        long              Position,
                                        TParam            *Param,
                                        TFichier          *Val);


/*--------------- FUNCTIONS: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EstCorrecteReference
* PARAMETRES: - le fichier … controler
* RETOUR:     la correctitude du fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Publique
* ROLE: Parcours le fichier en v‚rifiant que le fichier est lisible,
*       et que chaque item est bien form‚.
*
*       L'information est stock‚ dans la variable ok, qui est pass‚
*       en paramŠtre d‚cast‚ du Traverse.
*       La variable de retour du Traverse n'est pas exploit‚e.
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean EstCorrecteReference( TpTGereFic   GereFic1)
{
   FIC_enum_retour      retour;
   boolean              ok;

   ok = TRUE;

   retour = FIC_EN_COURS;
   while( retour == FIC_EN_COURS)
   {
     retour = TraverseReferenceFichier( GereFic1, EstCorrectItem, &ok);
   }

   return( ok);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ComparerReferences
* PARAMETRES: - le fichier de r‚f‚rence 1
*             - le fichier de r‚f‚rence 2
*             - la fonction … appeler en cas d'item diff‚rent
*             - la fonction … appeler en cas d'item identique
*             - un paramŠtre d‚cast‚
* RETOUR:     la correctitude du fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Publique
* ROLE: Parcours le fichier 1, et pour chaque item du fichier
*       nouveau, compare avec le fichier 2
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void ComparerReferences( TpTGereFic        GereFic1,
                                TpTGereFic        GereFic2,
                                TTraiterReference TraiterItemDifferent,
                                TTraiterReference TraiterItemIdentique,
                                boolean           ParReference,
                                void              *Param_in)
{
   FIC_enum_retour      retour;
   TComparerItemsParam  param;

   param.GereFic2 = GereFic2;
   param.TraiterItemDifferent = TraiterItemDifferent;
   param.TraiterItemIdentique = TraiterItemIdentique;
   param.Param = Param_in;
   param.ParReference = ParReference;

   retour = FIC_EN_COURS;
   while( retour == FIC_EN_COURS)
   {
     retour = TraverseReferenceFichier( GereFic1, ComparerItems, &param);
   }
}





/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EstCorrectItem
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: Pour un item du fichier, m‚morise dans le paramŠtre d‚cast‚
*       si l'item ‚tait incorrect
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour EstCorrectItem( TGereFic          *GereFic1,
                                        FIC_enum_retour   Etat,
                                        long              Position,
                                        TParam            *Param,
                                        TFichier          *Val)
{
   boolean     *ok;

   GereFic1 = GereFic1;
   Val = Val;
   Position = Position;
   if( ( Etat == FIC_ITEM_NOK) || ( Etat == FIC_NOK)
                               || ( Etat == FIC_ECHEC) )
   {
      ok = ( boolean *) Param;
      *ok = FALSE;
   }

   return( Etat);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ComparerItems
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: Pour un item du fichier, v‚rifie si l'item existait dans le
*       fichier 2.
*       L'item existait si l'Id existait avec la mˆme r‚f‚rence.
*
*       Si l'item dans le fichier 1 est diff‚rent, il d‚clenche
*       l'appel d'une fonction.
*       Si l'item dans le fichier 1 est identique, il d‚clenche
*       l'appel d'une autre fonction.
*
* REMARQUE : L'appel des deux fonctions TraiterItemXxxx se fait
*            avec un etat:
*            - … FIC_OK si l'autre ID/Iteration existait
*            - … FIC_NOK si l'autre ID/Iteration n'existait pas.
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour ComparerItems( TGereFic          *GereFic1,
                                       FIC_enum_retour   Etat,
                                       long              Position,
                                       TParam            *Param,
                                       TFichier          *Val_1)
{
   TComparerItemsParam  *compare;
   FIC_enum_retour      retour;
   TFichier             *val_2;

   compare = ( TComparerItemsParam*) Param;

   /*------------------------------------------*/
   if( Etat != FIC_OK)
   {
      return( Etat);
   }

   /*------------------------------------------*/
   if( compare->ParReference)
      retour = LireIdReference( compare->GereFic2, Val_1->Id, 0,
                                Val_1->Reference, NULL, &val_2);
   else
      retour = LireIdReference( compare->GereFic2, Val_1->Id,
                                Val_1->Iteration,
                                NULL, NULL, &val_2);

   if( retour == FIC_OK)
   {
      if( compare->TraiterItemIdentique != NULL)
          compare->TraiterItemIdentique( GereFic1, Etat, Position,
                                         compare->Param, Val_1);
   }
   else
   {
      if( compare->TraiterItemDifferent != NULL)
          compare->TraiterItemDifferent( GereFic1, retour,
                                         Position, compare->Param, Val_1);
   }

   return( Etat);
}


