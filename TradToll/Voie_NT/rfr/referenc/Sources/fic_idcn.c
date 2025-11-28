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
* DESCRIPTION: Encapsule les acces aux fonctions FIC_GERE
*
*              Les items sont de taille fixe, et le fichier de taille
*              variable
*              Le fichier d‚marre vide, et grandit chaque fois qu'il
*              n'est pas possible de cr‚er un nouvel item
* --------------------------------------------------------------------
* NOTA:
*      L'item 1 n'est pas disponible, car il sert … linker entre eux
*      les items disponibles.
*      Cependant, pour l'utilisateur, le fichier commence bien … 1.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/fic_idcn.c_v  $
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
#define FIC_IDCN_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "str.h"
#include "err.h"
#include "fic.h"

#include "rc_def.h"

#define TItem  TIdContext
#define TId    TIdReception
#define TVal   TIdContext
#include "fic_gere.h"
#include "rc_glob.h"

#include "fic_idcn.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- EXTERNALS: --------------*/



/*--------------- DEFINES: ----------------*/
#define           MAX_ITEM     ( sizeof( TIdContext))


/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour      LireIdVal( TIdContext  *Item,
                                        long    Position,
                                        TParam *Param,
                                        TId    *Id,
                                        TIdContext   *Val);
PRIVATE int                  ComparerId( TId *Id, TId *St_Id,
                                         TParam *Param);
PRIVATE FIC_enum_retour      TraiterIdVal( TpTGereFic        GereFic,
                                           FIC_enum_retour   etat,
                                           TIdContext             *Item,
                                           long              Position,
                                           TParam            *Param,
                                           TId               *Id,
                                           TIdContext              *Val);
PRIVATE FIC_enum_retour      AjouterIdContext( TGereFic      *GereFic,
                                               TIdContext    *Item,
                                               long          *Position);


/*--------------- VARIABLES: --------------*/
static TIdContext      PrivateItem;
static TId             PrivateId;
static TIdContext      PrivateVal;

typedef struct {
                  TTraiterIdContext    Fonction;
                  void                 *Param;
               } TTraiterVal, *TpTTraiterVal;

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour InitFichierIdContext( TGereFic *GereFic,
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
PUBLIC boolean InitFichierIdContext( TGereFic *GereFic, char *Attributs)
{
	char fichier[MAX_PATH];

	FIC_makepath (fichier, RFR.disque_virtuel, RFR.path_reference, "id_contx", "dat");

   if( !Fichier_Charger( GereFic, fichier, Attributs, MAX_ITEM, NULL, NULL))
   {
      return( FALSE);
   }
   if( !Fichier_Configurer( GereFic, FIC_NON_TRIE, 0,
                            LireIdVal, NULL, ComparerId,
                            &PrivateItem, &PrivateId, &PrivateVal))
   {
      return( FALSE);
   }

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour FermerFichierIdContext( TGereFic *GereFic)
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

PUBLIC boolean FermerFichierIdContext( TGereFic *GereFic)
{
   return( Fichier_Fermer( GereFic));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LireIdIdContext(TId *Id, TIdContext **Val)
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
PUBLIC FIC_enum_retour LireIdIdContext( TGereFic      *GereFic,
                                        TIdReception  Id,
                                        TIdContext    **Item)
{
   FIC_enum_retour      retour;
   long                 position;

   retour = Fichier_LireId( GereFic, &Id, NULL, &position, Item, NULL);

   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour AjouterPosIdContext
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ecriture de l'item dans le fichier, a un emplacement libre
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour AjouterPosIdContext( TGereFic      *GereFic,
                                            TIdContext    *Item,
                                            long          *Position)
{
   FIC_enum_retour      retour;
   TIdContext           *item_lu;
   TIdContext           item_libre;

   /* on enregistre que l'item n'est plus un item libre */
   Item->IndiceIdContext = ITEM_OCCUPE;

   retour = LirePosIdContext( GereFic, INDICE_ITEM_LIBRE, &item_lu);
   if( retour != FIC_OK)
   {
      /* le fichier est vide, l'item_libre n'existe pas, il faut le cr‚er */
      item_libre.IdReception = NO_ID_RECEPTION;
      item_libre.Id[ 0] = '\0';
      item_libre.Iteration   = 0;
      item_libre.AppelRaison = VERIFY;
      item_libre.NouveauFichier[ 0] = '\0';
      item_libre.AncienFichier[ 0] = '\0';
      item_libre.BufferParam[ 0] = '\0';
      item_libre.IndiceIdContext = INDICE_ITEM_LIBRE;

      retour = AjouterIdContext( GereFic, &item_libre, NULL);
      if( retour != FIC_OK)
      {
         /* pb inattendu */
         ERR_ErreurFatale();
      }
   }
   else
   {
      item_libre = *item_lu;
   }

   if( item_libre.IndiceIdContext == INDICE_ITEM_LIBRE)
   {
      /* il n'y a pas d'item disponible, il faut donc en cr‚er un nouveau */
      retour = AjouterIdContext( GereFic, Item, Position);
   }
   else
   {
      *Position = item_libre.IndiceIdContext;

      /* il y a un item libre; il faut reformer la chaine, en retirant
       * l'item
       */
      retour = LirePosIdContext( GereFic, *Position, &item_lu);
      if( retour != FIC_OK)
      {
         /* pb inattendu */
         ERR_ErreurFatale();
      }

      /* ecrire le nouvel item_libre */
      item_libre.IndiceIdContext = item_lu->IndiceIdContext;
      retour = EcrirePosIdContext( GereFic, INDICE_ITEM_LIBRE, &item_libre);

      /* ecrire l'item nouveau */
      retour = EcrirePosIdContext( GereFic, *Position, Item);
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LibererPosIdContext
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Liberation d'un item, qui est ajout‚ dans la liste des items
*       vides
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour LibererPosIdContext( TGereFic      *GereFic,
                                            long          Position)
{
   FIC_enum_retour      retour;
   TIdContext           *item_lu;
   TIdContext           item_libre;

   /* lire la racine du chainage */
   retour = LirePosIdContext( GereFic, INDICE_ITEM_LIBRE, &item_lu);
   if( retour == FIC_NOK)
   {
      /* le fichier est vide, l'item_libre n'existe pas */
      ERR_ErreurFatale();
   }
   item_libre = *item_lu;

   /*----------------------------*/
   /* on lit l'item … liberer */
   retour = LirePosIdContext( GereFic, Position, &item_lu);
   if( retour != FIC_OK)
      return( retour);

   /*----------------------------*/
   /* on verifie que l'item … liberer ‚tait bien occup‚ */
   if( item_lu->IndiceIdContext != ITEM_OCCUPE)
   {
      /* on essaie de liberer un item qui n'‚tait pas occup‚
       * DONC on ne fait rien
       */
       return( FIC_OK);
   }

   /* on remplace l'item lib‚r‚ par le contenu de l'item_racine ,
    * ce qui maintient le chainage
    */
   retour = EcrirePosIdContext( GereFic, Position, &item_libre);
   if( retour == FIC_NOK)
   {
      ERR_ErreurFatale();
   }

   /* on re‚crit l'item racine */
   item_libre.IndiceIdContext = Position;
   retour = EcrirePosIdContext( GereFic, INDICE_ITEM_LIBRE, &item_libre);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EcrirePosIdContext
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: R emplacement de l'item … une certaine position
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour EcrirePosIdContext( TGereFic *GereFic,
                                           long      Position,
                                           TIdContext    *Item)
{
   FIC_enum_retour      retour;

   Position += DECALAGE_ITEM_LIBRE;
   retour = Fichier_RemplacerPos( GereFic, NULL, Position, Item);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LirePosIdContext
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
PUBLIC FIC_enum_retour LirePosIdContext( TGereFic *GereFic,
                                         long      Position,
                                         TIdContext    **Item)
{
   Position += DECALAGE_ITEM_LIBRE;
   return( Fichier_LirePos( GereFic, Position, NULL, Item, NULL, NULL));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour TraverseIdContextFichier( TGereFic          *GereFic,
*                                         TTraiterIdContext Fonction,
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
PUBLIC FIC_enum_retour TraverseIdContextFichier(
                                          TGereFic          *GereFic,
                                          TTraiterIdContext Fonction,
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
* SYNTAXE: FIC_enum_retour LireIdVal( TIdContext *Item, int Position, TId *Id, TIdContext *Val)
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
PRIVATE FIC_enum_retour LireIdVal( TIdContext  *Item,
                                   long   Position,
                                   TParam *Param,
                                   TId    *Id,
                                   TIdContext   *Val)
{
   NO_WARNING( Param);
   NO_WARNING( Position);

   /* a cause de l'item libre */
   Item->Position = Position-1;

   /* est-ce qu'on teste un item libre ? */
   if( Item->IndiceIdContext == INDICE_ITEM_LIBRE)
      Item->IdReception = NO_ID_RECEPTION;

   /* valoriser les paramŠtres en entr‚e */
   if( Id != NULL)
   {
      *Id = Item->IdReception;
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

   if( *Id < *St_Id)
      cmp = -1;
   else if( *Id == *St_Id)
      cmp = 0;
   else
      cmp = 1;

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
                                      TIdContext        *Item,
                                      long              Position,
                                      TParam            *Param,
                                      TId               *Id,
                                      TIdContext        *Val)
{
   TpTTraiterVal    traiterVal;

   NO_WARNING( Item);
   NO_WARNING( Position);
   NO_WARNING( Id);

   traiterVal = ( TpTTraiterVal) Param;
   return( traiterVal->Fonction( GereFic, Etat, Position,
                                 traiterVal->Param, Val));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour AjouterIdContext
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ecriture de l'item en fin de fichier
*       Cette fonction est utilis‚ uniquement en interne, quand
*       le chainage des items libres est vide
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour AjouterIdContext( TGereFic      *GereFic,
                                          TIdContext    *Item,
                                          long          *Position)
{
   FIC_enum_retour      retour;

   retour = Fichier_AjouterPosition( GereFic, FIC_END, Item, NULL, Position);
   if( Position != NULL)
      *Position -= DECALAGE_ITEM_LIBRE;
   return( retour);
}

