#include <noyau.h>
#include <trc.h>
#include <reg.h>
#include <InstTracer.h>
#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <csr_tlm.h>

#define LOC_DEF
#include <DmvGlobal.h>
#undef LOC_DEF

#include <DmvAni.h>

#include <memclass.h>
#include "InstTracer.h"
#include "DmvWorker.h"
#include "MQApplication.h"

// DLL entry point
EXPORT BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk);

PRIVATE enum_instance_result SetCharConversionArray(short int inst_id, char * pcKey);

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

EXPORT enum_instance_result WINAPI AFFLance(char * pcKey, char * pcBalNam, noyau_bal_id * piBalId)
{
	dmv_inst_id				inst_id;
	noyau_enum_retour		cr_lance;
	long					line;
	short int				index;
	DWORD					dwLen;
	DWORD					dwPrioMax;							
	DWORD					dwPrioInitMax;						
	char					pcPoolName[MAX_PATH + 1];			
	char					pcThreadName[MAX_PATH];
	DWORD					dwTraceSizeMB = 1;

	DebutRegion();

	if (NB_INSTANCES >= INSTANCE_MAX)
	{
		FinRegion();
		return INST_INIT_ERR_MAX_INSTANCE;
	}

	inst_id = NB_INSTANCES++; // updating global instances counter 

	FinRegion();

	memset(&gsDMV[inst_id], 0, sizeof(struct_global));
	
	MInstTracer_InitTrace(pcBalNam, inst_id, 1);

	if (strlen(pcBalNam) > MAX_PATH)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Mbox name too long : %s", pcBalNam);
		return INST_INIT_ERR_LANCE;
	}

	// For format file
	strcpy_s(gsDMV[inst_id].pcKey, sizeof(gsDMV[inst_id].pcKey), pcKey);

	for (index = 0; index < NB_MAX_AFF_PRIO; index++)
		gsDMV[inst_id].sScreen.tab_request[index].bal = NO_BAL;

	// Pool
	dwLen = sizeof(pcPoolName);
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolName, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_POOL);
		return INST_INIT_ERR_REGISTRE;
	}

	// Priorité initiale max
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &dwPrioInitMax) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);

	// Priorité max
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &dwPrioMax) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority(dwPrioMax);

	dwLen = MAX_PATH;
	//Device IP address
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_DEVICE_ADDRESS, gsDMV[inst_id].sInitStructure.szDeviceIPAddress, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_DEVICE_ADDRESS);
		return INST_INIT_ERR_REGISTRE;
	}

	//Device Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_DEVICE_PORT, &gsDMV[inst_id].sInitStructure.dwDeviceIPPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_DEVICE_PORT);
		return INST_INIT_ERR_REGISTRE;
	}

	//Device Echo Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_DEVICE_ECHO_PORT, &gsDMV[inst_id].sInitStructure.dwDeviceEchoPort) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, setting to default 7", pcKey, AFF_REG_KEYv_DEVICE_ECHO_PORT);
		gsDMV[inst_id].sInitStructure.dwDeviceEchoPort = 7;
	}

	//Local IP address
	dwLen = MAX_PATH;
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_LOCAL_ADDRESS, gsDMV[inst_id].sInitStructure.szLocalIPAddress, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_LOCAL_ADDRESS);
		return INST_INIT_ERR_REGISTRE;
	}

	//Local Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_LOCAL_PORT, &gsDMV[inst_id].sInitStructure.dwLocalIPPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_LOCAL_PORT);
		return INST_INIT_ERR_REGISTRE;
	}

	//Local Echo Port
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_LOCAL_ECHO_PORT, &gsDMV[inst_id].sInitStructure.dwLocalEchoPort) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_LOCAL_ECHO_PORT);
		return INST_INIT_ERR_REGISTRE;
	}

	//Format file
	dwLen = MAX_PATH;
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_FORMAT_FILE, gsDMV[inst_id].sInitStructure.szFormatFile, &dwLen) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_FORMAT_FILE);
		return INST_INIT_ERR_REGISTRE;
	}

	//Device Type
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_DEVICE_TYPE, &gsDMV[inst_id].sInitStructure.dwDeviceType) != ERROR_SUCCESS)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s", pcKey, AFF_REG_KEYv_DEVICE_TYPE);
		return INST_INIT_ERR_REGISTRE;
	}

	//Font
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_FONT, &gsDMV[inst_id].sInitStructure.dwFont) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default font : LATIN", pcKey, AFF_REG_KEYv_FONT);
		gsDMV[inst_id].sInitStructure.dwFont = 1;
	}

	//Font Color
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_FONT_COLOR, &gsDMV[inst_id].sInitStructure.dwFontColor) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default font color : Red", pcKey, AFF_REG_KEYv_FONT_COLOR);
		gsDMV[inst_id].sInitStructure.dwFontColor = 1;
	}

	//Alignment
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_ALIGNMENT, &gsDMV[inst_id].sInitStructure.dwAlignment) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : Left Aligned", pcKey, AFF_REG_KEYv_ALIGNMENT);
		gsDMV[inst_id].sInitStructure.dwAlignment = 1;
	}

	//Spacing
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_MAX_SPACING, &gsDMV[inst_id].sInitStructure.dwMaxSpacing) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 3", pcKey, AFF_REG_KEYv_MAX_SPACING);
		gsDMV[inst_id].sInitStructure.dwMaxSpacing = 3;
	}

	//Brightness
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_BRIGHTNESS, &gsDMV[inst_id].sInitStructure.dwBrightness) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 100", pcKey, AFF_REG_KEYv_BRIGHTNESS);
		gsDMV[inst_id].sInitStructure.dwBrightness = 100;
	}

	//Text XL Coordinate
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_TEXT_XL_COORDINATE, &gsDMV[inst_id].sInitStructure.dwTextXLCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 0x00", pcKey, AFF_REG_KEYv_TEXT_XL_COORDINATE);
		gsDMV[inst_id].sInitStructure.dwTextXLCoordinate = 0x00;
	}

	//Text XR Coordinate
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_TEXT_XR_COORDINATE, &gsDMV[inst_id].sInitStructure.dwTextXRCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 0x3F", pcKey, AFF_REG_KEYv_TEXT_XR_COORDINATE);
		gsDMV[inst_id].sInitStructure.dwTextXRCoordinate = 0x3F;
	}

	//Text Y Coordinate
	dwLen = MAX_PATH;
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_TEXT_Y_COORDINATE, &gsDMV[inst_id].sInitStructure.dwTextYCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 0x00", pcKey, AFF_REG_KEYv_TEXT_Y_COORDINATE);
		gsDMV[inst_id].sInitStructure.dwTextYCoordinate = 0x00;
	}

	//Image X Coordinate
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_IMAGE_X_COORDINATE, &gsDMV[inst_id].sInitStructure.dwImageXCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, Setting default : 0x40", pcKey, AFF_REG_KEYv_IMAGE_X_COORDINATE);
		gsDMV[inst_id].sInitStructure.dwImageXCoordinate = 0x40;
	}

	//Text Y Coordinate
	dwLen = MAX_PATH;
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_IMAGE_Y_COORDINATE, &gsDMV[inst_id].sInitStructure.dwImageYCoordinate) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** TLMLance() => Registry error : [%s]:%s, Setting default : 0x10", pcKey, AFF_REG_KEYv_IMAGE_Y_COORDINATE);
		gsDMV[inst_id].sInitStructure.dwImageYCoordinate = 0x10;
	}


	// Response timeout
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_RESPONSE_TIMEOUT, &gsDMV[inst_id].sInitStructure.dwWaitForResponseTimeout) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, Setting default : 1 second", pcKey, AFF_REG_KEYv_RESPONSE_TIMEOUT);
		gsDMV[inst_id].sInitStructure.dwWaitForResponseTimeout = 2500;
	}

	// Status reception timeout
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_STATUS_RESPONSE_TO, &gsDMV[inst_id].sInitStructure.dwWaitForStatusTimeout) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, set to 100 by default", pcKey, AFF_REG_KEYv_STATUS_RESPONSE_TO);
		gsDMV[inst_id].sInitStructure.dwWaitForStatusTimeout = 100;
	}

	// Is TLM service integrated
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_IS_TLM_INTEGRATED, &gsDMV[inst_id].sInitStructure.dwIsTlmServiceIntegrated) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s, set to 1 by default", pcKey, AFF_REG_KEYv_IS_TLM_INTEGRATED);
		gsDMV[inst_id].sInitStructure.dwIsTlmServiceIntegrated = 1;
	}

	//Trace Max Size (MB)
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, AFF_REG_KEYv_TRACE_MAX_SIZE_MB, &dwTraceSizeMB) != ERROR_SUCCESS)
	{
		MInstTracer_FichierTrace(inst_id, "INIT ***** AFFLance() => Registry error : [%s]:%s , set to 1 as default", pcKey, AFF_REG_KEYv_TRACE_MAX_SIZE_MB);
	}
	else
	{
		//Reinit trace
		MInstTracer_InitTrace(pcBalNam, inst_id, dwTraceSizeMB);
	}

	if (SetCharConversionArray(inst_id, pcKey) == INST_INIT_ERR_REGISTRE)
	{
		MInstTrtacer_FichierError(inst_id, "INIT ***** AffLance() => Error with characters conversion table");
		return INST_INIT_ERR_REGISTRE;
	}

	sprintf_s(pcThreadName, sizeof(pcThreadName), "ANI (%s)", pcBalNam);
	NOYAU_INIT_TACHE(gsDMV[inst_id].threads[0],
					 TRUE,
					 dwPrioInitMax,
					 2048,
					 (LPTHREAD_START_ROUTINE)(Ani),
					 (PVOID)(inst_id),
					 NULL,
					 pcThreadName);

	gsDMV[inst_id].sInitStructure.pfInitWorker = (InitWorker *)InitDmvWorker;
	gsDMV[inst_id].sInitStructure.pfDeinitWorker = (DeinitWorker *)DeinitDmvWorker;
	gsDMV[inst_id].sInitStructure.pGlobalStructure = &gsDMV[inst_id];
	gsDMV[inst_id].sInitStructure.siInstId = inst_id;

	sprintf_s(pcThreadName, sizeof(pcThreadName), "IOS(QT) (%s)", pcBalNam);

	NOYAU_INIT_TACHE(gsDMV[inst_id].threads[1],
					 (noyau_enum_booleen)TRUE,
					 dwPrioInitMax,
					 2048,
					 (LPTHREAD_START_ROUTINE)(MQEventLoop_LoopThread),
					 (PVOID)&(gsDMV[inst_id].sInitStructure),
					 NULL,
					 pcThreadName);

	NOYAU_VIDE_TACHE(gsDMV[inst_id].threads[NB_THREADS]);

	gsDMV[inst_id].priority_max = dwPrioMax;

	gsDMV[inst_id].pool = NOYAU_GetPoolId(pcPoolName);

	sprintf_s(gsDMV[inst_id].ios_mbox_name,
			  sizeof(gsDMV[inst_id].ios_mbox_name),
			  "iBAL_AFF_IOS_%d_%d",
			  inst_id,
			  GetCurrentProcessId());

	strcpy_s(gsDMV[inst_id].ani_mbox_name, sizeof(gsDMV[inst_id].ani_mbox_name), pcBalNam);
	gsDMV[inst_id].position = 1;

	if (AFF_ReadFile(gsDMV[inst_id].sInitStructure.szFormatFile,
					 gsDMV[inst_id].LabelTable,
					 NB_MAX_LABEL,
					 &line,
					 &gsDMV[inst_id].position) == FALSE)
	{
		FreeTabLabel(gsDMV[inst_id].LabelTable, NB_MAX_LABEL);
		MInstTrtacer_FichierError(inst_id, "AFF_INIT ***** AffLance() => Error with file %s, line %ld *****",
								  gsDMV[inst_id].sInitStructure.szFormatFile,
								  line);
		return(INST_INIT_ERR_FICHIER_PARAM);
	}

	if ((gsDMV[inst_id].service[M_SRV_ETAT] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_SRV_ESPION] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_SRV_ARRET] = SrvLance(1)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_AFF_AFFICHAGE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_AFF_VISU] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_TLM_MESSENGER_SERVICE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if ((gsDMV[inst_id].service[M_TLM_FLASHER_SERVICE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	cr_lance = LanceTache(gsDMV[inst_id].threads);
	if (cr_lance != NOYAU_OK)
	{
		return INST_INIT_ERR_LANCE;
	}

	*piBalId = AttendBAL(pcBalNam);
	if (*piBalId <= 0)
	{
		MInstTrtacer_FichierError(inst_id, "AFFLance: Could not connect to mailbox");
		return INST_INIT_ERR_LANCE;
	}

	MInstTracer_FichierTrace(inst_id, "INIT: AFFLance() => init OK ");
	
	return INST_INIT_OK;
}

EXPORT enum_instance_result WINAPI MODLance(char * pcKey, char * pcBalName, noyau_bal_id * piBalId)
{
	return AFFLance(pcKey, pcBalName, piBalId);
}

EXPORT enum_instance_result WINAPI AFFArret(noyau_bal_id iBalId)
{
	dmv_inst_id				inst_id;
	noyau_enum_retour		cr_arret;

	// recherche en fonction du nombre de lecteur lance 
	for (inst_id = 0; inst_id < INSTANCE_MAX; inst_id++)
	{
		if (gsDMV[inst_id].ani_mbox == iBalId)
			break;
	}

	if (inst_id > INSTANCE_MAX)
		return  INST_ARRET_NOK;

	MQEventLoop_Quit(gsDMV[inst_id].sInitStructure.m_iLoopInstance);

	cr_arret = ArretTaches(gsDMV[inst_id].threads);

	// si l'arret s'est mal passe 
	if (cr_arret == NOYAU_ARRET_TACHE_NOK)
		return INST_ARRET_NOK;


	// suprresion des BAL dans la registry  
	SupprimeBAL(gsDMV[inst_id].ani_mbox_name);
	SupprimeBAL(gsDMV[inst_id].ios_mbox_name);

	// liberer les services
	if (SrvArret(&gsDMV[inst_id].service[M_SRV_ETAT]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_SRV_ESPION]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_SRV_ARRET]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_AFF_AFFICHAGE]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_AFF_VISU]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_TLM_MESSENGER_SERVICE]) == FALSE)
		ExitBad();

	if (SrvArret(&gsDMV[inst_id].service[M_TLM_FLASHER_SERVICE]) == FALSE)
		ExitBad();

	DebutRegion();

	if (NB_INSTANCES > 0)
		NB_INSTANCES--;

	FinRegion();

	// Arret du mode DEBUG 
	MInstTracer_DeinitTrace(inst_id);

	// retourner la valeur de l'arret des taches 
	return INST_ARRET_OK;
}

EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
	return AFFArret(iBalId);
}

PRIVATE enum_instance_result SetCharConversionArray(short int inst_id, char * pcKey)
{


	char SubKey[MAX_PATH];
	char KeyFound[MAX_PATH];
	char Valeur[MAX_PATH];
	DWORD result;
	DWORD Index;
	DWORD KeySize, SizeVal;
	int nAsciFrom;
	int nAsciTo;

	int n;

	// initialize conversion_array to default valuees

	for (n = 0; n <= 255; n++)
	{
		gsDMV[inst_id].char_conv_array[n] = n;
		gsDMV[inst_id].inv_char_conv_array[n] = n;
	}

	// generate subkey
	sprintf_s(SubKey, sizeof(SubKey), "%s\\%s", pcKey, AFF_REG_KEYv_CHARSET_CONVERSION);

	Index = 0;

	while (TRUE){

		SizeVal = sizeof(Valeur);
		KeySize = sizeof(KeyFound);

		result = REG_Enum_Valeurs_Chaine(CSR_REG_KEYi_ROOT, SubKey, Index, KeyFound, &KeySize, Valeur, &SizeVal);


		if (result != ERROR_SUCCESS)break;

		/* Le periph est inactif passe au suivant */
		if (!KeySize){
			Index++;
			continue;
		}

		nAsciFrom = atoi(KeyFound);
		nAsciTo = atoi(Valeur);

		if (nAsciFrom > 0 && nAsciFrom < 256 && nAsciTo>0 && nAsciTo < 256)
		{
			gsDMV[inst_id].char_conv_array[nAsciFrom] = nAsciTo;
		}
		else
		{
			MInstTrtacer_FichierError(inst_id, "AFF_INIT ***** SetCharConversionArray() => Bad character conversion table");
			return INST_INIT_ERR_REGISTRE;

		}

		Index++;
	}

	// building inverse charset conversion array
	for (n = 0; n <= 255; n++)
	{
		gsDMV[inst_id].inv_char_conv_array[gsDMV[inst_id].char_conv_array[n]] = n;
	}


	return INST_INIT_OK;
}