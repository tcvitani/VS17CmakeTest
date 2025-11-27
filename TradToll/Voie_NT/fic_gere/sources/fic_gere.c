/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FIC_GERE
* FICHIER: FIC_GERE.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:Gestion de fichier quelconques
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fic_gere/Sources/fic_gere.c_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:41:56   bph
 *  
 * 
 *    Rev 1.15   16 Apr 1998 10:01:30   HMO
 * Fonctionnement des deux fonctions de repositionnement
 * sur un fichier en mode texte ou binaire
 * 
* --------------------------------------------------------------------
* ATTENTION
* On effectue plusieurs fois de suite la fonction SePositionner,
* par exemple quand on ajout un item … la fin. -> A revoir
* --------------------------------------------------------------------
* $F_HEAD
*/

/*
EVOLUTIONS POSSIBLES :
Possibilite de se positionner par une fonction utilisateur sur l'‚l‚ment
suivant ou pr‚c‚dant, cette fonction remplacera celle utilis‚e par d‚faut
et elle permettra de fonctionner par dichotomie sur un fichier de taille
quelconque tri‚
...
*/

/*--------------- INCLUDES: ---------------*/
#include <IO.H>
#include <STDLIB.H>
#include <STDIO.H>
#include <share.h>

#include "fic.h"
#include "str.h"
#include "mem_c.h"
#include "err.h"

#include "fic_def.h"


/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- VERSION: --------------*/


/*--------------- DEFINES: ----------------*/
#ifndef TEST_PTR_NULL
#  define ERR_SauvePointeurNull() NULL
#  define ERR_TestPointeurNull(x) (x==x)
#endif

/*--------------- FUNCTIONS: --------------*/

PRIVATE FIC_enum_retour LireItemTailleFixe( FILE *fichier,
                                            TItem *Item,
                                            size_t taille);
PRIVATE FIC_enum_retour EcrireItemTailleFixe( FILE   *fichier,
                                              TItem  *Item,
                                              size_t taille);
PRIVATE FIC_enum_retour ChercherId( TGereFic *config,
                                    TId      *Id,
                                    TParam   *Param,
                                    TItem    *Item,
                                    TVal     *Val);
PRIVATE boolean SePositionnerA( TGereFic *config,
                                long     position_depart,
                                TParam *Param);
PRIVATE FIC_enum_retour LireItemInterne( TGereFic *config,
                                         TParam   *Param,
                                         TItem    *Item,
                                         TId      *Id,
                                         TVal     *Val);

/*--------------- VARIABLES: --------------*/



/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean Fichier_Charger( TGereFic     *config,
*                                          char         *nom_fichier,
*                                          char         *attribut,
*                                          size_t       taille_elem,
*                                          TLireItem    LireItem)
* PARAMETRES: la structure de configuration (config)
*             le fichier a utiliser (nom_fichier)
*             les attributs d'ouverture (attribut)
*             description de son contenu (taille_elem)
*             fonctions de traitement pour ce fichier (LireItem)
* RETOUR: TRUE = operation reussie, sinon FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Initialisation des informations necessaires pour utiliser le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean WINAPI Fichier_Charger( TGereFic     *config,
                                char         *nom_fichier,
                                char         *attribut,
                                size_t       taille_elem,
                                TLireItem    LireItem,
                                TEcrireItem  EcrireItem)
{
  // NO_WARNING( what);
  // errno_t err;

   /* Test pour le cas ou LireItem et EcrireItem est obligatoire */
   if( taille_elem == 0 && (LireItem == NULL || EcrireItem == NULL))
      return( FALSE);

   /* Ouverture du fichier */
//   err = fopen_s(&config->fichier, nom_fichier, attribut);

   config->fichier = _fsopen(nom_fichier, attribut, _SH_DENYNO);
   if (config->fichier == NULL)
      return( FALSE);

   FIC_setbuf( config->fichier, NULL);

   config->recherche = FIC_RECH_NONE;

   /* M‚morisatoin des parametres */
   config->TailleItem = (int)taille_elem;
   config->LireItem   = LireItem;
   config->EcrireItem = EcrireItem;

   /* Memorisation de la position initiale dans le ficher */
   config->position = 1;
   config->offset = 0;

   config->mode_tri          = FIC_NON_TRIE;
   config->cycle             = 0;
   config->LireIdVal         = NULL;
   config->Comparer          = NULL;
   config->Item              = NULL;
   config->Id                = NULL;
   config->Val               = NULL;
   config->conf              = FALSE;
   config->traverseEnCours   = FALSE;

   return( TRUE);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean Fichier_Echanger( TGereFic     *config,
*                                           char         *nom_fichier,
*                                           char         *attribut)
* PARAMETRES: la structure de configuration (config)
*             le fichier a utiliser (nom_fichier)
*             les attributs d'ouverture (attribut)
* RETOUR: TRUE = operation reussie, sinon FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Ouverture du fichier, quand la structure a d‚j… ete initialis‚
*       par un pr‚c‚dent appel.
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean WINAPI Fichier_Echanger( TGereFic     *config,
                                 char         *nom_fichier,
                                 char         *attribut)
{
   boolean	retour;
  // errno_t	err;

   if( config->fichier != NULL)
   {
      retour = Fichier_Fermer( config);
      if ( ! retour)
         return( FALSE);
   }

   /* Ouverture du fichier */
   
   //err = fopen_s(&config->fichier, nom_fichier, attribut);
   config->fichier = _fsopen(nom_fichier, attribut, _SH_DENYNO);

   if (config->fichier == NULL)
      return( FALSE);

   setvbuf(config->fichier, NULL, _IONBF, 0);

   config->recherche = FIC_RECH_NONE;

   /* Memorisation de la position initiale dans le ficher */
   config->position = 1;
   config->offset   = 0;
   config->conf     = TRUE;

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean Fichier_Configurer( TGereFic     *config,
*                                             FIC_enum_tri mode_tri,
*                                             unsigned int cycle,
*                                             TLireIdVal   LireIdVal,
*                                             TComparer    Comparer,
*                                             TItem        *Item,
*                                             TId          *Id,
*                                             TVal         *Val)
* PARAMETRES: la structure de configuration (config)
*             les attributs du fichier (mode_tri, cycle)
*             les fcts pour le fichier (LireId, LireVal, Comparer)
*             les zones de stockage temporaires (Item, Id, Val)
* RETOUR: TRUE = operation reussie, sinon FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Initialisation des informations necessaires pour utiliser le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean WINAPI Fichier_Configurer( TGereFic     *config,
                                   FIC_enum_tri mode_tri,
                                   unsigned int cycle,
                                   TLireIdVal   LireIdVal,
                                   TEcrireIdVal EcrireIdVal,
                                   TComparer    Comparer,
                                   TItem        *Item,
                                   TId          *Id,
                                   TVal         *Val)
{
   /* Test pour le cas ou LireItem est obligatoire */
   if( config->TailleItem == 0 && config->LireItem == NULL)
      return( FALSE);

   /* Test pour verifier l'existance des fonctions de travail */
   if( LireIdVal == NULL || Comparer == NULL)
      return( FALSE);

   /* Test pour verifier l'existance des zones de travail temporaires */
   if( Item == NULL || Id == NULL || Val == NULL)
      return( FALSE);

   /* M‚morisation des paramˆtres */
   config->mode_tri   = mode_tri;
   config->cycle      = cycle;
   config->LireIdVal  = LireIdVal;
   config->EcrireIdVal  = EcrireIdVal;
   config->Comparer   = Comparer;
   config->Item       = Item;
   config->Id         = Id;
   config->Val        = Val;
   config->conf       = TRUE;

   return( TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean Fichier_Fermer( TGereFic *config)
* PARAMETRES: la structure de configuration config
* RETOUR: TRUE = operation reussie, sinon FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Fin d'utilisation du fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean WINAPI Fichier_Fermer( TGereFic *config)
{
   boolean retour = FALSE;

   /* le fflush ne doit pas ˆtre test‚, car si le fichier n'est pas *
    * writable, il echoue
    */
   fflush( config->fichier);

   if( fclose( config->fichier) == 0)
      retour = TRUE;
   config->fichier = NULL;
   config->conf    = FALSE;

   return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour Fichier_LireIdPos( TGereFic *config,
*                                                    int      position_depart,
*                                                    TId      *Id,
*                                                    TParam   *param,
*                                                    int      *position,
*                                                    TItem    **Item,
*                                                    TVal     **Val)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_LireIdPos( TGereFic *config,
                                          long      position_depart,
                                          TId      *Id,
                                          TParam   *Param,
                                          long     *position,
                                          TItem    **Item,
                                          TVal     **Val)
{
   TVal  *tmp_val = NULL;
   FIC_enum_retour retour;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return(FIC_ECHEC);

   if( position != NULL)
      *position = position_depart;

   if( Val != NULL)
   {
      tmp_val = config->Val;
      *Val  = tmp_val;
   }
   else
      tmp_val = NULL;

   if( SePositionnerA( config, position_depart, Param) == FALSE)
      retour = FIC_NOK;
   else
      retour = ChercherId( config, Id, Param, config->Item, tmp_val);

   if( Item != NULL)
   {
      *Item = config->Item;
   }

   if( retour == FIC_OK)
   {
      if( position != NULL)
         *position = config->position - 1;
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour Fichier_LireId( TGereFic *config,
*                                                 TId      *Id,
*                                                 TParam   *param,
*                                                 int      *position,
*                                                 TItem    **Item,
*                                                 TVal     **Val)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_LireId( TGereFic *config,
                                       TId      *Id,
                                       TParam   *Param,
                                       long     *position,
                                       TItem    **Item,
                                       TVal     **Val)
{
   TVal  *tmp_val  = NULL;
   FIC_enum_retour retour;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return(FIC_ECHEC);

   if( position != NULL)
      *position = config->position;

   if( Val != NULL)
   {
      tmp_val = config->Val;
      *Val  = tmp_val;
   }
   else
      tmp_val = NULL;

   /* Choix de la position de d‚part par d‚faut */
   if( config->mode_tri == FIC_NON_TRIE || config->TailleItem == 0)
   {
      if( SePositionnerA( config, 1, Param) == TRUE)
         retour = ChercherId( config, Id, Param, config->Item, tmp_val);
      else
         retour = FIC_NOK;
   }
   else
   {
      retour = ChercherId( config, Id, Param, config->Item, tmp_val);
   }

   if( Item != NULL)
   {
      *Item = config->Item;
   }

   if( retour == FIC_OK)
   {
      if( position != NULL)
         *position = config->position - 1;
   }

   if( retour == FIC_FINI)
       retour = FIC_NOK;

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour Fichier_LirePos( TGereFic *config,
*                                                  int      position,
*                                                  TParam   *param,
*                                                  TItem    **Item,
*                                                  TId      **Id,
*                                                  TVal     **Val)
*
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_LirePos( TGereFic *config,
                                        long     position,
                                        TParam   *Param,
                                        TItem    **Item,
                                        TId      **Id,
                                        TVal     **Val)
{
   TVal  *tmp_val  = NULL;

   FIC_enum_retour retour;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return(FIC_ECHEC);

   if( Val != NULL)
   {
      tmp_val = config->Val;
   }

   if( SePositionnerA( config, position, Param) == TRUE)
      retour = LireItemInterne( config, Param, config->Item, config->Id, tmp_val);
   else
      retour = FIC_NOK;

   if( retour == FIC_OK)
   {
      if( Item != NULL)
         *Item = config->Item;
      if( Id != NULL)
         *Id = config->Id;
      if( Val != NULL)
         *Val = config->Val;
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour Fichier_FormatterId(
*                                                 TGereFic *config,
*                                                 TId      *Id,
*                                                 TParam   *param,
*                                                 int      position,
*                                                 TItem    *Item,
*                                                 TVal     *Val)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_FormatterId( TGereFic *config,
                                            TId      *Id,
                                            TParam   *Param,
                                            long     position,
                                            TItem    *Item,
                                            TVal     *Val)
{
   FIC_enum_retour retour;
//   long ad0;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return(FIC_ECHEC);

   if( config->EcrireIdVal != NULL)
   {
//      ad0 = ERR_SauvePointeurNull();
      retour = config->EcrireIdVal( Item, position, Param, Id, Val);
//      ERR_EstVrai( ERR_TestPointeurNull( ad0));
   }
   else
       retour = FIC_NOK;

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Ecriture (remplacement) d'un element dans le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_RemplacerId( TGereFic *config,
                                            TId      *Id,
                                            TParam   *Param,
                                            long     *position,
                                            TItem    *Item)
{
   TItem *tmp_item = NULL;
   long   position_locale;

   FIC_enum_retour retour;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return( FIC_ECHEC);

   if( position != NULL)
      *position = config->position;

   if( Item == NULL)
   {
      return( FIC_ECHEC);
   }

   /* Recherche de la position de l'Item */
   retour = Fichier_LireId( config, Id, Param, &position_locale, &tmp_item, NULL);

   if( retour == FIC_OK)
   {
      retour = Fichier_RemplacerPos( config, Param, position_locale, Item);
   }

   if( position != NULL)
      *position = position_locale;


   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Ecriture (remplacement) d'un element dans le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_RemplacerPos( TGereFic *config,
                                             TParam   *Param,
                                             long     position,
                                             TItem    *Item)
{
   FIC_enum_retour retour;
   int             taille;
//   long            ad0;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return( FIC_ECHEC);

   if( Item == NULL)
   {
      return( FIC_ECHEC);
   }

   if( SePositionnerA( config, position, Param) == FALSE)
      retour = FIC_NOK;
   else
   {
      if( config->TailleItem == 0)
      {
//         ad0 = ERR_SauvePointeurNull();
         retour = config->EcrireItem(config->fichier, Item, Param, &taille);
//         ERR_EstVrai( ERR_TestPointeurNull( ad0));
      }
      else
      {
         retour = EcrireItemTailleFixe( config->fichier,
                                        Item,
                                        config->TailleItem);
         taille = config->TailleItem;
      }
      if( retour == FIC_OK)
      {
         FIC_fflush( config->fichier);
         config->position++;
         config->offset += taille;
      }
      else
      {
         if( SePositionnerA( config, position, Param) == FALSE)
            retour = FIC_NOK;
      }
   }


   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Ecriture (ajout) d'un element dans le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_AjouterId( TGereFic *config,
                                          TId      *Id,
                                          TItem    *Item,
                                          TParam   *Param,
                                          long     *position)
{
   FIC_enum_retour retour;
   long            position_locale;
   int             comparaison;
//   long            ad0;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return( FIC_ECHEC);

   if( Item == NULL)
      return( FIC_ECHEC);

   retour = Fichier_LireId( config, Id, Param, NULL, config->Item, NULL);

   if( retour == FIC_OK)
   {
      /* Erreur, l'‚l‚ment existe d‚ja */
      retour = FIC_NOK;
   }
   else
   {
      position_locale = config->position;
      if( config->mode_tri == FIC_TRIE)
      {
         if(config->position == 1)
         {
            /* Le fichier est vide */
            /* On est d‚ja en position 1, offset 0 */
         }
         else
         {
//            ad0 = ERR_SauvePointeurNull();
            comparaison = config->Comparer(Id, config->Id, Param);
//            ERR_EstVrai( ERR_TestPointeurNull( ad0));

            if( !feof(config->fichier) && comparaison > 0)
            {
               /* Il faut se placer sur l'‚l‚ment pr‚c‚dant de */
               /* la recherche qui a ‚t‚ trouv‚ en FIC_RECH_UP */
               position_locale = config->position - 1;
            }
         }
      }
      retour = Fichier_AjouterPos( config, position_locale, Item, Param);
   }

   if( position != NULL)
       *position = position_locale;

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Ecriture (ajout) d'un element dans le fichier
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_AjouterPosition( TGereFic *config,
                                                long     position,
                                                TItem    *Item,
                                                TParam   *Param,
                                                long     *position_ecrite)
{
   FIC_enum_retour retour;

   /* La configuration est coh‚rente */
   if( config->conf != TRUE)
      return( FIC_ECHEC);

   if( Item == NULL)
      return( FIC_ECHEC);

   if( SePositionnerA( config, position, Param) == FALSE)
      retour = FIC_NOK;
   else
   {
      /* V‚rifier la fin du fichier */
      if( Fichier_LirePos( config, position, Param,
                           &config->Item, &config->Id, NULL) != FIC_OK)
      {
         retour = Fichier_RemplacerPos( config, Param, position, Item);
         if( position_ecrite != NULL)
             *position_ecrite = config->position -1;
      }
      else
         retour = FIC_NOK;
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FIC_enum_retour Fichier_Traverse( TGereFic *GereFic,
*                                                   TFcn_func Function,
*                                                   void *Param)
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Lecture de la valeur associ‚e a l'Id
* --------------------------------------------------------------------
* $F_FCTN
*/
FIC_enum_retour Fichier_Traverse( TGereFic      *GereFic,
                                  TTraiterIdVal Function,
                                  void          *Param)
{
  FIC_enum_retour    retour;
  long               pos;
  TItem              *item;
  TId                *id;
  TVal               *val;
  unsigned int       nbItemsTraverses;

  if( ! GereFic->traverseEnCours)
  {
      pos = 1;
      GereFic->traverseEnCours = TRUE;
  }
  else
      pos = GereFic->position;

  nbItemsTraverses = 0;
  while( pos != 0)
  {
    retour = Fichier_LirePos( GereFic, pos, Param, &item, &id, &val);
    retour =  Function( GereFic, retour, item, pos, Param, id, val);
    if( retour == FIC_OK || retour == FIC_ITEM_NOK )
    {
      nbItemsTraverses ++;
      if( ( GereFic->cycle != 0) && (nbItemsTraverses >= GereFic->cycle))
      {
          pos = 0;
          retour = FIC_EN_COURS;
      }
      else
          pos ++;
    }
    else
    {
      pos = 0;
      GereFic->traverseEnCours = FALSE;
    }
  }

  return( retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour Fichier_Memoriser( TGereFic  *GereFic,
*                                             TInfoFlux *InfoFlux)
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: M‚morisation des informations de flux de l'item en cours
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_Memoriser( TGereFic  *GereFic,
                                          TInfoFlux *InfoFlux)
{
   FIC_enum_retour    retour = FIC_NOK;

   /* Verification sur le fichier */
   if( GereFic->fichier != NULL)
   {
      /* Sauvegarde des informations */
      InfoFlux->position = GereFic->position;
      InfoFlux->offset   = ftell( GereFic->fichier);
      if( InfoFlux->offset == -1)
         retour = FIC_NOK;
      else
         retour = FIC_OK;
      retour = FIC_OK;
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_enum_retour Fichier_Restaurer( TGereFic  *GereFic,
*                                             TInfoFlux *InfoFlux)
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: M‚morisation des informations de flux de l'item en cours
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC FIC_enum_retour WINAPI Fichier_Restaurer( TGereFic  *GereFic,
                                          TInfoFlux *InfoFlux)
{
   FIC_enum_retour    retour = FIC_NOK;

   /* Verification sur le fichier */
   if( GereFic->fichier != NULL)
   {
      /* Restitution des informations */
      GereFic->position = InfoFlux->position;
      GereFic->offset   = InfoFlux->offset;

      /* Positionnement du fichier */
      if( fseek( GereFic->fichier, GereFic->offset, SEEK_SET))
         retour = FIC_NOK;
      else
         retour = FIC_OK;
   }

   return( retour);
}



/*==================================================================*/
/*==================================================================*/
/*==================================================================*/
/*==================================================================*/





/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE boolean LireItemTailleFixe( FILE   *fichier,
*                                              TItem  *Item,
*                                              size_t taille)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireItemTailleFixe( FILE   *fichier,
                                            TItem  *Item,
                                            size_t taille)
{
   size_t      taille_lue;

   taille_lue = fread( Item, 1, taille, fichier);

   if( taille_lue == taille)
      return( FIC_OK);
   else if( taille_lue == 0)
      return( FIC_FINI);
   else
      return( FIC_NOK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE boolean EcrireItemTailleFixe( FILE   *fichier,
*                                                TItem  *Item,
*                                                size_t taille)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Lecture d'un element
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour EcrireItemTailleFixe( FILE   *fichier,
                                              TItem  *Item,
                                              size_t taille)
{
   if( fwrite( Item, 1, taille, fichier) == taille)
      return( FIC_OK);
   else
      return( FIC_NOK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE boolean SePositionnerA( TGereFic *config,
*                                          int      position_depart,
*                                          TParam   *Param)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE boolean SePositionnerA( TGereFic *config,
                                long     position_depart,
                                TParam   *Param)
{
   int             index,
                   taille_lue;
   boolean         retour = TRUE;
   FIC_enum_retour etat;
//   long            ad0;

   if( position_depart == FIC_END)
   {
      /* Fichier de taille unique */
      if( config->TailleItem != 0)
      {
         if( fseek( config->fichier, 0, SEEK_END))
            retour = FALSE;
         config->offset = ftell( config->fichier);
         config->position =  (config->offset / config->TailleItem) + 1;
      }
      else
      {
         /* Fichier de taille quelconque */
         while(retour == TRUE)
         {
//            ad0 = ERR_SauvePointeurNull();
            etat = config->LireItem( config->fichier, Param, config->Item, &taille_lue);
//            ERR_EstVrai( ERR_TestPointeurNull( ad0));

            if( etat != FIC_OK)
            {
               retour = FALSE;
            }

            if( retour == TRUE)
            {
               /* Actualiser les nouvelles positions */
               config->position++;
               config->offset = ftell( config->fichier);
            }
         }
         if( etat == FIC_FINI)
             retour = TRUE;
      }
   }
   else if( position_depart != config->position)
   {
      /* Fichier de taille unique */
      if( config->TailleItem != 0)
      {
         config->position = position_depart;
         config->offset = (position_depart - 1) * config->TailleItem;
         if( fseek( config->fichier, config->offset, SEEK_SET))
            retour = FALSE;
      }
      else
      {
         /* Fichier de taille quelconque */
         FIC_fseek(config->fichier, 0, SEEK_SET);

         for( index = 1; index < position_depart; index++)
         {
//            ad0 = ERR_SauvePointeurNull();
            etat = config->LireItem( config->fichier, Param, config->Item, &taille_lue);
//            ERR_EstVrai( ERR_TestPointeurNull( ad0));

            if( etat != FIC_OK)
            {
               retour = FALSE;
               break;
            }
         }
         if( retour == TRUE)
         {
            /* Actualiser les nouvelles positions */
            config->position = index;
            config->offset = FIC_ftell( config->fichier);
         }
         else
         {
            /* Se repositionner … la position pr‚c‚dente */
            FIC_fseek( config->fichier, config->offset, SEEK_SET);
         }
      }
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PRIVATE FIC_enum_retour ChercherId( TGereFic *config,
*                                             int      position_depart,
*                                             TId      *Id,
*                                             TItem    *Item,
*                                             TVal     *Val)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: Recherche sur un fichier de taille fixe
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour ChercherId( TGereFic *config,
                                    TId      *Id,
                                    TParam   *Param,
                                    TItem    *Item,
                                    TVal     *Val)
{
   boolean         sortir = FALSE,
                   premiere_lecture = TRUE;
   FIC_enum_retour lecture;
   int             comparaison;
//   long            ad0;

   while( !sortir)
   {
      lecture = LireItemInterne( config, Param, Item, config->Id, Val);

      if( premiere_lecture == TRUE)
      {
         if( lecture != FIC_OK)
         {
            if( SePositionnerA( config, 1, Param) == TRUE)
                lecture = LireItemInterne( config, Param, Item, config->Id, Val);
         }
         premiere_lecture = FALSE;
      }

      if( ( lecture != FIC_OK) && ( lecture != FIC_ITEM_NOK))
         sortir = TRUE;

      if( lecture == FIC_OK)
      {

//         ad0 = ERR_SauvePointeurNull();
         comparaison = config->Comparer(Id, config->Id, Param);
//         ERR_EstVrai( ERR_TestPointeurNull( ad0));

         if( comparaison == 0)
         {
            /* L'Id a ‚t‚ trouv‚ */
            sortir = TRUE;
            lecture = FIC_OK;
            config->recherche = FIC_RECH_NONE;
         }
         /* Ce n'est pas le bon et le fichier n'est pas tri‚ */
         /* ou les Item sont de taille quelconque */
         else if( config->mode_tri == FIC_NON_TRIE ||
                  config->TailleItem == 0)
         {
            /* Recherche dans le sens de mont‚e */
            comparaison = 1;
         }

         /* D‚terminer le sens de recherche */
         if( !sortir && config->recherche == FIC_RECH_NONE)
         {
            if( comparaison < 0)
               config->recherche = FIC_RECH_DOWN;
            else
               config->recherche = FIC_RECH_UP;
         }

         if( !sortir)
         {
            if( comparaison > 0 && config->recherche == FIC_RECH_UP)
            {
               /* Le pointeur du fichier est deja bien place */
            }
            else if( comparaison < 0 && config->recherche == FIC_RECH_DOWN)
            {
               config->position -= 2;
               config->offset -= 2 * config->TailleItem;
               fseek( config->fichier, config->offset, SEEK_SET);
            }
            else
            {
               sortir = TRUE;
               lecture = FIC_NOK;
               config->recherche = FIC_RECH_NONE;
            }
         }
      }
   }

   return( lecture);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE FIC_enum_retour LireItemInterne( TGereFic *config,
*                                            TParam   *param,
*                                            TItem    *Item,
*                                            TId      *Id,
*                                            TVal     *Val)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE:
*
* REMARQUE :
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour LireItemInterne( TGereFic *config,
                                  TParam   *Param,
                                  TItem    *Item,
                                  TId      *Id,
                                  TVal     *Val)
{
   FIC_enum_retour lecture;
   int             taille = config->TailleItem;
//   long            ad0;

   if(taille == 0)
   {
//      ad0 = ERR_SauvePointeurNull();
      lecture = config->LireItem( config->fichier, Param, Item, &taille);
//      ERR_EstVrai( ERR_TestPointeurNull( ad0));
   }
   else
   {
      lecture = LireItemTailleFixe( config->fichier, Item, config->TailleItem);
   }

   if( lecture == FIC_OK)
   {
      config->position++;
      config->offset += taille;

//      ad0 = ERR_SauvePointeurNull();
      lecture = config->LireIdVal( Item, config->position-1, Param, Id, Val);
//      ERR_EstVrai( ERR_TestPointeurNull( ad0));
   }
   else
       fseek( config->fichier, config->offset, SEEK_SET);

   return( lecture);
}

