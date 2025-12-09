// Standard CSRoute definitions
#include <csrlc32.h>

// Others software components includes files
#include <noyau.h>
#include <debug.h>
#include <reg.h>
#include <run.h>
#include <trc.h>
#include <module.h>
#include <csr_srv.h>
#include <InstTracer.h>
#include <csr_tlm.h>
#define LOC_DEF
#include <TlmDmvGlobal.h>
#undef LOC_DEF

#include <TlmDmvAni.h>
#include <TlmDmvWorker.h>

#include <memclass.h>

#include "MQApplication.h"

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- VARIABLES:---------------*/

/*--------------- FUNCTIONS: ---------------*/

PRIVATE void  InitRegionModule(IN tlm_inst_id inst_id);
PRIVATE void  DeleteRegionModule(IN tlm_inst_id inst_id);

// Point d'entre standard DLL
EXPORT BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk);

/*--------------- CODE: ---------------*/

EXPORT BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk)
{
	BOOL bOK = TRUE;

	switch (dwWhy)
	{
		// When the process does the first LoadLibrary for this DLL
	case DLL_PROCESS_ATTACH:
		StartApplicationThread();
		break;

		// When the process makes the last FreeLibrary for that DLL
	case DLL_PROCESS_DETACH:
		//exit thread with QApplication exec...
		StopApplicationThread();
		break;
	}

	return bOK;
}

EXPORT enum_instance_result WINAPI TLMLance(char * pcKey,
											char * pcBalNam,
											noyau_bal_id * piBalId)
{
	tlm_inst_id			inst_id;
	noyau_enum_retour	cr_lance;
	DWORD				dwLen;
	DWORD				dwPrioMax;							// REG : Priorit max taches
	DWORD				dwPrioInitMax;						// REG : Priorit max taches initiale
	char				pcPoolName[MAX_PATH + 1];			// REG : Id de pool
	char				pcNomTache[MAX_PATH];
	DWORD				dwTraceSizeMB = 1;

	// Determiner le numro de l'instance
	DebutRegion();
	if (TLM_NB_INSTANCES >= TLM_INSTANCE_MAX)
	{
		FinRegion();
		return INST_INIT_ERR_MAX_INSTANCE;
	}
	inst_id = TLM_NB_INSTANCES++; // updating global instances counter 

	memset(&gsTLM[inst_id], 0, sizeof(struct_global));

	InitRegionModule(inst_id);

	FinRegion();

	if (!MInstTracer_InitTrace(pcBalNam, inst_id, 1))
		return INST_INIT_ERR_FICHIER_DEBUG;

	if (strlen(pcBalNam) > MAX_PATH)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Mbox name too long : %s", pcBalNam);
		return INST_INIT_ERR_LANCE;
	}

	strcpy_s(gsTLM[inst_id].pcKey, sizeof(gsTLM[inst_id].pcKey), pcKey);

	dwLen = sizeof(pcPoolName);
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolName, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_POOL);
		return INST_INIT_ERR_REGISTRE;
	}

	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &dwPrioInitMax) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);

	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &dwPrioMax) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority(dwPrioMax);


	//Other params

	dwLen = MAX_PATH;
	//Device IP address
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_DEVICE_ADDRESS, gsTLM[inst_id].sInitStructure.szDeviceIPAddress, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_DEVICE_ADDRESS);
		return INST_INIT_ERR_REGISTRE;
	}

	//Device Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_DEVICE_PORT, &gsTLM[inst_id].sInitStructure.dwDeviceIPPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_DEVICE_PORT);
		return INST_INIT_ERR_REGISTRE;
	}

	//Device Echo Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_DEVICE_ECHO_PORT, &gsTLM[inst_id].sInitStructure.dwDeviceEchoPort) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, setting to default 7", pcKey, TLM_REG_KEYv_DEVICE_ECHO_PORT);
		gsTLM[inst_id].sInitStructure.dwDeviceEchoPort = 7;
	}

	//Local IP address
	dwLen = MAX_PATH;
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_LOCAL_ADDRESS, gsTLM[inst_id].sInitStructure.szLocalIPAddress, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_LOCAL_ADDRESS);
		return INST_INIT_ERR_REGISTRE;
	}

	//Local Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_LOCAL_PORT, &gsTLM[inst_id].sInitStructure.dwLocalIPPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_LOCAL_PORT);
		return INST_INIT_ERR_REGISTRE;
	}

	//Local Echo Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_LOCAL_ECHO_PORT, &gsTLM[inst_id].sInitStructure.dwLocalEchoPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_LOCAL_ECHO_PORT);
		return INST_INIT_ERR_REGISTRE;
	}


	//Device Type
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_DEVICE_TYPE, &gsTLM[inst_id].sInitStructure.dwDeviceType) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s", pcKey, TLM_REG_KEYv_DEVICE_TYPE);
		return INST_INIT_ERR_REGISTRE;
	}

	// MaxLinkError
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_MAX_LINK_ERROR, &gsTLM[inst_id].sInitStructure.MaxLinkError) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, Setting default value to 3", pcKey, TLM_REG_KEYv_MAX_LINK_ERROR);
		gsTLM[inst_id].sInitStructure.MaxLinkError = 3;
	}

	// Response timeout
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_RESPONSE_TO, &gsTLM[inst_id].sInitStructure.dwWaitForResponseTimeout) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, set to 1000 by default", pcKey, TLM_REG_KEYv_RESPONSE_TO);
		gsTLM[inst_id].sInitStructure.dwWaitForResponseTimeout = 2500;
	}

	// Status reception timeout
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_STATUS_RESPONSE_TO, &gsTLM[inst_id].sInitStructure.dwWaitForStatusTimeout) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, set to 100 by default", pcKey, TLM_REG_KEYv_STATUS_RESPONSE_TO);
		gsTLM[inst_id].sInitStructure.dwWaitForStatusTimeout = 100;
	}

	//Image X Coordinate
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_IMAGE_X_COORDINATE, &gsTLM[inst_id].sInitStructure.dwImageXCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, Setting default : 0x40", pcKey, TLM_REG_KEYv_IMAGE_X_COORDINATE);
		gsTLM[inst_id].sInitStructure.dwImageXCoordinate = 0x40;
	}

	//Text Y Coordinate
	dwLen = MAX_PATH;
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_IMAGE_Y_COORDINATE, &gsTLM[inst_id].sInitStructure.dwImageYCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, Setting default : 0x10", pcKey, TLM_REG_KEYv_IMAGE_Y_COORDINATE);
		gsTLM[inst_id].sInitStructure.dwImageYCoordinate = 0x10;
	}

	//Trace Max Size (MB)
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, TLM_REG_KEYv_TRACE_MAX_SIZE_MB, &dwTraceSizeMB) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s , set to 1 as default", pcKey, TLM_REG_KEYv_TRACE_MAX_SIZE_MB);
	}
	else
	{
		//Reinit trace
		MInstTracer_InitTrace(pcBalNam, inst_id, dwTraceSizeMB);
	}

	sprintf_s(pcNomTache, sizeof(pcNomTache), "ANI (%s)", pcBalNam);
	NOYAU_INIT_TACHE(gsTLM[inst_id].taches[0],
					 TRUE,
					 dwPrioInitMax,
					 2048,
					 (LPTHREAD_START_ROUTINE)(Ani),
					 (PVOID)(inst_id),
					 NULL,
					 pcNomTache);

	strcpy_s(gsTLM[inst_id].nom_bal_ani, sizeof(gsTLM[inst_id].nom_bal_ani), pcBalNam);

	gsTLM[inst_id].sInitStructure.pfInitWorker = (InitWorker *)InitTlmWorker;
	gsTLM[inst_id].sInitStructure.pfDeinitWorker = (DeinitWorker *)DeinitTlmWorker;
	gsTLM[inst_id].sInitStructure.pGlobalStructure = &gsTLM[inst_id];
	gsTLM[inst_id].sInitStructure.siInstId = inst_id;

	sprintf_s(pcNomTache, sizeof(pcNomTache), "IOS(QT) (%s)", pcBalNam);
	NOYAU_INIT_TACHE(gsTLM[inst_id].taches[1],
					 (noyau_enum_booleen)TRUE,
					 dwPrioInitMax,
					 2048,
					 (LPTHREAD_START_ROUTINE)(MQEventLoop_LoopThread),
					 (PVOID)&(gsTLM[inst_id].sInitStructure),
					 NULL,
					 pcNomTache);

	NOYAU_VIDE_TACHE(gsTLM[inst_id].taches[2]);

	gsTLM[inst_id].priorite_max = dwPrioMax;

	gsTLM[inst_id].pool = NOYAU_GetPoolId(pcPoolName);

	sprintf_s(gsTLM[inst_id].nom_bal_ios,
			  sizeof(gsTLM[inst_id].nom_bal_ios),
			  "iBAL_TLM_IOS_%d_%d",
			  inst_id,
			  GetCurrentProcessId());

	if ((gsTLM[inst_id].service[M_TLM_MESSENGER_SERVICE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	if ((gsTLM[inst_id].service[M_SRV_ETAT] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	if ((gsTLM[inst_id].service[M_SRV_ESPION] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	if ((gsTLM[inst_id].service[M_SRV_ARRET] = SrvLance(1)) == NULL)
		ExitBad();

	cr_lance = LanceTache(gsTLM[inst_id].taches);
	if (cr_lance != NOYAU_OK)
	{
		return INST_INIT_ERR_LANCE;
	}

	*piBalId = AttendBAL(gsTLM[inst_id].nom_bal_ani);
	if (*piBalId <= 0)
	{
		MInstTrtacer_FichierError(inst_id, "TLMLance: Pb InitBal() pour TLM");
		return INST_INIT_ERR_LANCE;
	}

	// redirection trace 
	MInstTracer_FichierTrace(inst_id, "INIT: TLMLance() => init OK ");

	return INST_INIT_OK;
}

EXPORT enum_instance_result WINAPI MODLance(char *pcKey, char *pcBalName, noyau_bal_id *piBalId)
{
	return TLMLance(pcKey, pcBalName, piBalId);
}

EXPORT enum_instance_result WINAPI TLMArret(noyau_bal_id	mailbox_id)
{
	tlm_inst_id		inst_id;
	noyau_enum_retour	cr_arret;

	for (inst_id = 0; inst_id < TLM_INSTANCE_MAX; inst_id++)
	{
		if (gsTLM[inst_id].ani_bal == mailbox_id)
			break;
	}

	if (inst_id > TLM_INSTANCE_MAX)
		return  INST_ARRET_NOK;

	cr_arret = ArretTaches(gsTLM[inst_id].taches);

	if (cr_arret == NOYAU_ARRET_TACHE_NOK)
		return INST_ARRET_NOK;

	SupprimeBAL(gsTLM[inst_id].nom_bal_ani);
	SupprimeBAL(gsTLM[inst_id].nom_bal_ios);

	if (SrvArret(&gsTLM[inst_id].service[M_SRV_ETAT]) == FALSE)
		ExitBad();

	if (SrvArret(&gsTLM[inst_id].service[M_SRV_ESPION]) == FALSE)
		ExitBad();

	if (SrvArret(&gsTLM[inst_id].service[M_SRV_ARRET]) == FALSE)
		ExitBad();

	DebutRegion();

	if (TLM_NB_INSTANCES > 0)

		TLM_NB_INSTANCES--;
	DeleteRegionModule(inst_id);

	FinRegion();

	MInstTracer_DeinitTrace(inst_id);

	return INST_ARRET_OK;
}

EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
	return TLMArret(iBalId);
}

PRIVATE void  InitRegionModule(IN tlm_inst_id inst_id)
{
	InitializeCriticalSection(&gsTLM[inst_id].criticalSectionInstance);
}

PRIVATE void  DeleteRegionModule(IN tlm_inst_id inst_id)
{
	DeleteCriticalSection(&gsTLM[inst_id].criticalSectionInstance);
}

PROTECTED void DebutRegionModule(IN tlm_inst_id inst_id)
{
	EnterCriticalSection(&gsTLM[inst_id].criticalSectionInstance);
}

PROTECTED void FinRegionModule(IN tlm_inst_id inst_id)
{
	LeaveCriticalSection(&gsTLM[inst_id].criticalSectionInstance);
}
