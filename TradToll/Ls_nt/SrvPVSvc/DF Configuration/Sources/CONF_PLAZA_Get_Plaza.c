/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_PLAZA_Get_Plaza.c											 */
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

PROTECTED BOOL WINAPI CONF_PLAZA_Get_First_Plaza(OUT DWORD *plazanum, DWORD dwDefaultPlazaNumber)
{
	BOOL		bRet				= FALSE;
	DWORD		num_section			= 0;
	DWORD		sauv_num_section	= 0;
	DWORD		nbcriteres			= 0;
	DWORD		*pt_num_section		= NULL;
	st_TabInfo	TabInfo[MAX_ENR]	= { 0 };
	DWORD		Sortie				= FALSE;
	DWORD		ind_resul			= 0;

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	if (dwDefaultPlazaNumber > 0)
	{
		*plazanum = dwDefaultPlazaNumber;
		return TRUE;
	}

	/* ID_PLAZA */
	strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "ID_PLAZA");

	pt_num_section = &num_section;

	while (Sortie == FALSE)
	{
		sauv_num_section = num_section;

		num_section++;

		bRet = FALSE;
		nbcriteres = 1;

		bRet = INI_Recupere(IN "T_PLAZA.ini", num_section, nbcriteres, TabInfo);

		sauv_num_section = num_section;

		/* ID_PLAZA*/
		if (bRet == TRUE)
		{
			strcpy_s(Resultat[ind_resul].info_0, sizeof(Resultat[ind_resul].info_0), TabInfo[0].info_1);
			ind_resul++;
		}
		else
			Sortie = TRUE;
	}

	Taille = ind_resul;

	if (ind_resul > 0)
	{
		*plazanum = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PLAZA_Get_First_Plaza(), Plazanum = %d", *plazanum);

		bRet = TRUE;
	}
	else
		bRet = FALSE;

	return bRet;
}

PROTECTED BOOL WINAPI CONF_PLAZA_Get_Next_Plaza(OUT DWORD *plazanum, DWORD dwDefaultPlazaNumber)
{
	BOOL bRet = FALSE;

	if (dwDefaultPlazaNumber > 0)
		return TRUE;

	if (Index < Taille)
	{
		*plazanum = atoi(Resultat[Index].info_0);
		Index = Index + 1;

		NTSVCInfo("CONF_PLAZA_Get_Next_Plaza(), Plazanum = %d", *plazanum);

		bRet = TRUE;
	}
	else
		INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/