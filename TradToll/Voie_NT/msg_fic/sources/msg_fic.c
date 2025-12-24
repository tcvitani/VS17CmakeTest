/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: FICHIER de MESSAGES
* FICHIER: msg_fic.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fonctions concernant les fichiers o— sont stock‚s les messages
* --------------------------------------------------------------------
* DESCRIPTION: Gestion des fichiers de messages et des fichiers date :
*    1 - Creation, ecriture, destruction, copie, recherche et renommage
*        des fichiers de messages :
*        * … partir d'un numero de fichier
*        * … partir d'une date
*    2 - Gestion des fichiers DATE de type TGereFic
* --------------------------------------------------------------------
* A NOTER
*        On ne gŠre pas de rebouclage, sur d‚cision RLP
*        Donc, les fichiers varient de 00000001 … 99999999, et ne
*        repassent pas … 0
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Outils_C/Msg_files/Sources/msg_fic.c_v  $
 * 
 *    Rev 1.0   13 Dec 1999 16:53:42   afx
 *  
 * 
 *    Rev 1.17   21 Nov 1997 10:19:54   HMO
 * Scrutation complète du fichier date seulement à son ouverture
 * 
 *    Rev 1.16   23 Sep 1997 15:45:56   HMO
 * Modifications effectuées sur site
 * 
 *    Rev 1.15   Aug 06 1997 15:39:54   DPI
 * Correction de l'ecriture de 2 zeros à la fin
 * de certains fichiers
 * 
 *    Rev 1.14   Jul 28 1997 11:25:54   HMO
 * Correction sur les FIC_EstRepertoireVide
 * Ajout d'un FIC_fflush
 * 
 *    Rev 1.13   Jul 24 1997 14:29:56   HMO
 *  
 * 
 *    Rev 1.12   Jul 21 1997 10:31:54   HMO
 * Gestion de la purge sur tout les fichiers
 * 
 *    Rev 1.11   Jul 16 1997 15:43:00   ANA
 * Modification pour obtenir la taille d'un fichier de messages,
 * Taille limitée à 29Ko pour transfert pr FTP,
 * ajout du fichier include "stdcsee.h"
 * 
 *    Rev 1.10   Jul 03 1997 14:37:02   DPI
 * Ajout de la fonction TerminerMessageFichier
 * 
 *    Rev 1.9   Jun 24 1997 14:36:58   HMO
 *  
 * 
 *    Rev 1.8   Jun 20 1997 17:32:08   HMO
 * Ajout du nom de fichier Cold Start a rechercher
 * 
 *    Rev 1.7   Jun 20 1997 11:08:28   HMO
 * Ajout d'un setbuf()
 * 
 *    Rev 1.6   Jun 12 1997 11:13:44   HMO
 * Gestion correcte pour un message de taille 0
 * 
 *    Rev 1.5   Jun 11 1997 12:26:00   DPI
 * Correction du codage d'une longueur nulle
 * 
 *    Rev 1.4   Jun 11 1997 12:23:28   DPI
 * Ajout du codage de la taille en tete des 
 * messages
 * 
 *    Rev 1.3   Jun 06 1997 12:49:20   HMO
 * Ouverture du fichier au démarrage sans effacer ses données
 * 
 *    Rev 1.2   Jun 04 1997 10:49:04   ANA
 * Ecriture de "\0" au lieu de "0"
 * 
 *    Rev 1.1   May 20 1997 16:18:40   ANA
 * Corrections dans la fonction CopierFichierDisque()
 *
 *    Rev 1.0   May 16 1997 11:51:44   ANA
 * Reprise de EMI_ACT.C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <io.h>
//#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <dos.h>
#include <sys\stat.h>
#include <limits.h>

#include <csrlc32.h>

#include <run.h>
#include <reg.h>

//#include <pcs.h>

#include "err.h"
#include "str.h"
#include "fic.h"

/* lecture d'un fichier de configuration */
//#include <fic_conf.h>

#define LOC_DEF
#include "msg_fic.h"
#include "msg_fic_glob.h"
#undef LOC_DEF

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define MAX_LONGUEUR          10

#define PCS_REG_KEYn_CFG	"PCS Communication\\"
#define PCS_REG_KEYn_DIR	"Directories\\"

#define PCS_REG_KEYv_DISK				"disk_path"
#define PCS_REG_KEYv_BACKUP				"backup_path"
#define PCS_REG_KEYv_LOCAL_DRIVE		"local_drive"
/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/
PRIVATE BOOL WINAPI	MsgFicReadRegistry (void);

PRIVATE long ObtenirRepertoire( long num_fichier);
PRIVATE long ObtenirSousRepertoire( long num_fichier);

/*--------------- VARIABLES: ---------------*/

PRIVATE struct_directories	DIR;



/*--------------- CODE: ---------------*/
extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:

			// Lecture base de registre
			if (!MsgFicReadRegistry ())
				exit (INST_INIT_ERR_REGISTRE);
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  
            break;          

         default:             
            break;     
    }      
    
    return TRUE; 

    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL WINAPI MsgFicReadRegistry ( void )
 * PARAMETERS: void : 
 * RETURN    : TRUE si lecture registre OK, FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lecture de parametres dans le registre.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL WINAPI	MsgFicReadRegistry (void)
{
	DWORD                 dwLen;
	char pcKey[MAX_PATH];

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE,
		CSR_REG_KEYn_CONFIG, PCS_REG_KEYn_CFG, PCS_REG_KEYn_DIR);

	// Backup path
	dwLen = sizeof(DIR.path_backup);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, PCS_REG_KEYv_BACKUP, DIR.path_backup, &dwLen ) != ERROR_SUCCESS )
		return FALSE;

	// Disk path
	dwLen = sizeof(DIR.path_disk);
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, PCS_REG_KEYv_DISK, DIR.path_disk, &dwLen ) != ERROR_SUCCESS )
		return FALSE;

	// Local drive

	dwLen = sizeof(DIR.local_drive);

	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, PCS_REG_KEYv_LOCAL_DRIVE, DIR.local_drive, &dwLen ) != ERROR_SUCCESS )
	{
		return FALSE;

		if (STR_strlen( MAX_PATH, DIR.local_drive) == 0)
			return FALSE;
	}
	
	return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean FermerMessageFichier( TGereFic *DateFile,
*                                        long *FileNumber,
*                                        FILE **MsgFile)
* PARAMETRES: Fichier "jj.txt"
*             Numero de fichier
*             Fichier de messages
* RETOUR: etat du changement
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Fermeture des fichiers de stockages
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean FermerMessageFichier( TGereFic *DateFile,
                                        long *FileNumber,
                                        FILE **MsgFile)
{
   char path_fichier[MAX_PATH];

   /* ecriture de l'octet "0" et fermeture du fichier */
   /* le fichier doit ˆtre ouvert */
   /* obtention du chemin du fichier sans l'extension */
   ObtenirFichier(*FileNumber,path_fichier);

   if( TerminerFichier( *MsgFile, path_fichier,TRUE) == FALSE )
      return FALSE;

   /* fermer le fichier date en cours */
   if( !FermerFichierDate( DateFile))
     return FALSE;

   return TRUE;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean RechercherPremierMessageFichier( TGereFic *DateFile,
*                                                   long *FileNumber,
*                                                   FILE **MsgFile)
* PARAMETRES: Fichier "jj.txt"
*             Numero de fichier
*             Fichier de messages
* RETOUR: fichier date trouve
*         fichier de message trouve
*         NULL si no trouve
*         Etat de la recherche: TRUE /FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche du premier fichier dans lequel seront ecrits les
*       messages lors de l'initialisation :
*    1- Recherche du premier repertoire existant XXX … partir de c:\BACKUP\999
*    2- Dans XXX, recherche du premier sous_repertoire existant YYY …
*       partir de c:\BACKUP\XXX\XXX999
*    3- Dans XXXYYY, recherche du premier fichier XXXYYYZZ.*
*       … partir de la fin. Si un fichier est trouve sans extension,
*       on le renomme XXXYYYZZ.000.
*    4- Ouverture du fichier de num‚ro imm‚diatement sup‚rieur, avec creation
*       des repertoires si n‚cessaire;
*    5- Si le fichier "date" C:\BACKUP\aaaa\mm\jj.txt n'existe pas,
*       creation en lecture/ecriture et ecriture des items PREMIER et DERNIER,
*       Si le fichier existe, recherche de la derniŠre it‚ration valide i,
*       creation de PREMIER[i+1] et DERNIER[i+1].
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean RechercherPremierMessageFichier( TGereFic *DateFile,
                                                   long *FileNumber,
                                                   FILE **MsgFile)
{
   boolean        status;
   char           path_fichier[MAX_PATH];
   int            extension;

   /* Initialisation du compteur de position */
   Position_fichier_date = 0;

   /* recherche du dernier fichier de message*/
   status = RechercheFichierInitCreer(FileNumber,&extension,path_fichier);
   if( status == FALSE )
      return(FALSE);

   if( extension == FICHIER_SANS_EXTENSION)
   {
      /* terminer le fichier par le caractere "0"
       * et le renommer
       */
     *MsgFile = FIC_fopen(path_fichier,"a+b");
     if( *MsgFile == NULL )
        return FALSE;

     if( TerminerFichier(*MsgFile, path_fichier, FALSE) == FALSE )
        return FALSE;
   }

   /* Creation et ouverture du fichier de numero sup‚rieur */
   *FileNumber = *FileNumber +1;
   *MsgFile = CreerFichier(*FileNumber, FICHIER_SANS_EXTENSION, path_fichier);
   if( *MsgFile == NULL )
      return(FALSE);

   /* creation et/ou ouverture du fichier date correspondant */
   GetLocalTime(&date);
   ObtenirFichierDateJour(date, path_fichier);

   /* creation/ouverture du fichier */
   if( ! OuvrirFichierDate(DateFile,&date,path_fichier) )
      return FALSE;

   /* ecriture des identificateurs PREMIER et DERNIER */
   status = EcrireFichierDate(DateFile,DATE_LES_DEUX,*FileNumber);
   if( status == FALSE )
      return(FALSE);

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean CreerPremierMessageFichier( TGereFic *DateFile,
*                                              long *FileNumber,
*                                              FILE **MsgFile)
* PARAMETRES: Fichier "jj.txt"
*             Numero de fichier
*             Fichier de messages
* RETOUR: etat de la creation des fichiers date et numero
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Lors d'un COLD START :
*   1- Destruction des repertoires c:\BACKUP\*\*\*
*   2- Creation du repertoire C:\BACKUP
*   3- Creation du repertoire C:\BACKUP\000\000000
*   4- Creation du fichier numero 1
*   5- Creation du fichier date associ‚ C:\BACKUP\aaaa\mm\jj.txt
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean CreerPremierMessageFichier( TGereFic *DateFile,
                                              long *FileNumber,
                                              FILE **MsgFile)
{
   char  path_fichier[MAX_PATH];
   boolean status;

   /* Initialisation du compteur de position */
   Position_fichier_date = 0;

   /* destruction du repertoire DISK */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_disk, NULL, NULL);
   FIC_deltree(path_fichier);

   /* Destruction du repertoire BACKUP */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);
   FIC_deltree(path_fichier);

   /* creation of backup directory */
   FIC_CreatePath(path_fichier);

   /* Creation et ouverture du fichier de numero 1 */
   *FileNumber = 1;
   *MsgFile = CreerFichier(*FileNumber, FICHIER_SANS_EXTENSION, path_fichier);
   if( *MsgFile == NULL )
      return(FALSE);

   /* creation et/ou ouverture du fichier date correspondant */
   GetLocalTime(&date);
   ObtenirFichierDateJour(date, path_fichier);

   /* creation/ouverture du fichier */
   if( ! OuvrirFichierDate(DateFile,&date,path_fichier) )
      return FALSE;

   /* ecriture des identificateurs PREMIER et DERNIER */
   status = EcrireFichierDate(DateFile,DATE_LES_DEUX,*FileNumber);
   if( status == FALSE )
      return(FALSE);

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean EcrireMessageFichier( char *Msg,
*                                        int TailleMsg,
*                                        FILE *MsgFile,
*                                        long *TailleFile)
* PARAMETRES:
*            - le message a ecrire ( peut ˆtre NULL si taille =0)
*            - la taille du message a ecrire
*            - le fichier dans lequel on ecrit
*            - la taille du fichier aprŠs ecriture ( peut ˆtre NULL)
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean EcrireMessageFichier( char *Msg,
                                        int TailleMsg,
                                        FILE *MsgFile,
                                        long *TailleFile)
{
   int      nb_objet;
   int      file_handle;

   /* ecrire la taille du message mˆme si 0 */
   nb_objet = (int)fwrite( &TailleMsg, sizeof(TailleMsg), 1, MsgFile);

   if( nb_objet != 1 )
      return FALSE;

   /* ecrire le message si sa taille est non nulle */
   if( TailleMsg == 0)
      return TRUE;

   /* si sa taille est non nulle, mais que le ptr est NULL -> pb */
   if( MsgFile == NULL )
      return FALSE;

   nb_objet = (int)fwrite( Msg, TailleMsg, 1, MsgFile);

   if( nb_objet != 1 )
      return FALSE;

   if( TailleFile != NULL)
   {
      /* nouvelle Taille du fichier */
      file_handle = _fileno (MsgFile);
      *TailleFile = FIC_filelength(file_handle);
   }

   FIC_fflush( MsgFile);

   return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean ChangerMessageFichier( TGereFic *DateFile,
*                                         long *FileNumber,
*                                         FILE **MsgFile)
* PARAMETRES: Fichier "jj.txt"
*             Numero de fichier
*             Fichier de messages
* RETOUR: etat du changement
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Changement de fichier :
*    1- Fermeture le fichier en ajoutant l'octet "0" a la fin du fichier,
*    2- Renommage du fichier en .000,
*    3- Creation du fichier suivant,
*    4- Mise … jour du fichier jj.txt pour l'identificateur DERNIER.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean ChangerMessageFichier( TGereFic *DateFile,
                                         long *FileNumber,
                                         FILE **MsgFile)
{
   char path_fichier[MAX_PATH];
   boolean status;

   /* ecriture de l'octet "0" et fermeture du fichier */
   /* le fichier doit ˆtre ouvert */
   /* obtention du chemin du fichier sans l'extension */
   ObtenirFichier(*FileNumber,path_fichier);

   if( TerminerFichier( *MsgFile, path_fichier,TRUE) == FALSE )
      return FALSE;

   /* creation du fichier suivant  et mise … jour du fichier jj.txt */
   *FileNumber = *FileNumber + 1;
   *MsgFile = CreerFichier(*FileNumber, FICHIER_SANS_EXTENSION, path_fichier);
   if( *MsgFile == NULL )
      return(FALSE);

   /* ecriture de l'identificateur  DERNIER */
   status = EcrireFichierDate(DateFile,DATE_DERNIER,*FileNumber);
   if( status == FALSE )
      return(FALSE);

   return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean ChangerJourMessageFichier( TGereFic *DateFile,
*                                             long *FileNumber,
*                                             FILE **MsgFile)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean ChangerJourMessageFichier( TGereFic *DateFile,
                                             long *FileNumber,
                                             FILE **MsgFile)
{
   char path_fichier[MAX_PATH];
   boolean status;

   /* ecriture de l'octet "0" et fermeture du fichier */
   /* le fichier doit ˆtre ouvert */
   /* obtention du chemin du fichier sans l'extension */
   ObtenirFichier(*FileNumber,path_fichier);

   if( TerminerFichier( *MsgFile, path_fichier, TRUE) == FALSE )
      return FALSE;

   /* fermer le fichier date en cours */
   if( !FermerFichierDate( DateFile))
     return FALSE;

   /* creation du fichier suivant  et mise … jour du fichier jj.txt */
   *FileNumber = *FileNumber + 1;
   *MsgFile = CreerFichier(*FileNumber, FICHIER_SANS_EXTENSION, path_fichier);
   if( *MsgFile == NULL )
      return(FALSE);

   /* creation et/ou ouverture du fichier date correspondant */
   GetLocalTime(&date);
   ObtenirFichierDateJour(date, path_fichier);

   /* creation/ouverture du fichier */
   if( ! OuvrirFichierDate(DateFile,&date,path_fichier) )
      return FALSE;

   /* ecriture des l'identificateurs PREMIER et/ou DERNIER */
   status = EcrireFichierDate(DateFile,DATE_LES_DEUX,*FileNumber);
   if( status == FALSE )
      return(FALSE);

   return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC FILE * CreerFichier( long num_fichier, int extension,
*                                       char *nom_fichier)
* PARAMETRES: numero du fichier a cr‚er
*             extension du fichier
*             nom du fichier a creer
* RETOUR: pointeur sur fichier cr‚e
*         NULL si pas de creation
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Cr‚e un fichier dans les rep et sous-rep a partir de son numero,
*       avec ou sans extension : c:\BACKUP\XXX\XXXYYY\XXXYYYZZ.*
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED FILE * CreerFichier(long num_fichier, int num_extension, char *path_fichier)
{
   char				   tempformat[MAX_PATH]	;
   char				   ficname[FICHIER_TAILLE_NOM];
   char				   extname[FICHIER_TAILLE_EXT];
   FILE                *fichier_cree;

   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   /* obtention des repertoire et sous repertoire */
   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_SOUS_REP, ObtenirSousRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   /* si repertoire non existant, le cr‚er */
   if( _access(path_fichier,0) == -1 )
      FIC_CreatePath(path_fichier);

   /* obtention du nom ascii a partir d'un long */
   sprintf_s(ficname,sizeof(ficname), FICHIER_FORMAT_NOM, num_fichier);
   
   /* extension du fichier */
   if( num_extension != FICHIER_SANS_EXTENSION )
   {
	   sprintf_s(extname, sizeof(extname), FICHIER_FORMAT_EXTENSION, num_extension);
	  FIC_makepath(path_fichier, path_fichier, NULL, ficname, extname);
   }
   else
	   FIC_makepath(path_fichier, path_fichier, NULL, ficname, NULL);


   /* creer le fichier */
   fichier_cree = FIC_fopen(path_fichier,"a+b");
   if( fichier_cree != NULL )
   {
      FIC_setbuf( fichier_cree, NULL);
      return( fichier_cree );
   }
   else
      return ( NULL );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean EffacerFichier( long num_fichier, int extension)
* PARAMETRES: numero du fichier a effacer
*             extension du fichier
* RETOUR: etat de l'effacement : TRUE/FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Efface un fichier a partir de son numero et de son extension
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean EffacerFichier(long num_fichier, int num_extension)
{
   char   path_fichier[MAX_PATH];
   char   nom_extension[FICHIER_TAILLE_EXT];
   //char * ptr_path;
   boolean report = FALSE;
   struct _finddata_t  ffblk;
   intptr_t  hfind;
   short int done;
   char  tempformat[MAX_PATH];
   char  FullPath[MAX_PATH];
   char  ficname[FICHIER_TAILLE_NOM];

   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_SOUS_REP, ObtenirSousRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   strcpy_s(FullPath, sizeof(FullPath), path_fichier);

	// ficname contains xxxyyyzz.nnn
   sprintf_s(ficname, sizeof(ficname), FICHIER_FORMAT_NOM, num_fichier);

   /* extension du fichier s'il y en a une */
   if( num_extension == FICHIER_SANS_EXTENSION)
	   sprintf_s(nom_extension, sizeof(nom_extension), "*");
   else
	   sprintf_s(nom_extension, sizeof(nom_extension), FICHIER_FORMAT_EXTENSION, num_extension);

   //STR_strcat(MAX_PATH,path_fichier,nom_extension);
/*
   STR_strcat(MAX_PATH,ficname, ".");
   STR_strcat(MAX_PATH,ficname, nom_extension);
*/	
   FIC_makepath(path_fichier, FullPath, NULL, ficname, nom_extension);

   /* recherche de tous les fichier … effacer */
   if ((hfind = FIC_FindFirst(path_fichier,&ffblk)) >= 0)
   {	
       done = 0;
	   while( done == 0 )
	   {
			FIC_makepath(path_fichier, FullPath, NULL, ffblk.name, NULL);
			if( FIC_remove(path_fichier) == 0)
			report = TRUE;
			done = FIC_FindNext(hfind,&ffblk);
		}
	   FIC_FindClose(hfind);
   }

   if( FIC_EstRepertoireVide( FullPath))
   {
      FIC_rmdir( FullPath);

      /* Suppression du repertoire pr‚c‚dant s'il est vide */
      ObtenirFichierSansDernierRepertoire(num_fichier, path_fichier);
      if( FIC_EstRepertoireVide( path_fichier))
      {
         FIC_rmdir( path_fichier);
      }
   }

   return( report );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean RenommerFichier( char *nom_fichier)
* PARAMETRES: fichier : fichier a renommer
* RETOUR:   Etat du renommage TRUE /FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Renomme un fichier par incrementation de son extension
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean RenommerFichier(char * nom_fichier)
{
   char                nouveau_nom[MAX_PATH];
   int                 num_extension;
   //char                *ptr_extension;
   char					drive[MAX_PATH];
   char					path[MAX_PATH];
   char					name[MAX_PATH];
   char					extension[MAX_PATH];

   STR_strcpy( MAX_PATH, nouveau_nom, nom_fichier);

   /* recherche de l'extension : recherche du point */
   FIC_splitpath(nouveau_nom, drive, path, name, extension);
   
   //ptr_extension = strchr( nouveau_nom, '.');

   if(strcmp("", extension) != 0)
   //if( ptr_extension != NULL)
   {
      num_extension = atoi( extension + 1);
      num_extension ++;
   }
   else
   {
      //ptr_extension = nouveau_nom + STR_strlen( MAX_PATH, nouveau_nom);
      num_extension = 0;
   }

   sprintf_s(extension, sizeof(extension), "%03d", num_extension);

   FIC_makepath(nouveau_nom, drive, path, name, extension);

   if( rename( nom_fichier, nouveau_nom) == -1 )
      return (FALSE );

   return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean TerminerFichier(FILE *fichier, char *path_fichier,
*                                  boolean flag_renommage)
* PARAMETRES: pointeur sur le fichier ,
*             chemin complet du fichier
*             flag sur renammage
* RETOUR: etat : TRUE/FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Ecrire … la fin du fichier le caractere "\0" et le fermer;
*       Renommer le fichier si necessaire.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean TerminerFichier(FILE *fichier,char *path_fichier,boolean flag_renomme)
{
   //char *ptr_path;
	char extension[MAX_PATH];


   if( EcrireMessageFichier( NULL, 0, fichier, NULL) == FALSE )
      return FALSE;

   if( FIC_fclose (fichier) != 0 )
      return FALSE;

   if( flag_renomme == FALSE)
   {
      /* recherche de l'extension */
      /* seuls les fichiers SANS extension sont renommes */
      //ptr_path = strchr(path_fichier,'.');
	  FIC_splitpath(path_fichier, NULL, NULL, NULL,extension);	
      //if( ptr_path != NULL )
	  if (strcmp("", extension) != 0)
         return TRUE;
   }

   /* renommer en .000 */
   if( RenommerFichier(path_fichier) == FALSE )
      return FALSE;

   return TRUE;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean OuvrirFichierDate( TGereFic *GereFic_date,
*                                     struct dostime_t date,
*                                     char *path_fichier)
* PARAMETRES: date du fichier
* RETOUR: Fichier DATE avec la structure TGereFic,
*         chemin complet du fichier DATE cr‚e ou ouvert,
*         etat de l'effacement : TRUE/FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Ouverture d'un fichier de type TGereFic a partir de sa dat
*       Creation s'il n'existe pas ;
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean OuvrirFichierDate(TGereFic *GereFic_date,
                                   SYSTEMTIME *p_date,
                                   char *path_fichier)
{
   //char *    ptr_path;
   char		 ficname[MAX_PATH];
   char		 temp[MAX_PATH];
   boolean   ouvert;


   /* Initialisation du compteur de position */
   Position_fichier_date = 0;

   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);
   
   /* repertoire ANNEE */
   sprintf_s(temp, sizeof(temp), "%4d", p_date->wYear);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

   /* repertoire  MOIS */
   sprintf_s(temp, sizeof(temp), "%02d", p_date->wMonth);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

   if( _access(path_fichier, 0) == -1 )
   {
      FIC_CreatePath(path_fichier);
   }

   /* fichier date */
   sprintf_s(ficname, sizeof(ficname),"%02d",p_date->wDay);

   FIC_makepath(path_fichier, path_fichier, NULL, ficname, "txt");

   /* ouverture du fichier en lecture/ecriture */
   ouvert = InitFichierConfig(GereFic_date,path_fichier,"r+t",item,&id,&valeur);
   if( ! ouvert )
   {
      /* si le fichier n'existe pas, il est cr‚e */
      ouvert = InitFichierConfig(GereFic_date,path_fichier,"w+t",item,&id,&valeur);

      if( !ouvert )
         return FALSE;
   }
   else
   {
      /* recherche de la derniere iteration du fichier */
      while( LireFichierConfig( GereFic_date,
                                ID_PREMIER_FICHIER,
                                Position_fichier_date,
                                &valeur) == FIC_OK )
      {
         Position_fichier_date += 1;
      }
   }

   /*************************************************
    *   REMARQUE : avec l'utilisation de FIC_CONF   *
    *  Si on ouvre le fichier avec l'attribut "a+"  *
    *  au lieu de "r+" ou "w+", lors d'une demande  *
    *  de remplacement d'item, une nouvelle ligne   *
    *  avec le mˆme item sera cr‚e et non remplac‚e *
    *  => ajout en fin de fichier   !!!             *
    **************************************************/

   return( TRUE );
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean FermerFichierDate( TGereFic)
* PARAMETRES: pointeur sur le fichier a fermer
* RETOUR: etat de la fermeture : TRUE/FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Ferme un fichier de type TGereFic
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean FermerFichierDate(TGereFic *DateFile)
{
   if( !FermerFichierConfig( DateFile))
     return FALSE;

   return( TRUE );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean EffacerFichierDate( struct dostime_t date)
* PARAMETRES: date du fichier a effacer
* RETOUR: etat de l'effacement : TRUE/FALSE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Efface un fichier jj.txt
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean EffacerFichierDate(SYSTEMTIME date)
{
   char         path_fichier[MAX_PATH];
   boolean      report = FALSE;

   ObtenirFichierDateJour(date,path_fichier);

   if( _access(path_fichier,0) == 0)
   {
      if( FIC_remove(path_fichier) == 0)
         report = TRUE;
   }

   /* Suppression du repertoire s'il est vide */
   ObtenirFichierDateMois(date, path_fichier);
   if( FIC_EstRepertoireVide( path_fichier))
   {
//      ObtenirFichierDateMois(date, path_fichier);
      FIC_rmdir( path_fichier);

      /* V‚rification sur le repertoire pr‚c‚dant */
      ObtenirFichierDateAnnee(date, path_fichier);
      if( FIC_EstRepertoireVide( path_fichier))
      {
//         ObtenirFichierDateAnnee(date, path_fichier);
         FIC_rmdir( path_fichier);
      }
   }

   return( report );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean EcrireFichierDate( TGereFic *DateFile,
*                                     enum_fic_ident ident,
*                                     long numero )
* PARAMETRES: - Fichier de type TGereFic
*             - identificateur PREMIER ou DERNIER ou LES_DEUX
*             - numero du fichier … ecrire
* RETOUR: ecriture correcte ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Ecrit l'ident specifi‚ dans le fichier jj.txt :
*       Si les deux ident, PREMIER et DERNIER sont … ecrire,l'iteration
*       est incr‚ment‚e de 1, idem pour ident PREMIER seul,
*       Si seul l'ident DERNIER est … ecrire, on remplace
*       celui de la derniŠre it‚ration trouv‚e;
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean EcrireFichierDate(TGereFic *DateFile,
                                      enum_fic_ident ident,
                                      long numero)
{
   FIC_enum_retour    status;
   char               contenu[FICHIER_TAILLE_NOM];
   int iteration = Position_fichier_date;

   sprintf_s(contenu, sizeof(contenu), FICHIER_FORMAT_NOM, numero);

   /* recherche de la derniere iteration du fichier */
/*   while( LireFichierConfig(DateFile,ID_PREMIER_FICHIER,iteration,&valeur) == FIC_OK )
   {
      iteration += 1;
   }
*/
   if( ident == DATE_LES_DEUX )
   {
      status = AjouterFichierConfig(DateFile,ID_PREMIER_FICHIER,iteration,contenu );
      if( status != FIC_OK)
         return FALSE;
      status = AjouterFichierConfig(DateFile,ID_DERNIER_FICHIER,iteration, contenu );
      if( status != FIC_OK)
         return FALSE;
      /* Memoriser la nouvelle position */
      Position_fichier_date ++;
   }
   else if( ident == DATE_DERNIER )
   {
      /* l'iteration ne change pas */
      if(iteration >= 1 )
         iteration -= 1;
      status = RemplacerFichierConfig(DateFile,ID_DERNIER_FICHIER,iteration, contenu );
      if( status != FIC_OK)
         return FALSE;
   }

   return TRUE;

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichierDateJour(SYSTEMTIME date, char *path_fichier)
* PARAMETRES: date du fichier … trouver
* RETOUR: chemin complet du fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche un fichier date du type jj.txt
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichierDateJour(SYSTEMTIME date, char *path_fichier)
{
	char temp[MAX_PATH];
	char ficname[MAX_PATH];

   /* initialisation du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);
   
   /* repertoire ANNEE */
   sprintf_s(temp, sizeof(temp), "%4d", date.wYear);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

   /* repertoire  MOIS */
   sprintf_s(temp, sizeof(temp), "%02d", date.wMonth);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

   /* jour */
   sprintf_s(ficname, sizeof(ficname), "%02d", date.wDay);

   FIC_makepath(path_fichier, path_fichier, NULL, ficname, "txt");

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichierDateMois(SYSTEMTIME date, char *path_fichier)
* PARAMETRES: date du fichier … trouver
* RETOUR: chemin complet du fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche un fichier date du type jj.txt
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichierDateMois(SYSTEMTIME date, char *path_fichier)
{
   //char * ptr_path;
	char temp[MAX_PATH];

   /* initialisation du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);
   
   /* repertoire ANNEE */
   sprintf_s(temp, sizeof(temp), "%4d", date.wYear);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

   /* repertoire  MOIS */
   sprintf_s(temp, sizeof(temp), "%02d", date.wMonth);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);

}
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichierDateAnnee(SYSTEMTIME date, char *path_fichier)
* PARAMETRES: date du fichier … trouver
* RETOUR: chemin complet du fichier
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche un fichier date du type jj.txt
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichierDateAnnee(SYSTEMTIME date, char *path_fichier)
{
   //char * ptr_path;
	char temp[MAX_PATH];

   /* initialisation du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);
   
   /* repertoire ANNEE */
   sprintf_s(temp, sizeof(temp), "%4d", date.wYear);
   FIC_makepath(path_fichier, path_fichier, temp, NULL, NULL);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: boolean RechercheFichierInitEmettre( char *FichierColdStart,
*                                               long *ptr_num_fichier)
* PARAMETRES: le nom du fichier cold start
*             pointeur sur le numero du fichier cherche
* RETOUR: numero de fichier trouve
*         Etat de la recherche : OK / NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche le premier fichier … emettre :
*   1- A partir de c:\BACKUP\000, recherche du premier repertoire inexistant,
*      et revenir au precedent
*   2- Dans XXX, recherche … prtir de C:\BACKUP\XXX\XXX000 du premier
*      sous-repertoire inexistant et revenir au pr‚c‚dent
*   3- Dans XXXYYY, recherche du premier fichier d'extension .000,
*      si ce fichier est XXXYYY00.000, on recherche dans le
*      sous-repertoire pr‚cedent les ‚ventuels fichiers d'extension .000,
*      pour ˆtre s–r de partir du plus ancien fichier.
*   4- Si aucun fichier n'a ‚t‚ trouv‚, c'est impossible, on ne sait pas
*      traiter
*   5- Si un fichier a ‚t‚ trouv‚, on renvoie son numero.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean RechercheFichierInitEmettre( char *FichierColdStart,
                                               long *ptr_num_fichier)
{
   long					   rep, sous;
   int					   done;
   long                    calcul;
   struct _finddata_t      ffblk;
   char					   path_fichier[MAX_PATH];
   //char *				   ptr_repertoire;
   //char *				   ptr_sous_rep;
   boolean				   ok = FALSE;
   int					   extension;
   intptr_t				   hfind;
   char					   Path[MAX_PATH];
   char					   PathRep[MAX_PATH];
   char					   PathSRep[MAX_PATH];
   char					   temp[MAX_PATH];
	
//KONTROLA
   
   /* Verification du fichier de Cold start */
   
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, FichierColdStart, NULL);

   done = _access(path_fichier,0);
   if( done == 0)
   {
      *ptr_num_fichier = 0;
      return( TRUE);
   }

   FIC_makepath(Path, DIR.local_drive, DIR.path_backup, NULL, NULL);

   /* Recherche dans les repertoires */
   for( rep = 0L; rep <= 1000L; rep ++ )
   {
	   sprintf_s(temp, sizeof(temp), FICHIER_FORMAT_REPERTOIRE, rep);

	  FIC_makepath(PathRep, Path, temp, NULL, NULL);

      done = _access(PathRep,0);
      if( done == 0 )
      {
         /* recherche dans les sous repertoires */
         for( sous= 0L; sous <= 1000L; sous ++ )
         {
            calcul = (rep*1000L) + sous;
			sprintf_s(temp, sizeof(temp), FICHIER_FORMAT_SOUS_REP, calcul);

            FIC_makepath(PathSRep, PathRep, temp, NULL, NULL);

			done = _access(PathSRep,0);
            if( done == 0 )
            {
               /* recherche d'un fichier par son extension */
				FIC_makepath(path_fichier, PathSRep, NULL, "*", "000");

               hfind = FIC_FindFirst(path_fichier,&ffblk);
               if( hfind >= 0)
               {
                  /* Un fichier avec la bonne extension a ete trouve */
                  ok = TRUE;
               }
            }

            /* Fichier trouve */
            if( ok == TRUE)
               break;
         }
      }
      /* Fichier trouve */
      if( ok == TRUE)
         break;
   }

   if( ok == TRUE)
   {
      *ptr_num_fichier = LONG_MAX;

      /* Recherche du plus petit fichier */
      while( done == 0 )
      {
         /* fichier de numero le plus faible si plusieurs existent */
         if( *ptr_num_fichier  > atol(ffblk.name))
         {
            *ptr_num_fichier = atol(ffblk.name);
         }

         done = FIC_FindNext(hfind,&ffblk);
      }
	  FIC_FindClose(hfind);
   }
   else
   {
      /* aucun fichier emissible n'a ‚t‚ trouv‚ dans le repertoire */
      /* donc on cherche le dernier fichier emis, et on incremente */
      ok = RechercheFichierInitCreer( ptr_num_fichier,
                                      &extension,
                                      path_fichier);
      ERR_EstVrai( ok);
      if( ( extension != FICHIER_SANS_EXTENSION) &&
          ( extension != 0))
      {
        /* ce fichier a deja ete emis, il faut passer
         * au suivant qui n'est pas encore cree
         */
         (*ptr_num_fichier)++;
      }
   }

   return( ok );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int RechercheFichierInitCreer(long *)
* PARAMETRES: pointeur sur le numero du fichier cherche
* RETOUR: numero de fichier trouve
*         Etat de la recherche : OK / NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche le dernier fichier cree
*       Cette recherche part de la fin et d‚cremente
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean RechercheFichierInitCreer( long *num_fichier,
                                             int  *extension,
                                             char *path_fichier)
{
   long					rep, sous, fic;
   int					done;
   long					calcul;
   struct _finddata_t   ffblk;
   //char					*ptr_path, *ptr_ext;
   char					*ptr_ext;
   intptr_t				hfind;
   char					Path[MAX_PATH];
   char					PathRep[MAX_PATH];
   char					PathSRep[MAX_PATH];
   char					ficname[MAX_PATH];
   char					temp[MAX_PATH];

   
   FIC_makepath(Path, DIR.local_drive, DIR.path_backup, NULL, NULL);
   
   done = _access( Path, 0);
   if( done != 0 )
      return( FALSE);

   /* tant que le fichier n'est pas trouve */
   for( rep = 999L; rep >= 0L; rep -- )
   {
      //sprintf( ptr_path, FICHIER_FORMAT_REPERTOIRE, rep);
	   sprintf_s(temp, sizeof(temp), FICHIER_FORMAT_REPERTOIRE, rep);

	  FIC_makepath(path_fichier, Path, temp, NULL, NULL);

      done = _access( path_fichier, 0);
      if( done == 0 )
         break;
   }
   if( rep < 0)
      return( FALSE);

   /* le repertoire de numero le plus haut a ete trouve */

   STR_strcpy(MAX_PATH, PathRep, path_fichier);


   /* recherche des sous repertoires */
   for( sous= 999L; sous >= 0L; sous -- )
   {
      calcul = (rep*1000L) + sous;
	  sprintf_s(temp, sizeof(temp), FICHIER_FORMAT_SOUS_REP, calcul);

	  FIC_makepath(path_fichier, PathRep, temp, NULL, NULL);

      done = _access( path_fichier, 0);
      if( done == 0 )
         break;
   }
   if( sous < 0)
      return( FALSE);

   /* le sous-repertoire de numero le plus haut a ete trouve */

   STR_strcpy(MAX_PATH, PathSRep, path_fichier);

   /* recherche du fichier */
   for( fic= 99L; fic >= 0L; fic -- )
   {
      calcul = (rep*100000L) + ( sous*100) + fic;
      sprintf_s( ficname, sizeof(ficname), FICHIER_FORMAT_NOM, calcul);

	  // Extension "*" pour rechercher tout type de fichier
	  FIC_makepath(path_fichier, PathSRep, NULL, ficname, "*");

      /* ici on fait un findfirst a cause du joker */
      hfind = FIC_FindFirst(path_fichier,&ffblk);
      if( hfind >= 0 )
         break;
   }
   if( fic < 0)
      return( FALSE);

   /* nom complet du fichier trouve */
	FIC_makepath(path_fichier,PathSRep,NULL, ffblk.name, NULL);
  
   /* extraire le numero du fichier trouve */
   *num_fichier = atol( ffblk.name);

   /* verifier l'extension */
   ptr_ext =  strchr( ffblk.name, '.');

   if( ptr_ext == NULL)
      *extension = FICHIER_SANS_EXTENSION;
   else
      *extension = atoi(ptr_ext+1);

   FIC_FindClose(hfind);

   return (TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED boolean RechercherFichier( long num_fichier,
*                                                 int extension,
*                                                 char *path_fichier)
* PARAMETRES: numero du fichier recherch‚
*             extension du fichier recherch‚
*             pointeur sur le chemin du fichier cherche
* RETOUR:     chemin du fichier trouve
*             Etat de la recherche : OK / NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche un fichier a partir de son numero et de
*       son extension.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean RechercherFichier(long num_fichier, int num_extension, char *path_fichier)
{
   boolean        retour = TRUE;
   struct _finddata_t    ffblk;
   //char *         ptr_path;
   char           nom_extension[FICHIER_TAILLE_EXT];
   intptr_t		  hfile;
   char			  Path[MAX_PATH];
   char			  tempformat[MAX_PATH];
   char			  ficname[MAX_PATH];

   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_SOUS_REP, ObtenirSousRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

	STR_strcpy(MAX_PATH,Path,path_fichier);

   /* extension du fichier s'il y en a une */
   if( num_extension == FICHIER_SANS_EXTENSION)
	   sprintf_s(nom_extension, sizeof(nom_extension), "*");
   else
	   sprintf_s(nom_extension, sizeof(nom_extension), FICHIER_FORMAT_EXTENSION, num_extension);

   //STR_strcat(MAX_PATH,path_fichier,nom_extension);

   sprintf_s(ficname, sizeof(ficname), FICHIER_FORMAT_NOM, num_fichier);
/*
   STR_strcat(MAX_PATH,ficname, ".");
   STR_strcat(MAX_PATH,ficname, nom_extension);
*/
   FIC_makepath(path_fichier, Path, NULL, ficname, nom_extension);

   /* le fichier existe */
   if ( (hfile = FIC_FindFirst(path_fichier,&ffblk)) >= 0)
   {
      /* verifier l'extension */
      if( num_extension < 0 )
	  {
         //sprintf(ptr_path,"\\%s",ffblk.name );
		 STR_strcpy(MAX_PATH,ficname,ffblk.name);
		 FIC_makepath(path_fichier, Path, NULL, ficname, NULL);
	  }

	  FIC_FindClose(hfile);
   }
   else
      retour = FALSE;

   return (retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int RechercheFichierParDate( char *date_debut,
*                                                  char *date_fin,
*                                                  long *premier_num
*                                                  long *dernier_num)
* PARAMETRES: date de debut / date de fin
*          pointeur sur les premier et dernier fichiers a trouve
* RETOUR:  Premier fichier trouve
*          dernier fichier trouve
*          Etat de la recherche : OK / NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche les intervalles de numeros de fichiers a restituer
*       a partir d'une date : [premier - dernier]
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int RechercherFichierParDate(SYSTEMTIME date,
                                             long *premier_numero,
                                             long *dernier_numero,
                                             int iteration)
{
   TGereFic           gereFic;
   char               item[CONF_MAX_ITEM];
   TValeur            id;
   TValeur            valeur;
   FIC_enum_retour    status;
   char               path_fichier[MAX_PATH];
   short int          retour;

   /* recherche du fichier DATE.txt */
   ObtenirFichierDateJour(date, path_fichier);

   retour = EMI_PAS_ERREUR;

   if( ! InitFichierConfig(&gereFic,path_fichier,"r+t", item,&id,&valeur))
      retour = EMI_ERREUR_FICHIER;
   else
   {
      status = LireFichierConfig(&gereFic,ID_PREMIER_FICHIER, iteration, &valeur);
      if( status != FIC_OK)
	  {
		FermerFichierConfig( &gereFic); // Pas de test de retour ...
        return EMI_ERREUR_CONTENU;  // ... car on priviligie ce code d'erreur
	  }

      *premier_numero = atol(valeur.Contenu);

      status = LireFichierConfig(&gereFic,ID_DERNIER_FICHIER, iteration, &valeur);
      if( status != FIC_OK)
	  {
		FermerFichierConfig( &gereFic); // Pas de test de retour ...
        return EMI_ERREUR_CONTENU;  // ... car on priviligie ce code d'erreur
	  }

      *dernier_numero = atol(valeur.Contenu);

      if( !FermerFichierConfig( &gereFic))
         return EMI_ERREUR_FICHIER;
   }

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int CopierFichierDisque
					( char *FilePath_source, char *FileName_dest)
* PARAMETRES: full name (with path) of source file, destination file name
* RETOUR:     envoi correct ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Copier un fichier sur le disque local,
*       dans un repertoire specifique au backup sur disquette,
*       L'application DOIT VIDER CE REPERTOIRE APRES CHAQUE RECOPIE
*       des fichiers sur la disquette.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean CopierFichierDisque(char *FilePath_source, char *FileName_dest)
{
   boolean copy_report;
   char path_dest[MAX_PATH];
   char  ficname[MAX_PATH];
   char	 extname[MAX_PATH];

   /* initialisation de variables de travail */
   copy_report = FALSE;

   FIC_makepath(path_dest, DIR.local_drive, DIR.path_disk, NULL, NULL);
   FIC_makepath(path_dest, path_dest, DIR.path_backup, NULL, NULL);
   
   
   if( _access(path_dest,0 ) == -1 )
   {
     if( FIC_CreatePath(path_dest) == -1 )
        return FALSE;
   }

   if (FileName_dest != NULL)
	   FIC_makepath(path_dest, path_dest, NULL, FileName_dest, NULL);
   else
   {
	   FIC_splitpath(FilePath_source, NULL, NULL, ficname, extname);
	   FIC_makepath(path_dest, path_dest, NULL, ficname, extname);
   }


   //FIC_splitpath(path_fichier, NULL, NULL, ficname, extname);

   //FIC_makepath(path_dest, path_dest, NULL, ficname, extname);
	if( _access(path_dest,0 ) == 0 )
   {
     if( FIC_remove(path_dest) != 0)
        return FALSE;
   }
   
   copy_report = FIC_copy(path_dest,FilePath_source,FIC_COPY_WITH_DATE);

   return (copy_report);
}


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichier(long num_fichier, char * path_fichier)
* PARAMETRES:
*             - numero du fichier cherche
* RETOUR: chemin complet du fichier sans extension
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Trouve le chemin du fichier … partir de son numero
*       sans tenir compte de l'extension
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichier(long num_fichier,char * path_fichier)
{
   char tempformat[MAX_PATH] ;
   //char * ptr_path;
   char ficname[FICHIER_TAILLE_NOM];

   /* initialisation du pointeur sur le nom du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   /* obtention des repertoire et sous repertoire */
   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_SOUS_REP, ObtenirSousRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   /* obtention du nom ascii a partir d'un long */
   sprintf_s(ficname, sizeof(ficname), FICHIER_FORMAT_NOM, num_fichier);

   FIC_makepath(path_fichier, path_fichier, NULL, ficname, NULL);
}
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichierSansNom(long num_fichier, char * path_fichier)
* PARAMETRES:
*             - numero du fichier cherche
* RETOUR: chemin complet du fichier sans extension
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Trouve le chemin du fichier … partir de son numero
*       sans tenir compte de l'extension
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichierSansNom(long num_fichier,char * path_fichier)
{
   //char * ptr_path;
   char tempformat[MAX_PATH];

   /* initialisation du pointeur sur le nom du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   /* obtention des repertoire et sous repertoire */
   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);

   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_SOUS_REP, ObtenirSousRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ObtenirFichierSansDernierRepertoire(long num_fichier, char * path_fichier)
* PARAMETRES:
*             - numero du fichier cherche
* RETOUR: chemin sans le dernier r‚pertoire et sans le nom
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Trouve le chemin du fichier … partir de son numero
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ObtenirFichierSansDernierRepertoire(long num_fichier,char * path_fichier)
{
   //char * ptr_path;
   char  tempformat[MAX_PATH];

   /* initialisation du pointeur sur le nom du repertoire */
   FIC_makepath(path_fichier, DIR.local_drive, DIR.path_backup, NULL, NULL);

   /* obtention des repertoire et sous repertoire */
   sprintf_s(tempformat, sizeof(tempformat), FICHIER_FORMAT_REPERTOIRE, ObtenirRepertoire(num_fichier));
   FIC_makepath(path_fichier, path_fichier, tempformat, NULL, NULL);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: short int ObtenirRepertoire
* PARAMETRES:
*             - numero du fichier a cr‚er
* RETOUR:
*             - numero du repertoire
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Trouve le numero du repertoire a partir du nom du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE long ObtenirRepertoire( long num_fichier)
{
   long temp;

   /* repertoire */
   if( num_fichier > 99999L )
      temp = num_fichier / 100000L;
   else
      temp = 0L;

   return temp;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: short int ObtenirSousRepertoire
* PARAMETRES:
*             - numero du fichier a cr‚er
* RETOUR:
*             - numero du repertoire
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Trouve le numero du sous repertoire a partir du nom du fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE long ObtenirSousRepertoire( long num_fichier)
{
   long temp;

   /* sous repertoire */
   if( num_fichier > 99L )
      temp = num_fichier / 100L;
   else
      temp = 0L;

   return temp;
}


