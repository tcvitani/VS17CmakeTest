/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Display.c											 */
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

PRIVATE DWORD CONF_PV_Get_Display_Suite(CHAR *Tab0);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Display(IN DWORD plazanum, OUT DWORD *disp_id)
{
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		sauv_num_section	= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]	= { 0 };
	DWORD		Sortie				= FALSE;
	DWORD		ind_resul			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	if (plazanum != 0)
	{
		/* ID_PLAZA, ID_LANE from T_LANE_ATTACH_TO */
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PLAZA");
		_itoa_s(plazanum, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_PLAZA_NUM);
		
		nbcriteres = 1;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* ID_LANE_GROUP */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_LANE_GROUP");

		/* T_PROFILE.INI */
		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			*pt_num_section = sauv_num_section;

			nbcriteres = 1;

			num_section = INI_Recherche("T_LANE_ATTACH_TO.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_LANE_ATTACH_TO.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
					ind_resul = CONF_PV_Get_Display_Suite(TabInfo[0].info_1);
				else
					Resultat[ind_resul].info_0[0] = '\0';
			}
			else
				Sortie = TRUE;
		}

		Taille = ind_resul;

		if (ind_resul > 0)
		{
			*disp_id = atoi(Resultat[Index].info_0);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_Display(), %d = %d", plazanum, *disp_id);

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

DWORD CONF_PV_Get_Display_Suite(CHAR *Tab0)
{
	DWORD i;
	BOOL Sortie = FALSE;
	DWORD x, y;

	for (i = 0; (i < MAX_ENR) && (Sortie == FALSE); i++)
	{
		x = atoi(Resultat[i].info_0);
		y = atoi(Tab0);

		if (x != y)
		{
			if (Resultat[i].info_0[0] == '\0')
			{
				strcpy_s(Resultat[i].info_0, sizeof(Resultat[i].info_0), Tab0);
				Sortie = TRUE;
			}
		}
		else
			Sortie = TRUE;
	}

	i = 0;
	while ((Resultat[i].info_0[0] != '\0') && (i < MAX_ENR))
		i++;

	return i;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Display(OUT DWORD *disp_id)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*disp_id = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_Display(), %d", *disp_id);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/