/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_ALARM_Get_Shift_Mode_And_Vault_Events.c						 */
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

PROTECTED BOOL WINAPI CONF_ALARM_Get_Shift_Mode_And_Vault_Events(OUT DWORD *pShiftAndModeEvtType,
																 OUT DWORD *pShiftEvtSubType,
																 OUT DWORD *pModeEvtSubType,
																 OUT DWORD *pVault1EvtType,
																 OUT DWORD *pVault2EvtType)
{
	BOOL bRet					= FALSE;
	DWORD num_section			= 0;
	DWORD nbcriteres			= 0;
	st_TabInfo TabInfo[MAX_ENR] = { 0 };

	Index = 0;

	INI_Resultat_ZeroMemory(Resultat, MAX_ENR);

	*pShiftAndModeEvtType = 0;
	*pShiftEvtSubType = 0;
	*pModeEvtSubType = 0;
	*pVault1EvtType = 0;
	*pVault2EvtType = 0;

	nbcriteres = 1;

	if (nbcriteres == 0)
		bRet = FALSE;
	else
	{
		/* *pShiftEvtSubType, *pModeEvtSubType */
		strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "C_SUB_TYPE_EVENT_STATUT");
		strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "C_TYPE_SUB_EVENT_MODE_OUVERT");
		
		nbcriteres = 2;
		num_section = 1;

		bRet = INI_Recupere(IN "GEN_DEF.ini", num_section, nbcriteres, TabInfo);

		if (bRet == TRUE)
		{
			/* *pShiftEvtSubType,*pModeEvtSubType */
			*pShiftEvtSubType = atoi(TabInfo[0].info_1);
			*pModeEvtSubType = atoi(TabInfo[1].info_1);

			/* *pVault1EvtType,*pVault2EvtType */
			strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "C_TYPE_EVENT_CHENAL");
			strcpy_s(TabInfo[1].info_0, sizeof(TabInfo[1].info_0), "C_SUB_TYPE_EVENT_MARQUISE");
			nbcriteres = 2;

			bRet = INI_Recupere(IN "GEN_DEF.ini", num_section, nbcriteres, TabInfo);
			if (bRet == TRUE)
			{
				*pVault1EvtType = atoi(TabInfo[0].info_1);
				*pVault2EvtType = atoi(TabInfo[1].info_1);

				strcpy_s(TabInfo[0].info_0, sizeof(TabInfo[0].info_0), "C_TYPE_EVENT_POSTE");
				nbcriteres = 1;

				bRet = INI_Recupere(IN "GEN_DEF.ini", num_section, nbcriteres, TabInfo);
				if (bRet == TRUE)
				{
					*pShiftAndModeEvtType = atoi(TabInfo[0].info_1);

					NTSVCInfo("CONF_ALARM_Get_Shift_Mode_And_Vault_Events(), poste=%d id=%d mode=%d coffre1=%d coffre2=%d", 
							*pShiftAndModeEvtType, 
							*pShiftEvtSubType, 
							*pModeEvtSubType, 
							*pVault1EvtType, 
							*pVault2EvtType);
				}
			}
		}
	}

	if (bRet != TRUE)
	{
		*pShiftAndModeEvtType = 0;
		*pShiftEvtSubType = 0;
		*pModeEvtSubType = 0;
		*pVault1EvtType = 0;
		*pVault2EvtType = 0;
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/