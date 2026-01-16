/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Auth_Cmd.c									 */
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

PRIVATE BOOL CONF_PV_Get_User_A_C_suite(DWORD *profile_id, DWORD *cmd_id, DWORD *id_command);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_User_Auth_Cmd(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
												IN DWORD matricule_in,
												IN CHAR *password,
												IN DWORD dwPasswordSize,
												IN DWORD cmd_id,
												OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
												OUT DWORD *matricule_out,
												OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
												OUT DWORD *user_ok,
												OUT DWORD *profile_id,
												OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME])
{
	BOOL	bRet						= FALSE;
	DWORD	num_section					= 0;
	DWORD	nbcriteres					= 0;
	DWORD	*pt_num_section				= NULL;
	DWORD	prof_max_connections		= 0;
	DWORD	*profile_max_connections	= NULL;
	DWORD	id_command					= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	name_out[0] = '\0';
	matricule_out[0] = '\0';
	firstname[0] = '\0';
	*user_ok = 0;
	*profile_id = 0;
	profile_name[0] = '\0';

	if (cmd_id != 0)
	{
		prof_max_connections = 0;
		profile_max_connections = &prof_max_connections;
		pt_num_section = &num_section;

		bRet = CONF_PV_Get_User_Login(IN name_in,
									IN matricule_in,
									IN password,
									IN dwPasswordSize,
									OUT name_out,
									OUT matricule_out,
									OUT firstname,
									OUT user_ok,
									OUT profile_max_connections,
									OUT profile_id,
									OUT profile_name);

		if (bRet)
		{
			*user_ok = 0;

			bRet = CONF_PV_Get_User_A_C_suite(profile_id, &cmd_id, &id_command);
			if (bRet)
				*user_ok = 1;
		}

		NTSVCInfo("CONF_PV_Get_User_Auth_Cmd(),  name = %s, matricule = %d, firstname = %s, userok = %d",
				name_out,
				*matricule_out,
				firstname,
				*user_ok);

		bRet = TRUE;
	}

	return bRet;
}

PROTECTED BOOL CONF_PV_Get_User_A_C_suite(DWORD *profile_id, DWORD *cmd_id, DWORD *id_command)
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

	/* func_id = id_menu = t_profile_use_menu.id_profile->t_staff.id_profile */
	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PROFILE");
	_itoa_s(*profile_id, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_PROFILE_ID);

	strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_COMMAND");
	_itoa_s(*cmd_id, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ID_COMMAND);

	nbcriteres = 2;

	*pt_num_section = 0;

	num_section = INI_Recherche("T_PROFILE_USE_COMMAND.ini", OUT pt_num_section, nbcriteres, TabCritere);
	if (num_section > 0)
	{
		/* FLAG_PASSWORD from T_PROFILE_USE_COMMAND */
		nbcriteres = 1;

		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "FLAG_PASSWORD");

		bRet = INI_Recupere(IN "T_PROFILE_USE_COMMAND.ini", num_section, nbcriteres, TabInfo);
		if (bRet == TRUE)
		{
			if (TabInfo[0].info_1[0] == '\0')
				bRet = FALSE;
		}
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/