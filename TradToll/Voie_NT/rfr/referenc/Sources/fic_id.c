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
* DESCRIPTION: Encapsule les acces aux fonctions FIC_GERE
*
*              Les items sont de taille fixe, et le fichier de taille
*              variable
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/fic_id.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:48   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   03 Nov 1997 16:07:24   DPI
 * Version 4.00
 * 
 *    Rev 1.1   Jun 04 1997 19:13:08   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:12   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#define FIC_ID_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "str.h"
#include "err.h"
#include <fic.h>

#include "rc_def.h"

#define TItem  TReception
#define TId    char
#define TVal   TReception
#include "fic_gere.h"
#include "rc_glob.h"

#include "fic_id.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- EXTERNALS: --------------*/



/*--------------- DEFINES: ----------------*/
#define           MAX_ITEM     ( sizeof( TReception))


/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour      LireIdVal( TReception  *Item,
                                        long    Position,
                                        TParam *Param,
                                        TId    *Id,
                                        TReception   *Val);
PRIVATE int                  ComparerId( TId *Id, TId *St_Id,
                                         TParam *Param);
PRIVATE FIC_enum_retour      TraiterIdVal( TpTGereFic        GereFic,
                                           FIC_enum_retour   etat,
                                           TReception             *Item,
                                           long              Position,
                                           TParam            *Param,
                                           TId               *Id,
                                           TReception              *Val);


/*--------------- VARIABLES: --------------*/
static TReception      PrivateItem;
static TId             PrivateId[ RFR_MAX_ID];
static TReception      PrivateVal;

typedef struct {
                  TTraiterIdReception    Fonction;
                  void                 *Param;
               } TTraiterVal, *TpTTraiterVal;

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour InitFichierIdReception( TGereFic *GereFic,
*                                                       char *NomFichier)
* RETOUR: TRUE  : op‚ration r‚ussie
*         FALSE : op‚ration ‚chou‚e
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture et configuration d'un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean InitFichierIdReception( TGereFic *GereFic, char *Attributs)
{
	char fichier[MAX_PATH];

	FIC_makepath (fichier, RFR.disque_virtuel, RFR.path_reference, "id", "dat");

   if( !Fichier_Charger( GereFic, fichier, Attributs, MAX_ITEM, NULL, NULL))
   {
      return( FALSE);
   }
   if( !Fichier_Configurer( GereFic, FIC_NON_TRIE, 0,
                            LireIdVal, NULL, ComparerId,
                            &PrivateItem, PrivateId, &PrivateVal))
   {
      return( FALSE);
   }

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour FermerFichierIdReception( TGereFic *GereFic)
* RETOUR: TRUE  : op‚ration r‚ussie
*         FALSE : op‚ration ‚chou‚e
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Fermeture du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC boolean FermerFichierIdReception( TGereFic *GereFic)
{
   return( Fichier_Fermer( GereFic));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LireIdIdReception(TId *Id, TReception **Val)
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associ‚e a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour LireIdIdReception( TGereFic *GereFic,
                                          char     *Id,
                                          TReception    **Item)
{
   FIC_enum_retour      retour;

   retour = Fichier_LireId( GereFic, Id, NULL, NULL, Item, NULL);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour AjouterIdIdReception
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ecriture de l'item en fin de fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour AjouterPosIdReception( TGereFic *GereFic,
                                             TReception    *Item)
{
   FIC_enum_retour      retour;

   retour = Fichier_AjouterPos( GereFic, FIC_END, Item, NULL);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EcrireIdIdReception
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Remplacement de l'item … une certaine position
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour EcrirePosIdReception( TGereFic *GereFic,
                                             long      Position,
                                             TReception    *Item)
{
   FIC_enum_retour      retour;

   retour = Fichier_RemplacerPos( GereFic, NULL, Position, Item);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LirePosIdReception
* RETOUR: FIC_OK       : Id trouv‚
*         FIC_NOK      : Id non trouv‚
*         FIC_FINI     : Fichier termin‚
*         FIC_ITEM_NOK : Pb durant l'analyse de l'id/val
*         FIC_ECHEC    : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associ‚e a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour LirePosIdReception( TGereFic *GereFic,
                                           long      Position,
                                           TReception    **Item)
{
   return( Fichier_LirePos( GereFic, Position, NULL, Item, NULL, NULL));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour TraverseIdReceptionFichier( TGereFic          *GereFic,
*                                         TTraiterIdReception Fonction,
*                                         void              *Param)
* RETOUR: FIC_OK       : Id trouv‚
*         FIC_NOK      : Id non trouv‚
*         FIC_FINI     : Fichier termin‚
*         FIC_ITEM_NOK : Pb durant l'analyse de l'id/val
*         FIC_ECHEC    : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associ‚e a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour TraverseIdReceptionFichier(
                                          TGereFic          *GereFic,
                                          TTraiterIdReception Fonction,
                                          void              *Param)
{
   TTraiterVal          traiterVal;

   traiterVal.Fonction = Fonction;
   traiterVal.Param = Param;

   return( Fichier_Traverse( GereFic, TraiterIdVal, &traiterVal));
}





/*=====================================================================/*
/*=====================================================================/*
/*=====================================================================/*
/*=====================================================================/*






/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour LireIdVal( TReception *Item, int Position, TId *Id, TReception *Val)
* PARAMETRES: pointeur sur l'Item
*             position de l'Item dans le fichier
*             pointeur sur la zone de retour
* RETOUR: resultat de la fonction
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Sert pour la lecture de l'‚l‚ment Id dans l'‚l‚ment Item
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireIdVal( TReception  *Item,
                                   long   Position,
                                   TParam *Param,
                                   TId    *Id,
                                   TReception   *Val)
{
   NO_WARNING( Param);
   NO_WARNING( Position);

   Item->IdReception = Position;

   /* valoriser les paramŠtres en entr‚e */
   if( Id != NULL)
   {
      STR_strcpy( RFR_MAX_ID, Id, Item->Id);
   }

   if( Val != NULL)
   {
      *Val = *Item;
   }

   return( FIC_OK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int ComparerId( TId *Id, TId *St_Id)
* PARAMETRES: pointeurs sur les 2 Id … comparer
* RETOUR: comparaison entre les 2 Id
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Si Id < St_Id   => retour < 0
*       Si Id = St_Id   => retour = 0
*       Si Id > St_Id   => retour > 0
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE int ComparerId( TId *Id, TId *St_Id, TParam *Param)
{
   int      cmp;

   NO_WARNING( Param);

   cmp = strcmp( Id, St_Id);

   return( cmp);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int ComparerId( TId *Id, TId *St_Id)
* PARAMETRES: pointeurs sur les 2 Id … comparer
* RETOUR: comparaison entre les 2 Id
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Si Id < St_Id   => retour < 0
*       Si Id = St_Id   => retour = 0
*       Si Id > St_Id   => retour > 0
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour TraiterIdVal( TGereFic          *GereFic,
                                      FIC_enum_retour   Etat,
                                      TReception        *Item,
                                      long              Position,
                                      TParam            *Param,
                                      TId               *Id,
                                      TReception        *Val)
{
   TpTTraiterVal    traiterVal;

   NO_WARNING( Item);
   NO_WARNING( Position);
   NO_WARNING( Id);

   traiterVal = ( TpTTraiterVal) Param;
   return( traiterVal->Fonction( GereFic, Etat, Position,
                                 traiterVal->Param, Val));
}
