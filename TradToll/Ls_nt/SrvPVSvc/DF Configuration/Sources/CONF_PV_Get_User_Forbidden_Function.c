/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Forbidden_Function.c							 */
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
#include <srvpv_dbif.h>
#include <ntsvc.h>
#include <conf_srvpv.h>
#include <conf_define.h>
#include <Db_connection.h>
#include <gen.h>
#include <conf_ini.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Forbidden_Function(IN char * name,
																IN DWORD matricule,
																OUT DWORD * func_id,
																OUT DWORD * func_mode)
{
	BOOL		bRet			= FALSE;
	DWORD		num_section			= 0;
	DWORD		sauv_num_section	= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	DWORD		Sortie				= FALSE;
	DWORD		ind_resul			= 0;
	DWORD		id_profile			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*func_id = 0;
	*func_mode = 0;

	if ((name[0] != '\0') && (matricule != 0))
	{
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "NAME");
		strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), name);

		strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "REF_NUMBER");
		_itoa_s(matricule, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_MATRICULE);

		nbcriteres = 2;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		pt_num_section = &num_section;

		num_section = INI_Recherche("T_STAFF.ini", OUT pt_num_section, nbcriteres, TabCritere);
		if (num_section > 0)
		{
			bRet = FALSE;
			nbcriteres = 1;

			/* ID_PROFILE */
			strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_PROFILE");

			bRet = INI_Recupere(IN "T_STAFF.ini", num_section, nbcriteres, TabInfo);
			if (bRet == TRUE)
			{
				id_profile = atoi(TabInfo[0].info_1);

				/* ID_PROFILE from T_PROFILE_USE_MENU */
				strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
				_itoa_s(id_profile, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_PROFILE_ID);

				/* ID_MENU and FLAG_PASSWORD */
				strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_MENU");
				strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "FLAG_PASSWORD");

				while (Sortie == FALSE)
				{
					nbcriteres = 1;
					*pt_num_section = sauv_num_section;

					/* T_TRANSLATE.INI */
					num_section = INI_Recherche("T_PROFILE_USE_MENU.ini", OUT pt_num_section, nbcriteres, TabCritere);
					if (num_section > 0)
					{
						sauv_num_section = num_section;
						bRet = FALSE;
						nbcriteres = 2;

						bRet = INI_Recupere(IN "T_PROFILE_USE_MENU.ini", num_section, nbcriteres, TabInfo);
						if (bRet == TRUE)
						{
							*func_id = atoi(TabInfo[0].info_1);
							*func_mode = atoi(TabInfo[1].info_1);

							if ((*func_mode != 0) || (strlen(TabInfo[1].info_1) == 0))
							{

								strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
								strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfo[1].info_1);

								if (ind_resul < MAX_ENR)
									ind_resul++;
							}
						}
						else
						{
							Resultat[ind_resul].info_0[0] = '\0';
							Resultat[ind_resul].info_1[0] = '\0';
							Resultat[ind_resul].info_2[0] = '\0';
						}
					}
					else
						Sortie = TRUE;
				}
			}
		}

		Taille = ind_resul;

		if (ind_resul > 0)
		{
			*func_id = atoi(Resultat[Index].info_0);
			*func_mode = atoi(Resultat[Index].info_1);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_User_Forbidden_Function() => func. id: %d, func. mode: %d", *func_id, *func_mode);

			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
			*func_id = 0;
			*func_mode = 0;
		}
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Forbidden_Function(OUT DWORD *func_id, OUT DWORD *func_mode)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*func_id = atoi(Resultat[Index].info_0);
		*func_mode = atoi(Resultat[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_User_Forbidden_Function() => func. id: %d, func. mode: %d", *func_id, *func_mode);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/