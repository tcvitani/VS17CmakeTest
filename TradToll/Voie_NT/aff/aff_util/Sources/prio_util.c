/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     prio_util.c												     */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*--------------------------- INCLUDES:  --------------------------*/

#include <stdio.h>
#include <string.h>

#include <noyau.h>
#include <debug.h>
#include <horodate.h>
#include <mem_c.h>
#include <fic_gere.h>

#include <aff_ext.h>

#include <aff_util.h>

/*--------------------------- RESERVED:  --------------------------*/

#include <memclass.h>

/*--------------------------- EXTERNALS: --------------------------*/

/*--------------------------- DEFINES:   --------------------------*/

/*--------------------------- TYPEDEFS:  --------------------------*/

/*--------------------------- FUNCTIONS: --------------------------*/

/*--------------------------- VARIABLES: --------------------------*/

/*--------------------------- CODE: -------------------------------*/

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED void WINAPI AFF_MsgRequestFreeLine(struct_request_screen *pLigne)
{
   struct_list_text *liste, *liste_suiv;

   liste = pLigne->texte;

   /* Liberation des listes */
   while (liste != NULL)
   {
      liste_suiv = liste->suiv;

      MEM_free(liste);

      liste = liste_suiv;
   }

   /* Reset des information demande ecran */
   pLigne->texte = NULL;
   pLigne->bal = NO_BAL;
}

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED void AFF_MsgRequestFreeTab(struct_screen *pScreen)
{
	int						index;
	struct_request_screen	*table;

	for (index = 0; index < NB_MAX_AFF_PRIO; index++)
	{
		table = &pScreen->tab_request[index];

		AFF_MsgRequestFreeLine(table);
	}
}

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED struct_list_text *AFF_MsgResearchLineNumber(struct_request_screen *pLine, short int iNumber)
{
   int index;
   struct_list_text **liste;

   liste = &pLine->texte;

   /* Recherche de la ligne demand‚e */
   index = iNumber;
   while( index > 1)
   {
      if( *liste == NULL)
      {
         /* On sort */
         break;
      }
      liste = &(*liste)->suiv;
      index --;
   }

   return(*liste);
}

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED struct_list_text *AFF_MsgRequestLineNumber(struct_request_screen *pLine, short int iNumber)
{
   int index;
   struct_list_text **liste, *liste_trouvee = NULL;

   liste = &pLine->texte;

   /* Recherche de la ligne demand‚e */
   index = iNumber;
   while( index >= 1)
   {
      if( *liste == NULL)
      {
         /* Si la ligne n'existe pas, elle est cr‚e */
         *liste = MEM_malloc( sizeof( struct_list_text));
         memset( *liste, 0, sizeof( struct_list_text));
      }
      liste_trouvee = *liste;
      liste = &(*liste)->suiv;
      index --;
   }

   return( liste_trouvee);
}

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED int AFF_MsgRequestCurrentPriority(struct_screen *pScreen)
{
   int index;

   /* Recherche de la priorit‚ affich‚e */
   for (index = NB_MAX_AFF_PRIO-1; index >= 0; index--)
   {
	   if (pScreen->tab_request[index].bal != NO_BAL)
         break;
   }

   return( index);
}

/**/
/*******************************************************************/
/*SYNTAX:														   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*                                                                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*                                                                 */
/*******************************************************************/
PROTECTED int AFF_MsgRequestLinePriority(short int iNumber, struct_screen *pScreen)
{
	int						index;
	int						prio_en_cours;
	struct_list_text		*p_texte;
	struct_request_screen	*p_ligne;

	prio_en_cours = AFF_MsgRequestCurrentPriority(pScreen);
	
	/* Recherche de la priorit‚ affich‚e */
	for( index = prio_en_cours; index >= 0; index--)
	{
		if (pScreen->tab_request[index].bal != NO_BAL)
		{
			/* Il existe une demande */
			p_ligne = &pScreen->tab_request[index];
			p_texte = AFF_MsgResearchLineNumber(p_ligne, iNumber);

			/* Si cette ligne n'a pas de donnee associee et que ce n'est pas la premiere */
			/* Ce cas arrive lorsque la premiere ligne est de police double ('B') */
			if ((p_texte == NULL) && (iNumber != 1))
				/* recherche des informations sur la premiere ligne */
				p_texte = AFF_MsgResearchLineNumber(p_ligne, 1);

			/* Si la police existe */
			if ((p_texte != NULL) && (p_texte->police != LGN_INVISIBLE))
				break;
		}
	}
		
	return( index);
}
