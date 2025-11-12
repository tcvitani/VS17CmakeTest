/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_DBG.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "noyau.h"
#include "noy_loc.h"
//#include "cdg.h"
#include "noy_dbg.h"
#include "reg.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

PRIVATE dbg_struct_trace NOY_TRACES[NOY_NB_TRACES] =
{
   { NOYAU_FAUX, "NOY_INIT" },
   { NOYAU_FAUX, "NOY_BAL" },
   { NOYAU_FAUX, "NOY_CRO" },
   { NOYAU_FAUX, "NOY_EVT" },
   { NOYAU_FAUX, "NOY_MEM" },
   { NOYAU_FAUX, "NOY_BLOC" },
   { NOYAU_FAUX, "NOY_RES" },
   { NOYAU_FAUX, "NOY_RGN" },
   { NOYAU_FAUX, "NOY_SEM" },
   { NOYAU_FAUX, "NOY_TCH" },
   { NOYAU_FAUX, "NOY_TCOU.ECR" },
   { NOYAU_FAUX, "NOY_PILE" }
};

/*--------------- CODE: ---------------*/

PROTECTED unsigned long IncrementeCompteurDebug(void)
{
   return _NOYAU_.debug_ticks++;
}

PUBLIC noyau_enum_booleen InterditDebugNoyau (void)
{
   if (DBG_InterditDebug (&_noyau_.dbg) != DBG_OK)
      return NOYAU_FAUX;

   return NOYAU_VRAI;
}

PUBLIC noyau_enum_booleen AutoriseDebugNoyau (void)
{
   if (DBG_AutoriseDebug (&_noyau_.dbg) != DBG_OK)
      return NOYAU_FAUX;

   return NOYAU_VRAI;
}

PROTECTED noyau_enum_retour LanceDebugNoyau (dbg_struct_debug *debug)
{
    DWORD TailleValeur;

    memset (debug, 0, sizeof (dbg_struct_debug));
	
   // récuparation des répertoires traces et erreurs dans la registry
   TailleValeur = _MAX_PATH;
   if (REG_Lire_Chaine(
        CSR_REG_KEYi_ROOT,
        CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG,
	    NOYAU_REPERTOIRE_TRACES,
	    debug->rep_fichiers_traces,
	    &TailleValeur) != ERROR_SUCCESS)
        return NOYAU_FICHIER_ERREURS_NOK;
   
   TailleValeur = _MAX_PATH;
   if (REG_Lire_Chaine(
        CSR_REG_KEYi_ROOT,
        CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG,
	    NOYAU_REPERTOIRE_ERREURS,
	    debug->rep_fichier_erreurs,
	    &TailleValeur) != ERROR_SUCCESS)
        return NOYAU_FICHIER_ERREURS_NOK;

   strcpy_s(debug->nom_fichier_traces_ecran, _MAX_PATH, NOYAU_FICHIER_TRACES_ECRAN);
   strcpy_s(debug->nom_fichier_erreurs, _MAX_PATH, NOYAU_FICHIER_ERREURS);
   
   // récupération de la taille limite des fichiers
   if (REG_Lire_Entier (
        CSR_REG_KEYi_ROOT, 
        CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG,
        NOYAU_FICHIER_TAILLE_LIMITE, 
        &debug->taille_limite) != ERROR_SUCCESS)
         return NOYAU_FICHIER_ERREURS_NOK;

   memcpy (_noyau_.tab_traces, NOY_TRACES, sizeof(dbg_struct_trace) * NOY_NB_TRACES);
   _NOYAU_.debug_ticks = 0;

   debug->tab_traces = _noyau_.tab_traces;
   debug->nb_fichiers_traces = NOY_NB_TRACES;

   if (DBG_Lance(debug) != DBG_OK)
      return NOYAU_FICHIER_ERREURS_NOK;

   return NOYAU_OK;
}

PROTECTED noyau_enum_retour ArretDebugNoyau (dbg_struct_debug *debug)
{
   if (NOYAU_NB_INSTANCES == 0)
   {
      if (DBG_Arret(debug) != DBG_OK)
         return NOYAU_NOK;
   }

   return NOYAU_OK;
}

PUBLIC enum index_traces TesteTailleFichiersTracesNoyau (void)
{
   return DBG_TesteTailleFichiersTraces (&_noyau_.dbg);
}


PUBLIC void DEFINE_ExitBad(const char *file, int line)
{
   FILE *fichier;
   SYSTEMTIME today;
   errno_t iError;
//	unsigned int far *start;
//	void far (*func)( void );

   EnterRegion ();

   // on bloque ind‚finiment
   GetLocalTime (&today);

   DBG_FILE = file;
   DBG_LINE = line;
   DEFINE_DBG_EcritFichierErreurs (NOY_ERREUR, &_noyau_.dbg,
            "!! EXIT BAD !! ligne %d dans '%s' le %02d/%02d/%02u à %02d:%02d:%02d (Last Error = %lu)",
            line, file,
            today.wDay, today.wMonth, today.wYear%100,
            today.wHour, today.wMinute, today.wSecond,
            GetLastError());

   /* stop le chien de garde */
//   CDG_Arret ();

   /* ferme tous les fichiers */
   _fcloseall();

   _flushall();

   /* teste pr‚sence fichier et si archivage n‚cessaire */
   if (DBG_ArchiveFichierSiTropGros (NOYAU_FICHIER_NO_EXIT_BAD, _noyau_.dbg.taille_limite)
         != DBG_FICHIER_ABSENT)
   {
      /* ouverture en fin de fichier */
	  iError = fopen_s(&fichier, NOYAU_FICHIER_NO_EXIT_BAD, "a+t");
      if (fichier != NULL)
      {
         GetLocalTime (&today);

         /* horodatage de l'exit bad */
         fprintf (fichier, "!! EXIT BAD !! ligne %d dans '%s' le %02d/%02d/%02u à %02d:%02d:%02d\n",
                  line, file,
                  today.wDay, today.wMonth, today.wYear%100,
                  today.wHour, today.wMinute, today.wSecond);

         fflush (fichier);
         fclose (fichier);
      }
   }
   else
   {
      // REBOOT !
/*
	   start = (unsigned int far *) MK_FP (0x40, 0x72);
      *start = 0; // cold start = 0 / warm start = 0x1234
   	func = MK_FP (0xFFFF, 0);
	   func ();
*/
   }

   /* horodatage de l'exit bad */
   printf ("!! EXIT BAD !! ligne %d dans '%s' tache %lu le %02d/%02d/%02u à %02d:%02d:%02d (Last Error = %lu)\n",
            line, file, GetCurrentThreadId(),
            today.wDay, today.wMonth, today.wYear%100,
            today.wHour, today.wMinute, today.wSecond,
            GetLastError());
   
   LeaveRegion ();

   // fin du process !!!!!!!
   ExitProcess(0xBAD);
}

PUBLIC void TraceTout (void)
{

   TraceRegions (_NOYAU_.imbrication_region);
   TraceRegionsLocales (_noyau_.imbrication_region_locale);
   TraceBlocs ();
   TraceBALs ();
//   TraceTaches (_NOYAU_.info.noyau_nb_tache);
}

PROTECTED void TestsPeriodiques (void)
{
/*
   if (DBG_DebugEstInterdit (&_noyau_.dbg))
      return;

   EnterRegion ();

   TestePileTacheCourante ();
   TesteBlocs ();
   TesteTacheCourante ();

   LeaveRegion ();
*/
}

PUBLIC char * NoyCompleteWithInstallPath(char *pszPath)
{	
	char			szBuiltPath[MAX_PATH] = {0};

	if (pszPath != NULL && pszPath[0] == '\\')
	{
		NoyGetInstallPath(szBuiltPath);
		if (strlen(pszPath) + strlen(szBuiltPath) < MAX_PATH)
		{
			strcat_s(szBuiltPath, MAX_PATH, pszPath);
			szBuiltPath[MAX_PATH-1] = '\0';
			strcpy_s(pszPath, MAX_PATH, szBuiltPath);
			return pszPath;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return pszPath;
	}
}

PUBLIC void NoyGetInstallPath(char *pszPath)
{
	DWORD			result = ERROR_SUCCESS, len;	
	char			szKey[MAX_PATH];

	sprintf_s(szKey, MAX_PATH, "%s%s%s", CSR_REG_KEYn_CSRBASE,
							CSR_REG_KEYn_LANE_BASE, 
							CSR_REG_KEYn_CONFIG);
	
	len = MAX_PATH;
	result = REG_Lire_Chaine (CSR_REG_KEYi_ROOT, szKey, NOYAU_REPERTOIRE_APP, pszPath, &len); 
	
	if (result != ERROR_SUCCESS) 
		strcpy_s(pszPath, MAX_PATH, "C:");
}

PUBLIC void NoyGetTracesPath(char *pszPath)	
{
	DWORD			result = ERROR_SUCCESS, len;	
	char			szKey[MAX_PATH];
	
	sprintf_s(szKey, MAX_PATH, "%s%s%s", CSR_REG_KEYn_CSRBASE,
							 CSR_REG_KEYn_LANE_BASE, 
							 CSR_REG_KEYn_CONFIG);
	
	len = MAX_PATH;
	result = REG_Lire_Chaine (CSR_REG_KEYi_ROOT, szKey, NOYAU_REPERTOIRE_TRACES, pszPath, &len); 
	
	if (result != ERROR_SUCCESS) 
		pszPath[0] = '\0';
}

PUBLIC void NoyGetErrorsPath(char *pszPath)	
{
	DWORD			result = ERROR_SUCCESS, len;	
	char			szKey[MAX_PATH];
	
	sprintf_s(szKey, MAX_PATH, "%s%s%s", CSR_REG_KEYn_CSRBASE,
							 CSR_REG_KEYn_LANE_BASE,
							 CSR_REG_KEYn_CONFIG);
	
	len = MAX_PATH;
	result = REG_Lire_Chaine (CSR_REG_KEYi_ROOT, szKey, NOYAU_REPERTOIRE_ERREURS, pszPath, &len); 
	
	if (result != ERROR_SUCCESS) 
		pszPath[0] = '\0';
}
