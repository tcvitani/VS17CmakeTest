/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Login.c										 */
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


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_User_Login(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
											 IN DWORD matricule_in,
											 IN CHAR *password,
											 IN DWORD dwPasswordSize,
											 OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
											 OUT DWORD *matricule_out,
											 OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
											 OUT DWORD *user_ok,
											 OUT DWORD *profile_max_connections,
											 OUT DWORD *profile_id,
											 OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME])
{
	DWORD		ind					= 0;
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]	= { 0 };
	SYSTEMTIME	sNow				= { 0 };
	CHAR		szNow[MAX_PATH]		= { 0 };
	LONG		lNow				= 0;
	LONG		lToCompare			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	name_out[0] = '\0';
	*matricule_out = 0;
	firstname[0] = '\0';
	*user_ok = 0;
	*profile_max_connections = 0;
	*profile_id = 0;
	profile_name[0] = '\0';

	if (name_in[0] != '\0')
		nbcriteres++;

	if (matricule_in != 0)
		nbcriteres++;

	if (password != NULL && password[0] != '\0' && dwPasswordSize != 0)
		nbcriteres++;

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* name_in, matricule_in, password */
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
		pt_num_section = &num_section;

		num_section = INI_Recherche("T_STAFF.ini", OUT pt_num_section, nbcriteres, TabCritere);
		if (num_section > 0)
		{
			*user_ok = 1;
			nbcriteres = 5;

			/* name-out, matricule_out, firstname, profile_id */
			strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "NAME");
			strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "REF_NUMBER");
			strcpy_s(TabInfo[2].info_0, sizeof(TabInfo[2].info_0), "FIRST_NAME");
			strcpy_s(TabInfo[3].info_0, sizeof(TabInfo[3].info_0), "ID_PROFILE");
			strcpy_s(TabInfo[4].info_0, sizeof(TabInfo[4].info_0), "DTE_PWD_EXPIRE");

			bRet = INI_Recupere(IN "T_STAFF.ini", num_section, nbcriteres, TabInfo);
			if (bRet)
			{
				// check if account is expired
				GetLocalTime(&sNow);
				sprintf_s(szNow, sizeof(szNow), "%04d%02d%02d", sNow.wYear, sNow.wMonth, sNow.wDay);
				lNow = atol(szNow);
				lToCompare = atol(TabInfo[4].info_1);
				
				if (lToCompare < lNow) // compares two dates
					*user_ok = 2;

				/* name-out */
				strcpy_s(name_out, MSG_PV_MAX_USER_NAME, TabInfo[0].info_1);
				*matricule_out = atoi(TabInfo[1].info_1);

				/* firstname */
				strcpy_s(firstname, MSG_PV_MAX_USER_FIRSTNAME, TabInfo[2].info_1);
				*profile_id = atoi(TabInfo[3].info_1);

				/* ID_PROFILE */
				strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
				strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[3].info_1);

				nbcriteres = 1;
				*pt_num_section = 0;

				/* T_SESSION_PROFILE_STATION.INI */
				num_section = INI_Recherche("T_SESSION_PROFILE_STATION.ini", OUT pt_num_section, nbcriteres, TabCritere);
				if (num_section > 0)
				{
					nbcriteres = 1;
					strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "NB_SESSION");

					bRet = INI_Recupere(IN "T_SESSION_PROFILE_STATION.ini", num_section, nbcriteres, TabInfo);
					if (bRet == TRUE)
					{
						*profile_max_connections = atoi(TabInfo[0].info_1);

						/* Profile_name = t_staff.id_profile */
						/* t_profile.id_profile = cde_label_profile */
						/* t_translate.cde_label = short_label_1 */
						/* ID_PROFILE */
						_itoa_s(*profile_id, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_PROFILE_ID);
						nbcriteres = 1;
						*pt_num_section = 0;

						/* T_PROFILE.INI */
						num_section = INI_Recherche("T_PROFILE.ini", OUT pt_num_section, nbcriteres, TabCritere);
						if (num_section > 0)
						{
							nbcriteres = 1;
							/* CDE_LABEL_PROFILE */
							strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "CDE_LABEL_PROFILE");

							bRet = INI_Recupere(IN "T_PROFILE.ini", num_section, nbcriteres, TabInfo);
							if (bRet)
							{
								strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "CDE_LABEL");
								strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[0].info_1);

								/* T_TRANSLATE.INI */
								*pt_num_section = 0;
								num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritere);
								if (num_section > 0)
								{
									/* NB_SESSION */
									strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "SHORT_LABEL_1");

									bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfo);
									if (bRet)
									{
										/* profile_name */
										strcpy_s(profile_name, MSG_PV_MAX_USER_PROFILE_NAME, TabInfo[0].info_1);
										NTSVCInfo("CONF_PV_Get_User_Login() => Name: %s, Password: %d, Firstname: %s, Profile: %s, Userok: %d, Usermax: %d",
												name_out,
												*matricule_out,
												firstname,
												profile_name,
												*user_ok,
												*profile_max_connections);
									}
								}
							}
						}
					}
				}
			}
		}

		NTSVCInfo("CONF_PV_Get_User_Login() => Name: %s, Password: %d, Firstname: %s, Profile: %s, Userok: %d, Usermax: %d",
				name_out,
				*matricule_out,
				firstname,
				profile_name,
				*user_ok,
				*profile_max_connections);

		bRet = TRUE;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/