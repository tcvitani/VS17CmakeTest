/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Display_Zone_Lane.c								 */
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

PROTECTED BOOL WINAPI CONF_PV_Get_First_Display_Zone_Lane(IN DWORD plazanum,
														  IN DWORD disp_id,
														  IN DWORD zone_id,
														  OUT DWORD *lane_number)
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

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*lane_number = 0;

	if ((plazanum != 0) && (disp_id != 0) && (zone_id != 0))
	{
		/* ID_PLAZA, ID_LANE_GROUP from RTO_ZONE */
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PLAZA");
		_itoa_s(plazanum, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_PLAZA_NUM);

		strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_LANE_GROUP");
		_itoa_s(disp_id, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ID_LANE_GROUP);

		strcpy_s(TabCritere[2].info_0, sizeof(TabCritere[2].info_0), "RTO_ZONE");
		_itoa_s(zone_id, TabCritere[2].info_1, sizeof(TabCritere[2].info_1), MAX_RTO_ZONE);

		nbcriteres = 3;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* ID_LANE */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_LANE");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			/* T_LANE_ATTACH_TO.INI */
			*pt_num_section = sauv_num_section;

			nbcriteres = 3;

			num_section = INI_Recherche("T_LANE_ATTACH_TO.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_LANE_ATTACH_TO.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
				{
					strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
					if (ind_resul < MAX_ENR)
						ind_resul++;
				}
				else
					Resultat[ind_resul].info_0[0] = '\0';
			}
			else
				Sortie = TRUE;
		}

		Taille = ind_resul;

		if (ind_resul > 0)
		{
			*lane_number = atoi(Resultat[Index].info_0);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_Display_Zone_Lane(), %d = %d", disp_id, zone_id, *lane_number);

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Display_Zone_Lane(OUT DWORD *lane_number)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*lane_number = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_Display_Zone_Lane(), %d", *lane_number);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/