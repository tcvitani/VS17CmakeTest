/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DES ACTIONS
* FICHIER: FIC_ALAR.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion du fichier dans lequel sont stock‚s les alarmes
* --------------------------------------------------------------------
* DESCRIPTION: Encapsule les acces aux fonctions FIC_GERE
*
*              Les items sont de taille fixe, et le fichier de taille
*              variable
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/fic_alar.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:48   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:22   DPI
 * Version 4.00
 * 
 *    Rev 1.2   Jul 18 1997 17:57:54   DPI
 *  
 * 
 *    Rev 1.1   Jun 04 1997 19:13:06   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:10   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#define FIC_ALAR_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "str.h"
#include "err.h"
#include "fic.h"

#include "rc_def.h"

#define TItem  TReceptionAlarme
#define TId    TReceptionAlarme
#define TVal   TReceptionAlarme

#include "fic_gere.h"
#include "rc_glob.h"

#include "fic_alar.h"


/*--------------- INCLUDE 1: ----------------*/


/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- EXTERNALS: --------------*/



/*--------------- DEFINES: ----------------*/
#define           MAX_ITEM     ( sizeof( TReceptionAlarme))


/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour      LireIdVal( TItem  *Item,
                                        long    Position,
                                        TParam *Param,
                                        TId    *Id,
                                        TVal   *Val);
PRIVATE int                  ComparerId( TId *Id, TId *St_Id,
                                         TParam *Param);
PRIVATE FIC_enum_retour AjouterAlarme( TGereFic      *GereFic,
                                       TItem         *Item,
                                       long          *Position);
PRIVATE FIC_enum_retour      TraiterIdVal( TpTGereFic        GereFic,
                                           FIC_enum_retour   etat,
                                           TItem             *Item,
                                           long              Position,
                                           TParam            *Param,
                                           TId               *Id,
                                           TVal              *Val);


/*--------------- VARIABLES: --------------*/
static TItem      PrivateItem;
static TId        PrivateId;
static TVal       PrivateVal;

typedef struct {
                  TTraiterAlarme    Fonction;
                  void                 *Param;
               } TTraiterVal, *TpTTraiterVal;

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour InitFichierAlarme( TGereFic *GereFic,
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
PUBLIC boolean InitFichierAlarme( TGereFic *GereFic, char *Attributs)
{
	char fichier[MAX_PATH];

	FIC_makepath (fichier, RFR.disque_virtuel, RFR.path_reference, "alar", "dat");

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
* SYNTAXE: PUBLIC FIC_enum_retour FermerFichierAlarme( TGereFic *GereFic)
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

PUBLIC boolean FermerFichierAlarme( TGereFic *GereFic)
{
   return( Fichier_Fermer( GereFic));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LireIdAlarme(TId *Id, TVal **Val)
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
PUBLIC FIC_enum_retour LireIdAlarme( TGereFic          *GereFic,
                                     TDateHeure        *DateHeure,
                                     TAlarmeExterne    *AlarmeExterne,
                                     TItem             **Item)
{
   FIC_enum_retour      retour;
   long                 position;
   TId                  id;

   if( DateHeure == NULL)
   {
      id.DateHeure.Date.annee = 0;
      id.DateHeure.Date.mois  = 0;
      id.DateHeure.Date.jour  = 0;

      id.DateHeure.Heure.heure = 0;
      id.DateHeure.Heure.minute = 0;
      id.DateHeure.Heure.seconde = 0;
   }
   else
      id.DateHeure     = *DateHeure;

   if( AlarmeExterne == NULL)
      id.AlarmeExterne = NO_ALARME_EXTERNE;
   else
      id.AlarmeExterne = *AlarmeExterne;

   retour = Fichier_LireId( GereFic, &id, NULL, &position, Item, NULL);

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour AjouterPosAlarme
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ecriture de l'item a un emplacement vide, ou … la fin du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour AjouterPosAlarme( TGereFic *GereFic,
                                         TItem    *Item,
                                         long     *Position)
{
   FIC_enum_retour      retour;
   TItem                *item_lu;
   TItem                item_libre;

   /* on enregistre que l'item n'est plus un item libre */
   Item->IndiceAlarme = ITEM_OCCUPE;

   retour = LirePosAlarme( GereFic, INDICE_ITEM_LIBRE, &item_lu);
   if( retour != FIC_OK)
   {
      /* le fichier est vide, l'item_libre n'existe pas, il faut le cr‚er */
      item_libre.Etat              = NO_ALARME;
      item_libre.PremiereAction    = NO_ACTION;
      item_libre.DerniereAction    = NO_ACTION;
      item_libre.IndiceAlarme      = INDICE_ITEM_LIBRE;


      retour = AjouterAlarme( GereFic, &item_libre, NULL);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      item_libre = *item_lu;
   }

   if( item_libre.IndiceAlarme == INDICE_ITEM_LIBRE)
   {
      /* il n'y a pas d'item disponible, il faut donc en cr‚er un nouveau */
      retour = AjouterAlarme( GereFic, Item, Position);
   }
   else
   {
      *Position = item_libre.IndiceAlarme;

      /* il y a un item libre; il faut reformer la chaine, en retirant
       * l'item
       */
      retour = LirePosAlarme( GereFic, *Position, &item_lu);
      if( retour != FIC_OK)
      {
         /* pb inattendu */
         ERR_ErreurFatale();
      }

      /* ecrire le nouvel item_libre */
      item_libre.IndiceAlarme = item_lu->IndiceAlarme;
      retour = EcrirePosAlarme( GereFic, INDICE_ITEM_LIBRE, &item_libre);

      /* ecrire l'item nouveau */
      retour = EcrirePosAlarme( GereFic, *Position, Item);
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LibererPosAlarme
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
PUBLIC FIC_enum_retour LibererPosAlarme( TGereFic      *GereFic,
                                         long          Position)
{
   FIC_enum_retour      retour;
   TItem                *item_lu;
   TItem                item_libre;

   /*----------------------------*/
   /* lire la racine du chainage */
   retour = LirePosAlarme( GereFic, INDICE_ITEM_LIBRE, &item_lu);
   if( retour == FIC_NOK)
   {
      /* le fichier est vide, l'item_libre n'existe pas */
      ERR_ErreurFatale();
   }
   item_libre = *item_lu;

   /*----------------------------*/
   /* on lit l'item … liberer */
   retour = LirePosAlarme( GereFic, Position, &item_lu);
   if( retour != FIC_OK)
      return( retour);

   /*----------------------------*/
   /* on verifie que l'item … liberer ‚tait bien occup‚ */
   if( item_lu->IndiceAlarme != ITEM_OCCUPE)
   {
      /* on essaie de liberer un item qui n'‚tait pas occup‚
       * DONC on ne fait rien
       */
       return( FIC_OK);
   }

   /*----------------------------*/
   /* on remplace l'item lib‚r‚ par le contenu de l'item_racine ,
    * ce qui maintient le chainage
    */
   retour = EcrirePosAlarme( GereFic, Position, &item_libre);
   if( retour == FIC_NOK)
   {
      ERR_ErreurFatale();
   }

   /* on re‚crit l'item racine */
   item_libre.IndiceAlarme = Position;
   retour = EcrirePosAlarme( GereFic, INDICE_ITEM_LIBRE, &item_libre);

   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EcrireIdAlarme
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
PUBLIC FIC_enum_retour EcrirePosAlarme( TGereFic *GereFic,
                                        long     Position,
                                        TItem    *Item)
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
* SYNTAXE: PUBLIC FIC_enum_retour LirePosAlarme
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
PUBLIC FIC_enum_retour LirePosAlarme( TGereFic *GereFic,
                                      long     Position,
                                      TItem    **Item)
{
   Position += DECALAGE_ITEM_LIBRE;
   return( Fichier_LirePos( GereFic, Position, NULL, Item, NULL, NULL));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour TraverseAlarmeFichier( TGereFic          *GereFic,
*                                         TTraiterAlarme Fonction,
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
PUBLIC FIC_enum_retour TraverseAlarmeFichier(
                                          TGereFic          *GereFic,
                                          TTraiterAlarme    Fonction,
                                          void              *Param)
{
   TTraiterVal          traiterVal;

   traiterVal.Param = Param;
   traiterVal.Fonction = Fonction;


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
* SYNTAXE: FIC_enum_retour LireIdVal( TItem *Item, int Position, TId *Id, TVal *Val)
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
PRIVATE FIC_enum_retour LireIdVal( TItem  *Item,
                                   long   Position,
                                   TParam *Param,
                                   TId    *Id,
                                   TVal   *Val)
{
   NO_WARNING( Param);
   NO_WARNING( Position);

   /* a cause de l'item libre */
   Item->Position = Position-1;
   /* valoriser les paramŠtres en entr‚e */
   if( Id != NULL)
   {
      *Id = *Item;
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
   int            cmp;

   NO_WARNING( Param);
   NO_WARNING( Id);
   NO_WARNING( St_Id);

   /* est-ce qu'on teste un item libre ? */
   if( St_Id->IndiceAlarme == INDICE_ITEM_LIBRE)
     return( -1);

   if( Id->AlarmeExterne != NO_ALARME_EXTERNE)
   {
      if( Id->AlarmeExterne < St_Id->AlarmeExterne)
         cmp = -1;
      else if( Id->AlarmeExterne == St_Id->AlarmeExterne)
         cmp = 0;
      else
         cmp = 1;
 }
   else
   {
      #ifndef HRD_BOUCHON
      cmp = HRDCompareDatesHeures( Id->DateHeure.Date,
                                   Id->DateHeure.Heure,
                                   St_Id->DateHeure.Date,
                                   St_Id->DateHeure.Heure);
      #else
      cmp = 0;
      #endif
   }


   return( cmp);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour AjouterAlarme
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
PRIVATE FIC_enum_retour AjouterAlarme( TGereFic      *GereFic,
                                       TItem         *Item,
                                       long          *Position)
{
   FIC_enum_retour      retour;

   retour = Fichier_AjouterPosition( GereFic, FIC_END, Item, NULL, Position);
   if( Position != NULL)
      *Position -= DECALAGE_ITEM_LIBRE;

   return( retour);
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
                                      TItem             *Item,
                                      long              Position,
                                      TParam            *Param,
                                      TId               *Id,
                                      TVal              *Val)
{
   TpTTraiterVal    traiterVal;

   NO_WARNING( Item);
   NO_WARNING( Position);
   NO_WARNING( Id);

   traiterVal = ( TpTTraiterVal) Param;
   return( traiterVal->Fonction( GereFic, Etat, Position,
                                 traiterVal->Param, Val));
}
