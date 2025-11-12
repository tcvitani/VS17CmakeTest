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

#include <csr_list.h>
 
/*--------------- DEFINES: ---------------*/

#define ITEM_GET_HANDLE(i)  ((struct_item *) (((BYTE *) i) - sizeof(struct_item)))
#define ITEM_GET_DATA(i)    ((struct_item *) (((BYTE *) i) + sizeof(struct_item)))

#define ITEM_IS_LINKED(i)   (i != NULL && ITEM_GET_HANDLE(i)->next != NULL && ITEM_GET_HANDLE(i)->prev != NULL)
#define ITEM_IS_UNLINKED(i) (i != NULL && ITEM_GET_HANDLE(i)->next == NULL && ITEM_GET_HANDLE(i)->prev == NULL)

/*--------------- TYPEDEFS: ---------------*/

typedef struct Item 
{
    struct Item *next;
    struct Item *prev;
}
struct_item;

typedef struct List
{
    struct_item Head;
    DWORD dwCount;
    CRITICAL_SECTION cs;
}
struct_list;

/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/

// CONSTRUCTEURS / DESTRUCTEURS

HLIST WINAPI List_New (VOID)
{
    struct_list *pList;

    // allocation de la structure du handle
    if ((pList = malloc (sizeof(struct_list))) == NULL)
        return NULL;

    // tete de liste pointe sur elle-meme,
    // la queue pointe sur la tete,
    // (mais non, ce n'est pas sale, pense aux fleurs...)
    pList->Head.next = &pList->Head;
    pList->Head.prev = &pList->Head;
    pList->dwCount = 0;

    InitializeCriticalSection (&pList->cs);

    return pList;
}

BOOL WINAPI List_Delete (HLIST *phList)
{
    struct_list *pList = *phList;
    
    // si NULL, erreur
    if (phList == NULL || *phList == NULL)
        return FALSE;

    // si la liste est non vide, on sort
    if (List_IsEmpty(*phList) == FALSE)
        return FALSE;

    // on efface le handle
    *phList = NULL;    
    
    DeleteCriticalSection(&pList->cs);
    
    // libere la liste
    free (pList);

    return TRUE;
}

BOOL WINAPI List_DeleteAll (HLIST *phList, list_func_action fAction, PVOID Param)
{
    struct_list *pList = *phList;
    struct_item *pItem;
    
    // si NULL, erreur
    if (phList == NULL || *phList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    // Pour chaque item...
	while ((pItem = List_RemoveHead(*phList)) != NULL)
    {
        // une dernière volonté avant l'exécution ?
        if (fAction != NULL)
        {
            // une cigarette.
            if (fAction(pItem, Param) == FALSE)
            {
                LeaveCriticalSection(&pList->cs);
                return FALSE;
            }
        }

        // feu !
		List_ItemDelete (*phList, &pItem);
    }

    // on efface le handle
    *phList = NULL;    
    
    DeleteCriticalSection(&pList->cs);
    
    // libere la liste
    free (pList);

    return TRUE;
}

PVOID WINAPI List_ItemNew (HLIST hList, DWORD SizeOfItem)
{
    struct_item *pItem;
    struct_list *pList = hList;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);
    
    // ajouter la taille de l'entete d'item
    if ((pItem = malloc (sizeof(struct_item) + SizeOfItem)) == NULL)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    memset (pItem, 0, sizeof(struct_item) + SizeOfItem);

    LeaveCriticalSection(&pList->cs);

    return ITEM_GET_DATA(pItem);
}

BOOL WINAPI List_ItemDelete (HLIST hList, PVOID *Item)
{
    struct_list *pList = hList;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    // si NULL, erreur
    if (Item == NULL || *Item == NULL)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    // si l'item est lié à une liste, ne pas le libérer
    if (ITEM_IS_LINKED(*Item) == TRUE)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    // prendre en compte l'adresse de debut réel du bloc
    free (ITEM_GET_HANDLE(*Item));

    // supprime le handle
    *Item = NULL;

    LeaveCriticalSection(&pList->cs);

    return TRUE;
}

// INSERTIONS / EXTRACTIONS

BOOL WINAPI List_InsertAfter (HLIST hList, PVOID RefItem, PVOID NewItem)
{
    struct_list *pList = hList;
    struct_item *pRefItem;
    struct_item *pItem;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    // teste si l'item n'est pas deja lié
    if (ITEM_IS_UNLINKED(NewItem) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    // si la référence est NULL, utiliser la tete
    if (RefItem == NULL)
        pRefItem = &pList->Head;
    else
    {
        if (ITEM_IS_LINKED(RefItem) == FALSE)
        {
            LeaveCriticalSection(&pList->cs);
            return FALSE;
        }

        pRefItem = ITEM_GET_HANDLE(RefItem);
    }

    pItem = ITEM_GET_HANDLE(NewItem);

    // préparation de l'item
    pItem->next = pRefItem->next;
    pItem->prev = pRefItem;
    
    // insertion de l'item
    pRefItem->next->prev  = pItem;
    pRefItem->next  = pItem;

    pList->dwCount ++;

    LeaveCriticalSection(&pList->cs);
    
    return TRUE;
}
    
BOOL WINAPI List_InsertBefore (HLIST hList, PVOID RefItem, PVOID NewItem)
{
    struct_list *pList = hList;
    struct_item *pRefItem;
    struct_item *pItem;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    // teste si l'item n'est pas deja lié
    if (ITEM_IS_UNLINKED(NewItem) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    // si la référence est NULL, utiliser la tete
    if (RefItem == NULL)
        pRefItem = &pList->Head;
    else
    {
        if (ITEM_IS_LINKED(RefItem) == FALSE)
        {
            LeaveCriticalSection(&pList->cs);
            return FALSE;
        }

        pRefItem = ITEM_GET_HANDLE(RefItem);
    }

    pItem = ITEM_GET_HANDLE(NewItem);
    
    // préparation de l'item
    pItem->next = pRefItem;
    pItem->prev = pRefItem->prev;
    
    // insertion de l'item
    pRefItem->prev->next  = pItem;
    pRefItem->prev  = pItem;
        
    pList->dwCount ++;

    LeaveCriticalSection(&pList->cs);
    
    return TRUE;
}

BOOL WINAPI List_MergeAfter (HLIST hList, PVOID RefItem, HLIST hListToMerge)
{
    struct_list *pList = hList;
    struct_list *pListToMerge = hListToMerge;
    struct_item *pRefItem;
    struct_item *pHead;

    if (hList == NULL || hListToMerge == NULL)
        return FALSE;

    // teste si la liste est vide, rien a faire !
    if (List_IsEmpty(hListToMerge) == TRUE)
        return TRUE;

    EnterCriticalSection(&pList->cs);
    EnterCriticalSection(&pListToMerge->cs);

    // si la référence est NULL, utiliser la tete
    if (RefItem == NULL)
        pRefItem = &pList->Head;
    else
    {
        if (ITEM_IS_LINKED(RefItem) == FALSE)
        {
            LeaveCriticalSection(&pListToMerge->cs);
            LeaveCriticalSection(&pList->cs);
            return FALSE;
        }

        pRefItem = ITEM_GET_HANDLE(RefItem);
    }

    pHead = &pListToMerge->Head;

    // préparation des items de debut et fin de liste
    pHead->prev->next = pRefItem->next;
    pHead->next->prev = pRefItem;
    
    // insertion du premier et du dernier item 
    pRefItem->next->prev  = pHead->prev;
    pRefItem->next  = pHead->next;

    pList->dwCount += pListToMerge->dwCount;

    // Vidage de la liste source
    pListToMerge->Head.next = &pListToMerge->Head;
    pListToMerge->Head.prev = &pListToMerge->Head;
    pListToMerge->dwCount = 0;
    
    LeaveCriticalSection(&pListToMerge->cs);
    LeaveCriticalSection(&pList->cs);
    
    return TRUE;
}

BOOL WINAPI List_MergeBefore (HLIST hList, PVOID RefItem, HLIST hListToMerge)
{
    struct_list *pList = hList;
    struct_list *pListToMerge = hListToMerge;
    struct_item *pRefItem;
    struct_item *pHead;

    if (hList == NULL || hListToMerge == NULL)
        return FALSE;

    // teste si la liste est vide, rien a faire ! 
    if (List_IsEmpty(hListToMerge) == TRUE)
        return TRUE;

    EnterCriticalSection(&pList->cs);
    EnterCriticalSection(&pListToMerge->cs);

    // si la référence est NULL, utiliser la tete
    if (RefItem == NULL)
        pRefItem = &pList->Head;
    else
    {
        if (ITEM_IS_LINKED(RefItem) == FALSE)
        {
            LeaveCriticalSection(&pListToMerge->cs);
            LeaveCriticalSection(&pList->cs);
            return FALSE;
        }

        pRefItem = ITEM_GET_HANDLE(RefItem);
    }

    pHead = &pListToMerge->Head;

    // préparation des items de debut et fin de liste
    pHead->prev->next = pRefItem;
    pHead->next->prev = pRefItem->prev;
    
    // insertion du premier et du dernier item 
    pRefItem->next->prev  = pHead->prev;
    pRefItem->next  = pHead->next;

    pList->dwCount += pListToMerge->dwCount;

    // Vidage de la liste source
    pListToMerge->Head.next = &pListToMerge->Head;
    pListToMerge->Head.prev = &pListToMerge->Head;
    pListToMerge->dwCount = 0;
    
    LeaveCriticalSection(&pListToMerge->cs);
    LeaveCriticalSection(&pList->cs);
    
    return TRUE;
}

BOOL WINAPI List_InsertInOrder (HLIST hList, PVOID NewItem, list_func_compare Compare, BOOL Duplicate)
{
    struct_list *pList = hList;
    PVOID *pItem;
    BOOL bRet;
    list_enum_compare Test;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    // teste si l'item n'est pas deja lié
    if (ITEM_IS_UNLINKED(NewItem) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return FALSE;
    }

    // on insére dans l'ordre croissant à partir de la tete
    pItem = List_GetHead(hList);
    
    // par defaut, si liste vide
    Test = LIST_IS_LOWER;

    // tant que l'on a pas atteind la fin de la liste
    while (pItem != NULL)
    {
        // si l'element à insérer est plus grand ou egal
        // on sort de la boucle
        Test = Compare (NewItem, pItem);
        if (Test <= LIST_IS_EQUAL)
            break;

        // sinon passe au suivant
        pItem = List_GetNext (hList, pItem);
    }

    // si un item existe deja et que les doublons sont interdits, ne pas insérer
    if (Test == LIST_IS_EQUAL && Duplicate == FALSE)
        bRet = FALSE;
    else
        // si pItem est NULL (liste vide ou le plus grand), l'insértion se fera en queue
        bRet = List_InsertBefore (hList, pItem, NewItem);

    LeaveCriticalSection(&pList->cs);
    
    return bRet;
}


PVOID WINAPI List_Remove (HLIST hList, PVOID Item)
{
    struct_list *pList = hList;
    struct_item *pItem;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);
    
    // si l'item n'est pas lié
    if (ITEM_IS_LINKED(Item) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return NULL;
    }
    
    pItem = ITEM_GET_HANDLE(Item);
    
    // on shunte les liens autour de l'item
    pItem->next->prev  = pItem->prev;
    pItem->prev->next  = pItem->next;
    
    // on casse les liens de l'item vers la liste
    pItem->next = NULL;
    pItem->prev = NULL;
    
    pList->dwCount --;
    
    LeaveCriticalSection(&pList->cs);

    return Item;
}

BOOL WINAPI List_AddHead (HLIST hList, PVOID NewItem)
{
    return List_InsertBefore(hList, List_GetHead(hList), NewItem);
}

BOOL WINAPI List_AddTail (HLIST hList, PVOID NewItem)
{
    return List_InsertAfter(hList, List_GetTail(hList), NewItem);
}

PVOID WINAPI List_RemoveHead (HLIST hList)
{
    return List_Remove (hList, List_GetHead(hList));
}

PVOID WINAPI List_RemoveTail (HLIST hList)
{
    return List_Remove (hList, List_GetTail(hList));
}

// ACCES / ITERATEURS

PVOID WINAPI List_GetHead (HLIST hList)
{
    struct_list *pList = hList;
    PVOID pItem;

    if (hList == NULL)
        return NULL;

    EnterCriticalSection(&pList->cs);

    // si liste vide
    if (List_IsEmpty(hList) == TRUE)
        pItem = NULL;
    else
        pItem = ITEM_GET_DATA(pList->Head.next);

    LeaveCriticalSection(&pList->cs);

    return pItem;
}

PVOID WINAPI List_GetTail (HLIST hList)
{
    struct_list *pList = hList;
    PVOID pItem;

    if (hList == NULL)
        return NULL;

    EnterCriticalSection(&pList->cs);

    // si liste vide
    if (List_IsEmpty(hList) == TRUE)
        pItem = NULL;
    else
        pItem = ITEM_GET_DATA(pList->Head.prev);

    LeaveCriticalSection(&pList->cs);

    return pItem;
}

PVOID WINAPI List_GetNext (HLIST hList, PVOID RefItem)
{
    struct_list *pList = hList;
    struct_item *pItem;

    if (hList == NULL)
        return NULL;

    EnterCriticalSection(&pList->cs);

    if (ITEM_IS_LINKED(RefItem) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return NULL;
    }
    
    pItem = ITEM_GET_HANDLE(RefItem)->next;
    
    // si element suivant est le dernier element de la liste
    if (pItem == &pList->Head)
        pItem = NULL;
    else
        pItem = ITEM_GET_DATA(pItem);

    LeaveCriticalSection(&pList->cs);
    
    return pItem;
}

PVOID WINAPI List_GetPrevious (HLIST hList, PVOID RefItem)
{
    struct_list *pList = hList;
    struct_item *pItem;

    if (hList == NULL)
        return NULL;
        
    EnterCriticalSection(&pList->cs);

    if (ITEM_IS_LINKED(RefItem) == FALSE)
    {
        LeaveCriticalSection(&pList->cs);
        return NULL;
    }
    
    pItem = ITEM_GET_HANDLE(RefItem)->prev;
    
    // si l'element précédent est le dernier element de la liste
    if (pItem == &pList->Head)
        pItem = NULL;
    else
        pItem = ITEM_GET_DATA(pItem);

    LeaveCriticalSection(&pList->cs);
    
    return pItem;
}

BOOL WINAPI List_EnterCriticalSection (HLIST hList)
{
    struct_list *pList = hList;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    return TRUE;
}

BOOL WINAPI List_LeaveCriticalSection (HLIST hList)
{
    struct_list *pList = hList;

    if (hList == NULL)
        return FALSE;

    LeaveCriticalSection(&pList->cs);

    return TRUE;
}

// PARCOURS

BOOL WINAPI List_ForEachItem (HLIST hList, list_func_action fAction, PVOID Param)
{
    struct_list *pList = hList;
    struct_item *pItem, *pTemp;

    if (hList == NULL)
        return FALSE;

    EnterCriticalSection(&pList->cs);

    pItem = List_GetHead(hList);
    while (pItem != NULL)
    {
        // on utilise une variable intermédiaire pour pouvoir
        // passer a l'item suivant AVANT d'appeler la fonction de l'utilisateur
        // Cette derniere pourrait tres bien supprimer l'element courant
        // d'ou probleme pour en chercher le suivant en retour
        pTemp = pItem;
        pItem = List_GetNext(hList, pItem);

        if (fAction(pTemp, Param) == FALSE)
        {
            LeaveCriticalSection(&pList->cs);
            return FALSE;
        }
    }

    LeaveCriticalSection(&pList->cs);

    return TRUE;
}


// ETAT

BOOL WINAPI List_IsEmpty(HLIST hList) 
{
    struct_list *pList = hList;

    // si NULL, vide
    if (hList == NULL)
        return TRUE;

    return pList->Head.next == &pList->Head;
}

DWORD WINAPI List_GetCount(HLIST hList)
{
    struct_list *pList = hList;

    // si NULL, 0 element
    if (hList == NULL) 
        return 0UL;

    return pList->dwCount;
}

DWORD WINAPI List_GetItemOverheadSize (VOID)
{
    return sizeof(struct_item);
}

// RECHERCHE

PVOID WINAPI List_FindHeadFirst (list_struct_find *pFind)
{
    struct_list *pList = pFind->hList;
    PVOID *pItem;

    EnterCriticalSection(&pList->cs);

    // ListFindNext() cherche a partir de l'element suivant LastItemFound
    // on passe donc la tete comme dernier element trouvé 
    // pour commencer le arecherche sur le suivant soit le premier de la liste
    pFind->LastItemFound = ITEM_GET_DATA(&pList->Head);
    pFind->LastFoundCount = 0;

    pItem = List_FindNext(pFind);

    LeaveCriticalSection(&pList->cs);

    return pItem;
}

PVOID WINAPI List_FindNext (list_struct_find *pFind)
{
    struct_list *pList = pFind->hList;
    PVOID *pItem;
    
    EnterCriticalSection(&pList->cs);
    
    // on continue la recherche à partir de l'element
    // suivant le dernier trouvé
    pItem = List_GetNext(pList, pFind->LastItemFound);

    // tant que l'on a pas atteind la fin de la liste
    while (pItem != NULL)
    {
        pFind->LastFoundCount ++;

        // sort de la boucle si l'element est celui recherché
        if (pFind->Equal (pFind->ItemToFind, pItem) == TRUE)
            break;

        // sinon passe au suivant
        pItem = List_GetNext (pList, pItem);
    }

    // pItem est NULL si pas trouvé
    pFind->LastItemFound = pItem;
    
    if (pItem == NULL)
        pFind->LastFoundCount = 0;

    LeaveCriticalSection(&pList->cs);
    
    return pItem;
}

PVOID WINAPI List_FindTailFirst (list_struct_find *pFind)
{
    struct_list *pList = pFind->hList;
    PVOID *pItem;

    EnterCriticalSection(&pList->cs);

    // ListFindNext() cherche a partir de l'element suivant LastItemFound
    // on passe donc la tete comme dernier element trouvé 
    // pour commencer le arecherche sur le suivant soit le premier de la liste
    pFind->LastItemFound = ITEM_GET_DATA(&pList->Head);
    pFind->LastFoundCount = 0;

    pItem = List_FindPrevious(pFind);

    LeaveCriticalSection(&pList->cs);

    return pItem;
}

PVOID WINAPI List_FindPrevious (list_struct_find *pFind)
{
    struct_list *pList = pFind->hList;
    PVOID *pItem;
    
    EnterCriticalSection(&pList->cs);
    
    // on continue la recherche à partir de l'element
    // suivant le dernier trouvé
    pItem = List_GetPrevious(pList, pFind->LastItemFound);

    // tant que l'on a pas atteind la fin de la liste
    while (pItem != NULL)
    {
        pFind->LastFoundCount ++;

        // sort de la boucle si l'element est celui recherché
        if (pFind->Equal (pFind->ItemToFind, pItem) == TRUE)
            break;

        // sinon passe au suivant
        pItem = List_GetPrevious (pList, pItem);
    }

    // pItem est NULL si pas trouvé
    pFind->LastItemFound = pItem;
    
    if (pItem == NULL)
        pFind->LastFoundCount = 0;

    LeaveCriticalSection(&pList->cs);
    
    return pItem;
}
