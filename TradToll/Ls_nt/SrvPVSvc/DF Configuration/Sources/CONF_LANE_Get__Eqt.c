/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_LANE_Get_Eqt.c											 */
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

PROTECTED BOOL WINAPI CONF_LANE_Get_First_Eqt(IN DWORD plazanum,
											  IN DWORD lanenum,
											  OUT DWORD *eqt_id,
											  OUT DWORD *alarm_event,
											  OUT DWORD *alarm_sub_event)
{
	DWORD		ind							= 0;
	BOOL		bRet						= FALSE;
	DWORD		num_section					= 0;
	DWORD		sauv_num_section			= 0;
	DWORD		nbcriteres					= 0;
	DWORD		*pt_num_section				= NULL;
	st_TabInfo	TabInfo[MAX_ENR]			= { 0 };
	st_TabInfo	TabInfoSuite[MAX_ENR]		= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]			= { 0 };
	st_TabInfo	TabCritereSuite[MAX_ENR]	= { 0 };
	DWORD		Sortie						= FALSE;
	DWORD		ind_resul					= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*eqt_id = 0;
	*alarm_event = 0;
	*alarm_sub_event = 0;

	if ((plazanum != 0) && (lanenum != 0))
	{
		/* ID_PLAZA et ID_LANE */
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

		/* ID_LANE_EQUIPEMENT_TYPE */
		strcpy_s(TabCritereSuite[0].info_0, sizeof(TabCritereSuite[0].info_0), "ID_LANE_EQUIPEMENT_TYPE");

		/* ID_EVENT_TYPE et ID_EVENT_SUB_TYPE */
		strcpy_s(TabInfoSuite[0].info_0, sizeof(TabInfoSuite[0].info_0), "ID_EVENT_TYPE");
		strcpy_s(TabInfoSuite[1].info_0, sizeof(TabInfoSuite[1].info_0), "ID_EVENT_SUB_TYPE");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			/* T_LANE_EQUIPEMENT.INI */
			nbcriteres = 2;

			*pt_num_section = sauv_num_section;

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

					/* T_LANE_EQUIPEMENT_TYPE.INI */
					strcpy_s(TabCritereSuite[0].info_1, sizeof(TabCritereSuite[0].info_1), TabInfo[0].info_1);

					nbcriteres = 1;
					*pt_num_section = 0;

					num_section = INI_Recherche("T_LANE_EQUIPEMENT_TYPE.ini", OUT pt_num_section, nbcriteres, TabCritereSuite);
					if (num_section > 0)
					{
						bRet = FALSE;
						nbcriteres = 2;

						bRet = INI_Recupere(IN "T_LANE_EQUIPEMENT_TYPE.ini", num_section, nbcriteres, TabInfoSuite);
						if (bRet == TRUE)
						{
							/** *alarm_event et *alarm_sub_event */
							strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfoSuite[0].info_1);
							strcpy_s(Resultat[ind_resul].info_2, sizeof(Resultat[ind_resul].info_2), TabInfoSuite[1].info_1);
							
							if (ind_resul < MAX_ENR)
								ind_resul++;
						}
						else
						{
							Resultat[ind_resul].info_0[0] = '\0';
							Resultat[ind_resul].info_1[0] = '\0';
							Resultat[ind_resul].info_2[0] = '\0';
						}
					}
					else
						Resultat[ind_resul].info_0[0] = '\0';
				}
			}
			else
				Sortie = TRUE;
		}

		Taille = ind_resul;

		if (ind_resul > 0)
		{
			*eqt_id = atoi(Resultat[Index].info_0);
			*alarm_event = atoi(Resultat[Index].info_1);
			*alarm_sub_event = atoi(Resultat[Index].info_2);
			Index = Index + 1;

			NTSVCInfo("CONF_LANE_Get_First_Eqt(), Eqtid = %d, Event = %d, SubEvent = %d", 
					*eqt_id, 
					*alarm_event, 
					*alarm_sub_event);

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_LANE_Get_Next_Eqt(OUT DWORD *eqt_id,
											 OUT DWORD *alarm_event,
											 OUT DWORD *alarm_sub_event)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*eqt_id = atoi(Resultat[Index].info_0);
		*alarm_event = atoi(Resultat[Index].info_1);
		*alarm_sub_event = atoi(Resultat[Index].info_2);
		Index = Index + 1;

		NTSVCInfo("CONF_LANE_Get_Next_Eqt(), Eqtid = %d, Event = %d, SubEvent = %d", *eqt_id, *alarm_event, *alarm_sub_event);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

