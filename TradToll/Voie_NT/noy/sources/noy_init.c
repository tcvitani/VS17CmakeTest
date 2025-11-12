/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include "noyau.h" 
//#include "err.h"

#include "noy_cro.h"
#include "noy_dbg.h"


#include "noy_loc.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

#ifdef ERREURS
   #define NOY_COMMENT1 " ERREURS "
#else
   #define NOY_COMMENT1 ""
#endif

#ifdef TRACES
   #define NOY_COMMENT2 " TRACES "
#else
   #define NOY_COMMENT2 ""
#endif

/*** version du noyau ***/
//PRIVATE char  *what = DEFINIR_WHAT_VERSION("NOYAU", "1.0", NOY_COMMENT1 NOY_COMMENT2);

/*--------------- CODE: ---------------*/

#define Error() DEF_Error(__FILE__, __LINE__)

void DEF_Error (const char *file, int line)
{
    LPVOID lpMsgBuf;
	char Title[256] = {0};

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                   FORMAT_MESSAGE_FROM_SYSTEM |     
                   FORMAT_MESSAGE_IGNORE_INSERTS,    
                   NULL,
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                   (LPTSTR) &lpMsgBuf,    
                   0,    
                   NULL );// Process any inserts in lpMsgBuf.

    // ...// Display the string.
	_snprintf_s(Title, sizeof(Title), sizeof(Title)-1, "Error in file '%s' line %d", file, line);

    MessageBox( NULL, (LPCTSTR)lpMsgBuf, Title, MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    LocalFree( lpMsgBuf );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour InitNoyau (struct_noyau *p_noyau,long int *MemSize)
* PARAMETRES: - pointeur sur structure de type noyau
*             - adresse d'un pointeur sur la taille des pools
* RETOUR:
*         - NOYAU_OK:            init Ok
*         - NOYAU_POOL_NOK:      Pb init pool
*         - NOYAU_IOS_INIT_NOK:  Pb init ios
*         - NOYAU_KERNEL_NOK:    Pb d'init du kernel
*         - NOYAU_PB_MALLOC    : pb allocation memoire
*         - NOYAU_IOS_RES_NOK  : pb ios ressource
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui initialise le noyau temps r‚el.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour InitNoyau (void)
{
   noyau_enum_retour retour;

   /* pour supprimer un warning */
//   what = what;
   
   /* Windows won't handle faults by itself */
   SetErrorMode(SEM_FAILCRITICALERRORS|
                SEM_NOGPFAULTERRORBOX|
                SEM_NOOPENFILEERRORBOX);

   memset (&_noyau_, 0, sizeof (struct_local_noyau));
   
   // initialisation des régions
   retour = InitRegion();
   if (retour != NOYAU_OK)
      return retour;

   EnterRegion();
   
   if (NOYAU_NB_INSTANCES == 0)
      memset (&_NOYAU_, 0, sizeof (struct_global_noyau));

   /*** Recupere la structure struct_noyau ***/
//   _NOYAU_.tache_courante = GetCurrentThreadId();

   /*** Initialiser le mode debug */
   retour = LanceDebugNoyau(&_noyau_.dbg);
   if (retour != NOYAU_OK)
   {
      Error();
      return retour;
   }

   // Initialisation des blocks mémoire
   InitMem(_noyau_.dbg.nom_fichier_erreurs);

   // initialisation des BAL
   retour = InitBAL();
   if (retour != NOYAU_OK)
   {
      Error();
      return retour;
   }

   /*** Récupérer infos reseau ***/
//   _NOYAU_.info_reseau = infos_reseau;

   /*** Initialisation des chronomètres ***/
   retour = Noyau_Init_Chrono ();
   if (retour != NOYAU_OK)
   {
      Error();
      return retour;
   }

   /* InitNoyau OK */
#ifdef TRACES
   RtcFichierTrace (NOY_INIT, &_noyau_.dbg, "InitNoyau() : instances %ld OK", NOYAU_NB_INSTANCES);
#endif

   // pour actions particulières première instance
   NOYAU_NB_INSTANCES++;

   LeaveRegion();

   return NOYAU_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ArretNoyau(void)
* PARAMETRES:
* ENTREE:  - poineur sur la structure noyau
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Reseter le noyau .
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour ArretNoyau( void )
{

   /* These calls are not protected by csr_excpt as they are called when
      noyau dll is unloaded.
      Protect the last tracing so that an exception in those doesn't
      use the default windows exception handler */
   __try
   {
      EnterRegion();

	  /* There's a higher chance of exception in trace functions */
      __try
      {
#ifdef TRACES
         RtcFichierTrace(NOY_INIT, &_noyau_.dbg, "ArretNoyau() : instance %ld PID =%ld", NOYAU_NB_INSTANCES, GetCurrentProcessId());
#endif
         /* lance les traces */
         TraceBALs ();
         TraceBlocs ();
	  }
	  __except(EXCEPTION_EXECUTE_HANDLER){}

      if (NOYAU_NB_INSTANCES > 0)
         NOYAU_NB_INSTANCES --;

      LeaveRegion();
   }
   __except(EXCEPTION_EXECUTE_HANDLER){}

   /* tout c'est bien pass‚ */
   return NOYAU_OK;
}


extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    DBG_DebutDebugSansErreurs(&_noyau_.dbg);

    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:
           if (InitNoyau () != NOYAU_OK)
           {
              return FALSE;
           }
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  
           if (ArretNoyau () != NOYAU_OK)
              return FALSE;
            break;          

         default:             
            break;     
    }      
    
    DBG_FinDebugSansErreurs(&_noyau_.dbg);    
    
    return TRUE; 

    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

