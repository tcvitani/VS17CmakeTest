/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Payment_Sub_Type.c								 */
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

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Payment_Sub_Type(IN DWORD pay_id,
															OUT DWORD *pay_sub_id,
															OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	BOOL bRet								= FALSE;
	DWORD num_section						= 0;
	DWORD sauv_num_section					= 0;
	DWORD nbcriteres						= 0;
	DWORD *pt_num_section					= NULL;
	st_TabInfo	TabInfo[MAX_ENR]			= { 0 };
	st_TabInfo	TabInfoSuite[MAX_ENR]		= { 0 };
	st_TabInfo	TabCritere[MAX_ENR]			= { 0 };
	st_TabInfo	TabCritereSuite[MAX_ENR]	= { 0 };
	DWORD Sortie							= FALSE;
	DWORD ind_resul							= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*pay_sub_id = 0;
	pay_name[0] = '\0';

	if (pay_id != 0)
	{
		/* ID_PAYMENT_TYPE */
		strcpy_s(TabCritere[0].info_0, sizeof(TabCritere[0].info_0), "ID_PAYMENT_TYPE");
		_itoa_s(pay_id, TabCritere[0].info_1, sizeof(TabCritere[0].info_1), MAX_PAYMENT_TYPE);

		nbcriteres = 1;
	}

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_PAYMENT_SUB_TYPE");
		strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "CDE_LABEL_PAYMENT_SUB_TYPE");

		/* CDE_LABEL */
		strcpy_s(TabCritereSuite[0].info_0, sizeof(TabCritereSuite[0].info_0), "CDE_LABEL");

		/* SHORT_LABEL_1 */
		strcpy_s(TabInfoSuite[0].info_0, sizeof(TabInfoSuite[0].info_0), "SHORT_LABEL_1");

		pt_num_section = &num_section;

		while (Sortie == FALSE)
		{
			/* T_PROFILE.INI */
			nbcriteres = 1;

			*pt_num_section = sauv_num_section;

			num_section = INI_Recherche("T_PAYMENT_SUB_TYPE.ini", OUT pt_num_section, nbcriteres, TabCritere);
			if (num_section > 0)
			{
				sauv_num_section = num_section;
				bRet = FALSE;
				nbcriteres = 2;

				bRet = INI_Recupere(IN "T_PAYMENT_SUB_TYPE.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
				{
					strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
					strcpy_s(TabCritereSuite[0].info_1, sizeof(TabCritereSuite[0].info_1), TabInfo[1].info_1);

					nbcriteres = 1;
					*pt_num_section = 0;

					/* T_TRANSLATE.INI */
					num_section = INI_Recherche("T_TRANSLATE.ini", OUT pt_num_section, nbcriteres, TabCritereSuite);
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
						else
						{
							Resultat[ind_resul].info_0[0] = '\0';
							Resultat[ind_resul].info_1[0] = '\0';
						}
					}
				}
			}
			else
				Sortie = TRUE;
		}

		Taille = ind_resul;

		if (ind_resul > 0)
		{
			*pay_sub_id = atoi(Resultat[Index].info_0);
			strcpy_s(pay_name, MSG_PV_MAX_PAYMENT_LABEL, Resultat[Index].info_1);
			Index = Index + 1;

			NTSVCInfo("CONF_PLAZA_Get_First_Payment_Sub_Type(), %s = %d", pay_name, *pay_sub_id);

			bRet = TRUE;
		}
		else
			bRet = FALSE;
	}

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Payment_Sub_Type(OUT DWORD *pay_sub_id,
														   OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL])
{
	BOOL bRet = FALSE;

	if (Index < Taille)
	{
		*pay_sub_id = atoi(Resultat[Index].info_0);
		strcpy_s(pay_name, MSG_PV_MAX_PAYMENT_LABEL, Resultat[Index].info_1);
		Index = Index + 1;

		NTSVCInfo("CONF_PLAZA_Get_Next_Payment_Sub_Type(), %s = %d", pay_name, *pay_sub_id);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
