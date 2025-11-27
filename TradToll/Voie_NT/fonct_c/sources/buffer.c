/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  ITEM
* FICHIER: TRAITBUF.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Traitement des buffers
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Sources/buffer.c_v  $
 * 
 *    Rev 1.0   11 May 1999 17:31:52   afx
 *  
 * 
 *    Rev 1.5   Jul 28 1997 17:17:48   DPI
 * Gestion de la version de la librairie
 * 
 *    Rev 1.4   Apr 30 1997 10:30:22   PDU
 *  
 * 
 *    Rev 1.3   Apr 25 1997 16:11:20   PDU
 * La taille d'un buffer est maintenant un int.
 * Dans la version precedente, c'etait un long.
 * 
 *    Rev 1.2   Apr 11 1997 10:47:16   DPI
 * Pb avec l'historique 
 *
 *    Rev 1.1   Apr 11 1997 10:14:24   DPI
 * Evolution apres creation

 *    Rev 1.0   Apr 11 1997 10:14:24   DPI
 * Creation
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "buffer.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"


/*--------------- VERSION: --------------*/



/*--------------- DEFINES: ----------------*/

#define MIN(x,y) (x>y?y:x)
#define MAX(x,y) (x>y?x:y)

/*--------------- FUNCTIONS: --------------*/
/*
PUBLIC void InitBuffer( TBuffer *buff );
PUBLIC void InitialiserBuffer( TBuffer *buff, char *contenu, int taille);
PUBLIC void AfficherBuffer( TBuffer *buff);

PUBLIC void AffecterBuffer( TBuffer *dest, TBuffer *source);
PUBLIC boolean CopierBuffer( TBuffer *dest,
                             TBuffer *source,
                             int      posDest,
                             int      tailleMaxDest );

PUBLIC void DecalerBuffer( TBuffer *buff,
                           int      offset);

PUBLIC boolean ChangerTailleBuffer( TBuffer *buff,
                                    int      nvTaille,
                                    int      tailleMaxBuff);

PUBLIC long LireEntierDansBuffer( TBuffer *buff, int tailleEntier );
PUBLIC boolean EcrireEntierDansBuffer( TBuffer *buff,
                                       int      tailleEntier,
                                       long     entier,
                                       int      positionEcriture,
                                       int      tailleMaxBuff );
*/

/*--------------- VARIABLES: --------------*/

/*--------------- CODE: -------------------*/


PUBLIC void WINAPI InitialiserBuffer( TBuffer *buff, char *contenu, int taille)
{
  //NO_WARNING( what);
  buff->contenu=contenu;
  buff->taille=taille;
}


PUBLIC void WINAPI InitBuffer( TBuffer *buff )
{
  InitialiserBuffer( buff, NULL, 0);
}


PUBLIC void WINAPI AfficherBuffer( TBuffer *buff )
{
  char contenu[ TAILLE_MAX_BUFFER+1];

  if (contenu)
  {
    memcpy(contenu, buff->contenu, buff->taille);
    contenu[buff->taille]='\0';

    printf("Buffer : contenu = %s,\n",contenu);
    printf("         taille  = %d.\n",buff->taille);
  }
}


PUBLIC void WINAPI AffecterBuffer( TBuffer *dest, TBuffer *source)
{
  dest->contenu = source->contenu;
  dest->taille  = source->taille;
}



PUBLIC boolean WINAPI CopierBuffer( TBuffer *dest,
                             TBuffer *source,
                             int      posDest,
                             int      tailleMaxDest )
/*PUBLIC boolean CopierBuffer( TBuffer *dest,
                             TBuffer *source,
                             long posD,
                             long tailleMaxD )*/
{
   boolean retour = TRUE;
   int i=0;

  if (posDest > 0)
  {
    i = 0;
    while ( (i+posDest-1 < tailleMaxDest) &&
            (i < source->taille) )
    {
      dest->contenu[i+posDest-1] = source->contenu[i];

      i++;
    }

    if(i+posDest-1 >= tailleMaxDest)
    {
      retour = FALSE;
    }

    dest->taille = MAX(i+posDest-1,dest->taille);
  }

  return( retour);
}


PUBLIC void WINAPI DecalerBuffer(TBuffer *buff, int offset)
{
  if (offset > 0)
  {
    if (buff->taille != -1)
       buff->taille -= offset;
    buff->contenu += offset;
  }
}

PUBLIC boolean WINAPI ChangerTailleBuffer( TBuffer *buff,
                                    int      nvTaille,
                                    int      tailleMaxBuff)
{
   boolean retour = TRUE;

  if (nvTaille >=0)
  {
    if (tailleMaxBuff != -1)
    {
      if (nvTaille >= tailleMaxBuff)
      {
         retour = FALSE;
      }
      else
      {
         buff->taille = MIN( nvTaille, tailleMaxBuff );
      }
    }
    else
    {
      buff->taille = nvTaille;
    }
  }

  return( retour);
}


PUBLIC long WINAPI LireEntierDansBuffer( TBuffer *buff, int tailleEntier )
{
   long retour=0;

   switch (tailleEntier)
   {
      case 1:
      {
         char *val = (char *)(buff->contenu);
         retour = (long)(*val);
      }
      break;

      case 2:
      {
         short *val = (short *)(buff->contenu);
         retour = (long)(*val);
      }
      break;

      case 4:
      {
         long *val = (long *)(buff->contenu);
         retour = (long)(*val);
      }
      break;

      default : printf("ERREUR INTEGRATION (taille=%d) \n", tailleEntier);
                ERR_ErreurFatale();
   } // fin switch

   return( retour);
}


PUBLIC boolean WINAPI EcrireEntierDansBuffer( TBuffer *buff,
                                       int      tailleEntier,
                                       long     entier,
                                       int      positionEcriture,
                                       int      tailleMaxBuff )
{
   boolean retour = TRUE;

   char * contenu = buff->contenu;
   contenu += positionEcriture-1;

   // on verifie si on peut faire la copie
   if ( (tailleMaxBuff == -1) ||
        (tailleEntier+positionEcriture-1 <= tailleMaxBuff ) )
   {
      // oui, on peut, c'est parti ...
      switch (tailleEntier)
      {
         case 1:
         {
            char *val = (char *)(contenu);
            *val = (char)entier;
         }
         break;

         case 2:
         {
            short *val = (short *)(contenu);
            *val = (short)entier;
         }
         break;

         case 4:
         {
            long *val = (long *)(contenu);
            *val = (long)entier;
         }
         break;

         default : printf("ERREUR INTEGRATION !!!\n");
                   ERR_ErreurFatale();

      } // fin switch

      // MAJ de la taille du buffer.
      buff->taille = MAX( buff->taille, tailleEntier+positionEcriture-1 );
   }
   else
   {
     // le buffer destination va deborder !!!
     retour = FALSE;
   }

   return( retour );
}


