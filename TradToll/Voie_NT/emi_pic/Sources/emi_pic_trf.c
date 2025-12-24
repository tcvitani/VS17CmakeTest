/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_trf.c                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The transfer thread functions.                                */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <wingdi.h>
#include <imagehlp.h>
#include <stdio.h>

// Module interface
#include <emi_pic_glob.h>
#include <emi_pic_mess.h>
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE short int InitEmiPicTrf(void *param);
PRIVATE void EmiPicTrfReceptionAni(short siInstId,
                               struct_neutre *pNeutre);
PRIVATE void EmiPicTrfEnvoiAni(short siInstId,
                               enum_emi_pic_internal_type eTypeMessage,
                               void *pvParam);
PRIVATE void DeleteFilesFromDir(short siInstId, char *pcDir, char *pcMask);
PRIVATE void SendFilesFromDir(short siInstId, char *pcDir, char *pcMask);
PRIVATE void EmiPicGestionArretTrf(short siInstId);
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: DWORD WINAPI EmiPicTrf(PVOID pvParam)                              */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the messages received from the         */
/*            animation thread or from the timer.                            */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  PVOID pvParam  - Pointer to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  0                     This function allways returns zero.                */
/*****************************************************************************/
PROTECTED DWORD WINAPI EmiPicTrf(PVOID pvParam)
{
	struct_neutre *pNeutre;
	noyau_enum_retour eResult;
	short siInstId;

	// Thread initialisation (get instance id)
	siInstId = InitEmiPicTrf(pvParam);

	ChangePriorite(TacheCourante(), EMI_PIC[siInstId].dwPrioMax);

	// Wait and process the messages
	while(TRUE)
	{
		// Wait for the message in the EMI thread mailbox
		// (infinite timeout)
		eResult = Recoit(EMI_PIC[siInstId].dwTrfBalId,
						 (struct_neutre **)(&pNeutre), 
						 NOYAU_ATTENTE_INFINIE);

		// Protecting the message processing
		DebutRegion();

		if(eResult == NOYAU_BAL_MESS)
		{
			if(pNeutre->bl_retour == EMI_PIC[siInstId].dwAniBalId)
			{
				// Message received from the ANI thread
				EmiPicTrfReceptionAni(siInstId, pNeutre);
			}
			else
			{
				// Undefined maibox
				EmiPicFichierDebug(siInstId,
								   "EMI_PIC_TRF ***** EmiPicTrf ==> Undefined mailbox %d *****",
								   pNeutre->bl_retour);
			}

			// Free the message in the mailbox
			ExitLibere((struct_neutre **)(&pNeutre));
		}

		FinRegion();
	}

	return 0;
}

/**/
/*****************************************************************************/
/*SYNTAX: short int InitEmiPicTrf(void *pvParam)                             */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function opens the thread mailbox and waits for other      */
/*            module mailbox publication.                                    */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  void *pvParam   - Poiter to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  short int             Instance ID.                                       */
/*****************************************************************************/
PRIVATE short int InitEmiPicTrf(void *pvParam)
{
	short siInstId;

	// Get instance id
	siInstId = (short)pvParam;
   
	// publication of 'EMI_PIC' internal mailbox
	EMI_PIC[siInstId].dwTrfBalId = PublieBAL(EMI_PIC[siInstId].szTrfBalName, 0);
	if(EMI_PIC[siInstId].dwTrfBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by EMI thread)
	EMI_PIC[siInstId].dwEmiBalId = AttendBAL(EMI_PIC[siInstId].szEmiBalName);
	if(EMI_PIC[siInstId].dwEmiBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by RESTIT thread)
	EMI_PIC[siInstId].dwRestitBalId = AttendBAL(EMI_PIC[siInstId].szRestitBalName);
	if(EMI_PIC[siInstId].dwRestitBalId <= 0)
		ExitBad();

	// Waiting for the external mailbox publication (by ANI thread)
	EMI_PIC[siInstId].dwAniBalId = AttendBAL(EMI_PIC[siInstId].szAniBalName);
	if(EMI_PIC[siInstId].dwAniBalId <= 0)
		ExitBad();

	return (siInstId);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicTrfReceptionAni(short siInstId, struct_neutre *pNeutre) */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the ANI      */
/*            thread.                                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId          - Instance ID.                           */
/*      IN  struct_neutre *pNeutre  - Pointer to message received in the     */
/*                                    mailbox.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicTrfReceptionAni(short siInstId, struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;

	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_TRANSFER_SEND:
			FinRegion();
			SendFilesFromDir(siInstId,
							 pMessage->u.sAniTrf.sTrf.szDirName,
							 pMessage->u.sAniTrf.sTrf.szFileMask);
			DebutRegion();
			break;
		case M_TRANSFER_DEL:
			FinRegion();
			DeleteFilesFromDir(siInstId,
							   pMessage->u.sAniTrf.sTrf.szDirName,
							   pMessage->u.sAniTrf.sTrf.szFileMask);
			DebutRegion();
			break;
		case M_ARRET_DEMANDE:
			EmiPicGestionArretTrf(siInstId);
			break;

		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_TRF ***** EmiPicTrfReceptionAni ==> type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicTrfEnvoiAni(short siInstId,                             */
/*                               enum_emi_pic_type eTypeMessage,             */
/*                               void *vParam)                               */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends a message from the EMI thread to the        */
/*            animation thread.                                              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                  - Instance ID.                   */
/*      IN  enum_emi_pic_type eTypeMessage  - Message type.                  */
/*      IN  void *vParam                    - Poiter to the message.         */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicTrfEnvoiAni(short siInstId,
                               enum_emi_pic_internal_type eTypeMessage,
                               void *pvParam)
{
	struct_emi_pic_int_message *pMessage = NULL;
	struct_emi_pic_int_message *pData = (struct_emi_pic_int_message *)pvParam;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pMessage),
			   sizeof(struct_emi_pic_int_message),
			   EMI_PIC[siInstId].hPool);

	// Filling the message type field
	pMessage->entete.type_message = eTypeMessage;

	switch(eTypeMessage)
	{
		case M_TRANSFER_SEND_EFFECTUE:
		case M_TRANSFER_SEND_NON_EFFECTUE:
		case M_TRANSFER_DEL_EFFECTUE:
		case M_TRANSFER_DEL_NON_EFFECTUE:
			memcpy(&pMessage->u.sAniTrf,
				   &pData->u.sAniTrf,
				   sizeof(struct_emi_pic_ani_trf));
			break;
		case M_ARRET_EFFECTUE:
			break;

		default:
			break;
	}

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_TRF : EmiPicTrfEnvoiAni ==> Type_message '%d'",
					   pMessage->entete.type_message);

	// Send the message
	ExitEnvoie(EMI_PIC[siInstId].dwAniBalId,
			   EMI_PIC[siInstId].dwTrfBalId,
			   (struct_neutre *)pMessage);
}

/**/
/*****************************************************************************/
/*SYNTAX: void DeleteFilesFromDir(short siInstId, char *pcDir, char *pcMask) */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function deletes files that corespond to requested file    */
/*            mask from the directory.                                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId  - Instance ID.                                   */
/*      IN  char *pcDir     - Directory name.                                */
/*      IN  char *pcMask    - File mask.                                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void DeleteFilesFromDir(short siInstId, char *pcDir, char *pcMask)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA sFileData;
	char szFileName[MAX_PATH], szFileMask[MAX_PATH], szDir[MAX_PATH];
	boolean bDelResult = TRUE;
	struct_emi_pic_int_message sIntMessage;

	if(pcDir[strlen(pcDir)-1] != '\\')
		sprintf_s(szDir,sizeof(szDir), "%s\\", pcDir);
	else
		sprintf_s(szDir,sizeof(szDir), "%s", pcDir);
	
	if(strlen(pcMask) == 0)
		sprintf_s(szFileMask,sizeof(szFileMask), "%s*", szDir);
	else
		sprintf_s(szFileMask,sizeof(szFileMask), "%s%s", szDir, pcMask);

	hFindFile = FindFirstFile(szFileMask, &sFileData);

	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Remove all file flags except directory attribute
			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
			
			if(sFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				sprintf_s(szFileName,sizeof(szFileName), "%s%s", szDir, sFileData.cFileName);
				if(DeleteFile(szFileName) == FALSE)
				{
					// Error delete file
					bDelResult = FALSE;
				}
			}
		}while(FindNextFile(hFindFile, &sFileData) != 0);
	}

	FindClose(hFindFile);

	strcpy_s(sIntMessage.u.sAniTrf.sTrf.szDirName,sizeof(sIntMessage.u.sAniTrf.sTrf.szDirName), pcDir);
	strcpy_s(sIntMessage.u.sAniTrf.sTrf.szFileMask,sizeof(sIntMessage.u.sAniTrf.sTrf.szFileMask), pcMask);

	if(bDelResult == TRUE)
	{
		EmiPicTrfEnvoiAni(siInstId,
						  M_TRANSFER_DEL_EFFECTUE,
						  &sIntMessage);
	}
	else
	{
		EmiPicTrfEnvoiAni(siInstId,
						  M_TRANSFER_DEL_NON_EFFECTUE,
						  &sIntMessage);
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void GetDateTimeFromFileName(char *pcNameFromat,                   */
/*                                     char *pcFileName,                     */
/*                                     SYSTEMTIME *psTime)                   */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function takes the file date and time from the file name.  */
/*            picture will be stored.                                        */
/*            The file name is composed of the data used in the lane         */
/*            transaction and it uses the following convension:              */
/*                PPPP  - plaza number                                       */
/*                LLL   - lane number                                        */
/*                YYYY  - year of the trs. generation)                       */
/*                MM    - month of the trs. generation)                      */
/*                DD    - day of the trs. generation)                        */
/*                HH    - hour of the trs. generation)                       */
/*                MI    - minute of the trs. generation)                     */
/*                SE    - second of the trs. generation)                     */
/*                TTTTT - transaction number                                 */
/*                I     - camera (from 1 to 4) or scanned document           */
/*                        number (from 5 to 9)                               */
/*            The format string is stored in the registry parameters and it  */
/*            can be changed. It has to corespond to format of               */
/*            picture files generated by other modules (IHM or VES)          */
/*            The default format is: PPPPLLLYYYYMMDDHHMISETTTTTI             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  char *pcNameFromat  - File name format decsribed above.          */
/*      IN  char *pcFileName    - File name.                                 */
/*      OUT SYSTEMTIME *psTime  - Detected date and time.                    */
/*===========================================================================*/
/*  Return                 Description                                       */
/*---------------------------------------------------------------------------*/
/*  void                   This function does not return a value.            */
/*****************************************************************************/
PRIVATE void GetDateTimeFromFileName(char *pcNameFromat,
                                     char *pcFileName,
                                     SYSTEMTIME *psTime)
{
	char  szNameFormat[MAX_PATH];
	char  *pBegin;
	char  *pEnd;
	DWORD dwLen, dwStart;
	char szVal[10];

	strcpy_s(szNameFormat,sizeof(szNameFormat), pcNameFromat);

	pBegin = strstr(szNameFormat, "MI");
	if(pBegin != NULL)
	{
		pEnd = pBegin + 1;

		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wMinute = (unsigned short)atol(szVal);
	}

	pBegin = strstr(szNameFormat, "SE");
	if(pBegin != NULL)
	{
		pEnd = pBegin + 1;

		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wSecond = (unsigned short)atol(szVal);
	}

	pBegin = strchr(szNameFormat, 'Y');
	pEnd = strrchr(szNameFormat, 'Y');
	if(pBegin != NULL && pEnd != NULL)
	{
		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wYear = (unsigned short)atol(szVal);
	}

	pBegin = strstr(szNameFormat, "MM");
	if(pBegin != NULL)
	{
		pEnd = pBegin + 1;

		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wMonth = (unsigned short)atol(szVal);
	}

	pBegin = strchr(szNameFormat, 'D');
	pEnd = strrchr(szNameFormat, 'D');
	if(pBegin != NULL && pEnd != NULL)
	{
		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wDay = (unsigned short)atol(szVal);
	}
	pBegin = strchr(szNameFormat, 'H');
	pEnd = strrchr(szNameFormat, 'H');
	if(pBegin != NULL && pEnd != NULL)
	{
		dwLen = (DWORD)(pEnd - pBegin +1);
		dwStart = (DWORD)(pBegin - szNameFormat);
		strncpy_s(szVal,sizeof(szVal), &pcFileName[dwStart], dwLen);
		szVal[dwLen] = '\0';
		psTime->wHour = (unsigned short)atol(szVal);
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void SendFilesFromDir(short siInstId, char *pcDir, char *pcMask)   */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function moves files that corespond to requested file      */
/*            mask from the temporary directory to the output directory.     */
/*            It also creates the subdirectories according to date extracted */
/*            from the file name.                                            */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId  - Instance ID.                                   */
/*      IN  char *pcDir     - Directory name.                                */
/*      IN  char *pcMask    - File mask.                                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void SendFilesFromDir(short siInstId, char *pcDir, char *pcMask)
{
	LPFILEFOUND
		lpFiles		= NULL,
		lpToDelete	= NULL;
	BOOL
		Result;
	struct_emi_pic_int_message
		sIntMessage;
	
	EmiPicFichierTrace(siInstId, "SendFilesFromDir()->Dir [%s], file mask[%s]", pcDir, pcMask );

	do
	{
		Result = TRUE;
		
		if( _access( pcDir, FILE_EXISTANCE ) == FILE_NOT_FOUND )
		{
			EmiPicFichierTrace( siInstId, "SendFilesFromDir()->Dir does not exist" );
			Result = FALSE;
			break;
		}

		if( !GetFilesFromDir( siInstId, pcDir, &lpFiles, pcMask ) )
		{
			EmiPicFichierTrace( siInstId, "SendFilesFromDir()->No files found" );
			Result = FALSE;
			break;
		}
		
		ProcessNewFiles( siInstId, &lpFiles, pcDir );		
	}while( FALSE );

	memset( &sIntMessage, 0, sizeof(struct_emi_pic_int_message) );
	strcpy_s(sIntMessage.u.sAniTrf.sTrf.szDirName,sizeof(sIntMessage.u.sAniTrf.sTrf.szDirName), pcDir);
	strcpy_s(sIntMessage.u.sAniTrf.sTrf.szFileMask,sizeof(sIntMessage.u.sAniTrf.sTrf.szFileMask), pcMask);	

	if( Result )
	{		
		EmiPicTrfEnvoiAni(siInstId,
						  M_TRANSFER_SEND_EFFECTUE,
						  &sIntMessage);
	}
	else
	{
		EmiPicTrfEnvoiAni(siInstId,
						  M_TRANSFER_SEND_NON_EFFECTUE,
						  &sIntMessage);
	}
	
	
	
//	HANDLE hFindFile;
//	WIN32_FIND_DATA sFileData;
//	char szFileName[MAX_PATH], szFileMask[MAX_PATH], szDir[MAX_PATH];
//	char szServerDir[MAX_PATH], szTempDir[MAX_PATH], szFileNoExt[MAX_PATH];
//	char szOutFile[MAX_PATH];
//	boolean bSendResult = TRUE;
//	struct_emi_pic_int_message sIntMessage;
//	SYSTEMTIME sTime;
//	char *pExtension;
//
//	if(pcDir[strlen(pcDir)-1] != '\\')
//		sprintf(szDir, "%s\\", pcDir);
//	else
//		sprintf(szDir, "%s", pcDir);
//
//	if(strlen(pcMask) == 0)
//		sprintf(szFileMask, "%s*", szDir);
//	else
//		sprintf(szFileMask, "%s%s", szDir, pcMask);
//
//	hFindFile = FindFirstFile(szFileMask, &sFileData);
//
//	// TODO Redo to new way
//
//	if(hFindFile != INVALID_HANDLE_VALUE)
//	{
//		do
//		{
//			// Remove all file flags except directory attribute
//			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
//			
//			if(sFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
//			{
//				sprintf(szFileName, "%s%s", szDir, sFileData.cFileName);
//				// Get date from file name
//				memset(&sTime, 0, sizeof(SYSTEMTIME));
//				GetDateTimeFromFileName(EMI_PIC[siInstId].szNameFormat,
//										sFileData.cFileName,
//										&sTime);
//				// Create output subdirectories
//				strcpy(szServerDir, EMI_PIC[siInstId].ConfigLocal.szDestinationDir);
//				sprintf(szTempDir, "%04d\\", sTime.wYear);
//				strcat(szServerDir, szTempDir);
//				sprintf(szTempDir, "%02d\\", sTime.wMonth);
//				strcat(szServerDir, szTempDir);
//				sprintf(szTempDir, "%02d\\", sTime.wDay);
//				strcat(szServerDir, szTempDir);
//				// Create subdirectories if they don't exist
//				MakeSureDirectoryPathExists(szServerDir);
//				// Add counter to file name
//				pExtension = strrchr(sFileData.cFileName, '.');
//				if(pExtension != NULL)
//				{
//					strncpy(szFileNoExt, sFileData.cFileName, pExtension - sFileData.cFileName);
//					szFileNoExt[pExtension - sFileData.cFileName] = '\0';
//					sprintf(szOutFile, "%s%s.0000%s", szServerDir, szFileNoExt, pExtension);
//				}
//				else
//				{
//					sprintf(szOutFile, "%s%s.0000", szServerDir, sFileData.cFileName);
//				}
//				// Move file to out directory
//				if(MoveFileEx(szFileName,
//							  szOutFile,
//							  MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) == FALSE)
//				{
//					// Error move file
//					bSendResult = FALSE;
//				}
//			}
//		}while(FindNextFile(hFindFile, &sFileData) != 0);
//	}
//
//	FindClose(hFindFile);
//
//	strcpy(sIntMessage.u.sAniTrf.sTrf.szDirName, pcDir);
//	strcpy(sIntMessage.u.sAniTrf.sTrf.szFileMask, pcMask);
//
//	if(bSendResult == TRUE)
//	{
//		EmiPicTrfEnvoiAni(siInstId,
//						  M_TRANSFER_SEND_EFFECTUE,
//						  &sIntMessage);
//	}
//	else
//	{
//		EmiPicTrfEnvoiAni(siInstId,
//						  M_TRANSFER_SEND_NON_EFFECTUE,
//						  &sIntMessage);
//	}

}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicGestionArretTrf(short siInstId)                         */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function stops the emission thread.                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId  - Instance ID.                                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicGestionArretTrf(short siInstId)
{
	EmiPicTrfEnvoiAni(siInstId, M_ARRET_EFFECTUE, NULL);
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_TRF : EmiPicGestionArretTrf ==> Arret de la tache TRF");

	EmiPicLibereBAL(EMI_PIC[siInstId].dwTrfBalId);
	Termine();
}
/*-------------------------------- END OF FILE ------------------------------*/