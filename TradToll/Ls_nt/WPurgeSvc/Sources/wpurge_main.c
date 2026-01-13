/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WPURGE_vt
 * FILE       : wpurge_main.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module principal du service
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
#include <ntsvc.h>
#include <reg.h>
#include <imagehlp.h>
#include <io.h>
#include <CommonHelpFuncs.h>
#include <resource.h>

#define LOC_DEF
#include <wpurge_glob.h>
#undef LOC_DEF

#define LOC_DEF
#include <wpurge_main.h>
#undef LOC_DEF

#include <memclass.h>

// --------------- MACROS           ----------------------

// --------------- FONCTION PRIVEES ----------------------
PROTECTED BOOL ReadTargetsConfig();
PRIVATE DWORD MainInitService();
PRIVATE DWORD MainTerminateService();
PRIVATE void MainTreatment();
PRIVATE DWORD WINAPI MainCommand( char * pcParams );
PROTECTED DWORD WINAPI EmisionWorker(PVOID param);
PRIVATE BOOL EnumerateFilesToTransfer(struct_config_target * pConfig, char *pSubDir, DWORD *pdwFilesCount, BOOL *pbIsSubFolderEmpty);
PRIVATE void ResetTaskData();
PRIVATE BOOL GetIfTaskQueueEmpty();
// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
 *                        OUT char ** ppcServiceName,
 *                        OUT NTSVCCommandMain ** ppfCommand )
 * PARAMETERS: Cf doc NTSVC
 * RETURN    : Cf doc NTSVC
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Cf doc NTSVC
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(
        OUT     char ** ppcServiceName,
        OUT     NTSVCCommandMain ** ppfCommand )
{
    (*ppcServiceName) = gszSvcName;
    (*ppfCommand) = MainCommand;
    return TRUE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
 *                          IN      DWORD    dwArgc,
 *                          IN      char  ** ppcArgv )
 * PARAMETERS: dwArgc  : nombre de params.
 *             ppcArgv : liste des arguments
 * RETURN    : paramètre, ligne de commande, initialisation
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré pour le service
 * --------------------------------------------------------------------
 */
NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(
        IN      DWORD    dwArgc,
        IN      char  ** ppcArgv )
{
    DWORD dwErr;
    DWORD dwWait;
    DWORD dwDelay;
	BOOL bDoExit = FALSE;

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );

	do 
	{
        // Passer dans l'état "en cours de démarrage"
        NTSVCSetCurrentState( SERVICE_START_PENDING, 10000, NO_ERROR );

        // Lire les paramètres d'init du service
        dwErr = MainInitService();
        if ( dwErr != NO_ERROR )
        {
            SVC_ERR( dwErr, "ERR_INIT_SERVICE" );
            break;
        }

        // Passer dans l'état "running"
        NTSVCSetCurrentState( SERVICE_RUNNING, 0, NO_ERROR );
        NTSVCError( "SVC_STARTED" );

        // Les traitements ont été lancés avec les workers

        NTSVCInfo( "NTSVCExternalMain(), démarrage de la boucle principale" );
        // Attendre la demande d'arrêt tout en gerant l'interface avec la
        // base de données
        do
        {
			DWORD dwTotalFilesEnqued;
			DWORD i;

			NTSVCInfo("NTSVCExternalMain: periodical treatment start");

			do 
			{
				if (!bDoExit && GetIfTaskQueueEmpty()) //enumerate new files when tasks are finished
				{
					dwTotalFilesEnqued = 0;

					for (i = 0; i < gsSvcWork.sParmWork.dwNbDefinedTargets; i++)
					{
						DWORD dwFilesCount = 0;
						BOOL bFoo;
						EnumerateFilesToTransfer(&gsSvcWork.sParmWork.asConfigTarget[i], "", &dwFilesCount, &bFoo);
						
						if (NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent, 0) != WAIT_TIMEOUT)
						{
							bDoExit = TRUE;
							break;
						}

						dwTotalFilesEnqued += dwFilesCount;
						NTSVCInfo("NTSVCExternalMain: found to process %d files in folder[%s]", dwFilesCount, gsSvcWork.sParmWork.asConfigTarget[i].szSourceDir);

					}
				}
			
				// Attendre la demande de fin ou l'écoulement du délai de
				// polling de la base de données
				dwWait = NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent,	100);
				if (dwWait != WAIT_TIMEOUT) 
				{
					bDoExit = TRUE;
					break;
				}

			} while (dwTotalFilesEnqued>0 && !bDoExit);

            dwDelay = 0;

			if ((dwWait == WAIT_TIMEOUT && !bDoExit))
				NTSVCInfo("NTSVCExternalMain: entering periodic sleep for %u ms", gsSvcWork.sParmWork.dwMainPolling);

			while (dwWait == WAIT_TIMEOUT && !bDoExit)
            {
                // Attendre la demande de fin ou l'écoulement du délai de
                // polling de la base de données
                dwWait = NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent, 1000);
                if ( dwWait != WAIT_TIMEOUT ) 
				{
					bDoExit = TRUE;
					break;
				}

                dwDelay = dwDelay + 1000;
                if ( dwDelay >= gsSvcWork.sParmWork.dwMainPolling ) { dwWait = WAIT_TIMEOUT; break; }
            }
        }
		while (!bDoExit);

	} while (FALSE);

	NTSVCInfo( "NTSVCExternalMain(), détection d'une demande d'arrêt" );

	// Passer dans l'état "en cours d'arrêt"
	NTSVCSetCurrentState( SERVICE_STOP_PENDING, 10000, NO_ERROR );

	NTSVCInfo( "NTSVCExternalMain(), waiting for worker threads to stop" );

	dwWait = WaitForMultipleObjects(
		gsSvcWork.sParmWork.dwNumWorkers,   // number of handles in array
		gsSvcWork.ahThread,     // array of thread handles
		TRUE,          // wait until all are signaled
		10000); //10sec

	switch (dwWait)
	{
		// All thread objects were signaled
	case WAIT_OBJECT_0:
		NTSVCInfo("All threads ended, cleaning up for application exit...\n");
		break;

		// An error occurred
	default:
		NTSVCInfo("WaitForMultipleObjects failed (%d)\n", GetLastError());
	}


	NTSVCError("SVC_STOPPED");
	MainTerminateService();

 

     NTSVCSetCurrentState( SERVICE_STOPPED, 0, dwErr );

    // Tout est terminé
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainInitService()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Charge les données de conf. et initialise le service
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainInitService()
{
    DWORD dwErr = NO_ERROR;   // Code d'erreur à renvoyer
	DWORD i;

    //// Charger les paramètres
	
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
		WPURGE_REG_KEY,
		WPURGE_REG_VAL_MAINPOLLING,
		&gsSvcWork.sParmWork.dwMainPolling) != ERROR_SUCCESS)
	{
		gsSvcWork.sParmWork.dwMainPolling = 3000000;
		NTSVCError("MainInitService()->Error reading registry value [%s %s] - defaults to %u",
			WPURGE_REG_KEY, WPURGE_REG_VAL_MAINPOLLING, gsSvcWork.sParmWork.dwMainPolling);
	}


	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
		WPURGE_REG_KEY,
		WPURGE_REG_VAL_NumWorkers,
		&gsSvcWork.sParmWork.dwNumWorkers) != ERROR_SUCCESS || gsSvcWork.sParmWork.dwNumWorkers==0)
	{
		gsSvcWork.sParmWork.dwNumWorkers = 10;
		NTSVCError("MainInitService()->Error reading registry value [%s %s] - defaults to %u",
			WPURGE_REG_KEY, WPURGE_REG_VAL_NumWorkers, gsSvcWork.sParmWork.dwNumWorkers);
	}
	else
	{
		if (gsSvcWork.sParmWork.dwNumWorkers > MAX_WORKERS)
		{
			gsSvcWork.sParmWork.dwNumWorkers = MAX_WORKERS;
			NTSVCError("MainInitService()->Value [%s %s] - maximum set to %u",
				WPURGE_REG_KEY, WPURGE_REG_VAL_NumWorkers, gsSvcWork.sParmWork.dwNumWorkers);
		}
	}


	if (!ReadTargetsConfig())
	{
		NTSVCError("MainInitService()->ReadTargetsConfig returned false!");
		return ERROR_APP_INIT_FAILURE;
	}

    // Initialiser les valeurs calculées ou pré-initialisées
    gsSvcWork.bIsDebug = NTSVCIsDebugMode();        

    gsSvcWork.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( gsSvcWork.hEvent == NULL )
        return GetLastError();

	if (dwErr != NO_ERROR)
    {
        CloseHandle( gsSvcWork.hEvent );
        return dwErr;
    }

	InitializeCriticalSection(&gsSvcWork.csForTasks);
	ResetTaskData();

	for (i = 0; i < gsSvcWork.sParmWork.dwNumWorkers; i++)
	{
		gsSvcWork.ahThread[i] = NTSVCCreateThread(NULL, 0, EmisionWorker, (LPVOID)&gsSvcWork.ahThread[i], 0, &gsSvcWork.adwThreadId[i], "EmisionWorker");
	}



	return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD MainTerminateService()
 * PARAMETERS: Aucun
 * RETURN    : Code d'erreur Win32, NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Termine le service et décharge les données de conf.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD MainTerminateService()
{
    DWORD dwErr = NO_ERROR;
	struct_task_data * pTaskData = NULL;

    CloseHandle( gsSvcWork.hEvent );

	DeleteCriticalSection(&gsSvcWork.csForTasks);

    return dwErr;
}


PRIVATE BOOL GetIfTaskQueueEmpty()
{
	BOOL bEmpty = TRUE;

	EnterCriticalSection(&gsSvcWork.csForTasks);

	bEmpty = (gsSvcWork.iFirstTask < 0 || gsSvcWork.iLastTask < 0);

	LeaveCriticalSection(&gsSvcWork.csForTasks);

	return bEmpty;
}


PRIVATE void ResetTaskData()
{
	EnterCriticalSection(&gsSvcWork.csForTasks);

		gsSvcWork.iFirstTask = -1;
		gsSvcWork.iLastTask = -1;
	
	LeaveCriticalSection(&gsSvcWork.csForTasks);

}

PRIVATE BOOL AddTaskData(struct_config_target *pCfg, CHAR* szFileName, CHAR* szRelativePath, BOOL bIsFolder)
{
	struct_task_data sNewTask = { 0 };
	struct_task_data *pTargetAddress;
	BOOL bAddressOK = TRUE;

	sNewTask.bDoBackup = pCfg->bDoBackup;
	sNewTask.bIsFolder = bIsFolder;
	strcpy_s(sNewTask.m_szCurrentRootPath, sizeof(sNewTask.m_szCurrentRootPath), pCfg->szSourceDir);
	strcpy_s(sNewTask.m_szTargetRootPath, sizeof(sNewTask.m_szTargetRootPath), pCfg->szBackupDir);
	strcpy_s(sNewTask.m_szFileName, sizeof(sNewTask.m_szFileName), szFileName);
	strcpy_s(sNewTask.m_szRelativePath, sizeof(sNewTask.m_szRelativePath), szRelativePath);


	EnterCriticalSection(&gsSvcWork.csForTasks);

		if (gsSvcWork.iFirstTask<0 || gsSvcWork.iLastTask<0)
		{
			gsSvcWork.iFirstTask = 0;
			gsSvcWork.iLastTask = 0;
		}
		else if (gsSvcWork.iLastTask >= gsSvcWork.iFirstTask)
		{
			if (gsSvcWork.iLastTask + 1  >= MAX_TODO_TASKS)
			{
				if (gsSvcWork.iFirstTask > 0)
					gsSvcWork.iLastTask = 0;
				else
					bAddressOK = FALSE; //CIRCULAR BUFFER IS FULL
			}
			else
			{
				gsSvcWork.iLastTask++;
			}
		}
		else if (gsSvcWork.iLastTask + 2 < gsSvcWork.iFirstTask)
		{
			gsSvcWork.iLastTask++;
		}
		else 
		{
			bAddressOK = FALSE; //CIRCULAR BUFFER IS FULL
		}

		if (bAddressOK)
		{
			pTargetAddress = &gsSvcWork.asToDoTasks[gsSvcWork.iLastTask];
			memcpy(pTargetAddress, &sNewTask, sizeof(struct_task_data));
		}

	LeaveCriticalSection(&gsSvcWork.csForTasks);


	return bAddressOK;
}

PRIVATE BOOL GetNextTaskData(struct_task_data * data)
{
	BOOL bFound = FALSE;
	int iTargetAddress = -1;

	EnterCriticalSection(&gsSvcWork.csForTasks);

		if (gsSvcWork.iFirstTask < 0 || gsSvcWork.iLastTask < 0)
		{
			; // //CIRCULAR BUFFER IS EMPTY
		}
		else
		{
			iTargetAddress = gsSvcWork.iFirstTask;

			if (gsSvcWork.iFirstTask == gsSvcWork.iLastTask) //if it was the last item in the circular buffer reset the pointers
			{
				gsSvcWork.iFirstTask = -1;
				gsSvcWork.iLastTask = -1;
			}
			else if (gsSvcWork.iFirstTask + 1 >= MAX_TODO_TASKS)
			{
				gsSvcWork.iFirstTask = 0;
			}
			else
			{
				gsSvcWork.iFirstTask++;
			}

			memcpy(data, &gsSvcWork.asToDoTasks[iTargetAddress], sizeof(struct_task_data));
			bFound = TRUE;
		}

	LeaveCriticalSection(&gsSvcWork.csForTasks);

	return bFound;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD WINAPI MainCommand( char * pcParams )
 * PARAMETERS: Cf doc NTSVC
 * RETURN    : Cf doc NTSVC
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Cf doc NTSVC
 * --------------------------------------------------------------------
 */
PRIVATE DWORD WINAPI MainCommand( char * pcParams )
{
    //DWORD dwErr;

    // Traitement ligne de commande
    if ( _stricmp( pcParams, "NULLPIPE" ) == 0 )
    {
        //printf( "AuthSvc - Null session pipe setup ...\n" );
        //dwErr = AComSetNullSessionPipe( WPURGE_PIPE_CMD, TRUE );
        //if ( dwErr != NO_ERROR )
        //    printf( "AuthSvc - Error %u\n", dwErr );
        //else
        //    printf( "AuthSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}

PROTECTED BOOL ReadTargetsConfig()
{
	CHAR
		szQueryKey[MAX_PATH],
		szKeyName[MAX_PATH],
		szConfigKey[MAX_PATH],
		szBuffer[MAX_PATH];
	DWORD
		dwRet,
		dwIndex = 0,
		dwKeyNameLen = MAX_PATH,
		dwBufferLen;
	DWORD dwTemp;

	// Create configuration key path
	_makepath_s(szQueryKey, sizeof(szQueryKey), NULL, WPURGE_REG_KEY, WPURGE_REG_KEYn_Target_CONFIG, NULL);

	// Get first key
	dwRet = REG_Enum_Cles(CSR_REG_KEYi_ROOT, szQueryKey, dwIndex, szKeyName, &dwKeyNameLen);

	if (dwRet != ERROR_SUCCESS)
		NTSVCInfo("ReadTargetsConfig()->Error REG_Enum_Cles defined in [%s] ", szQueryKey);

	while (dwRet == ERROR_SUCCESS)
	{
		if (dwIndex >= MAX_TARGETS)
		{
			NTSVCError("ReadTargetsConfig()->There is more targets defined [%d] than allowed [%d]", dwIndex, MAX_TARGETS);
			return FALSE;
		}

		// Read key configuration
		// Create current config key path
		_makepath_s(szConfigKey, sizeof(szConfigKey), NULL, szQueryKey, szKeyName, NULL);
		// Query values from key
		NTSVCInfo("ReadTargetsConfig()- in [%s] ", szConfigKey);

		// Read source path
		dwBufferLen = MAX_PATH;
		if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_SourceDir,
			szBuffer,
			&dwBufferLen) != ERROR_SUCCESS)
		{
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s]", szConfigKey, WPURGE_REG_KEYn_SourceDir);
			return FALSE;
		}
		else
		{
			strcpy_s(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir), szBuffer);

			Trimm(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir));
			removePathEndingBackslashes(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir));

			if (strlen(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir) <= 3)
			{
				NTSVCError("ReadTargetsConfig()->Error invalid source path in [%s %s] = [%s]", 
					szConfigKey, WPURGE_REG_KEYn_SourceDir, gsSvcWork.sParmWork.asConfigTarget[dwIndex].szSourceDir);
				return FALSE;
			}
		}

		// Read backup path
		dwBufferLen = MAX_PATH;
		if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_BackupDir,
			szBuffer,
			&dwBufferLen) != ERROR_SUCCESS)
		{
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s]", szConfigKey, WPURGE_REG_KEYn_BackupDir);
			return FALSE;
		}
		else
		{
			strcpy_s(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir), szBuffer);

			Trimm(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir));
			removePathEndingBackslashes(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szBackupDir));

		}
	

		dwBufferLen = MAX_PATH;
		if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_ExtFilter,
			szBuffer,
			&dwBufferLen) != ERROR_SUCCESS)
		{
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s]", szConfigKey, WPURGE_REG_KEYn_ExtFilter);
			return FALSE;
		}
		else
			strcpy_s(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szExtFilter, sizeof(gsSvcWork.sParmWork.asConfigTarget[dwIndex].szExtFilter), szBuffer);


		// Read source path
		dwBufferLen = MAX_PATH;


		if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_DoBackup,
			&dwTemp) != ERROR_SUCCESS)
		{
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s] - defaults to TRUE", szConfigKey, WPURGE_REG_KEYn_DoBackup);
			gsSvcWork.sParmWork.asConfigTarget[dwIndex].bDoBackup = TRUE;
		}
		else
			gsSvcWork.sParmWork.asConfigTarget[dwIndex].bDoBackup = dwTemp ? TRUE : FALSE;


		if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_MaxFilesEnumStep,
			&dwTemp) != ERROR_SUCCESS)
		{
			gsSvcWork.sParmWork.asConfigTarget[dwIndex].dwMaxFilesEnumStep = 100;
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s] - defaults to %u", szConfigKey, WPURGE_REG_KEYn_MaxFilesEnumStep,
				gsSvcWork.sParmWork.asConfigTarget[dwIndex].dwMaxFilesEnumStep);

		}
		else
		{	
			if (dwTemp < 100)
				dwTemp = 100;
			gsSvcWork.sParmWork.asConfigTarget[dwIndex].dwMaxFilesEnumStep = dwTemp;
			
		}

		if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
			szConfigKey,
			WPURGE_REG_KEYn_RetentionPeriodHours,
			&gsSvcWork.sParmWork.asConfigTarget[dwIndex].dwRetentionPeriodHours) != ERROR_SUCCESS)
		{
			NTSVCError("ReadTargetsConfig()->Error reading registry value [%s %s]", szConfigKey, WPURGE_REG_KEYn_RetentionPeriodHours);
			return FALSE;
		}
		

		// Save number of read targets
		gsSvcWork.sParmWork.dwNbDefinedTargets++;

		// Reset key name length
		dwKeyNameLen = MAX_PATH;
		// Increase index
		dwIndex++;
		// Get next key
		dwRet = REG_Enum_Cles(CSR_REG_KEYi_ROOT, szQueryKey, dwIndex, szKeyName, &dwKeyNameLen);
	}

	if (dwRet != ERROR_NO_MORE_ITEMS)
	{
		NTSVCInfo(" ReadTargetsConfig()->Error while enumerating registry key[%s]", szConfigKey);
		return FALSE;
	}

	return TRUE;
}


#define FILE_EXISTANCE	0
#define FILE_NOT_FOUND	-1

PROTECTED DWORD WINAPI EmisionWorker(PVOID param)
{
	DWORD dwErr = NO_ERROR;
	DWORD dwWait;
	HANDLE hThread = *((HANDLE*)param);
	CHAR szCurrentFilePath[MAX_PATH];
	struct_task_data sTaskData = { 0 };
	BOOL bExitLoop = FALSE;
	CHAR szTemporaryPictureFolderPath[MAX_PATH];
	CHAR sRandomExtension[MAX_PATH];
	CHAR szTemporaryPicturePath[MAX_PATH];
	CHAR szFinalPictureFolderPath[MAX_PATH];
	CHAR szFinalPicturePath[MAX_PATH];

	do 
	{
		//- access global collection get first job structure and do the copy/delete
		while (!bExitLoop && GetNextTaskData(&sTaskData))
		{
		
			NTSVCInfo("EmisionWorker(%u) - Processing [%s]:[%s] at subdir:[%s]", hThread, sTaskData.bIsFolder ? "Folder" : "File", sTaskData.m_szFileName, sTaskData.m_szRelativePath);

			if (sTaskData.bIsFolder)
			{
				sprintf_s(szCurrentFilePath, sizeof(szCurrentFilePath), "%s\\%s%s", sTaskData.m_szCurrentRootPath, sTaskData.m_szRelativePath, sTaskData.m_szFileName);

				if (!RemoveDirectory(szCurrentFilePath))
				{
					NTSVCInfo("EmisionWorker(%u):: Error deleting folder [%s] ", hThread, szCurrentFilePath);
				}
				else
				{
					NTSVCInfo("EmisionWorker(%u):: Folder [%s] removed OK!", hThread, szCurrentFilePath);
				}
			}
			else if (sTaskData.bDoBackup)
			{
				BOOL bAllOK = FALSE;
				BOOL bTempFolderOK = FALSE;

				sprintf_s(szCurrentFilePath, sizeof(szCurrentFilePath), "%s\\%s%s", sTaskData.m_szCurrentRootPath, sTaskData.m_szRelativePath, sTaskData.m_szFileName);
				sprintf_s(szTemporaryPictureFolderPath, sizeof(szTemporaryPictureFolderPath), "%s\\TEMP\\", sTaskData.m_szTargetRootPath);
				sprintf_s(sRandomExtension, sizeof(sRandomExtension), ".%d", rand());
				sprintf_s(szTemporaryPicturePath, sizeof(szTemporaryPicturePath), "%s\\TEMP\\%s%s", sTaskData.m_szTargetRootPath, sTaskData.m_szFileName, sRandomExtension);
				sprintf_s(szFinalPicturePath, sizeof(szFinalPicturePath), "%s\\%s%s", sTaskData.m_szTargetRootPath, sTaskData.m_szRelativePath, sTaskData.m_szFileName);
				sprintf_s(szFinalPictureFolderPath, sizeof(szFinalPictureFolderPath), "%s\\%s", sTaskData.m_szTargetRootPath, sTaskData.m_szRelativePath);

				if (_access(szTemporaryPictureFolderPath, FILE_EXISTANCE) == FILE_NOT_FOUND)
				{
					if (MakeSureDirectoryPathExists(szTemporaryPictureFolderPath) != TRUE)
					{
						bTempFolderOK = FALSE;
						NTSVCInfo("EmisionWorker(%u):: Error creating remote storage path [%s]", hThread, szTemporaryPictureFolderPath);
					}
					else if (_access(szTemporaryPictureFolderPath, FILE_EXISTANCE) != FILE_NOT_FOUND)
					{
						bTempFolderOK = TRUE;
					}
				}
				else
					bTempFolderOK = TRUE;


				NTSVCInfo("EmisionWorker(%u):: Processing[%s].Access OK!", hThread, szCurrentFilePath);

				if (bTempFolderOK)
				{
					if (CopyFileEx(szCurrentFilePath, szTemporaryPicturePath, NULL, NULL, NULL, COPY_FILE_NO_BUFFERING | COPY_FILE_ALLOW_DECRYPTED_DESTINATION) != TRUE)
					{
						DWORD dwErr = GetLastError();
						NTSVCInfo("EmisionWorker(%u):: Error %d copying [%s] to remote storage path [%s] ", hThread, dwErr, szCurrentFilePath, szTemporaryPicturePath);

						if (DeleteFile(szTemporaryPicturePath) == FALSE)
						{
							NTSVCInfo("EmisionWorker(%u):: Error deleting temp file [%s] - Error :%d",
								hThread, szTemporaryPicturePath, GetLastError());
						}
					}
					else
					{
						NTSVCInfo("EmisionWorker(%u):: Copy of [%s] to TEMP folder  OK!", hThread, szCurrentFilePath);

						if (MakeSureDirectoryPathExists(szFinalPictureFolderPath) != TRUE ||
							_access(szFinalPictureFolderPath, FILE_EXISTANCE) == FILE_NOT_FOUND)
						{
							NTSVCInfo("EmisionWorker(%u):: Error %d accessing remote storage path [%s]", hThread, GetLastError(), szFinalPictureFolderPath);
						}
						else if (MoveFileEx(szTemporaryPicturePath, szFinalPicturePath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != TRUE)
						{
							NTSVCInfo("EmisionWorker(%u):: Error %d moving [%s] to remote storage path [%s]", hThread, GetLastError(),
								szTemporaryPicturePath, szFinalPicturePath);

						}
						else
						{
							NTSVCInfo("EmisionWorker(%u):: Picture sucessfuly moved to remote storage path [%s]. Deleting local image[%s].", 
								hThread, szFinalPicturePath, szCurrentFilePath);


							if (DeleteFile(szCurrentFilePath) == FALSE)
							{
								NTSVCInfo("EmisionWorker(%u):: Error deleting [%s] - Error :%d", hThread, szCurrentFilePath, GetLastError());
							}
							else
							{
								bAllOK = TRUE;
								NTSVCInfo("EmisionWorker(%u):: Deleting local image[%s] OK!", hThread, szCurrentFilePath);
							}
						}
					}
				}

			}
			else //delete file no backup...
			{
				sprintf_s(szCurrentFilePath, sizeof(szCurrentFilePath), "%s\\%s%s", sTaskData.m_szCurrentRootPath, sTaskData.m_szRelativePath, sTaskData.m_szFileName);

				if (!DeleteFile(szCurrentFilePath))
				{
					NTSVCInfo("EmisionWorker(%u)::Error deleting file [%s]", hThread, szCurrentFilePath);
				}
				else
				{
					NTSVCInfo("EmisionWorker(%u)::File [%s] deleted!", hThread, szCurrentFilePath);
				}
			}

			dwWait = NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent, 0);

			if (dwWait != WAIT_TIMEOUT)
			{
				NTSVCInfo("EmisionWorker(%u) - Exiting thread! NTSVCWaitForEndOrMultipleObjects returned dwWait:%u", hThread, dwWait);
				bExitLoop = TRUE;
				break;
			}

		}

		if (!bExitLoop)
		{
			dwWait = NTSVCWaitForEndOrMultipleObjects(1,&gsSvcWork.hEvent,100);

			if (dwWait != WAIT_TIMEOUT)
			{
				NTSVCInfo(" EmisionWorker(%u) - Exiting thread! NTSVCWaitForEndOrMultipleObjects returned dwWait:%u", hThread, dwWait);
				bExitLoop = TRUE;
				break;
			}
		}

	} while (!bExitLoop);


	NTSVCInfo(" EmisionWorker(%u) - Exiting thread!", hThread);

	return dwErr;
}

#define FILETIME_ONEMIN    10000000*60
#define FILETIME_ONEHOUR    10000000ULL*60 * 60


unsigned __int64 convertToInt64(const FILETIME *pFtFileTime)
{
	ULARGE_INTEGER    lv_Large;

	lv_Large.LowPart = pFtFileTime->dwLowDateTime;
	lv_Large.HighPart = pFtFileTime->dwHighDateTime;

	return lv_Large.QuadPart;
}

BOOL EnumerateFilesToTransfer(struct_config_target * pConfig, char *pSubDir, DWORD *pdwFilesCount, BOOL *pbIsSubFolderEmpty)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA sFileData;
	char szDir[MAX_PATH] = { 0 }, szSubDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 };
	HANDLE hFile;
	FILETIME ftTimeCreation = { 0 }, ftTimeAccess = { 0 }, ftTimeWrite = { 0 };
	BOOL bGetIntoFolder = TRUE;
	unsigned __int64 llNow;
	BOOL bExitRecursion = FALSE;
	DWORD dwNumFilesFound = 0;
	DWORD dwNumNotEmptySubfoldersFound = 0;
	
	*pbIsSubFolderEmpty = FALSE;

	NTSVCInfo("EnumerateFilesToTransfer: START folder[%s] - subdir[%s] - files count:%d", pConfig->szSourceDir, pSubDir, *pdwFilesCount);

	if (*pdwFilesCount >= pConfig->dwMaxFilesEnumStep ||
		(NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent, 0) != WAIT_TIMEOUT))
			return TRUE;

	GetSystemTimeAsFileTime((FILETIME*)&llNow);

	sprintf_s(szDir, sizeof(szDir), "%s\\%s*", pConfig->szSourceDir, pSubDir);

	hFindFile = FindFirstFile(szDir, &sFileData);

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (sFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if ((strcmp(sFileData.cFileName, ".") != 0) &&
					(strcmp(sFileData.cFileName, "..") != 0))
				{
					unsigned __int64 llFileTime = convertToInt64(&sFileData.ftCreationTime);

					if (llNow - llFileTime > (__int64)pConfig->dwRetentionPeriodHours* FILETIME_ONEHOUR)
						bGetIntoFolder = TRUE;
					else
						bGetIntoFolder = FALSE;
				
					if (bGetIntoFolder == TRUE)
					{
						BOOL bIsSubFolderEmpty = FALSE;
						sprintf_s(szSubDir, sizeof(szSubDir), "%s%s\\", pSubDir, sFileData.cFileName);
						bExitRecursion = EnumerateFilesToTransfer(pConfig, szSubDir, pdwFilesCount, &bIsSubFolderEmpty);

						if (bIsSubFolderEmpty && !bExitRecursion)
						{
							//addd sub dir for folder removal ...
							if (AddTaskData(pConfig, sFileData.cFileName, pSubDir, TRUE))
							{
								*pdwFilesCount = *pdwFilesCount + 1;
							}
							else
							{
								NTSVCInfo("EnumerateFilesToTransfer:: AddTaskData - failed! - exit recursion!");
								bExitRecursion = TRUE;
								break;
							}
						}
						else
						{
							dwNumNotEmptySubfoldersFound++; //so the parent will know it is not empty
						}

					}
					else
					{
						dwNumNotEmptySubfoldersFound++; //so the parent will know it is not empty
					}
				}
			}
			else
			{
				dwNumFilesFound++;

				if (strlen(pConfig->szExtFilter) != 0)
				{
					char strExt[MAX_PATH] = { 0 };
					int i = (int)strlen(sFileData.cFileName);

					for (; i >= 0; i--)
					{
						if (sFileData.cFileName[i] == '.')
							break;
					}

					if (i != 0)
					{
						strcpy_s(strExt, sizeof(strExt), sFileData.cFileName + i + 1);
						if (strcmp(strExt, pConfig->szExtFilter) != 0)
						{
							continue;
						}
					}
					else
						continue;
				}

				sprintf_s(szFileName, sizeof(szFileName), "%s\\%s%s", pConfig->szSourceDir, pSubDir, sFileData.cFileName);
				
				hFile = CreateFile(szFileName,
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				if (hFile != INVALID_HANDLE_VALUE)
					if (GetFileTime(hFile, &ftTimeCreation, &ftTimeAccess, &ftTimeWrite))
					{
						unsigned __int64 llFileTime = convertToInt64(&ftTimeCreation);
						unsigned __int64 llWriteTime = convertToInt64(&ftTimeWrite);

						if (llFileTime < llWriteTime)
							llFileTime = llWriteTime;

						if (llNow - llFileTime >(__int64)pConfig->dwRetentionPeriodHours* FILETIME_ONEHOUR)
						{
							CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;
							
							if (AddTaskData(pConfig, sFileData.cFileName, pSubDir, FALSE))
							{
								*pdwFilesCount = *pdwFilesCount + 1;
							}
							else
							{
								NTSVCInfo("AddTaskData - failed! - exit recursion!");
								bExitRecursion = TRUE;
								break; 
							}
						}
					}
				
				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;
				}
			}

			if (*pdwFilesCount >= pConfig->dwMaxFilesEnumStep ||
				(NTSVCWaitForEndOrMultipleObjects(1, &gsSvcWork.hEvent, 0) != WAIT_TIMEOUT))
			{
				NTSVCInfo("EnumerateFilesToTransfer Exit recursion!");
				bExitRecursion = TRUE;
				break;
			}

		} while (FindNextFile(hFindFile, &sFileData) != 0);

		FindClose(hFindFile);

		if (dwNumFilesFound == 0 && dwNumNotEmptySubfoldersFound == 0 && !bExitRecursion)
		{
			*pbIsSubFolderEmpty = TRUE;
		}
	
		NTSVCInfo("EnumerateFilesToTransfer: END folder[%s] - subdir[%s] - files count:%d", pConfig->szSourceDir, pSubDir, *pdwFilesCount);

	}
	else
	{
		NTSVCError("EnumerateFilesToTransfer:FindFirstFile returned invalid handle for folder[%s] - error:%u", szDir, GetLastError());
	}


	return bExitRecursion;
}


















