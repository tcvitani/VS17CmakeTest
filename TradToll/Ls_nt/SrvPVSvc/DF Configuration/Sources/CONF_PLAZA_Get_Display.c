/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Display.c										 */
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
#include <lane.h>
#include <gen.h>
#include <conf_ini.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE DWORD CONF_PLAZA_Get_Display_Suite(CHAR *Tab0, CHAR *Tab1, CHAR *Tab2);

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

#pragma warning (disable : 4996)

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Display(IN DWORD plazanum,
												   OUT DWORD *disp_id,
												   OUT DISP_COUNT counter[LANE_MAX_COUNTERS])
{
	DWORD		i							= 0;
	DWORD		ind							= 0;
	BOOL		bRet						= FALSE;
	DWORD		num_section					= 0;
	DWORD		sauv_num_section			= 0;
	DWORD		nbcriteres					= 0;
	DWORD		*pt_num_section				= NULL;
	st_TabInfo	TabInfo[MAX_ENR]			= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]			= { 0 };
	DWORD		Sortie						= FALSE;
	DWORD		ind_resul					= 0;
	DWORD		pos[LANE_MAX_COUNTERS]		= { 0 };
	DWORD		cnt[LANE_MAX_COUNTERS + 1]	= { 0 };

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*disp_id = 0;
	counter[0] = '\0';

	if (plazanum != 0)
	{
		/* ID_PLAZA */
		strcpy(TabCritere[0].info_0, "ID_PLAZA");
		_itoa_s(plazanum, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ID_PLAZA_NUM);

		nbcriteres = 1;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* ID_LANE, COUNTER_TYPE */
		strcpy(TabInfo[0].info_0, "ID_LANE_GROUP");
		strcpy(TabInfo[1].info_0, "ID_RTO_COUNTER");
		strcpy(TabInfo[2].info_0, "COUNTER_TYPE");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			/* T_RTO_LANE_COUNTER.INI */
			nbcriteres = 1;

			*pt_num_section = sauv_num_section;

			num_section = INI_Recherche("T_RTO_LANE_COUNTER.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 3;
				
				bRet = INI_Recupere(IN "T_RTO_LANE_COUNTER.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
					ind_resul = CONF_PLAZA_Get_Display_Suite(TabInfo[0].info_1, TabInfo[1].info_1, TabInfo[2].info_1);
			}
			else
				Sortie = TRUE;
		}

		Taille = ind_resul;

		for (i = 0; i < Taille; i++)
		{
			ind = atoi(Resultat[i].info_3);

			while (ind < LANE_MAX_COUNTERS)
			{
				strcat(Resultat[i].info_1, "|");
				strcat(Resultat[i].info_1, "0");

				strcat(Resultat[i].info_2, "|");
				strcat(Resultat[i].info_2, "0");
				
				ind++;
			}

			itoa(ind, Resultat[i].info_3, sizeof(Resultat[i].info_3));
		}

		if (ind_resul > 0)
		{
			*disp_id = atoi(Resultat[Index].info_0);

			if (sscanf_s(Resultat[Index].info_1, "%d|%d|%d|%d|%d", &pos[0], &pos[1], &pos[2], &pos[3], &pos[4]) < LANE_MAX_COUNTERS)
				bRet = FALSE;
			else
			{
				for (i = 0; i < LANE_MAX_COUNTERS + 1; i++) 
					cnt[i] = 0;

				if (sscanf_s(Resultat[Index].info_2, 
							"%d|%d|%d|%d|%d", 
							&cnt[pos[0]], 
							&cnt[pos[1]], 
							&cnt[pos[2]], 
							&cnt[pos[3]], 
							&cnt[pos[4]]) < LANE_MAX_COUNTERS)
				{
					bRet = FALSE;
				}
				else
				{
					for (i = 0; i < LANE_MAX_COUNTERS; i++) counter[i] = cnt[i + 1];

					Index = Index + 1;

					NTSVCInfo("CONF_PLAZA_Get_First_Display(), Display = %d ( %d %d %d %d %d )", 
							*disp_id, 
							counter[0], 
							counter[1], 
							counter[2], 
							counter[3], 
							counter[4]);

					bRet = TRUE;
				}
			}
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

DWORD CONF_PLAZA_Get_Display_Suite(CHAR *Tab0, CHAR *Tab1, CHAR *Tab2)
{
	DWORD		i				= 0;
	DWORD		tot_count		= 0;
	DWORD		x				= 0; 
	DWORD		y				= 0;
	DWORD		counter_type	= 0;
	DWORD		counter_id		= 0;
	BOOL		Sortie			= FALSE;
	BOOL		Continue		= TRUE;
	DISP_COUNT	counter_resultat;
	CHAR		ltab[6]			= { 0 };
	CHAR		ltab1[6]		= { 0 };
	CHAR		*pt_ltab		= NULL;
	CHAR		*pt_ltab1		= NULL;

	pt_ltab = ltab;
	pt_ltab1 = ltab1;

	for (i = 0; (i < MAX_ENR) && (Sortie == FALSE); i++)
	{
		// Reading the display_id for index i
		x = atoi(Resultat[i].info_0);
		y = atoi(Tab0);

		// If the display_id from the array does not match the processed display_id
		if (x != y)
		{
			// If the display_id read from the array is non-existent
			if (Resultat[i].info_0[0] == '\0')
			{
				// The processed display_id is put in the array
				strcpy(Resultat[i].info_0, Tab0);
				strcat(Resultat[i].info_1, Tab1);
				strcat(Resultat[i].info_2, Tab2);

				tot_count = 1;
				itoa(tot_count, Resultat[i].info_3, 2);

				Sortie = TRUE;
			}
		}
		// Otherwise the processed display_id already exists in the array
		else
		{
			tot_count = atoi(Resultat[i].info_3);

			// if the number of COUNTER_TYPE written in the table is correct
			if (tot_count < LANE_MAX_COUNTERS)
			{
				Sortie = TRUE;

				for (y = 0; (y < MAX_ENR) && (Continue == TRUE); y++)
				{
					// Comparison between the value entered in Result and COUNTER_TYPE
					counter_id = atoi(Tab1);
					counter_type = atoi(Tab2);

					while ((Resultat[i].info_2[y] != '|') && (Resultat[i].info_2[y] != '\0'))
					{
						*pt_ltab = Resultat[i].info_1[y];
						*pt_ltab1 = Resultat[i].info_2[y];
						y++;
						pt_ltab++;
						pt_ltab1++;
					}

					*pt_ltab = '\0';
					*pt_ltab1 = '\0';

					counter_resultat = atoi(ltab1);

					// if the counter from the result array is equal to the COUNTER_TYPE and if the counter of the array is not null
					if (((DWORD)counter_type == (DWORD)atoi(ltab1)) && ((DWORD)counter_id == (DWORD)atoi(ltab)) && (counter_resultat > 0))
						Continue = FALSE;
					// Otherwise, if the counter from the result array is null, it is because the one that is at the end of search
					else if (counter_resultat == 0)
						y = MAX_ENR;

					pt_ltab = ltab;
					pt_ltab1 = ltab1;
				}

				if ((Continue == TRUE) && (y >= (LANE_MAX_COUNTERS * 2)))
				{
					Sortie = TRUE;
					tot_count++;

					strcat(Resultat[i].info_1, "|");
					strcat(Resultat[i].info_1, Tab1);

					strcat(Resultat[i].info_2, "|");
					strcat(Resultat[i].info_2, Tab2);

					itoa(tot_count, Resultat[i].info_3, sizeof(Resultat[i].info_3));
				}
			}
			else
				Sortie = TRUE;
		}
	}

	i = 0;
	while ((Resultat[i].info_2[0] != '\0') && (i < MAX_ENR))
		i++;

	return i;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Display(OUT DWORD *disp_id,
												  OUT DISP_COUNT counter[LANE_MAX_COUNTERS])
{
	BOOL bRet = FALSE;

	DWORD i;
	DWORD pos[LANE_MAX_COUNTERS];
	DWORD cnt[LANE_MAX_COUNTERS + 1];

	if (Index < Taille)
	{
		*disp_id = atoi(Resultat[Index].info_0);

		if (sscanf_s(Resultat[Index].info_1, 
					"%d|%d|%d|%d|%d", 
					&pos[0], 
					&pos[1], 
					&pos[2], 
					&pos[3], 
					&pos[4]) < LANE_MAX_COUNTERS)
		{
			bRet = FALSE;
		}
		else
		{
			for (i = 0; i < LANE_MAX_COUNTERS + 1; i++) 
				cnt[i] = 0;

			if (sscanf_s(Resultat[Index].info_2, 
						"%d|%d|%d|%d|%d", 
						&cnt[pos[0]], 
						&cnt[pos[1]], 
						&cnt[pos[2]], 
						&cnt[pos[3]], 
						&cnt[pos[4]]) < LANE_MAX_COUNTERS)
			{
				bRet = FALSE;
			}
			else
			{
				for (i = 0; i < LANE_MAX_COUNTERS; i++) 
					counter[i] = cnt[i + 1];

				Index = Index + 1;

				NTSVCInfo("CONF_PLAZA_Get_Next_Display(), Display = %d ( %d %d %d %d %d )", 
						*disp_id, 
						counter[0], 
						counter[1], 
						counter[2], 
						counter[3], 
						counter[4]);

				bRet = TRUE;
			}
		}
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/