/* --------------------------------------------------------------------
* (C) 2000 CS SI - UORO - All rights reserved
* --------------------------------------------------------------------
* MODULE     : PRT
* FILE       : PRT.C
* LANGAGE    : C/C++
* --------------------------------------------------------------------
* KEYWORDS   :
* --------------------------------------------------------------------
* SUMMARY    :
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORY    :
*
*   * $Log : $
*
* --------------------------------------------------------------------
*/

#include <windows.h>
#include <stdio.h>

#include <noyau.h>
#include <mbx2slot.h>

#include <memclass.h>

PROTECTED DWORD WINAPI Mbx2SlotThread(void * pvParam);
PRIVATE noyau_enum_retour WINAPI CleanUpBeforeExit(mbx2slot_wrapper * psMbx2slot);
extern void SlotNewMessage(void *pQObject, struct_neutre *psMsg);



PROTECTED void    * WINAPI PublieBALToQueue(char *nom_bal, unsigned long nb_messages_en_attente_max, mbx2slot_wrapper * psMbx2slot)
{
	BOOL                fSuccess = FALSE;
	BOOL                fTasks = FALSE;
	noyau_enum_retour   eNoyRet;

	__try
	{
		strncpy_s(psMbx2slot->szBalName, sizeof(psMbx2slot->szBalName), nom_bal, sizeof(psMbx2slot->szBalName) - 1);

		if (AlloueEvent(&psMbx2slot->hEvtTerminateMbx2Slot, NULL) != NOYAU_OK)
		{
			__leave;
		}

		// Cr-ation de la bal publique
		psMbx2slot->hBal = PublieBAL(psMbx2slot->szBalName, nb_messages_en_attente_max);
		if (psMbx2slot->hBal == NOYAU_ID_NOK)
		{

			__leave;
		}

		NOYAU_INIT_TACHE(
			psMbx2slot->tsTasks[0],                        // structure - mettre - jour
			TRUE,                                          // tache active
			THREAD_PRIORITY_NORMAL,                        // priorit- initiale
			0,                                             // taille de pile pr-allou-e
			(LPTHREAD_START_ROUTINE)(Mbx2SlotThread),      // fonction thread
			(PVOID)(psMbx2slot),                           // param-tre de tache
			NULL,                                          // valeur mise - jour par LanceTache()
			"");

		// Pas d'autre tache
		NOYAU_VIDE_TACHE(psMbx2slot->tsTasks[1]);

		// Lancement des taches 
		if ((eNoyRet = LanceTache(psMbx2slot->tsTasks)) != NOYAU_OK)
		{
			__leave;
		}

		fSuccess = TRUE;
	}
	__finally
	{
		if (!fSuccess)
		{
			if (fTasks)
				ArretTaches(psMbx2slot->tsTasks);

			if (psMbx2slot->hBal == NOYAU_ID_NOK)
				SupprimeBAL(psMbx2slot->szBalName);

			if (psMbx2slot->hEvtTerminateMbx2Slot != NULL)
				LibereEvent(&psMbx2slot->hEvtTerminateMbx2Slot);

		}

	}

	return psMbx2slot->pQObject;
}

PROTECTED noyau_enum_retour WINAPI SupprimeBALToQueue(mbx2slot_wrapper * psMbx2queue)
{
	noyau_enum_retour   eResult;

	eResult = ArretTaches(psMbx2queue->tsTasks);
	if (eResult == NOYAU_ARRET_TACHE_OK)
		eResult = CleanUpBeforeExit(psMbx2queue);

	return eResult;
}

PRIVATE noyau_enum_retour WINAPI CleanUpBeforeExit(mbx2slot_wrapper * psMbx2queue)
{
	noyau_enum_retour   eResult;

	eResult = SupprimeBAL(psMbx2queue->szBalName);

	LibereEvent(&psMbx2queue->hEvtTerminateMbx2Slot);

	return eResult;
}

PROTECTED DWORD WINAPI Mbx2SlotThread(void * pvParam)
{
	mbx2slot_wrapper *  psMbx2slot = (mbx2slot_wrapper *)pvParam;
	struct_neutre* psMsg;
	noyau_enum_retour   eResult;
	DWORD               dwResult;

	while (TRUE)
	{
		dwResult = WaitForMultipleObjects(1, &psMbx2slot->hEvtTerminateMbx2Slot, FALSE, 5);
		if (dwResult == (NOYAU_OBJET_0 + 0))
			break;

		eResult = Recoit(psMbx2slot->hBal, (struct_neutre**)&psMsg, 50);
		if (eResult == NOYAU_BAL_MESS)
		{
			SlotNewMessage(psMbx2slot->pQObject, psMsg);
		}
	}

	CleanUpBeforeExit(psMbx2slot);

	return 0;
}