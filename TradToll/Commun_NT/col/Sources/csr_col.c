/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : COL
 * FILE       : COL.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Collection
 * --------------------------------------------------------------------
 * SUMMARY    : Implémentation de l'objet collection
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#include <windows.h>

#include <col.h>

#include <memclass.h>


typedef struct COLLECTION_ITEM
{
    struct COLLECTION_STRUCT * psCol;
    struct COLLECTION_ITEM * psPrev;
    struct COLLECTION_ITEM * psNext;
    void * pvData;
    DWORD dwDataSize;
    BYTE tbIndex[1];
}
COLLECTION_ITEM;

typedef struct COLLECTION_STRUCT
{
    DWORD dwIndexType;
    DWORD dwIndexMaxSize;
    DWORD dwItemStructSize;
    BOOL bCopyItem;
    BOOL bProtected;
    CRITICAL_SECTION sCritical;
    COLLECTION_ITEM * psFirst;
}
COLLECTION_STRUCT;



/*
 * --------------------------------------------------------------------
 * SYNTAX    : HCOLLECTION WINAPI ColCreate( DWORD dwIndexType, DWORD dwIndexMaxSize, BOOL bCopyItem, BOOL bProtected );
 * PARAMETERS: dwIndexType : Type d'indexation. Les valeurs possibles sont COL_INDEX_BINARY
 *                           et COL_INDEX_ZSTRING.
 *             dwIndexMaxSize : Taille maximum en octets de la valeur d'index.
 *             bCopyItem : Si TRUE, chaque fois qu'un élément est rajouté à la collection,
 *                         la totalité des données de cet élément est placé dans la collection.
 *                         Si FALSE, seul un pointeur sur cet élément est placé dans la collection.
 *             bProtected : Si TRUE, la collection utilise des accès protégé. Si FALSE,
 *                          la collection n'est pas protégée, les accès concurents ne sont
 *                          pas possibles.
 * RETURN    : Un handle de collection ou NULL en cas d'erreur (pas de mémoire).
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Création d'un nouvel objet collection
 * --------------------------------------------------------------------
 */
PUBLIC HCOLLECTION WINAPI ColCreate( DWORD dwIndexType, DWORD dwIndexMaxSize, BOOL bCopyItem, BOOL bProtected )
{
    HCOLLECTION hCol = NULL;
    DWORD dwErr = NO_ERROR;

    if ( ( dwIndexMaxSize > 0 ) &&
         ( ( dwIndexType == COL_INDEX_BINARY ) ||
           ( dwIndexType == COL_INDEX_ZSTRING )
       ) )
    {
        hCol = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( COLLECTION_STRUCT ) );
        if ( hCol != NULL )
        {
            hCol->dwIndexType = dwIndexType;
            hCol->dwIndexMaxSize = dwIndexMaxSize;
            hCol->bCopyItem = bCopyItem;
            hCol->dwItemStructSize = sizeof( COLLECTION_ITEM ) + dwIndexMaxSize;
            if ( bProtected ) InitializeCriticalSection( &hCol->sCritical );
        }
    }

    return hCol;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI ColLock( HCOLLECTION hCol )
 * PARAMETERS: hCol  : Handle de collection obtenu avec ColCreate().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Poser un vérou sur la collection. Si un vérou déjà présent, 
 *             cette fonction attend qu'il soit libéré. Dans une même tache,
 *             plusieurs vérous sur la même collection peuvent être imbriqués.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI ColLock( HCOLLECTION hCol )
{
    if ( hCol->bProtected )
        EnterCriticalSection( &hCol->sCritical );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI ColUnlock( HCOLLECTION hCol )
 * PARAMETERS: hCol  : Handle de collection obtenu avec ColCreate().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Terminer un vérou sur la collection. 
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI ColUnlock( HCOLLECTION hCol )
{
    if ( hCol->bProtected )
        LeaveCriticalSection( &hCol->sCritical );
}

            

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC HCOLLECTIONITEM WINAPI ColItemFind( HCOLLECTION hCol, void * pvIndex )
 * PARAMETERS: hCol  : Handle de collection obtenu avec ColCreate().
 *             pvIndex : Pointeur sur la valeur d'index à rechercher
 * RETURN    : NULL si l'élément n'a pas été trouvé. Un handle d'élément sinon.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Rechercher un élement dans une collection d'après sa valeur d'index
 * --------------------------------------------------------------------
 */
PUBLIC HCOLLECTIONITEM WINAPI ColItemFind( HCOLLECTION hCol, void * pvIndex )
{
    COLLECTION_ITEM * psScan = hCol->psFirst;

    ColLock( hCol );

    while ( psScan != NULL )
    {
        if ( hCol->dwIndexType == COL_INDEX_BINARY )
        {
            if ( memcmp( psScan->tbIndex, pvIndex, hCol->dwIndexMaxSize ) == 0 )
                break;
        }
        else /* hCol->dwIndexType == COL_INDEX_ZSTRING */
        {
            if ( strncmp( psScan->tbIndex, pvIndex, hCol->dwIndexMaxSize ) == 0 )
                break;
        }
        psScan = psScan->psNext;
    }
    
    ColUnlock( hCol );

    return psScan;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : HCOLLECTIONITEM WINAPI ColItemAdd( HCOLLECTION hCol, void * pvIndex, void * pvData, DWORD dwDataSize )
 * PARAMETERS: hCol  : Handle de collection obtenu avec ColCreate()
 *             pvIndex : Pointeur sur la valeur d'index de l'élément à ajouter
 *             pvData : Pointeur sur l'élément à ajouter
 *             dwDataSize : Taille de l'élément à ajouter
 * RETURN    : NULL si l'élément n'a pas pas pu être ajouté (pas assez de mémoire, ou
 *             valeur d'index déjà utilisée). Un handle d'élément sinon.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajouter un élément dans une collection
 * --------------------------------------------------------------------
 */
PUBLIC HCOLLECTIONITEM WINAPI ColItemAdd( HCOLLECTION hCol, void * pvIndex, void * pvData, DWORD dwDataSize )
{
    void * pvBuffer = NULL;
    COLLECTION_ITEM * psItem = NULL;

    ColLock( hCol );
    
    if ( ColItemFind( hCol, pvIndex ) == NULL )
    {
        if ( hCol->bCopyItem )
            pvBuffer = HeapAlloc( GetProcessHeap(), 0, dwDataSize );
        else
            pvBuffer = pvData;
        psItem = HeapAlloc( GetProcessHeap(), 0, hCol->dwItemStructSize );
        if ( ( psItem != NULL ) && ( pvBuffer != NULL ) )
        {
            psItem->psCol = hCol;
            psItem->psPrev = NULL;
            psItem->psNext = hCol->psFirst;
            if ( hCol->psFirst != NULL ) hCol->psFirst->psPrev = psItem;
            hCol->psFirst = psItem;
            psItem->pvData = pvBuffer;
            psItem->dwDataSize = dwDataSize;
            if ( hCol->bCopyItem ) memcpy( pvBuffer, pvData, dwDataSize );
            if ( hCol->dwIndexType == COL_INDEX_BINARY )
                memcpy( psItem->tbIndex, pvIndex, hCol->dwIndexMaxSize );
            else /* hCol->dwIndexType == COL_INDEX_ZSTRING */
                strncpy( psItem->tbIndex, pvIndex, hCol->dwIndexMaxSize );
        }
        else
        {
            if ( hCol->bCopyItem && ( pvBuffer != NULL ) ) HeapFree( GetProcessHeap(), 0, pvBuffer );
            if ( psItem != NULL ) HeapFree( GetProcessHeap(), 0, psItem );
            pvBuffer = NULL;
            psItem = NULL;
        }
    }
    
    ColUnlock( hCol );

    return psItem;
}
            
    
/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL WINAPI ColItemScan( HCOLLECTION hCol, HCOLLECTIONITEM * phItem )
 * PARAMETERS: hCol  : Handle de collection obtenu avec ColCreate().
 *             phItem : En entrée, handle du dernier élément scanné. En sortie, handle
 *                      de l'élément suivant. En début de scan, phItem pointe sur la valeur
 *                      COL_SCAN_BEGIN. Lorsque le dernier élément a été scanné, la valeur
 *                      retournée par phItem est COL_SCAN_END.
 * RETURN    : TRUE si élément trouvé. FALSE fin atteinte
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Scanner une collection élément par élément.
 * --------------------------------------------------------------------
 */
PUBLIC BOOL WINAPI ColItemScan( HCOLLECTION hCol, HCOLLECTIONITEM * phItem )
{
    BOOL bContinue;
    COLLECTION_ITEM * psItem = *phItem;

    if ( psItem == COL_SCAN_BEGIN )
    {
        bContinue = ( hCol->psFirst != NULL );
        *phItem = ( bContinue ? hCol->psFirst : COL_SCAN_END );
    }
    else if ( psItem != COL_SCAN_END )
    {
        bContinue = ( psItem->psNext != NULL );
        *phItem = ( bContinue ? psItem->psNext : COL_SCAN_END );
    }
    else
        bContinue = FALSE;

    return bContinue;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI ColItemRemove( HCOLLECTIONITEM hItem )
 * PARAMETERS: hItem : handle de l'élément à supprimer.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Suprime un élément de la collection à laquelle il appartient.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI ColItemRemove( HCOLLECTIONITEM hItem )
{
    HCOLLECTION hCol = hItem->psCol;

    ColLock( hCol );

    if ( hItem->psPrev != NULL ) hItem->psPrev->psNext = hItem->psNext;
    if ( hItem->psNext != NULL ) hItem->psNext->psPrev = hItem->psPrev;
    if ( hCol->psFirst == hItem )
        hCol->psFirst = hItem->psNext;
    if ( hCol->bCopyItem )
        HeapFree( GetProcessHeap(), 0, hItem->pvData );
    HeapFree( GetProcessHeap(), 0, hItem );

    ColUnlock( hCol );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void WINAPI ColClear( HCOLLECTION hCol );
 * PARAMETERS: hCol : Handle de la collection à vider.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Vide le contenu d'une collection.
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI ColClear( HCOLLECTION hCol )
{
    HCOLLECTIONITEM hItem;
    HCOLLECTIONITEM hItemNext;

    ColLock( hCol );

    hItem = hCol->psFirst;
    while ( hItem != NULL )
    {
        hItemNext = hItem->psNext;
        if ( hItem->psPrev != NULL ) hItem->psPrev->psNext = hItem->psNext;
        if ( hItem->psNext != NULL ) hItem->psNext->psPrev = hItem->psPrev;
        if ( hCol->psFirst == hItem )
            hCol->psFirst = hItem->psNext;
        if ( hCol->bCopyItem )
            HeapFree( GetProcessHeap(), 0, hItem->pvData );
        HeapFree( GetProcessHeap(), 0, hItem );
        hItem = hItemNext;
    }
    hCol->psFirst = NULL;

    ColUnlock( hCol );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void * WINAPI ColItemData( HCOLLECTIONITEM hItem )
 * PARAMETERS: hItem : handle de l'élément à intérroger.
 * RETURN    : Un pointeur sur les données de l'élément. Si la collection est en mode
 *             copie (bCopyItem = TRUE à la création), le pointeur obtenue pointe sur
 *             la copie, sinon, il pointe sur les données originales.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère un pointeur sur les données d'un élément
 * --------------------------------------------------------------------
 */
PUBLIC void * WINAPI ColItemData( HCOLLECTIONITEM hItem )
{
    return hItem->pvData;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD WINAPI ColItemDataSize( HCOLLECTIONITEM hItem )
 * PARAMETERS: hItem : handle de l'élément à intérroger.
 * RETURN    : Taille des données de l'élément.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère la taille des données d'un élément.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI ColItemDataSize( HCOLLECTIONITEM hItem )
{
    return hItem->dwDataSize;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI ColDestroy( HCOLLECTION hCol )
 * PARAMETERS: hCol : Handle de la collection à détruire.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Destruction d'une collection
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI ColDestroy( HCOLLECTION hCol )
{
    ColLock( hCol );
    ColClear( hCol );
    if ( hCol->bProtected ) DeleteCriticalSection( &hCol->sCritical );
    HeapFree( GetProcessHeap(), 0 , hCol );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : DWORD WINAPI ColCount( HCOLLECTION hCol )
 * PARAMETERS: hCol : Handle de la collection à interroger
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : Nombre d'éléments
 * --------------------------------------------------------------------
 * ROLE      : Déterminer le nombre d'éléments dans une collection.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI ColCount( HCOLLECTION hCol )
{
    DWORD dwCount = 0;
    HCOLLECTIONITEM hItem = COL_SCAN_BEGIN;

    ColLock( hCol );

    while ( ColItemScan( hCol, &hItem ) )
        dwCount ++;

    ColUnlock( hCol );

    return dwCount;
}