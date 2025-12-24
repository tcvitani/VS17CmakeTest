/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   .																 */
/* FILE:     emi_pic_import.c												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
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
#define FORMAT_ELEMENT_SEPARATOR	";"
#define FORMAT_VALUE_SEPARATOR		":"

#define EMI_PIC_REG_KEYn_FORMATS	"ConfigFormats\\"
#define EMI_PIC_REG_KEYn_EXTENSIONS	"ConfigExtensions\\"

#define EMI_PIC_REG_KEYn_FILTER		"Filters\\"
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct
{
	LPSTR
		szFormatElement;
	INT
		iDefaultLength;
}struct_format_values;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE short int InitEmiPicImport(void *pvParam);
PRIVATE void EmiImportReceptionChrono(short siInstId,
                                      struct_neutre *pNeutre);

PROTECTED BOOL ReadDestinationConfig( short siInstId );
PRIVATE void EmiPicImportReceptionAni(short siInstId, struct_neutre *pNeutre);
PRIVATE void EmiPicGestionArretImport(short siInstId);
/*-------------------------------- VARIABLES: -------------------------------*/
struct_format_values FORMAT_ELEMENT[NB_FORMAT_ELEMENTS] = 
{// index of value MUST match enumeration
	{	"PLAZA",		 3	},	// ELEMENT_PLAZA
	{	"LANE",			 2	},	// ELEMENT_LANE
	{	"YEAR",			 4	},	// ELEMENT_YEAR
	{	"MONTH",		 2	},	// ELEMENT_MONTH
	{	"DAY",			 2	},	// ELEMENT_DAY
	{	"HOUR",			 2	},	// ELEMENT_HOUR
	{	"MINUTE",		 2	},	// ELEMENT_MINUTE
	{	"SECOND",		 2	},	// ELEMENT_SECOND
	{	"MILLISECOND",	 3	},	// ELEMENT_MILLISECOND
	{	"ID",			 6	},	// ELEMENT_RECORD_ID
	{	"TRANSACTION",	 6	},	// ELEMENT_TRANSACTION
	{	"GENERIC1",		10	},	// ELEMENT_GENERIC_1
	{	"GENERIC2",		10	},	// ELEMENT_GENERIC_2
	
};
/*-------------------------------- CODE:      -------------------------------*/
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
/*  0                     This function always returns zero.                 */
/*****************************************************************************/
PROTECTED DWORD WINAPI EmiPicImport(PVOID pvParam)
{
	struct_neutre *pNeutre;
	noyau_enum_retour eResult;
	short siInstId;
	
	// Thread initialisation (get instance id)
	siInstId = InitEmiPicImport(pvParam);
	
	ChangePriorite(TacheCourante(), EMI_PIC[siInstId].dwPrioMax);
	
	// Wait and process the messages
	while(TRUE)
	{
		// Wait for the message in the EMI thread mailbox
		// (infinite timeout)
		eResult = Recoit(	EMI_PIC[siInstId].dwImportBalId,
							(struct_neutre **)(&pNeutre),
							NOYAU_ATTENTE_INFINIE );
		
		// Protecting the message processing
		DebutRegion();
		
		if(eResult == NOYAU_BAL_MESS)
		{
			if(pNeutre->bl_retour == NOYAU_BAL_HORLOGE_ID)
			{// Message received from the timer
				EmiImportReceptionChrono(siInstId, pNeutre);
			}
			else if(pNeutre->bl_retour == EMI_PIC[siInstId].dwAniBalId)
			{
				// Message received from the ANI thread
				EmiPicImportReceptionAni( siInstId, pNeutre );
			}			
			else
			{// Undefined maibox
				EmiPicFichierDebug(siInstId,
					"EMI_PIC_EMI ***** EmiPicImport ==> Undefined mailbox %d *****",
					pNeutre->bl_retour);
			}
			
			// Free the message in the mailbox
			ExitLibere((struct_neutre **)(&pNeutre));
		}
		
		FinRegion();
	}
	
	return 0;
}
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
PRIVATE short int InitEmiPicImport(void *pvParam)
{
	short
		siInstId;
	noyau_bal_id
		BalEMI;
	
	// Get instance id
	siInstId = (short)pvParam;

	if( ReadDestinationConfig( siInstId ) != TRUE )
	{
		EmiPicFichierDebug( siInstId, "IMPORT : Error reading configuration" );
		ExitBad();
	}
    
	// publication of 'EMI_PIC_IMPORT' internal mailbox
	EMI_PIC[siInstId].dwImportBalId = PublieBAL(EMI_PIC[siInstId].szImportBalName, 0);

	if(EMI_PIC[siInstId].dwImportBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by EMI thread)
	BalEMI = AttendBAL(EMI_PIC[siInstId].szEmiBalName);
	if( BalEMI <= 0)
		ExitBad();

	LaunchTimer( siInstId, CHRONO_IMPORT, EMI_PIC[siInstId].dwImportBalId, DEFAULT_DELAY );
	
	return (siInstId);
}
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
PRIVATE void EmiImportReceptionChrono(short siInstId,
                                      struct_neutre *pNeutre)
{
	struct_chrono  *pMessage = (struct_chrono *)pNeutre;

	LPFILEFOUND
		lpFileList	= NULL;
	
	if(pMessage->numero == EMI_PIC[siInstId].sChronoData[CHRONO_IMPORT].chrono_id )
	{
		if( GetFilesFromDir( siInstId, EMI_PIC[siInstId].szImportDir, &lpFileList, "*.*" ) )
		{
			FinRegion();
			ProcessNewFiles( siInstId, &lpFileList, EMI_PIC[siInstId].szImportDir );
			DebutRegion();
		}

		if( LaunchTimer( siInstId, CHRONO_IMPORT, EMI_PIC[siInstId].dwImportBalId, DEFAULT_DELAY ) != TRUE )
		{
			_flushall();
			ExitBad();
		}
	}
	else
	{
		EmiPicFichierDebug(siInstId,
						   "EMI_PIC_EMI ***** EmiPicEmiReceptionChrono ==> undefined chrono %d *****",
						   pMessage->numero);
	}
}
/*****************************************************************************/
/*SYNTAX:void EmiPicImportReceptionAni(short siInstId, struct_neutre *pNeutre)*/
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
PRIVATE void EmiPicImportReceptionAni(short siInstId, struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;
	
	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_ARRET_DEMANDE:
			EmiPicGestionArretImport(siInstId);
			break;
			
		default:
			EmiPicFichierDebug(siInstId,
				"EMI_PIC_TRF ***** EmiPicTrfReceptionAni ==> type_message '%d' inconnu *****",
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
PROTECTED BOOL GetFilesFromDir( short siInstId, LPSTR szPath, LPFILEFOUND* lpFiles, CONST LPSTR szSearchMask )
{
	HANDLE
		hFindFile;
	WIN32_FIND_DATA
		sFileData;

	LPFILEFOUND
		lpNewFile		= NULL,
		lpFirstFile		= NULL,
		lpLastFile		= NULL;
	LONG
		lFileCount;
	CHAR
		szFilePathMask[MAX_PATH];

	_makepath_s( szFilePathMask,sizeof(szFilePathMask), NULL, szPath, szSearchMask, NULL );
	EmiPicFichierTrace( siInstId, "GetFilesFromDir()->Full search mask [%s]", szFilePathMask );

	START_TRY		
	hFindFile = FindFirstFile( szFilePathMask, &sFileData );

	if( hFindFile == INVALID_HANDLE_VALUE )
	{// No files found
		(*lpFiles) = NULL;
		return FALSE;
	}

	lFileCount = 0;
	
	do
	{
		// Remove all file flags except directory attribute
		sFileData.dwFileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
		
		if( sFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
		{// List only valid files
			// Prepare storage for file data
			lpNewFile = malloc( sizeof( struct_file_found ) );

			if( lpNewFile == NULL )
			{
				EmiPicFichierDebug( siInstId, "IMPORT : Failed to allocate memory" );
				_flushall();
				ExitBad();
			}

			memset( lpNewFile, 0, sizeof( struct_file_found ) );

			EmiPicFichierTrace( siInstId, "IMPORT : Found file [%s]", sFileData.cFileName );

			// If first file is not set, this is first file
			if( lpFirstFile == NULL )
				lpFirstFile = lpNewFile;
			else
				lpLastFile->lpNextFile = lpNewFile;

			strcpy_s( lpNewFile->szFileName,sizeof(lpNewFile->szFileName), sFileData.cFileName );

			// Remember this file node as last file node
			lpLastFile = lpNewFile;
			// Prepare for next loop
			lpNewFile = NULL;

			lFileCount++;
		}
	}
	while( FindNextFile( hFindFile, &sFileData ) != 0 );

	FindClose( hFindFile );

	if( lpFirstFile != NULL )
	{
		(*lpFiles) = lpFirstFile;

		EmiPicFichierTrace( siInstId, "IMPORT : GetFilesFromDir()->Found [%d] file(s)", lFileCount );
		return TRUE;
	}

	END_TRY

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
PROTECTED VOID CreateDestinationPath( IN LPSTR szDestination, LP_CONFIG_DATA lpDestinationConfig, LP_STRING_DATA lpStringData )
{
	CHAR
		szPath[MAX_PATH];

	// Create storage path for destination
	CreateStringByFormat( szPath, &(lpDestinationConfig->sFormatData), lpStringData );
	
	_makepath( szDestination, NULL, lpDestinationConfig->szDestinationDir, szPath, NULL );
	strcpy_s( szPath,sizeof(szPath), szDestination );
	_makepath( szDestination, NULL, szPath, NULL, NULL );
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
PROTECTED VOID ProcessNewFiles( short siInstId, LPFILEFOUND* lpFiles, CONST LPSTR szSourceDir )
{
	LPFILEFOUND
		lpCurrentFile	= (*lpFiles),
		lpToDelete		= NULL;

	struct_string_data
		sStringData;

	LPVOID
		lpMsgBuf;

	CHAR
		szStorePath[MAX_PATH]		= { 0 },
		szFileDestination[MAX_PATH]	= { 0 },
		szFileSource[MAX_PATH]		= { 0 };
	LONG
		iNbProcessedFiles;

	START_TRY

	iNbProcessedFiles = 0;

	while( lpCurrentFile != NULL )
	{
		if( GetDataFromString( lpCurrentFile->szFileName, &(EMI_PIC[siInstId].sFileNameFormat), &sStringData ) != FALSE )
		{
			CreateDestinationPath( szStorePath, &(EMI_PIC[siInstId].ConfigLocal), &sStringData );
			MakeSureDirectoryPathExists( szStorePath );
			// Create full path for file
			_makepath_s( szFileDestination,sizeof(szFileDestination), NULL, szStorePath, lpCurrentFile->szFileName, NULL );
			// Get current file path
//			_makepath( szFileSource, NULL, EMI_PIC[siInstId].szImportDir, lpCurrentFile->szFileName, NULL );
			_makepath_s( szFileSource,sizeof(szFileSource), NULL, szSourceDir, lpCurrentFile->szFileName, NULL );

//			EmiPicFichierTrace( siInstId, "TRANSFER : Source [%s] Destination [%s]", szFileSource, szFileDestination );

			if( EMI_PIC[siInstId].bUsingCounter != FALSE )
			{// Add counter to file name
				AddCounterData( szFileDestination );
			}

			START_TRY
			// Move file to backup directory
			if( MoveFileEx( szFileSource,
							szFileDestination,
							MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH ) != TRUE )
			{
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
				
				EmiPicFichierDebug(	siInstId,
									"IMPORT : Error while moving file [%s], description = [%s]",
									szFileSource,
									lpMsgBuf );
				
				// Free the buffer.
				LocalFree( lpMsgBuf );
			}
			END_TRY
		}
		else
		{
			EmiPicFichierTrace( siInstId,
				"TRANSFER : File [%s] does not fit format",
				lpCurrentFile->szFileName );
		}
		
		// Set current node to be deleted
		lpToDelete = lpCurrentFile;
		// Move to next node
		lpCurrentFile = lpCurrentFile->lpNextFile;
		// Delete old node
		free( lpToDelete );
		lpToDelete = NULL;

		iNbProcessedFiles++;
	}

	END_TRY

	EmiPicFichierTrace( siInstId,
		"IMPORT : Imported [%d] file(s)",
		iNbProcessedFiles );
	
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
PROTECTED BOOL GetFormatConfiguration( LPSTR szFormat, LP_FORMAT_DATA lpFormatData )
{
	INT
		i,
		iOrderElement	= 0;
	CHAR
		szFormatBuffer[MAX_PATH],
		szOutFormat[MAX_PATH]		= { 0 },
		szBuffer[MAX_PATH];
	LPSTR
		lpSearch	= NULL,
		lpStart,
		lpEnd,
		lpEndOfSearch,
		lpFoundKeyword,
		lpFoundValue;
	BOOL
		bUseDefaultValue;

	if( szFormat == NULL || lpFormatData == NULL )
		return FALSE;

	strcpy_s( szFormatBuffer,sizeof(szFormatBuffer), szFormat );
	memset( lpFormatData, 0, sizeof( struct_format_data ) );	

	lpSearch = strstr( szFormatBuffer, FORMAT_ELEMENT_SEPARATOR );

	if( lpSearch == NULL )
	{// No format keywords found, nothing to replace in format
		strcpy_s( lpFormatData->szFormatString,sizeof(lpFormatData->szFormatString), szFormat );
		return TRUE;
	}

	lpStart = szFormatBuffer;

	while( lpSearch != NULL )
	{
		bUseDefaultValue = FALSE;

		(*lpSearch) = '\0';

		// Add bits that are not part of keywords to format output
		strcat_s( szOutFormat,sizeof(szOutFormat), lpStart );

		lpStart = lpSearch;
		// Move start pointer into keyword
		lpStart++;
		// Mark keyword
		lpFoundKeyword = lpStart;

		// Find end of keyword definition
		lpEnd = strstr( lpStart, FORMAT_ELEMENT_SEPARATOR );

		if( lpEnd == NULL )
		{// End of keyword definition not found, error in format
			return FALSE;
		}

		// Save end position
		lpEndOfSearch = lpEnd;
		// Move it into next bit (or end of format)
		lpEndOfSearch++;

		// Break keyword definition
		(*lpEnd) = '\0';

		// Search for value definition
		lpSearch = strstr( lpStart, FORMAT_VALUE_SEPARATOR );

		if( lpSearch != NULL )
		{
			// Terminate keyword
			(*lpSearch) = '\0';
			// Move pointer into value
			lpSearch++;
			// Mark value
			lpFoundValue = lpSearch;
			
		}
		else
		{// Keyword doesn't define value
			lpFoundValue = NULL;
		}

		// Identify found keyword
		for( i = 0; i < NB_FORMAT_ELEMENTS; i++)
		{
			if( strcmp( FORMAT_ELEMENT[i].szFormatElement, lpFoundKeyword ) == 0 )
			{
				lpFormatData->Element[i].bIsUsed = TRUE;				
				lpFormatData->Order[iOrderElement++] = i;
				
				if( lpFoundValue != NULL )
				{
					lpFormatData->Element[i].iFormatLen = atoi( lpFoundValue );
				}
				else
				{// Format length is not defined, use default
					lpFormatData->Element[i].iFormatLen = FORMAT_ELEMENT[i].iDefaultLength;
				}

				sprintf_s( szBuffer,sizeof(szBuffer), "%d", lpFormatData->Element[i].iFormatLen );

				strcat_s( szOutFormat,sizeof(szOutFormat), "%0");
				strcat_s( szOutFormat,sizeof(szOutFormat), szBuffer );
				strcat_s( szOutFormat,sizeof(szOutFormat), "d" );


				break;
			}
		}

		lpStart = lpEndOfSearch;
		// Search for next keyword
		lpSearch = strstr( lpEndOfSearch, FORMAT_ELEMENT_SEPARATOR );
	}

	strcpy_s( lpFormatData->szFormatString,sizeof(lpFormatData->szFormatString), szOutFormat );


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
PROTECTED BOOL GetDataFromString( LPSTR szDataString, LP_FORMAT_DATA lpFormatData, LP_STRING_DATA lpStringData )
{
	INT
		iNbDataElements	= 0,
		iNbElementsRead,
		iIndex,
		DataBuffer[16]	= { 0 };

	memset( lpStringData->lValue, 0, sizeof(LONG)*NB_FORMAT_ELEMENTS );
	
	// Get number of data elements usef for this format
	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
	{
		if( lpFormatData->Element[iIndex].bIsUsed )
			iNbDataElements++;
	}

	iIndex = 0;
	// 
	iNbElementsRead = sscanf_s(	szDataString,
								lpFormatData->szFormatString,
								&DataBuffer[0],
								&DataBuffer[1],
								&DataBuffer[2],
								&DataBuffer[3],
								&DataBuffer[4],
								&DataBuffer[5],
								&DataBuffer[6],
								&DataBuffer[7],
								&DataBuffer[8],
								&DataBuffer[9],
								&DataBuffer[10],
								&DataBuffer[11],
								&DataBuffer[12],
								&DataBuffer[13],
								&DataBuffer[14],
								&DataBuffer[15]
								);
	
	if( iNbElementsRead != iNbDataElements )
	{// Number of elements read from string is not same as number of elements specified by format
		return FALSE;
	}
	
	// Arrange read data according to format
	// MFR start 2009/11/23
	// BUG in version 2.0.1
	// Going trough all declared elements instead of only read ones
	// In case that some available elements were skiped (eg. used PLAZA, LANE, 
	// YEAR, MONTH, DAY and then skiped to TRANSACTION), it would incorrectly
	// use Order parameter, skiped over read values and again assign values to
	// first read parameter. Bug manifested in lose of PLAZA data from file 
	// name (reseting plaza value to 0)
//	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
//	{
//		if( lpFormatData->Element[iIndex].bIsUsed != TRUE )
//			continue;
//		
//		lpStringData->lValue[lpFormatData->Order[iIndex]] = DataBuffer[iIndex];
//	}

	for( iIndex = 0; iIndex < iNbElementsRead; iIndex++ )
	{
		// Order defines which element is at position iIndex in data string
		if( lpFormatData->Element[lpFormatData->Order[iIndex]].bIsUsed != TRUE )
			continue;
		
		lpStringData->lValue[lpFormatData->Order[iIndex]] = DataBuffer[iIndex];
	}	
	// MFR end
	
	return TRUE;
}/*****************************************************************************/
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
PROTECTED VOID GetPartialDataFromString( LPSTR szDataString, LP_FORMAT_DATA lpFormatData, LP_STRING_DATA lpStringData )
{
	INT
		iNbDataElements	= 0,
		iNbElementsRead,
		iIndex,
		DataBuffer[16]	= { 0 };

	memset( lpStringData, 0, sizeof(struct_string_data) );
	
	// Get number of data elements usef for this format
	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
	{
		if( lpFormatData->Element[iIndex].bIsUsed )
			iNbDataElements++;
	}
	
	iIndex = 0;
	// 
	iNbElementsRead = sscanf_s(	szDataString,
								lpFormatData->szFormatString,
								&DataBuffer[0],
								&DataBuffer[1],
								&DataBuffer[2],
								&DataBuffer[3],
								&DataBuffer[4],
								&DataBuffer[5],
								&DataBuffer[6],
								&DataBuffer[7],
								&DataBuffer[8],
								&DataBuffer[9],
								&DataBuffer[10],
								&DataBuffer[11],
								&DataBuffer[12],
								&DataBuffer[13],
								&DataBuffer[14],
								&DataBuffer[15]
								);

	// Arrange read data according to format
	// MFR start 2009/11/23	
	// BUG in version 2.0.1
//	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
//	{
//		if( lpFormatData->Element[iIndex].bIsUsed != TRUE )
//			continue;
//
//		lpStringData->lValue[lpFormatData->Order[iIndex]] = DataBuffer[iIndex];
//	}
	// MFR end

	for( iIndex = 0; iIndex < iNbElementsRead; iIndex++ )
	{
		// Order defines which element is at position iIndex in data string
		if( lpFormatData->Element[lpFormatData->Order[iIndex]].bIsUsed != TRUE )
			continue;
		
		lpStringData->lValue[lpFormatData->Order[iIndex]] = DataBuffer[iIndex];
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
PROTECTED BOOL CreateStringByFormat( OUT LPSTR szDataString, IN LP_FORMAT_DATA lpFormatData, IN LP_STRING_DATA lpStringData )
{
	INT
		iNbDataElements	= 0,
		iIndex,
		DataBuffer[16]	= { 0 };

	// Arrange write data according to format
	for( iIndex = 0; iIndex < NB_FORMAT_ELEMENTS; iIndex++ )
	{			
		DataBuffer[iIndex] = lpStringData->lValue[lpFormatData->Order[iIndex]];
	}

	sprintf(	szDataString,
				lpFormatData->szFormatString,
				DataBuffer[0],
				DataBuffer[1],
				DataBuffer[2],
				DataBuffer[3],
				DataBuffer[4],
				DataBuffer[5],
				DataBuffer[6],
				DataBuffer[7],
				DataBuffer[8],
				DataBuffer[9],
				DataBuffer[10],
				DataBuffer[11],
				DataBuffer[12],
				DataBuffer[13],
				DataBuffer[14],
				DataBuffer[15]
				);
	
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
PRIVATE VOID FreeFilterList( LP_FILTER* lpFilterList )
{
	LP_FILTER
		lpCurrentFilter = (*lpFilterList),
		lpToDelete;

	while( lpCurrentFilter != NULL )
	{
		if( lpCurrentFilter->lpstrFilter != NULL )
		{
			free( lpCurrentFilter->lpstrFilter );
			lpCurrentFilter->lpstrFilter = NULL;
		}

		lpToDelete = lpCurrentFilter;
		lpCurrentFilter = lpCurrentFilter->lpNextFilter;

		free( lpToDelete );
		lpToDelete = NULL;
	}

	(*lpFilterList) = NULL;
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
PRIVATE BOOL GetFilterData( short siInstId, LPSTR lpstrRegPath, LP_FILTER* lpFilterList )
{
	CHAR
		szFilterKey[MAX_PATH],
		szFilterName[MAX_PATH],
		szDataBuffer[MAX_PATH];
	DWORD
		dwRet,
		dwIndex			= 0,
		dwFilterNameLen	= MAX_PATH,
		dwBufferLen		= MAX_PATH;

	LP_FILTER
		lpFirstFilter	= NULL,	
		lpCurrentFilter	= NULL,
		lpLastFilter	= NULL;

	_makepath_s(	szFilterKey,
				sizeof(szFilterKey),
				NULL,
				lpstrRegPath,
				EMI_PIC_REG_KEYn_FILTER,
				NULL );

	// Get first key
	dwRet = REG_Enum_Valeurs_Chaine(	CSR_REG_KEYi_ROOT,
										szFilterKey,
										dwIndex,
										szFilterName,
										&dwFilterNameLen,
										szDataBuffer,
										&dwBufferLen );

	while( dwRet == ERROR_SUCCESS )
	{

		// Allocate memory for filter
		lpCurrentFilter = malloc( sizeof( struct_filter ) );

		if( lpCurrentFilter == NULL )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : GetFilterData()->Failed to allocate memory" );
			_flushall();
			ExitBad();
		}

		memset( lpCurrentFilter, 0, sizeof( struct_filter ) );

		// Increase size to accommodate terminator
		dwBufferLen++;
		lpCurrentFilter->lpstrFilter = malloc( sizeof( CHAR ) * dwBufferLen );

		if( lpCurrentFilter->lpstrFilter == NULL )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : GetFilterData()->Failed to allocate memory" );
			_flushall();
			ExitBad();
		}		

		memset( lpCurrentFilter->lpstrFilter, 0, sizeof( CHAR ) *dwBufferLen );

		strcpy_s( lpCurrentFilter->lpstrFilter,sizeof( CHAR ) *dwBufferLen, szDataBuffer );

		if( lpFirstFilter == NULL )
			lpFirstFilter =lpCurrentFilter;

		if( lpLastFilter != NULL )
			lpLastFilter->lpNextFilter = lpCurrentFilter;

		lpLastFilter = lpCurrentFilter;

		// Reset data for next cycle
		dwFilterNameLen = MAX_PATH;
		dwBufferLen		= MAX_PATH;
		// Move index to next registry value
		dwIndex++;

		// Get next registry value
		dwRet = REG_Enum_Valeurs_Chaine(	CSR_REG_KEYi_ROOT,
											szFilterKey,
											dwIndex,
											szFilterName,
											&dwFilterNameLen,
											szDataBuffer,
											&dwBufferLen );
	}

	if( dwRet != ERROR_NO_MORE_ITEMS )
	{// Error while reading registry values, free data
		FreeFilterList( &lpFirstFilter );
		return FALSE;
	}

	(*lpFilterList) = lpFirstFilter;
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
PROTECTED BOOL ReadDestinationConfig( short siInstId )
{
	CHAR
		szQueryKey[MAX_PATH],
		szKeyName[MAX_PATH],
		szConfigKey[MAX_PATH],
		szBuffer[MAX_PATH],
		szTemp[MAX_PATH];

	DWORD
		dwRet,
		dwIndex			= 0,
		dwKeyNameLen	= MAX_PATH,
		dwBufferLen;

	// Create configuration key path
	_makepath_s( szQueryKey,sizeof(szQueryKey), NULL, EMI_PIC[siInstId].szKey, EMI_PIC_REG_KEYn_DESTINATION_CONFIG, NULL );

	// Get first key
	dwRet = REG_Enum_Cles( CSR_REG_KEYi_ROOT, szQueryKey, dwIndex, szKeyName, &dwKeyNameLen );
	
	if (dwRet != ERROR_SUCCESS)
		EmiPicFichierDebug(siInstId, "IMPORT : ReadDestinationConfig()->Error REG_Enum_Cles destinations defined in [%s] ", szQueryKey);

	while( dwRet == ERROR_SUCCESS )
	{
		if( dwIndex >= MAX_DESTINATIONS )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->There is more destinations defined [%d] than allowed [%d]", dwIndex, MAX_DESTINATIONS );
			return FALSE;
		}

		// Read key configuration
		// Create current config key path
		_makepath_s( szConfigKey,sizeof(szConfigKey), NULL, szQueryKey, szKeyName, NULL );
		// Query values from key
		EmiPicFichierDebug(siInstId, "IMPORT : ReadDestinationConfig()->Destination defined in [%s] ", szConfigKey);

		// Read destination path
		dwBufferLen = MAX_PATH;
		if( REG_Lire_Chaine(	CSR_REG_KEYi_ROOT,
								szConfigKey,
								EMI_PIC_REG_KEYv_DESTINATION_DIR,
								szBuffer,
								&dwBufferLen ) != ERROR_SUCCESS )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error reading registry value [%s %s]", szConfigKey, EMI_PIC_REG_KEYv_DESTINATION_DIR );			
			return FALSE;
		}

		strcpy_s( EMI_PIC[siInstId].ConfigDest[dwIndex].szDestinationDir,sizeof(EMI_PIC[siInstId].ConfigDest[dwIndex].szDestinationDir), szBuffer );

		// Read temporary extension
		dwBufferLen = MAX_PATH;
		if( REG_Lire_Chaine(	CSR_REG_KEYi_ROOT,
			szConfigKey,
			EMI_PIC_REG_KEYv_TEMP_EXTENSION,
			szBuffer,
			&dwBufferLen ) != ERROR_SUCCESS )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error reading registry value [%s %s]", szConfigKey, EMI_PIC_REG_KEYv_TEMP_EXTENSION );			
			return FALSE;
		}
		
		strcpy_s( EMI_PIC[siInstId].ConfigDest[dwIndex].szTempExtension,sizeof(EMI_PIC[siInstId].ConfigDest[dwIndex].szTempExtension), szBuffer );
		
		// Read destination path format
		dwBufferLen = MAX_PATH;
		if( REG_Lire_Chaine(	CSR_REG_KEYi_ROOT,
							szConfigKey,
							EMI_PIC_REG_KEYv_PATH_FORMAT,
							szBuffer,
							&dwBufferLen ) != ERROR_SUCCESS )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error reading registry value [%s %s]", szConfigKey, EMI_PIC_REG_KEYv_PATH_FORMAT );			
			return FALSE;
		}
		
		if( GetFormatConfiguration( szBuffer, &EMI_PIC[siInstId].ConfigDest[dwIndex].sFormatData ) != TRUE )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error reading destination format data, destination [%s]", szConfigKey );			
			return FALSE;
		}
		else
			EmiPicFichierTrace(siInstId,
						"emi_pic_import::GetFormatConfiguration: szFormatString[%s]",
						escapeCStringForSPrintf(EMI_PIC[siInstId].ConfigDest[dwIndex].sFormatData.szFormatString, szTemp, sizeof(szTemp)));


		if( GetFilterData( siInstId, szConfigKey, &(EMI_PIC[siInstId].ConfigDest[dwIndex].lpFilterList) ) != TRUE )
		{
			EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error reading filter format data, destination [%s]", szConfigKey );			
			return FALSE;
		}
		
		// Save number of read destinations
		EMI_PIC[siInstId].iNbDefinedDest = dwIndex;
									
		// Reset key name length
		dwKeyNameLen = MAX_PATH;
		// Increase index
		dwIndex++;
		// Get next key
		dwRet = REG_Enum_Cles( CSR_REG_KEYi_ROOT, szQueryKey, dwIndex, szKeyName, &dwKeyNameLen );
	}

	if( dwRet != ERROR_NO_MORE_ITEMS )
	{
		EmiPicFichierDebug( siInstId, "IMPORT : ReadDestinationConfig()->Error while enumerating registry key [%s]", szConfigKey );			
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
PROTECTED BOOL CompareStringToFilterList( IN LPSTR szTestString, IN LP_FILTER lpFilterList )
{
	LP_FILTER
		lpCurrentFilter = lpFilterList;

	while( lpCurrentFilter != NULL )
	{
		if( strcmp( lpCurrentFilter->lpstrFilter, "*" ) == 0 )
			return TRUE;
		
		if( strstr( szTestString, lpCurrentFilter->lpstrFilter ) != NULL )
			return TRUE;

		lpCurrentFilter = lpCurrentFilter->lpNextFilter;
	}

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
PROTECTED VOID FillStringDataWithDateTime( OUT LP_STRING_DATA lpStringData, IN LPSYSTEMTIME lpDateTime )
{
	lpStringData->lValue[ELEMENT_YEAR] = lpDateTime->wYear;
	lpStringData->lValue[ELEMENT_MONTH] = lpDateTime->wMonth;
	lpStringData->lValue[ELEMENT_DAY] = lpDateTime->wDay;
	lpStringData->lValue[ELEMENT_HOUR] = lpDateTime->wHour;
	lpStringData->lValue[ELEMENT_MINUTE] = lpDateTime->wMinute;
	lpStringData->lValue[ELEMENT_SECOND] = lpDateTime->wSecond;
	lpStringData->lValue[ELEMENT_MILLISECOND] = lpDateTime->wMilliseconds;
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
PROTECTED VOID FillDateTimeWithStringData( OUT LPSYSTEMTIME lpDateTime, IN LP_STRING_DATA lpStringData )
{
	int
		i;
	WORD
		wValue;

	memset( lpDateTime, 0, sizeof(SYSTEMTIME) );

	for( i = 0; i < NB_FORMAT_ELEMENTS; i++ )
	{
		if( lpStringData->lValue[i] != 0 )
			wValue = (WORD)lpStringData->lValue[i];
		else
		{
			if( i >= ELEMENT_HOUR &&  i <=ELEMENT_MILLISECOND )
				wValue = (WORD)lpStringData->lValue[i];
			else
				wValue = (WORD)1;
		}

		switch( i )
		{
			case ELEMENT_YEAR:
				lpDateTime->wYear = wValue;
				break;
			case ELEMENT_MONTH:
				lpDateTime->wMonth = wValue;
				break;
			case ELEMENT_DAY:
				lpDateTime->wDay = wValue;
				break;
			case ELEMENT_HOUR:
				lpDateTime->wHour = wValue;
				break;
			case ELEMENT_MINUTE:
				lpDateTime->wMinute = wValue;
				break;
			case ELEMENT_SECOND:
				lpDateTime->wSecond = wValue;
				break;
			case ELEMENT_MILLISECOND:
				lpDateTime->wMilliseconds = wValue;
				break;		
			default:
				break;
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
PROTECTED LONG GetNumberOfSubdirs( IN LPSTR szPath )
{
	HANDLE
		hFindFile;
	WIN32_FIND_DATA
		sFileData;
	CHAR
		szDir[MAX_PATH],
		szSubDir[MAX_PATH];
	LONG
		NbSubdirs = 0;
	
	if( szPath == NULL || _access( szPath, FILE_EXISTANCE ) == FILE_NOT_FOUND )
	{
		return 0;
	}
	
	_makepath_s( szDir,sizeof(szDir), NULL, szPath, "*", NULL );
	
	hFindFile = FindFirstFile( szDir, &sFileData );
	
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
					_makepath_s( szSubDir,sizeof(szSubDir), NULL, szPath, sFileData.cFileName, NULL );
					NbSubdirs += ( GetNumberOfSubdirs( szSubDir ) + 1 );
				}
			}
		}while(FindNextFile( hFindFile, &sFileData ) != 0 );
	}
	
	if( hFindFile != INVALID_HANDLE_VALUE )
	{
		FindClose( hFindFile );
	}
	
	return NbSubdirs;
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
PUBLIC VOID AddCounterData( IN OUT LPSTR szFilePath )
{
	CHAR
		szDrive[MAX_PATH],
		szDir[MAX_PATH],
		szFileName[MAX_PATH],
		szExtension[MAX_PATH],
		szPathBuffer[MAX_PATH];
	LPSTR
		lpDrive		= NULL,
		lpDir		= NULL,
		lpFileName	= NULL,
		lpExtension	= NULL,
		lpCounter	= NULL;
	
	// Sanity check
	if( szFilePath == NULL || strlen( szFilePath ) == 0 )
	{// Something wrong with input path
		return;
	}
	
	// Path should be like c:\xxx\xxx.000.jpg
	_splitpath( szFilePath, szDrive, szDir, szFileName, szExtension );
	
	if( strlen( szDrive ) != 0 )
		lpDrive = szDrive;
	
	if( strlen( szDir ) != 0 )
		lpDir = szDir;
	
	if( strlen( szFileName ) != 0 )
		lpFileName = szFileName;
	
	if( strlen( szExtension ) != 0 )
		lpExtension = szExtension;	
	
	// Now recreate file path
	_makepath( szFilePath, lpDrive, lpDir, lpFileName, "000" );
	
	if( lpExtension != NULL )
	{// There is also extension to add (current extension is counter)
		strcpy_s( szPathBuffer,sizeof(szPathBuffer), szFilePath );
		_makepath( szFilePath, NULL, NULL, szPathBuffer, lpExtension );
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
PUBLIC VOID RemoveCounterData( IN OUT LPSTR szFilePath )
{
	CHAR
		szDrive[MAX_PATH],
		szDir[MAX_PATH],
		szFileName[MAX_PATH],
		szExtension[MAX_PATH],
		szFileNameBuffer[MAX_PATH];
	LPSTR
		lpDrive		= NULL,
		lpDir		= NULL,
		lpFileName	= NULL,
		lpExtension	= NULL;

	// Sanity check
	if( szFilePath == NULL || strlen( szFilePath ) == 0 )
	{// Something wrong with input
		return;
	}

	// Path should be like c:\xxx\xxx.000.jpg
	_splitpath( szFilePath, szDrive, szDir, szFileName, szExtension );

	if( strlen( szDrive ) != 0 )
		lpDrive = szDrive;
	
	if( strlen( szDir ) != 0 )
		lpDir = szDir;
	
	if( strlen( szExtension ) != 0 )
		lpExtension = szExtension;

	if( strchr( szFileName, '.' ) != NULL )
	{// File has both extension and counter
		// Removing counter part of name (now extension)
		_splitpath( szFileName, NULL, NULL, szFileNameBuffer, NULL );
		lpFileName = szFileNameBuffer;
	}
	else
	{// File has only extension and no counter
		lpFileName = szFileName;
	}

	// Now recreate file path
	_makepath( szFilePath, lpDrive, lpDir, lpFileName, lpExtension );
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
PUBLIC VOID IncreaseCounterData( IN OUT LPSTR szFilePath )
{
	CHAR
		szDrive[MAX_PATH],
		szDir[MAX_PATH],
		szFileName[MAX_PATH],
		szExtension[MAX_PATH],
		szFileNameBuffer[MAX_PATH],
		szExtensionBuffer[MAX_PATH],
		szPathBuffer[MAX_PATH],
		szCounterData[MAX_PATH];
	LPSTR
		lpDrive		= NULL,
		lpDir		= NULL,
		lpFileName	= NULL,
		lpExtension	= NULL,
		lpCounter	= NULL;
	LONG
		lCounter;
	
	// Sanity check
	if( szFilePath == NULL || strlen( szFilePath ) == 0 )
	{// Something wrong with input path
		return;
	}

	// Path should be like c:\xxx\xxx.000.jpg
	_splitpath( szFilePath, szDrive, szDir, szFileName, szExtension );
	
	if( strlen( szDrive ) != 0 )
		lpDrive = szDrive;
	
	if( strlen( szDir ) != 0 )
		lpDir = szDir;
	
	if( strlen( szExtension ) != 0 )
		lpExtension = szExtension;
	
	if( strchr( szFileName, '.' ) != NULL )
	{// File has both extension and counter
		_splitpath( szFileName, NULL, NULL, szFileNameBuffer, szExtensionBuffer );
		lpFileName = szFileNameBuffer;
		lpCounter = szExtensionBuffer;
	}
	else
	{// File has only counter and no real extension
		strcpy_s( szExtensionBuffer,sizeof(szExtensionBuffer), "" );
		lpFileName = szFileName;
		lpCounter = szExtension;
		lpExtension = NULL;
	}

	if( lpCounter == NULL || strlen( lpCounter ) == 0 )
	{// Something wrong here
		return;
	}

	if( *lpCounter == '.' )
	{// _splitpath leaves '.' in extension
		lpCounter++;
	}

	lCounter = atol( lpCounter );
	lCounter++;

	sprintf_s( szCounterData,sizeof(szCounterData), "%03d", lCounter );	
	
	// Now recreate file path with counter as extension
	_makepath( szFilePath, lpDrive, lpDir, lpFileName, szCounterData );

	if( lpExtension != NULL )
	{// There is also extension to add (current extension is counter)
		strcpy_s( szPathBuffer,sizeof(szPathBuffer), szFilePath );
		_makepath( szFilePath, NULL, NULL, szPathBuffer, lpExtension );
	}
}
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
PRIVATE void EmiPicImportEnvoiAni( short siInstId,
								   enum_emi_pic_internal_type eTypeMessage,
								   void *pvParam )
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
		case M_ARRET_EFFECTUE:
			break;
			
		default:
			break;
	}
	
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_TRF : EmiPicImportEnvoiAni ==> Type_message '%d'",
					   pMessage->entete.type_message);
	
	// Send the message
	ExitEnvoie( EMI_PIC[siInstId].dwAniBalId,
				EMI_PIC[siInstId].dwImportBalId,
				(struct_neutre *)pMessage );
}
/*****************************************************************************/
/*SYNTAX: void EmiPicGestionArretImport(short siInstId)                      */
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
PRIVATE void EmiPicGestionArretImport(short siInstId)
{
	LONG
		i;

	for( i = 0; i < EMI_PIC[siInstId].iNbDefinedDest; i++ )
	{
		FreeFilterList( &(EMI_PIC[siInstId].ConfigDest[i].lpFilterList) );
		EMI_PIC[siInstId].ConfigDest[i].lpFilterList = NULL;
	}	
	

	EmiPicImportEnvoiAni( siInstId, M_ARRET_EFFECTUE, NULL);
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_TRF : EmiPicGestionArretTrf ==> Arret de la tache TRF");
	
	EmiPicLibereBAL(EMI_PIC[siInstId].dwTrfBalId);
	Termine();
}
/*-------------------------------- END OF FILE ------------------------------*/