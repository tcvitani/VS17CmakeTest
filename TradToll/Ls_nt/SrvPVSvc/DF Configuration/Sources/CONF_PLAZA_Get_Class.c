/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Class.c											 */
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
#include <Msg_Pv_define.h>
#include <gen.h>
#include <conf_ini.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Class(OUT DWORD *class_id,
												 OUT char class_name[MSG_PV_MAX_CLASS_LABEL])
{
	BOOL bRet							= FALSE;
	DWORD num_section					= 0;
	DWORD sauv_section					= 0;
	DWORD nbcriteres					= 0;
	DWORD *pt_num_section				= NULL;
	st_TabInfo TabInfo[MAX_ENR]			= { 0 };
	st_TabInfo TabInfoSuite[MAX_ENR]	= { 0 };
	st_TabInfo TabCritere[MAX_ENR]		= { 0 };
	DWORD type_p						= 5;
	BOOL Sortie							= FALSE;
	DWORD ind_resul						= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*class_id = 0;
	class_name[0] = '\0';

	/*  *curr_id, curr_label */
	strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_CLASS");
	strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "CDE_LABEL_CLASS");

	/* CDE_LABEL */
	strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "CDE_LABEL");

	/*  SHORT_LABEL_1 */
	strcpy_s(TabInfoSuite[0].info_0, sizeof(TabInfoSuite[0].info_0), "SHORT_LABEL_1");

	pt_num_section = &num_section;

	while (Sortie == FALSE)
	{
		nbcriteres = 2;

		num_section = sauv_section;
		num_section++;

		bRet = INI_Recupere(IN "T_CLASS.ini", num_section, nbcriteres, TabInfo);

		sauv_section = num_section;
		if (bRet == TRUE)
		{
			strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
			strcpy_s(TabCritere[0].info_1, sizeof(TabCritere[0].info_1), TabInfo[1].info_1);

			nbcriteres = 1;

			*pt_num_section = 0;

			num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				nbcriteres = 1;

				bRet = INI_Recupere(IN "T_TRANSLATE.ini", num_section, nbcriteres, TabInfoSuite);
				if (bRet == TRUE)
				{
					strcpy_s(Resultat[ind_resul].info_1, sizeof(Resultat[ind_resul].info_1), TabInfoSuite[0].info_1);
					
					if (ind_resul < MAX_ENR)
						ind_resul++;
				}
				else
				{
					Resultat[ind_resul].info_0[0] = '\0';
					Resultat[ind_resul].info_1[0] = '\0';
				}
			}
			else
			{
				Resultat[ind_resul].info_0[0] = '\0';
				Resultat[ind_resul].info_1[0] = '\0';
			}
		}
		else
			Sortie = TRUE;
	}

	Taille = ind_resul;

	if (ind_resul > 0)
	{
		*class_id = atoi(Resultat[Index].info_0);
		strcpy_s(class_name, MSG_PV_MAX_CLASS_LABEL, Resultat[Index].info_1);

		NTSVCInfo("CONF_PLAZA_Get_First_Class(), %s = %d", class_name, *class_id);

		Index = Index + 1;

		bRet = TRUE;
	}
	else
		bRet = FALSE;

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Class(OUT DWORD *class_id,
												OUT CHAR class_name[MSG_PV_MAX_CLASS_LABEL])
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*class_id = atoi(Resultat[Index].info_0);
		strcpy_s(class_name, MSG_PV_MAX_CLASS_LABEL, Resultat[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_PLAZA_Get_Next_Class(), %s = %d", class_name, *class_id);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
