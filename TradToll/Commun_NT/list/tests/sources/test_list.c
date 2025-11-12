/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Liste générique
* FICHIER: Liste.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "..\..\includes\csr_list.h"

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/

#define MAX_ITEM 30

BOOL WINAPI Cherche(UINT *Item1, UINT *Item2)
{
    return *Item1 == *Item2;
}


list_enum_compare WINAPI Croissant(UINT *Item1, UINT *Item2)
{
    if (*Item1 < *Item2)
        return LIST_IS_LOWER;

    if (*Item1 > *Item2)
        return LIST_IS_GREATER;

    return LIST_IS_EQUAL;
}

list_enum_compare WINAPI Decroissant(UINT *Item1, UINT *Item2)
{
    if (*Item1 > *Item2)
        return LIST_IS_LOWER;

    if (*Item1 < *Item2)
        return LIST_IS_GREATER;

    return LIST_IS_EQUAL;
}

int main(void)
{
    UINT *Item[MAX_ITEM+1];
    UINT i;
    UINT *pItem;
    HLIST hList;
    list_struct_find find;

    srand( (unsigned)time( NULL ) );
    
    // allocation de la liste
    if ((hList = List_New()) == NULL)
        return -1;

    // allocation des items
    for (i=0 ; i<=MAX_ITEM; i++)
    {
        Item[i] = List_ItemNew(hList, sizeof(UINT));
        *Item[i] = rand() % MAX_ITEM;
    }

    // insertion en tete
    for (i=0 ; i<MAX_ITEM; i++)
        List_AddHead (hList, Item[i]);

    // extraction en queue
    while ((i = List_GetCount(hList)) > 0)
        Item[i-1] = List_RemoveTail(hList);

    // insertion en queue
    for (i=0 ; i<MAX_ITEM; i++)
        List_AddTail (hList, Item[i]);

    // Recherche dans la liste
    find.hList = hList;
    find.ItemToFind = Item[rand()%MAX_ITEM];
    find.Equal = Cherche;

    printf ("Recherche dans la liste :\n");
    pItem = List_FindHeadFirst(&find);
    while (pItem != NULL)
    {
        printf(" -> %lu at %lu\n", *(UINT *)find.LastItemFound, find.LastFoundCount);
        pItem = List_FindNext(&find);
    }

    // extraction en tete
    for (i=0 ; i<MAX_ITEM; i++)
        Item[i] = List_RemoveHead(hList);

    // insertion dans l'ordre
    for (i=0 ; i<MAX_ITEM; i++)
        List_InsertInOrder (hList, Item[i], Croissant, TRUE);

    // parcours de la liste
    printf ("Parcours de la liste ordonnée (%lu) :\n", List_GetCount(hList));
    pItem = List_GetHead(hList);
    while (pItem != NULL)
    {
        printf(" -> %lu", *pItem);
        pItem = List_GetNext(hList, pItem);
    }

    // Libération de la liste non vide
    List_DeleteAll(&hList, NULL, NULL);

	getch();
    return 0;
}
