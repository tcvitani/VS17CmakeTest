/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Lane_Device.c										 */
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

extern DWORD INI_Recherche(IN char *fichier,
						   OUT DWORD *pt_num_section,
						   IN DWORD nb_critere,
						   IN OUT struct tab TabCritere[]);

extern BOOL INI_Recupere(IN char *fichier,
						 IN DWORD num_section,
						 IN DWORD nb,
						 IN OUT struct tab TabInfo[]);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PV_Get_First_Lane_Device(IN DWORD plazanum,
													IN DWORD lanenum,
													OUT DWORD *dev_id)
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

	*dev_id = 0;

	if ((plazanum != 0) && (lanenum != 0))
	{
		/* ID_PLAZA, ID_LANE */
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PLAZA");
		_itoa_s(plazanum, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_PLAZA_NUM);

		strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_LANE");
		_itoa_s(lanenum, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ID_LANE);

		nbcriteres = 2;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* ID_LANE_EQUIPEMENT_TYPE*/
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_LANE_EQUIPEMENT_TYPE");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			/* T_LANE_EQUIPEMENT.INI */
			*pt_num_section = sauv_num_section;

			nbcriteres = 2;

			num_section = INI_Recherche("T_LANE_EQUIPEMENT.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;

				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_LANE_EQUIPEMENT.ini", num_section, nbcriteres, TabInfo);
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
			*dev_id = atoi(Resultat[Index].info_0);
			Index = Index + 1;

			NTSVCInfo("CONF_PV_Get_First_Lane_Device(), %d = %d", lanenum, *dev_id);

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Lane_Device(OUT DWORD *dev_id)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*dev_id = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_Lane_Device(), %d", *dev_id);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/