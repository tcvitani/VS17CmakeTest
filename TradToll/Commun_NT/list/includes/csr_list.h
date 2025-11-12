/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Generic circular double linked list component
* FICHIER: csr_list.h
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef CSR_LIST_H
#define CSR_LIST_H

/*--------------- INCLUDES: ---------------*/
#include <windows.h>

/*--------------- TYPEDEFS: ---------------*/

typedef HANDLE HLIST;

typedef enum 
{
    LIST_IS_LOWER = -1,
    LIST_IS_EQUAL,
    LIST_IS_GREATER
}
list_enum_compare;

typedef list_enum_compare (WINAPI * list_func_compare)(PVOID FirstItem, PVOID SecondItem);

typedef BOOL (WINAPI * list_func_equal)(PVOID FirstItem, PVOID SecondItem);

typedef struct ListFind
{
    IN HLIST hList;
    IN PVOID ItemToFind;
    IN list_func_equal Equal;
    OUT PVOID LastItemFound;
    OUT DWORD LastFoundCount;
}
list_struct_find;

typedef BOOL (WINAPI * list_func_action)(PVOID Item, PVOID Param);

/*--------------- FUNCTIONS: ---------------*/

// CONSTRUCTORS / DESTRUCTORS

HLIST WINAPI List_New (VOID);

BOOL WINAPI List_Delete (IN OUT HLIST *phList);

BOOL WINAPI List_DeleteAll (IN OUT HLIST *phList, list_func_action fAction, PVOID Param);

PVOID WINAPI List_ItemNew (HLIST hList, DWORD SizeOfItem);

BOOL WINAPI List_ItemDelete (HLIST hList, IN OUT PVOID *pItem);

// INSERTION / EXTRACTION

BOOL WINAPI List_AddTail (HLIST hList, PVOID NewItem);

BOOL WINAPI List_AddHead (HLIST hList, PVOID NewItem);

PVOID WINAPI List_RemoveHead (HLIST hList);

PVOID WINAPI List_RemoveTail (HLIST hList);

BOOL WINAPI List_InsertAfter (HLIST hList, PVOID RefItem, PVOID NewItem);
    
BOOL WINAPI List_InsertBefore (HLIST hList, PVOID RefItem, PVOID NewItem);

BOOL WINAPI List_InsertInOrder (HLIST hList, PVOID NewItem, list_func_compare Compare, BOOL Duplicate);

BOOL WINAPI List_MergeAfter (HLIST hList, PVOID RefItem, HLIST hListToMerge);

BOOL WINAPI List_MergeBefore (HLIST hList, PVOID RefItem, HLIST hListToMerge);

PVOID WINAPI List_Remove (HLIST hList, PVOID Item);

// ACCESS / ITERATORS

PVOID WINAPI List_GetHead (HLIST hList);

PVOID WINAPI List_GetTail (HLIST hList);

PVOID WINAPI List_GetNext (HLIST hList, PVOID RefItem);

PVOID WINAPI List_GetPrevious (HLIST hList, PVOID RefItem);

BOOL WINAPI List_ForEachItem (HLIST hList, list_func_action fAction, PVOID Param);

BOOL WINAPI List_EnterCriticalSection (HLIST hList);

BOOL WINAPI List_LeaveCriticalSection (HLIST hList);

// STATUS

BOOL WINAPI List_IsEmpty(HLIST hList);

DWORD WINAPI List_GetCount(HLIST hList);

DWORD WINAPI List_GetItemOverheadSize (VOID);

// SEARCH

PVOID WINAPI List_FindHeadFirst (IN OUT list_struct_find *pFind);

PVOID WINAPI List_FindNext (IN OUT list_struct_find *pFind);

PVOID WINAPI List_FindTailFirst (IN OUT list_struct_find *pFind);

PVOID WINAPI List_FindPrevious (IN OUT list_struct_find *pFind);

#endif
