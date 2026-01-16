/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Command_Lane_Type.c							 */
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

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Command_Lane_Suite(DWORD id_command, DWORD *ind_resul);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Command_Lane_Type(IN CHAR name[MSG_PV_MAX_USER_NAME],
															   IN DWORD matricule,
															   IN DWORD cmd_id,
															   OUT DWORD *type_id)
{
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		sauv_num_section	= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	DWORD		Sortie				= FALSE;
	DWORD		ind_resul			= 0;
	DWORD		profile_id			= 0;
	DWORD		id_command			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*type_id = 0;

	if ((name[0] != '\0') && (matricule != 0) && (cmd_id != 0))
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
				profile_id = atoi(TabInfo[0].info_1);

				/* ID_PROFILE from T_PROFILE_USE_COMMAND */
				strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
				_itoa_s(profile_id, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_PROFILE_ID);

				/* ID_COMMAND  */
				strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_COMMAND");

				while (Sortie == FALSE)
				{
					nbcriteres = 1;
					*pt_num_section = sauv_num_section;

					/* T_PROFILE_USE_COMMAND.INI */
					num_section = INI_Recherche("T_PROFILE_USE_COMMAND.ini", OUT pt_num_section, nbcriteres, TabCritere);
					if (num_section > 0)
					{
						bRet = FALSE;
						sauv_num_section = num_section;
						nbcriteres = 1;

						bRet = INI_Recupere(IN "T_PROFILE_USE_COMMAND.ini", num_section, nbcriteres, TabInfo);
						if (bRet == TRUE)
						{
							/* compare ID_COMMAND against cmd_id */
							id_command = atoi(TabInfo[0].info_1);
							if (cmd_id == id_command)
							{
								bRet = CONF_PV_Get_First_User_Command_Lane_Suite(id_command, &ind_resul);
								Sortie = TRUE;

							}
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
			*type_id = atoi(Resultat[Index].info_0);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_User_Command_Lane_Type() => typeid: %d", *type_id);

			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
			*type_id = 0;
		}
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Command_Lane_Type(OUT DWORD * type_id)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*type_id = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_User_Command_Lane_Type() => typeid: %d ", *type_id);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Command_Lane_Suite(DWORD id_command, DWORD *ind_resul)
{
	BOOL		bRet				= FALSE;
	DWORD		Sortie				= FALSE;
	DWORD		*pt_num_section		= NULL;
	DWORD		sauv_num_section	= 0;
	DWORD		num_section			= 0;
	DWORD		nbcriteres			= 0;
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	DWORD		indice_resul		= 0;

	pt_num_section = &num_section;

	indice_resul = *ind_resul;
	*pt_num_section = 0;

	while (Sortie == FALSE)
	{
		/* D_COMMAND from T_CONCERN */
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_COMMAND");
		_itoa_s(id_command, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_COMMAND);

		nbcriteres = 1;
		*pt_num_section = sauv_num_section;

		/* T_CONCERN.INI */
		num_section = INI_Recherche("T_CONCERN.ini", OUT pt_num_section, nbcriteres, TabCritere);

		if (num_section > 0)
		{
			sauv_num_section = num_section;
			bRet = FALSE;
			nbcriteres = 1;

			/* ID_LANE_TYPE */
			strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_LANE_TYPE");

			bRet = INI_Recupere(IN "T_CONCERN.ini", num_section, nbcriteres, TabInfo);
			if (bRet == TRUE)
			{
				strcpy_s(Resultat[indice_resul].info_0, sizeof(Resultat[indice_resul].info_0), TabInfo[0].info_1);

				if (indice_resul < MAX_ENR)
					indice_resul++;
			}
		}
		else
			Sortie = TRUE;
	}

	*ind_resul = indice_resul;

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/