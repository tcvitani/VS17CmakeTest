/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_MEM.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

//#ifndef _DEBUG
	#include <malloc.h>
//#endif

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/

#define FMT_ADR " { %p }"
#define FMT_TACHE " ds { tache %d }"
#define FMT_ADR_TACHE_POOL_TAILLE " { %p / tache %lu / pool %u / taille %lu }"
#define FMT_BLOC " { @%p / pool %u / taille %lu / tache %d / segid %lx }"
#define FMT_SEG_ID " / { segid %lx }"


/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/

PRIVATE noyau_enum_retour InsereBloc (void *adr, noyau_taille_bloc taille, noyau_pool_id pool_id);
PRIVATE noyau_enum_retour SupprimeBloc (void *adr);

/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED noyau_enum_retour AlloueBloc (void **adresse,
*                                                  unsigned short int longueur,
*                                                  unsigned short int numero_pool)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - pointeur de pointeur sur un bloc de pool IOS
*              - taille de la memoire a allouer (en octets)
*              - indice du pool
*
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui alloue de la m‚moire dans un pool IOS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour AlloueBloc (void **pp_neutre,
                                        noyau_taille_bloc longueur,
                                        noyau_pool_id numero_pool)
{
    // numero_pool n'est plus utilisé -> par defaut on prend le heap du process
    numero_pool = numero_pool;
    
//    EnterRegion();
    
    /*** Appel a la procedure d'allocation de memoire ***/
    *pp_neutre = malloc (longueur);

    /*** Si l'allocation s'est bien pass‚e ***/
    if (*pp_neutre == NULL)
    {
        /*** Appel au fichier trace ***/
#ifdef ERREURS
        RtcFichierDebug(NOY_MEM, &_noyau_.dbg, "Alloue() :" FMT_ADR_TACHE_POOL_TAILLE,
            *pp_neutre, GetCurrentThreadId(), numero_pool, longueur);
#endif
        
//        LeaveRegion();
        
        /*** For‡age de la valeur de retour … NOK ***/
        return NOYAU_NOK;
    }
    
    memset (*pp_neutre, 0, longueur);    

#ifdef TRACES
    RtcFichierTrace(NOY_MEM, &_noyau_.dbg, "Alloue() :" FMT_ADR_TACHE_POOL_TAILLE,
        *pp_neutre, GetCurrentThreadId(), numero_pool, longueur);
#endif
    
//    LeaveRegion();
    
    TestsPeriodiques ();
    
    /*** Retour du compte rendu de l'allocation ***/
    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED noyau_enum_retour LibereBloc (void **adresse)
* PARAMETRES:
* RETOUR:
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Fonction qui libere une zone memoire d'un pool IOS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour LibereBloc (void **pp_neutre)
{
    noyau_enum_retour retour;
    
//    EnterRegion();
    
    retour = NOYAU_OK;
    
    /*** Test positionnement pointeur ***/
    if ( *pp_neutre == NULL)
    {
        /*** Appel au fichier trace ***/
#ifdef ERREURS
        RtcFichierDebug(NOY_MEM, &_noyau_.dbg, "Libere() :" FMT_ADR FMT_TACHE,
            *pp_neutre, GetCurrentThreadId());
#endif
        
//        LeaveRegion();
        
        /*** For‡age de la valeur de retour … NOK ***/
        return NOYAU_NOK;
    }

//TIC Removed verification because of VS6.0 runtime bug
//     // teste si le pointeur est valide
//     if (!_CrtIsValidHeapPointer(*pp_neutre))
//     {
//         /*** Appel au fichier trace ***/
// #ifdef ERREURS
//         RtcFichierDebug(NOY_MEM, &_noyau_.dbg, "Libere() : _CrtIsValidHeapPointer " FMT_ADR FMT_TACHE, 
//             *pp_neutre, GetCurrentThreadId());
// #endif
//         
//         return NOYAU_NOK;
//     }
    
    /* libération du buffer */
    free (*pp_neutre);
    
#ifdef TRACES
    RtcFichierTrace(NOY_MEM, &_noyau_.dbg, "Libere() :" FMT_ADR FMT_TACHE, 
        *pp_neutre, GetCurrentThreadId());
#endif
    
    *pp_neutre = NULL;
    
//    LeaveRegion();
    
    TestsPeriodiques ();
    
    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour Alloue (struct_neutre **pp_neutre,
*                                           unsigned short int longueur,
*                                           unsigned short int numero_pool)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - pointeur de pointeur sur une structure de type neutre
*              - taille de la memoire a allouer (en octets)
*              - indice bloc memoire
*
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui alloue de la m‚moire en vue d'une
* communication par boites aux lettes. Elle remplit le
* debut de cette zone memoire avec les champs SegId et tache_id
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour Alloue (struct_neutre **pp_neutre,
                                 noyau_taille_bloc longueur,
                                 noyau_pool_id numero_pool)
{
    return AlloueBloc (pp_neutre, longueur, numero_pool);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour Libere (struct_neutre **pp_neutre)
* PARAMETRES: pointeur de pointeur sur une structure de type neutre
* RETOUR:
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Fonction qui libere une zone memoire (utilisee
*       apres la recuperation d une communication)
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour Libere (struct_neutre **pp_neutre)
{
    return LibereBloc (pp_neutre);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ExitAlloue (struct_neutre **pp_neutre,
*                                  unsigned short int longueur,
*                                  unsigned short int numero_pool)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - pointeur de pointeur sur une structure de type neutre
*              - taille de la memoire a allouer (en octets)
*              - indice bloc memoire
*
*      Retour: - aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui alloue de la m‚moire en vue d'une
* communication par boites aux lettes. Elle remplit le
* debut de cette zone memoire avec les champs SegId et tache_id
* Sur echec, sort de l'application en laissant une trace.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void DEFINE_ExitAlloue (const char *file,
                               int line,
                               struct_neutre **pp_neutre,
                               noyau_taille_bloc longueur,
                               noyau_pool_id numero_pool)
{
    if (Alloue (pp_neutre, longueur, numero_pool) != NOYAU_OK)
        DEFINE_ExitBad (file, line);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ExitLibere (struct_neutre **pp_neutre)
* PARAMETRES: pointeur de pointeur sur une structure de type neutre
* RETOUR:
*      Retour: - aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Fonction qui libere une zone memoire (utilisee
*       apres la recuperation d une communication)
* Sur echec, sort de l'application en laissant une trace.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void DEFINE_ExitLibere (const char *file,
                               int line,
                               struct_neutre **pp_neutre)
{
    if (Libere (pp_neutre) != NOYAU_OK)
        DEFINE_ExitBad (file, line);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_taille_bloc DonneTailleBloc (struct_neutre *p_neutre)
* PARAMETRES: pointeur sur une structure de type neutre
* RETOUR:
*      Retour: - taille du bloc
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: calcul la taille d'un bloc sans préfixe du n° de pool
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_taille_bloc DonneTailleBloc (struct_neutre *p_neutre)
{
	return (noyau_taille_bloc)_msize(p_neutre);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE void TraceBlocs(noyau_pool_id nb_pools)
* PARAMETRES:
* RETOUR:
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Fonction qui trace dans un fichier les etat des blocs et des pools
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void TraceBlocs(void)
{ 
    _CrtMemState state = {0};
    
    // _CrtDumpMemoryLeaks();        
    
    _CrtMemCheckpoint( &state );

    /* inhibe les erreurs */
    DBG_DebutDebugSansErreurs (&_noyau_.dbg);
 
    RtcFichierDebug(NOY_MEM, &_noyau_.dbg, "TraceBlocs() : total max alloue = %ld octets, reste alloue = %ld octets, nb blocks restant = %ld",
        state.lHighWaterCount, state.lSizes[_NORMAL_BLOCK],
        state.lCounts[_NORMAL_BLOCK]);

    DBG_FinDebugSansErreurs (&_noyau_.dbg);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE noyau_enum_retour TesteBlocs(void)
* PARAMETRES:
* RETOUR:
*      Retour: - NOYAU_OK ou NOYAU_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Fonction qui teste si les segid des blocs ont chang‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour TesteBlocs(void)
{
    noyau_enum_retour retour;

    retour = NOYAU_OK;
    
    if (!_CrtDumpMemoryLeaks())
        retour = NOYAU_NOK;

    return retour;
}

PROTECTED noyau_enum_retour InitMem(char *error_file)
{
    _HFILE hFile;

    // The following macros set and clear, respectively, given bits
    // of the C runtime library debug flag, as specified by a bitmask.
    #ifdef   _DEBUG
        #define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
        #define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
    #else
        #define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
        #define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
    #endif
    
    // Set the debug-heap flag 
    
    // Enable debug heap allocations and use of memory block type identifiers, 
    // such as _CLIENT_BLOCK.
    // SET_CRT_DEBUG_FIELD( _CRTDBG_ALLOC_MEM_DF ); 
    // Call _CrtCheckMemory at every allocation and deallocation request.
    //SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_ALWAYS_DF ); 
    // Include _CRT_BLOCK types (Memory used internally by the run-time library) 
    // in leak detection and memory state difference operations.
    //SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_CRT_DF ); 
    // Keep freed memory blocks in the heap’s linked list, assign them the _FREE_BLOCK type, 
    // and fill them with the byte value 0xDD to catch any inadvertent use of freed memory.
//    SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF ); 
    // Perform automatic leak checking at program exit via a call to _CrtDumpMemoryLeaks and 
    // generate an error report if the application failed to free all the memory it allocated.
//    SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF ); 
    
    // Send all reports to a file
    hFile = CreateFile (error_file, 
                GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, 
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, hFile );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, hFile );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, hFile );

    return NOYAU_OK;
}