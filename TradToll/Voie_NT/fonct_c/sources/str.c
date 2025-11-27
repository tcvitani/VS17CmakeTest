/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Fonctions de base utilisant les string
* FICHIER: STR.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Ce module
*            - encapsule les fonctions de base ( ex:strcpy) en ajoutant
*              des garde-fous
*            - fournit des fonctions de tests en plus
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/STR.C_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:42:14   bph
 *  
 * 
 *    Rev 1.3   Jul 08 1997 18:18:40   HMO
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "err.h"

#include "str.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: --------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- FUNCTIONS: --------------*/

void STR_FullVerify( char *__FILE, int __LINE, int Taille, char *Source)
{
   STR_FullStrlen( __FILE, __LINE, Taille, Source);
}


char  *STR_FullStrcpy( char *__FILE, int __LINE,
                       int Taille, char *Dest, char *Source)
{
   int   i;

   i = 0;
   while( ( i != Taille ) && ( Source[i] != '\0'))
   {
      Dest[i] = Source[i],
      i++;
   }

   if( i >= Taille)
      ERR_TraiterErreurFatale( __FILE, __LINE);

   Dest[i] = '\0';
   return( Dest);
}

char  *STR_FullStrncpy( char *__FILE, int __LINE,
                       int Taille, char *Dest, char *Source)
{
   int   i;

   NO_WARNING( __FILE);
   NO_WARNING( __LINE);

   i = 0;
   while( ( i != Taille ) && ( Source[i] != '\0'))
   {
      Dest[i] = Source[i],
      i++;
   }

   if( i >= Taille)
       i = Taille -1;

   Dest[i] = '\0';
   return( Dest);
}


char  *STR_FullStrcat( char *__FILE, int __LINE,
                       int Taille, char *Dest, char *Source)
{
   int   i;
   char  *local_dest;

   local_dest = Dest;

   i = 0;
   while( ( i != Taille ) && ( local_dest[0] != '\0'))
   {
      local_dest++;
      i++;
   }

   if( i >= Taille)
      ERR_TraiterErreurFatale( __FILE, __LINE);

   STR_FullStrcpy( __FILE, __LINE, Taille - i, local_dest, Source);
   return( Dest);
}

int  STR_FullStrlen( char *__FILE, int __LINE,
                       int Taille, char *Source)
{
   int   i;

   i = 0;
   while( ( i != Taille ) && ( Source[i] != '\0'))
   {
      i++;
   }

   if( i >= Taille)
      ERR_TraiterErreurFatale( __FILE, __LINE);

   return( i);
}
