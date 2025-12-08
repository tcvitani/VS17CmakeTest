/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     file_util.c												     */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*--------------------------- INCLUDES:  --------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#include <mem_c.h>
#include <str.h>
#include <err.h>
#include <noyau.h>
#include <horodate.h>
#include <fic_gere.h>

#include <aff_ext.h>

#define LOC_DEF
#include <aff_util.h>
#undef LOC_DEF

/*--------------------------- RESERVED:  --------------------------*/

#include <memclass.h>

/*--------------------------- EXTERNALS: --------------------------*/

/*--------------------------- DEFINES:   --------------------------*/

PROTECTED struct_mot_cle mots_cles[NB_MAX_MOT_CLE]
= {
	{ "AFF_SOFT_REV"		},
	{ "AFF_PAYMENT_TYPE"	},
	{ "AFF_CLASS"			},
	{ "AFF_LIBELLE1"		},
	{ "AFF_LIBELLE2"		},
	{ "AFF_LIBELLE3"		},
	{ "AFF_FARE_INT"		},
	{ "AFF_PAID_INT"		},
	{ "AFF_SALEFARE_INT"	},
	{ "AFF_SALEPAID_INT"	},
	{ "AFF_FARE_DEC"		},
	{ "AFF_PAID_DEC"		},
	{ "AFF_SALEFARE_DEC"	},
	{ "AFF_SALEPAID_DEC"	},
	{ "AFF_YEAR"			}, 
	{ "AFF_YEAR_2_DIGIT"	},
	{ "AFF_MONTH"			},
	{ "AFF_DAY"				},
	{ "AFF_HOUR"			},
	{ "AFF_MINUTE"			},
	{ "AFF_SECOND"			},
	{ "PRECISION_TARIF"		},
	{ "AFF_AMOUNT_DUE_INT"	},
	{ "AFF_AMOUNT_DUE_DEC"	},
	{ "AFF_OVERPAY_INT"		},
	{ "AFF_OVERPAY_DEC"		},
};

/*--------------------------- TYPEDEFS:  --------------------------*/

/*--------------------------- FUNCTIONS: --------------------------*/

PRIVATE FIC_enum_retour ReadItem(FILE *Fichier, TParam *Param, struct_line_info *ItemRetour, int *Taille_Totale);

PRIVATE FIC_enum_retour WriteItem(FILE *Fichier, struct_line_info *Item, TParam *Param, int *Taille_ecrite);

PRIVATE FIC_enum_retour ReadIdValue(struct_line_info *Item, long Position, TParam *Param, struct_line_id *Id, long *Val);

PRIVATE int CompareId(struct_line_id *Id, struct_line_id *St_Id, TParam *Param);

PRIVATE void RemoveCharacter(char *chaine);

PRIVATE boolean AnalyzeLine(struct_line_list *ad_cur_ligne);

PRIVATE boolean ExtractVariable(char *fmt, char *var, struct_line_list *pl);

PRIVATE boolean ForamtYear(char *fmt, struct_line_list *pl);

/*--------------------------- VARIABLES: --------------------------*/

PROTECTED long no_ligne = 0;

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
PROTECTED boolean AFF_InitFile(TGereFic *pGereFic,
								char *pFileName, 
								char *pAttributes, 
								struct_line_info *pItem, 
								struct_line_id *pId, 
								long *pVal)
{
	if (!Fichier_Charger(pGereFic, pFileName, pAttributes, 0, ReadItem, WriteItem))
	{
      return FALSE;
	}

	if(!Fichier_Configurer(pGereFic, 
							FIC_NON_TRIE, 
							0,
							ReadIdValue, 
							NULL, 
							CompareId,
							pItem,
							pId,
							pVal))
	{
		return FALSE;
	}

	return TRUE;
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
PROTECTED boolean AFF_CloseFile(TGereFic *pGereFic)
{
	return (Fichier_Fermer(pGereFic));
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
PROTECTED FIC_enum_retour ReadTextPosition(TGereFic *pGereFic, long lPosition, struct_line_id  **ppId, long **ppVal)

{
	return(Fichier_LirePos(pGereFic, lPosition, NULL, NULL, ppId, ppVal));
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
PRIVATE FIC_enum_retour ReadItem(FILE *Fichier, TParam *Param, struct_line_info *ItemRetour, int *Taille_Totale)
{
   FIC_enum_retour   retour;
   char              *chaine;
   boolean           boucle;
   int				taille_lue;

   *Taille_Totale = 0;
   boucle = TRUE;

   NO_WARNING( Param);

   while ( boucle)
   {
      /* Lecture d'une ligne */
      chaine = fgets( ItemRetour->ligne, LGN_MAX_CAR, Fichier);

      if( chaine == NULL)
      {
         boucle = FALSE;
         retour = FIC_FINI;
      }
      else
      {
         taille_lue = (int)strlen( ItemRetour->ligne);
         *Taille_Totale = *Taille_Totale + taille_lue;

         /* M‚morisation du num‚ro de ligne */
         no_ligne++;

         /* Suppression des espaces inutiles */
         RemoveCharacter(chaine);

         /* est-ce que la chaine se limite au \r\n, ou bien est-ce un
          * commentaire
          */
         if(( taille_lue > 1) && ( ItemRetour->ligne[ 0] != LGN_COMMENTS ))
         {
            /* Memorisation de la ligne */
            ItemRetour->no_ligne = no_ligne;

            /* Suppression du \r\n en fin de chaine */
            chaine[strlen(chaine)-1] = 0;

            boucle = FALSE;
            retour = FIC_OK;
         }
      }
   }

   return( retour);
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
PRIVATE FIC_enum_retour WriteItem(FILE *Fichier, struct_line_info *Item, TParam *Param, int *Taille_ecrite)
{
   FIC_enum_retour   retour;

   NO_WARNING(Fichier);
   NO_WARNING(Item);
   NO_WARNING(Param);
   NO_WARNING(Taille_ecrite);

   retour = FIC_NOK;

   return( retour);
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
PRIVATE FIC_enum_retour ReadIdValue(struct_line_info *Item, long Position, TParam *Param, struct_line_id *Id, long *Val)
{
   FIC_enum_retour retour;
   enum_type_ligne type_ligne;

   NO_WARNING( Position);
   NO_WARNING( Val);
   NO_WARNING( Param);

   retour = FIC_OK;

   /* Analyse du type de mot suivant le premier caractŠre */
   switch( Item->ligne[0])
   {
      /* C'est un mot cl‚ */
      case LGN_CLE:
         type_ligne = LGN_MOT_CLE;
         break;

      /* C'est un label */
      case LGN_LABEL:
         type_ligne = LGN_MOT_LABEL;
         break;

      default:
         if( Item->ligne[1] == '"')
            type_ligne = LGN_LIGNE;
         else
            type_ligne = LGN_ERREUR;
         break;
   }

   if( type_ligne == LGN_ERREUR)
      retour = FIC_ITEM_NOK;
   else
   {
      STR_strcpy( LGN_MAX_CAR, Id->ligne, Item->ligne);
      Id->type_ligne = type_ligne;
      *Val = Item->no_ligne;
      retour = FIC_OK;
   }

   /* M‚morisation du num‚ro de ligne */
   Id->no_ligne = Item->no_ligne;

   return( retour);
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
PRIVATE int CompareId(struct_line_id *Id, struct_line_id *St_Id, TParam *Param)
{
   int      cmp;

   NO_WARNING( Param);

   cmp = _stricmp( Id->ligne, St_Id->ligne);

   return( cmp);
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
PROTECTED boolean ReadMessage(TGereFic *pGereFic, struct_label *pLabel, long *pLine, int *pPosition)
{
	boolean				msg_ok = TRUE;
	FIC_enum_retour		retour;
	TInfoFlux			InfoFlux;
	long				ancien_no_ligne;
	struct_line_id		*mon_id;
	long				*ma_val;
	struct_line_list	**ad_cur_ligne;

	memset(pLabel, 0, sizeof(struct_label));

	retour = ReadTextPosition(pGereFic, *pPosition, &mon_id, &ma_val);
	(*pPosition)++;

	if (retour == FIC_OK)
	{
		switch (mon_id->type_ligne)
		{
		case LGN_MOT_CLE:
			pLabel->type = MSG_MOT_CLE;
			STR_strcpy(AFF_MAX_LABEL, pLabel->label, &mon_id->ligne[1]);
			break;

		case LGN_MOT_LABEL:
			/* Remplissage du label */
			pLabel->type = MSG_LABEL;
			STR_strcpy(AFF_MAX_LABEL, pLabel->label, &mon_id->ligne[1]);

			/* Recherche des lignes associ‚es au label */
			mon_id->type_ligne = LGN_LIGNE;
			ad_cur_ligne = &pLabel->ligne;

			while (msg_ok && mon_id->type_ligne == LGN_LIGNE)
			{
				/* Memoriser l'element qui va etre lu et son no de pLine */
				if (Fichier_Memoriser(pGereFic, &InfoFlux) != FIC_OK)
					msg_ok = FALSE;
				ancien_no_ligne = no_ligne;

				retour = ReadTextPosition(pGereFic, *pPosition, &mon_id, &ma_val);

				/* La pLine contient une erreur */
				if (retour != FIC_OK)
				{
					mon_id->type_ligne = LGN_ERREUR;
					msg_ok = FALSE;
				}

				/* Analyse de la pLine */
				if (mon_id->type_ligne == LGN_LIGNE)
				{
					*ad_cur_ligne = MEM_malloc(sizeof(struct_line_list));
					memset(*ad_cur_ligne, 0, sizeof(struct_line_list));

					(*ad_cur_ligne)->txt_fmt = MEM_malloc((int)strlen(mon_id->ligne) + 1);

					STR_strcpy((int)strlen(mon_id->ligne) + 1, (*ad_cur_ligne)->txt_fmt, mon_id->ligne);

					/* Analyse de la police, des variables et des codes hexa */
					msg_ok = AnalyzeLine(*ad_cur_ligne);

					(*pPosition)++;
					
					ad_cur_ligne = &((*ad_cur_ligne)->suiv);
				}
				else
				{
					/* Se replacer sur l'element precedant */
					if (Fichier_Restaurer(pGereFic, &InfoFlux) != FIC_OK)
						msg_ok = FALSE;
					
					no_ligne = ancien_no_ligne;
				}
			}
			break;

		case LGN_LIGNE:
		case LGN_ERREUR:
		default:
			msg_ok = FALSE;
			break;
		}
	}
	else
	{
		/* mon_id n'a pas pu ˆtre actualis‚ */
		msg_ok = FALSE;
		*pLine = no_ligne;
		
		return(msg_ok);
	}

	/* M‚morisation du num‚ro de pLine */
	*pLine = mon_id->no_ligne;

	return(msg_ok);
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
PRIVATE boolean AnalyzeLine( struct_line_list *ad_cur_ligne)
{
   signed int val;

   char *debut_ligne,*fin_ligne,*anc_fin,*p_hexa;

   /* M‚morisation de la police */
   ad_cur_ligne->police = ad_cur_ligne->txt_fmt[0];

   if( ad_cur_ligne->txt_fmt[0] != LGN_INVISIBLE)
   {
      /* Le premier caractŠre doit ˆtre une lettre majuscule pour la police */
      if( isupper (ad_cur_ligne->txt_fmt[0]) )
      {
         /* Si le code de controle ne fait pas parti de la liste */
         /* des polices autoris‚es : (A et B) */
         if(( ad_cur_ligne->txt_fmt[0] - 'A') >= AFF_NB_POLICES ||
            ( ad_cur_ligne->txt_fmt[0] - 'A') < 0)
         {
            return( FALSE);
         }
      }
      else
         return( FALSE);
   }


   /* Recherche des guillemets de d‚but de ligne */
   debut_ligne = strstr (&ad_cur_ligne->txt_fmt[1], "\"");
   if( debut_ligne == NULL)  /* Guillemets de d‚but non trouv‚s */
      return( FALSE);

   /* Recherche des guillemets de fin de chaine */
   anc_fin = NULL;
   fin_ligne = debut_ligne;
   while( (fin_ligne = strstr( fin_ligne+1, "\"")) != NULL)
   {
      anc_fin = fin_ligne;
   }
   fin_ligne = anc_fin;

   /* Guillemets de fin non trouv‚s */
   if (fin_ligne == NULL)
      return( FALSE);


   /* Recherche des variables utilis‚es */
   if( ExtractVariable( debut_ligne, fin_ligne, ad_cur_ligne) == FALSE)
      return( FALSE);


   /* Rechercher des valeurs hexadecimales eventuelles */
   /* une valeur hexa est codee sous la forme "\xHH"   */
   p_hexa = debut_ligne;
   while( (p_hexa < fin_ligne) && (p_hexa != NULL) )
   {
      if ((p_hexa = strstr (debut_ligne, "\\x")) != NULL)
      {
         val = -1;
         sscanf_s(&p_hexa[2], "%2x", &val);
         if (val != -1)
         {
            *p_hexa = (char)val;
            memmove (&p_hexa[1], &p_hexa[4], strlen (&p_hexa[4])+1);
            fin_ligne -= 3;
         }
         else
         {
            p_hexa += 2;
         }
      }
   }


   /* D‚calage de la chaine pour la police et le premier guillemet */
   memmove( ad_cur_ligne->txt_fmt,
            &ad_cur_ligne->txt_fmt[2],
            strlen( &ad_cur_ligne->txt_fmt[2]) + 1);


   return( TRUE);
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
PRIVATE void RemoveCharacter( char *chaine)
{
   size_t taille,
          position;

   char *guillemet;


   taille = strlen( chaine);

   position = 0;
   while( position < taille)
   {
      switch( chaine[position])
      {
         /* Caractere a supprimer */
         case ' ' :
         case '\t':
            memmove( &chaine[position],
                     &chaine[position+1],
                     strlen( &chaine[position+1]) + 1);
            taille--;
            break;

         /* Pas de changement entre guillemet */
         case '"' :
            /* Recherche du guillemet suivant */
            guillemet = strstr (&chaine[position + 1], "\"");
            if( guillemet != NULL)
            {
               /* Positionnement apres le guillemet */
               position = (size_t)(guillemet - chaine) + 1;
            }
            else
               position++; // On passe l'erreur, sera analyse plus loin
            break;

         default :
            position++;
            break;
      }
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
PRIVATE boolean ExtractVariable (char *fmt, char *var, struct_line_list *pl)
{
   int n,i,ind_var,fin;
   char *pvar, *finvar, une_var[32];

   boolean retour = TRUE;

   fin = 0;
   ind_var = 0;
   pvar = var;

   /* nombre de mots cl‚s total */
   n = sizeof (mots_cles) / sizeof (mots_cles[0]);

   /* Recherche s'il y a des variables */
   pvar = strstr (pvar, ";");
   if( pvar != NULL)
   {
      /* On passe le premier ; */
      pvar++;

      while ( retour == TRUE && !fin)
      {
         /* recherche de la prochaine variable */
         finvar = strstr (pvar, ";");
         if (finvar != NULL)
         {
            /* recopie du nom de la variable */
			 strncpy_s(une_var, sizeof(une_var), pvar, (int)(finvar - pvar));
            /* fin de chaine */
            une_var[(int)(finvar - pvar)] = '\x0';
            pvar = finvar + 1;
         }
         else
         {
            /* pas d'autres variables */
			 strcpy_s(une_var, sizeof(une_var), pvar);
            fin = 1;
         }


         /* comparaison de la variable trouv‚e avec un mot cl‚ */
         retour = FALSE;
         for (i=0; i<n; i++)
         {
            if (strstr (une_var, mots_cles[i].nom) != NULL)
            {
               /* Le nombre max de variables autoris‚ est atteind */
               if( ind_var >= MAX_VARIABLES)
                  retour = FALSE;
               else
               {
                  retour = TRUE;

                  /* incrementer le nombre de variables de la ligne */
                  pl->nb_var ++;

                  /* associer la variable … un mot cl‚ */
                  pl->motcle[ind_var] = i;
                  ind_var ++;
                  /* cas particulier du format d'une date */
                  if (strcmp (mots_cles[i].nom, "AFF_YEAR") == 0)
                  {
                     retour = ForamtYear (fmt, pl);
                  }
               }

               break; /* Sortie de boucle for */
            }
         }
      }
   }

   /* Suppression du dernier guillemet */
   var[0] = 0;

   return( retour);
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
PRIVATE boolean ForamtYear(char *fmt, struct_line_list *pl)
{
   boolean retour = FALSE;

   char *deb;
   int indice;

   int annee;

   indice = 0;
   deb = fmt;

   /* Recherche du formateur de l'ann‚e dans la chaine */
   do
   {
      deb = strstr (deb, "%");
      if (deb != NULL)
      {
         indice ++;
         deb ++;
      }
   }
   while ((deb != NULL) && (indice < pl->nb_var));

   /* Si formateur trouve */
   if ((deb != NULL) && (indice == pl->nb_var))
   {
      retour = TRUE;
      annee = -1;
      do
      {
         switch (*deb)
         {
            case '2' :
               annee = AFF_DIGIT2;
               break;

            case '4' :
               annee = AFF_DIGIT4;
               break;

            case '0' :
               deb ++;
               break;

            default :
               retour = FALSE;
               break;
         }
      }
      while( (annee == -1) &&
             (deb < (fmt + strlen( fmt))) &&
             retour == TRUE);
   }

   /* M‚morisation du champ pour affichage 2 ou 4 digits */
   if( retour == TRUE && annee == AFF_DIGIT2)
      pl->motcle[pl->nb_var-1]++;


   return( retour);
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
PROTECTED boolean AFF_ReadFile(char *pFile, struct_label *pLabel, int iNbLabel, long *pLine, int *pPosition)
{
   TGereFic			pGereFic;
   struct_line_info Item;
   struct_line_id   Id;
   long             Val;
   boolean          retour;
   struct_label     sLabel;
   int              position1;
   long				i;
   struct_line_list *lpCurrentLine, *lpToDelete;

   // Free previouse format lines
   for (i = 0; i < iNbLabel; i++)
   {
	   lpCurrentLine = pLabel[i].ligne;

	   while (lpCurrentLine != NULL )
	   {
		   lpToDelete = lpCurrentLine;
		   lpCurrentLine = lpCurrentLine->suiv;

		   if (lpToDelete->txt_fmt != NULL)
		   {
			   free( lpToDelete->txt_fmt );
			   
			   lpToDelete->txt_fmt = NULL;
		   }

		   free(lpToDelete);

		   lpToDelete = NULL;
	   }
   }   

   memset(pLabel, 0, sizeof(struct_label) * iNbLabel);

   AFF_InitFile(&pGereFic, pFile, "rt", &Item, &Id, &Val);

   no_ligne = 0;

   retour = ReadMessage(&pGereFic, &sLabel, pLine, pPosition);

   if (retour == TRUE && sLabel.type == MSG_MOT_CLE && strcmp("MESSAGES", sLabel.label) == 0)
   {
	  position1 = 0;

      sLabel.type = MSG_LABEL;

      while (retour == TRUE && position1 < iNbLabel && sLabel.type == MSG_LABEL)
      {
         retour = ReadMessage(&pGereFic, &sLabel, pLine, pPosition);

         if(retour == TRUE && sLabel.type == MSG_LABEL)
         {
            pLabel[position1] = sLabel;

            position1++;
         }
      }

      if( !( retour == TRUE && sLabel.type == MSG_MOT_CLE && strcmp( "END", sLabel.label) == 0) )
      {
         retour = FALSE;
      }
   }
   else
   {
      retour = FALSE;

      return retour;
   }

   AFF_CloseFile(&pGereFic);

   return retour;
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
PROTECTED boolean FreeTabLabel(struct_label *pLabel, int iNbLabel)
{
   struct_line_list *ad_cur_ligne, *ad_suiv_ligne;
   int				position = 0;

   while( position < iNbLabel && pLabel[position].type == MSG_LABEL)
   {
      ad_cur_ligne = pLabel[position].ligne;

      while( ad_cur_ligne != NULL)
      {
         ad_suiv_ligne = ad_cur_ligne->suiv;

         /* Liberation du format texte et de l'‚l‚ment liste ligne */
         MEM_free(ad_cur_ligne->txt_fmt);
         MEM_free(ad_cur_ligne);

         ad_cur_ligne = ad_suiv_ligne;
      }

      pLabel[position].ligne = NULL;

      position++;
   }
   return( TRUE);
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
#pragma warning( push )
#pragma warning( disable : 4047 )
PROTECTED void AFF_CreateText(char *pText, size_t iTextSize, struct_line_list *pLine, aff_infos *pBuffer)

{
	/* ligne avec variable */
	if (pLine->nb_var > 0)
	{
		/* mise en forme de la ligne avec les variables */
		sprintf_s(pText,
			iTextSize,
			pLine->txt_fmt,
			pBuffer->var[pLine->motcle[0]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[0]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[0]]),
			pBuffer->var[pLine->motcle[1]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[1]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[1]]),
			pBuffer->var[pLine->motcle[2]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[2]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[2]]),
			pBuffer->var[pLine->motcle[3]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[3]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[3]]),
			pBuffer->var[pLine->motcle[4]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[4]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[4]]),
			pBuffer->var[pLine->motcle[5]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[5]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[5]]),
			pBuffer->var[pLine->motcle[6]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[6]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[6]]),
			pBuffer->var[pLine->motcle[7]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[7]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[7]]),
			pBuffer->var[pLine->motcle[8]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[8]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[8]]),
			pBuffer->var[pLine->motcle[9]].bString ? AFF_CHAMP_DONNEE_STRING(pBuffer->var[pLine->motcle[9]]) : AFF_CHAMP_DONNEE_LONG(pBuffer->var[pLine->motcle[9]]));
	}
	else
	{
		/* Ligne sans variable */
		sprintf_s(pText, iTextSize, pLine->txt_fmt);
	}
}
#pragma warning( pop ) 
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
PROTECTED void AFF_DataConversion(aff_infos *pDataInfo, struct_aff_demande *pAffichage)
{
	unsigned long				lPrecision;
	struct_hrd_date_entree_hex	cur_date;
	struct_hrd_heure_hex		cur_time;
	
	memset(pDataInfo, 0, sizeof(aff_inf));

	/* Recopie des donn‚es */
	pDataInfo->infos.soft_revision = pAffichage->soft_revision;
	pDataInfo->infos.payment_type = pAffichage->vehicule.payment_type;
	pDataInfo->infos.veh_class = pAffichage->vehicule.veh_class;
	pDataInfo->infos.libelle1 = pAffichage->vehicule.libelle1;
	pDataInfo->infos.libelle2 = pAffichage->vehicule.libelle2;
	pDataInfo->infos.libelle3 = pAffichage->vehicule.libelle3;
	pDataInfo->infos.fare_int = pAffichage->vehicule.fare;
	pDataInfo->infos.fare_dec = pAffichage->vehicule.fare;
	pDataInfo->infos.paid_int = pAffichage->vehicule.paid;
	pDataInfo->infos.paid_dec = pAffichage->vehicule.paid;
	pDataInfo->infos.sale_fare_int = pAffichage->vehicule.sale_fare;
	pDataInfo->infos.sale_fare_dec = pAffichage->vehicule.sale_fare;
	pDataInfo->infos.sale_paid_int = pAffichage->vehicule.sale_paid;
	pDataInfo->infos.sale_paid_dec = pAffichage->vehicule.sale_paid;
	pDataInfo->infos.precision_tarif = pAffichage->precision_tarif;
	pDataInfo->infos.amount_due_int = pAffichage->vehicule.amount_due;
	pDataInfo->infos.amount_due_dec = pAffichage->vehicule.amount_due;
	pDataInfo->infos.overpay_int = pAffichage->vehicule.overpay;
	pDataInfo->infos.overpay_dec = pAffichage->vehicule.overpay;

	lPrecision = AFF_LIRE_DONNEE_AS_LONG(pAffichage->precision_tarif);

	// Qu'il s'agisse de WIN32 ou RTC, le type est déjà donné comme numérique
	// (car provient de pAffichage), on a juste à ajuster les valeurs
	// On peut donc utiliser AFF_CHAMP_DONNEE_LONG
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.fare_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.fare_dec) %= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.paid_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.paid_dec) %= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.sale_fare_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.sale_fare_dec) %= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.sale_paid_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.sale_paid_dec) %= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.amount_due_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.amount_due_dec) %= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.overpay_int) /= lPrecision;
	AFF_CHAMP_DONNEE_LONG(pDataInfo->infos.overpay_dec) %= lPrecision;

	/* Informations fournies par horodate */
	cur_date = HRDDonneDate();
	cur_time = HRDDonneHeure();

	// Qu'il s'agisse de WIN32 ou RTC, le type est n'est pas déjà donné comme numérique
	// (car provient de pAffichage), il faut donc définir les valeurs et le type,
	// on utilise donc AFF_ECRIRE_DONNEE_LONG.
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.year, cur_date.annee);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.year_2_digit, cur_date.annee % 100);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.month, cur_date.mois);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.day, cur_date.jour);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.hour, cur_time.heure);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.minute, cur_time.minute);
	AFF_ECRIRE_DONNEE_LONG(pDataInfo->infos.second, cur_time.seconde);
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
PROTECTED void AFF_CreateLine(unsigned char *pConversionArray, 
							struct_request_screen *pScreen, 
							struct_label *pLabel, 
							aff_infos *pBuffer)
{
	short int			numero_ligne	= 1;
	struct_line_list	*ligne			= pLabel->ligne;
	struct_list_text	*lst_texte;
	unsigned int		n;

	/* M‚morisation du label utilis‚ */
	pScreen->p_label = pLabel;

	while (ligne != NULL)
	{
		/* Recherche du texte dans le tableau de demande */
		lst_texte = AFF_MsgRequestLineNumber(pScreen, numero_ligne);

		/* Construction du texte */
		AFF_CreateText(lst_texte->texte, sizeof(lst_texte->texte), ligne, pBuffer);

		/* Character conversion */
		for (n = 0; n < strlen(lst_texte->texte); n++)
		{
			lst_texte->texte[n] = pConversionArray[lst_texte->texte[n]];
		}

		/* Et m‚morisation des autres donn‚es */
		lst_texte->police = ligne->police;

		if (ligne->suiv != NULL)
			lst_texte->suiv_valide = TRUE;
		else
			lst_texte->suiv_valide = FALSE;

		/* Passage a la ligne suivante */
		numero_ligne++;
		ligne = ligne->suiv;
	}
}
