/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_init.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions for the module  starting,    */
/*             stopping and initialization.                                  */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <wingdi.h>

// Module interface
#define LOC_DEF
#include <emi_pic_glob.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define MAX_TRACE_LINE_LEN 1024
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE VOID FreeTimers( short siInstId );
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: BOOL WINAPI DllMain(HINSTANCE hInst,                               */
/*                            DWORD dwWhy,                                   */
/*                            LPVOID pvJunk)                                 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            DLL entry point.                                               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  HINSTANCE hInst   - A handle to the DLL.                         */
/*      IN  DWORD dwWhy       - Specifies a flag indicating why the DLL      */
/*                              entry-point function is being called.        */
/*      IN  LPVOID pvJunk     - Specifies further aspects of DLL             */
/*                              initialization and cleanup.                  */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  Initialization succeeds.                           */
/*  FALSE                 Initialization fails.                              */
/*****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hInst,
                    DWORD dwWhy,
                    LPVOID pvJunk)
{
	BOOL bOK = TRUE;

	switch(dwWhy)
	{
		//  LoadLibrary
		case DLL_PROCESS_ATTACH :
			break;

		// FreeLibrary
		case DLL_PROCESS_DETACH :
			break;
	}

	return bOK;
}




/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PRIVATE BOOL InitTimers( IN short siInstId, IN LPSTR szKey )
{
	INT
		iIndex;
	struct_chrono_data
		sChronoData[NB_CHRONOS] = 
	{
		{ 0, "Emission period",		EMI_PIC_REG_KEYv_EMI_PERIOD			},
		{ 0, "Import period",		EMI_PIC_REG_KEYv_IMPORT_PERIOD		},
		{ 0, "Reemission period",	EMI_PIC_REG_KEYv_REEMISSION_PERIOD	},
	};

	memcpy( &(EMI_PIC[siInstId].sChronoData), &sChronoData, sizeof(struct_chrono_data)*NB_CHRONOS );

	for( iIndex = FIRST_CHRONO; iIndex < END_CHRONO; iIndex++ )
	{
		if( AlloueChrono(	&(EMI_PIC[siInstId].sChronoData[iIndex].chrono_id),
							EMI_PIC[siInstId].sChronoData[iIndex].szChronoName ) != NOYAU_OK )
		{
			return FALSE;
		}

		// Emission period
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
			szKey, 
			EMI_PIC[siInstId].sChronoData[iIndex].szRegKey, 
			&(EMI_PIC[siInstId].sChronoData[iIndex].chrono_dalay) ) != ERROR_SUCCESS)
		{
			EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", szKey, EMI_PIC[siInstId].sChronoData[iIndex].szRegKey );
			return FALSE;
		}
		
		EMI_PIC[siInstId].sChronoData[iIndex].chrono_dalay /= 55;
	}

	return TRUE;
}
/**/
/*****************************************************************************/
/*SYNTAX: enum_instance_result WINAPI EmiPicLance(char *pcKey,               */
/*											      char *pcBalNam,            */
/*											      noyau_bal_id *piBalId)     */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The module entry point.                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  char *pcKey            - Name of the registry key that contains  */
/*                                   module specific parameters.             */
/*      IN  char *pcBalNam         - Mailbox name.                           */
/*      OUT noyau_bal_id *piBalId  - Mailbox ID.                             */
/*===========================================================================*/
/*  Return                       Description                                 */
/*---------------------------------------------------------------------------*/
/*  INST_INIT_OK                 The module is started.                      */
/*  INST_INIT_ERR_MAX_INSTANCE   The number of instances exceeded.           */
/*  INST_INIT_ERR_FICHIER_DEBUG  Error initializing the module trace.        */
/*  INST_INIT_ERR_LANCE          Error starting threads or allocating        */
/*                               memory.                                     */
/*  INST_INIT_ERR_REGISTRE       Error reading the registry.                 */
/*****************************************************************************/
EXPORT enum_instance_result WINAPI EmiPicLance(char *pcKey,
                                               char *pcBalNam, 
                                               noyau_bal_id *piBalId)
{
	short
		siInstId;

	noyau_enum_retour
		eResult;

	DWORD
		dwLen,
//		dwNum,
		dwPrioMax,
		dwPrioInitMax,
		dwUsingCounter,
		dwPurgeFileDelay;

	CHAR
		pcPoolId[MAX_PATH + 1],
		pcNomTache[MAX_PATH + 1],
		szValue[MAX_PATH + 1],
		szBuffer[MAX_PATH],
		szTemp[MAX_PATH];

//	LPSTR
//		pBegin,
//		pEnd;

	DWORD
		dwDay		= 0,
		dwMonth		= 0,
		dwYear		= 0,
		dwHour		= 0,
		dwMinute	= 0;

	// Update instance identifier (protected in critical section)
	DebutRegion();
	if(EMI_PIC_NB_INSTANCES >= EMI_PIC_INSTANCE_MAX)
	{
		FinRegion();
		return INST_INIT_ERR_MAX_INSTANCE;
	}
	
	// Updating global instances counter 
    siInstId = EMI_PIC_NB_INSTANCES++;
	FinRegion();

	// Clear public structure
	memset(&EMI_PIC[siInstId], 0, sizeof(struct_emi_pic_public));

	strcpy_s(EMI_PIC[siInstId].szKey,sizeof(EMI_PIC[siInstId].szKey), pcKey);

	// Trace mode initialisation
	if(EmiPicInitTrace(pcBalNam, siInstId) != INST_INIT_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;

	// Verify length of the mailbox name
	if(strlen(pcBalNam) > MAX_PATH)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> MailBox name is too long : %s *****", pcBalNam);
		return INST_INIT_ERR_LANCE;
	}
	
	// Reading registry entries
	// Pool (reserved) - for future usage
	dwLen = sizeof(pcPoolId);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   MOD_REG_KEYv_POOL, 
					   pcPoolId, 
					   &dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, MOD_REG_KEYv_POOL);
		return INST_INIT_ERR_REGISTRE;
	}

	// Initialisation priority
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   MOD_REG_KEYv_PRIO_INIT, 
					   &dwPrioInitMax) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, MOD_REG_KEYv_PRIO_INIT);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);

	// Main thread priority
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   MOD_REG_KEYv_PRIO_MAX, 
					   &dwPrioMax) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, MOD_REG_KEYv_PRIO_MAX);
		return INST_INIT_ERR_REGISTRE;
	}
	dwPrioMax = NOYAU_MapPriority(dwPrioMax);

	// Directory that contains non-transfered picture files
	dwLen = sizeof( szBuffer );
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_LOCAL_DIR, 
					   szBuffer, 
					   &dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_LOCAL_DIR);
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		strcpy_s( EMI_PIC[siInstId].ConfigLocal.szDestinationDir,sizeof(EMI_PIC[siInstId].ConfigLocal.szDestinationDir), szBuffer );
	}

	// Directory that contains backup picture files
	dwLen = sizeof(szBuffer);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_BACKUP_DIR, 
					   szBuffer, 
					   &dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_BACKUP_DIR);
		return INST_INIT_ERR_REGISTRE;
	}
	else
		strcpy_s( EMI_PIC[siInstId].ConfigBackup.szDestinationDir,sizeof(EMI_PIC[siInstId].ConfigBackup.szDestinationDir), szBuffer );

	// Directory that contains files to import
	dwLen = sizeof(EMI_PIC[siInstId].szImportDir);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_IMPORT_DIR, 
					   EMI_PIC[siInstId].szImportDir, 
					   &dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_IMPORT_DIR );
		return INST_INIT_ERR_REGISTRE;
	}	

	// File name format string
	dwLen = sizeof(szBuffer);
	if( REG_Lire_Chaine(	CSR_REG_KEYi_ROOT,
							pcKey,
							EMI_PIC_REG_KEYv_FILENAME_FORMAT,
							szBuffer,
							&dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug( siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_FILENAME_FORMAT );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if( GetFormatConfiguration( szBuffer, &(EMI_PIC[siInstId].sFileNameFormat) ) != TRUE )
		{
			EmiPicFichierDebug( siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s, invalid data *****", pcKey, EMI_PIC_REG_KEYv_FILENAME_FORMAT );
			return INST_INIT_ERR_REGISTRE;
		}
		else
			EmiPicFichierTrace( siInstId,
			"EMI_PIC_INIT::GetFormatConfiguration:sFileNameFormat szFormatString[%s]",
			escapeCStringForSPrintf(EMI_PIC[siInstId].sFileNameFormat.szFormatString, szTemp, sizeof(szTemp)));
	}

	// Local store path configuration
	dwLen = sizeof(szBuffer);
	if( REG_Lire_Chaine(	CSR_REG_KEYi_ROOT,
							pcKey,
							EMI_PIC_REG_KEYv_LOCAL_FORMAT,
							szBuffer,
							&dwLen) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug( siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_LOCAL_FORMAT );
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if( GetFormatConfiguration( szBuffer, &(EMI_PIC[siInstId].ConfigLocal.sFormatData) ) != TRUE )
		{
			EmiPicFichierDebug( siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s, invalid data *****", pcKey, EMI_PIC_REG_KEYv_LOCAL_FORMAT );
			return INST_INIT_ERR_REGISTRE;
		}
		else
		{// Same format is used for backup
			memcpy( &(EMI_PIC[siInstId].ConfigBackup.sFormatData), &(EMI_PIC[siInstId].ConfigLocal.sFormatData), sizeof(struct_format_data) );

			EmiPicFichierTrace( siInstId,
						"EMI_PIC_INIT::GetFormatConfiguration:ConfigLocal.sFormatData szFormatString[%s]",
						escapeCStringForSPrintf(EMI_PIC[siInstId].ConfigLocal.sFormatData.szFormatString, szTemp, sizeof(szTemp)));
		}
	}

	// Using transmission counters
	if( REG_Lire_Entier( CSR_REG_KEYi_ROOT,
						 pcKey, 
						 EMI_PIC_REG_KEYv_USE_COUNTER, 
						 &dwUsingCounter ) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s *****", pcKey, EMI_PIC_REG_KEYv_USE_COUNTER);
		return INST_INIT_ERR_REGISTRE;
	}
	( dwUsingCounter != 1 )?( EMI_PIC[siInstId].bUsingCounter = FALSE ):( EMI_PIC[siInstId].bUsingCounter = TRUE );


	// Last purge time
	dwLen = sizeof(szValue);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_LAST_PURGE_TIME, 
					   szValue, 
					   &dwLen) != ERROR_SUCCESS)
	{
		memset(&EMI_PIC[siInstId].sLastPurgeDTime, 0, sizeof(SYSTEMTIME));
	}
	else
	{
		if(dwLen ==0)
			memset(&EMI_PIC[siInstId].sLastPurgeDTime, 0, sizeof(SYSTEMTIME));
		else
		{
			if(sscanf_s(szValue,
					  "%d.%d.%d %d:%d",
					  &dwDay,
					  &dwMonth,
					  &dwYear,
					  &dwHour,
					  &dwMinute) != 5)
			{
				memset(&EMI_PIC[siInstId].sLastPurgeDTime, 0, sizeof(SYSTEMTIME));
			}
			else
			{
				EMI_PIC[siInstId].sLastPurgeDTime.wDay = (WORD)dwDay;
				EMI_PIC[siInstId].sLastPurgeDTime.wMonth = (WORD)dwMonth;
				EMI_PIC[siInstId].sLastPurgeDTime.wYear = (WORD)dwYear;
				EMI_PIC[siInstId].sLastPurgeDTime.wHour = (WORD)dwHour;
				EMI_PIC[siInstId].sLastPurgeDTime.wMinute = (WORD)dwMinute;
			}
		}
	}
	
	// Purge period
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_PURGE_PERIOD, 
					   &EMI_PIC[siInstId].dwPurgePeriod) != ERROR_SUCCESS)
	{
		EMI_PIC[siInstId].dwPurgePeriod = 0;
	}

	// Purge time
	dwLen = sizeof(szValue);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
					   pcKey, 
					   EMI_PIC_REG_KEYv_PURGE_TIME, 
					   szValue, 
					   &dwLen) != ERROR_SUCCESS)
	{
		EMI_PIC[siInstId].dwPurgeHour = 0;
		EMI_PIC[siInstId].dwPurgeMinute = 0;
	}
	else
	{
		if(dwLen ==0)
		{
			EMI_PIC[siInstId].dwPurgeHour = 0;
			EMI_PIC[siInstId].dwPurgeMinute = 0;
		}
		else
		{
			if(sscanf_s(szValue,
					  "%d:%d",
					  &EMI_PIC[siInstId].dwPurgeHour,
					  &EMI_PIC[siInstId].dwPurgeMinute) != 2)
			{
				EMI_PIC[siInstId].dwPurgeHour = 0;
				EMI_PIC[siInstId].dwPurgeMinute = 0;
			}
		}
	}

	// MFR start 2010/1/25
	// Purge, delete files delay
	if( REG_Lire_Entier( CSR_REG_KEYi_ROOT,
		pcKey, 
		EMI_PIC_REG_KEYv_PURGE_FILE_DELAY, 
		&dwPurgeFileDelay ) != ERROR_SUCCESS)
	{
		EmiPicFichierDebug( siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Erreur registre : [%s]:%s, setting default value 0 *****", pcKey, EMI_PIC_REG_KEYv_PURGE_FILE_DELAY );
		dwPurgeFileDelay = 0;
	}

	EMI_PIC[siInstId].dwPurgeFileDelay = dwPurgeFileDelay;
	// MFR end

	// Timer initialization
	if( InitTimers( siInstId, pcKey ) != TRUE )
	{
		return INST_INIT_ERR_LANCE;
	}

	// Init critical sections
	InitializeCriticalSection( &(EMI_PIC[siInstId].CS_Abort) );
	
	// Ani thread
    sprintf_s(pcNomTache,sizeof(pcNomTache), "EMI_PIC animation (%s)", pcBalNam);
    NOYAU_INIT_TACHE(
			EMI_PIC[siInstId].sTaches[EMI_PIC_ID_THREAD_ANI],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(EmiPicAni),
			(PVOID)(siInstId),
			NULL,
            pcNomTache);

		// Emi thread
    sprintf_s(pcNomTache,sizeof(pcNomTache), "EMI_PIC picture emission (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			EMI_PIC[siInstId].sTaches[EMI_PIC_ID_THREAD_EMI],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(EmiPicEmi),
			(PVOID)(siInstId),
			NULL,
            pcNomTache);

	// Restit thread
    sprintf_s(pcNomTache,sizeof(pcNomTache), "EMI_PIC restitution (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			EMI_PIC[siInstId].sTaches[EMI_PIC_ID_THREAD_RESTIT],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(EmiPicRestit),
			(PVOID)(siInstId),
			NULL,
            pcNomTache);

	// Trf thread
    sprintf_s(pcNomTache,sizeof(pcNomTache), "EMI_PIC transfer (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			EMI_PIC[siInstId].sTaches[EMI_PIC_ID_THREAD_TRF],
			TRUE,
			dwPrioInitMax,
			2048,
			(LPTHREAD_START_ROUTINE)(EmiPicTrf),
			(PVOID)(siInstId),
			NULL,
            pcNomTache);

	// Import thread
    sprintf_s(pcNomTache,sizeof(pcNomTache), "EMI_PIC import (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
		EMI_PIC[siInstId].sTaches[EMI_PIC_ID_THREAD_IMPORT],
		TRUE,
		dwPrioInitMax,
		2048,
		(LPTHREAD_START_ROUTINE)(EmiPicImport),
		(PVOID)(siInstId),
		NULL,
		pcNomTache);
	

	// No more threads (clear end of threads description array)
	NOYAU_VIDE_TACHE(EMI_PIC[siInstId].sTaches[EMI_PIC_NB_ID_THREAD]);

	// Save thread priority
	EMI_PIC[siInstId].dwPrioMax = dwPrioMax;
	
	// Save pool id
	EMI_PIC[siInstId].hPool = NOYAU_GetPoolId(pcPoolId);

	// Mailboxes names ('pcBalNam' must be different for each instance)
	strcpy_s(EMI_PIC[siInstId].szAniBalName,sizeof(EMI_PIC[siInstId].szAniBalName), pcBalNam);

	// Internal mailbox names use instance id and process id to avoid conflicts between instances
	sprintf_s(EMI_PIC[siInstId].szEmiBalName,sizeof(EMI_PIC[siInstId].szEmiBalName), "iBAL_EMI_PIC_EMI_%d_%d", siInstId, GetCurrentProcessId());
	sprintf_s(EMI_PIC[siInstId].szRestitBalName,sizeof(EMI_PIC[siInstId].szRestitBalName), "iBAL_EMI_PIC_RESTIT_%d_%d", siInstId, GetCurrentProcessId());
	sprintf_s(EMI_PIC[siInstId].szTrfBalName,sizeof(EMI_PIC[siInstId].szTrfBalName), "iBAL_EMI_PIC_TRF_%d_%d", siInstId, GetCurrentProcessId());
	sprintf_s(EMI_PIC[siInstId].szImportBalName,sizeof(EMI_PIC[siInstId].szImportBalName), "iBAL_EMI_PIC_IMPORT_%d_%d", siInstId, GetCurrentProcessId());

	// Global values initialization
	EMI_PIC[siInstId].dwArretBalId = -1;

	// Initialisation of services software component (for each service, set nb token)
	if((EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if((EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if((EMI_PIC[siInstId].hService[M_EMI_PIC_EMISSION] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if((EMI_PIC[siInstId].hService[M_SRV_ETAT] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();
	if((EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE] = SrvLance( 1 )) == NULL)
		ExitBad();



	// Launch thread(s) software component
	eResult = LanceTache(EMI_PIC[siInstId].sTaches);
	if(eResult != NOYAU_OK)
	{
		return INST_INIT_ERR_LANCE;
	}

    // Waiting mailbox publication for this instance
    *piBalId = AttendBAL(pcBalNam);
    if(*piBalId <= 0)
    {
		EmiPicFichierDebug(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> Pb InitBal() *****");
		return INST_INIT_ERR_LANCE;
    }

	EmiPicFichierTrace(siInstId, "EMI_PIC_INIT ***** EmiPicLance ==> init OK *****");

	return INST_INIT_OK;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_instance_result WINAPI MODLance(char * pcKey,                 */
/*                                             char * pcBalNam,              */
/*                                             noyau_bal_id * piBalId)       */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The module entry point.                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  char * pcKey            - Name of the registry key that contains */
/*                                    module specific parameters.            */
/*      IN  char * pcBalNam         - Mailbox name.                          */
/*      OUT noyau_bal_id * piBalId  - Mailbox ID.                            */
/*===========================================================================*/
/*  Return                       Description                                 */
/*---------------------------------------------------------------------------*/
/*  INST_INIT_OK                 The module is started.                      */
/*  INST_INIT_ERR_MAX_INSTANCE   The number of instances exceeded.           */
/*  INST_INIT_ERR_FICHIER_DEBUG  Error initializing the module trace.        */
/*  INST_INIT_ERR_LANCE          Error starting threats or allocating        */
/*                               memory.                                     */
/*  INST_INIT_ERR_REGISTRE       Error reading the registry.                 */
/*****************************************************************************/
EXPORT enum_instance_result WINAPI MODLance(char * pcKey,
											char * pcBalName,
											noyau_bal_id * piBalId)
{
    return EmiPicLance(pcKey, pcBalName, piBalId);
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_instance_result WINAPI EmiPicArret(noyau_bal_id iBalId)       */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function stops the module instance.                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  noyau_bal_id * piBalId  - Mailbox ID.                            */
/*===========================================================================*/
/*  Return                        Description                                */
/*---------------------------------------------------------------------------*/
/*  INST_ARRET_OK                 The function is successful.                */
/*  INST_ARRET_NOK                The function is not successful.            */
/*  INST_ARRET_ERR_TACHE          Error stopping threads.                    */
/*  INST_ARRET_ERR_FICHIER_DEBUG  Error stopping trace.                      */
/*****************************************************************************/
EXPORT enum_instance_result WINAPI EmiPicArret(noyau_bal_id iBalId)
{
    short  siInstId;

	// Search instance id with parameter
	for(siInstId = 0; siInstId < EMI_PIC_INSTANCE_MAX; siInstId++)
	{
		if(EMI_PIC[siInstId].dwAniBalId == iBalId)
			break;
	}

	if(siInstId > EMI_PIC_INSTANCE_MAX)
		// Instance id not found
		return  INST_ARRET_NOK ;

	// Free timer
	FreeTimers( siInstId );

	// Stop thread(s) 
	if(ArretTaches(EMI_PIC[siInstId].sTaches) == NOYAU_ARRET_TACHE_NOK)
		return INST_ARRET_ERR_TACHE;

	// Remove Mailboxes
	if(SupprimeBAL(EMI_PIC[siInstId].szAniBalName)!=NOYAU_OK)
		return INST_ARRET_NOK;
	if(SupprimeBAL(EMI_PIC[siInstId].szEmiBalName)!=NOYAU_OK)
		return INST_ARRET_NOK;
	if(SupprimeBAL(EMI_PIC[siInstId].szRestitBalName)!=NOYAU_OK)
		return INST_ARRET_NOK;
	if(SupprimeBAL(EMI_PIC[siInstId].szTrfBalName)!=NOYAU_OK)
		return INST_ARRET_NOK;
	if(SupprimeBAL(EMI_PIC[siInstId].szImportBalName)!=NOYAU_OK)
		return INST_ARRET_NOK;

	// Stop services software component
	if(SrvArret(&EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION]) == FALSE)
		ExitBad();
	if(SrvArret(&EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER]) == FALSE)
		ExitBad();
	if(SrvArret(&EMI_PIC[siInstId].hService[M_EMI_PIC_EMISSION]) == FALSE)
		ExitBad();
	if(SrvArret(&EMI_PIC[siInstId].hService[M_SRV_ETAT]) == FALSE)
		ExitBad();
	if(SrvArret(&EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE]) == FALSE)
		ExitBad();

	DeleteCriticalSection( &(EMI_PIC[siInstId].CS_Abort) );	
	
	// Update instance identifier (protected in critical section)
	DebutRegion();

	if(EMI_PIC_NB_INSTANCES > 0)
		EMI_PIC_NB_INSTANCES --;

	FinRegion();

	// Stop DEBUG mode
	if (DBG_Arret(&EMI_PIC[siInstId].sDbg) != DBG_OK)
		return INST_ARRET_ERR_FICHIER_DEBUG;

	return INST_ARRET_OK;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)          */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function stops the module instance.                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  noyau_bal_id * piBalId  - Mailbox ID.                            */
/*===========================================================================*/
/*  Return                        Description                                */
/*---------------------------------------------------------------------------*/
/*  INST_ARRET_OK                 The function is successful.                */
/*  INST_ARRET_NOK                The function is not successful.            */
/*  INST_ARRET_ERR_TACHE          Error stopping threads.                    */
/*  INST_ARRET_ERR_FICHIER_DEBUG  Error stopping trace.                      */
/*****************************************************************************/
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
    return EmiPicArret(iBalId);
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_instance_result EmiPicInitTrace(char * pcBal, short siInstId) */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function initializes the module trace.                    */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  char *pcBal    - Mailbox name.                                   */
/*      IN  short siInstId - Instance ID.                                    */
/*===========================================================================*/
/*  Return                       Description                                 */
/*---------------------------------------------------------------------------*/
/*  INST_INIT_OK                 Trace initialization is successful.         */
/*  INST_INIT_ERR_FICHIER_DEBUG  Trace initialization is not successful.     */
/*****************************************************************************/
PROTECTED enum_instance_result EmiPicInitTrace(char *pcBal, short siInstId)
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
	dbg_struct_debug *pDebug;
	dbg_struct_trace *pTabTraces;

	pDebug = &EMI_PIC[siInstId].sDbg;
	pTabTraces = &EMI_PIC[siInstId].sTabTraces[EMI_PIC_TRC];

    sprintf_s(pcKey,sizeof(pcKey),
			"%s%s%s",
			CSR_REG_KEYn_CSRBASE,
			CSR_REG_KEYn_LANE_BASE,
			CSR_REG_KEYn_CONFIG);

  	// Trace path 
    dwLen = sizeof(pDebug->rep_fichiers_traces);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, pDebug->rep_fichiers_traces, &dwLen) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;

  	// Error path
    dwLen = sizeof(pDebug->rep_fichier_erreurs);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, pDebug->rep_fichier_erreurs, &dwLen) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;
	
    // Maximal length of the trace files
    if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;
	pDebug->taille_limite = dwTailleMax;
	    
	// Initialisation of the trace on the screen
	// each instance of the EMI_PIC module will have its own file
    // defined by the mailbox name (unique)
	strcpy_s(pDebug->nom_fichier_traces_ecran,sizeof(pDebug->nom_fichier_traces_ecran), pcBal);

	// Each instance of the EMI_PIC module will also have its own
    // error file defined by its mailbox name (unique)
    strcpy_s(pDebug->nom_fichier_erreurs,sizeof(pDebug->nom_fichier_erreurs), pcBal);

	// Each instance of the EMI_PIC module will also have its trace file
	strcpy_s(pTabTraces->nom,sizeof(pTabTraces->nom), pcBal);

	pDebug->tab_traces = pTabTraces;
	pDebug->nb_fichiers_traces = EMI_PIC_NB_TRACES;
	
	if(DBG_Lance(pDebug) != DBG_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	return INST_INIT_OK;
}


PROTECTED char * escapeCStringForSPrintf(char *szInput ,char * szOutput, DWORD dwSize)
{
	int j=0;
	int i;
	int iSize;

	iSize = (int)strlen(szInput);
	memset(szOutput,0,dwSize);

	for(i=0; i < iSize; i++)
	{
		if(*(szInput+i)=='%')
		{	
			*(szOutput+j) = '%'; j++; 
			*(szOutput+j) = '%'; j++;
		}
		else
		{
			*(szOutput+j) = *(szInput+i); j++;
		}

		if(j>(int)dwSize-2)
			break;
	}


	return szOutput;
}


/**/
/*****************************************************************************/
/*SYNTAX: void  EmiPicFichierTrace(short siInstId, char *szFmt,...)          */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the trace file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId - Instance ID.                                    */
/*      IN  char *szFmt    - Pointer to the format string.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicFichierTrace(short siInstId, char *szFmt,...)
{
	va_list args;
	char szString[MAX_TRACE_LINE_LEN];
	
	va_start(args, szFmt);
	vsprintf_s(szString,sizeof(szString), szFmt, args);
	
	DBG_EcritFichierTraces(EMI_PIC_TRC, &EMI_PIC[siInstId].sDbg, szString);
	
	va_end(args);
}
/**/
/*****************************************************************************/
/*SYNTAX: void DEFINE_EmiPicFichierDebug(short siInstId, char *szFmt,...)    */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the error file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId - Instance ID.                                    */
/*      IN  char *szFmt    - Pointer to the format string.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void DEFINE_EmiPicFichierDebug(short siInstId, char *szFmt,...)
{
	va_list args;
	char szString[MAX_TRACE_LINE_LEN];

	va_start(args, szFmt);
	vsprintf_s(szString,sizeof(szString), szFmt, args);

	DBG_FILE = EMI_PIC_FILE;
	DBG_LINE = EMI_PIC_LINE;

	DEFINE_DBG_EcritFichierErreurs(EMI_PIC_TRC, &EMI_PIC[siInstId].sDbg, szString);

	va_end(args);
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PROTECTED BOOL LaunchTimer( short siInstId, enum_chrono enChrono, noyau_bal_id balDest, int iDelay )
{
	INT
		iChronoDelay;

	if( enChrono < FIRST_CHRONO && enChrono > LAST_CHRONO )
		return FALSE;

	if( iDelay != DEFAULT_DELAY )
		iChronoDelay = iDelay;
	else
		iChronoDelay = EMI_PIC[siInstId].sChronoData[enChrono].chrono_dalay;

	if( TestChrono( EMI_PIC[siInstId].sChronoData[enChrono].chrono_id ) == NOYAU_OK )
		ArretChrono( EMI_PIC[siInstId].sChronoData[enChrono].chrono_id );		

	if( LanceChrono(	EMI_PIC[siInstId].sChronoData[enChrono].chrono_id,
						iChronoDelay,
						balDest ) != NOYAU_OK )
	{
		EmiPicFichierDebug( siInstId,
			"LaunchTimer()->FAILED, timer ID [%d], delay [%d]",
			enChrono,
			iChronoDelay );

		return FALSE;
	}
	
	return TRUE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PRIVATE VOID FreeTimers( short siInstId )
{
	INT
		iIndex;

	for( iIndex = FIRST_CHRONO; iIndex < END_CHRONO; iIndex++ )
	{
		LibereChrono(&EMI_PIC[siInstId].sChronoData[iIndex].chrono_id );
	}
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PROTECTED int Eval_Exception( SHORT instance, int n_except, LPSTR szFile, int iLine )
{
	CHAR szException[ MAX_PATH ] = { 0 };

	switch( n_except )
	{
		case EXCEPTION_ACCESS_VIOLATION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_ACCESS_VIOLATION" );
			break;

		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_ARRAY_BOUNDS_EXCEEDED" );
			break;
			
		case EXCEPTION_BREAKPOINT:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_BREAKPOINT" );
			break;
			
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_DATATYPE_MISALIGNMENT" );
			break;
			
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_DENORMAL_OPERAND" );
			break;
			
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_DIVIDE_BY_ZERO" );
			break;
			
		case EXCEPTION_FLT_INEXACT_RESULT:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_INEXACT_RESULT" );
			break;
			
		case EXCEPTION_FLT_INVALID_OPERATION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_INVALID_OPERATION" );
			break;
			
		case EXCEPTION_FLT_OVERFLOW:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_OVERFLOW" );
			break;
			
		case EXCEPTION_FLT_STACK_CHECK:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_STACK_CHECK" );
			break;
			
		case EXCEPTION_FLT_UNDERFLOW:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_FLT_UNDERFLOW" );
			break;
			
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_ILLEGAL_INSTRUCTION" );
			break;
			
		case EXCEPTION_IN_PAGE_ERROR:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_IN_PAGE_ERROR" );
			break;
			
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_INT_DIVIDE_BY_ZERO" );
			break;
			
		case EXCEPTION_INT_OVERFLOW:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_INT_OVERFLOW" );
			break;
			
		case EXCEPTION_INVALID_DISPOSITION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_INVALID_DISPOSITION" );
			break;
			
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_NONCONTINUABLE_EXCEPTION" );
			break;
			
		case EXCEPTION_PRIV_INSTRUCTION:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_PRIV_INSTRUCTION" );
			break;
			
		case EXCEPTION_SINGLE_STEP:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_SINGLE_STEP" );
			break;
			
		case EXCEPTION_STACK_OVERFLOW:
			strcpy_s( szException,sizeof(szException), "EXCEPTION_STACK_OVERFLOW" );
			break;

		default:
			sprintf_s( szException,sizeof(szException), "%d", n_except );
			break;
	}

	EmiPicFichierDebug( instance, "Exception raised in file %s, line = %d. Exception code = %s", szFile, iLine, szException );
	_flushall();
    return EXCEPTION_EXECUTE_HANDLER;
}
// MFR start 2010/1/13
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PROTECTED VOID SetAbort( IN SHORT instance, IN BOOL Value )
{
	EnterCriticalSection( &(EMI_PIC[instance].CS_Abort) );
	EMI_PIC[instance].bAbort = Value;
	LeaveCriticalSection( &(EMI_PIC[instance].CS_Abort) );	
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PROTECTED BOOL GetAbort( IN SHORT instance )
{
	BOOL Value;

	EnterCriticalSection( &(EMI_PIC[instance].CS_Abort) );
	Value = EMI_PIC[instance].bAbort;
	LeaveCriticalSection( &(EMI_PIC[instance].CS_Abort) );	

	return Value ;
}
// MFR end
/*-------------------------------- END OF FILE ------------------------------*/