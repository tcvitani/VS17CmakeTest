/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_restit.c                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The restitution thread functions.                             */
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
	FIRST_RESULT		= 0,

	RESULT_COMPLETE			= FIRST_RESULT,
	RESULT_IN_PROGRESS,
	RESULT_ERROR,
	
	END_RESULT,

	LAST_RESULT	= END_RESULT - 1,
	NB_RESULTS	= END_RESULT - FIRST_RESULT, 
}enum_result;

typedef struct
{
	BOOL
		bFound;
	LONG
		lRecordId,
		lMaxRecordId,
		lMinRecordId;
	SYSTEMTIME
		sStartDate,
		sEndDate;
	LPFILEFOUND
		lpFirst,
		lpLast;	
}struct_search_data, *LPSEARCHDATA;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE short int InitEmiPicRestit(void *param);
PRIVATE void EmiPicRestitReceptionAni(short siInstId,
                                  struct_neutre *pNeutre);
PRIVATE VOID EmiPicRestitReceptionChrono( short siInstId,
										 struct_neutre *pNeutre );
PRIVATE void EmiPicRestitEnvoiAni(short siInstId,
                                  enum_emi_pic_internal_type eTypeMessage,
                                  void *pvParam);
PRIVATE void EmiPicGestionArretRestit(short siInstId);

PRIVATE enum_result ResendFilesByDateTime( short siInstId );
PRIVATE BOOL StartSearchForRecordByID( short siInstId, LONG lRecordID );
// MFR start 2010/1/13
PRIVATE VOID EmiPicRestitPurgeBackup( IN short siInstId );
// MFR end
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: DWORD WINAPI EmiPicRestit(PVOID pvParam)                           */
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
PROTECTED DWORD WINAPI EmiPicRestit(PVOID pvParam)
{
	struct_neutre *pNeutre;
	noyau_enum_retour eResult;
	short siInstId;

	// Thread initialisation (get instance id)
	siInstId = InitEmiPicRestit(pvParam);

	ChangePriorite(TacheCourante(), EMI_PIC[siInstId].dwPrioMax);

	// Wait and process the messages
	while(TRUE)
	{
		// Wait for the message in the EMI thread mailbox
		// (infinite timeout)
		eResult = Recoit(EMI_PIC[siInstId].dwRestitBalId,
						 (struct_neutre **)(&pNeutre), 
						 NOYAU_ATTENTE_INFINIE);

		// Protecting the message processing
		DebutRegion();

		if(eResult == NOYAU_BAL_MESS)
		{
			if(pNeutre->bl_retour == EMI_PIC[siInstId].dwAniBalId)
			{
				// Message received from the ANI thread
				EmiPicRestitReceptionAni(siInstId, pNeutre);
			}
			else if( pNeutre->bl_retour == NOYAU_BAL_HORLOGE_ID )
			{// Message received from the CHRONO
				EmiPicRestitReceptionChrono(siInstId, pNeutre);
			}
			else
			{
				// Undefined maibox
				EmiPicFichierDebug(siInstId,
								   "EMI_PIC_RESTIT ***** EmiPicRestit ==> Undefined mailbox %d *****",
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
/*SYNTAX: short int InitEmiPicRestit(void *pvParam)                          */
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
PRIVATE short int InitEmiPicRestit(void *pvParam)
{
	short siInstId;

	// Get instance id
	siInstId = (short)pvParam;

	

 	// publication of 'EMI_PIC' internal mailbox
	EMI_PIC[siInstId].dwRestitBalId = PublieBAL(EMI_PIC[siInstId].szRestitBalName, 0);
	if(EMI_PIC[siInstId].dwRestitBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by EMI thread)
	EMI_PIC[siInstId].dwEmiBalId = AttendBAL(EMI_PIC[siInstId].szEmiBalName);
	if(EMI_PIC[siInstId].dwEmiBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by TRF thread)
	EMI_PIC[siInstId].dwTrfBalId = AttendBAL(EMI_PIC[siInstId].szTrfBalName);
	if(EMI_PIC[siInstId].dwTrfBalId <= 0)
		ExitBad();

	// Waiting for the external mailbox publication (by ANI thread)
	EMI_PIC[siInstId].dwAniBalId = AttendBAL(EMI_PIC[siInstId].szAniBalName);
	if(EMI_PIC[siInstId].dwAniBalId <= 0)
		ExitBad();

	return (siInstId);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicRestitReceptionAni(short siInstId,                      */
/*                                      struct_neutre *pNeutre)              */
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
PRIVATE void EmiPicRestitReceptionAni(short siInstId,
                                      struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;

	// Type of the message
	switch(pMessage->entete.type_message)
	{
		// MFR start 23/7/2008
		case M_RESTIT_DATETIME_REQUEST:
			memcpy(	&(EMI_PIC[siInstId].sResendFrom),
					&(pMessage->u.sAniRestit.sRestit.u.sRequestByDateTime.sBegin),
					sizeof( SYSTEMTIME ) );

			memcpy(	&(EMI_PIC[siInstId].sResendTo),
					&(pMessage->u.sAniRestit.sRestit.u.sRequestByDateTime.sEnd),
					sizeof( SYSTEMTIME ) );

			LaunchTimer( siInstId, CHRONO_REEMISSION, EMI_PIC[siInstId].dwRestitBalId, 1 );
			break;

		case M_RESTIT_RECORD_ID_REQUEST:
			if( StartSearchForRecordByID( siInstId, pMessage->u.sAniRestit.sRestit.u.sRequestByRecordID.lRecordID ) != FALSE )
				EmiPicRestitEnvoiAni( siInstId, M_RESTIT_RECORD_ID_EFFECTUE, NULL );
			else
				EmiPicRestitEnvoiAni( siInstId, M_RESTIT_RECORD_ID_NON_EFFECTUE, NULL );
				
			break;
		// MFR end   23/7/2008
		// MFR start 2010/1/11
		case M_RESTIT_PURGE_BACKUP:
			EMI_PIC[siInstId].bPurgeInProgress = TRUE;
			EmiPicRestitPurgeBackup( siInstId );			
			break;		
		// MFR end
		case M_ARRET_DEMANDE:
			EmiPicGestionArretRestit(siInstId);
			break;

		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_RESTIT ***** EmiPicRestitReceptionAni ==> type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
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
PRIVATE VOID EmiPicRestitReceptionChrono( short siInstId,
							 struct_neutre *pNeutre )
{
	struct_chrono
		*pMessage = (struct_chrono *)pNeutre;

	enum_result
		enResult;

	if( pMessage->numero == EMI_PIC[siInstId].sChronoData[CHRONO_REEMISSION].chrono_id )
	{
		enResult = ResendFilesByDateTime( siInstId );

		switch( enResult )
		{
			case RESULT_COMPLETE:
				// Report complete
				EmiPicRestitEnvoiAni( siInstId, M_RESTIT_DATETIME_EFFECTUE, NULL );				
				break;
				
			case RESULT_IN_PROGRESS:
				// Set timer for next cycle
				LaunchTimer( siInstId, CHRONO_REEMISSION, EMI_PIC[siInstId].dwRestitBalId, DEFAULT_DELAY );
				break;
				
			case RESULT_ERROR:
				// Report error
				EmiPicRestitEnvoiAni( siInstId, M_RESTIT_DATETIME_NON_EFFECTUE, NULL );				
				break;
				
			default:
				break;
		}
	}
	else
	{
		EmiPicFichierDebug(siInstId,
			"EMI_PIC_EMI ***** EmiPicRestitReceptionChrono ==> undefined chrono %d *****",
			pMessage->numero);
	}
}
/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicRestitEnvoiAni(short siInstId,                          */
/*                                  enum_emi_pic_type eTypeMessage,          */
/*                                  void *vParam)                            */
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
PRIVATE void EmiPicRestitEnvoiAni(short siInstId,
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
		case M_ARRET_EFFECTUE:
			break;

		default:
			break;
	}

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_RESTIT : EmiPicRestitEnvoiAni ==> Type_message '%d'",
					   pMessage->entete.type_message);

	// Send the message
	ExitEnvoie(EMI_PIC[siInstId].dwAniBalId,
			   EMI_PIC[siInstId].dwRestitBalId,
			   (struct_neutre *)pMessage);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicGestionArretRestit(short siInstId)                      */
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
PRIVATE void EmiPicGestionArretRestit(short siInstId)
{
	EmiPicRestitEnvoiAni(siInstId, M_ARRET_EFFECTUE, NULL);
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_RESTIT : EmiPicGestionArretRestit ==> Arret de la tache RESTIT");

	EmiPicLibereBAL(EMI_PIC[siInstId].dwRestitBalId);
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
PROTECTED VOID IncreaseByFormatTimeType( OUT LPSYSTEMTIME lpSystemTime, IN enum_format_element enFormatElement )
{
	FILETIME
		ftFileTime;
	LONGLONG
		llTime;
	
	switch( enFormatElement )
	{
		case ELEMENT_YEAR:
			lpSystemTime->wYear++;
			break;

		case ELEMENT_MONTH:
			lpSystemTime->wMonth++;

			if( lpSystemTime->wMonth > 12 )
			{
				lpSystemTime->wMonth = 1;
				lpSystemTime->wYear++;
			}
			break;

		case ELEMENT_DAY:
		case ELEMENT_HOUR:
		case ELEMENT_MINUTE:
		case ELEMENT_SECOND:
		case ELEMENT_MILLISECOND:			
			SystemTimeToFileTime( lpSystemTime, &ftFileTime );
			llTime = *((LONGLONG*)&ftFileTime);			
			
			switch( enFormatElement )
			{
				case ELEMENT_DAY:
					llTime += (LONGLONG)36000000000*(LONGLONG)24;					
					break;
				case ELEMENT_HOUR:
					llTime += (LONGLONG)36000000000;
					break;
				case ELEMENT_MINUTE:
					llTime += (LONGLONG)600000000;
					break;
				case ELEMENT_SECOND:
					llTime += (LONGLONG)10000000;
					break;
				case ELEMENT_MILLISECOND:
					llTime += (LONGLONG)10000;
					break;
			}

			ftFileTime = *((FILETIME*)&llTime);
			FileTimeToSystemTime( &ftFileTime, lpSystemTime );
			break;

		default:
			break;
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
PROTECTED VOID DecreaseByFormatTimeType( OUT LPSYSTEMTIME lpSystemTime, IN enum_format_element enFormatElement )
{
	FILETIME
		ftFileTime;
	LONGLONG
		llTime;
	
	switch( enFormatElement )
	{
	case ELEMENT_YEAR:
		lpSystemTime->wYear--;
		break;
		
	case ELEMENT_MONTH:
		lpSystemTime->wMonth--;
		
		if( lpSystemTime->wMonth < 1 )
		{
			lpSystemTime->wMonth = 12;
			lpSystemTime->wYear--;
		}
		break;
		
	case ELEMENT_DAY:
	case ELEMENT_HOUR:
	case ELEMENT_MINUTE:
	case ELEMENT_SECOND:
	case ELEMENT_MILLISECOND:			
		SystemTimeToFileTime( lpSystemTime, &ftFileTime );
		llTime = *((LONGLONG*)&ftFileTime);			
		
		switch( enFormatElement )
		{
		case ELEMENT_DAY:
			llTime -= (LONGLONG)36000000000*(LONGLONG)24;					
			break;
		case ELEMENT_HOUR:
			llTime -= (LONGLONG)36000000000;
			break;
		case ELEMENT_MINUTE:
			llTime -= (LONGLONG)600000000;
			break;
		case ELEMENT_SECOND:
			llTime -= (LONGLONG)10000000;
			break;
		case ELEMENT_MILLISECOND:
			llTime -= (LONGLONG)10000;
			break;
		}
		
		ftFileTime = *((FILETIME*)&llTime);
		FileTimeToSystemTime( &ftFileTime, lpSystemTime );
		break;
		
		default:
			break;
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
PRIVATE VOID DecreaseTimeByFormat( OUT LPSYSTEMTIME lpDateTime, IN LP_FORMAT_DATA lpFormatData )
{
	INT
		iIndex;
	SYSTEMTIME
		sDateTime;
	
	memcpy( &sDateTime, lpDateTime, sizeof(SYSTEMTIME) );
	
	for( iIndex = ELEMENT_MILLISECOND; iIndex >= ELEMENT_YEAR; iIndex-- )
	{
		if( lpFormatData->Element[iIndex].bIsUsed )
		{
			DecreaseByFormatTimeType( lpDateTime, iIndex );
			return;
		}
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
PRIVATE VOID IncreaseTimeByFormat( OUT LPSYSTEMTIME lpDateTime, IN LP_FORMAT_DATA lpFormatData )
{
	INT
		iIndex;
	SYSTEMTIME
		sDateTime;

	memcpy( &sDateTime, lpDateTime, sizeof(SYSTEMTIME) );

	for( iIndex = ELEMENT_MILLISECOND; iIndex >= ELEMENT_YEAR; iIndex-- )
	{
		if( lpFormatData->Element[iIndex].bIsUsed )
		{
			IncreaseByFormatTimeType( lpDateTime, iIndex );
			return;
		}
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
PRIVATE INT CompareTimesByFormat( LPSYSTEMTIME lpFirst, LPSYSTEMTIME lpSecond, LP_FORMAT_DATA lpFormat )
{
	SYSTEMTIME
		sSystemTime	= { 0 };

	LPSYSTEMTIME
		lpCurrentSource;

	FILETIME
		ftFileTime;

	LONGLONG
		llFirst,
		llSecond;

	LONGLONG*
		lpCurrentValue;

	INT
		i,
		j;

	for( i = 0; i < 2; i++ )
	{
		if( i == 0 )
		{
			lpCurrentSource = lpFirst;
			lpCurrentValue = &llFirst;
		}
		else
		{
			lpCurrentSource = lpSecond;
			lpCurrentValue = &llSecond;
		}

		for( j = 0; j < NB_FORMAT_ELEMENTS; j++ )
		{
			switch( j )
			{
				case ELEMENT_YEAR:
					sSystemTime.wYear = lpCurrentSource->wYear;
					break;

				case ELEMENT_MONTH:
					sSystemTime.wMonth = lpCurrentSource->wMonth;
					break;
					
				case ELEMENT_DAY:
					sSystemTime.wDay = lpCurrentSource->wDay;
					break;
					
				case ELEMENT_HOUR:
					sSystemTime.wHour = lpCurrentSource->wHour;
					break;
					
				case ELEMENT_MINUTE:
					sSystemTime.wMinute = lpCurrentSource->wMinute;
					break;
					
				case ELEMENT_SECOND:
					sSystemTime.wSecond = lpCurrentSource->wSecond;
					break;
					
				case ELEMENT_MILLISECOND:
					sSystemTime.wMilliseconds = lpCurrentSource->wMilliseconds;
					break;
					
				default:
					break;
			}
		}

		SystemTimeToFileTime( &sSystemTime, &ftFileTime );
		(*lpCurrentValue) = *((LONGLONG*)&ftFileTime);
	}
	
	if( llFirst < llSecond )
		return -1;
	else if( llFirst > llSecond )
		return 1;
	else
		return 0;
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
PRIVATE BOOL FilterFoundFiles( short siInstId, LPFILEFOUND* lpFiles )
{
	LPFILEFOUND
		lpFirst		= NULL,
		lpCurrent	= (*lpFiles),
		lpLast		= NULL,
		lpToDelete	= NULL;

	struct_string_data
		sStringData;

	SYSTEMTIME
		sFileData;

	INT
		iCompareFrom,
		iCompareTo;

	while( lpCurrent != NULL )
	{
		
		GetDataFromString(	lpCurrent->szFileName,
							&(EMI_PIC[siInstId].sFileNameFormat),
							&sStringData );

		FillDateTimeWithStringData( &sFileData, &sStringData );
		
		iCompareFrom = CompareTimesByFormat( &(EMI_PIC[siInstId].sResendFrom), &sFileData, &EMI_PIC[siInstId].sFileNameFormat );
		iCompareTo = CompareTimesByFormat( &(EMI_PIC[siInstId].sResendTo), &sFileData, &EMI_PIC[siInstId].sFileNameFormat );

		// If file date is lower then start of interval or greater than end of interval
		if( (iCompareFrom	> 0 ) ||
			(iCompareTo		< 0 )	)
		{// Remove file from list
			if( lpLast != NULL )
			{
				lpLast->lpNextFile = lpCurrent->lpNextFile;
				lpToDelete = lpCurrent;
			}
		}
		else
		{
			if( lpFirst == NULL )
				lpFirst = lpCurrent;
		}

		// Move to next
		lpCurrent = lpCurrent->lpNextFile;

		// If current member is set to be delete it
		if( lpToDelete != NULL )
		{
			lpToDelete->lpNextFile = NULL;
			free( lpToDelete );
			lpToDelete = NULL;
		}
	}

	(*lpFiles) = lpFirst;

	if( lpFirst != NULL )
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
PRIVATE VOID SendFilesFromDir( short siInstId, LPSTR szDirPath )
{
	LPFILEFOUND
		lpFiles		= NULL,
		lpToDelete	= NULL;
	CHAR
		szFilePath[MAX_PATH],
		szName[MAX_PATH],
		szExtension[MAX_PATH];
	
	if( GetFilesFromDir( siInstId, szDirPath, &lpFiles, "*.*" ) != TRUE )
		return;

	// Fils found in current dir
	if( FilterFoundFiles( siInstId, &lpFiles ) != TRUE )
		return;

	// Send files that passed trough filter
	while( lpFiles != NULL )
	{
		_makepath_s( szFilePath,sizeof(szFilePath), NULL, szDirPath, lpFiles->szFileName, NULL );
		
		TransferFileToDestinations( siInstId, szFilePath, sizeof(szFilePath) );

		_splitpath( szFilePath, NULL, NULL, szName, szExtension );
		_makepath_s( lpFiles->szFileName,sizeof(lpFiles->szFileName), NULL, NULL, szName, szExtension );

		// Prepare to delete node
		lpToDelete = lpFiles;

		lpFiles = lpFiles->lpNextFile;

		// Delete and clean node
		lpToDelete->lpNextFile = NULL;
		free( lpToDelete );
		lpToDelete = NULL;
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
PRIVATE enum_result ResendFilesByDateTime( short siInstId )
{
	struct_string_data
		sStringData				= { 0 };

	CHAR
		szOutString[MAX_PATH],
		szDirPath[MAX_PATH],
		szFormatedPath[MAX_PATH];


	FillStringDataWithDateTime( &sStringData, &(EMI_PIC[siInstId].sResendFrom) );
	CreateStringByFormat( szFormatedPath, &EMI_PIC[siInstId].ConfigBackup.sFormatData, &sStringData );

	_makepath_s( szDirPath,sizeof(szDirPath), NULL, EMI_PIC[siInstId].ConfigBackup.szDestinationDir, NULL, NULL );
	strcpy_s( szOutString,sizeof(szOutString), szDirPath );
	_makepath_s( szDirPath,sizeof(szDirPath), NULL, szOutString, szFormatedPath, NULL );
	strcpy_s( szOutString,sizeof(szOutString), szDirPath );
	_makepath_s( szDirPath,sizeof(szDirPath), NULL, szOutString, NULL, NULL );

	if( _access( szDirPath,FILE_EXISTANCE ) != FILE_NOT_FOUND )
	{// Dir that matches time interval is found, go trough the files in it
		SendFilesFromDir( siInstId, szDirPath );		
	}
	
	IncreaseTimeByFormat( &(EMI_PIC[siInstId].sResendFrom), &EMI_PIC[siInstId].ConfigBackup.sFormatData );

	if( CompareTimesByFormat( &(EMI_PIC[siInstId].sResendFrom), &(EMI_PIC[siInstId].sResendTo), &EMI_PIC[siInstId].ConfigBackup.sFormatData ) > 0 )
		return RESULT_COMPLETE;
	else
		return RESULT_IN_PROGRESS;
}

/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/* Function is searching available data for the given time range and is 	 */
/* modifying the time range to fit available data.							 */
/* For example, if given time range range is from 1/1/2008 to 1/2/2008,		 */ 
/* function will start to search for existing directories with data that was */
/* created at 1/1/2008. If in our example data is available from 5/1/2008 to */
/* 8/2/2008 function will modify search dates to those vales. 				 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  There is data available for queried time range	 */
/*  FALSE                 There is no data available for queried time range	 */
/*****************************************************************************/
PRIVATE BOOL FindAvailableDataRangeForTimeRange( short siInstId, LPSEARCHDATA lpSearchData )
{
	CHAR
		szDirPath[MAX_PATH]	= { 0 };
	struct_string_data
		sStringData			= { 0 };
	BOOL
		bMarginFound;
	INT
		iMargin	= 0;		// 0 = left margin, 1 = right margin
	LPSYSTEMTIME
		lpTime;
	

	// Search for margins
	while( iMargin < 2 )
	{
		bMarginFound = FALSE;

		switch( iMargin )
		{
			case 0:
				lpTime = &(lpSearchData->sStartDate);
				break;
		
			case 1:
				lpTime =&(lpSearchData->sEndDate);
				break;
		
			default:
				break;
		}

		// While start date is smaller than end date
		while( CompareTimesByFormat( &(lpSearchData->sStartDate), &(lpSearchData->sEndDate), &(EMI_PIC[siInstId].ConfigBackup.sFormatData) ) == -1 )
		{
			FillStringDataWithDateTime( &sStringData, lpTime );
			CreateDestinationPath( szDirPath, &(EMI_PIC[siInstId].ConfigBackup), &sStringData );

			if( _access( szDirPath, FILE_EXISTANCE ) != FILE_NOT_FOUND )
			{
				bMarginFound = TRUE;
				break;
			}

			if( iMargin == 0 )
				IncreaseTimeByFormat( lpTime, &(EMI_PIC[siInstId].ConfigBackup.sFormatData) );
			else
				DecreaseTimeByFormat( lpTime, &(EMI_PIC[siInstId].ConfigBackup.sFormatData) );
		}

		if( bMarginFound != TRUE )
			return FALSE;

		iMargin++;
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
PRIVATE VOID PrepareTimesForRecordIdSearch( short siInstId, LPSEARCHDATA lpSearchData )
{
	SYSTEMTIME
		sSystemTimeNow,
		sSystemTimeMin;
	FILETIME
		sFileTimeNow,
		sFileTimeMin;
	LONGLONG
		llTimeNow,
		llTimeMin;


	GetSystemTime( &sSystemTimeNow );

	SystemTimeToFileTime( &sSystemTimeNow, &sFileTimeNow );
	llTimeNow = *((LONGLONG*)&sFileTimeNow);

	llTimeMin = (LONGLONG)llTimeNow - (LONGLONG)((LONGLONG)36000000000*(LONGLONG)24*(LONGLONG)EMI_PIC[siInstId].dwPurgePeriod );

	sFileTimeMin = *((FILETIME*)&llTimeMin);
	FileTimeToSystemTime( &sFileTimeMin, &sSystemTimeMin );

	memcpy( &(lpSearchData->sStartDate), &sSystemTimeMin, sizeof( SYSTEMTIME ) );
	memcpy( &(lpSearchData->sEndDate), &sSystemTimeNow, sizeof( SYSTEMTIME ) );
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
PRIVATE BOOL FindRecordIdRangeInTimeRange( short siInstId, LPSEARCHDATA lpSearchData  )
{
	struct_string_data
		sStringData	= { 0 };
	CHAR
		szDirPath[MAX_PATH];
	LPFILEFOUND
		lpFiles,
		lpToDelete,
		lpLast		= NULL,
		lpTemp;
	BOOL
		bMoveToNext;
	INT
		iStep = 0;
	
	lpSearchData->lMaxRecordId = 0;

	memset( &(lpSearchData->lMinRecordId ), 0xFF, sizeof( LONG ) - 1 );

	// Step 0 will search end of range, step 1 will search start of range
	while( iStep < 2 )
	{
		switch( iStep )
		{
			case 0:
				FillStringDataWithDateTime( &sStringData, &(lpSearchData->sEndDate) );				
				break;
		
			case 1:
				FillStringDataWithDateTime( &sStringData, &(lpSearchData->sStartDate) );				
				break;
		
			default:
				break;
		}
		
		CreateDestinationPath( szDirPath, &(EMI_PIC[siInstId].ConfigBackup), &sStringData );

		if( _access( szDirPath, FILE_EXISTANCE ) == FILE_NOT_FOUND )
			return FALSE;

		if( GetFilesFromDir( siInstId, szDirPath, &lpFiles, "*.*" ) != TRUE )
			return FALSE;

		while( lpFiles != NULL )
		{
			// Set current node to be deleted
			lpToDelete = lpFiles;
			bMoveToNext = TRUE;
			
			if( GetDataFromString( lpFiles->szFileName,  &(EMI_PIC[siInstId].sFileNameFormat), &sStringData ) != FALSE )
			{
				// Check if this is the record we are looking for
				if( sStringData.lValue[ELEMENT_RECORD_ID] == lpSearchData->lRecordId )
				{
					lpSearchData->bFound = TRUE;

					lpTemp = lpFiles->lpNextFile;

					if( lpSearchData->lpLast == NULL )
						lpSearchData->lpLast = lpFiles;
					else
					{
						lpSearchData->lpLast->lpNextFile = lpFiles;
						lpSearchData->lpLast = lpFiles;
					}

					if( lpSearchData->lpFirst == NULL )
						lpSearchData->lpFirst = lpFiles;

					// Terminate list of results
					lpSearchData->lpLast->lpNextFile = NULL;

					// Do not delete this node, it is part of result
					lpToDelete = NULL;
					// Move to next node here and set flag not to move
					lpFiles = lpTemp;
					bMoveToNext = FALSE;
				}

				if( sStringData.lValue[ELEMENT_RECORD_ID] > lpSearchData->lMaxRecordId )
					lpSearchData->lMaxRecordId = sStringData.lValue[ELEMENT_RECORD_ID];

				if( sStringData.lValue[ELEMENT_RECORD_ID] < lpSearchData->lMinRecordId )
					lpSearchData->lMinRecordId = sStringData.lValue[ELEMENT_RECORD_ID];				
			}

			// Move to next node
			if( bMoveToNext != FALSE )
				lpFiles = lpFiles->lpNextFile;

			// If current node is set to be deleted
			if( lpToDelete != NULL )
			{// Free node
				free( lpToDelete );
				lpToDelete = NULL;
			}
		}

		iStep++;
	}

	if( lpSearchData->lMaxRecordId >= lpSearchData->lRecordId &&
		lpSearchData->lMinRecordId <= lpSearchData->lRecordId )
	{
		return TRUE;
	}
	else
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
PROTECTED VOID GetMiddleTime( IN LPSYSTEMTIME lpFirst, IN LPSYSTEMTIME lpSecond, OUT LPSYSTEMTIME lpResult )
{
	
	FILETIME
		sFileTimeFirst,
		sFileTimeSecond,
		sFileTimeResult;
	LONGLONG
		llTimeFirst,
		llTimeSecond,
		llTimeResult;
	
	SystemTimeToFileTime( lpFirst, &sFileTimeFirst );
	SystemTimeToFileTime( lpSecond, &sFileTimeSecond );


	llTimeFirst = *((LONGLONG*)&sFileTimeFirst);
	llTimeSecond = *((LONGLONG*)&sFileTimeSecond);

	if( llTimeFirst > llTimeSecond )
		llTimeResult = llTimeSecond + ((llTimeFirst - llTimeSecond) / 2 );
	else
		llTimeResult = llTimeFirst + ((llTimeSecond - llTimeFirst) / 2 );
	
	
	sFileTimeResult = *((FILETIME*)&llTimeResult);
	FileTimeToSystemTime( &sFileTimeResult, lpResult );
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
VOID SendFoundFiles( short siInstId, LPFILEFOUND lpFile)
{
	LPFILEFOUND
		lpToDelete;

	struct_string_data
		sStringData;

	CHAR
		 szOutString[MAX_PATH],
		 szDrive[MAX_PATH],
		 szDir[MAX_PATH],
		 szFilePath[MAX_PATH],
		 szName[MAX_PATH],
		 szExtension[MAX_PATH];

	while( lpFile != NULL )
	{
		GetDataFromString(  lpFile->szFileName, &(EMI_PIC[siInstId].sFileNameFormat), &sStringData );		

		CreateDestinationPath( szOutString, &EMI_PIC[siInstId].ConfigBackup, &sStringData );
		
		_splitpath( szOutString, szDrive, szDir, NULL, NULL );
		_makepath_s( szOutString,sizeof(szOutString), szDrive, szDir, NULL, NULL );

		_makepath_s( szFilePath,sizeof(szFilePath), NULL, szOutString, lpFile->szFileName, NULL );
		TransferFileToDestinations( siInstId, szFilePath, sizeof(szFilePath) );

		_splitpath( szFilePath, NULL, NULL, szName, szExtension );
		_makepath_s( lpFile->szFileName,sizeof(lpFile->szFileName), NULL, NULL, szName, szExtension );
		
		lpToDelete = lpFile;
		lpFile = lpFile->lpNextFile;
		free( lpToDelete );
		lpToDelete = NULL;
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
PRIVATE BOOL StartSearchForRecordByID( short siInstId, LONG lRecordID )
{
	struct_search_data
		sSearchData	= { 0 };

	SYSTEMTIME
		sStartDate,
		sEndDate,
		sMidDate	= { 0 };

	INT
		iStep;

	 PrepareTimesForRecordIdSearch( siInstId, &sSearchData );

	 if( FindAvailableDataRangeForTimeRange( siInstId, &sSearchData ) != TRUE  )
		 return FALSE;

	 sSearchData.lRecordId = lRecordID;

	 if( FindRecordIdRangeInTimeRange( siInstId, &sSearchData ) != TRUE )
	 {// Record ID is not present in records
		 return FALSE;
	 }
	 else
	 {
		 if( sSearchData.bFound == TRUE )
		 {// Send files
			 SendFoundFiles( siInstId, sSearchData.lpFirst );
			 return TRUE;
		 }
	 }

	 memcpy( &sStartDate, &sSearchData.sStartDate, sizeof( SYSTEMTIME ) );
	 memcpy( &sEndDate, &sSearchData.sEndDate, sizeof( SYSTEMTIME ) );

	 GetMiddleTime( &sSearchData.sStartDate, &sSearchData.sEndDate, &sMidDate );

	 while( TRUE )
	 {
		 iStep = 0;

		 while( iStep < 2 )
		 {
			 memset( &sSearchData, 0, sizeof( struct_search_data ) );
			 sSearchData.lRecordId = lRecordID;
			 
			 switch( iStep )
			 {
			 	case 0:
					memcpy( &sSearchData.sStartDate, &sStartDate, sizeof( SYSTEMTIME ) );
					memcpy( &sSearchData.sEndDate, &sMidDate, sizeof( SYSTEMTIME ) );
			 		break;
			 
			 	case 1:
					memcpy( &sSearchData.sStartDate, &sMidDate, sizeof( SYSTEMTIME ) );
					memcpy( &sSearchData.sEndDate, &sEndDate, sizeof( SYSTEMTIME ) );
			 		break;
			 
			 	default:
			 		break;
			 }

			 if( FindRecordIdRangeInTimeRange( siInstId, &sSearchData ) != FALSE )
			 {// Record ID found in range
				 if( sSearchData.bFound != TRUE )
				 {// Go into new search
					 memcpy( &sStartDate, &sSearchData.sStartDate, sizeof( SYSTEMTIME ) );
					 memcpy( &sEndDate, &sSearchData.sEndDate, sizeof( SYSTEMTIME ) );

					 GetMiddleTime( &sSearchData.sStartDate, &sSearchData.sEndDate, &sMidDate );

					 iStep = 0;
				 }
				 else
				 {// Send found files
					 SendFoundFiles( siInstId, sSearchData.lpFirst );
					 return TRUE;
				 }
			 }
		 }
	 }
}
// MFR start 2010/1/11
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
PRIVATE BOOL DeleteDirContent( IN short siInstId, IN CONST LPSTR szDir )
{
	HANDLE
		hFindFile;
	WIN32_FIND_DATA
		sFileData;
	CHAR
		szSubDir[MAX_PATH],
		szSearchMask[MAX_PATH];
	BOOL
		bFound,
		bRet;
	LPFILEFOUND
		lpNewFile,
		lpHead		= NULL,
		lpLast		= NULL,
		lpToDelete;
	enum_data_type
		enDataType;
	
	bRet = TRUE;

	_makepath_s( szSearchMask,sizeof(szSearchMask), NULL, szDir, "*", NULL );

	hFindFile = FindFirstFile( szSearchMask, &sFileData );
	
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			bFound = TRUE;
			// Remove all file flags except directory attribute
			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
			
			if( sFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				if( ( strcmp( sFileData.cFileName, "." ) != 0 ) &&
					( strcmp( sFileData.cFileName, ".." ) != 0 ) )
				{// Valid directory
					// Delete subdir content
					_makepath_s( szSubDir,sizeof(szSubDir), NULL, szDir, sFileData.cFileName, NULL );
					
					if( DeleteDirContent( siInstId, szSubDir ) != TRUE )
					{
						bRet = FALSE;
						break;
					}
				}
				else
				{// Not valid result to delete
					bFound = FALSE;
				}

				enDataType = DATA_TYPE_DIR;

			}
			else
			{
				enDataType = DATA_TYPE_FILE;
			}

			if( bFound != FALSE )
			{// Found dir or file
				lpNewFile = calloc( 1, sizeof( struct_file_found ) );

				lpNewFile->enDataType = enDataType;
				_makepath_s( lpNewFile->szFileName,sizeof(lpNewFile->szFileName), NULL, szDir, sFileData.cFileName, NULL );

				if( lpHead != NULL )
				{
					lpLast->lpNextFile = lpNewFile;
					lpLast = lpNewFile;
				}
				else
				{
					lpHead = lpNewFile;
					lpLast = lpNewFile;
				}
			}

		}while( FindNextFile(hFindFile, &sFileData) != 0 );
		
		FindClose( hFindFile );
		hFindFile = NULL;
	}


	while( lpHead != NULL )
	{
		lpToDelete = lpHead;

		lpHead = lpHead->lpNextFile;

		if( bRet != FALSE )
		{
			switch( lpToDelete->enDataType )
			{
				case DATA_TYPE_FILE:
					if( DeleteFile( lpToDelete->szFileName ) == FALSE )
					{// Error deleting file
						bRet = FALSE;
						EmiPicFichierTrace( siInstId, "Failed to delete file [%s]", lpToDelete->szFileName );
					}
					break;

				case DATA_TYPE_DIR:
					if( RemoveDirectory( lpToDelete->szFileName ) == FALSE )
					{// Error deleting directory
						bRet = FALSE;
						EmiPicFichierTrace( siInstId, "Failed to delete directory [%s]", lpToDelete->szFileName );
					}
					break;

				default:
					// Something very, very wrong
					bRet = FALSE;
					EmiPicFichierTrace( siInstId, "DeleteDirContent()->Unsuported type [%d]", lpToDelete->enDataType );
					break;
			}
			
			// MFR start 2010/1/25
			Sleep( EMI_PIC[siInstId].dwPurgeFileDelay );
			// MFR end
		}

		// Free node
		lpToDelete->lpNextFile = NULL;
		free( lpToDelete );
		lpToDelete = NULL;
	}

	return bRet;
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
PRIVATE VOID DeleteByDirData( IN short siInstId, IN CONST LPSTR szDir, IN LONG lPurgePeriod )
{
	HANDLE
		hFindFile;
	WIN32_FIND_DATA
		sFileData;
	CHAR
		szSearchMask[MAX_PATH],
		szSubDir[MAX_PATH];
	LONG
		iIndex;
		
	_makepath_s( szSearchMask,sizeof(szSearchMask), NULL, szDir, "*", NULL );

	hFindFile = FindFirstFile( szSearchMask, &sFileData );

	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Remove all file flags except directory attribute
			sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
			
			if( sFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				if( ( strcmp( sFileData.cFileName, "." ) != 0 ) &&
					( strcmp( sFileData.cFileName, ".." ) != 0 ) )
				{
					_makepath_s( szSubDir,sizeof(szSubDir), NULL, szDir, sFileData.cFileName, NULL );

					iIndex = (LONG)strlen( EMI_PIC[siInstId].ConfigBackup.szDestinationDir );					
					if( szSubDir[iIndex] == '\\' )
						iIndex++;
					

					if( IsOkToRemoveDir( siInstId, &(szSubDir[iIndex]), lPurgePeriod ) )
					{// Delete all contents then actual dir
						if( DeleteDirContent( siInstId, szSubDir ) != FALSE )
						{
							if( RemoveDirectory( szSubDir ) == FALSE )
							{
								EmiPicFichierTrace( siInstId,
									"DeleteByDirData()->Failed to delete directory [%s]",
									szSubDir );
							}
						}
					}
					else
					{// Check subdirs
						DeleteByDirData( siInstId, szSubDir, lPurgePeriod );
					}
				}
			}				
		}while( FindNextFile( hFindFile, &sFileData) != 0 );
	}

	FindClose(hFindFile);
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
PRIVATE VOID EmiPicRestitPurgeBackup( IN short siInstId )
{
	CHAR
		szBuffer[MAX_PATH];
	SYSTEMTIME
		sCurrentTime;
	
	GetLocalTime( &sCurrentTime );

	// Save the last purge date and time
	sprintf_s(szBuffer,sizeof(szBuffer),
		"%d.%d.%d %d:%d",
		sCurrentTime.wDay,
		sCurrentTime.wMonth,
		sCurrentTime.wYear,
		sCurrentTime.wHour,
		sCurrentTime.wMinute);

	EmiPicFichierTrace( siInstId, "Purge started [%s]", szBuffer );

	
	if( EMI_PIC[siInstId].ConfigBackup.sFormatData.Element[ELEMENT_YEAR].bIsUsed &&
		EMI_PIC[siInstId].ConfigBackup.sFormatData.Element[ELEMENT_MONTH].bIsUsed &&
		EMI_PIC[siInstId].ConfigBackup.sFormatData.Element[ELEMENT_DAY].bIsUsed )
	{// Backup dir name contains year, month and day data
		FinRegion();
		DeleteByDirData( siInstId, EMI_PIC[siInstId].ConfigBackup.szDestinationDir, (long)EMI_PIC[siInstId].dwPurgePeriod );
		DebutRegion();
	}
	else
	{// File properties contain year, month and day data
		FinRegion();
		PurgeOldFiles( EMI_PIC[siInstId].ConfigBackup.szDestinationDir, (long)EMI_PIC[siInstId].dwPurgePeriod, EMI_PIC[siInstId].dwPurgeFileDelay );
		DebutRegion();
	}

	
	REG_Ecrire_Chaine(CSR_REG_KEYi_ROOT,
		EMI_PIC[siInstId].szKey,
		EMI_PIC_REG_KEYv_LAST_PURGE_TIME,
		szBuffer);
	
	memcpy(&EMI_PIC[siInstId].sLastPurgeDTime, &sCurrentTime, sizeof(SYSTEMTIME));

	EMI_PIC[siInstId].bPurgeInProgress = FALSE;

	// Trace purge and time
	GetLocalTime( &sCurrentTime );
	
	sprintf_s(szBuffer,sizeof(szBuffer),
		"%d.%d.%d %d:%d",
		sCurrentTime.wDay,
		sCurrentTime.wMonth,
		sCurrentTime.wYear,
		sCurrentTime.wHour,
		sCurrentTime.wMinute);
	
	EmiPicFichierTrace( siInstId, "Purge ended [%s]", szBuffer );
	
}
// MFR end
/*-------------------------------- END OF FILE ------------------------------*/