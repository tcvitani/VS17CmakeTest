/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Encapsulation des fonctions d'accŠs aux fichiers
* FICHIER: FIC.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Les fonctions standard d'accŠs aux fichiers sont encapsul‚s
*         pour effectuer un traitement d'erreur fatale en cas d'‚chec
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d'interface
*              Toutes ces fonctions en cas d'‚chec partent en
*              Erreur Fatale ( sauf si elles ne peuvent ‚chouer)
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/FIC.H_v  $
 * 
 *    Rev 1.4   Mar 02 1999 17:54:50   afx
 *  
 * 
 *    Rev 1.3   Feb 09 1999 10:32:18   afx
 * - Correction FIC_deltree().
 * - Ajout surcouche pour findfirst(), findnext() et findclose()
 *   (FIC_FindFist() et FIC_FindNext() ignorent les repertoires systemes "." et "..").
 * 
 * 
 *    Rev 1.2   Jan 20 1999 11:29:44   bph
 * passage en WINAPI
 * 
 *    Rev 1.1   02 Oct 1998 11:42:04   bph
 *  
 * 
 *    Rev 1.10   17 Mar 1998 18:12:52   DPI
 * Ajout des fonctions FIC_Floppy
 * 
 *    Rev 1.9   08 Jan 1998 17:48:36   DPI
 * Ajout des fonctions :
 * - FIC_GetFreeFileHandlers
 * - FIC_SetFileHandlersTable
 * - FIC_ResetFileHandlersTable
 * 
 *    Rev 1.8   Jul 28 1997 16:44:06   DPI
 *  
 * 
 *    Rev 1.7   Jul 24 1997 14:07:18   DPI
 * Ajout des fonctions :
 * - FIC_EstRepertoireVide
 * - FIC_copy
 * 
 *    Rev 1.6   Apr 09 1997 16:59:48   DPI
 * Ajout de 'FIC_deltree'
 * 
 *    Rev 1.5   Mar 04 1997 12:39:08   HMO
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef FIC_H
#define FIC_H


/*--------------- INCLUDES: ---------------*/
#ifdef FIC_DEF
   #include <public.h>
#else
   #include <export.h>
#endif

#include <windows.h>
#include <stdio.h>
#include <io.h>

/*--------------- CONSTANTES: ----------------*/

#define MAX_HDL 254

typedef enum
{
   FIC_COPY_WITH_DATE,
   FIC_COPY_WITHOUT_DATE
}FIC_enum_copy_date;

typedef enum
{
   FIC_DISQUETTE_OK,
   FIC_DISQUETTE_NOK,
   FIC_DISQUETTE_LECTEUR_NOK,
   FIC_DISQUETTE_ABSENTE,
   FIC_DISQUETTE_FORMAT_NOK,
   FIC_DISQUETTE_WRITE_PROTECTED,
   FIC_DISQUETTE_CHANGEE,
} FIC_enum_disquette;

/*--------------- FUNCTIONS: --------------*/

#define FIC_fcloseall()            FIC_Fullfcloseall  (__FILE__, __LINE__)
#define FIC_fflush(a)              FIC_Fullfflush     (__FILE__, __LINE__, a)
#define FIC_fgets(a,b,c)           FIC_Fullfgets      (__FILE__, __LINE__,a,b,c)
#define FIC_remove(a)              FIC_Fullremove     (__FILE__, __LINE__,a)
#define FIC_chsize(a,b)            FIC_Fullchsize     (__FILE__, __LINE__,a,b)
#define FIC_fprintf                FIC_Fullfprintf
#define FIC_mktemp(a)              FIC_Fullmktemp     (__FILE__, __LINE__,a)
#define FIC_rename(a,b)            FIC_Fullrename     (__FILE__, __LINE__,a,b)
#define FIC_system(a)              FIC_Fullsystem     (__FILE__, __LINE__,a)
#define FIC_mkdir(a)               FIC_Fullmkdir      (__FILE__, __LINE__,a)
#define FIC_rmdir(a)               FIC_Fullrmdir      (__FILE__, __LINE__,a)
#define FIC_unlink(a)              FIC_Fullunlink     (__FILE__, __LINE__,a)
#define FIC_makepath(a,b,c,d,e)    FIC_Full_makepath (__FILE__, __LINE__,a,b,c,d,e)
#define FIC_splitpath(a,b,c,d,e)   FIC_Full_splitpath (__FILE__, __LINE__,a,b,c,d,e)

#define FIC_fopen(a,b)             FIC_Fullfopen      (__FILE__, __LINE__,a,b)
#define FIC_fclose(a)              FIC_Fullfclose     (__FILE__, __LINE__,a)
#define FIC_fwrite(a,b,c,d)        FIC_Fullfwrite     (__FILE__, __LINE__,a,b,c,d)
#define FIC_fread(a,b,c,d)         FIC_Fullfread      (__FILE__, __LINE__,a,b,c,d)
#define FIC_ftell(a)               FIC_Fullftell      (__FILE__, __LINE__,a)
#define FIC_fseek(a,b,c)           FIC_Fullfseek      (__FILE__, __LINE__,a,b,c)
#define FIC_rewind(a)              FIC_Fullrewind     (__FILE__, __LINE__,a)

#define FIC_open(a,b,c)            FIC_Fullopen3      (__FILE__, __LINE__,a,b,c)
#define FIC_close(a)               FIC_Fullclose      (__FILE__, __LINE__,a)
#define FIC_write(a,b,c)           FIC_Fullwrite      (__FILE__, __LINE__,a,b,c)
#define FIC_read(a,b,c)            FIC_Fullread       (__FILE__, __LINE__,a,b,c)
#define FIC_tell(a)                FIC_Fulltell       (__FILE__, __LINE__,a)
#define FIC_lseek(a,b,c)           FIC_Fulllseek      (__FILE__, __LINE__,a,b,c)

#define FIC_filelength(a)          FIC_Fullfilelength (__FILE__, __LINE__,a)
#define FIC_setbuf(a,b)            FIC_Fullsetbuf     (__FILE__, __LINE__,a,b)
#define FIC_getftime(a,b)          FIC_Fullgetftime   (__FILE__, __LINE__,a,b)
#define FIC_setftime(a,b)          FIC_Fullgetftime   (__FILE__, __LINE__,a,b)
#define FIC_dup(a)                 FIC_Fulldup        (__FILE__, __LINE__,a)

#define FIC_FindFirst(a,b)         FIC_FullFindFirst  (__FILE__, __LINE__, a, b)
#define FIC_FindNext(a,b)          FIC_FullFindNext   (__FILE__, __LINE__, a, b)
#define FIC_FindClose(a)           FIC_FullFindClose  (__FILE__, __LINE__, a)

#define FIC_CreatePath(a)          FIC_FullCreatePath (__FILE__, __LINE__, a)
#define FIC_deltree(a)             FIC_Fulldeltree    (__FILE__, __LINE__, a)

#define FIC_EstRepertoireVide(a)   FIC_FullEstRepertoireVide    (__FILE__, __LINE__,a)
#define FIC_copy(a,b,c)            FIC_Fullcopy       (__FILE__, __LINE__,a,b,c)

#define FIC_SetFileHandlersTable(a)  FIC_FullSetFileHandlersTable   (__FILE__, __LINE__,a)
#define FIC_ResetFileHandlersTable() FIC_FullResetFileHandlersTable (__FILE__, __LINE__)


EXPORT int      WINAPI FIC_Fullfcloseall  ( char *__FILE, int __LINE);
EXPORT int      WINAPI FIC_Fullfflush     ( char *__FILE, int __LINE, FILE *stream);
EXPORT char    *WINAPI FIC_Fullfgets      ( char *__FILE, int __LINE, char *s, int n, FILE *stream);
EXPORT int      WINAPI FIC_Fullremove     ( char *__FILE, int __LINE, char *__FILEname);
EXPORT int      WINAPI FIC_Fullchsize     ( char *__FILE, int __LINE, int handle, long size);
EXPORT int      WINAPI FIC_Fullfprintf    ( FILE *stream, char *format, ...);
EXPORT char    *WINAPI FIC_Fullmktemp     ( char *__FILE, int __LINE, char *temp);
EXPORT int      WINAPI FIC_Fullrename     ( char *__FILE, int __LINE, char *oldname, char *newname);
EXPORT int      WINAPI FIC_Fullsystem     ( char *__FILE, int __LINE, char *command);
EXPORT int      WINAPI FIC_Fullmkdir     ( char *__FILE, int __LINE, char *path);
EXPORT int      WINAPI FIC_Fullrmdir     ( char *__FILE, int __LINE, char *path);
EXPORT int      WINAPI FIC_Fullunlink     ( char *__FILE, int __LINE, char *fichier);
EXPORT void     WINAPI FIC_Full_makepath( char *__FILE, int __LINE, char *path, char *drive, char *dir, char *name, char *ext);
EXPORT void     WINAPI FIC_Full_splitpath( char *__FILE, int __LINE, char *path, char *drive, char *dir, char *name, char *ext);

EXPORT FILE    *WINAPI FIC_Fullfopen      ( char *__FILE, int __LINE, char *__FILEname, char *mode);
EXPORT int      WINAPI FIC_Fullfclose     ( char *__FILE, int __LINE, FILE *stream);
EXPORT size_t   WINAPI FIC_Fullfwrite     ( char *__FILE, int __LINE, void *ptr, size_t size, size_t n, FILE *stream);
EXPORT size_t   WINAPI FIC_Fullfread      ( char *__FILE, int __LINE, void *ptr, size_t size, size_t n, FILE *stream);
EXPORT long     WINAPI FIC_Fullftell      ( char *__FILE, int __LINE, FILE *stream);
EXPORT int      WINAPI FIC_Fullfseek      ( char *__FILE, int __LINE, FILE *stream, long offset, int whenc);
EXPORT void     WINAPI FIC_Fullrewind     ( char *__FILE, int __LINE, FILE *stream);

EXPORT int      WINAPI FIC_Fullopen2      ( char *__FILE, int __LINE, char *path, int access);
EXPORT int      WINAPI FIC_Fullopen3      ( char *__FILE, int __LINE, char *path, int access, unsigned mode);
EXPORT int      WINAPI FIC_Fullclose      ( char *__FILE, int __LINE, int handle);
EXPORT int      WINAPI FIC_Fullwrite      ( char *__FILE, int __LINE, int handle, void *buf, unsigned len);
EXPORT int      WINAPI FIC_Fullread       ( char *__FILE, int __LINE, int handle, void *buf, unsigned len);
EXPORT long     WINAPI FIC_Fulltell       ( char *__FILE, int __LINE, int handle);
EXPORT long     WINAPI FIC_Fulllseek      ( char *__FILE, int __LINE, int handle, long offset, int fromwher);
EXPORT long     WINAPI FIC_Fullfilelength ( char *__FILE, int __LINE, int handle);
EXPORT void     WINAPI FIC_Fullsetbuf     ( char *__FILE, int __LINE, FILE *stream, char *buf);
EXPORT int      WINAPI FIC_Fullgetftime   ( char *__FILE, int __LINE, int handle, FILETIME *ftimep);
EXPORT int      WINAPI FIC_Fullsetftime   ( char *__FILE, int __LINE, int handle, FILETIME *ftimep);
EXPORT int      WINAPI FIC_Fulldup        ( char *__FILE, int __LINE, int handle);

EXPORT intptr_t WINAPI FIC_FullFindFirst  (char *__FILE, int __LINE, char *p_filter, struct _finddata_t *p_ffblk);
EXPORT int      WINAPI FIC_FullFindNext   (char *__FILE, int __LINE, intptr_t hfind, struct _finddata_t *p_ffblk);
EXPORT int      WINAPI FIC_FullFindClose  (char *__FILE, int __LINE, intptr_t hfind);

EXPORT BOOL     WINAPI FIC_FullCreatePath ( char *__FILE, int __LINE, char *pathname);
EXPORT int      WINAPI FIC_Fulldeltree    ( char *__FILE, int __LINE, char *pathname);

EXPORT boolean  WINAPI FIC_FullEstRepertoireVide    ( char *__FILE, int __LINE, char *pathname);
EXPORT boolean  WINAPI FIC_Fullcopy       ( char               *__FILE,
                                            int                __LINE,
                                            char               *destination_path,
                                            char               *source_path,
                                            FIC_enum_copy_date flag_date);


/* ces fonctions ne levent pas d'erreurs */
EXPORT boolean              WINAPI FIC_FloppyDiskDriveInit( void);
/* FIC_FloppyDiskChanged doit ˆtre appel‚e avant FIC_...FormatChecking
 * et ne retourne pas FIC_DISQUETTE_WRITE_PROTECTED */
EXPORT FIC_enum_disquette   WINAPI FIC_FloppyDiskChanged ( void);
/* FIC_...FormatChecking ne retourne pas FIC_DISQUETTE_CHANGEE */
EXPORT FIC_enum_disquette   WINAPI FIC_FloppyDiskFormatChecking ( void);

#endif
