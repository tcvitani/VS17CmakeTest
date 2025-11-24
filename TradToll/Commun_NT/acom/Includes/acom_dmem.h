#ifndef DYNMEM_H
#define DYNMEM_H

// Mise à zéro d'un bloc d'après pointeur typé
#define DMEM_ZERO(p)                        (ZeroMemory(p,sizeof(*p)))

// Copie d'un bloc vers un autre d'après pointeur typé de la destination
#define DMEM_COPY(pd,ds)                    (CopyMemory(pd,ps,sizeof(*pd)))

// Allocation d'un bloc d'après pointeur typé
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ALLOC(p)                       (p=HeapAlloc(GetProcessHeap(),0,sizeof(*p)))

// Allocation d'un tableau de blocs d'après pointeur typé
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ALLOC_TABLE(p,n)               (p=HeapAlloc(GetProcessHeap(),0,sizeof(*p)*n))

// Allocation d'un block d'une taille différente de celle du type du pointeur
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ALLOC_BUFFER(p,b)              (p=HeapAlloc(GetProcessHeap(),0,b))

// Aloocation d'un bloc d'après pointeur typé, avec mise à zéro du bloc
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ZALLOC(p)                      (p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*p)))

// Allocation d'un tableau de blocs d'après pointeur typé, avec mise à zéro du tableau
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ZALLOC_TABLE(p,n)              (p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*p)*n))

// Allocation d'un block d'un taille différente de celle du type du pointeur avec mise à zéro du bloc
// Valeur retournée = pointeur alloué ou NULL
#define DMEM_ZALLOC_BUFFER(p,b)             (p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,b))

// Libération d'un bloc avec mise à NULL du pointeur
#define DMEM_FREE(p)                        (HeapFree(GetProcessHeap(),0,p),p=NULL)


#endif DYNMEM_H