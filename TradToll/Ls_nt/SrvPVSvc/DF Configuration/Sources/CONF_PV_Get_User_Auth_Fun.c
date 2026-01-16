/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Auth_Fun.c									 */
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
#include <Msg_Pv_Define.h>
#include <gen.h>
#include <conf_ini.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE WINAPI CONF_PV_Get_User_A_F_suite(DWORD *profile_id, DWORD *func_id, DWORD *id_menu);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_User_Auth_Fun(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
												IN DWORD matricule_in,
												IN CHAR *password,
												IN DWORD dwPasswordSize,
												IN DWORD func_id,
												OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
												OUT DWORD *matricule_out,
												OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
												OUT DWORD *user_ok,
												OUT DWORD *profile_id,
												OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME])
{

	DWORD		ind					= 0;
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	DWORD		id_menu				= 0;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	// par défaut...
	name_out[0] = '\0';
	*matricule_out = 0;
	firstname[0] = '\0';
	*user_ok = 0;
	*profile_id = 0;
	profile_name[0] = '\0';

	pt_num_section = &num_section;

	if (name_in[0] != '\0')
		nbcriteres++;

	if (matricule_in != 0)
		nbcriteres++;

	if (password != NULL && password[0] != '\0' && dwPasswordSize != 0)
		nbcriteres++;

	if (func_id != 0)
		nbcriteres++;
	else
		nbcriteres = 0;

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* name_in, matricule_in, password, func_id = Id_profile */
		ind = 0;

		if (name_in[0] != '\0')
		{
			strcpy_s(TabCritere[ind].info_0, sizeof(TabCritere[ind].info_0), "NAME");
			strcpy_s(TabCritere[ind++].info_1, sizeof(TabCritere[ind++].info_1), name_in);
		}

		if (matricule_in != 0)
		{
			strcpy_s(TabCritere[ind].info_0, sizeof(TabCritere[ind].info_0), "REF_NUMBER");
			_itoa_s(matricule_in, TabCritere[ind++].info_1, sizeof(TabCritere[ind++].info_1), MAX_MATRICULE);
		}

		if (password != NULL && password[0] != '\0' && dwPasswordSize != 0)
		{
			strcpy_s(TabCritere[ind].info_0, sizeof(TabCritere[ind].info_0), "PASSWORD");
			strcpy_s(TabCritere[ind++].info_1, sizeof(TabCritere[ind++].info_1), password);
		}

		nbcriteres = ind;
		*pt_num_section = 0;

		num_section = INI_Recherche("T_STAFF.ini", OUT pt_num_section, nbcriteres, TabCritere);
		if (num_section > 0)
		{

			nbcriteres = 4;
			/* name-out, matricule_out, firstname and profile_id */
			strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "NAME");
			strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "REF_NUMBER");
			strcpy_s(TabInfo[2].info_0, sizeof(TabInfo[2].info_0), "FIRST_NAME");
			strcpy_s(TabInfo[3].info_0, sizeof(TabInfo[3].info_0), "ID_PROFILE");

			bRet = INI_Recupere(IN "T_STAFF.ini", num_section, nbcriteres, TabInfo);
			if (bRet == TRUE)
			{
				/* name-out, matricule_out, firstname and profile_id */
				strcpy_s(name_out, MSG_PV_MAX_USER_NAME, TabInfo[0].info_1);
				*matricule_out = atoi(TabInfo[1].info_1);

				strcpy_s(firstname, MSG_PV_MAX_USER_FIRSTNAME, TabInfo[2].info_1);
				*profile_id = atoi(TabInfo[3].info_1);

				/* t_staff.id_profile->t_profile.id_profile=cde_label_profile */
				/* t_translate.cde_label = short_label_1 */
				nbcriteres = 1;

				/* id_profile */
				strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
				strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[3].info_1);
				*pt_num_section = 0;

				num_section = INI_Recherche("T_PROFILE.ini", OUT pt_num_section, nbcriteres, TabCritere);
				if (num_section > 0)
				{
					nbcriteres = 1;
					/* cde_label_profile */
					strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "CDE_LABEL_PROFILE");

					bRet = INI_Recupere(IN "T_PROFILE.ini", num_section, nbcriteres, TabInfo);
					if (bRet == TRUE)
					{
						strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "CDE_LABEL");
						strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[0].info_1);
						*pt_num_section = 0;

						num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritere);
						if (num_section > 0)
						{
							nbcriteres = 1;
							strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "SHORT_LABEL_1");

							bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfo);
							if (bRet == TRUE)
							{
								strcpy_s(profile_name, MSG_PV_MAX_USER_PROFILE_NAME, TabInfo[0].info_1);
								bRet = CONF_PV_Get_User_A_F_suite(profile_id, &func_id, &id_menu);
							}
						}
					}
				}
			}

			if (bRet)
				*user_ok = 1;
		}

		NTSVCInfo("CONF_PV_Get_User_Auth_Fun(),  name = %s, matricule = %d, firstname = %s, userok = %d",
				name_out,
				*matricule_out,
				firstname,
				*user_ok);

		bRet = TRUE;
	}

	return bRet;
}

PROTECTED WINAPI CONF_PV_Get_User_A_F_suite(DWORD *profile_id, DWORD *func_id, DWORD *id_menu)
{
	DWORD		Sortie				= FALSE;
	BOOL		bRet				= FALSE;
	DWORD		*pt_num_section		= NULL;
	DWORD		sauv_num_section	= 0;
	DWORD		num_section			= 0;
	DWORD		nbcriteres			= 0;
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };

	pt_num_section = &num_section;

	/* func_id=id_menu = t_profile_use_menu.id_profile->t_staff.id_profile*/
	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
	_itoa_s(*profile_id, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_PROFILE_ID);

	strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_MENU");
	_itoa_s(*func_id, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ID_MENU);
	nbcriteres = 2;

	*pt_num_section = 0;

	num_section = INI_Recherche("T_PROFILE_USE_MENU.ini", OUT pt_num_section, nbcriteres, TabCritere);
	if (num_section > 0)
	{
		nbcriteres = 1;

		/*  cde_label_profile */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "FLAG_PASSWORD");

		bRet = INI_Recupere(IN "T_PROFILE_USE_MENU.ini", num_section, nbcriteres, TabInfo);
		if (bRet == TRUE)
		{
			if (TabInfo[0].info_1[0] == '\0')
				bRet = FALSE;
		}
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/