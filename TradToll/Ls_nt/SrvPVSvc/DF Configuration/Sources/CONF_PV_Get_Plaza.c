/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PV_Get_Plaza.c											 */
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

PROTECTED BOOL WINAPI CONF_PV_Get_First_Plaza(OUT DWORD *plazanum,
											  OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
											  IN DWORD dwDefaultPlazaNumber)
{
	BOOL		bRet					= FALSE;
	DWORD		num_section				= 0;
	DWORD		sauv_num_section		= 0;
	DWORD		nbcriteres				= 0;
	DWORD		*pt_num_section			= NULL;
	st_TabInfo	TabInfo[MAX_ENR]		= { 0 };
	st_TabInfo	TabInfoSuite[MAX_ENR]	= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]		= { 0 };
	DWORD		Sortie					= FALSE;
	DWORD		ind_resul				= 0;
	BOOL		all_criteres_trouves	= FALSE;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	if (dwDefaultPlazaNumber > 0)
	{
		*plazanum = dwDefaultPlazaNumber;
		return TRUE;
	}

	/* CDE_LABEL_PLAZA */
	strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_PLAZA");
	strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "CDE_LABEL_PLAZA");

	/* SHORT_LABEL_1 */
	strcpy_s(TabInfoSuite[0].info_0, sizeof(TabInfoSuite[0].info_0), "SHORT_LABEL_1");

	/* CDE_LABEL from T_TRANSLATE */
	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "CDE_LABEL");

	pt_num_section = &num_section;

	while (Sortie == FALSE)
	{
		/* T_PROFILE.INI */
		/* ID_PLAZA */
		num_section = sauv_num_section;
		num_section++;
		bRet = FALSE;
		nbcriteres = 2;

		bRet = INI_Recupere(IN "T_PLAZA.ini", num_section, nbcriteres, TabInfo);

		sauv_num_section = num_section;

		if (bRet == TRUE)
		{
			nbcriteres = 1;

			strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
			strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[1].info_1);

			*pt_num_section = 0;

			/* T_TRANSLATE.INI */
			num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				bRet = FALSE;
				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfoSuite);
				if (bRet == TRUE)
				{
					strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfoSuite[0].info_1);
					
					if (ind_resul < MAX_ENR)
						ind_resul++;
				}
			}
		}
		else
			Sortie = TRUE;
	}

	Taille = ind_resul;

	if (ind_resul > 0)
	{
		*plazanum = atoi(Resultat[Index].info_0);
		strcpy_s(plazaname, MSG_PV_MAX_PLAZA_NAME, Resultat[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_First_Plaza(), %s = %d", plazaname, *plazanum);

		bRet = TRUE;
	}
	else
		bRet = FALSE;

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PV_Get_Next_Plaza(OUT DWORD *plazanum,
											 OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
											 IN DWORD dwDefaultPlazaNumber)
{
	BOOL bRet = FALSE;

	if (dwDefaultPlazaNumber > 0)
		return TRUE;

	if (Index < Taille)
	{
		*plazanum = atoi(Resultat[Index].info_0);
		strcpy_s(plazaname, MSG_PV_MAX_PLAZA_NAME, Resultat[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_PV_Get_Next_Plaza(), %s = %d", plazaname, *plazanum);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/