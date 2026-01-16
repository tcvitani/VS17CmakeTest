/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_INI.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <gen.h>
#include <ntsvc.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define INI_REG_VAL_PATH           "INI_Files_Path"

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE DWORD INI_rech_section(char *f_header, char *list);

PRIVATE DWORD INI_rech_clef(char *list, 
							DWORD nb,
							DWORD *pt_indice_critere,
							char *f_phase_rech_section,
							struct tab TabTravail[]);

PRIVATE DWORD INI_section(DWORD num_section, char *list);

PRIVATE void INI_remplissage_tableau(DWORD ind_critere, 
									char *list,
									DWORD taille, 
									DWORD *pt_nb_criteres_trouves,
									IN DWORD *pt_indice_critere,
									struct tab TabTravail[]);

PRIVATE DWORD INI_recup_num_section(DWORD *pt_num_section, char *list);

PRIVATE DWORD INI_comparaison(DWORD ind_critere, 
							char *list,
							DWORD taille, 
							IN DWORD *pt_indice_critere,
							DWORD *pt_nb_criteres_trouves,
							struct tab TabInfo[]);

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct
{
	IN char szINIFilesPath[_MAX_PATH];
}
INI = { 0 };

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL INI_Initialize(void)
{
	DWORD dwErr = 0;
	NTSVC_PARAMETER_DEF * psParams;

	// Définition des paramètres
	psParams = NTSVCOpenParameters(INI_REG_VAL_PATH, REG_SZ, _MAX_PATH, "c:\\csr\\exe\\ini", &INI.szINIFilesPath, NULL);
	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	return TRUE;
}

PROTECTED char * INI_Get_Files_Path(void)
{
	return INI.szINIFilesPath;
}

PROTECTED VOID INI_Resultat_ZeroMemory(IN OUT struct resul Resultat[], IN DWORD Taille)
{
	DWORD i = 0;

	for (i = 0; i < Taille; i++)
	{
		ZeroMemory(Resultat[i].info_0, sizeof(Resultat[i].info_0));
		ZeroMemory(Resultat[i].info_1, sizeof(Resultat[i].info_1));
		ZeroMemory(Resultat[i].info_2, sizeof(Resultat[i].info_2));
		ZeroMemory(Resultat[i].info_3, sizeof(Resultat[i].info_3));
		ZeroMemory(Resultat[i].info_4, sizeof(Resultat[i].info_4));
	}
}

PROTECTED BOOL INI_Recupere(IN char *conf_fic, 
							IN DWORD num_section, 
							IN DWORD nb_critere, 
							IN OUT struct tab TabInfo[])
{
	FILE		*stream					= NULL;
	DWORD		i						= 0;
	BOOL		bRet					= FALSE;
	DWORD		ret						= FALSE;
	DWORD		taille					= 0;
	DWORD		indice_critere			= 0;
	DWORD		*pt_indice_critere		= NULL;
	DWORD		nb_criteres_trouves		= 0;
	DWORD		*pt_nb_criteres_trouves	= NULL;
	char		f_header				= FALSE;
	char		f_phase_rech_section	= TRUE;
	char		list[MAX_CAR]			= { 0 };
	char		fichier[_MAX_PATH + 50] = { 0 };
	st_TabInfo	TabTravail[MAX_ENR]		= { 0 };
	errno_t		err;

	sprintf_s(fichier, sizeof(fichier), "%s\\%s", INI_Get_Files_Path(), conf_fic);

	pt_nb_criteres_trouves = &nb_criteres_trouves;
	pt_indice_critere = &indice_critere;

	for (i = 0; i < MAX_ENR; i++)
	{
		TabTravail[i].info_0[0] = '\0';
		TabTravail[i].info_1[0] = '\0';
	}

	for (i = 0; i < nb_critere; i++)
	{
		strcpy_s(TabTravail[i].info_0, MAX_INFO, TabInfo[i].info_0);
		TabTravail[i].info_1[0] = '\0';
	}

	if ((err = fopen_s(&stream, fichier, "rt")) == 0)
	{
		while (ret == FALSE)
		{
			memset(list, '\0', sizeof(list));
			if (fgets(list, MAX_CAR, stream) != NULL)
			{
				if (f_phase_rech_section == TRUE)
				{
					ret = INI_rech_section(&f_header, list);

					if (ret == TRUE)
					{
						ret = INI_section(num_section, list);
						if (ret == TRUE)
							f_phase_rech_section = FALSE;

						ret = FALSE;
					}
				}
				else
				{
					taille = INI_rech_clef(list, nb_critere, pt_indice_critere, &f_phase_rech_section, TabTravail);
					if (taille > 0)
					{
						INI_remplissage_tableau(nb_critere, list, taille, pt_indice_critere, pt_nb_criteres_trouves, TabTravail);
						if (*pt_nb_criteres_trouves == nb_critere)
							ret = TRUE;
						else
							ret = FALSE;
					}
					else
					{
						if (f_phase_rech_section == TRUE)
							ret = TRUE;
					}
				}
			}
			else
				ret = TRUE;
		}

		fclose(stream);

		if (nb_criteres_trouves == nb_critere)
			bRet = TRUE;

		for (i = 0; i < MAX_ENR; i++)
		{
			strcpy_s(TabInfo[i].info_0, MAX_INFO, TabTravail[i].info_0);
			strcpy_s(TabInfo[i].info_1, MAX_INFO, TabTravail[i].info_1);
		}
	}

	return bRet;
}

DWORD INI_rech_section(char *f_header, char *list)
{
	DWORD	f_sortie	= 0;
	DWORD	retour		= 0;
	DWORD	sortie		= 0;
	DWORD	ret			= 0;
	DWORD	i			= 0;
	DWORD	crochet_ouv = 0;

	while ((i < MAX_CAR) && (list[i] != '\0') && (sortie == FALSE))
	{
		if (list[i] == '[')
		{
			if (*f_header == FALSE)
			{
				sortie = TRUE;
				*f_header = TRUE;
			}
			else
				crochet_ouv = TRUE;
		}
		else if ((list[i] == ']') && (crochet_ouv == TRUE))
		{
			sortie = TRUE;
			retour = TRUE;
		}

		i++;
	}

	return (retour);
}

DWORD INI_section(DWORD num_section, char *list)
{
	DWORD	ret			= 0;
	DWORD	num			= 0;
	char	numero[4]	= { 0 };
	DWORD	y			= 0;
	DWORD	i			= 0;

	while (list[i] != ']')
	{
		if (list[i] != '[')
			numero[y++] = list[i];
		i++;
	}

	numero[y] = '\0';
	num = atoi(numero);

	if (num_section == num)
		ret = TRUE;
	else
		ret = FALSE;

	return(ret);
}

DWORD INI_rech_clef(char *list, 
					DWORD nb_critere,
					DWORD *pt_indice_critere,
					char *f_phase_rech_section,
					struct tab TabTravail[])
{
	DWORD resul = FALSE;
	short i;
	DWORD retour = FALSE;
	DWORD ind_critere = 0;
	DWORD taille = 0;
	DWORD taille_list = 0;

	if (list[0] == '[')
	{
		retour = FALSE;
		*f_phase_rech_section = TRUE;
	}
	else
	{
		i = 0;

		while ((list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D) && (list[i] != 0x20) && (list[i] != 0x3d))
		{
			taille_list++;

			i++;
		}

		while ((ind_critere < nb_critere) && (resul == FALSE))
		{
			taille = 0;
			i = 0;

			while (TabTravail[ind_critere].info_0[i] != 0x00)
			{
				taille++;
				i++;
			}

			if (taille_list == taille)
			{
				resul = strncmp(TabTravail[ind_critere].info_0, list, taille);
				if (resul == 0)
				{
					resul = TRUE;
					*pt_indice_critere = ind_critere;

				}
				else
					resul = FALSE;
			}

			ind_critere++;
		}
	}

	if (resul == TRUE)
		retour = taille;

	return(retour);
}

void INI_remplissage_tableau(DWORD ind_critere, 
							char *list,
							DWORD taille, 
							DWORD *pt_indice_critere,
							DWORD *pt_nb_criteres_trouves,
							struct tab TabTravail[])
{
	DWORD i = taille;
	DWORD y = 0;
	DWORD v = 0;
	char sauv[50];

	while ((list[i] == ' ') && (list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D)) 
		i++;

	while ((list[i] == '=') && (list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D)) 
		i++;

	while ((list[i] == ' ') && (list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D))
		i++;

	while ((list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D))
	{
		TabTravail[*pt_indice_critere].info_1[y++] = list[i];
		sauv[v++] = list[i];
		i++;
	}

	TabTravail[*pt_indice_critere].info_1[y] = '\0';
	*pt_nb_criteres_trouves = *pt_nb_criteres_trouves + 1;
}

PROTECTED DWORD INI_Recherche(IN char *conf_fic, 
							OUT DWORD *pt_num_section,
							IN DWORD nb_critere, 
							IN OUT struct tab TabCritere[])
{
	DWORD i;
	FILE *stream;
	struct tab TabTravail[MAX_ENR];

	BOOL bRet = FALSE;
	DWORD section_trouvee;
	DWORD ret = FALSE;
	char list[MAX_CAR];
	char fichier[_MAX_PATH + 50];
	DWORD taille;
	DWORD indice_critere = 0;
	DWORD nb_criteres_trouves = 0;
	DWORD *pt_indice_critere;
	DWORD *pt_nb_criteres_trouves;
	char f_header = FALSE;
	char f_phase_rech_section = TRUE;
	errno_t err;

	sprintf_s(fichier, sizeof(fichier), "%s\\%s", INI_Get_Files_Path(), conf_fic);

	pt_indice_critere = &indice_critere;
	pt_nb_criteres_trouves = &nb_criteres_trouves;

	ret = FALSE;

	for (i = 0; i < nb_critere; i++)
	{
		strcpy_s(TabTravail[i].info_0, MAX_INFO, TabCritere[i].info_0);
		strcpy_s(TabTravail[i].info_1, MAX_INFO, TabCritere[i].info_1);
	}

	if ((err = fopen_s(&stream, fichier, "rt")) == 0)
	{
		while (ret == FALSE)
		{
			memset(list, '\0', sizeof(list));
			if (fgets(list, MAX_CAR, stream) != NULL)
			{
				if (f_phase_rech_section == TRUE)
				{
					*pt_nb_criteres_trouves = 0;

					ret = INI_rech_section(&f_header, list);
					if (ret == TRUE)
					{
						ret = INI_recup_num_section(pt_num_section, list);

						if (ret == TRUE)
							f_phase_rech_section = FALSE;

						ret = FALSE;
					}
				}
				else
				{
					taille = INI_rech_clef(list, nb_critere, pt_indice_critere, &f_phase_rech_section, TabTravail);
					if (taille > 0)
					{
						ret = INI_comparaison(nb_critere, list, taille, pt_indice_critere, pt_nb_criteres_trouves, TabTravail);
						if (ret == TRUE)
						{
							if (*pt_nb_criteres_trouves == nb_critere)
								section_trouvee = *pt_num_section;
							else
								ret = FALSE;
						}
						else
						{
							f_phase_rech_section = TRUE;
							ret = FALSE;
						}
					}
					else
					{
						if (f_phase_rech_section == TRUE)
							ret = TRUE;
					}
				}
			}
			else
				ret = TRUE;
		}

		fclose(stream);

		if ((*pt_num_section != 0) && (*pt_nb_criteres_trouves == nb_critere))
			bRet = *pt_num_section;
	}

	return (bRet);
}

DWORD INI_recup_num_section(DWORD *pt_num_section, char *list)
{
	BOOL bRet = FALSE;
	char numero[4];
	DWORD y = 0;
	DWORD i = 0;

	while (list[i] != ']')
	{
		if (list[i] != '[')
			numero[y++] = list[i];
		i++;
	}

	numero[y] = '\0';

	if (*pt_num_section > 0)
	{
		if (*pt_num_section < (DWORD)atoi(numero))
		{
			*pt_num_section = atoi(numero);
			bRet = TRUE;
		}
	}
	else
	{
		*pt_num_section = atoi(numero);
		bRet = TRUE;
	}

	return (bRet);
}

DWORD INI_comparaison(DWORD ind_critere, 
					char *list,
					DWORD taille, 
					DWORD *pt_indice_critere,
					DWORD *pt_nb_criteres_trouves,
					struct tab TabTravail[])
{
	DWORD i = taille;
	DWORD retour = FALSE;
	DWORD ret = FALSE;
	char sauv[MAX_CAR];
	DWORD y = 0;

	while ((list[i] != 0X0A) && (list[i] != 0X00) && (list[i] != 0X0D) && (i < MAX_CAR))
	{
		if ((list[i] != ' ') && (list[i] != '='))
			sauv[y++] = list[i];

		i++;
	}

	sauv[y] = '\0';

	ret = strcmp(sauv, TabTravail[*pt_indice_critere].info_1);
	if (ret == 0)
	{
		retour = TRUE;
		*pt_nb_criteres_trouves = *pt_nb_criteres_trouves + 1;
	}

	return (retour);
}

/*-------------------------------- END OF FILE ------------------------------*/