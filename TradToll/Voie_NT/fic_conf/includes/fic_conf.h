/*------   (v) 1998 CS-Route -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER DE CONFIGURATION
* FICHIER: FIC_CONF.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Acces aux fichiers de configuration
* --------------------------------------------------------------------
* DESCRIPTION: Lecture des fichier de configuration
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fic_conf/Includes/fic_conf.h_v  $
 * 
 *    Rev 1.2   Jan 20 1999 11:29:36   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:41:48   bph
 *  
 * 
 *    Rev 1.7   Jul 28 1997 16:40:54   DPI
 * Gestion de la version de la librairie
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef FIC_CONF_H
#define FIC_CONF_H

/*--------------- DEFINES: ----------------*/

#define           CONF_MAX_ID       30
#define           CONF_MAX_VAL      255
#define           CONF_MAX_ITEM     ( CONF_MAX_ID + 30 + CONF_MAX_VAL)

#define           COMMENTS     '#'

typedef struct StructValeur
               {
                  char              Id[ CONF_MAX_ID];
                  unsigned int      Iteration;
                  char              Contenu[ CONF_MAX_VAL];
               } TValeur;

typedef char   TConfItem[ CONF_MAX_ITEM];

#include "fic_gere.h"


/*--------------- INCLUDES: ---------------*/

#ifdef FIC_DEF
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- TYPEDEF ---------------*/

/*--------------- VERSION ---------------*/
#define FIC_CONF_VERSION               "1.00"

/*--------------- FUNCTIONS: --------------*/
/* Item, Id et Val sont des zones temporaires pour le fonctionnement
 * qui doivent ˆtre valides tout le temps de l'utilisation
 * Attributs est le format standard du fopen , ex: "r+t"
 */
EXPORT boolean WINAPI InitFichierConfig( TGereFic    *GereFic,
                                         char        *NomFichier,
                                         char        *Attributs,
                                         char        *Item,
                                         TValeur     *Id,
                                         TValeur     *Val);

/* remplace le fichier en cours par un autre */
EXPORT boolean WINAPI EchangerFichierConfig( TGereFic *GereFic,
                                             char     *NomFichier,
                                             char     *Attributs);
EXPORT boolean WINAPI FermerFichierConfig( TGereFic *GereFic);

/* Val est un TValeur qui est pass‚ par adresse */
EXPORT FIC_enum_retour WINAPI LireFichierConfig( TGereFic *GereFic,
                                                 char     *Identifiant,
                                                 int      Iteration,
                                                 TValeur  *Val);

/* Remplace dans un id la valeur
 * ATTENTION, manier avec pr‚caution, VOUS devez garantir que
 * ce que vous ecrivez … la meme longueur que ce que vous ecraser
 */
EXPORT FIC_enum_retour WINAPI RemplacerFichierConfig( TGereFic *GereFic,
                                                      char     *Id,
                                                      int      Iteration,
                                                      char     *Contenu);
/* ajoute a la fin du fichier */
EXPORT FIC_enum_retour WINAPI AjouterFichierConfig( TGereFic *GereFic,
                                                    char     *Id,
                                                    int      Iteration,
                                                    char     *Contenu);
/* ajoute un commentaire … la fin du fichier */
/* le caractŠre de d‚but et de fin de fichier est ajout‚ automatiquement */
EXPORT FIC_enum_retour WINAPI CommentaireFichierConfig( TGereFic *GereFic,
                                                        char     *Commentaire);

#undef EXPORT
#undef I
#undef INIT
#endif
