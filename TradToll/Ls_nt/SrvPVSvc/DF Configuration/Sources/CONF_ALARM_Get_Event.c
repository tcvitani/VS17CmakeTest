/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Event.c											 */
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

PROTECTED BOOL WINAPI CONF_ALARM_Get_First_Event(IN DWORD alarm_type,
												 IN DWORD alarm_sub_type,
												 OUT DWORD *event_id,
												 OUT DWORD *event_ack_mode,
												 OUT DWORD *event_level,
												 OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
												 OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE])
{
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

	*event_id = 0;
	*event_ack_mode = 0;
	*event_level = 0;
	event_wording[0] = '\0';
	event_directive[0] = '\0';

	if ((alarm_type != 0) && (alarm_sub_type != 0))
	{
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_EVENT_TYPE");
		_itoa_s(alarm_type, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_ALARM_TYPE);

		strcpy_s(TabCritere[1].info_0, sizeof(TabCritere[1].info_0), "ID_EVENT_SUB_TYPE");
		_itoa_s(alarm_sub_type, TabCritere[1].info_1, sizeof(TabCritere[1].info_1), MAX_ALARM_SUB_TYPE);

		nbcriteres = 2;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* ID_EVENT , ID_EVENT_ACK_TYPE, GRAVITY_LEVEL, CDE_LABEL_EVENT, SERVICING_ORDERS */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_EVENT");
		strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "ID_EVENT_ACK_TYPE");
		strcpy_s(TabInfo[2].info_0, sizeof(TabInfo[2].info_0), "GRAVITY_LEVEL");
		strcpy_s(TabInfo[3].info_0, sizeof(TabInfo[3].info_0), "CDE_LABEL_EVENT");
		strcpy_s(TabInfo[4].info_0, sizeof(TabInfo[4].info_0), "SERVICING_ORDERS");

		strcpy_s(TabCritereSuite[0].info_0, sizeof(TabCritereSuite[0].info_0), "CDE_LABEL");
		strcpy_s(TabInfoSuite[0].info_0, sizeof(TabInfoSuite[0].info_0), "LONG_LABEL");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			nbcriteres = 2;

			*pt_num_section = sauv_num_section;
			num_section = INI_Recherche("T_EVENT.ini", OUT pt_num_section, nbcriteres, TabCritere);

			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 5;

				bRet = INI_Recupere(IN "T_EVENT.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
				{
					strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
					strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfo[1].info_1);
					strcpy_s(Resultat[ind_resul].info_2, sizeof(Resultat[ind_resul].info_2), TabInfo[2].info_1);
					strcpy_s(Resultat[ind_resul].info_4, sizeof(Resultat[ind_resul].info_3), TabInfo[4].info_1);

					nbcriteres = 1;

					strcpy_s(TabCritereSuite[0].info_1, sizeof(TabCritereSuite[0].info_1), TabInfo[3].info_1);

					*pt_num_section = 0;

					num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritereSuite);
					if (num_section > 0)
					{
						bRet = FALSE;
						nbcriteres = 1;

						bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfoSuite);
						if (bRet == TRUE)
						{
							strcpy_s(Resultat[ind_resul].info_3, sizeof(Resultat[ind_resul].info_3), TabInfoSuite[0].info_1);
							
							if (ind_resul < MAX_ENR)
								ind_resul++;
						}
					}
				}

				if (bRet != TRUE)
				{
					Resultat[ind_resul].info_0[0] = '\0';
					Resultat[ind_resul].info_1[0] = '\0';
					Resultat[ind_resul].info_2[0] = '\0';
					Resultat[ind_resul].info_3[0] = '\0';
					Resultat[ind_resul].info_4[0] = '\0';
				}
			}
			else
				Sortie = TRUE;
		}
		
		Taille = ind_resul;
		
		if (ind_resul > 0)
		{
			*event_id = atoi(Resultat[Index].info_0);
			*event_ack_mode = atoi(Resultat[Index].info_1);
			*event_level = atoi(Resultat[Index].info_2);
			strcpy_s(event_wording, MSG_PV_MAX_ALARM_LABEL, Resultat[Index].info_3);
			strcpy_s(event_directive, MSG_PV_MAX_DETAIL_SIZE, Resultat[Index].info_4);

			NTSVCInfo("CONF_ALARM_Get_First_Event(), Id = %d, ackmode = %d, level = %d, wording = %s, directive = %s",
					*event_id, 
					*event_ack_mode, 
					*event_level, 
					event_wording, 
					event_directive);

			Index = Index + 1;

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_ALARM_Get_Next_Event(OUT DWORD *event_id,
												OUT DWORD *event_ack_mode,
												OUT DWORD *event_level,
												OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
												OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE])
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*event_id = atoi(Resultat[Index].info_0);
		*event_ack_mode = atoi(Resultat[Index].info_1);
		*event_level = atoi(Resultat[Index].info_2);
		strcpy_s(event_wording, MSG_PV_MAX_ALARM_LABEL, Resultat[Index].info_3);
		strcpy_s(event_directive, MSG_PV_MAX_DETAIL_SIZE, Resultat[Index].info_4);
		Index = Index + 1;

		NTSVCInfo("CONF_ALARM_Get_Next_Event(), Id = %d, ackmode = %d, level = %d, wording = %s, directive = %s",
				*event_id, 
				*event_ack_mode, 
				*event_level, 
				event_wording, 
				event_directive);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/


