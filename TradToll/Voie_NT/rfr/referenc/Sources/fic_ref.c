/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DE REFERENCE
* FICHIER: FIC_REF.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Permet de lire les couples <ID> <fichier> d'un fichier
*         de reference VM
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/fic_ref.c_v  $
 * 
 *    Rev 1.1   Sep 03 2001 13:11:22   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 15:17:48   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:26   DPI
 * Version 4.00
 * 
 *    Rev 1.2   Jun 04 1997 19:13:10   DPI
 *  
 * 
 *    Rev 1.1   Apr 24 1997 16:31:58   DPI
 * Mise en PRIVATE de EcrireItem
 * 
 *    Rev 1.0   Apr 24 1997 15:02:14   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define FIC_REF_DEF

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "str.h"
#include "err.h"

#include "rc_def.h"

#define TItem  char
#define TId    TFichier
#define TVal   TFichier

#include "fic_gere.h"
#include "rc_glob.h"
#include "fic_ref.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: --------------*/

/*--------------- DEFINES: ----------------*/

#define           RFR_MAX_ITEM     ( RFR_MAX_ID+RFR_MAX_FIC+RFR_MAX_FIC+RFR_MAX_HOST)

/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour LireItem (FILE    *Fichier,
                                  TParam  *Param,
                                  TItem   *ItemRetour,
                                  int     *Taille_lue);
PRIVATE FIC_enum_retour EcrireItem (FILE    *Fichier,
                                    TItem   *Item,
                                    TParam  *Param,
                                    int     *Taille_Ecrite);
PRIVATE FIC_enum_retour LireIdVal (TItem     *Item,
                                   long      Position,
                                   TParam    *Param,
                                   TFichier  *Id,
                                   TFichier  *Val);
PRIVATE FIC_enum_retour EcrireIdVal (TItem     *Item,
                                     long      Position,
                                     TParam    *Param,
                                     TFichier  *Id,
                                     TFichier  *Val);
PRIVATE int ComparerId (TFichier  *Id,
                        TFichier  *St_Id,
                        TParam    *Param);
PRIVATE FIC_enum_retour TraiterIdVal (TpTGereFic       GereFic,
                                      FIC_enum_retour  etat,
                                      TItem            *Item,
                                      long             Position,
                                      TParam           *Param,
                                      TFichier         *Id,
                                      TFichier         *Val);

/*--------------- VARIABLES: --------------*/

static char PrivateItem[RFR_MAX_ITEM];

typedef struct {
                  TTraiterReference    Fonction;
                  void                 *Param;
               } TTraiterVal, *TpTTraiterVal;

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour InitFichierReference( TGereFic *GereFic,
*                                                       char *NomFichier)
* RETOUR: TRUE  : operation reussie
*         FALSE : operation echouee
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture et configuration d'un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean InitFichierReference (TGereFic  *GereFic,
                                     char      *NomFichier,
                                     char      *Attributs,
                                     TFichier  *Id,
                                     TFichier  *Val)
{
   char     destination[RFR_MAX_PATH];

   /* caluler la destination */
   STR_strcpy( RFR_MAX_PATH, destination, NomFichier);
   destination[0] = RFR.disque_virtuel[0];

   if( !Fichier_Charger( GereFic, destination, Attributs, 0, LireItem, EcrireItem))
   {
      return( FALSE);
   }
   if( !Fichier_Configurer( GereFic, FIC_NON_TRIE, 10,
                            LireIdVal, EcrireIdVal, ComparerId,
                            PrivateItem, Id, Val))
   {
      return( FALSE);
   }

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EchangerFichierReference( TGereFic *GereFic,
*                                                       char *NomFichier)
* RETOUR: TRUE  : operation reussie
*         FALSE : operation echouee
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture d'un fichier en exploitant la structure precedemment
*       remplie
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean EchangerFichierReference (TGereFic  *GereFic,
                                         char      *NomFichier,
                                         char      *Attributs)
{
   char     destination[RFR_MAX_PATH];

   /* caluler la destination */
   STR_strcpy( RFR_MAX_PATH, destination, NomFichier);
//   destination[0] = DISQUE_VIRTUEL[0];

   if( !Fichier_Echanger( GereFic, destination, Attributs))
   {
      return( FALSE);
   }

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour FermerFichierReference( TGereFic *GereFic)
* RETOUR: TRUE  : operation reussie
*         FALSE : operation echouee
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Fermeture du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean FermerFichierReference (TGereFic *GereFic)
{
   return( Fichier_Fermer( GereFic));
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LireIdReference(TFichier *Id, TFichier **Val)
* RETOUR: FIC_OK    : Id trouve
*         FIC_NOK   : Id non trouve
*         FIC_ECHEC : probleme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associee a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour LireIdReference (TGereFic  *GereFic,
                                        char      *Id,
                                        int       Iteration,
                                        char      *Reference,
                                        char      *Fichier,
                                        TFichier  **Val)
{
   TFichier   id_recherche;

   id_recherche.Iteration = Iteration;
   if ( Id != NULL)
      STR_strcpy( RFR_MAX_ID, id_recherche.Id, Id);
   else
      id_recherche.Id[0] = '\0';

   if ( Reference != NULL)
      STR_strcpy( RFR_MAX_REF, id_recherche.Reference, Reference);
   else
      id_recherche.Reference[0] = '\0';

   if ( Fichier != NULL)
      STR_strcpy( RFR_MAX_FIC, id_recherche.Fichier, Fichier);
   else
      id_recherche.Fichier[0] = '\0';

   return( Fichier_LireId( GereFic, &id_recherche, NULL, NULL, NULL, Val));
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EcrireIdReference
* RETOUR: FIC_OK    : Id trouve
*         FIC_NOK   : Id non trouve
*         FIC_ECHEC : probleme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associee a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour EcrirePosReference (TGereFic  *GereFic,
                                           char      *Id,
                                           int       Iteration,
                                           char      *Fichier,
                                           char      *Reference,
                                           char      *Hostname)
{
   FIC_enum_retour      retour;
   TFichier             val_ecrire;
   TItem                item[RFR_MAX_ITEM];

   STR_strcpy( RFR_MAX_ID, val_ecrire.Id, Id);
   val_ecrire.Iteration = Iteration;
   STR_strcpy( RFR_MAX_FIC, val_ecrire.Fichier, Fichier);
   STR_strcpy( RFR_MAX_REF, val_ecrire.Reference, Reference);
   STR_strcpy( RFR_MAX_HOST, val_ecrire.Hostname, Hostname);

   retour = Fichier_FormatterId( GereFic, NULL, NULL, 0, item, &val_ecrire);
   if( retour == FIC_OK)
   {
      retour = Fichier_AjouterPos( GereFic, FIC_END, item, NULL);
   }

   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LirePosReference( int Position,
*                                 TFichier *Id, int *Iteration, TFichier **Val)
* RETOUR: FIC_OK       : Id trouve
*         FIC_NOK      : Id non trouve
*         FIC_FINI     : Fichier termine
*         FIC_ITEM_NOK : Pb durant l'analyse de l'id/val
*         FIC_ECHEC    : probleme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associee a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour LirePosReference (TGereFic  *GereFic,
                                         int       Position,
                                         TFichier  **Val)
{
   return( Fichier_LirePos( GereFic, Position, NULL, NULL, NULL, Val));
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour TraverseReferenceFichier( TGereFic          *GereFic,
*                                         TTraiterReference Fonction,
*                                         void              *Param)
* RETOUR: FIC_OK       : Id trouve
*         FIC_NOK      : Id non trouve
*         FIC_FINI     : Fichier termine
*         FIC_ITEM_NOK : Pb durant l'analyse de l'id/val
*         FIC_ECHEC    : probleme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associee a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour TraverseReferenceFichier (TGereFic           *GereFic,
                                                 TTraiterReference  Fonction,
                                                 void               *Param)
{
   TTraiterVal          traiterVal;

   traiterVal.Fonction = Fonction;
   traiterVal.Param = Param;

   return( Fichier_Traverse( GereFic, TraiterIdVal, &traiterVal));    
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour LireItem( FILE *Fichier,
*                                    TParam *Param,
*                                    TItem *ItemRetour,
*                                    int *Taille_lue)
* PARAMETRES: pointeur sur le fichier
*             pointeur sur la zone de retour
*             pointeur sur la taille lue
* RETOUR: resultat de la fonction
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Sert pour la lecture d'un element Item sur un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireItem (FILE    *Fichier,
                                  TParam  *Param,
                                  TItem   *ItemRetour,
                                  int     *Taille_Totale)
{
   FIC_enum_retour   retour;
   char              *chaine;
   boolean           boucle;
   int               taille_lue;

   Param = Param;

   *Taille_Totale = 0;
   boucle = TRUE;
   while ( boucle)
   {
      chaine = fgets( ItemRetour, RFR_MAX_ITEM, Fichier);

      if( chaine == NULL)
      {
         boucle = FALSE;
         retour = FIC_FINI;
      }
      else
      {
         taille_lue = (int)strlen( ItemRetour);
         *Taille_Totale = *Taille_Totale + taille_lue;

         /* est-ce que la chaine se limite au RC, ou bien est-ce un
          * commentaire
          */
         if(( taille_lue != 1) && ( ItemRetour[ 0] != COMMENTS))
         {
            boucle = FALSE;
            retour = FIC_OK;
         }
      }
   }

   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour EcrireItem( FILE *Fichier,
*                                      TItem *Item,
*                                      TParam *Param,
*                                      int    *Taille_Ecrite)
* PARAMETRES: pointeur sur le fichier
*             pointeur sur la zone de retour
*             pointeur sur la taille lue
* RETOUR: resultat de la fonction
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Sert pour l'ecriture d'un element Item sur un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour EcrireItem (FILE    *Fichier,
                                    TItem   *Item,
                                    TParam  *Param,
                                    int     *Taille_Ecrite)
{
   FIC_enum_retour retour;
   int             taille;

   Param = Param;

   taille = (int)strlen(Item);
   if( fwrite( Item, 1, taille, Fichier) == strlen(Item))
      retour = FIC_OK;
   else
      retour = FIC_NOK;

   *Taille_Ecrite = taille;
   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour LireIdVal( TItem *Item, int Position, TFichier *Id, TFichier *Val)
* PARAMETRES: pointeur sur l'Item
*             position de l'Item dans le fichier
*             pointeur sur la zone de retour
* RETOUR: resultat de la fonction
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Sert pour la lecture de l'element Id dans l'element Item
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireIdVal (TItem     *Item,
                                   long      Position,
                                   TParam    *Param,
                                   TFichier  *Id,
                                   TFichier  *Val)
{
   int             taille_lue;
   char            local_id[ RFR_MAX_ID];
   char            local_fichier[ RFR_MAX_FIC];
   char            local_reference[ RFR_MAX_REF];
   char            local_hostname[ RFR_MAX_HOST];
   int             iteration;
   char            *local_buffer;

   Position        = Position;
   Param           = Param;

   local_buffer = Item;

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

   /* Lecture de l'Id et de l'iteration */
   taille_lue = sscanf( local_buffer, FORMAT_1, local_id, &iteration);
   if( ( taille_lue == 0) || ( taille_lue == EOF))
       return ( FIC_ITEM_NOK);

   STR_verify( RFR_MAX_ID, local_id);

   /* y avait-il un numero d'iteration ou pas ? */
   if( taille_lue == 1)
       iteration = 0;

   /* Se positionner apres le separateur */
   local_buffer = strchr( local_buffer, SEPARATEUR_1);
   if( local_buffer == NULL)
       return ( FIC_ITEM_NOK);
   local_buffer ++;       /* pour retirer le separateur */

   /* Lecture du fichier */
   taille_lue = sscanf( local_buffer, FORMAT_2, local_fichier, local_reference);
   if( taille_lue != 2)
       return ( FIC_ITEM_NOK);

   STR_verify( RFR_MAX_FIC, local_fichier);
   STR_verify( RFR_MAX_REF, local_reference);

   /* Se positionner apres le separateur */
   local_buffer = strchr( local_buffer, SEPARATEUR_2);
   if( local_buffer == NULL)
       return ( FIC_ITEM_NOK);
   local_buffer ++;       /* pour retirer le separateur */

   /* Lecture du hostname optionnel*/
   taille_lue = sscanf( local_buffer, FORMAT_3, local_hostname);
   if( ( taille_lue == 0) || ( taille_lue == EOF))
       local_hostname[ 0] = '\0';

#pragma warning(pop)

   STR_verify( RFR_MAX_HOST, local_hostname);

   /* valoriser les parametres en entree */
   if( Id != NULL)
   {
      STR_strcpy( RFR_MAX_ID, Id->Id, local_id);
      Id->Iteration = iteration;
      STR_strcpy( RFR_MAX_FIC, Id->Fichier, local_fichier);
      STR_strcpy( RFR_MAX_REF, Id->Reference, local_reference);
      STR_strcpy( RFR_MAX_HOST, Id->Hostname, local_hostname);
   }

   if( Val != NULL)
   {
      STR_strcpy( RFR_MAX_ID, Val->Id, local_id);
      Val->Iteration = iteration;
      STR_strcpy( RFR_MAX_FIC, Val->Fichier, local_fichier);
      STR_strcpy( RFR_MAX_REF, Val->Reference, local_reference);
      STR_strcpy( RFR_MAX_HOST, Val->Hostname, local_hostname);
   }

   return( FIC_OK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour EcrireIdVal( TItem *Item, int Position, TFichier *Id, TFichier *Val)
* PARAMETRES: pointeur sur l'Item
*             position de l'Item dans le fichier
*             pointeur sur la zone de retour
* RETOUR: resultat de la fonction
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Sert pour la lecture de l'element Id dans l'element Item
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour EcrireIdVal (TItem     *Item,
                                     long      Position,
                                     TParam    *Param,
                                     TFichier  *Id,
                                     TFichier  *Val)
{
   char            *local_id;
   char            *local_fichier;
   char            *local_reference;
   char            *local_hostname;
   int             iteration;

   NO_WARNING( Position);
   NO_WARNING( Param);

   /* positionner les parametres en entree */
   if( Val != NULL)
   {
      local_id = Val->Id;
      iteration = Val->Iteration;
      local_fichier = Val->Fichier;
      local_reference = Val->Reference;
      local_hostname = Val->Hostname;
   }
   else if( Id != NULL)
   {
      local_id = Id->Id;
      iteration = Id->Iteration;
      local_fichier = Id->Fichier;
      local_reference = Id->Reference;
      local_hostname = Id->Hostname;
   }

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

   /* Lecture de l'Id et de l'iteration */
   if( iteration == 0)
   {
      sprintf( Item, FORMAT_ECRIT_1A, local_id);
   }
   else
   {
      sprintf( Item, FORMAT_ECRIT_1B, local_id, iteration);
   }

#pragma warning(pop)

   STR_strcat( RFR_MAX_ITEM, Item, local_fichier);
   STR_strcat( RFR_MAX_ITEM, Item, FORMAT_REF_1);
   STR_strcat( RFR_MAX_ITEM, Item, local_reference);
   STR_strcat( RFR_MAX_ITEM, Item, FORMAT_REF_2);

   if( local_hostname[0] != '\0')
   {
      STR_strcat( RFR_MAX_ITEM, Item, FORMAT_HOST);
      STR_strcat( RFR_MAX_ITEM, Item, local_hostname);
   }
   STR_strcat( RFR_MAX_ITEM, Item, "\n");

   STR_verify( RFR_MAX_ITEM, Item);

   return( FIC_OK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int ComparerId( TFichier *Id, TFichier *St_Id)
* PARAMETRES: pointeurs sur les 2 Id a comparer
* RETOUR: comparaison entre les 2 Id
*         Si Id < St_Id   => retour < 0
*         Si Id = St_Id   => retour = 0
*         Si Id > St_Id   => retour > 0
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Comparer deux Id.
*       La comparaison se fait sur l'Id s'il existe, puis soit
*       - sur la reference, si la chaine Reference est different de ""
*       - sur l'iteration, sinon
*       Si l'Id n'est pas positionne, alors la recherche se fait sur le
*       nom du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE int ComparerId (TFichier *Id, TFichier *St_Id, TParam *Param)
{
   int      cmp;

   Param = Param;
   if( Id->Id[0] == '\0')
   {
//	   if(strstr(St_Id->Fichier, Id->Fichier)==NULL)
//		   cmp=1;
//	   else
//		   cmp=0;
       cmp = _stricmp( Id->Fichier, St_Id->Fichier);
   }
   else
   {
       cmp = _stricmp( Id->Id, St_Id->Id);
       if( cmp == 0)
       {
         if( Id->Reference[0] == '\0')
         {
            if( Id->Iteration < St_Id->Iteration)
                cmp = -1;
            else if( Id->Iteration == St_Id->Iteration)
                cmp = 0;
            else
                cmp = 1;
         }
         else
            cmp = _stricmp( Id->Reference, St_Id->Reference);
       }
   }

   return( cmp);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int ComparerId( TFichier *Id, TFichier *St_Id)
* PARAMETRES: pointeurs sur les 2 Id a comparer
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
PRIVATE FIC_enum_retour TraiterIdVal (TGereFic         *GereFic,
                                      FIC_enum_retour  Etat,
                                      TItem            *Item,
                                      long             Position,
                                      TParam           *Param,
                                      TFichier         *Id,
                                      TFichier         *Val)
{
   TpTTraiterVal    traiterVal;

   Item       = Item;
   Position   = Position;
   Id         = Id;

   traiterVal = ( TpTTraiterVal) Param;
   return( traiterVal->Fonction( GereFic, Etat, Position,
                                 traiterVal->Param, Val));
}
