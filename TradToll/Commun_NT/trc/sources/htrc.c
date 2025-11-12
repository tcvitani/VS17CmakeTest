/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : trc
 * FILE       : htrc.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Handle
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion de "handles" pour la librairie de trace
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

// Pour activation des traces Heap
// #define TRC_DEBUG

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include "trcdbg.h"
#include "htrc.h"


/*--------------- MACROS et TYPES SPECIFIQUES AU MODULE : ---------------*/

// Nombre maxi de handles gérés par le systeme
#define HTRC_MAX_HANDLES 1024

// Struture de gestion d'un handme
typedef struct _HTRC_HANDLE_STRUCT
{
    DWORD dwType;
    HTRC_HANDLE_STATE eState;
    void * pbHandle;
}
HTRC_HANDLE_STRUCT;


/*--------------- VARIABLES GLOBALES SPECIFIQUES AU MODULE : ---------------*/

// Section critique pour l'accès à la table des handles
CRITICAL_SECTION gsCriticalHTable;

// Table des handles
HTRC_HANDLE_STRUCT gpsHandles[ HTRC_MAX_HANDLES ] = { 0 };



/*--------------- CODE : ---------------*/


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void HTRC_Initialise (  )
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialiser la gestion des handles
 * --------------------------------------------------------------------
 */
void HTRC_Initialise()
{
    DWORD dwIdx;
    InitializeCriticalSection( &gsCriticalHTable );

    for ( dwIdx = 0 ; dwIdx < HTRC_MAX_HANDLES ; dwIdx ++ )
    {
        gpsHandles[dwIdx].eState = HTRC_FREE_HANDLE;
        gpsHandles[dwIdx].pbHandle = NULL;
        gpsHandles[dwIdx].dwType = 0;
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : void HTRC_Termine (  )
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Terminer la gestion des handles. Tous les handles
 *             alloués sont automatiquement désalloués.
 * --------------------------------------------------------------------
 */
void HTRC_Termine()
{
    DWORD dwIdx;
    EnterCriticalSection( &gsCriticalHTable );
    for ( dwIdx = 0 ; dwIdx < HTRC_MAX_HANDLES ; dwIdx ++ )
        if ( gpsHandles[dwIdx].eState == HTRC_VALID_HANDLE || gpsHandles[dwIdx].eState == HTRC_INVALID_HANDLE )
            HeapFree( GetProcessHeap(), 0, gpsHandles[dwIdx].pbHandle );
    DeleteCriticalSection( &gsCriticalHTable );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : void * HTRC_Creer_Handle ( DWORD dwSize, DWORD dwType )
 * PARAMETERS: DWORD dwSize : Taille de la structure du handle
 *             DWORD dwType : Identifiant de type de handle
 * RETURN    : un handle ou NULL si erreur (utiliser alors GetLastError())
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Créer un nouveau handle dans l'état HTRC_VALID_HANDLE
 * --------------------------------------------------------------------
 */
void * HTRC_Creer_Handle( DWORD dwSize, DWORD dwType )
{
    DWORD dwErreur = ERROR_NOT_ENOUGH_MEMORY;
    DWORD dwIdx;
    BYTE * pbHandle = NULL;
    
    EnterCriticalSection( &gsCriticalHTable );
    
    // Rechercher un emplacement vide
    for ( dwIdx = 0 ; dwIdx < HTRC_MAX_HANDLES ; dwIdx ++ )
        if ( gpsHandles[dwIdx].eState == HTRC_FREE_HANDLE )
            break;

    // Si emplacement vide trouvé
    if ( dwIdx < HTRC_MAX_HANDLES )
    {
        pbHandle = HeapAlloc( GetProcessHeap(), 0, dwSize + sizeof(DWORD) );
        if ( pbHandle != NULL )
        {
            // Zone allouee avec succes
            gpsHandles[dwIdx].eState = HTRC_VALID_HANDLE;
            gpsHandles[dwIdx].pbHandle = pbHandle;
            gpsHandles[dwIdx].dwType = dwType;
            *((DWORD*)(pbHandle)) = dwIdx;
            dwErreur = NO_ERROR;
        }
    }
    LeaveCriticalSection( &gsCriticalHTable );

    if ( pbHandle != NULL )
        pbHandle = pbHandle + sizeof(DWORD);

    SetLastError( dwErreur );
    return (void *)pbHandle;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Type_Handle( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle dont on souhaite connaitre le type
 * RETURN    : Type du handle. 0xFFFFFFFF si erreur (GetLastError() pour
 *             code complémentaire)
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Connaitre le type d'un handle, tester si un pointeur est
 *             bien un handle.
 * --------------------------------------------------------------------
 */
DWORD HTRC_Type_Handle( void * pvHandle )
{
    BYTE * pbHandle = ((BYTE *)(pvHandle)) - sizeof(DWORD);
    DWORD dwIdx;
    DWORD dwType = 0xFFFFFFFF;
    DWORD dwErreur = ERROR_INVALID_HANDLE;

    EnterCriticalSection( &gsCriticalHTable );

    if ( ! IsBadReadPtr( pbHandle, sizeof(DWORD) ) )
    {
        dwIdx = *((DWORD*)(pbHandle));
        if ( dwIdx < HTRC_MAX_HANDLES )
        {
            if ( gpsHandles[dwIdx].pbHandle == pbHandle && gpsHandles[dwIdx].eState != HTRC_FREE_HANDLE )
            {
                dwType = gpsHandles[dwIdx].dwType;
                dwErreur = NO_ERROR;
            }
        }
    }

    LeaveCriticalSection( &gsCriticalHTable );

    SetLastError( dwErreur );
    return dwType;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : HTRC_HANDLE_STATE HTRC_Etat_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle dont on souhaite connaitre l'état
 * RETURN    : Etat du handle. C'est à dire HTRC_VALID_HANDLE si ce
 *             le handle est alloué et qu'il est marqué comme valide,
 *             HTRC_INVALID_HANDLE si il est alloué, mais marqué comme
 *             non valide, HTRC_NOT_HANDLE si ce n'est pas un handle.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Connaitre l'état d'un handle, tester si un pointeur est
 *             bien un handle.
 * --------------------------------------------------------------------
 */
HTRC_HANDLE_STATE HTRC_Etat_Handle( void * pvHandle )
{
    HTRC_HANDLE_STATE eState = HTRC_NOT_HANDLE;
    BYTE * pbHandle = ((BYTE *)(pvHandle)) - sizeof(DWORD);
    DWORD dwIdx;

    EnterCriticalSection( &gsCriticalHTable );

    if ( ! IsBadReadPtr( pbHandle, sizeof(DWORD) ) )
    {
        dwIdx = *((DWORD*)(pbHandle));
        if ( dwIdx < HTRC_MAX_HANDLES )
        {
            if ( gpsHandles[dwIdx].pbHandle == pbHandle )
                eState = gpsHandles[dwIdx].eState;
        }
    }

    LeaveCriticalSection( &gsCriticalHTable );

    return eState;
}

    


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Invalider_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle à marquer comme non valide
 * RETURN    : Code d'erreur win32, NO_ERROR si marquage ok.
 *             Si le handle est déja marqué invalide, on obtient
 *             une erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Marque un handle comme non valide (il reste alloué et
 *             les données qu'il contient ne sont pas altérées).
 * --------------------------------------------------------------------
 */
DWORD HTRC_Invalider_Handle( void * pvHandle )
{
    BYTE * pbHandle = ((BYTE *)(pvHandle)) - sizeof(DWORD);
    DWORD dwIdx;
    DWORD dwErreur = ERROR_INVALID_HANDLE;

    EnterCriticalSection( &gsCriticalHTable );

    if ( ! IsBadReadPtr( pbHandle, sizeof(DWORD) ) )
    {
        dwIdx = *((DWORD*)(pbHandle));
        if ( dwIdx < HTRC_MAX_HANDLES )
        {
            if ( gpsHandles[dwIdx].pbHandle == pbHandle )
                if ( gpsHandles[dwIdx].eState = HTRC_VALID_HANDLE )
                {
                    gpsHandles[dwIdx].eState = HTRC_INVALID_HANDLE;
                    dwErreur = NO_ERROR;
                }
                else
                    dwErreur = ERROR_INVALID_HANDLE_STATE;
        }
    }
    LeaveCriticalSection( &gsCriticalHTable );
    
    SetLastError( dwErreur );
    return dwErreur;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Valider_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle à marquer comme valide
 * RETURN    : Code d'erreur win32, NO_ERROR si marquage ok.
 *             Si le handle est déja marqué valide, on obtient
 *             une erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Marque un handle comme valide (il reste alloué et
 *             les données qu'il contient ne sont pas altérées).
 * --------------------------------------------------------------------
 */
DWORD HTRC_Valider_Handle( void * pvHandle )
{
    BYTE * pbHandle = ((BYTE *)(pvHandle)) - sizeof(DWORD);
    DWORD dwIdx;
    DWORD dwErreur = ERROR_INVALID_HANDLE;

    EnterCriticalSection( &gsCriticalHTable );

    if ( ! IsBadReadPtr( pbHandle, sizeof(DWORD) ) )
    {
        dwIdx = *((DWORD*)(pbHandle));
        if ( dwIdx < HTRC_MAX_HANDLES )
        {
            if ( gpsHandles[dwIdx].pbHandle == pbHandle )
                if ( gpsHandles[dwIdx].eState = HTRC_INVALID_HANDLE )
                {
                    gpsHandles[dwIdx].eState = HTRC_VALID_HANDLE;
                    dwErreur = NO_ERROR;
                }
                else
                    dwErreur = ERROR_INVALID_HANDLE_STATE;
        }
    }

    LeaveCriticalSection( &gsCriticalHTable );

    SetLastError( dwErreur );
    return dwErreur;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD HTRC_Detruire_Handle ( void * pvHandle )
 * PARAMETERS: void * pvHandle : handle a detruire
 * RETURN    : Un code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Supprime un handle, c'est à dire le désalloue, qu'il
 *             soit marqué non valide ou valide
 * --------------------------------------------------------------------
 */
DWORD HTRC_Detruire_Handle( void * pvHandle )
{
    BYTE * pbHandle = ((BYTE *)(pvHandle)) - sizeof(DWORD);
    DWORD dwIdx;
    DWORD dwErreur = ERROR_INVALID_HANDLE;

    EnterCriticalSection( &gsCriticalHTable );

    if ( ! IsBadReadPtr( pbHandle, sizeof(DWORD) ) )
    {
        dwIdx = *((DWORD*)(pbHandle));
        if ( dwIdx < HTRC_MAX_HANDLES )
        {
            if ( gpsHandles[dwIdx].pbHandle == pbHandle )
                if ( gpsHandles[dwIdx].eState == HTRC_VALID_HANDLE || gpsHandles[dwIdx].eState == HTRC_INVALID_HANDLE )
                {
                    HeapFree( GetProcessHeap(), 0, pbHandle );
                    gpsHandles[dwIdx].eState = HTRC_FREE_HANDLE;
                    gpsHandles[dwIdx].pbHandle = NULL;
                    dwErreur = NO_ERROR;
                }
                else
                    dwErreur = ERROR_INVALID_HANDLE_STATE;
        }
                                
    }

    LeaveCriticalSection( &gsCriticalHTable );

    SetLastError( dwErreur );

    return dwErreur;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : void * HTRC_Trouver_Handle ( HTRC_HANDLE_STATE eState, DWORD dwType )
 * PARAMETERS: HTRC_HANDLE_STATE eState : Etat du handle a trouver
 *             DWORD dwType             : Type du handle a trouver
 * RETURN    : Handle trouvé (NULL si non trouvé)
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recherche le premier handle dans un état donné d'un type donné
 * --------------------------------------------------------------------
 */
void * HTRC_Trouver_Handle( HTRC_HANDLE_STATE eState, DWORD dwType )
{
    BYTE * pbHandle = NULL;
    void * pvHandle = NULL;
    DWORD dwIdx;

    EnterCriticalSection( &gsCriticalHTable );

    for ( dwIdx = 0 ; dwIdx < HTRC_MAX_HANDLES ; dwIdx ++ )
        if ( gpsHandles[dwIdx].eState == eState && gpsHandles[dwIdx].dwType == dwType )
        {
            pbHandle = gpsHandles[dwIdx].pbHandle;
            pvHandle = (void *)( pbHandle + sizeof(DWORD) );
            break;
        }

    LeaveCriticalSection( &gsCriticalHTable );

    SetLastError( NO_ERROR );

    return pvHandle;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL HTRC_Enum_Handle ( DWORD dwType, HTRC_ENUM_PROC * pfEnum, void * pvContext )
 * PARAMETERS: DWORD dwType             : Type des handles a trouver
 *             HTRC_ENUM_PROC * pfEnum  : Callback fonction pour l'énumération
 *             void * pvContext         : Pour passer des paramètres à la callback fonction
 * RETURN    : TRUE si complet, FALSE si arrété par la callback procedure
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Enumére les handles par callback procedure
 * --------------------------------------------------------------------
 */
BOOL HTRC_Enum_Handle ( DWORD dwType, HTRC_ENUM_PROC * pfEnum, void * pvContext )
{
    BYTE * pbHandle = NULL;
    void * pvHandle = NULL;
    DWORD dwIdx;
    BOOL bRet = TRUE;

    EnterCriticalSection( &gsCriticalHTable );

    for ( dwIdx = 0 ; dwIdx < HTRC_MAX_HANDLES ; dwIdx ++ )
        if ( gpsHandles[dwIdx].eState != HTRC_FREE_HANDLE &&
             gpsHandles[dwIdx].eState != HTRC_NOT_HANDLE &&
             gpsHandles[dwIdx].dwType == dwType )
        {
            pbHandle = gpsHandles[dwIdx].pbHandle;
            pvHandle = (void *)( pbHandle + sizeof(DWORD) );
            if ( ! (*pfEnum)( pvHandle, pvContext) )
            {
                bRet = FALSE;
                break;
            }
        }

    LeaveCriticalSection( &gsCriticalHTable );

    SetLastError( NO_ERROR );

    return bRet;
}
