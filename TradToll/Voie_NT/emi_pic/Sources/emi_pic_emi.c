/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_emi.c                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The emission thread functions.                                */
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
typedef enum
{
	FIRST_STATUS		= 0,	

		STATUS_LINK_ERR	= FIRST_STATUS,

		END_STATUS,
		LAST_STATUS		= END_STATUS - 1,
		NB_STATUSS		= END_STATUS - FIRST_STATUS,

}enum_status;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE short int InitEmiPicEmi(void *param);
PRIVATE void EmiPicEmiReceptionAni(short siInstId,
                               struct_neutre *pNeutre);
PRIVATE void EmiPicEmiReceptionChrono(short siInstId,
                               struct_neutre *pNeutre);
PRIVATE void EmiPicEmiEnvoiAni(short siInstId,
                               enum_emi_pic_internal_type eTypeMessage,
                               void *pvParam);
PRIVATE DWORD DateToDays(SYSTEMTIME *sTime);
PRIVATE void CopyFileToServer(short siInstId, char *pDir);
// MFR start 2010/1/11
//PRIVATE void PurgeOldFiles(char *pDir, long lPurgePeriod);
// MFR end
PRIVATE void EmiPicGestionArretEmi(short siInstId);
// MFR start 2010/1/13
PRIVATE VOID CompleteMaintenance( short siInstId );
// MFR end
/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: DWORD WINAPI EmiPicEmi(PVOID pvParam)                              */
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
PROTECTED DWORD WINAPI EmiPicEmi(PVOID pvParam)
{
	struct_neutre *pNeutre;
	noyau_enum_retour eResult;
	short siInstId;

	// Thread initialisation (get instance id)
	siInstId = InitEmiPicEmi(pvParam);

	ChangePriorite(TacheCourante(), EMI_PIC[siInstId].dwPrioMax);

	// Wait and process the messages
	while(TRUE)
	{
		// Wait for the message in the EMI thread mailbox
		// (infinite timeout)
		eResult = Recoit(EMI_PIC[siInstId].dwEmiBalId,
						 (struct_neutre **)(&pNeutre), 
						 NOYAU_ATTENTE_INFINIE);

		// Protecting the message processing
		DebutRegion();

		if(eResult == NOYAU_BAL_MESS)
		{
			if(pNeutre->bl_retour == EMI_PIC[siInstId].dwAniBalId)
			{
				// Message received from the ANI thread
				EmiPicEmiReceptionAni(siInstId, pNeutre);
			}
			else if(pNeutre->bl_retour == NOYAU_BAL_HORLOGE_ID)
			{
				// Message received from the timer
				EmiPicEmiReceptionChrono(siInstId, pNeutre);
			}
			else
			{
				// Undefined mailbox
				EmiPicFichierDebug(siInstId,
								   "EMI_PIC_EMI ***** EmiPicEmi ==> Undefined mailbox %d *****",
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
/*SYNTAX: short int InitEmiPicEmi(void *pvParam)                             */
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
PRIVATE short int InitEmiPicEmi(void *pvParam)
{
	short
		siInstId;
	// MFR start 10/7/2008
	noyau_bal_id
		BalANI,
		BalRestit,
		BalTRF;
	// MFR end   10/7/2008

	// Get instance id
	siInstId = (short)pvParam;
    
	// publication of 'EMI_PIC' internal mailbox
	EMI_PIC[siInstId].dwEmiBalId = PublieBAL(EMI_PIC[siInstId].szEmiBalName, 0);
	if(EMI_PIC[siInstId].dwEmiBalId <= 0)
		ExitBad();

	// MFR start 10/7/2008
	// Waiting for the internal mailbox publication (by RESTIT thread)
	BalRestit = AttendBAL(EMI_PIC[siInstId].szRestitBalName);
	if( BalRestit <= 0)
		ExitBad();
	
	// Waiting for the internal mailbox publication (by TRF thread)
	BalTRF = AttendBAL(EMI_PIC[siInstId].szTrfBalName);
	if( BalTRF <= 0)
		ExitBad();	
	
	// Waiting for the external mailbox publication (by ANI thread)
	BalANI = AttendBAL(EMI_PIC[siInstId].szAniBalName);
	if(BalANI <= 0)
		ExitBad();
/*
	// Waiting for the internal mailbox publication (by RESTIT thread)
	EMI_PIC[siInstId].dwRestitBalId = AttendBAL(EMI_PIC[siInstId].szRestitBalName);
	if(EMI_PIC[siInstId].dwRestitBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by TRF thread)
	EMI_PIC[siInstId].dwTrfBalId = AttendBAL(EMI_PIC[siInstId].szTrfBalName);
	if(EMI_PIC[siInstId].dwTrfBalId <= 0)
		ExitBad();
	
	// Waiting for the external mailbox publication (by ANI thread)
	EMI_PIC[siInstId].dwAniBalId = AttendBAL(EMI_PIC[siInstId].szAniBalName);
	if(EMI_PIC[siInstId].dwAniBalId <= 0)
		ExitBad();
*/
	// MFR end   10/7/2008

	if( LaunchTimer( siInstId, CHRONO_EMISSION, EMI_PIC[siInstId].dwEmiBalId, DEFAULT_DELAY ) != TRUE )
	{
		EmiPicFichierDebug( siInstId, "Error launching timer" );
		ExitBad();
	}
	
//	LanceChrono(EMI_PIC[siInstId].uiChronoEmiId,
//				EMI_PIC[siInstId].dwChronoEmiPeriod,
//				EMI_PIC[siInstId].dwEmiBalId);

	return (siInstId);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEmiReceptionAni(short siInstId, struct_neutre *pNeutre) */
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
PRIVATE void EmiPicEmiReceptionAni(short siInstId, struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;
	LONG
		i;
	
	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_ARRET_DEMANDE:
			EmiPicGestionArretEmi(siInstId);
			break;

		case M_EMISSION_PURGE:
			// Purge all the directories
			PurgeOldFiles( EMI_PIC[siInstId].ConfigLocal.szDestinationDir, -1, EMI_PIC[siInstId].dwPurgeFileDelay );
			PurgeOldFiles(EMI_PIC[siInstId].ConfigBackup.szDestinationDir, -1, EMI_PIC[siInstId].dwPurgeFileDelay );
			EmiPicEmiEnvoiAni(siInstId, M_EMISSION_PURGE_EFFECTUE, NULL);
			break;

		case M_EMI_PIC_MAINT_REQUEST:

			// Backup original configuration
			memset( EMI_PIC[siInstId].BackupCopy, 0, sizeof(struct_config_data)*MAX_DESTINATIONS );
			memcpy( EMI_PIC[siInstId].BackupCopy, EMI_PIC[siInstId].ConfigDest, sizeof(struct_config_data)*MAX_DESTINATIONS );

			for( i = 0; i < MAX_DESTINATIONS; i++ )
			{
				// Replace distant destination (probably remote file server) with local destination (probably USB disk or removable disk)
				strcpy_s( EMI_PIC[siInstId].ConfigDest[i].szDestinationDir,sizeof(EMI_PIC[siInstId].ConfigDest[i].szDestinationDir), pMessage->u.sAniMaintMode.u.szPath );
				// Remove temporary file extensions, can speed up process
				strcpy_s( EMI_PIC[siInstId].ConfigDest[i].szTempExtension,sizeof(EMI_PIC[siInstId].ConfigDest[i].szTempExtension), "");
			}

			FinRegion();
			// Slow function
			EMI_PIC[siInstId].NbSubdirs = GetNumberOfSubdirs( EMI_PIC[siInstId].ConfigLocal.szDestinationDir );
			DebutRegion();
			

			if( EMI_PIC[siInstId].NbSubdirs == 0 )
			{// There are nothing to export

				EmiPicEmiEnvoiAni( siInstId,
					M_EMI_PIC_MAINT_ACCEPTED,
					(LPVOID)1 );
				
				EmiPicEmiEnvoiAni( siInstId,
					M_EMI_PIC_MAINT_UPDATE,
					NULL );
				
				EmiPicEmiEnvoiAni( siInstId, M_EMI_PIC_MAINT_EFFECTUE, 0 );				
			}
			else
			{
				EMI_PIC[siInstId].bMainModeON = TRUE;

				EmiPicEmiEnvoiAni( siInstId,
					M_EMI_PIC_MAINT_ACCEPTED,
					(LPVOID)EMI_PIC[siInstId].NbSubdirs );
				
				// Save original value for the timer and decrease current to minimum (maximize execution speed)
				EMI_PIC[siInstId].dwChronoEmiPeriodBackup = EMI_PIC[siInstId].dwChronoEmiPeriod;
				EMI_PIC[siInstId].dwChronoEmiPeriod = 1;

				// Reset counter for transfered files
				EMI_PIC[siInstId].NbFilesMaint = 0;

				LaunchTimer( siInstId, CHRONO_EMISSION, EMI_PIC[siInstId].dwEmiBalId, 1 );
			}

			// Resume work
			SetAbort( siInstId, FALSE );
			break;

		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_EMI ***** EmiPicEmiReceptionAni ==> type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEmiReceptionChrono(short siInstId,                      */
/*                                      struct_neutre *pNeutre)              */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the timer.   */
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
PRIVATE void EmiPicEmiReceptionChrono(short siInstId,
                                      struct_neutre *pNeutre)
{
	struct_chrono  *pMessage = (struct_chrono *)pNeutre;
	SYSTEMTIME     sCurrentTime;
//	char           szBuffer[MAX_PATH + 1];
	LONG
		CurrentDay,
		LastPurgeDay;

	if( pMessage->numero == EMI_PIC[siInstId].sChronoData[CHRONO_EMISSION].chrono_id )
	{

		if( GetAbort( siInstId ) != FALSE )
		{
			if( EMI_PIC[siInstId].bMainModeON != FALSE )
			{// Maintenance mode is ON
				CompleteMaintenance( siInstId );
			}
			return;
		}
		FinRegion();
		// Check the content of the Out directory
		CopyFileToServer(siInstId, EMI_PIC[siInstId].ConfigLocal.szDestinationDir );
		DebutRegion();

		// Check if it is necessary to purge backup directory
		if(EMI_PIC[siInstId].dwPurgePeriod != 0)
		{
			GetLocalTime(&sCurrentTime);

			CurrentDay = DateToDays(&sCurrentTime);
			LastPurgeDay = DateToDays(&EMI_PIC[siInstId].sLastPurgeDTime);

			if((EMI_PIC[siInstId].sLastPurgeDTime.wYear == 0) ||
				((( CurrentDay - LastPurgeDay ) >= 1) && 
				(sCurrentTime.wHour >= EMI_PIC[siInstId].dwPurgeHour) &&
				(sCurrentTime.wMinute >= EMI_PIC[siInstId].dwPurgeMinute)) )
			{
				// MFR start 2010/1/11				
				/* 
				FinRegion();
				PurgeOldFiles(EMI_PIC[siInstId].ConfigBackup.szDestinationDir, (long)EMI_PIC[siInstId].dwPurgePeriod);
				DebutRegion();
				// Save the last purge date and time
				sprintf(szBuffer,
						"%d.%d.%d %d:%d",
						sCurrentTime.wDay,
						sCurrentTime.wMonth,
						sCurrentTime.wYear,
						sCurrentTime.wHour,
						sCurrentTime.wMinute);

				REG_Ecrire_Chaine(CSR_REG_KEYi_ROOT,
								  EMI_PIC[siInstId].szKey,
								  EMI_PIC_REG_KEYv_LAST_PURGE_TIME,
								  szBuffer);

				memcpy(&EMI_PIC[siInstId].sLastPurgeDTime, &sCurrentTime, sizeof(SYSTEMTIME));
				*/

				if( EMI_PIC[siInstId].bPurgeInProgress != TRUE )
				{
					EmiPicAniEnvoiRestit( siInstId, M_RESTIT_PURGE_BACKUP, NULL );
					EMI_PIC[siInstId].bPurgeInProgress = TRUE;
				}
				// MFR end
			}
		}

		LaunchTimer( siInstId, CHRONO_EMISSION, EMI_PIC[siInstId].dwEmiBalId, DEFAULT_DELAY );
		

//		LanceChrono(EMI_PIC[siInstId].uiChronoEmiId,
//					EMI_PIC[siInstId].dwChronoEmiPeriod,
//					EMI_PIC[siInstId].dwEmiBalId);
	}
	else
	{
		EmiPicFichierDebug(siInstId,
						   "EMI_PIC_EMI ***** EmiPicEmiReceptionChrono ==> undefined chrono %d *****",
						   pMessage->numero);
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEmiEnvoiAni(short siInstId,                             */
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
PRIVATE void EmiPicEmiEnvoiAni(short siInstId,
                               enum_emi_pic_internal_type eTypeMessage,
                               void *pvParam)
{
	struct_emi_pic_int_message *pMessage = NULL;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pMessage),
			   sizeof(struct_emi_pic_int_message),
			   EMI_PIC[siInstId].hPool);

	// Filling the message type field
	pMessage->entete.type_message = eTypeMessage;

	switch(eTypeMessage)
	{
		case M_EMISSION_PURGE_EFFECTUE:
		case M_STATUS_CHANGED:
		case M_ARRET_EFFECTUE:
			break;

		case M_EMI_PIC_MAINT_EFFECTUE:
			pMessage->u.sAniMaintMode.u.NbFiles = (LONG)pvParam;
			break;
			
		case M_EMI_PIC_MAINT_ACCEPTED:
			pMessage->u.sAniMaintMode.u.NbDirectories = (LONG)pvParam;
			break;

		default:
			break;
	}

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_EMI : EmiPicEmiEnvoiAni ==> Type_message '%d'",
					   pMessage->entete.type_message);

	// Send the message
	ExitEnvoie(EMI_PIC[siInstId].dwAniBalId,
			   EMI_PIC[siInstId].dwEmiBalId,
			   (struct_neutre *)pMessage);
}

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE DWORD DateToDays(SYSTEMTIME *sTime)                        */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function calculates the number of days from 1.1.0000.     */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  SYSTEMTIME *sTime - Pointer to the date and time structure.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 This function returns the number of days.          */
/*****************************************************************************/
PRIVATE DWORD DateToDays(SYSTEMTIME *sTime)
{
	DWORD dwDays;
	DWORD dwDays1[12] = { 0,
				/* +31 */ 31,
				/* +28 */ 59,
				/* +31 */ 90,
				/* +30 */ 120,
				/* +31 */ 151,
				/* +30 */ 181,
				/* +31 */ 212,
				/* +31 */ 243,
				/* +30 */ 273,
				/* +31 */ 304,
				/* +30 */ 334 };

	DWORD dwDays2[12] = { 0,
				/* +31 */ 31,
				/* +29 */ 60,
				/* +31 */ 91,
				/* +30 */ 121,
				/* +31 */ 152,
				/* +30 */ 182,
				/* +31 */ 213,
				/* +31 */ 244,
				/* +30 */ 274,
				/* +31 */ 305,
				/* +30 */ 335 };

	dwDays = (sTime->wYear - 1) *365 +
		(DWORD)((sTime->wYear - 1)/4) -
		(DWORD)((sTime->wYear - 1)/100) +
		(DWORD)((sTime->wYear - 1)/400);
	

	if(sTime->wMonth > 0 && sTime->wMonth < 13)
	{
		if((sTime->wYear%400 == 0) ||
			((sTime->wYear%4 == 0) && (sTime->wYear%100 != 0)))
		{
			dwDays = dwDays + dwDays2[sTime->wMonth-1];
		}
		else
		{
			dwDays = dwDays + dwDays1[sTime->wMonth-1];
		}
	}

	dwDays = dwDays + sTime->wDay;

	return dwDays;
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
PROTECTED BOOL IsOkToRemoveDir( short siInstId, LPSTR szDir, DWORD dwMaxDaysOld )
{
	INT
		iIndex,
		iValue;
	
	struct_string_data
		sStringData,
		sDummy			= { 0 };

	CHAR
		szDestinationDir[MAX_PATH],
		szCurrentDir[MAX_PATH];

	SYSTEMTIME
		sTime,
		sFileTime = { 0 };
	LONG
		DaysToday,
		DaysFileTime;

	_makepath_s( szDestinationDir,sizeof(szDestinationDir), NULL, EMI_PIC[siInstId].ConfigLocal.szDestinationDir, NULL, NULL );
	_makepath_s( szCurrentDir,sizeof(szCurrentDir), NULL, szDir, NULL, NULL );
	
	GetLocalTime(&sTime);

	sDummy.lValue[ELEMENT_MONTH]	= sTime.wMonth;
	sDummy.lValue[ELEMENT_DAY]		= sTime.wDay;
	
	GetPartialDataFromString( szDir, &(EMI_PIC[siInstId].ConfigLocal.sFormatData), &sStringData );

	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
	{
		if( sStringData.lValue[iIndex] != 0 )
			iValue = sStringData.lValue[iIndex];
		else
			iValue = sDummy.lValue[iIndex];
		
		switch( iIndex )
		{
			case ELEMENT_YEAR:
				sFileTime.wYear = iValue;
				break;
				
			case ELEMENT_MONTH:
				sFileTime.wMonth = iValue;
				break;
				
			case ELEMENT_DAY:
				sFileTime.wDay = iValue;
				break;
				
			case ELEMENT_HOUR:
				sFileTime.wHour = iValue;
				break;
				
			case ELEMENT_MINUTE:
				sFileTime.wMinute = iValue;
				break;
				
			case ELEMENT_SECOND:
				sFileTime.wSecond = iValue;
				break;
				
			case ELEMENT_MILLISECOND:
				sFileTime.wMilliseconds = iValue;
				break;
				
			default:
				break;
		}						
	}

	DaysToday = DateToDays(&sTime);
	DaysFileTime = DateToDays(&sFileTime);
	
	if( DaysToday - DaysFileTime > (LONG)dwMaxDaysOld )
		return TRUE;

	return FALSE;
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
PRIVATE VOID GetTimeDiff( IN LPSYSTEMTIME lpTimeStart, LPSYSTEMTIME lpTimeStop, IN OUT LPSYSTEMTIME lpTimeDiff )
{
	LONGLONG
		llStart,
		llStop,
		llDiff;
	FILETIME
		ftStart,
		ftStop,
		ftDiff;

	if( lpTimeStart	== NULL	||
		lpTimeStop	== NULL	||
		lpTimeDiff	== NULL )
	{
		return;
	}

	SystemTimeToFileTime( lpTimeStart, &ftStart );
	SystemTimeToFileTime( lpTimeStop, &ftStop );

	llStart	= *((LONGLONG*)&ftStart);
	llStop	= *((LONGLONG*)&ftStop);

	llDiff = llStop - llStart;

	ftDiff	= *((FILETIME*)&llDiff);

	FileTimeToSystemTime( &ftDiff, lpTimeDiff );
}
/*****************************************************************************/
/*SYNTAX: PRIVATE void CopyFileToServer(short siInstId, char * pDir)         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function copies the pictures from the local directory to  */
/*            the server directory. After successful file coping it moves    */
/*            the file to the local backup directory. It also creates        */
/*            sub-directories inside the server directory and inside the     */
/*            local backup directory depending on the lane number, month and */
/*            day of the picture acquiring. The sub-directory creation can   */
/*            be disabled or reconfigured by module parameters.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId - Instance ID.                                    */
/*      IN  char * pDir    - Directory that contains pictures.               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function doesn't return a value.              */
/*****************************************************************************/
PRIVATE void CopyFileToServer (short siInstId, char *pDir)
{
	HANDLE
		hFindFile;

	WIN32_FIND_DATA
		sFileData;

	CHAR
		szDir[MAX_PATH],
		szSubDir[MAX_PATH],
		szStorePath[MAX_PATH],
		szFileSource[MAX_PATH],
		szFileBackup[MAX_PATH],
		szName[MAX_PATH],
		szExtension[MAX_PATH];

	BOOL
		bFound = FALSE,			// Flag found files/dir in dir
		bFoundFiles = FALSE;	// Flag found files/dir in dir
	LONG
		lNbFilesFound		= 0,
		lNbFilesTransfered	= 0,
		lNbFilesFailed		= 0,
		iIndex;
	struct_string_data
		sStringData;
	SYSTEMTIME
		sTimeStart,
		sTimeStop,
		sTimeDiff;

	EmiPicFichierTrace( siInstId, "TRANSFER : Transfer files from dir [%s]", pDir );
	
	_makepath_s( szDir,sizeof(szDir), NULL, pDir, "*", NULL );

	GetLocalTime( &sTimeStart );
	
	hFindFile = FindFirstFile( szDir, &sFileData );

	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// MFR start 2010/1/13
			if( GetAbort( siInstId ) != FALSE )
			{// Abort operation
				
				if( EMI_PIC[siInstId].bMainModeON != FALSE )
				{// Maintenance mode is ON
					CompleteMaintenance( siInstId );
				}

				break;
			}
			// MFR end
			
			// Remove all file flags except directory attribute
			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
			
			if(sFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				if((strcmp(sFileData.cFileName, ".") != 0) &&
					(strcmp(sFileData.cFileName, "..") != 0))
				{
					_makepath_s( szSubDir,sizeof(szSubDir), NULL, pDir, sFileData.cFileName, NULL );
					
					CopyFileToServer(siInstId, szSubDir);
					bFound = TRUE;
				}
			}
			else
			{
				lNbFilesFound++;

				// Create source path
				_makepath_s( szFileSource,sizeof(szFileSource), NULL, pDir, sFileData.cFileName, NULL );				
				
				if( TransferFileToDestinations( siInstId, szFileSource, sizeof(szFileSource) ) != FALSE )
				{
					if( GetDataFromString( sFileData.cFileName, &(EMI_PIC[siInstId].sFileNameFormat), &sStringData ) != FALSE )
					{
						CreateDestinationPath( szStorePath, &(EMI_PIC[siInstId].ConfigBackup), &sStringData );

						if( MakeSureDirectoryPathExists( szStorePath ) != FALSE )
						{
							// File name changed (increase in transmission counter)
							// Get new file name
							_splitpath( szFileSource, NULL, NULL, szName, szExtension );
							_makepath_s( sFileData.cFileName,sizeof(sFileData.cFileName), NULL, NULL, szName, szExtension );
							
							// Create full file path with name
							_makepath_s( szFileBackup,sizeof(szFileBackup), NULL, szStorePath, sFileData.cFileName, NULL );
							
							START_TRY
							if( MoveFileEx( szFileSource,
										szFileBackup,
										MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) != FALSE )
							{
								lNbFilesTransfered++;

								if( EMI_PIC[siInstId].bMainModeON != FALSE )
								{// Increase counter of files transfered in maintenance mode
									EMI_PIC[siInstId].NbFilesMaint++;
								}
							}
							else
							{
								EmiPicFichierDebug( siInstId, "TRANSFER : Failed to copy file" );
								EmiPicFichierDebug( siInstId, "\t File path [%d]", szFileSource );
								EmiPicFichierDebug( siInstId, "\t File destination [%s]", szFileBackup );
									
								lNbFilesFailed++;
							}
							END_TRY
						}
					}
					else
					{
						EmiPicFichierTrace( siInstId,
							"TRANSFER : File [%s], does not fit format",
							sFileData.cFileName );
						lNbFilesFailed++;
					}
				}
				else
					lNbFilesFailed++;

				bFound = TRUE;
				bFoundFiles = TRUE;
			}

		}while( FindNextFile(hFindFile, &sFileData) != 0);

		// Remove an empty directory
		if(bFound == FALSE)
		{
			if( (GetLastError() == ERROR_NO_MORE_FILES) &&
				(EMI_PIC[siInstId].ConfigLocal.sFormatData.Element[ELEMENT_YEAR].bIsUsed == TRUE) )// Format must contain at least year
			{
				iIndex = (LONG)strlen( EMI_PIC[siInstId].ConfigLocal.szDestinationDir );
				
				_makepath_s( szDir,sizeof(szDir), NULL, pDir, NULL, NULL );
				
				if( szDir[iIndex] == '\\' )
					iIndex++;

				
				if( (strlen( EMI_PIC[siInstId].ConfigLocal.szDestinationDir ) != strlen( szDir )) &&				
					IsOkToRemoveDir( siInstId, &(pDir[iIndex]), 1 ) != FALSE )
				{
					FindClose(hFindFile);

					EmiPicFichierTrace( siInstId, "TRANSFER : RemoveDirectory [%s]", pDir );
					
					if( RemoveDirectory( pDir ) != TRUE )
					{
						LPSTR
							lpMsgBuf;

						lpMsgBuf = NULL;
						
						FormatMessage( 
							FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							GetLastError(),
							0, // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL 
							);

						EmiPicFichierDebug( siInstId,
							"TRANSFER : Failed to remove dir [%s], description = [%s]",
							pDir,
							lpMsgBuf );
						
						// Free the buffer.
						LocalFree( lpMsgBuf );
					}
					else
					{
						EmiPicFichierTrace( siInstId, "TRANSFER : Directory [%s] removed", pDir );
					}
					return;
				}
			}
		}
		else
		{
			if( EMI_PIC[siInstId].bMainModeON != FALSE )
			{// Maintenance mode is ON
				EMI_PIC[siInstId].NbSubdirs--;
				
				EmiPicEmiEnvoiAni( siInstId,
					M_EMI_PIC_MAINT_UPDATE,
					NULL );
				
				if( EMI_PIC[siInstId].NbSubdirs == 0 ||
					GetAbort( siInstId ) != FALSE )
				{// All files transfered or operation aborted
					CompleteMaintenance( siInstId );
				}
			}
		}
	}
	else
	{
		EmiPicFichierDebug( siInstId, "TRANSFER : FindFirstFile returned invalid handle" );
	}

	FindClose(hFindFile);

	if( bFoundFiles != FALSE )
	{
		GetLocalTime( &sTimeStop );		
		GetTimeDiff( &sTimeStart, &sTimeStop, &sTimeDiff );

		EmiPicFichierTrace( siInstId,
			"TRANSFER : Found [%d] file(s), moved [%d] file(s), [%d] file(s) failed, total time [%02dh %02dmin %02dsec %02dmili]",
			lNbFilesFound,
			lNbFilesTransfered,
			lNbFilesFailed,
			sTimeDiff.wHour+((sTimeDiff.wDay-1)*24),
			sTimeDiff.wMinute,
			sTimeDiff.wSecond,
			sTimeDiff.wMilliseconds );
	}
}
/**/
/*****************************************************************************/
/*SYNTAX: PROTECTED void PurgeOldFiles(char *pDir, DWORD dwPurgePeriod)      */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function purges the pictures older than the date          */
/*            calculated by substracting the dwPurgeParameter parameter from */
/*            the system date.                                               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  char *pDir          - Directory that will be purged. The         */
/*                                procedure will purge subdirectories too.   */
/*      IN  long lPurgePeriod   - After number of days determined by this    */
/*                                parameter the picture will be deleted.     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function doesn't return a value.              */
/*****************************************************************************/
PROTECTED void PurgeOldFiles( char *pDir, long lPurgePeriod, LONG lPurgeFileDelay )
{
	HANDLE
		hFindFile,
		hFile;
	WIN32_FIND_DATA
		sFileData;
	CHAR
		szDir[MAX_PATH],
		szSubDir[MAX_PATH],
		szFileName[MAX_PATH];
	SYSTEMTIME
		sFileTime,
		sCurrentTime;
	FILETIME
		sTimeCreation,
		sTimeAccess,
		sTimeWrite;

	GetLocalTime(&sCurrentTime);

//	sprintf(szDir, "%s*", pDir);
	_makepath_s( szDir,sizeof(szDir), NULL, pDir, "*", NULL );
		
	hFindFile = FindFirstFile(szDir, &sFileData);
	
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Remove all file flags except directory attribute
			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;

			if( sFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				if((strcmp(sFileData.cFileName, ".") != 0) &&
					(strcmp(sFileData.cFileName, "..") != 0))
				{
//					sprintf(szSubDir, "%s%s\\", pDir, sFileData.cFileName);

					_makepath_s( szSubDir,sizeof(szSubDir), NULL, pDir, sFileData.cFileName, NULL );

					PurgeOldFiles( szSubDir, lPurgePeriod, lPurgeFileDelay );
					RemoveDirectory(szSubDir);
				}
			}
			else
			{
//				sprintf(szFileName, "%s%s", pDir, sFileData.cFileName);

				_makepath_s( szFileName,sizeof(szFileName), NULL, pDir, sFileData.cFileName, NULL );

				hFile = CreateFile( szFileName,
									GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL );

				if(GetFileTime(hFile, &sTimeCreation, &sTimeAccess, &sTimeWrite))
				{
					FileTimeToSystemTime(&sTimeCreation, &sFileTime);

					if((long)(DateToDays(&sCurrentTime) - DateToDays(&sFileTime)) > lPurgePeriod)
					{
						CloseHandle(hFile);
						if(DeleteFile(szFileName) == FALSE)
						{
							// Error delete file							
							// I don't have module instance here to write to log file...
						}
					}
					else
					{
						CloseHandle(hFile);
						hFile = INVALID_HANDLE_VALUE;
					}
				}
				else
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}
			// MFR start 2010/1/25
			Sleep( lPurgeFileDelay ); // DelaiTache(param);
			// MFR end
		}
		while(FindNextFile(hFindFile, &sFileData) != 0);
	}

	if( hFindFile != INVALID_HANDLE_VALUE )
	{
		FindClose( hFindFile );
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicGestionArretEmi(short siInstId)                         */
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
PRIVATE void EmiPicGestionArretEmi( short siInstId )
{
	EmiPicEmiEnvoiAni(siInstId, M_ARRET_EFFECTUE, NULL);
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_EMI : EmiPicGestionArretEmi ==> Arret de la tache EMI");

	EmiPicLibereBAL(EMI_PIC[siInstId].dwEmiBalId);
	Termine();
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
PRIVATE VOID SetStatus( IN short siInstId, IN enum_status sStatus, IN BOOL bValue )
{
	switch( sStatus )
	{
		case STATUS_LINK_ERR:
			EMI_PIC[siInstId].sStatus.errorLink = bValue;
			break;

		default:
			break;
	}

	if( memcmp( &(EMI_PIC[siInstId].sStatus), &(EMI_PIC[siInstId].sOldStatus), sizeof(struct_emi_pic_etat)) != 0 )
	{
		memcpy( &(EMI_PIC[siInstId].sOldStatus), &(EMI_PIC[siInstId].sStatus), sizeof(struct_emi_pic_etat) );
		EmiPicEmiEnvoiAni(siInstId, M_STATUS_CHANGED, NULL);
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
PROTECTED BOOL TransferFileToDestinations( short siInstId, LPSTR szFileSource, size_t uiFileSourceSize )
{
	INT
		iIndex;
	struct_string_data
		sStringData;
	CHAR
		szStorePath[MAX_PATH],
		szFileDestination[MAX_PATH],
		szTempFileDestination[MAX_PATH],
		szFileName[MAX_PATH],
		szFileSourceBuffer[MAX_PATH],
		szName[MAX_PATH],
		szExtension[MAX_PATH];
	BOOL
		bTransferedToAllDestinations = TRUE;

	_splitpath( szFileSource, NULL, NULL, szName, szExtension );

	// Create full file name
	_makepath_s( szFileName,sizeof(szFileName), NULL, NULL, szName, szExtension );

	if (GetDataFromString(szFileName, &(EMI_PIC[siInstId].sFileNameFormat), &sStringData) != TRUE)
	{
		EmiPicFichierTrace(siInstId, "TransferFileToDestinations: GetDataFromString for %s returned FALSE!", szFileSource);

		return FALSE;
	}

	for( iIndex = 0; iIndex <= EMI_PIC[siInstId].iNbDefinedDest; iIndex++ )
	{// For each configured destination
		EmiPicFichierTrace(siInstId, "TransferFileToDestinations: source file[%s] - Dest dir [%s]", szFileSource, EMI_PIC[siInstId].ConfigDest[iIndex].szDestinationDir);

		if( CompareStringToFilterList( szFileName, EMI_PIC[siInstId].ConfigDest[iIndex].lpFilterList ) == TRUE )
		{// File name matches one of the filters for this destination
			CreateDestinationPath( szStorePath, &(EMI_PIC[siInstId].ConfigDest[iIndex]), &sStringData );
			
			EmiPicFichierTrace(siInstId, "TransferFileToDestinations: Dest path: [%s]", szStorePath);

			if( MakeSureDirectoryPathExists( szStorePath ) != TRUE ||
				_access( szStorePath, FILE_EXISTANCE ) == FILE_NOT_FOUND )
			{
				bTransferedToAllDestinations = FALSE;
				SetStatus( siInstId, STATUS_LINK_ERR, TRUE );

				EmiPicFichierTrace( siInstId, "Error accessing remote storage path [%s]", szStorePath );

				continue;
			}

			if( EMI_PIC[siInstId].bUsingCounter != FALSE )
			{// There is transmission counter data in name, remove it from destination file name
				RemoveCounterData( szFileName );				
			}			

			// Create full file path with name
			_makepath_s( szFileDestination,sizeof(szFileDestination), NULL, szStorePath, szFileName, NULL );

			// Create temporary storage path
			if( strlen(EMI_PIC[siInstId].ConfigDest[iIndex].szTempExtension) != 0 )
			{// Copy with temporary extension
				_makepath_s( szTempFileDestination,sizeof(szTempFileDestination), NULL, szStorePath, szFileName, EMI_PIC[siInstId].ConfigDest[iIndex].szTempExtension );				
			}
			else
			{// Copy with final file name
				_makepath_s( szTempFileDestination,sizeof(szTempFileDestination), NULL, szStorePath, szFileName, NULL );				
			}
			
			// Check if file already exists on destination
			if( _access( szFileDestination, FILE_EXISTANCE ) == FILE_NOT_FOUND )
			{				
				if( CopyFile( szFileSource, szTempFileDestination, FALSE ) != TRUE )
				{
					bTransferedToAllDestinations = FALSE;
					SetStatus( siInstId, STATUS_LINK_ERR, TRUE );
					EmiPicFichierTrace( siInstId, "Error coping file to remote location [%s]", szStorePath );
				}
				else
				{// Transfered successfully	
					// Rename file (if copied with temporary file name)
					if( strlen(EMI_PIC[siInstId].ConfigDest[iIndex].szTempExtension) != 0 )
					{
						if( MoveFileEx( szTempFileDestination, szFileDestination, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) != TRUE )
						{
							bTransferedToAllDestinations = FALSE;
							
							SetStatus( siInstId, STATUS_LINK_ERR, TRUE );
							EmiPicFichierTrace( siInstId, "Error accessing remote file [%s]", szTempFileDestination );						
						}
					}

					if( EMI_PIC[siInstId].bUsingCounter != FALSE )
					{// Increase transmission counter
						strcpy_s( szFileSourceBuffer,sizeof(szFileSourceBuffer), szFileSource );
						IncreaseCounterData( szFileSourceBuffer );
						MoveFileEx( szFileSource, szFileSourceBuffer, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH );
						strcpy_s( szFileSource,uiFileSourceSize, szFileSourceBuffer );
					}
				}
			}
		}
		else
		{
			//File not matching dest filter
			EmiPicFichierTrace(siInstId, "TransferFileToDestinations: [%s] File not matching destinations filter!", szFileSource);
		}
	}

	if( bTransferedToAllDestinations != FALSE )
	{
		SetStatus( siInstId, STATUS_LINK_ERR, FALSE );		
	}

	return bTransferedToAllDestinations;
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
PRIVATE VOID CompleteMaintenance( short siInstId )
{
	// Maintenance is aborted, switch off and restore original configuration
	EMI_PIC[siInstId].bMainModeON = FALSE;
	
	// Restore original configuration
	memcpy( EMI_PIC[siInstId].ConfigDest, EMI_PIC[siInstId].BackupCopy, sizeof(struct_config_data)*MAX_DESTINATIONS );
	EMI_PIC[siInstId].dwChronoEmiPeriod = EMI_PIC[siInstId].dwChronoEmiPeriodBackup;
	
	EmiPicEmiEnvoiAni( siInstId, M_EMI_PIC_MAINT_EFFECTUE, (LPVOID)EMI_PIC[siInstId].NbFilesMaint );

	SetAbort( siInstId, FALSE );
	LaunchTimer( siInstId, CHRONO_REEMISSION, EMI_PIC[siInstId].dwRestitBalId, DEFAULT_DELAY );	
}
/*-------------------------------- END OF FILE ------------------------------*/