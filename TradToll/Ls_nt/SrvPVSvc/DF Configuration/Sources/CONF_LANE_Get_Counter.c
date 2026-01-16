/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_LANE_Get_Counter.c									     */
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

PROTECTED BOOL WINAPI CONF_LANE_Get_Counter(IN DWORD plazanum,
											IN DWORD lanenum,
											IN DWORD disp_id,
											OUT BOOL counter[LANE_MAX_COUNTERS])
{
	DWORD		i						= 0;
	BOOL		bRet					= FALSE;
	DWORD		num_section				= 0;
	DWORD		sauv_num_section		= 0;
	DWORD		nbcriteres				= 0;
	DWORD		*pt_num_section			= NULL;
	st_TabInfo	TabInfo[MAX_ENR]		= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]		= { 0 };
	DWORD		Sortie					= FALSE;
	DWORD		ind_resul				= 0;
	DWORD		total_counter			= 0;
	DWORD		pos						= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	counter[0] = '\0';

	if ((plazanum != 0) && (lanenum != 0) && (disp_id != 0))
	{
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PLAZA");
		_itoa_s(plazanum, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_PLAZA_NUM);

		strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_LANE");
		_itoa_s(lanenum, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ID_LANE);

		strcpy_s(TabCritere[2].info_0, sizeof(TabCritere[2].info_0), "ID_LANE_GROUP");
		_itoa_s(disp_id, TabCritere[2].info_1, sizeof(TabCritere[2].info_1), MAX_ID_LANE_GROUP);

		nbcriteres = 3;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* COUNTER_TYPE */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_RTO_COUNTER");

		pt_num_section = &num_section;

		for (i = 0; i < LANE_MAX_COUNTERS; i++)
			counter[i] = '\0';

		while (Sortie == FALSE)
		{

			/* T_EVENT.INI */
			nbcriteres = 3;

			*pt_num_section = sauv_num_section;

			num_section = INI_Recherche("T_RTO_LANE_COUNTER.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_RTO_LANE_COUNTER.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
				{
					pos = atoi(TabInfo[0].info_1) - 1;

					if (pos < LANE_MAX_COUNTERS)
						counter[pos] = 1;
				}

			}
			else
				Sortie = TRUE;
		}

		NTSVCInfo("CONF_LANE_Get_Counter(), ( %d %d %d %d %d )", 
				counter[0], 
				counter[1], 
				counter[2], 
				counter[3], 
				counter[4]);

		bRet = TRUE;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/