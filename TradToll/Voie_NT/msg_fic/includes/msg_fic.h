/*------   (v) 1997 CS-Route  ------------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: FICHIER de MESSAGES
* FICHIER: msg_fic.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Gestion des fichiers de messages et des fichiers DATE.
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des fonctions et constante sur la
*        manipulation des fichiers de messages
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Outils_C/Msg_files/Includes/msg_fic.h_v  $
 * 
 *    Rev 1.0   13 Dec 1999 16:51:22   afx
 *  
 * 
 *    Rev 1.3   Jul 21 1997 10:31:56   HMO
 *  
 * 
 *    Rev 1.2   Jul 03 1997 14:37:02   DPI
 * Ajout de la fonction TerminerMessageFichier
 * 
 *    Rev 1.1   Jun 20 1997 17:32:10   HMO
 *  
 * 
 *    Rev 1.0   May 16 1997 11:53:16   ANA
 * Reprise EMI_ACT.H
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef MSG_FIC_H
#define MSG_FIC_H

/*--------------- INCLUDES: ---------------*/

//#include <fic_conf.h>
#include <fic_gere.h>

#ifdef MSG_FIC_EXPORTS
#include "public.h"
#else
#include "export.h"
#endif


/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define FICHIER_TAILLE_NOM             MAX_PATH
#define FICHIER_TAILLE_EXT             5  /* comprenant \\ et \0 */

#define FICHIER_SANS_EXTENSION         -1

/* format des chaines de caracteres specifiques aux noms des repertoires et fichiers */
#define FICHIER_FORMAT_REPERTOIRE      "%03.3d"
#define FICHIER_FORMAT_SOUS_REP        "%06.6ld"
#define FICHIER_FORMAT_NOM             "%08.8ld"
#define FICHIER_FORMAT_EXTENSION       "%03.3d"

/* identificateur attendus dans les fichiers de date */
#define ID_PREMIER_FICHIER        "PREMIER"
#define ID_DERNIER_FICHIER        "DERNIER"
#define ID_REMARQUE               "REMARQUE"

/* taille des buffers pour la copie de fichier */
#define FICHIER_BUFFER_SIZE          512

/*----------------TYPEDEF:----------------*/
typedef enum
{
   DATE_PREMIER =1,
   DATE_DERNIER,
   DATE_LES_DEUX,

}enum_fic_ident;

typedef enum
{
   EMI_PAS_ERREUR = CSR_OK,
   EMI_ERREUR_FICHIER,
   EMI_ERREUR_CONTENU
}enum_erreur_rech;

/* -------------- FONCTIONS: --------------*/
EXPORT boolean FermerMessageFichier( TGereFic *DateFile,
                                         long *FileNumber,
                                         FILE **MsgFile);
EXPORT FILE * CreerFichier(long num_fichier,
                              int extension,
                              char * path_fichier);

EXPORT boolean EffacerFichier(long num_fichier,
                              int extension);

EXPORT boolean RenommerFichier(char *path_fichier);

EXPORT boolean TerminerFichier(FILE *fichier,
                                  char *path_fichier,
                                  boolean flag_nom);

EXPORT void ObtenirFichier(long num_fichier, char *path_fichier);
EXPORT void ObtenirFichierSansNom(long num_fichier,char * path_fichier);
EXPORT void ObtenirFichierSansDernierRepertoire(long num_fichier,char * path_fichier);

EXPORT boolean RechercherPremierMessageFichier( TGereFic *DateFile,
                                                   long *FileNumber,
                                                   FILE **MsgFile);

EXPORT boolean CreerPremierMessageFichier( TGereFic *DateFile,
                                              long *FileNumber,
                                              FILE **MsgFile);

EXPORT boolean EcrireMessageFichier( char *Msg,
                                        int TailleMsg,
                                        FILE *MsgFile,
                                        long *TailleFile);

EXPORT boolean ChangerMessageFichier( TGereFic *DateFile,
                                         long *FileNumber,
                                         FILE **MsgFile);

EXPORT boolean ChangerJourMessageFichier( TGereFic *DateFile,
                                             long *FileNumber,
                                             FILE **MsgFile);

EXPORT boolean RechercheFichierInitEmettre( char *FichierColdStart,
                                               long *ptr_num_fichier);

EXPORT boolean RechercheFichierInitCreer( long *num_fichier,
                                             int  *extension,
                                             char *path_fichier);

EXPORT short int RechercherFichierParDate(SYSTEMTIME date,
                                             long *num_debut,
                                             long *num_fin,
                                             int iteration);

EXPORT boolean RechercherFichier(long num_fichier,
                                    int extension,
                                    char *path_fichier);

EXPORT boolean CopierFichierDisque(char *FilePath_source, char *FileName_dest);

EXPORT boolean OuvrirFichierDate(TGereFic *DateFile,
                                   SYSTEMTIME *p_date,
                                   char *path_fichier);

EXPORT boolean EffacerFichierDate(SYSTEMTIME date);

EXPORT void ObtenirFichierDateJour(SYSTEMTIME ,char* path_fichier);
EXPORT void ObtenirFichierDateMois(SYSTEMTIME ,char* path_fichier);
EXPORT void ObtenirFichierDateAnnee(SYSTEMTIME ,char* path_fichier);

EXPORT boolean EcrireFichierDate(TGereFic *DateFile,
                                      enum_fic_ident ident,
                                      long numero);

EXPORT boolean FermerFichierDate(TGereFic *DateFile);


/*------------------VARIABLES:-----------------*/

#include <undef.h>
#endif
