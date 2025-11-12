/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: 
* FICHIER: DEBUG.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: 
* --------------------------------------------------------------------
* DESCRIPTION: 
* --------------------------------------------------------------------
* HISTORIQUE: 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef DEBUG_H
#define DEBUG_H


/*--------------- INCLUDES: ---------------*/

#include <stdlib.h>

#ifdef NOY_DEF
   #include <public.h>
#else
   #include <export.h>
#endif

/*--------------- DEFINES: ---------------*/

#define DBG_EcritFichierErreurs DBG_FILE=__FILE__,\
								DBG_LINE=__LINE__,\
								DEFINE_DBG_EcritFichierErreurs

/*--------------- TYPEDEFS: ---------------*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   DBG_OK,
   DBG_NULL,
   DBG_REPERTOIRE_ERREURS_NOK,
   DBG_FICHIER_ERREURS_NOK
}
dbg_enum_retour;

typedef enum
{
   DBG_FICHIER_NOK,
   DBG_FICHIER_ABSENT,
   DBG_FICHIER_OK,
   DBG_FICHIER_ARCHIVE
}
dbg_enum_archive;

typedef struct
{
   noyau_enum_booleen est_actif;
   char nom[_MAX_PATH];
   void *emet;
}
dbg_struct_trace;

typedef struct
{
   // PUBLIC
   char rep_fichiers_traces[_MAX_DRIVE+_MAX_DIR];
   char nom_fichier_traces_ecran[_MAX_PATH];
   char rep_fichier_erreurs[_MAX_DRIVE+_MAX_DIR];
   char nom_fichier_erreurs[_MAX_PATH];
   long taille_limite;

   dbg_struct_trace *tab_traces;
   enum index_traces nb_fichiers_traces;


   // PRIVATE
   struct {
      noyau_enum_booleen sans_erreurs;
      noyau_enum_booleen interdit;
      noyau_enum_booleen traces_ecran_activees;
      enum index_traces index_test_traces;
      void *emet;
   } priv;
}
dbg_struct_debug;

/*--------------- VARIABLES: ---------------*/

EXPORT const char *DBG_FILE;
EXPORT int DBG_LINE;

/*--------------- FUNCTIONS: ---------------*/

EXPORT dbg_enum_retour WINAPI DBG_Lance (dbg_struct_debug *debug);
EXPORT dbg_enum_retour WINAPI DBG_Arret (dbg_struct_debug *debug);

EXPORT dbg_enum_retour WINAPI DBG_InterditDebug (dbg_struct_debug *debug);
EXPORT noyau_enum_booleen WINAPI DBG_DebugEstInterdit (dbg_struct_debug *debug);
EXPORT dbg_enum_retour WINAPI DBG_AutoriseDebug (dbg_struct_debug *debug);

EXPORT void WINAPI DEFINE_DBG_EcritFichierErreurs (enum index_traces nom_trace, dbg_struct_debug *debug, char *fmt,...);
EXPORT dbg_enum_retour WINAPI DBG_DebutDebugSansErreurs (dbg_struct_debug *debug);
EXPORT dbg_enum_retour WINAPI DBG_FinDebugSansErreurs (dbg_struct_debug *debug);

EXPORT void WINAPI DBG_EcritFichierTraces (enum index_traces nom_trace, dbg_struct_debug *debug, char *fmt,...);
EXPORT noyau_enum_booleen WINAPI DBG_FichierTracesPresent (enum index_traces nom_trace, dbg_struct_debug *debug);
EXPORT enum index_traces WINAPI DBG_TesteTailleFichiersTraces (dbg_struct_debug *debug);

EXPORT dbg_enum_archive WINAPI DBG_ArchiveFichierSiTropGros (char *chemin_fichier,
                                                      unsigned long taille_limite);
EXPORT int WINAPI DBG_RAZFichiers (char *chemin_fichiers);

EXPORT noyau_enum_retour WINAPI DBG_Flush (FILE *file);

#ifdef __cplusplus
}
#endif

#undef I
#undef INIT
#undef PROTECTED
#endif /* DEBUG_H */
