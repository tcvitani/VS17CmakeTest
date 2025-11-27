/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base utilisant la memoire
* FICHIER: MEM.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module
*            - encapsule les fonctions de base ( ex:malloc) en ajoutant
*              des garde-fous
*            - fournit des fonctions de tests en plus
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/MEM_C.C_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:42:12   bph
 *  
 * 
 *    Rev 1.6   02 Dec 1997 17:14:04   DPI
 * Ajout de la fonction 
 * MEM_GetMaxFreeMemoryBlock
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"

#include "mem_c.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: --------------*/
/* Permet de controler le nombre d'allocations effectu‚es */
/* NbAllocation = nombre de Malloc,Calloc moins le nombre de Free */
PRIVATE signed long NbAllocation = 0;

/*--------------- DEFINES: ----------------*/

/*--------------- FUNCTIONS: --------------*/

void   *MEM_FullMalloc( char *__FILE, int __LINE, int taille)
{
   void           *ptr;

   ptr = malloc( taille);
   if( ptr ==NULL)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   NbAllocation++;
   return( ptr);
}

void   *MEM_FullCalloc( char *__FILE, int __LINE, int nb, int Taille)
{
   void           *ptr;

   ptr = calloc( nb, Taille);
   if( ptr ==NULL)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   NbAllocation++;
   return( ptr);
}

void   *MEM_FullRealloc( char *__FILE, int __LINE, void *Ptr, int Taille)
{
   void           *ptr;

   ptr = realloc( Ptr, Taille);
   if( ptr ==NULL)
       ERR_TraiterErreurFatale( __FILE, __LINE);

   return( ptr);
}

void MEM_FullFree( char *__FILE, int __LINE, char **Ptr)
{
   __FILE = __FILE;
   __LINE = __LINE;
   free( *Ptr);
   NbAllocation--;
   *Ptr = NULL;
}

signed long MEM_NbAllocation( void)
{
   return( NbAllocation);
}

unsigned long MEM_GetMaxFreeMemoryBlock(void)
{
   MEMORYSTATUS memory;

   memory.dwLength = sizeof(MEMORYSTATUS);

   GlobalMemoryStatus (&memory);

   return (unsigned long)memory.dwAvailVirtual;
}
