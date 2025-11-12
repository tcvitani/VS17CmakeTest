/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_RGN.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
#define FMT_TACHE " { tache %lu }"
#define FMT_TACHE_IMB " { tache %lu / imbrications %d }"

/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/


PROTECTED noyau_enum_retour InitRegion (void)
{
    // crée un mutex partagé entre les instances de la DLL pour simuler les régions
    // ATTENTION : pas de blocage de scheduling en région entre les zones en région
    // et les zones hors région contrairement à RTC !
    _noyau_.hRegion = CreateMutex (NULL, FALSE, "CSR_REGION_PAR_MUTEX");
    if (_noyau_.hRegion == NULL)
        return NOYAU_NOK;

    // création d'un mutex de protection des données globales propres au noyau
    _noyau_.hRegionNoyau = CreateMutex (NULL, FALSE, "CSR_REGION_NOYAU_PAR_MUTEX");
    if (_noyau_.hRegionNoyau == NULL)
        return NOYAU_NOK;
    
    // ajout d'une région locale a un process par section critique
    InitializeCriticalSection (&_noyau_.hLocalRegion);

    // protection des chronos
    InitializeCriticalSection (&_noyau_.csChrono);

    // protection des traces
    InitializeCriticalSection (&_noyau_.csDbg);

    return NOYAU_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_retour DebutRegion (void);
* PARAMETRES: aucun
* RETOUR:
*             - NOYAU_OK
*             - NOYAU_REGION_MAX : Nbre de EnterRegion > 32767
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - Entree dans une region critique (pas d'interruption possible
*         par d'autres taches
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour DebutRegion (void)
{
    /*** Entree dans une region critique ***/
    
    // pas de test car TIME_OUT infini et WAIT_ABANDONNED est OK
    WaitForSingleObject (_noyau_.hRegion, INFINITE);
    
    if (_NOYAU_.imbrication_region == 0)
    {
        _NOYAU_.thread_en_region = GetCurrentThreadId();

#ifdef TRACES
        RtcFichierTrace(NOY_RGN, &_noyau_.dbg, "DebutRegion() :" FMT_TACHE, GetCurrentThreadId());
#endif
    }
    _NOYAU_.imbrication_region ++;
    
    TestsPeriodiques ();
    
    /*** Valeur de retour ***/
    return NOYAU_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_retour FinRegion (void);
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - Sortie d'une region critique
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour FinRegion (void)
{
    _NOYAU_.imbrication_region --;
    
    if (_NOYAU_.imbrication_region < 0)
    {
#ifdef ERREURS
        RtcFichierDebug(NOY_RGN, &_noyau_.dbg, "FinRegion() : deja hors region");
#endif
    }
    else
        if (_NOYAU_.imbrication_region == 0)
        {
            _NOYAU_.thread_en_region = 0;

#ifdef TRACES
            RtcFichierTrace(NOY_RGN, &_noyau_.dbg, "FinRegion()");
#endif
        }
        
    /*** Quitter une region critique ***/
    if (!ReleaseMutex (_noyau_.hRegion))
    {
#ifdef ERREURS
        RtcFichierDebug(NOY_RGN, &_noyau_.dbg, "FinRegion() :" FMT_TACHE_IMB FMT_STATUS, 
            GetCurrentThreadId(), _NOYAU_.imbrication_region, GetLastError());
#endif
        return NOYAU_NOK;
    }
    
    TestsPeriodiques ();
    
    /*** Valeur de retour ***/
    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_booleen EstEnRegion (void)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - renvoie NOYAU_VRAI dans une region critique, NOYAU_FAUX sinon
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_booleen EstEnRegion (void)
{
    
    TestsPeriodiques ();
    
    if (_NOYAU_.imbrication_region > 0)
        return NOYAU_VRAI;
    
    return NOYAU_FAUX;
}

PROTECTED void TraceRegions (short int imbrications)
{
    /* inhibe les erreurs */
    DBG_DebutDebugSansErreurs (&_noyau_.dbg);
    
    RtcFichierDebug(NOY_RGN, &_noyau_.dbg, "TraceRegions() : imbrications = %02d", imbrications);
    
    DBG_FinDebugSansErreurs (&_noyau_.dbg);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_retour _DebutRegion (void);
* PARAMETRES: aucun
* RETOUR:
*             - NOYAU_OK
*             - NOYAU_REGION_MAX : Nbre de EnterRegion > 32767
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - Entree dans une region critique (pas d'interruption possible
*         par d'autres taches
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour DebutRegionLocale (void)
{
    /*** Entree dans une region critique ***/
    
    // pas de test car TIME_OUT infini et WAIT_ABANDONNED est OK
    EnterCriticalSection (&_noyau_.hLocalRegion);
    
    if (_noyau_.imbrication_region_locale == 0)
    {
        _noyau_.thread_en_region_locale = GetCurrentThreadId();

#ifdef TRACES
        RtcFichierTrace(NOY_RGN, &_noyau_.dbg, "DebutRegionLocale() :" FMT_TACHE, GetCurrentThreadId());
#endif
    }
    _noyau_.imbrication_region_locale ++;
    
    TestsPeriodiques ();
    
    /*** Valeur de retour ***/
    return NOYAU_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_retour _FinRegion (void);
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - Sortie d'une region critique
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour FinRegionLocale (void)
{
    _noyau_.imbrication_region_locale --;
    
    if (_noyau_.imbrication_region_locale < 0)
    {
#ifdef ERREURS
        RtcFichierDebug(NOY_RGN, &_noyau_.dbg, "FinRegionLocale() : deja hors region");
#endif
    }
    else
        if (_noyau_.imbrication_region_locale == 0)
        {
            _noyau_.thread_en_region_locale = 0;

#ifdef TRACES
            RtcFichierTrace(NOY_RGN, &_noyau_.dbg, "FinRegionLocale()");
#endif
        }
        
        /*** Quitter une region critique ***/
        LeaveCriticalSection (&_noyau_.hLocalRegion);
        
        TestsPeriodiques ();
        
        /*** Valeur de retour ***/
        return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC noyau_enum_booleen _EstEnRegion (void)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: - renvoie NOYAU_VRAI dans une region critique, NOYAU_FAUX sinon
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_booleen EstEnRegionLocale (void)
{
    
    TestsPeriodiques ();
    
    if (_noyau_.imbrication_region_locale > 0)
        return NOYAU_VRAI;
    
    return NOYAU_FAUX;
}

PROTECTED void TraceRegionsLocales (short int imbrications)
{
    /* inhibe les erreurs */
    DBG_DebutDebugSansErreurs (&_noyau_.dbg);
    
    RtcFichierDebug(NOY_RGN, &_noyau_.dbg, "TraceRegionsLocale() : imbrications = %02d", imbrications);
    
    DBG_FinDebugSansErreurs (&_noyau_.dbg);
}

