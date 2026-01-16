/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Alarm.c											 */
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

st_resul Resultat2[MAX_ENR_RESUL];

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_ALARM_Get_First_Alarm(OUT DWORD *alarm_type, 
												 OUT DWORD *alarm_sub_type)
{
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		sauv_num_section	= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR] = { 0 };
	BOOL		Sortie				= FALSE;
	DWORD		ind_resul			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat2, MAX_ENR_RESUL);

	*alarm_type = 0;
	*alarm_sub_type = 0;

	/*  T_EVENT_SUB_TYPE.INI */
	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "FLAG_MONITORING");
	_itoa_s(1, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), 2);

	/*	*alarm_type, *alarm_sub_type */
	strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_EVENT_TYPE");
	strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "ID_EVENT_SUB_TYPE");

	pt_num_section = &num_section;

	while (Sortie == FALSE)
	{
		nbcriteres = 1;

		*pt_num_section = sauv_num_section;
		num_section = INI_Recherche("T_EVENT_SUB_TYPE.ini", OUT pt_num_section, nbcriteres, TabCritere);

		if (num_section > 0)
		{
			sauv_num_section = num_section;
			bRet = FALSE;
			nbcriteres = 2;

			bRet = INI_Recupere(IN "T_EVENT_SUB_TYPE.ini", num_section, nbcriteres, TabInfo);

			if (bRet == TRUE)
			{
				strcpy_s(Resultat2[ind_resul].info_0, sizeof(Resultat2[ind_resul].info_0), TabInfo[0].info_1);
				strcpy_s(Resultat2[ind_resul].info_1, sizeof(Resultat2[ind_resul].info_0), TabInfo[1].info_1);
				
				if (ind_resul < MAX_ENR_RESUL)
					ind_resul++;
			}
			else
			{
				Resultat2[ind_resul].info_0[0] = '\0';
				Resultat2[ind_resul].info_1[0] = '\0';
			}
		}
		else
			Sortie = TRUE;
	} 

	Taille = ind_resul;
	
	if (ind_resul > 0)
	{
		*alarm_type = atoi(Resultat2[Index].info_0);
		*alarm_sub_type = atoi(Resultat2[Index].info_1);
		
		Index = Index + 1;

		NTSVCInfo("CONF_ALARM_Get_First_Alarm(), AlarmType = %d, AlarmSubType = %d", *alarm_type, *alarm_sub_type);

		bRet = TRUE;
	}
	else
		bRet = FALSE;

	return bRet;
}

PROTECTED BOOL WINAPI CONF_ALARM_Get_Next_Alarm(OUT DWORD *alarm_type, 
												OUT DWORD *alarm_sub_type)
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*alarm_type = atoi(Resultat2[Index].info_0);
		*alarm_sub_type = atoi(Resultat2[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_ALARM_Get_Next_Alarm(), AlarmType = %d, AlarmSubType = %d", *alarm_type, *alarm_sub_type);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat2, MAX_ENR_RESUL);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/

