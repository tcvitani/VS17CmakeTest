/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DE CONFIGURATION
* FICHIER: FIC_CONF.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Permet de lire les couples <ID> <valeur> d'un fichier
*         de configuration quelconque
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fic_conf/Sources/fic_conf.c_v  $
 * 
 *    Rev 1.2   13 Dec 1999 16:24:36   afx
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:41:50   bph
 *  
 * 
 *    Rev 1.12   Jul 28 1997 16:40:52   DPI
 * Gestion de la version de la librairie
 * 
 *    Rev 1.11   Jul 16 1997 17:26:14   HMO
 * Ajout du LF sur l'ecriture des guillemets
 * 
 *    Rev 1.10   May 23 1997 16:34:02   HMO
 * Ajout de la fonction CommentaireFichierConfig qui  permet
 * d'ajouter des commentaires a la fin du fichier
 * 
 * 
 *    Rev 1.9   May 13 1997 11:29:42   HMO
 *  
 * 
 *    Rev 1.8   Apr 09 1997 09:41:10   DPI
 * Intégration des fonctions d'écritures de 
 * fichier de configuration
 * 
 *    Rev 1.7   Apr 07 1997 14:42:28   HMO
 * Passage du buffer pour les items en externe,
 * defini par l'utilisateur
 * 
 *    Rev 1.6   Apr 07 1997 12:03:26   HMO
 * Correction suppression sur le dernier "
 * 
 *    Rev 1.5   Apr 05 1997 16:16:08   DPI
 * Correction de bugs
 * 
 *    Rev 1.4   Apr 04 1997 11:48:46   DPI
 * Oubli de la sauvegarde du fichier
 * 
 *    Rev 1.3   Apr 04 1997 11:47:28   DPI
 * Afficher l'historique
 *
 *    Rev 1.2   Apr 04 1997 11:44:56   DPI
 * Ramener la declaration des TItem, TVal et TId
 * dans le .C
 * 
 *    Rev 1.1   Apr 04 1997 11:44:56   DPI
 * Mise en PRIVATE de 'EcrireItem'
 *
 *    Rev 1.0   Mar 19 1997 15:47:24   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include <windows.h>

#include "str.h"
#include "err.h"

typedef struct StructValeur TValeur;

#define TItem  char
#define TId    TValeur
#define TVal   TValeur

#include "fic_gere.h"

#include "fic_conf.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- VERSION --------------*/

/*--------------- EXTERNALS: --------------*/



/*--------------- DEFINES: ----------------*/

#define FIC_CYCLE_LECTURE           0

/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour      LireItem( FILE   *Fichier,
                                       TParam *Param,
                                       TItem  *ItemRetour,
                                       int    *Taille_lue);
PRIVATE FIC_enum_retour      EcrireItem( FILE   *Fichier,
                                         TItem  *Item,
                                         TParam *Param,
                                         int    *Taille_ecrite);
PRIVATE FIC_enum_retour      LireIdVal( TItem  *Item,
                                        long    Position,
                                        TParam *Param,
                                        TId    *Id,
                                        TVal   *Val);
PRIVATE FIC_enum_retour      EcrireIdVal( TItem     *Item,
                                          long      Position,
                                          TParam    *Param,
                                          TId       *Id,
                                          TVal      *Val);
PRIVATE int                  ComparerId( TId    *Id,
                                         TId    *St_Id,
                                         TParam *Param);


/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean InitFichierConfig( TGereFic *GereFic,
*                                            char     *NomFichier,
*                                            TItem    *Item,
*                                            TId      *Id,
*                                            TId      *Val)
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
PUBLIC boolean InitFichierConfig( TGereFic *GereFic,
                                  char     *NomFichier,
                                  char     *Attributs,
                                  TItem    *Item,
                                  TId      *Id,
                                  TId      *Val)
{
   //NO_WARNING( what);

   if( !Fichier_Charger( GereFic, NomFichier, Attributs, 0, LireItem, EcrireItem))
   {
      return( FALSE);
   }

   if( !Fichier_Configurer( GereFic, FIC_NON_TRIE, FIC_CYCLE_LECTURE,
                            LireIdVal, EcrireIdVal, ComparerId,
                            Item, Id, Val) )
   {
      return( FALSE);
   }

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour EchangerFichierConfig( TGereFic *GereFic,
*                                                       char *NomFichier)
* RETOUR: TRUE  : op‚ration r‚ussie
*         FALSE : op‚ration ‚chou‚e
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture d'un fichier en exploitant la structure pr‚c‚demment
*       remplie
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean EchangerFichierConfig( TGereFic *GereFic,
                                      char     *NomFichier,
                                      char     *Attributs)
{
   if( !Fichier_Echanger( GereFic, NomFichier, Attributs))
   {
      return( FALSE);
   }

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour FermerFichierConfig( TGereFic *GereFic)
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

PUBLIC boolean FermerFichierConfig( TGereFic *GereFic)
{
   return( Fichier_Fermer( GereFic));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour LireConfig(TId *Id, TVal **Val)
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
PUBLIC FIC_enum_retour LireFichierConfig( TGereFic *GereFic,
                                          char     *Identifiant,
                                          int      Iteration,
                                          TVal     *p_Val)
{
   FIC_enum_retour   retour;
   TValeur           id_recherche;
   TValeur           *valeur_lue;

   STR_strcpy( CONF_MAX_ID, id_recherche.Id, Identifiant);
   id_recherche.Iteration = Iteration;

   retour = Fichier_LireId( GereFic, &id_recherche, NULL, NULL, NULL, &valeur_lue);
   if( retour == FIC_OK)
      *p_Val = *valeur_lue;

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour RemplacerFichierConfig
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
PUBLIC FIC_enum_retour RemplacerFichierConfig( TGereFic *GereFic,
                                               char     *Id,
                                               int      Iteration,
                                               char     *Contenu)
{
   FIC_enum_retour      retour;
   TVal                 val_ecrire;
   TItem                item[ CONF_MAX_ITEM];

   STR_strcpy( CONF_MAX_ID, val_ecrire.Id, Id);
   val_ecrire.Iteration = Iteration;
   STR_strcpy( CONF_MAX_VAL, val_ecrire.Contenu, Contenu);

   retour = Fichier_FormatterId( GereFic, NULL, NULL, 0, item, &val_ecrire);
   if( retour == FIC_OK)
   {
      retour = Fichier_RemplacerId( GereFic, &val_ecrire, NULL, NULL, item);
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour AjouterFichierConfig
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
PUBLIC FIC_enum_retour AjouterFichierConfig( TGereFic *GereFic,
                                             char     *Id,
                                             int      Iteration,
                                             char     *Contenu)
{
   FIC_enum_retour      retour;
   TVal                 val_ecrire;
   TItem                item[ CONF_MAX_ITEM];

   STR_strcpy( CONF_MAX_ID, val_ecrire.Id, Id);
   val_ecrire.Iteration = Iteration;
   STR_strcpy( CONF_MAX_VAL, val_ecrire.Contenu, Contenu);

   retour = Fichier_FormatterId( GereFic, NULL, NULL, 0, item, &val_ecrire);
   if( retour == FIC_OK)
   {
      retour = Fichier_AjouterPos( GereFic, FIC_END, item, NULL);
   }

   return( retour);
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour CommentaireFichierConfig
* RETOUR: FIC_OK    : Id trouv‚
*         FIC_NOK   : Id non trouv‚
*         FIC_ECHEC : problŠme sur le fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ecriture d'un commentaire a la fin du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour CommentaireFichierConfig( TGereFic *GereFic,
                                                 char     *Commentaire)
{
   FIC_enum_retour      retour;
   TItem                item[ CONF_MAX_ITEM];

   /* Debut de commentaire */
   item[0] = COMMENTS;
   item[1] = 0;
   STR_strcat( CONF_MAX_ITEM, item, Commentaire);
   /* Fin de commentaire */
   STR_strcat( CONF_MAX_ITEM, item, "\n");

   retour = Fichier_AjouterPos( GereFic, FIC_END, item, NULL);

   return( retour);
}

/*=====================================================================/*
/*=====================================================================/*
/*=====================================================================/*
/*=====================================================================/*






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
* ROLE: Sert pour la lecture d'un ‚l‚ment Item sur un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireItem( FILE   *Fichier,
                                  TParam *Param,
                                  TItem  *ItemRetour,
                                  int    *Taille_Totale)
{
   FIC_enum_retour   retour;
   char              *chaine;
   boolean           boucle;
   int               taille_lue;

   NO_WARNING( Param);

   *Taille_Totale = 0;
   boucle = TRUE;
   while ( boucle)
   {
      chaine = fgets( ItemRetour, CONF_MAX_ITEM, Fichier);

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
* ROLE: Sert pour l'ecriture d'un ‚l‚ment Item sur un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour EcrireItem( FILE   *Fichier,
                                   TItem  *Item,
                                   TParam *Param,
                                   int    *Taille_Ecrite)
{
   FIC_enum_retour retour;
   size_t          taille;

   NO_WARNING( Param);

   taille = strlen( Item);

   // ATTENTION cette ligne de code n'est pas une abberation ! Elle permet au fwrite qui
   // suit de ne pas echouer (voir explications MSDN sur ftell, fseek sur fichier en mode texte)
   fseek (Fichier, (long)0, SEEK_CUR);  // recalibrage du pointeur de fichier

   if( fwrite( Item, 1, taille, Fichier) == taille)
      retour = FIC_OK;
   else
      retour = FIC_NOK;

   *Taille_Ecrite = (int)taille;
   return( retour);
}


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
   int          taille_lue;
   char         local_id[CONF_MAX_ID];
   char         local_val[CONF_MAX_VAL];
   int          iteration;
   char         *local_buffer;

   NO_WARNING( Position);
   NO_WARNING( Param);

   local_buffer = Item;

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
   /* Lecture de l'Id et de l'it‚ration */
   //taille_lue = sscanf_s( local_buffer, " %[_0-9a-zA-Z-] [ %d ]", local_id, &iteration);
   taille_lue = sscanf(local_buffer, " %[_0-9a-zA-Z-] [ %d ]", local_id, &iteration);

#pragma warning(pop)

   if( ( taille_lue == 0) || ( taille_lue == EOF))
       return ( FIC_ITEM_NOK);

   /* y avait-il un numero d'iteration ou pas ? */
   if( taille_lue == 1)
       iteration = 0;

   /* Se positionner aprŠs le '=' */
   local_buffer = strchr( local_buffer, '=');
   if( local_buffer == NULL)
       return ( FIC_ITEM_NOK);
   local_buffer ++;       /* pour retirer le '=' */


#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
   /* recherche d'un guillemet pour un debut de chaine */
   //taille_lue = sscanf_s( local_buffer, " %[\"] ",local_val);
   taille_lue = sscanf(local_buffer, " %[\"] ", local_val);

#pragma warning(pop)

   if( taille_lue == EOF)
      return ( FIC_ITEM_NOK);

   /* pas de '\"' : valeur autre qu'une chaine */
   if( taille_lue == 0 )
   {
#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
      /* Lecture du Val */
      //taille_lue = sscanf_s( local_buffer, " %s", local_val);
	   taille_lue = sscanf(local_buffer, " %s", local_val);

#pragma warning(pop)

      if( ( taille_lue == 0) || ( taille_lue == EOF))
         return ( FIC_ITEM_NOK);
   }
   else
   {
      /* Se positionner aprŠs le '"' */
      local_buffer = strchr( local_buffer, '"');
      if( local_buffer == NULL)
         return ( FIC_ITEM_NOK);
      local_buffer ++;       /* pour retirer le '"' */

      /* Recopie de la chaine */
      STR_strcpy( CONF_MAX_VAL, local_val, local_buffer);

      /* recherche du second guillemet de fin de chaine DANS la copie*/
      local_buffer = strchr(local_val,'"');
      if( local_buffer == NULL)
         return ( FIC_ITEM_NOK);

      /* verifier qu'il n'y en a pas un troisieme */
      if( strchr(local_buffer +1,'"') != NULL)
         return ( FIC_ITEM_NOK);

      *local_buffer = '\0';
   }

   if( Id != NULL)
   {
      strcpy_s( Id->Id,sizeof(Id->Id), local_id);
      Id->Iteration = iteration;
	  strcpy_s(Id->Contenu, sizeof(Id->Contenu), local_val);
   }

   if( Val != NULL)
   {
	  strcpy_s(Val->Id, sizeof(Val->Id), local_id);
      Val->Iteration = iteration;
	  strcpy_s(Val->Contenu, sizeof(Val->Contenu), local_val);
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
* ROLE: Sert pour la lecture de l'‚l‚ment Id dans l'‚l‚ment Item
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour EcrireIdVal( TItem       *Item,
                                     long        Position,
                                     TParam      *Param,
                                     TId         *Id,
                                     TVal        *Val)
{
   char            *local_id;
   char            *local_contenu;
   int             iteration;
   char            *ptrItem;
   boolean         ajouterGuillemets;

   NO_WARNING( Position);
   NO_WARNING( Param);

   ptrItem = Item;
   /* positionner les paramŠtres en entr‚e */
   if( Val != NULL)
   {
      local_id = Val->Id;
      iteration = Val->Iteration;
      local_contenu = Val->Contenu;
   }
   else if( Id != NULL)
   {
      local_id = Id->Id;
      iteration = Id->Iteration;
      local_contenu = Id->Contenu;
   }
#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
   /* Ecriture de l'Id et de l'it‚ration */
   if( iteration == 0)
   {
	   sprintf(ptrItem,  "%s = ", local_id);
   }
   else
   {
      sprintf( ptrItem, "%s [ %d] = ", local_id, iteration);
   }
   ptrItem += strlen( ptrItem);

#pragma warning(pop)

   /* faut-il introduire des "" ? */
   ajouterGuillemets = ( strchr( local_contenu, ' ') != NULL);
   if( ! ajouterGuillemets)
   {
      ajouterGuillemets = ( strchr( local_contenu, '\t') != NULL);
   }

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

   if( ajouterGuillemets)
   {
      sprintf( ptrItem, "\"%s\"\n", local_contenu);
   }
   else
   {
      sprintf( ptrItem, "%s\n", local_contenu);
   }

#pragma warning(pop)

   STR_verify( CONF_MAX_ITEM, Item);

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

   if( Id->Iteration < St_Id->Iteration)
      cmp = -1;
   else if( Id->Iteration == St_Id->Iteration)
      cmp = strcmp( Id->Id, St_Id->Id);
   else
      cmp = 1;

   return( cmp);
}
