/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_User_Allowed_Command.c								 */
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

PRIVATE BOOL WINAPI CONF_PV_Get_First_User_Allowed_Suite(DWORD id_command, DWORD ind_resul);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Allowed_Command(IN CHAR name[MSG_PV_MAX_USER_NAME],
															 IN DWORD matricule,
															 OUT DWORD *cmd_id,
															 OUT DWORD *cmd_mode,
															 OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL])
{
	BOOL		bRet						= FALSE;
	DWORD		num_section					= 0;
	DWORD		sauv_num_section			= 0;
	DWORD		nbcriteres					= 0;
	DWORD		*pt_num_section				= NULL;
	st_TabInfo	TabInfo[MAX_ENR]			= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]			= { 0 };
	st_TabInfo	TabCritereSuite[MAX_ENR]	= { 0 };
	DWORD		Sortie						= FALSE;
	DWORD		ind_resul					= 0;
	DWORD		id_command					= 0;
	DWORD		id_profile					= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*cmd_id = 0;
	*cmd_mode = 0;
	cmd_label[0] = '\0';

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

		/* NAME, REF_NUMBER from T_STAFF */
		nbcriteres = 2;

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

				/* ID_PROFILE from T_PROFILE_USE_COMMAND */
				strcpy_s(TabCritereSuite[0].info_0, sizeof(TabCritereSuite[0].info_0), "ID_PROFILE");
				_itoa_s(id_profile, TabCritereSuite[0].info_1, sizeof(TabCritereSuite[0].info_1), MAX_PROFILE_ID);

				/* ID_COMMAND, FLAG_PASSWORD */
				strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_COMMAND");
				strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "FLAG_PASSWORD");

				while (Sortie == FALSE)
				{
					nbcriteres = 1;
					*pt_num_section = sauv_num_section;

					/* T_PROFILE_USE_COMMAND.INI */
					num_section = INI_Recherche("T_PROFILE_USE_COMMAND.ini", OUT pt_num_section, nbcriteres, TabCritereSuite);
					if (num_section > 0)
					{
						bRet = FALSE;
						sauv_num_section = num_section;
						nbcriteres = 2;

						bRet = INI_Recupere(IN "T_PROFILE_USE_COMMAND.ini", num_section, nbcriteres, TabInfo);
						if (bRet == TRUE)
						{
							if (TabInfo[1].info_1[0] == '\0')
								bRet = FALSE;
							else
							{
								strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
								strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfo[1].info_1);
								id_command = atoi(TabInfo[0].info_1);

								bRet = CONF_PV_Get_First_User_Allowed_Suite(id_command, ind_resul);
								if (bRet == FALSE)
								{
									Resultat[ind_resul].info_0[0] = '\0';
									Resultat[ind_resul].info_1[0] = '\0';
									Resultat[ind_resul].info_2[0] = '\0';
								}
								else
								{
									if (ind_resul < MAX_ENR)
										ind_resul++;
								}
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

			*cmd_id = atoi(Resultat[Index].info_0);
			*cmd_mode = atoi(Resultat[Index].info_1);
			strcpy_s(cmd_label, MSG_PV_MAX_COMMAND_LABEL, Resultat[Index].info_2);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_User_Allowed_Command(), cmdid = %d cmdmode = %d cmdlabel = %s", *cmd_id, *cmd_mode, cmd_label);

			bRet = TRUE;
		}
		else
		{
			*cmd_id = 0;
			*cmd_mode = 0;
			cmd_label[0] = '\0';
			bRet = FALSE;
		}
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_User_Allowed_Command(OUT DWORD *cmd_id,
															OUT DWORD *cmd_mode,
															OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL])
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*cmd_id = atoi(Resultat[Index].info_0);
		*cmd_mode = atoi(Resultat[Index].info_1);
		strcpy_s(cmd_label, MSG_PV_MAX_COMMAND_LABEL, Resultat[Index].info_2);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_User_Allowed_Command(), cmdid = %d cmdmode = %d cmdlabel = %s", *cmd_id, *cmd_mode, cmd_label);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_First_User_Allowed_Suite(DWORD id_command, DWORD ind_resul)
{
	BOOL		bRet				= FALSE;
	DWORD		*pt_num_section		= NULL;
	DWORD		sauv_num_section	= 0;
	DWORD		num_section			= 0;
	DWORD		nbcriteres			= 0;
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };

	/* T_TRANSLATE.CDE_LABEL = LONG_LABEL */
	/* T_COMMAND.ID_COMMAND = T_PROFILE_USE_COMMAND.ID_PROFILE */
	/* ID_COMMAND from T_COMMAND */
	pt_num_section = &num_section;

	bRet = FALSE;

	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_COMMAND");
	_itoa_s(id_command, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_COMMAND);
	
	nbcriteres = 1;
	*pt_num_section = 0;

	/*  T_COMMAND.INI */
	num_section = INI_Recherche("T_COMMAND.ini", OUT pt_num_section, nbcriteres, TabCritere);
	if (num_section > 0)
	{
		bRet = FALSE;
		nbcriteres = 1;

		/* CDE_LABEL_COMMAND */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "CDE_LABEL_COMMAND");

		bRet = INI_Recupere(IN "T_COMMAND.ini", num_section, nbcriteres, TabInfo);
		if (bRet == TRUE)
		{

			/* T_TRANSLATE.INI => CDE_LABEL*/
			/* CDE_LABEL_RTO_COMMAND */
			*pt_num_section = 0;
			nbcriteres = 1;

			strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "CDE_LABEL");
			strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[0].info_1);

			num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				bRet = FALSE;
				nbcriteres = 1;

				/* LONG_LABEL */
				strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "LONG_LABEL");

				bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
					strcpy_s(Resultat[ind_resul].info_2, sizeof(Resultat[ind_resul].info_2), TabInfo[0].info_1);
			}
		}
	}
	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/