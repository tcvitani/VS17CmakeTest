/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FIC_GERE (Gestion de fichiers quelconques).
* FICHIER: FIC_GERE.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ficher d'entete du module FIC_GERE
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
*
* ATTENTION
*
*           Il ne faut pas modifier les donn‚es fournies en retour
*           par ces fonctions, car le contenu des zones de stockage peut
*           ˆtre modifi‚es lors d'un nouvel appel
*
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fic_gere/Includes/fic_gere.h_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:40   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:41:52   bph
 *  
 * 
 *    Rev 1.9   16 Apr 1998 10:01:32   HMO
 * Fonctionnement des deux fonctions de repositionnement
 * sur un fichier en mode texte ou binaire
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_GERE_H
#define FIC_GERE_H

/*--------------- INCLUDES: ---------------*/
#ifdef FIC_GERE_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>
#include <stdio.h>

/*--------------- RESERVED: ---------------*/


/*--------------- VERSION: --------------*/

#define FIC_GERE_VERSION                  "3.30"

/*--------------- DEFINES: ----------------*/

typedef enum
{
   FIC_NON_TRIE,
   FIC_TRIE
}FIC_enum_tri;

typedef enum
{
   FIC_ECHEC,     /* Une erreur s'est produite sur le fichier */

   FIC_ITEM_NOK,  /* Une erreur s'est produite pendant l'analyse de l'item */

   FIC_NOK,       /* L'‚l‚ment n'a pas ‚t‚ trouv‚ */
   FIC_OK,         /* L'‚l‚ment a ‚t‚ trouv‚, ou alors tout va bien */

   FIC_FINI,      /* On a atteint la fin du fichier */

   FIC_EN_COURS,  /* Un traitement par cycle est en cours et n'est pas fini */
}FIC_enum_retour;

#define FIC_END -1

typedef enum
{
   FIC_RECH_NONE,
   FIC_RECH_UP,
   FIC_RECH_DOWN
}FIC_enum_rech;

/**/
/* D‚finition des donn‚es par d‚faut */
/* Vous pouvez d‚finir vos types de donn‚es avant d'appeler fic_gere.h */
/* pour pouvoir les utiliser directement au lieu d'utiliser un void */
#ifndef TItem
#define TItem void
#endif
#ifndef TId
#define TId void
#endif
#ifndef TVal
#define TVal void
#endif
#ifndef TParam
#define TParam void
#endif

/*--------------- TYPEDEF: --------------*/

typedef struct Struct_GereFic TGereFic, *TpTGereFic;

/*--------------- FUNCTIONS: --------------*/

/* Prototypes des fonctions a la charge de l'utilisateur */
/* Lecture d'un Item dans le fichier */
typedef FIC_enum_retour (*TLireItem)( FILE   *Fichier,
                                      TParam *Param,
                                      TItem  *ItemRetour,
                                      int    *Taille_lue);

/* Ecriture d'un Item dans le fichier */
typedef FIC_enum_retour (*TEcrireItem)( FILE   *Fichier,
                                        TItem  *Item,
                                        TParam *Param,
                                        int    *Taille_ecrite);

/* Recherche d'un Id dans l'Item */
typedef FIC_enum_retour (*TLireIdVal)( TItem  *Item,
                                       long   Position,
                                       TParam *Param,
                                       TId    *Id,
                                       TVal   *Val);

/* Remplir l'Item pour l'‚crire */
typedef FIC_enum_retour (*TEcrireIdVal)( TItem  *Item,
                                         long   Position,
                                         TParam *Param,
                                         TId    *Id,
                                         TVal   *Val);

/* Traitement d'un item */
typedef FIC_enum_retour (*TTraiterIdVal)( TpTGereFic        GereFic,
                                          FIC_enum_retour   etat,
                                          TItem             *Item,
                                          long              Position,
                                          TParam            *Param,
                                          TId               *Id,
                                          TVal              *Val);

/* La fonction de comparaison est bas‚e sur le fonctionnement */
/* de la fonction de comparaison des chaines de caractŠres : strcmp() */
typedef int (*TComparer)( TId    *Id,
                          TId    *St_Id,
                          TParam *Param);

/*--------------- TYPEDEF: --------------*/

typedef struct Struct_GereFic
{
   /* Le fichier de travail en cours */
   FILE          *fichier;

   /* Les caracteristiques du fichier */
   int           TailleItem;
   FIC_enum_tri  mode_tri;

   /* Les methodes a utiliser sur ce fichier */
   TLireItem     LireItem;
   TEcrireItem   EcrireItem;
   TLireIdVal    LireIdVal;
   TEcrireIdVal  EcrireIdVal;
   TComparer     Comparer;

   /* Les buffers temporaires de travail */
   TItem         *Item;
   TId           *Id;
   TVal          *Val;

   /* La gestion cyclique (0:une passe, n:nombre de cycle a effectuer avant de rendre la main) */
   /* non impl‚ment‚ */
   unsigned int  cycle;

   /* L'indice de l'item en cours et l'offset de position actuel */
   long          position;
   long          offset;

   /* Le type de recherche */
   FIC_enum_rech recherche;

   /* Etat de la configuration (TRUE:les informations de la structures sont remplies) */
   boolean       conf;

   /* Etat du traverse (TRUE: un traverse est en cours) */
   boolean       traverseEnCours;
} TStruct_GereFic;

typedef struct
{
   /* Ces donnees ne sont pas prevues pour etre utilisees a l'exterieur */
   /* du module FIC_GERE, NE PAS MODIFIER position et offset */
   /* L'indice de l'item en cours et l'offset de position actuel demande */
   long          position;
   long          offset;
} TInfoFlux;

/**/

/* Cette fonction positionne les champs par default */
/* elle doit ˆtre suivie par l'appel a Fichier_configuration() */
EXPORT boolean WINAPI Fichier_Charger( TGereFic     *config,
                                      char         *nom_fichier,
                                      char         *attribut, /* attribut de fopen() soit "r+b" ou "r+t" */
                                      size_t       taille_elem,
                                      TLireItem    LireItem,
                                      TEcrireItem  EcrireItem);

                                      /* Cette fonction permet de charger un nouveau fichier ayant les memes
                                      * caract‚ristiques que le pr‚cedent.
                                      * Le pr‚c‚dent fichier est ferm‚.
*/
EXPORT boolean WINAPI Fichier_Echanger( TGereFic     *config,
                                       char         *nom_fichier,
                                       char         *attribut); /* attribut de fopen() soit "r+b" ou "r+t" */

EXPORT boolean WINAPI Fichier_Configurer( TGereFic     *config,
                                         FIC_enum_tri mode_tri,
                                         unsigned int cycle,
                                         TLireIdVal   LireIdVal,
                                         TEcrireIdVal EcrireIdVal,
                                         TComparer    Comparer,
                                         TItem        *Item,
                                         TId          *Id,
                                         TVal         *Val);

                                         /* cette fonction ferme le fichier mais n'efface pas sa configuration.
                                         * Il est donc possible de la r‚utiliser en appelant la fonction
                                         * Fichier_Echanger
*/
EXPORT boolean WINAPI Fichier_Fermer( TGereFic *config);

EXPORT FIC_enum_retour WINAPI Fichier_LireId( TGereFic *config,
                                             TId      *Id,
                                             TParam   *param,
                                             long      *position,
                                             TItem    **Item,
                                             TVal     **Val);

EXPORT FIC_enum_retour WINAPI Fichier_LirePos( TGereFic *config,
                                              long      position,
                                              TParam   *Param,
                                              TItem    **Item,
                                              TId      **Id,
                                              TVal     **Val);

EXPORT FIC_enum_retour WINAPI Fichier_FormatterId( TGereFic *config,
                                                  TId      *Id,
                                                  TParam   *param,
                                                  long     position,
                                                  TItem    *Item,
                                                  TVal     *Val);
/* La taille de l'‚l‚ment … remplac‚ doit ˆtre g‚r‚ par l'utilisateur */
/* Aucune v‚rification n'est faite par le module */
EXPORT FIC_enum_retour WINAPI Fichier_RemplacerId( TGereFic *config,
                                                  TId      *Id,
                                                  TParam   *Param,
                                                  long      *position,
                                                  TItem    *Item);
/* La taille de l'‚l‚ment … remplac‚ doit ˆtre g‚r‚ par l'utilisateur */
/* Aucune v‚rification n'est faite par le module */
EXPORT FIC_enum_retour WINAPI Fichier_RemplacerPos( TGereFic *config,
                                                   TParam   *Param,
                                                   long      position,
                                                   TItem    *Item);
/* Seul les ajouts … la fin du fichier sont impl‚ment‚s */
EXPORT FIC_enum_retour WINAPI Fichier_AjouterId( TGereFic *config,
                                                TId      *Id,
                                                TItem    *Item,
                                                TParam   *Param,
                                                long      *position);
/* Seul les ajouts … la fin du fichier sont impl‚ment‚s */
#define Fichier_AjouterPos(c,p,i,pa) Fichier_AjouterPosition( c,p,i,pa, NULL)
EXPORT FIC_enum_retour WINAPI Fichier_AjouterPosition( TGereFic *config,
                                                      long     position,
                                                      TItem    *Item,
                                                      TParam   *Param,
                                                      long     *position_ecrite);
EXPORT FIC_enum_retour WINAPI Fichier_Traverse( TGereFic         *GereFic,
                                               TTraiterIdVal    Function,
                                               void             *Param);

/* Memorisation et restitution des informations de l'item actuel demande */
/* Ces deux fonctions permettent de restaurer les parametres associes */
/* a la gestion du fichier qui ont ete memorises */
/* InfoFlux contient en retour les informations de flux a sauvegarder */
EXPORT FIC_enum_retour WINAPI Fichier_Memoriser( TGereFic  *GereFic,
                                                TInfoFlux *InfoFlux);
/* InfoFlux contient les information de flux qui sont a restaurer */
EXPORT FIC_enum_retour WINAPI Fichier_Restaurer( TGereFic  *GereFic,
                                                TInfoFlux *InfoFlux);

#undef EXPORT
#undef I
#undef INIT
#endif
