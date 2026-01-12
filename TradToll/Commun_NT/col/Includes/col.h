/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : COL
 * FILE       : COL.h
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
#ifndef CSR_COL_H
#define CSR_COL_H


#include <public.h>


typedef struct COLLECTION_STRUCT;
typedef struct COLLECTION_STRUCT * HCOLLECTION;

typedef struct COLLECTION_ITEM;
typedef struct COLLECTION_ITEM * HCOLLECTIONITEM;


#define COL_INDEX_BINARY  0
#define COL_INDEX_ZSTRING 1

#define COL_SCAN_BEGIN ((HCOLLECTIONITEM)0)
#define COL_SCAN_END  ((HCOLLECTIONITEM)0xFFFFFFFF)



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
PUBLIC HCOLLECTION WINAPI ColCreate( DWORD dwIndexType, DWORD dwIndexMaxSize, BOOL bCopyItem, BOOL bProtected );



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
PUBLIC void WINAPI ColDestroy( HCOLLECTION hCol );



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
PUBLIC void WINAPI ColLock( HCOLLECTION hCol );


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
PUBLIC void WINAPI ColUnlock( HCOLLECTION hCol );


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
PUBLIC HCOLLECTIONITEM WINAPI ColItemFind( HCOLLECTION hCol, void * pvIndex );


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
PUBLIC HCOLLECTIONITEM WINAPI ColItemAdd( HCOLLECTION hCol, void * pvIndex, void * pvData, DWORD dwDataSize );


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
PUBLIC BOOL WINAPI ColItemScan( HCOLLECTION hCol, HCOLLECTIONITEM * phItem );


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
PUBLIC void WINAPI ColItemRemove( HCOLLECTIONITEM hItem );


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
PUBLIC void WINAPI ColClear( HCOLLECTION hCol );


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
PUBLIC void * WINAPI ColItemData( HCOLLECTIONITEM hItem );


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
PUBLIC DWORD WINAPI ColItemDataSize( HCOLLECTIONITEM hItem );


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
PUBLIC DWORD WINAPI ColCount( HCOLLECTION hCol );


#endif