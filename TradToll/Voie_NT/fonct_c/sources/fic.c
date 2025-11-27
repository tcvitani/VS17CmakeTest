/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Encapsulation des fonctions d'accŠs aux fichiers
* FICHIER: FIC.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Les fonctions standard d'accŠs aux fichiers sont encapsul‚s
*         pour effectuer un traitement d'erreur fatale en cas d'‚chec
* --------------------------------------------------------------------
 * DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/FIC.C_v  $
 * 
 *    Rev 1.7   13 Dec 1999 16:24:50   afx
 *  
 * 
 *    Rev 1.6   01 Jul 1999 17:33:50   afx
 * Modif de la fct FIC_makepath()
 * 
 *    Rev 1.5   10 Mar 1999 15:12:42   afx
 * Correction fonction acces disquette :
 * ajout SetLastError(ERROR_SUCCESS) avant appel ulterieur a GetLastError().
 * 
 *    Rev 1.4   Mar 02 1999 17:54:54   afx
 *  
 * 
 *    Rev 1.3   Feb 09 1999 10:32:20   afx
 * - Correction FIC_deltree().
 * - Ajout surcouche pour findfirst(), findnext() et findclose()
 *   (FIC_FindFist() et FIC_FindNext() ignorent les repertoires systemes "." et "..").
 * 
 * 
 *    Rev 1.2   Jan 14 1999 09:58:18   bph
 * Remplacement de fflush par _commit
 * 
 *    Rev 1.1   02 Oct 1998 11:42:12   bph
 *  
 * 
 *    Rev 1.11   17 Mar 1998 18:15:02   DPI
 * Enlever un warning
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <direct.h>
#include <string.h>
#include <errno.h>
#include <share.h>
#include <sys/stat.h>

#include "err.h"
#include "mem_c.h"
#include "str.h"

#include "fic.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/

/*--------------- CONSTANTES ---------------*/
#define FIC_BUFFER_SIZE               512

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

PRIVATE unsigned char OLD_NB_HDL = 0;
PRIVATE unsigned char *OLD_FHT = NULL;
PRIVATE DWORD CURRENT_SERIAL = 0;

/*--------------- CODE: ---------------*/

PUBLIC int WINAPI FIC_Fullfcloseall( char *__FILE, int __LINE)
{
   int result;

   result = _fcloseall();
   if( result == EOF)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

char *FIC_Fullfgets( char *__FILE, int __LINE,
                 char *s, int n, FILE *stream)
{
   NO_WARNING( __FILE);
   NO_WARNING( __LINE);

   return( fgets(s, n, stream));
}

int WINAPI FIC_Fullremove( char *__FILE, int __LINE, char *filename)
{
   int result;

   result = remove(filename);
   if( result == -1)
	   if (errno != ENOENT) // Si le fichier existe
			ERR_TraiterErreurFatale( __FILE, __LINE);
	   else
		   result = 0; // Le fichier n'existe pas, ignorer l'erreur

   return(result);
}

int WINAPI FIC_Fullchsize( char *__FILE, int __LINE,
                int handle, long size)
{
   int result;

   result = _chsize(handle, size);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullfprintf( FILE *stream, char *format, ...)
{
   va_list       args;
   char          buf[100];
   int           result;

   va_start( args, format );
   vsprintf_s( buf,sizeof(buf), format, args );
   va_end(args);

   result = fprintf (stream, "%s", buf);
   if( result == EOF)
       ERR_ErreurFatale();

   return result;
}

char * WINAPI FIC_Fullmktemp( char *__FILE, int __LINE, char *temp)
{
	char *result;

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
	
	result = mktemp(temp);

#pragma warning(pop)

	if (result == NULL)
		ERR_TraiterErreurFatale(__FILE, __LINE);

	return(result);
}

int WINAPI FIC_Fullrename( char *__FILE, int __LINE,
                    char *oldname, char *newname)
{
   int result;

   result = rename(oldname, newname);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullsystem( char *__FILE, int __LINE, char *command)
{
   int result;

   result = system (command);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullmkdir( char *__FILE, int __LINE, char *path)
{
   int result;

   result = _mkdir (path);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullrmdir( char *__FILE, int __LINE, char *path)
{
   int result;

   result = _rmdir (path);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullunlink( char *__FILE, int __LINE, char *fichier)
{
   int result;

   result = _unlink (fichier);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

void WINAPI FIC_Full_makepath( char *__FILE, int __LINE,
                       char *path, char *drive, char *dir, char *name, char *ext)
{
	int   taille;
	char fullpath[_MAX_PATH];
	char copydrive[_MAX_PATH];
	char disk_unit[_MAX_DRIVE];
	char disk_root[_MAX_DIR];
	char disk_name[_MAX_FNAME];
	char disk_ext[_MAX_EXT];
	char last_char;
	
	NO_WARNING( __FILE);
	NO_WARNING( __LINE);

	if (dir != NULL)
		strcpy_s (fullpath,sizeof(fullpath), dir);
	else
		fullpath[0] = '\x0';

	if (drive != NULL)
		strcpy_s (copydrive,sizeof(copydrive), drive);
	else
		copydrive[0] = '\x0';

	taille = (int)strlen (copydrive);
	if (taille) /* Le disque est specifie a l'appel */
	{
		/* Verifier si le disque n'est pas lui-meme une arborescence complete (Ex : c:\vdisk) */
		/* Si c'est le cas, il faut separer l'unite de disque du reste et deplacer ce reste */
		/* dans l'argument 'dir' du _makepath() final */

		if (strstr (copydrive, ":") != NULL)
		{
			last_char = copydrive[taille-1];
			if ((last_char != '\\') && (last_char != ':'))
				/* Ajout d'un '\' sinon on recupere un nom de fichier a l'appel de _splitpath() */
				strcat_s (copydrive,sizeof(copydrive), "\\");

			/* Decomposer le disque passe en parametre */
			_splitpath_s (copydrive, disk_unit,sizeof(disk_unit), disk_root,sizeof(disk_root), disk_name,sizeof(disk_name), disk_ext,sizeof(disk_ext));

			taille = (int)strlen (disk_root);
			if (taille > 0) /* Si le disque represente une arborescence */
			{
				strcpy_s (copydrive,sizeof(copydrive), disk_unit); /* Isoler l'unite de disque */
				strcpy_s (fullpath,sizeof(fullpath), disk_root); /* Recuperer l'arborescence disque */
				if (dir != NULL)
				{
					/* Supprimer un eventuel '\' de trop */
					if ((fullpath[taille-1] == '\\') && (dir[0] == '\\'))
						fullpath[taille-1] = '\x0';

					/* Ajouter l'arborescence initiale */
					strcat_s (fullpath,sizeof(fullpath), dir);
				}
			}
		}
	}
	
	taille = (int)strlen (copydrive);
	if (taille)
	{
		/* Supprimer un eventuel '\' de trop */
		if ((copydrive[taille-1] == '\\') && (fullpath[0] == '\\'))
			copydrive[taille-1] = '\x0';
	}

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

	/* Composer l'arborescence complete */
	_makepath(path, copydrive, fullpath, name, ext);

#pragma warning(pop)

	if( name == NULL)
	{
	/* le path est termin‚ par un '\', ce qui pose des pb
	* lors de l'appel des fonction mkdir,..
		*/
		taille = (int)strlen( path);
		path[ taille - 1] = '\0';
	}
}

void WINAPI FIC_Full_splitpath( char *__FILE, int __LINE,
                       char *path, char *drive, char *dir, char *name, char *ext)
{
   NO_WARNING( __FILE);
   NO_WARNING( __LINE);

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

   _splitpath(path, drive, dir, name, ext);

#pragma warning(pop)

}



FILE * WINAPI FIC_Fullfopen( char *__FILE, int __LINE,
                 char *filename, char *mode)
{
   FILE *result;
   errno_t error;

   result = _fsopen(filename, mode, _SH_DENYNO);
   if (result == NULL)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

int WINAPI FIC_Fullfclose( char *__FILE, int __LINE,
                FILE *stream)
{
   int result;

   result = fclose( stream);
   if( result == EOF)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

size_t WINAPI FIC_Fullfwrite( char *__FILE, int __LINE,
                   void *ptr, size_t size, size_t n, FILE *stream)
{
   size_t result;

   result = fwrite (ptr, size, n, stream);
   if( result != n)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

size_t WINAPI FIC_Fullfread( char *__FILE, int __LINE,
                  void *ptr, size_t size, size_t n, FILE *stream)
{
   size_t result;

   result = fread( ptr, size, n, stream);
   if( result != n)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

long WINAPI FIC_Fullftell( char *__FILE, int __LINE,
                FILE *stream)
{
   long result;

   result = ftell( stream);
   if( result == -1L)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

int WINAPI FIC_Fullfseek( char *__FILE, int __LINE,
               FILE *stream, long offset, int whenc)
{
   int result;

   result = fseek( stream, offset, whenc);
   if( result < 0)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

void WINAPI FIC_Fullrewind( char *__FILE, int __LINE,
                 FILE *stream)
{
   NO_WARNING( __FILE);
   NO_WARNING( __LINE);

   rewind( stream);
}

int WINAPI FIC_Fullopen2( char *__FILE, int __LINE,
               char *path, int access)
{
   int result;
   errno_t err;

   err = _sopen_s(&result, path, access, _SH_DENYNO, _S_IREAD | _S_IWRITE);
   if (err != 0)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullopen3( char *__FILE, int __LINE,
               char *path, int access , unsigned mode)
{
   int result;
   errno_t err;

   err = _sopen_s(&result, path, access, _SH_DENYNO, mode);
   if (err != 0)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

int WINAPI FIC_Fullclose( char *__FILE, int __LINE,
               int handle)
{
   int result;

   result = _close( handle);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

int WINAPI FIC_Fullwrite( char *__FILE, int __LINE,
               int handle, void *buf, unsigned len)
{
   int result;

   result = _write (handle, buf, len);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return result;
}

int WINAPI FIC_Fullread( char *__FILE, int __LINE,
              int handle, void *buf, unsigned len)
{
   int result;

   result = _read( handle, buf, len);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

long WINAPI FIC_Fulltell( char *__FILE, int __LINE,
               int handle)
{
   long result;

   result = _tell( handle);
   if( result == -1L)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

long WINAPI FIC_Fulllseek( char *__FILE, int __LINE,
                int handle, long offset, int fromwher)
{
   long result;

   result = _lseek( handle, offset, fromwher);
   if( result == -1L)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullgetc( char *__FILE, int __LINE,
              FILE *stream)
{
   int result;

   result = getc( stream);
   if( result == EOF)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

long WINAPI FIC_Fullfilelength( char *__FILE, int __LINE,
                     int handle)
{
   long result;

   result = _filelength( handle);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

void WINAPI FIC_Fullsetbuf( char *__FILE, int __LINE,
                 FILE *stream, char *buf)
{
   NO_WARNING( __FILE);
   NO_WARNING( __LINE);

#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code

   setbuf(stream, buf);

#pragma warning(pop)
}

int WINAPI FIC_Fullgetftime( char *__FILE, int __LINE,
                  int handle, FILETIME *ftimep)
{
   if (!GetFileTime( (HANDLE) _get_osfhandle(handle), NULL, NULL, ftimep))
   {
      ERR_TraiterErreurFatale( __FILE, __LINE);
      return FALSE;
   }

   return TRUE;
}

int WINAPI FIC_Fullsetftime( char *__FILE, int __LINE, int handle, FILETIME *ftimep)
{
   if (!SetFileTime( (HANDLE) _get_osfhandle(handle), NULL, NULL, ftimep))
   {
      ERR_TraiterErreurFatale( __FILE, __LINE);
      return FALSE;
   }

   return TRUE;
}

int WINAPI FIC_Fulldup( char *__FILE, int __LINE, int handle)
{
   int result;

   result = _dup( handle);
   if( result == -1)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return(result);
}

int WINAPI FIC_Fullfflush( char *__FILE, int __LINE, FILE *stream)
{
   int result;

   /* flush the stream's internal buffer */
   result = _commit(_fileno(stream));
   if( result != 0)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( result);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL FIC_FullCreatePath  ( char *path )
 * PARAMETERS: char *__FILE : nom du fichier source
 *             int __LINE   : ligne d'appel de cette fonction dans le source
 *             char *path   : arborescence a creer : CE PARAMETRE NE PEUT EXCEDER _MAX_PATH
 * RETURN    : TRUE si OK, FALSE sinon
 *			   Le parametre <path> n'est pas modifie par cette fonction
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une arborescence complete
 * REMARQUES : Cette fonction accepte les creations sur machine distante
 *			   a condition evidemment que les droits soient autorises sur
 *			   cette machine distante.
 * EXEMPLES  : result = FIC_CreerArborescence ("..\dir1\dir2");
 *			   result = FIC_CreerArborescence ("c:\dir1\dir2\dir3");
 *			   result = FIC_CreerArborescence ("\\ordinateur_distant\partage\dir1\dir2");
 *			   result = FIC_CreerArborescence ("\\ordinateur_distant\partage\nom long ok\dir");
 * --------------------------------------------------------------------
 */

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC BOOL FIC_FullCreatePath ( char *__FILE, int __LINE, char *path )
 * PARAMETERS: char *__FILE : 
 *             int __LINE   : 
 *             char *path   : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PUBLIC BOOL WINAPI FIC_FullCreatePath (char *__FILE, int __LINE, char *path)
{
    char *token;
    char full_path[_MAX_PATH], copy_path[_MAX_PATH], build_path[_MAX_PATH];
	char *slash="/", *backslash = "\\";
	char *disk_sep = ":";
	char *machine_sep = "\\\\";
	char *sep = "\\/";
	char *next_token = NULL;

    NO_WARNING( __FILE);
    NO_WARNING( __LINE);
    
	/* Test coherence parametre */
	if (path == NULL)
		return FALSE;
	if (strlen (path) > _MAX_PATH)
		return FALSE;

	/* Recopier la chaine passee en parametre pour ne pas la modifier */
	strcpy_s (copy_path,sizeof(copy_path), path);

	/* Remplacer les eventuels '/' par des '\' */
	/* Le plupart des fonctions de manipulation de repertoires interpretent correctement le '/' */
	/* mais pour garantir un bon comportement, on effectue le remplacement */
	token = copy_path;
	while ((token = strstr (copy_path, slash)) != NULL)
	{
		*token = *backslash;
		token ++;
	}

	/* Transformer l'arborescence passee en parametre en chemin absolu */
	/* C'est-a-dire avec nom de machine ou unite de disque en premier */
    if (_fullpath(full_path, copy_path, _MAX_PATH) == NULL)
        return FALSE;

	/* Initialiser l'arborescence a creer */
	strcpy_s (build_path,sizeof(build_path), "");
	/* Verifier si nom de machine inclus dans l'arborescence a construire */
	token = strstr (copy_path, machine_sep);
	/* Si ce n'est pas le cas ... */
	if (token == NULL)
		/* Verifier si unite de disque inclus dans l'arborescence a construire */
		token = strstr (copy_path, disk_sep);
	else /* Arborescence avec nom de machine */
		/* Completer l'arborescence a creer (avec identifiant machine ("\\")) */
		strcat_s (build_path,sizeof(build_path), machine_sep);

	/* Si on a trouve le nom de machine ou l'unite de disque */
	/* Ce qui doit etre le cas apres l'appel a _fullpath(...) */
	if (token != NULL)
	{
		/*****************************************************************************************/
		/* Attention on utilise la fonctionnalite de strtok() qui ignore les caracteres de dedut */
		/* de chaine s'ils font partis des caracteres recherches                                 */
		/* Exemple avec nom de machine  : strtok ("\\\\machine_name\\shared_dir\\dir1", "\\")==> */
		/*		strtok() renvoie "machine_name" (elle ignore les 2 '\' de debut de chaine        */
		/* Exemple avec unite de disque : strtok ("c:\\dir1\\dir2", "\\") ==>                    */
		/*		strtok() renvoie "c:"                                                            */
		/* Ainsi dans notre cas, un seul appel a strtok() nous donne soit :                      */
		/*	- le nom de machine                                                                  */
		/*	- l'unite de disque                                                                  */
		/*****************************************************************************************/
		token = strtok_s(copy_path, backslash, &next_token);

		/* Si aucun nom de machine ou unite de disque */
		if (token == NULL)
			/* Rien a creer */
			return FALSE;
	}
	else /* Ni nom de machine, ni unite de disque */
		return FALSE;

	/* Completer l'arborescence a creer (avec nom machine ou unite de disque) suivi de '\' */
	strcat_s (build_path,sizeof(build_path), token);
	strcat_s (build_path,sizeof(build_path), backslash);

    // Boucle de création des sous-répertoires (si inexistant) : critere de recherche = '\' */
    while ((token = strtok_s(NULL, backslash,&next_token)) != NULL)
    {
		/* Completer l'arborescence a creer (avec sous-repertoire trouve) */
		strcat_s (build_path,sizeof(build_path), token);
		/* Si ce repertoire n'existe pas deja */
        if (_access(build_path, 0) != 0)
			/* Tenter de le creer */
            if(!CreateDirectory(build_path, NULL))
                return FALSE; /* Erreur creation */

		/* Completer l'arborescence a creer (avec '\' entre chaque sous-repertoire trouve) */
		strcat_s (build_path,sizeof(build_path), backslash);
    }

    return TRUE;
}
        
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:     PUBLIC int deltree (char *pathname);
* PARAMETRES:  Repertoire a detruire
* RETOUR:      0 = OK, autre = erreur
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Detruit un repertoire (meme non vide!)
* --------------------------------------------------------------------
* NOTA: Heritage de GSZ
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC int WINAPI FIC_Fulldeltree ( char *__FILE, int __LINE, char *pathname)
{
    char             localpath[ _MAX_PATH];
    char             *path;
    char             wdir[ _MAX_PATH];
    char             dirsave[ _MAX_PATH];
    char             root[4];
    struct           _finddata_t ffblk;
    intptr_t         hfile = -1; 
    int              done,res,savedisk;
    unsigned char    end,errdir,first;
    unsigned char    i;
    char             *point=".";
    char             *pointpoint="..";
    char             *doublepoint=":";
    
    NO_WARNING( __FILE);
    NO_WARNING( __LINE);
    
    /* Par defaut resultat OK */
    res = 0;
    
    /* Recopie du repetoire a detruire en local */
    STR_strcpy ( _MAX_PATH, localpath, pathname);
    
    /* Positionner pointeur sur buffer local */
    path = localpath;
    
    /* Sauvegarder disque actif courant */
    savedisk = _getdrive ();
    
    /* Supprimer les espaces eventuels en debut de chaine passee */
    /* en parametre */
    while (*path == ' ')
    {
        path ++;
    }
    
    /* Remplacer les '/' eventuels par des '\' */
    for (i=0;i<strlen(path);i++)
    {
        if (path[i] == '/')
        {
            path[i] = '\\';
        }
    }
    
    /* Si le chemin passe en parametre contient les ":" */
    if (strstr (path, doublepoint) != NULL)
    {
        /* Le chemin contient une unite de disque */
        /* Il faut rendre actif le disque contenant le repertoire a detruire */
        
        /* Si l'unite de disque est une minuscule */
        if (islower ((int)path[0]))
        {
            path[0] -= ('a'-'A'); /* Mettre une majuscule */
        }
        
        /* Si changement de disque actif non OK */
        if (_chdrive ((int)path[0] - 'A' + 1) == -1)
        {
            res = -1; /* Indiquer erreur */
        }
    }
    
    /* Si sauvegarde repertoire courant non OK */
    if (_getcwd (dirsave, _MAX_PATH) == NULL)
    {
        res = -1;      /* Indiquer erreur */
        errdir = TRUE; /* Memoriser erreur sur repertoire courant */
        end = TRUE;    /* Fin du traitement */
    }
    else
    {
        errdir = FALSE; /* Aucune erreur sur repertoire courant */
        
        /* Recuperer repertoire racine du disque actif */
        strncpy_s (root,sizeof(root), dirsave, 3);
        root[3] = '\x0'; /* Caractere de fin de chaine */
        
        /* Si le repertoire a detruire est la racine ou chaine vide */
        if ((strcmp (path, root) == 0) || (strlen (path) == 0))
        {
            res = -1; /* Indiquer erreur */
        }
        else
        {
            /* Se placer a la racine */
            if (_chdir (root) == -1)
            {
                res = -1; /* Indiquer erreur */
            }
        }
        
        /* Si aucune erreur */
        if (res == 0)
        {
            /* Conditions OK pour traitement */
            end = FALSE;
        }
        else
        {
            /* Conditions non OK pour traitement */
            end = TRUE;
        }
    }
    
    /* Boucler tant que la destruction n'est pas terminee */
    while ((!end) && (res == 0))
    {
        /* Si deplacement dans repertoire a detruire est OK */
        if (_chdir (path) == 0)
        {
            /* Si recuperation du repertoire complet a detruire non OK*/
            if (_getcwd (path, _MAX_PATH) == NULL)
            {
                res = -1; /* Indiquer erreur */
            }
            
            /* Premiere recherche pour ce repertoire */
            first = TRUE;
            hfile = -1;
            
            done = 0; /* Init a "tout va bien" */
            
            /* Boucler tant que tout va bien */
            while ((res == 0) && (done == 0))
            {
                /* Si premiere recherche pour ce repertoire */
                if (first)
                {
                    if (hfile != -1) /* S'il reste un lien sur une recherche de fichier (findfirst(), findnext()) */
                    {
                        if (FIC_FindClose (hfile) == -1) /* Fermer ce lien pour liberer handle systeme */
                            res = -1; /* Signaler erreur */
                        hfile = -1;
                    }
                    
                    /* Recherche du premier repertoire ou fichier */
                    hfile = FIC_FindFirst ("*.*", &ffblk);
                    if (hfile == -1L)
                        done = -1;
                    else
                        done = 0;
                    
                    first = FALSE;
                }
                else
                {
                    /* Recherche du repertoire ou fichier suivant */
                    done = FIC_FindNext (hfile, &ffblk);
                }
                
                /* Si repertoire ou fichier trouve */
                if (done == 0)
                {
                    /* Si c'est un repertoire */
                    if (ffblk.attrib & _A_SUBDIR)
                    {
                        /* Descendre dans ce repertoire */
                        done = _chdir (ffblk.name);
                        
                        /* Si changement de repertoire OK */
                        if (done == 0)
                        {
                            /* Fermer le lien sur la recherche de fichier (findfirst(), findnext()) de */
                            /* l'ancien repertoire */
                            if (FIC_FindClose (hfile) == -1)
                                res = -1; /* Signaler erreur */
                            hfile = -1;
                            
                            /* Il faut recommencer la recherche dans ce */
                            /* nouveau repertoire */
                            first = TRUE;
                        }
                    }
                    else /* C'est un fichier */
                    {
                        /* Si destruction de ce fichier impossible */
                        if (remove (ffblk.name) == -1)
                        {
                            res = -1; /* Signaler erreur */
                        }
                    }
                }
                /* Ni repertoire ni fichier trouve */
                else
                {
                    if (hfile != -1) /* S'il reste un lien sur une recherche de fichier (findfirst(), findnext()) */
                    {
                        if (FIC_FindClose (hfile) == -1) /* Fermer ce lien pour liberer handle systeme */
                            res = -1; /* Signaler erreur */
                        hfile = -1;
                    }
                    
                    /* Si recuperation du repertoire de travail courant OK */
                    if (_getcwd (wdir, _MAX_PATH) != NULL)
                    {
                        /* Si changement vers repertoire pere OK */
                        if (_chdir (pointpoint) == 0)
                        {
                            /* Destruction du repertoire (fils) traite */
                            if (_rmdir (wdir) == -1)
                                res = -1;
                            
                            /* Si recuperation du repertoire de travail courant OK */
                            if (_getcwd (wdir, _MAX_PATH) != NULL)
                            {
                                /* Si on est sur le repertoire a detruire */
                                if (strcmp (path, wdir) == 0)
                                {
                                    /* Destruction du repertoire passe en parametre */
                                    _rmdir (path);
                                    //                            if (rmdir (path) == -1)
                                    //                                res = -1;
                                }
                            }
                        }
                    }
                    else /* Probleme pour recuperer repertoire courant */
                    {
                        res = -1;   /* Indiquer erreur */
                        end = TRUE; /* Fin du traitement */
                    }
                }
            }
      }
      else
      {
          end = TRUE; /* Fin du traitement */
      }
   }
   
   if (hfile != -1) /* S'il reste un lien sur une recherche de fichier (findfirst(), findnext()) */
   {
       if (FIC_FindClose (hfile) == -1) /* Fermer ce lien pour liberer handle systeme */
           res = -1; /* Signaler erreur */
       hfile = -1;
   }
   
   /* Si aucune erreur memorisee sur repertoire courant */
   if (!errdir)
   {
       /* Restaurer repertoire de travail initial */
       _chdir (dirsave);
   }
   
   /* Restaurer l'unite de disque initiale */
   _chdrive (savedisk);
   
   /* Retourner resultat traitement */
   return (res);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC BOOL FIC_FullFindFirst ( char *__FILE, int __LINE, char *p_filter, struct _finddata_t *p_ffblk )
 * PARAMETERS: char *__FILE                : pointeur sur fichier source
 *             int __LINE                  : ligne fichier source
 *             char *p_filter              [IN]  : pointeur sur filtre fichier/repertoire
 *             struct _finddata_t *p_ffblk [OUT] : pointeur sur structure d'information fichier/repertoire
 * RETURN    : identificateur de recherche (-1 si echec)
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recherche dans un repertoire donne un fichier ou repertoire correspondant au filtre passe
 *             et en IGNORANT les repertoires systemes ("." et "..")
 *             Renvoie la structure d'informations sur le fichier ou repertoire trouve
 *             Renvoie egalement l'identificateur de recherche necessaire au FIC_FindNext() et FIC_FindClose()
 * --------------------------------------------------------------------
 */
PUBLIC intptr_t WINAPI FIC_FullFindFirst (char *__FILE, int __LINE, char *p_filter, struct _finddata_t *p_ffblk)
{
    BOOL fin, resultat;
    char *point=".";
    char *pointpoint="..";
    intptr_t hfind;
    
    NO_WARNING( __FILE);
    NO_WARNING( __LINE);

    resultat = FALSE;

    hfind =_findfirst (p_filter, p_ffblk);
    
    if (hfind != -1)
    {    
        fin = FALSE;
        while (!fin)
        {
            /* Si ce n'est pas le repertoire "." du DOS */
            if (strcmp (p_ffblk->name, point) != 0)
                /* Si ce n'est pas le repertoire ".." du DOS */
                if (strcmp (p_ffblk->name, pointpoint) != 0)
                {
                    fin = TRUE;
                    resultat = TRUE;
                }
            if (!fin)
                if (_findnext (hfind, p_ffblk) == -1)
                    fin = TRUE;
        }
    }

    if (!resultat)
    {
        if (hfind != -1)
        {
            _findclose (hfind);
            hfind = -1;
        }
    }

    return hfind;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC int FIC_FullFindNext ( char *__FILE, int __LINE, intptr_t hfind, struct _finddata_t *p_ffblk )
 * PARAMETERS: char *__FILE                : pointeur sur fichier source
 *             int __LINE                  : ligne fichier source
 *             intptr_t hfind              : identificateur de recherche (renvoye par FIC_FindFirst())
 *             struct _finddata_t *p_ffblk [OUT] : pointeur sur structure d'information fichier/repertoire
 * RETURN    : -1 si erreur, 0 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recherche dans un repertoire donne le fichier ou repertoire suivant
 *             et en IGNORANT les repertoires systemes ("." et "..")
 *             Renvoie la structure d'informations sur le fichier ou repertoire trouve
 * --------------------------------------------------------------------
 */
PUBLIC int WINAPI FIC_FullFindNext (char *__FILE, int __LINE, intptr_t hfind, struct _finddata_t *p_ffblk)
{
    BOOL fin;
    char *point=".";
    char *pointpoint="..";
    int res_next;
    
    NO_WARNING( __FILE);
    NO_WARNING( __LINE);

    res_next = -1;

    if (hfind != -1)
    {    
        fin = FALSE;
        while (!fin)
        {
            if ((res_next = _findnext (hfind, p_ffblk)) != -1)
            {
                /* Si ce n'est pas le repertoire "." du DOS */
                if (strcmp (p_ffblk->name, point) != 0)
                    /* Si ce n'est pas le repertoire ".." du DOS */
                    if (strcmp (p_ffblk->name, pointpoint) != 0)
                        fin = TRUE;
            }
            else
                fin = TRUE;
        }
    }

    return res_next;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC int FIC_FullFindClose ( char *__FILE, int __LINE, intptr_t hfind )
 * PARAMETERS: char *__FILE                : pointeur sur fichier source
 *             int __LINE                  : ligne fichier source
 *             intptr_t hfind              : identificateur de recherche (renvoye par FIC_FindFirst())
 * RETURN    : -1 si erreur, 0 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme la recherche (dans un repertoire donne) de fichiers ou repertoires
 * --------------------------------------------------------------------
 */
PUBLIC int WINAPI FIC_FullFindClose (char *__FILE, int __LINE, intptr_t hfind)
{
    int res_close;
    
    NO_WARNING( __FILE);
    NO_WARNING( __LINE);

    res_close = -1;

    if (hfind != -1)
        res_close = _findclose (hfind);

    return res_close;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:     EstRepertoireVide
* PARAMETRES:  Repertoire a tester
* RETOUR:      TRUE si le repertoire est vide
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Determine si le repertoire est vide
* --------------------------------------------------------------------
* NOTA:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean   WINAPI   FIC_FullEstRepertoireVide( char *__FILE, int __LINE, char *pathname)
{
   char              path_local[ _MAX_PATH];
   struct _finddata_t ffblk;
   intptr_t           hfile;

   /* ajouter les jokers */
   STR_strcpy( _MAX_PATH, path_local, pathname);
   STR_strcat( _MAX_PATH, path_local, "\\*.*");

   hfile = FIC_FindFirst( path_local, &ffblk);
   if( hfile != -1)
   {
       FIC_FindClose (hfile);
	   return FALSE; // repertoire non vide
   }
   else
	   return TRUE; // repertoire vide
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: copy
* PARAMETRES:
*            - destination_path,
*            - source_path,
*            - flag_date
* RETOUR: copy_report (TRUE=copie reussie, sinon FALSE)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction publique
* ROLE: copie d un fichier depuis un chemin "source" vers un chemin
*       "destination" en conservant ou pas les dates et heures du
*       ficher source
*       <flag_date> = FIC_COPY_WITH_DATE   : date/heure dst = date/heure src
*       <flag_date> = FIC_COPY_WITHOUT_DATE: date/heure dst = date/heure copie
*
* REMARQUE : CETTE FONCTION EST BLOQUANTE !
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean WINAPI FIC_Fullcopy( char               *__FILE,
                             int                __LINE,
                             char               *destination_path,
                             char               *source_path,
                             FIC_enum_copy_date flag_date)
{

   // par defaut le fichier copier à la meme date que la source
   if (flag_date == FIC_COPY_WITHOUT_DATE)
      ERR_TraiterErreurFatale( __FILE, __LINE);
  
   // copie avec ecrasement si destination existe deja
   return CopyFile (source_path, destination_path, FALSE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_TestFloppy (void)
* PARAMETRES: numero de serie de la disquette en A:
* RETOUR: code de retour de la fonction de reinitialisation du lecteur
*         (interruption 0x13, fonction 0x00)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale a ce module
* ROLE: Etat du lecteur et de la disquette (si presents)
* --------------------------------------------------------------------
* $F_FCTN
*/
FIC_enum_disquette FIC_TestFloppy(DWORD *nSerial)
{
	UINT	nErr;
	DWORD	nVol;
	DWORD	nFlg;
	TCHAR szVol[_MAX_PATH];
	TCHAR	szDsk[] = "A:\\";
	FIC_enum_disquette retour;
	BOOL ret;
	DWORD last_error;
	
	// suppression de la boite d'erreur systeme
	nErr=SetErrorMode(SEM_FAILCRITICALERRORS);
	SetLastError (ERROR_SUCCESS); /* Initialiser code d'erreur pour GetLastError() */
	
	if (ret = GetVolumeInformation(szDsk,szVol,sizeof(szVol),nSerial,&nVol,&nFlg,NULL,0))
		ret = SetVolumeLabel(szDsk,szVol);
	
	last_error = GetLastError();
	switch(last_error)
	{
	case ERROR_SUCCESS:
		/* disquette OK */
		retour = FIC_DISQUETTE_OK;
		break;
		
	case ERROR_PATH_NOT_FOUND:
		/* lecteur absent */
		retour = FIC_DISQUETTE_LECTEUR_NOK;
		break;
		
	case ERROR_WRITE_PROTECT:
		/* disquette protegee en ecriture ... */
		retour = FIC_DISQUETTE_WRITE_PROTECTED;
		break;
		
	case ERROR_NOT_READY:
		/* disquette absente */
		retour = FIC_DISQUETTE_ABSENTE;
		break;
		
	case ERROR_UNRECOGNIZED_MEDIA:
		/* formattage inconnu ou mauvais */
		retour = FIC_DISQUETTE_FORMAT_NOK;
		break;
		
	default :
		retour = FIC_DISQUETTE_NOK;/* autres cas... */
		break;
	}
	
	SetErrorMode(nErr);
	
   return retour;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_FloppyDiskDriveInit (void)
* PARAMETRES: aucun
* RETOUR: code de retour de la fonction de reinitialisation du lecteur
*         (interruption 0x13, fonction 0x00)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale a ce module
* ROLE: Initialisation du lecteur disquette
* --------------------------------------------------------------------
* $F_FCTN
*/
boolean FIC_FloppyDiskDriveInit ( void)
{
   FIC_enum_disquette retour;
    
   retour = FIC_TestFloppy (&CURRENT_SERIAL);
   
   return retour == FIC_DISQUETTE_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_FloppyDiskChanged (void)
* PARAMETRES: aucun
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale a ce module
* ROLE: Indique si c'est une nouvelle disquette, ou s'il n'y a plus
*       rien dans le lecteur
* --------------------------------------------------------------------
* $F_FCTN
*/
FIC_enum_disquette   FIC_FloppyDiskChanged ( void)
{
   DWORD Serial;
   FIC_enum_disquette retour;

   retour = FIC_TestFloppy (&Serial);

   if (retour == FIC_DISQUETTE_OK)
      if (Serial != CURRENT_SERIAL)
      {
         CURRENT_SERIAL = Serial;
         retour = FIC_DISQUETTE_CHANGEE;
      }

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: FIC_FloppyDiskFormatChecking (void)
* PARAMETRES: aucun
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale a ce module
* ROLE: Analyse de la disquette
* --------------------------------------------------------------------
* $F_FCTN
*/
FIC_enum_disquette   FIC_FloppyDiskFormatChecking ( void)
{
   DWORD Serial;
   FIC_enum_disquette retour;

   retour = FIC_TestFloppy (&Serial);

   if (retour == FIC_DISQUETTE_OK)
      if (Serial != CURRENT_SERIAL)
         CURRENT_SERIAL = Serial;

   return retour;
}

