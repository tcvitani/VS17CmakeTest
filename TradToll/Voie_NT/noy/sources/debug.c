/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: DEBUG.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <io.h>

#include "noyau.h"
#include "noy_loc.h"
#include <trc.h>

#define LOC_DEF
#include "debug.h"
#undef LOC_DEF

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/

PROTECTED const char *DBG_FILE;
PROTECTED int DBG_LINE;

/*--------------- DEFINES: ---------------*/
#define FMT_ERREUR "*ERREUR* "

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

PRIVATE dbg_enum_archive DBG_ArchiveFichierErreursSiTropGros (dbg_struct_debug *debug);
PRIVATE dbg_enum_archive DBG_ArchiveFichierTracesSiTropGros (enum index_traces nom_trace,
                                                             dbg_struct_debug *debug);
/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

PRIVATE int CreatePath (char *path)
{
    char *token;
    char full_path[_MAX_PATH];


    if (_fullpath(full_path, path, _MAX_PATH) == NULL)
        return FALSE;

    // recherche du premier '\' normalement juste apres le ':' avec un path absolu
    token = strchr(full_path, '\\');
    token++;

    // création des sous-répertoires si inexistant
    while ((token = strchr(token, '\\')) != NULL)
    {
        *token = '\0';

        if (_access(full_path, 0) != 0)
            if(!CreateDirectory(full_path, NULL))
            {
                *token = '\\';
                return FALSE;
            }

        *token = '\\';
        token++;
    }

    // création du répertoire final si inexistant
    if (_access(full_path, 0) != 0)
        if(!CreateDirectory(full_path, NULL))
            return FALSE;

    return TRUE;
}
        
PUBLIC dbg_enum_retour DBG_Lance (dbg_struct_debug *debug)
{
    enum index_traces i;
    char path[_MAX_PATH+1];
    
    if (debug == NULL)
        return DBG_NULL;
    
    /* init de la partie privee de debug */
    memset (&debug->priv, 0, sizeof (debug->priv));
    
    /* creation du chemin complet vers le fichier de traces ecran */
    _snprintf_s (path,_MAX_PATH, MAX_PATH, "%s\\%s" NOYAU_EXTENSION_ECRAN, debug->rep_fichiers_traces, debug->nom_fichier_traces_ecran);
	strcpy_s(debug->nom_fichier_traces_ecran, _MAX_PATH, path);
    
    /* creation du chemin complet vers le fichier d'erreurs */
	_snprintf_s(path, _MAX_PATH, MAX_PATH, "%s\\%s" NOYAU_EXTENSION_ERREURS, debug->rep_fichier_erreurs, debug->nom_fichier_erreurs);
	strcpy_s(debug->nom_fichier_erreurs, _MAX_PATH, path);
    
    /* creation du repertoire d'erreurs */
    if (_access (debug->rep_fichier_erreurs, 0) != 0 )
        if (!CreatePath (debug->rep_fichier_erreurs))
            return DBG_REPERTOIRE_ERREURS_NOK;

    /* creation du repertoire de traces */
    if (_access (debug->rep_fichiers_traces, 0) != 0 )
        if (!CreatePath (debug->rep_fichiers_traces))
            return DBG_REPERTOIRE_ERREURS_NOK;

    /* initialise le fichier de debug */
    TRC_Initialise_Trace ("DEBUG", debug->nom_fichier_erreurs, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_TEXTE_SEUL|TRC_OPT_CONSOLE, (TRC_EMETTEUR *) &debug->priv.emet);
    TRC_Taille_Max_Fichier (debug->priv.emet, debug->taille_limite);

    /* teste le fichier de debug */
    TRC_Trace_Texte (debug->priv.emet, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, " => Debut du debug");
    
    /* cree les chemins complets de tous les fichiers de traces */
    for (i = 0; i < debug->nb_fichiers_traces; i++)
    {
        if (strchr (debug->tab_traces[i].nom, '.') == NULL)
        {
            /* creation du chemin complet vers le fichier de traces avec ajout de l'extension par defaut */
			_snprintf_s(path, _MAX_PATH, MAX_PATH, "%s\\%s" NOYAU_EXTENSION_TRACES, debug->rep_fichiers_traces, debug->tab_traces[i].nom);
        }
        else
        {
            /* creation du chemin complet vers le fichier de traces */
			_snprintf_s(path, _MAX_PATH, MAX_PATH, "%s\\%s", debug->rep_fichiers_traces, debug->tab_traces[i].nom);
        }
		strcpy_s(debug->tab_traces[i].nom, _MAX_PATH, path);
    }
    
    /* initialise les fichiers de traces */
    for (i = 0; i < debug->nb_fichiers_traces; i++)
    {
       TRC_Initialise_Trace ("TRACE", debug->tab_traces[i].nom, TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *) &debug->tab_traces[i].emet);
       TRC_Taille_Max_Fichier (debug->tab_traces[i].emet, debug->taille_limite);    
    }

    return DBG_OK;
}

PUBLIC enum index_traces DBG_TesteTailleFichiersTraces (dbg_struct_debug *debug)
{
    return DBG_FICHIER_OK;
}

PUBLIC dbg_enum_retour DBG_Arret (dbg_struct_debug *debug)
{
    int i;

    if (debug == NULL)
        return DBG_NULL;

    /* ferme fichier de debug */
    TRC_Trace_Texte (debug->priv.emet, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, " <= Fin du debug");
    
    TRC_Termine_Trace (debug->priv.emet);
    
    for (i = 0; i < debug->nb_fichiers_traces; i++)
    {
        TRC_Termine_Trace (debug->tab_traces[i].emet);
    }
    
    return DBG_OK;
}

#define MAX_STRING 16384

PUBLIC void DEFINE_DBG_EcritFichierErreurs (enum index_traces nom_trace, dbg_struct_debug *debug, char *fmt,...)
{
    va_list ap;
	char string[MAX_STRING+1] = { 0 };

	strcat_s(string, sizeof(string), FMT_ERREUR);

    if (debug == NULL)
        return;

    if (nom_trace > debug->nb_fichiers_traces)
        return;
    
    if (debug->priv.interdit == NOYAU_VRAI)
        return;
    
    if (NoyauEstInitialise() == NOYAU_VRAI)
        EnterCriticalSection (&_noyau_.csDbg);
    
    //va_start (ap,fmt);
    
    /* si est une erreur alors horodatage */
    if (debug->priv.sans_erreurs == NOYAU_FAUX)
		_snprintf_s(&string[sizeof(FMT_ERREUR) - 1], MAX_STRING - sizeof(FMT_ERREUR), MAX_STRING - sizeof(FMT_ERREUR) - 1, "ligne %d dans '%s'\n\r   => ", DBG_LINE, DBG_FILE);
    
    /* ecrit les infos */
    va_start(ap, fmt);

	_vsnprintf_s(&string[strlen(string) - 1], MAX_STRING - strlen(string) - 1, MAX_STRING - strlen(string) - 1, fmt, ap);
    
    if (debug->priv.sans_erreurs == NOYAU_FAUX)
		TRC_Direct_Trace_V(debug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, NULL, 0, string, NULL);
    else
		TRC_Direct_Trace_V(debug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_TEXTE_SEUL | TRC_OPT_CONSOLE, NULL, 0, string, NULL);
	
	//va_end(ap);

    /* reporte l'erreur dans le fichier de traces correspondant */
    
	//va_start (ap,fmt);
	
	//_vsnprintf_s(&string[sizeof(FMT_ERREUR) - 1], MAX_STRING - sizeof(FMT_ERREUR), MAX_STRING - sizeof(FMT_ERREUR) - 1, fmt, ap);
    
	/* si est une erreur alors le signaler dans traces */
	if(nom_trace < debug->nb_fichiers_traces && debug->tab_traces!=NULL )
	{
		if (debug->priv.sans_erreurs == NOYAU_FAUX && nom_trace < debug->nb_fichiers_traces)
			TRC_Trace_V(debug->tab_traces[nom_trace].emet, TRC_OPT_MASK, NULL, 0, string, NULL);
			//DBG_EcritFichierTraces (nom_trace, debug, string);
		else
			TRC_Trace_V(debug->tab_traces[nom_trace].emet, TRC_OPT_MASK, NULL, 0, &string[sizeof(FMT_ERREUR) - 1], NULL);
			//DBG_EcritFichierTraces(nom_trace, debug, &string[sizeof(FMT_ERREUR) - 1]);
	}
    va_end(ap);
    
    if (NoyauEstInitialise() == NOYAU_VRAI)
        LeaveCriticalSection (&_noyau_.csDbg);
}

PUBLIC dbg_enum_retour DBG_DebutDebugSansErreurs (dbg_struct_debug *debug)
{
    if (debug == NULL)
        return DBG_NULL;
    
    debug->priv.sans_erreurs = NOYAU_VRAI;
    
    return DBG_OK;
}

PUBLIC noyau_enum_retour DBG_Flush (FILE *file)
{
    TRC_Vider_Traces(0);

    /* flush les tampons du C */
    if (_commit (_fileno(file)) != 0)
        return NOYAU_NOK;
   
    return NOYAU_OK;
}

PUBLIC dbg_enum_retour DBG_FinDebugSansErreurs (dbg_struct_debug *debug)
{
    if (debug == NULL)
        return DBG_NULL;
    
    debug->priv.sans_erreurs = NOYAU_FAUX;
    
    return DBG_OK;
}

PUBLIC dbg_enum_retour DBG_InterditDebug (dbg_struct_debug *debug)
{
    if (debug == NULL)
        return DBG_NULL;
    
    debug->priv.interdit = NOYAU_VRAI;
    
    return DBG_OK;
}

PUBLIC noyau_enum_booleen DBG_DebugEstInterdit (dbg_struct_debug *debug)
{
    return debug->priv.interdit;
}

PUBLIC dbg_enum_retour DBG_AutoriseDebug (dbg_struct_debug *debug)
{
    if (debug == NULL)
        return DBG_NULL;
    
    debug->priv.interdit = NOYAU_FAUX;
    
    return DBG_OK;
}



PUBLIC noyau_enum_booleen DBG_FichierTracesPresent (enum index_traces nom_trace, dbg_struct_debug *debug)
{
    return TRC_Fichier_Actif (debug->tab_traces[nom_trace].emet);
}

PUBLIC void DBG_EcritFichierTraces (enum index_traces nom_trace, dbg_struct_debug *debug, char *fmt,...)
{
    va_list ap;
    //   short int x,y;
    dbg_struct_trace *fichier_traces;
    
    if (debug == NULL)
        return;

    if (nom_trace >= debug->nb_fichiers_traces)
        return;
    
    if (debug->priv.interdit == NOYAU_VRAI)
        return;

    fichier_traces = &debug->tab_traces[nom_trace];
    
// en ne faisant pas le test suivant, on peut créer un fichier de trace
// qui sera pris en compte en cours d'exécution (BPH 12/5/00)
//    if (!TRC_Fichier_Actif(fichier_traces->emet))
//        return;
    
    if (NoyauEstInitialise() == NOYAU_VRAI)
        EnterCriticalSection (&_noyau_.csDbg);

    
    /* ecrit la trace */
    va_start (ap,fmt);

//	TIC 2012/01/07 - the trace should never use DIRECT writing to file
//    if (debug->priv.sans_erreurs == NOYAU_FAUX)
        TRC_Trace_V (fichier_traces->emet, TRC_OPT_MASK, NULL, 0, fmt, ap);
//    else
//        TRC_Direct_Trace_V (fichier_traces->emet, TRC_OPT_MASK|TRC_OPT_CONSOLE, NULL, 0, fmt, ap);

    va_end (ap);
    
    if (NoyauEstInitialise() == NOYAU_VRAI)
        LeaveCriticalSection (&_noyau_.csDbg);
}

PUBLIC int DBG_RAZFichiers (char *chemin_fichiers)
{
	struct _finddata_t ffblk = { 0 };
    FILE *handle;
	intptr_t hfile;
    int done;
    int count;
	char file_path[_MAX_PATH] = {0};
	char drive[_MAX_DRIVE] = { 0 };
	char dir[_MAX_DIR] = { 0 };
	errno_t iError;
    
    hfile = _findfirst(chemin_fichiers, &ffblk);
    if (hfile == -1L)
        return 0;
    
	_splitpath_s(chemin_fichiers, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL,0, NULL,0);
    
    count = 0;
    done = 0;
    while (done == 0)
    {
		_snprintf_s(file_path, _MAX_PATH, _MAX_PATH-1, "%s%s%s", drive, dir, ffblk.name);
        
		if ((iError = fopen_s(&handle, file_path, "w+b") == 0)) //if no error
        {
            if (_chsize (_fileno(handle), 0L) == 0)
                count ++;
            fclose(handle);
        }
        done = _findnext(hfile, &ffblk);
    }
    
    _findclose (hfile);
    
    return count;
}

PUBLIC dbg_enum_archive DBG_ArchiveFichierSiTropGros (char *chemin_fichier,
                                                      unsigned long taille_limite)
{
    struct _finddata_t ffblk;
	intptr_t hfile;
	char copie[_MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE] = { 0 };
	char dir[_MAX_DIR] = { 0 };
	char file[_MAX_FNAME] = { 0 };
	char ext[_MAX_EXT] = { 0 };
    
    /* test pr‚sence fichier et r‚cupŠre sa taille */
    hfile = _findfirst(chemin_fichier, &ffblk);
    if (hfile == -1L)
        return DBG_FICHIER_ABSENT;
    
    _findclose (hfile);
    
    /* si taille fichier < limite admise */
    if (ffblk.size < taille_limite)
        return DBG_FICHIER_OK;
    
    /* construit fichier avec extension ".arc" … partir du fichier courant */
 	_splitpath_s(chemin_fichier, drive, _MAX_DRIVE, dir, _MAX_DIR, file, _MAX_FNAME, ext, _MAX_EXT);
	_snprintf_s(copie, _MAX_PATH, _MAX_PATH-1, "%s%s%s.ARC", drive, dir, file);
    
    /* d‚truit un ‚ventuel fichier.arc */
    remove (copie);
    /* renomme fichier courant en .arc */
    rename (chemin_fichier, copie);
    
    return DBG_FICHIER_ARCHIVE;
}
