/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_convert.c                                         */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that convert GEA file to     */
/*             CSSI file. The functions separate converted messages in       */
/*             different files depending on the lane number found in the     */
/*             message header.                                               */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>
// CSSI interface
#include <trc.h>
#include <csr_msg.h>
#include <msg_gea_lc_header.h>
#include <msg_gea_lc_header_ii.h>
#include <msg_gea_lc_endshift.h>
#include <msg_gea_lc_endshift_ii.h>
#include <msg_gea_lc_event.h>
#include <msg_gea_lc_startshift.h>
#include <msg_gea_lc_tab_ack.h>
#include <msg_gea_lc_transaction.h>
#include <msg_gea_lc_transaction_iii.h>
#include <msg_gea_lc_transaction_ii.h>

// Module interface
#include <cmworkgeafile.h>
#include <cmworkgeafile_file.h>
#include <cmworkgeafile_eos.h>
#include <cmworkgeafile_sos.h>
#include <cmworkgeafile_event.h>
#include <cmworkgeafile_trs.h>
#define LOC_DEF
#include <cmworkgeafile_convert.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define MAX_LANES                  40
#define MAX_TRS_LINE_LENGTH        400
#define MAX_TRS_ITEMS              100
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_LANE_CONTEXT
{
	DWORD dwLaneCount;

	struct CMW_LANES
	{
		HANDLE * hFile;
		HANDLE * hBakFile;
		struct CMW_SAVED_CONTEXT
		{
			char szLaneNumber[10];
			unsigned long dwFileCounter;
			unsigned long dwBakFileCounter;
			DWORD dwTrsCount;
			DWORD dwTrsPFareSum;
			DWORD dwLastTrsIndex;
			struct MSG_GEA_TRANSACTION_III szTrsItem[MAX_TRS_ITEMS];
		}sSaved;
	}sLanes[MAX_LANES];

}CMW_LANE_CONTEXT;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE BOOL IsLaneInList( IN CMW_PARAMS * pcParams, IN DWORD dwLaneNumber );
PRIVATE DWORD SaveMessage( IN CMW_PARAMS * pcParams,
						   IN char * szLaneNumber,
						   IN DWORD dwPlazaNumber,
						   IN char * pcMessage, 
						   IN DWORD dwMsgLength,
						   IN BOOL bBackup);
PRIVATE void CloseAllFiles( void );

PRIVATE DWORD ProcessTransaction( IN CMW_PARAMS * pcParams,
								  IN char	*pcMessage,
								  IN BOOL	bBackup,
								  IN int	iVersion );
PRIVATE DWORD ProcessMessage ( IN CMW_PARAMS * pcParams,
							   IN struct MSG_GEA_HEADER * pHeader,
							   IN char * pcMessage );
PRIVATE DWORD ProcessMessageII ( IN CMW_PARAMS * pcParams, 
							     IN struct MSG_GEA_HEADER_II * pHeader,
							     IN char * pcMessage );

PRIVATE int FindLaneByNumber( char * szLaneNumber );
PRIVATE int AddNewLane( char * szLaneNumber );

PRIVATE DWORD AddTrsItem ( int iLaneIndex, struct MSG_GEA_TRANSACTION_III * pTransaction );
PRIVATE void DelTrsItems ( int iLaneIndex );
/*-------------------------------- VARIABLES: -------------------------------*/
PRIVATE CMW_LANE_CONTEXT sContext = { 0, NULL, NULL, "", 1, 1, 0, 0, 10, 0 };
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertFile( IN CMW_PARAMS * pcParams, IN HANDLE * hFile )   */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function searches for the GEA messages that have to be     */
/*            converted. If it finds the transaction item it stores the item */
/*            in the memory array. When the function finds the last          */
/*            transaction item it calls the transaction conversion function. */
/*            All other messages are converted as soon as they are detected  */
/*            (because they contain only one item).                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN HANDLE * hFile        - File handle                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PROTECTED DWORD ConvertFile( IN CMW_PARAMS * pcParams, IN HANDLE * hFile )
{
	DWORD dwError, dwMsgSize;
	char * pcMessage = NULL;
	BOOL bBackup;
	struct MSG_GEA_HEADER * pHeader;
	struct MSG_GEA_HEADER_II * pHeaderII;

	if( SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) != 0 )
		return GetLastError();

	// Read the first message
	dwError = GEAReadNextMessage( pcParams, hFile, &pcMessage );

	while ( dwError == NO_ERROR )
	{
		// Check the lane number

		WorkTrace( pcParams, "\r\n-> Allocate GEA message header" );
		pHeader = MSG_GEA_HEADER_New();
 		if( pHeader != NULL )
		{
			if( MSG_GEA_HEADER_Read( pHeader, (BYTE *)pcMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize) )
			{
				// If the the lane has to be processed
				if ( IsLaneInList( pcParams, atol( pHeader->lane_number ) ) == FALSE )
				{
					WorkTrace( pcParams, "\r\n> Lane %d not in the parameter list of lanes (ignore message)", atol( pHeader->lane_number ) );
					WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
					MSG_GEA_HEADER_Delete_All( pHeader );
					WorkTrace( pcParams, "\r\n<== Free line" ); 
					FreeMessage( &pcMessage );

					// Get the next message
					dwError = GEAReadNextMessage( pcParams, hFile, &pcMessage );
					continue;
				}
				// If the header is extended header
				if ( MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_EXTENDED_CD, &bBackup ) )
				{
					WorkTrace( pcParams, "\r\n> Allocate GEA message header II" );
					pHeaderII = MSG_GEA_HEADER_II_New();
					if ( pHeaderII != NULL )
					{
						if ( MSG_GEA_HEADER_II_Read(pHeaderII, (BYTE *)pcMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize ) )
						{
							if ( MSG_GEA_HEADER_II_Check_CD( pHeaderII, MSG_GEA_TRANSACTION_II_CD, &bBackup ) )
							{
								// Is it the last transaction item?
								ProcessTransaction( pcParams, pcMessage, bBackup, atoi( pHeaderII->message_version ) );

								WorkTrace( pcParams, "\r\n< Deallocate GEA message header II" );
								MSG_GEA_HEADER_II_Delete_All( pHeaderII );
							}
							else if ( MSG_GEA_HEADER_II_Check_CD( pHeaderII, MSG_GEA_END_SHIFT_II_CD, &bBackup ) )
							{
								// If it is the end of shift mesage type II
								ProcessMessageII( pcParams, pHeaderII, pcMessage );

								MSG_GEA_HEADER_II_Delete_All( pHeaderII );
							}
							else
							{
								// If it is not the transaction message we don't use it
								WorkTrace( pcParams, "\r\n< Deallocate GEA message header II" );
								MSG_GEA_HEADER_II_Delete_All( pHeaderII );

								WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
								MSG_GEA_HEADER_Delete_All( pHeader );
								WorkTrace( pcParams, "\r\n<== Free line" ); 
								FreeMessage( &pcMessage );
								// Get the next message
								dwError = GEAReadNextMessage( pcParams, hFile, &pcMessage );
								continue;
							}
						}
						else
						{
							// Error reading extended header
							WorkTrace( pcParams, "\r\n< Deallocate GEA message header II" );
							MSG_GEA_HEADER_II_Delete_All( pHeaderII );

							WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
							MSG_GEA_HEADER_Delete_All( pHeader );
							WorkTrace( pcParams, "\r\n<== Free line" ); 
							FreeMessage( &pcMessage );
							CloseAllFiles();

							return GetLastError();
						}
					}
					else
					{
						// Error allocating extended header
						WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
						MSG_GEA_HEADER_Delete_All( pHeader );
						WorkTrace( pcParams, "\r\n<== Free line" ); 
						FreeMessage( &pcMessage );
						CloseAllFiles();

						return GetLastError();
					}
				}
				// Not extended headers
				else if( (!MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_END_SHIFT_CD, &bBackup ) &&
					!MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_EVENT_CD, &bBackup ) &&
					!MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_START_SHIFT_CD, &bBackup ) &&
					!MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_TAB_ACK_CD, &bBackup ) ) ||
					MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_TRANSACTION_CD, &bBackup ) )
				{
					// CSSI don't use this message
					WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
					MSG_GEA_HEADER_Delete_All( pHeader );
					WorkTrace( pcParams, "\r\n<== Free line" ); 
					FreeMessage( &pcMessage );
					// Get the next message
					dwError = GEAReadNextMessage( pcParams, hFile, &pcMessage );
					continue;
				}
				else
				{
					// Convert the message and add it in the appropriate file
					ProcessMessage( pcParams, pHeader, pcMessage );
				}
			}
			else
			{
				WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
				MSG_GEA_HEADER_Delete_All( pHeader );
				WorkTrace( pcParams, "\r\n<== Free line" ); 
				FreeMessage( &pcMessage );
				CloseAllFiles();

				return GetLastError();
			}

			WorkTrace( pcParams, "\r\n<- Deallocate GEA message header" );
			MSG_GEA_HEADER_Delete_All( pHeader );
		}
		else
		{
			WorkTrace( pcParams, "\r\n<== Free line" ); 
			FreeMessage( &pcMessage );
			CloseAllFiles();

			return GetLastError();
		}

		// Get the next message
		WorkTrace( pcParams, "\r\n<== Free line" );
		FreeMessage( &pcMessage );
		dwError = GEAReadNextMessage( pcParams, hFile, &pcMessage );
	}

	if ( dwError == ERROR_HANDLE_EOF )
		dwError = NO_ERROR;

	WorkTrace( pcParams, "\r\n<== Free line" ); 
	FreeMessage( &pcMessage );
	CloseAllFiles();

	return dwError;
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL IsLaneInList( IN CMW_PARAMS * pcParams,                       */
/*                           IN DWORD dwLaneNumber )                         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function checks if the lane ID is in the parameter list.   */
/*            The entry lane messages should not be converted so the         */
/*            parameter list should contain the entry lane IDs.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN DWORD dwLaneNumber    - Lane ID                                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  If the lane ID is in the list.                     */
/*  FALSE                 If the lane ID is not in the list.                 */
/*****************************************************************************/

PRIVATE BOOL IsLaneInList( IN CMW_PARAMS * pcParams, IN DWORD dwLaneNumber )
{
	char * pcToken;
	char szLaneList[MAX_PATH];
	char *next_token1 = NULL;

	if ( strlen( pcParams->szLaneList ) == 0 )
		return TRUE;

	strncpy_s(szLaneList, sizeof(szLaneList), pcParams->szLaneList, MAX_PATH);

	pcToken = strtok_s(szLaneList, ";", &next_token1);
	while( pcToken != NULL )
	{
		if ( ( DWORD ) atol( pcToken ) == dwLaneNumber )
			return TRUE;

		// Get next token
		pcToken = strtok_s(NULL, ";", &next_token1);
	}

	return FALSE;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD CreateFileName( IN CMW_PARAMS * pcParams,                    */
/*                              IN int iLaneIndex,                           */
/*                              IN DWORD dwPlazaID,                          */
/*                              IN BOOL bBackup,                             */
/*                              OUT char * pcFileName,						 */
/*								DWORD dwMaxFileNameSize)                     */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function creates the file name by following the CSSI       */
/*            naming convention.                                             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN int iLaneIndex        - Index of the structure that contains the  */
/*                                 context data for the appropriate lane (in */
/*                                 the array of the context structures)      */
/*      IN DWORD dwPlazaID       - Plaza ID                                  */
/*      IN BOOL bBackup          - TRUE if the file name needs to have BAK   */
/*                                 prefix (bacup files).                     */
/*      OUT char * pcFileName    - File name                                 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PRIVATE DWORD CreateFileName( IN CMW_PARAMS * pcParams,
							  IN int iLaneIndex,
							  IN DWORD dwPlazaID,
							  IN BOOL bBackup,
							  OUT char * pcFileName,
							  DWORD dwMaxFileNameSize)
{
	if( bBackup == FALSE)
	{
		// Increase file counter
		if( sContext.sLanes[iLaneIndex].sSaved.dwFileCounter >= 99999999 )
			sContext.sLanes[iLaneIndex].sSaved.dwFileCounter = 1;
		else
			sContext.sLanes[iLaneIndex].sSaved.dwFileCounter ++;

		sprintf_s( pcFileName,
				dwMaxFileNameSize,
				 "%s%s%04d%s%04d%s%08d",
				 pcParams->szTempDir,
				 "\\MSG.PCS",
				 dwPlazaID,
				 ".L",
				 atol( sContext.sLanes[iLaneIndex].sSaved.szLaneNumber ),
				 ".S",
				 sContext.sLanes[iLaneIndex].sSaved.dwFileCounter);
	}
	else
	{
		// Increase backup file counter
		if( sContext.sLanes[iLaneIndex].sSaved.dwBakFileCounter >= 99999999 )
			sContext.sLanes[iLaneIndex].sSaved.dwBakFileCounter = 1;
		else
			sContext.sLanes[iLaneIndex].sSaved.dwBakFileCounter ++;

		sprintf_s( pcFileName,
			dwMaxFileNameSize,
				 "%s%s%04d%s%04d%s%08d",
				 pcParams->szTempDir,
				 "\\BAK.PCS",
				 dwPlazaID,
				 ".L",
				 atol( sContext.sLanes[iLaneIndex].sSaved.szLaneNumber ),
				 ".S",
				 sContext.sLanes[iLaneIndex].sSaved.dwBakFileCounter);
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD SaveMessage( IN CMW_PARAMS * pcParams,                       */
/*                           IN char * szLaneNumber,                         */
/*                           IN DWORD dwPlazaNumber,                         */
/*                           IN char * pcMessage,                            */
/*                           IN DWORD dwMsgLength,                           */
/*                           IN BOOL bBackup )                               */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function stores the message converted to the string in the */
/*            file. If the file doesn't exist the function creates it.       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN char * szLaneNumber   - Lane number                               */
/*      IN DWORD dwPlazaNumber   - Plaza number                              */
/*      IN char * pcMessage      - Converted messages (CSSI format)          */
/*      IN DWORD dwMsgLength     - Message length stored in the pcMessage    */
/*                                 field                                     */
/*      IN BOOL bBackup          - TRUE if the converted message has to be   */
/*                                 stored in the backup file                 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PRIVATE DWORD SaveMessage( IN CMW_PARAMS * pcParams,
						   IN char * szLaneNumber,
						   IN DWORD dwPlazaNumber,
						   IN char * pcMessage, 
						   IN DWORD dwMsgLength,
						   IN BOOL bBackup )
{
	char  szFullPath[MAX_PATH];
	int   iLaneIndex;

	iLaneIndex = FindLaneByNumber( szLaneNumber );
	if (  iLaneIndex < 0 )
		iLaneIndex = AddNewLane( szLaneNumber );

	if ( iLaneIndex <0 )
		return ERROR_MAX_LANES;

	if ( ( bBackup == FALSE ) && ( pcParams->dwStoreInBakFiles == 0 ) )
	{
		if ( sContext.sLanes[iLaneIndex].hFile == NULL )
		{
			sprintf_s(szFullPath, sizeof(szFullPath), "%s\\", pcParams->szTempDir);
			MakeSureDirectoryPathExists( szFullPath );

			CreateFileName(pcParams, iLaneIndex, dwPlazaNumber, bBackup, szFullPath, sizeof(szFullPath));

			sContext.sLanes[iLaneIndex].hFile = CreateFile( szFullPath,
															GENERIC_WRITE,
															0,
															NULL,
															CREATE_ALWAYS,
															FILE_ATTRIBUTE_NORMAL,
															NULL );

			if ( sContext.sLanes[iLaneIndex].hFile == INVALID_HANDLE_VALUE )
			{
				return GetLastError();
			}
		}
	}
	else
	{
		if ( sContext.sLanes[iLaneIndex].hBakFile == NULL )
		{
			sprintf_s(szFullPath, sizeof(szFullPath), "%s\\", pcParams->szTempDir);
			MakeSureDirectoryPathExists( szFullPath );

			CreateFileName(pcParams, iLaneIndex, dwPlazaNumber, TRUE, szFullPath, sizeof(szFullPath));

			sContext.sLanes[iLaneIndex].hBakFile = CreateFile( szFullPath,
															   GENERIC_WRITE,
															   0,
															   NULL,
															   CREATE_ALWAYS,
															   FILE_ATTRIBUTE_NORMAL,
															   NULL );

			if ( sContext.sLanes[iLaneIndex].hBakFile == INVALID_HANDLE_VALUE )
			{
				return GetLastError();
			}
		}
	}

	if ( ( bBackup == FALSE ) && ( pcParams->dwStoreInBakFiles == 0 ) )
		return AppendMessage( sContext.sLanes[iLaneIndex].hFile, pcMessage, dwMsgLength );
	else
		return AppendMessage( sContext.sLanes[iLaneIndex].hBakFile, pcMessage, dwMsgLength );
}

/**/
/*****************************************************************************/
/*SYNTAX: void CloseAllFiles( void )                                         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            At the end of conversion this function closes all the data     */
/*            files.                                                         */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PRIVATE void CloseAllFiles( void )
{
	DWORD dwCount;

	for ( dwCount = 0; dwCount < sContext.dwLaneCount; dwCount ++ )
	{
		if ( sContext.sLanes[dwCount].hFile != NULL )
		{
			AppendMessage( sContext.sLanes[dwCount].hFile, "\0\0\0\0", 4 );
			CloseHandle( sContext.sLanes[dwCount].hFile );
			sContext.sLanes[dwCount].hFile = NULL;
		}

		if ( sContext.sLanes[dwCount].hBakFile != NULL )
		{
			AppendMessage( sContext.sLanes[dwCount].hBakFile, "\0\0\0\0", 4 );
			CloseHandle( sContext.sLanes[dwCount].hBakFile );
			sContext.sLanes[dwCount].hBakFile = NULL;
		}
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL FreeMessage( IN OUT char ** pcMessage )                       */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function dealocates the memory buffer.                     */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN OUT char ** pcMessage - Pointer to pointer to message buffer      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              If the function is successful.                     */
/*  !NO_ERROR             If the function is not successful it returns the   */
/*                        error code obtained from the HeapFree function.    */
/*****************************************************************************/

PROTECTED DWORD FreeMessage( IN OUT char ** pcMessage )
{
	DWORD dwError;

	if ( *pcMessage != NULL )
	{
		dwError = HeapFree( GetProcessHeap(), 0, *pcMessage );
		*pcMessage = NULL;
		return dwError;
	}
	else
		return NO_ERROR; 
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ProcessMessage ( IN CMW_PARAMS * pcParams,                   */
/*                               IN struct MSG_GEA_HEADER * pHeader,         */
/*                               IN char * pcMessage )                       */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function detects the type of message by decoding the      */
/*            message header. It calls appropriate message conversion        */
/*            depending on the message type and stores the converted message */
/*            in the file. This function processes al the messages except    */
/*            the transaction message.                                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams           - Pointer to the module global    */
/*                                           structure                       */
/*      IN struct MSG_GEA_HEADER * pHeader - Decoded GEA message header      */
/*      IN char * pcMessage                - Undecoded GEA message           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PRIVATE DWORD ProcessMessage ( IN CMW_PARAMS * pcParams, 
							   IN struct MSG_GEA_HEADER * pHeader,
							   IN char * pcMessage )
{
	BOOL bBackup;
	char * pConvertedMsg = NULL;
	DWORD dwError, dwMsgLength;

	if ( MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_END_SHIFT_CD, &bBackup ) )
	{
		dwError = ConvertEOS ( pcParams, pcMessage, &pConvertedMsg, &dwMsgLength );
		if ( dwError == NO_ERROR )
		{
			// Save converted message to CSSI file
			SaveMessage( pcParams,
						 pHeader->lane_number,
						 pHeader->plaza_number + pHeader->network_number * pcParams->dwNetMult,
						 pConvertedMsg,
						 dwMsgLength,
						 bBackup );
			// Free memory
			WorkTrace( pcParams, "\r\n<== Free line" ); 
			FreeMessage( &pConvertedMsg );
		}
		else
			return dwError;
	}
	else if ( MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_EVENT_CD, &bBackup ) )
	{
		dwError = ConvertEvent ( pcParams, pcMessage, &pConvertedMsg, &dwMsgLength );
		if ( dwError == NO_ERROR )
		{
			// If the GEA event is used in the CSSI database
			if ( dwMsgLength != 0 )
			{
				// Save converted message to CSSI file
				SaveMessage( pcParams,
							 pHeader->lane_number,
							 pHeader->plaza_number + pHeader->network_number * pcParams->dwNetMult,
							 pConvertedMsg,
							 dwMsgLength,
							 bBackup );
				// Free memory
				WorkTrace( pcParams, "\r\n<== Free line" ); 
				FreeMessage( &pConvertedMsg );
			}
		}
		else
			return dwError;
	}
	else if ( MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_START_SHIFT_CD, &bBackup ) )
	{
		dwError = ConvertSOS( pcParams, pcMessage, &pConvertedMsg, &dwMsgLength );
		if ( dwError == NO_ERROR )
		{
			// Save converted message to CSSI file
			SaveMessage( pcParams,
						 pHeader->lane_number,
						 pHeader->plaza_number + pHeader->network_number * pcParams->dwNetMult,
						 pConvertedMsg,
						 dwMsgLength,
						 bBackup );
			// Free memory
			WorkTrace( pcParams, "\r\n<== Free line" ); 
			FreeMessage( &pConvertedMsg );
		}
		else
			return dwError;
	}
	else if ( MSG_GEA_HEADER_Check_CD( pHeader, MSG_GEA_TAB_ACK_CD, &bBackup ) )
	{
		// ConvertTabAck ();
		// Save converted message to CSSI file
		// Free memory
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ProcessMessageII ( IN CMW_PARAMS * pcParams,                 */
/*                                 IN struct MSG_GEA_HEADER_II * pHeader,    */
/*                                 IN char * pcMessage )                     */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes all the messages that contain the new  */
/*            header type except the transaction message. It detects the     */
/*            message type by decoding the message header and calls the      */
/*            appropriate conversion function. After conversion it stores    */
/*            converted message in the file.                                 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams              - Pointer to the module global */
/*                                              structure                    */
/*      IN struct MSG_GEA_HEADER_II * pHeader - Decoded GEA message header   */
/*      IN char * pcMessage                   - Undecoded GEA message        */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PRIVATE DWORD ProcessMessageII ( IN CMW_PARAMS * pcParams, 
							     IN struct MSG_GEA_HEADER_II * pHeader,
							     IN char * pcMessage )
{
	BOOL bBackup;
	char * pConvertedMsg = NULL;
	DWORD dwError, dwMsgLength;

	if ( MSG_GEA_HEADER_II_Check_CD( pHeader, MSG_GEA_END_SHIFT_II_CD, &bBackup ) )
	{
		dwError = ConvertEOSII ( pcParams, pcMessage, &pConvertedMsg, &dwMsgLength );
		if ( dwError == NO_ERROR )
		{
			// Save converted message to CSSI file
			SaveMessage( pcParams,
						 pHeader->lane_number,
						 pHeader->plaza_number + pHeader->network_number * pcParams->dwNetMult,
						 pConvertedMsg,
						 dwMsgLength,
						 bBackup );
			// Free memory
			WorkTrace( pcParams, "\r\n<== Free line" ); 
			FreeMessage( &pConvertedMsg );
		}
		else
			return dwError;
	}

	return NO_ERROR;
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
PRIVATE VOID AdjustStructIItoIII( struct MSG_GEA_TRANSACTION_II		*lpTrsII,
								  struct MSG_GEA_TRANSACTION_III	*lpTrsIII )
{
	char	szAddDigits[10]		= { 0 };
	CHAR	szEntryNetId[10]	= { 0 };
	
//	memcpy( pTransaction, pTransBuffer, sizeof( struct MSG_GEA_TRANSACTION_II ) );

	//

	// Message Header
	memcpy( &lpTrsIII->header, &lpTrsII->header, sizeof( struct MSG_GEA_HEADER_II ) );    
    
    // Shift Reference
	lpTrsIII->ref_shift.lane_mode		= lpTrsII->ref_shift.lane_mode;	
	lpTrsIII->ref_shift.tag_status		= lpTrsII->ref_shift.tag_status;	
	lpTrsIII->ref_shift.collector_id	= lpTrsII->ref_shift.collector_id;	
    lpTrsIII->ref_shift.start_of_msg_id	= lpTrsII->ref_shift.start_of_msg_id;    
    lpTrsIII->ref_shift.start_time		=lpTrsII->ref_shift.start_time;
	
	
    // Transaction Reference
	lpTrsIII->ref_trans.id		= lpTrsII->ref_trans.id;	
	lpTrsIII->ref_trans.index	= lpTrsII->ref_trans.index;
	
	
   // Transaction Details
	lpTrsIII->det_trans.keyed_class				= lpTrsII->det_trans.keyed_class;	
	lpTrsIII->det_trans.keyed_entry_plaza_id	= lpTrsII->det_trans.keyed_entry_plaza_id;	
	lpTrsIII->det_trans.tag_entry_network		= lpTrsII->det_trans.tag_entry_network;	
	lpTrsIII->det_trans.tag_entry_plaza			= lpTrsII->det_trans.tag_entry_plaza;	
	lpTrsIII->det_trans.tag_entry_lane			= lpTrsII->det_trans.tag_entry_lane;
	lpTrsIII->det_trans.issuing_level			= lpTrsII->det_trans.issuing_level;	
	lpTrsIII->det_trans.entry_time				= lpTrsII->det_trans.entry_time;	
	lpTrsIII->det_trans.transit_ticket_type		= lpTrsII->det_trans.transit_ticket_type;	
	lpTrsIII->det_trans.ticket_number			= lpTrsII->det_trans.ticket_number;	
	lpTrsIII->det_trans.fare_table_ver			= lpTrsII->det_trans.fare_table_ver;	
	lpTrsIII->det_trans.fare					= lpTrsII->det_trans.fare;	
	lpTrsIII->det_trans.partial_fare			= lpTrsII->det_trans.partial_fare;	

	memcpy( lpTrsIII->det_trans.billing_code_abb, lpTrsII->det_trans.billing_code_abb, sizeof( CHAR ) * 5 );

	lpTrsIII->det_trans.authorisation_number	= lpTrsII->det_trans.authorisation_number;

	memcpy( lpTrsIII->det_trans.mop_data, lpTrsII->det_trans.mop_data, sizeof( CHAR ) * 40 );

	lpTrsIII->det_trans.acquisition_type		= lpTrsII->det_trans.acquisition_type;	
	lpTrsIII->det_trans.billing_code			= lpTrsII->det_trans.billing_code;	
	lpTrsIII->det_trans.detected_class			= lpTrsII->det_trans.detected_class;

	memcpy( lpTrsIII->det_trans.pt_date, lpTrsIII->det_trans.pt_date, sizeof( CHAR ) * 7 );

	lpTrsIII->det_trans.amount_unit				= lpTrsII->det_trans.amount_unit;	
	lpTrsIII->det_trans.product_id				= lpTrsII->det_trans.product_id;	
	lpTrsIII->det_trans.pt_serial_number		= lpTrsII->det_trans.pt_serial_number;	
	lpTrsIII->det_trans.tax						= lpTrsII->det_trans.tax;	
	lpTrsIII->det_trans.id_product_type			= lpTrsII->det_trans.id_product_type;
	
	memcpy( lpTrsIII->det_trans.tt_observation_code,	lpTrsII->det_trans.tt_observation_code, sizeof( CHAR ) * 2 );
	memcpy( lpTrsIII->det_trans.mop_observation_code,	lpTrsII->det_trans.mop_observation_code, sizeof( CHAR ) * 3 );
	memcpy( lpTrsIII->det_trans.seq_observation_code,	lpTrsII->det_trans.seq_observation_code, sizeof( CHAR ) * 2 );
	memcpy( lpTrsIII->det_trans.pass_observation_code,	lpTrsII->det_trans.pass_observation_code, sizeof( CHAR ) * 2 );
	memcpy( lpTrsIII->det_trans.transaction_code,		lpTrsII->det_trans.transaction_code, sizeof( CHAR ) * 3 );
	memcpy( lpTrsIII->det_trans.receipt_number,			lpTrsII->det_trans.receipt_number, sizeof( CHAR ) * 11 );

	lpTrsIII->det_trans.amount_in_foreign_cur	= lpTrsII->det_trans.amount_in_foreign_cur;

	strncpy_s(szEntryNetId, sizeof(szEntryNetId), &lpTrsII->det_trans.reserved6[18], 2);
	szEntryNetId[2] = '\0';
	lpTrsIII->det_trans.entry_network = atol(szEntryNetId);
	if( lpTrsIII->det_trans.entry_network == 0 )
		lpTrsIII->det_trans.entry_network = lpTrsIII->header.network_number;
	

	// Check if the exchange rate has additional digits
	if ( lpTrsII->det_trans.reserved6[5] == '+' )
	{
		lpTrsIII->det_trans.flag_format[0] = '+';
		
		strncpy_s(szAddDigits, sizeof(szAddDigits), lpTrsII->det_trans.reserved6, 5);
		szAddDigits[5] = '\0';
		if( lpTrsII->det_trans.exchange_rate != 0 )
		{
			lpTrsIII->det_trans.exchange_rate_double = ( (double) lpTrsII->det_trans.exchange_rate ) / 10 +
				( atof(szAddDigits) / 1000000 );
		}
	}
	else
	{
		lpTrsIII->det_trans.exchange_rate_double = ( (double) lpTrsII->det_trans.exchange_rate ) / 10000;
	}
	
}
/**/
/*****************************************************************************/
/*SYNTAX: DWORD ProcessTransaction( IN CMW_PARAMS * pcParams,                */
/*                                  IN char * pcMessage,                     */
/*                                  IN BOOL bBackup )                        */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function checks if the GEA transaction is completed. If    */
/*            not, it stores the transaction item in the memory array. If    */
/*            the GEA trasnaction is completed the function calls the        */
/*            transaction conversion and stores the converted message in the */
/*            file.                                                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*      IN char * pcMessage      - GEA message (not decoded)                 */
/*      IN BOOL bBackup          - TRUE if the converted message has to be   */
/*                                 stored in the backup file                 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PRIVATE DWORD ProcessTransaction( IN CMW_PARAMS	*pcParams,
								  IN char		*pcMessage,
								  IN BOOL		bBackup,
								  IN int		iVersion )
{
	DWORD dwMsgSize;
	struct MSG_GEA_TRANSACTION_II	*pTransBuffer	= NULL;
	struct MSG_GEA_TRANSACTION_III	*pTransaction;
	char * pConvertedMsg = NULL;
	DWORD dwError, dwMsgLength;

	int   iLaneIndex;

	WorkTrace( pcParams, "\r\n-->> Allocate the GEA trs. message" );
	
	pTransaction = MSG_GEA_TRANSACTION_III_New();

	if ( pTransaction != NULL )
	{

		// MFR start
		if( iVersion == 1 )
		{// Old type of transaction. Set to fit new structure
			pTransBuffer = MSG_GEA_TRANSACTION_II_New();

			if ( MSG_GEA_TRANSACTION_II_Read( pTransBuffer,
											  (BYTE *)pcMessage,
											  MAX_GEA_LINE_LENGTH,
											  &dwMsgSize ) != TRUE )
			{
				return GetLastError();
			}

			AdjustStructIItoIII( pTransBuffer,pTransaction );

			MSG_GEA_TRANSACTION_II_Delete_All( pTransBuffer );
		}
		else if( MSG_GEA_TRANSACTION_III_Read( pTransaction,
											  (BYTE *)pcMessage,
											  MAX_GEA_LINE_LENGTH,
											  &dwMsgSize ) != TRUE )
		{
			return GetLastError();
		}
		// MFR end

		// Find the lane index
		iLaneIndex = FindLaneByNumber( pTransaction->header.lane_number );
		if ( iLaneIndex < 0 )
			iLaneIndex = AddNewLane( pTransaction->header.lane_number );

		if ( iLaneIndex <0 )
		{
			WorkTrace( pcParams, "\r\n<<-- Deallocate the GEA trs. message" );
			MSG_GEA_TRANSACTION_III_Delete_All( pTransaction );
			WorkError( pcParams, "Maximum number (%d) of lanes is reached", iLaneIndex );

			return ERROR_MAX_LANES;
		}

		//**** Free mode
		if ( pTransaction->ref_shift.lane_mode == 2 )
		{
			if ( pTransaction->ref_trans.index == 9 )
			{
				// Generate the transaction for the free mode
				dwError = ConvertFreeModeTrs( pcParams,
											  pTransaction,
											  &pConvertedMsg,
											  &dwMsgLength );

				if ( dwError == NO_ERROR )
				{
					// Save converted message to CSSI file
					SaveMessage( pcParams,
								 pTransaction->header.lane_number,
								 pTransaction->header.plaza_number + pTransaction->header.network_number * pcParams->dwNetMult,
								 pConvertedMsg,
								 dwMsgLength,
								 bBackup );
				}
				// Free memory
				WorkTrace( pcParams, "\r\n<== Free line" ); 
				FreeMessage( &pConvertedMsg );

				WorkTrace( pcParams, "\r\n<<-- Deallocate the GEA trs. message" );
				MSG_GEA_TRANSACTION_III_Delete_All( pTransaction );

				return NO_ERROR;
			}
			else
			{
				// Ignore the transaction anomalies in the free mode

				WorkTrace( pcParams, "\r\n<<-- Deallocate the GEA trs. message" );
				MSG_GEA_TRANSACTION_III_Delete_All( pTransaction );

				return NO_ERROR;
			}
		}

		//**** All other modes
		// Process message 
		if ( ( ( pTransaction->det_trans.billing_code != 0 ) && 
			( pTransaction->det_trans.partial_fare >= pTransaction->det_trans.fare) &&
			( pTransaction->det_trans.fare != 0 ) )
			|| // Specific transactions (without revenue)
			( ( pTransaction->det_trans.billing_code == 0 ) &&
			( ( pTransaction->det_trans.pass_observation_code[0] == '3' ) ||// Violation
			( pTransaction->det_trans.seq_observation_code[0] == 'C' ) ) )	// Emergency
			|| // Non multi-payment transactions
			( ( ( pTransaction->det_trans.billing_code == 8 ) ||			// Requisition
			( pTransaction->det_trans.billing_code == 27 ) ||				// Exempt
			( pTransaction->det_trans.billing_code != 0 && 
			  pTransaction->det_trans.partial_fare == 0 &&
			  pTransaction->det_trans.fare == 0 ) ) &&
			( ( atol( pTransaction->det_trans.transaction_code ) == 70 ) ||
			( atol( pTransaction->det_trans.transaction_code ) == 77 ) ) ) )
		{
			// Last transaction item
			AddTrsItem( iLaneIndex, pTransaction );

			WorkTrace( pcParams,
					   "Last transaction item of %d items found (one payment)",
					   sContext.sLanes[iLaneIndex].sSaved.dwTrsCount );

			if ( pTransaction->det_trans.pass_observation_code[0] == '3' )
			{
				WorkTrace( pcParams,
						   "Violation transaction!" );
			}

			dwError = ConvertTransaction( pcParams,
										  sContext.sLanes[iLaneIndex].sSaved.szTrsItem,
										  sContext.sLanes[iLaneIndex].sSaved.dwTrsCount,
										  &pConvertedMsg,
										  &dwMsgLength );

			if ( dwError == NO_ERROR )
			{
				// Save converted message to CSSI file
				SaveMessage( pcParams,
							 pTransaction->header.lane_number,
							 pTransaction->header.plaza_number + pTransaction->header.network_number * pcParams->dwNetMult,
							 pConvertedMsg,
							 dwMsgLength,
							 bBackup );
			}
			// Free memory
			WorkTrace( pcParams, "\r\n<== Free line" ); 
			FreeMessage( &pConvertedMsg );
			DelTrsItems( iLaneIndex );
		}
		else if ( ( pTransaction->det_trans.billing_code != 0 ) && 
				  ( pTransaction->det_trans.billing_code != 8 ) && 
				  ( pTransaction->det_trans.billing_code != 27 ) && 
				  ( pTransaction->det_trans.partial_fare != 0) &&
			( pTransaction->det_trans.partial_fare < pTransaction->det_trans.fare) )
		{
			// Multi-payment
			sContext.sLanes[iLaneIndex].sSaved.dwTrsPFareSum += pTransaction->det_trans.partial_fare;
			if (sContext.sLanes[iLaneIndex].sSaved.dwTrsPFareSum >= pTransaction->det_trans.fare)
			{
				// Transaction has many items and this is the last one - convert
				AddTrsItem( iLaneIndex, pTransaction );

				WorkTrace( pcParams,
						   "Last transaction item of %d items found (multi payment)",
						   sContext.sLanes[iLaneIndex].sSaved.dwTrsCount );

				dwError = ConvertTransaction( pcParams,
											  sContext.sLanes[iLaneIndex].sSaved.szTrsItem,
											  sContext.sLanes[iLaneIndex].sSaved.dwTrsCount,
											  &pConvertedMsg,
											  &dwMsgLength );

				if ( dwError == NO_ERROR )
				{
					// Save converted message to CSSI file
					SaveMessage( pcParams,
								 pTransaction->header.lane_number,
								 pTransaction->header.plaza_number + pTransaction->header.network_number * pcParams->dwNetMult,
								 pConvertedMsg,
								 dwMsgLength,
								 bBackup );
				}
				// Free memory
				WorkTrace( pcParams, "\r\n<== Free line" ); 
				FreeMessage( &pConvertedMsg );
				DelTrsItems( iLaneIndex );
			}
			else
			{
				// This should not be the last transaction item
				if ( sContext.sLanes[iLaneIndex].sSaved.dwLastTrsIndex <= pTransaction->ref_trans.index )
				{
					// This is the first item of the next transaction
					// but the module didn't detect the end of the previous transaction
					
					// Convert the previos transaction (uncomment this part of code if it
					// is necessary to convert the transaction items without payment)
					// dwError = ConvertTransaction ( pcParams,
					//								  sContext.sLanes[iLaneIndex].sSaved.szTrsItem,
					//								  sContext.sLanes[iLaneIndex].sSaved.dwTrsCount,
					//								  &pConvertedMsg,
					//								  &dwMsgLength );
					// if ( dwError == NO_ERROR )
					// {
					//	// Save converted message to CSSI file
					//	SaveMessage( pcParams,
					//				 pTransaction->header.lane_number,
					//				 pTransaction->header.plaza_number,
					//				 pConvertedMsg,
					//				 dwMsgLength,
					//				 bBackup );
					// }
					// Free memory
					// FreeMessage( &pConvertedMsg );

					// Do not convert - ignore it (I have found the transaction items that
					// represents only the CSSI anomalies and there is no payment - unfinished
					// transaction ??)
					DelTrsItems( iLaneIndex );

					WorkError( pcParams,
							   "Transaction is not finished (ignore it)" );

					// Process the current transaction
					ProcessTransaction( pcParams, pcMessage, bBackup, 2 );
				}
				else
				{
					// This is not the last transaction item - save it
					AddTrsItem( iLaneIndex, pTransaction );

					WorkTrace( pcParams,
							   "Transaction item of %d items found is added to the list (payment message)",
							   sContext.sLanes[iLaneIndex].sSaved.dwTrsCount );
				}
			}
		}
		else
		{
			// The transaction item corresponds to the CSSI anomaly - save it
			AddTrsItem( iLaneIndex, pTransaction );

			WorkTrace( pcParams,
					   "Transaction item is added to the list (%d items found and this is transaction anomaly)",
					   sContext.sLanes[iLaneIndex].sSaved.dwTrsCount );
		}

		WorkTrace( pcParams, "\r\n<<-- Deallocate GEA trs. message" );

		MSG_GEA_TRANSACTION_III_Delete_All( pTransaction );
	}

	return NO_ERROR;
}

/*****************************************************************************/
/*                             CONTEXT FUNCTIONS                             */
/*****************************************************************************/

/**/
/*****************************************************************************/
/*SYNTAX: int FindLaneByNumber( IN char * szLaneNumber )                     */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The module keeps the parameter data for each lane in the array */
/*            of lane context data. The array contains one element per each  */
/*            lane. This function searches for the element index that        */
/*            contains data of appropriate lane ID (defined by the function  */
/*            parameter).                                                    */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN char * szLaneNumber - Lane ID                                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  -1                    If the lane ID is not found.                       */
/*  !-1                   The function returns the lane index in the array   */
/*                        of lane context data.                              */
/*****************************************************************************/

PRIVATE int FindLaneByNumber( IN char * szLaneNumber )
{
	DWORD dwCount;

	for ( dwCount = 0; dwCount < sContext.dwLaneCount; dwCount ++ )
	{
		if ( strcmp( sContext.sLanes[dwCount].sSaved.szLaneNumber, szLaneNumber ) == 0 )
			return dwCount;
	}

	return -1;
}

/**/
/*****************************************************************************/
/*SYNTAX: int AddNewLane( IN char * szLaneNumber )                           */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function creates the new element in the array of lane      */
/*            context data.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN char * szLaneNumber - Lane ID                                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  -1                    If the maximum number of array elements exceeds.   */
/* !-1                    The element index.                                 */
/*****************************************************************************/

PRIVATE int AddNewLane( IN char * szLaneNumber )
{
	if ( sContext.dwLaneCount >= MAX_LANES - 1 )
		return -1;

	sContext.dwLaneCount ++;
	strcpy_s(sContext.sLanes[sContext.dwLaneCount - 1].sSaved.szLaneNumber, 
		sizeof(sContext.sLanes[sContext.dwLaneCount - 1].sSaved.szLaneNumber),
		szLaneNumber);
	sContext.sLanes[sContext.dwLaneCount - 1].sSaved.dwFileCounter = 0;
	sContext.sLanes[sContext.dwLaneCount - 1].sSaved.dwBakFileCounter = 0;

	return sContext.dwLaneCount - 1;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD AddTrsItem( IN int iLaneIndex,                               */
/*                          IN struct MSG_GEA_TRANSACTION_II * pTransaction )*/
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function stores the GEA transaction item in the array of	 */
/*            the transaction items. The array contains the items of only    */
/*            one trasansction that should be converted when the module      */
/*            detects the end of transaction.                                */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN int iLaneIndex                               - Lane index         */
/*      IN struct MSG_GEA_TRANSACTION_II * pTransaction - The GEA            */
/*                                                        transaction item   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  ERROR_MAX_TRS_ITEMS   Returns ERROR_MAX_TRS_ITEMS if the maximum number  */
/*                        of transaction items exceeds                       */
/*****************************************************************************/

PRIVATE DWORD AddTrsItem( IN int iLaneIndex,
						  IN struct MSG_GEA_TRANSACTION_III * pTransaction )
{
	if( sContext.sLanes[iLaneIndex].sSaved.dwTrsCount >= MAX_TRS_ITEMS )
		return ERROR_MAX_TRS_ITEMS;

	// Check if the transaction item is retransmitted

	sContext.sLanes[iLaneIndex].sSaved.dwTrsCount ++;
	sContext.sLanes[iLaneIndex].sSaved.dwLastTrsIndex = pTransaction->ref_trans.index;

	memcpy( &sContext.sLanes[iLaneIndex].sSaved.szTrsItem[sContext.sLanes[iLaneIndex].sSaved.dwTrsCount - 1], 
			pTransaction,
			sizeof( struct MSG_GEA_TRANSACTION_III ) );

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: void DelTrsItems( IN int iLaneIndex )                              */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function dletes all the GEA trasaction items from the      */
/*            array of the transaction items.                                */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN int iLaneIndex - Lane index in the list of the lane data stored   */
/*                          in the context structure                         */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PRIVATE void DelTrsItems( IN int iLaneIndex )
{
	sContext.sLanes[iLaneIndex].sSaved.dwTrsCount = 0;
	sContext.sLanes[iLaneIndex].sSaved.dwTrsPFareSum = 0;
	sContext.sLanes[iLaneIndex].sSaved.dwLastTrsIndex = 10;
	memset( &sContext.sLanes[iLaneIndex].sSaved.szTrsItem,
			0,
			sizeof( struct MSG_GEA_TRANSACTION_II ) * MAX_TRS_ITEMS );
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD SaveContexts( IN CMW_PARAMS * pcParams )                     */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function saves the array of context data to the file.      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PROTECTED DWORD SaveContexts( IN CMW_PARAMS * pcParams )
{
	DWORD dwCount;
	HANDLE hCtxFile;
	char szFullPath[MAX_PATH];
	DWORD dwLength, dwWrite;

	if ( sContext.dwLaneCount == 0 )
		return NO_ERROR;

	sprintf_s(szFullPath, sizeof(szFullPath), "%s\\", pcParams->szCtxDir);
	MakeSureDirectoryPathExists( szFullPath );

	for ( dwCount = 0; dwCount < sContext.dwLaneCount; dwCount ++ )
	{
		sprintf_s( szFullPath,
				sizeof(szFullPath),
				 "%s\\Context.%s",
				 pcParams->szCtxDir,
				 sContext.sLanes[dwCount].sSaved.szLaneNumber );

		hCtxFile = CreateFile( szFullPath,
							   GENERIC_WRITE,
							   0,
							   NULL,
							   CREATE_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL );

		if ( hCtxFile == INVALID_HANDLE_VALUE )
		{
			return GetLastError();
		}
		else
		{
			dwLength = sizeof( sContext.sLanes[dwCount].sSaved );
			if ( ! WriteFile( hCtxFile, &sContext.sLanes[dwCount].sSaved , dwLength, &dwWrite, NULL ) )
			{
				CloseHandle( hCtxFile );
				return GetLastError();
			}
			if ( dwWrite != dwLength )
			{
				CloseHandle( hCtxFile );
				return GetLastError();
			}

			CloseHandle( hCtxFile );
		}
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD LoadContexts( IN CMW_PARAMS * pcParams )                     */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function loads the context file in the memory array of     */
/*            context data.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams - Pointer to the module global structure    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              Returns NO_ERROR if the function is successful     */
/*  !NO_ERROR             Returns the result of the GetLastError function if */
/*                        it is not successful                               */
/*****************************************************************************/

PROTECTED DWORD LoadContexts( IN CMW_PARAMS * pcParams )
{
	HANDLE hFind, hCtxFile;
	WIN32_FIND_DATA sFind;
	char szFullMask[MAX_PATH];
	char szSourceFile[MAX_PATH];
	char szBuffer[sizeof( struct CMW_SAVED_CONTEXT )];
	DWORD dwRead;
	// MFR start
	DWORD	dwContextSize = 0;
	DWORD	dwOldContextSize = 0;
	DWORD	dwOldArraySize = 0;
	DWORD	dwDataOffset = 0;
	struct	MSG_GEA_TRANSACTION_II* lpTransaction = NULL;
	int		i;
	BOOL	bFileValid;
	// MFR end

	sContext.dwLaneCount = 0;
	sprintf_s(szFullMask, sizeof(szFullMask), "%s\\Context.*", pcParams->szCtxDir);
    szFullMask[sizeof(szFullMask)-1] = '\0';

	hFind = FindFirstFile( szFullMask, &sFind );
    while ( hFind != INVALID_HANDLE_VALUE )
    {
		if ( ( sFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
        {
			sprintf_s(szSourceFile, sizeof(szSourceFile), "%s\\%s", pcParams->szCtxDir, sFind.cFileName);

			hCtxFile = CreateFile( szSourceFile,
							   GENERIC_READ,
							   0,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL );

			if ( hCtxFile == NULL )
			{
				FindClose( hFind );
				return GetLastError();
			}
			else
			{
				// MFR start
				dwContextSize	= sizeof( struct CMW_SAVED_CONTEXT );
				dwOldArraySize	= 0;
				dwDataOffset	= 0;
				// MFR end

				if( ReadFile( hCtxFile, szBuffer, dwContextSize, &dwRead, NULL ) == TRUE )
				{
					bFileValid = FALSE;
					// MFR start
					if( dwContextSize != dwRead )
					{
						// During first start of new version of the module
						// CMW_SAVED_CONTEXT currently used will be larger than saved context file
						//	(	old context is using array of structures MSG_GEA_TRANSACTION_II
						//		and MSG_GEA_TRANSACTION_III structure is used in new context	)
						// Extracting data from old context and converting to new context
						// If we don't, we will lose any stored data

						// Get old context size
						// Remove size of MSG_GEA_TRANSACTION_III array
						dwOldArraySize = sizeof( struct MSG_GEA_TRANSACTION_II ) * MAX_TRS_ITEMS;
						dwOldContextSize -= sizeof( struct MSG_GEA_TRANSACTION_III ) * MAX_TRS_ITEMS;
						// Add size of MSG_GEA_TRANSACTION_II array
						dwOldContextSize += dwOldArraySize;

						// Check if size matches old context
						if( dwOldContextSize == dwRead )
						{// We have old context file
							// Set pointer on start of array
							lpTransaction = (struct MSG_GEA_TRANSACTION_II*)((DWORD)szBuffer + (  dwRead - dwOldArraySize) );

							// Get old array data out of buffer
							for( i = 0; i < MAX_TRS_ITEMS; i++ )
							{
								memcpy( &sContext.sLanes[sContext.dwLaneCount].sSaved.szTrsItem[i],
										&lpTransaction[i],
										sizeof( struct MSG_GEA_TRANSACTION_II ) );

								AdjustStructIItoIII( &lpTransaction[i],
													 &sContext.sLanes[sContext.dwLaneCount].sSaved.szTrsItem[i] );
							}

							dwDataOffset = sizeof( struct MSG_GEA_TRANSACTION_III ) * MAX_TRS_ITEMS;
							bFileValid = TRUE;
						}
					}
					else
						bFileValid = TRUE;
					
//					memcpy( &sContext.sLanes[sContext.dwLaneCount].sSaved,
//						szBuffer,
//						sizeof( struct CMW_SAVED_CONTEXT ) );

					if( bFileValid )
					{
						// OldArraySize corrects data to be copyed not to overwrite
						// possible data already extracted and converted from old 
						// format of context
						memcpy( &sContext.sLanes[sContext.dwLaneCount].sSaved,
								szBuffer,
								sizeof( struct CMW_SAVED_CONTEXT ) - dwDataOffset );
					}
					// MFR end
					sContext.dwLaneCount ++;
					CloseHandle( hCtxFile );
				}
				else
				{
					CloseHandle( hCtxFile );
					FindClose( hFind );
					return GetLastError();
				}
			}
		}

		if ( ! FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... )         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the trace file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN HANDLE hWork    - Pointer to module parameters                    */
/*      IN char * szFormat - Pointer to the format string                    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PROTECTED void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... )
{
    va_list pMark;
	CMW_PARAMS * pcParams = hWork;

    if ( pcParams->hTrc != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Direct_Trace_Texte_V( pcParams->hTrc, TRC_OPT_MASK, szFormat, pMark );
    }
}

/**/
/*****************************************************************************/
/*SYNTAX: void WorkError( IN HANDLE hWork, IN char * szFormat, ... )         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function writes the string in the error file.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN HANDLE hWork    - Pointer to module parameters                    */
/*      IN char * szFormat - Pointer to the format string                    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/

PROTECTED void WorkError( IN HANDLE hWork, IN char * szFormat, ... )
{
    va_list pMark;
	CMW_PARAMS * pcParams = hWork;

    if ( pcParams->hErr != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Direct_Trace_Texte_V( pcParams->hErr, TRC_OPT_MASK, szFormat, pMark );
    }
}
/*-------------------------------- END OF FILE ------------------------------*/