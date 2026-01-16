/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_event.c                                           */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage conversion from  */
/*             the GEA exceptional data message to the CSSI event message.   */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>

// CSSI interface
#include <trc.h>
#include <msg_gea_lc_event.h>

#include <msg_lc_event.h>
// Module interface
#include <cmworkgeafile.h>
#define LOC_DEF
#include <cmworkgeafile_event.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_EVENT_CONVERSION
{
	DWORD  dwGEACategory;
	CHAR   szGEASubCategory[3+1];
	DWORD  dwGEAStatus;
	DWORD  dwCategory;
	DWORD  dwSubCategory;
	DWORD  dwStatus;
}CMW_EVENT_CONVERSION;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE BOOL FindEvent( IN DWORD dwGEACat,
						IN char * szGEASubCat,
						IN DWORD dwGEAStatus,
						OUT DWORD * pdwCat,
						OUT DWORD * pdwSubCat,
						OUT DWORD * pdwStatus );
/*-------------------------------- VARIABLES: -------------------------------*/
CMW_EVENT_CONVERSION sEventConversion[]={
	{  1, "002", 2,   1,  1, 2 },	// Software start
	{  2, "001", 0,   5,  4, 0 },	//  CXP  communication Error
	{  2, "001", 1,   5,  4, 1 },
	// MFR start
//	{  2, "105", 0,   5,  4, 0 },	
//	{  2, "105", 1,   5,  4, 2 },
	{  2, "105", 0,   5,  4, 0 },	// CXP presence loop fault
	{  2, "105", 1,   5,  4, 1 },
	{  2, "106", 0,   5,  4, 0 },	// CXP presence loop permanent detection
	{  2, "106", 1,   5,  4, 2 },
	// MFR end
	{  6, "001", 0,   8,  2, 0 },	// MCR  communication Error
	{  6, "001", 1,   8,  2, 1 },
	{  6, "002", 0,   8,  4, 0 },	// MCR reading error
	{  6, "002", 1,   8,  4, 1 },
	{  6, "003", 0,   8,  5, 0 },	// MCR ISO 3 writing error
	{  6, "003", 1,   8,  5, 1 },
	{  6, "004", 0,   8,  3, 0 },	// MCR jam
	{  6, "004", 1,   8,  3, 1 },
	{  8, "001", 0,   7,  2, 0 },	// ZIP  communication Error
	{  8, "001", 1,   7,  2, 1 },
	{  8, "002", 0,   7,  1, 0 },	// ZIP reading error
	{  8, "002", 1,   7,  1, 1 },
	{  9, "001", 0,   6,  2, 0 },	// RP communication Error
	{  9, "001", 1,   6,  2, 1 },
	{  9, "004", 0,   6,  4, 0 },	// RP paper jam
	{  9, "004", 1,   6,  4, 1 },
	{  9, "006", 0,   6,  3, 0 },	// RP paper lower
	{  9, "006", 1,   6,  3, 1 },
	{  9, "007", 0,   6,  3, 0 },	// RP end of paper
	{  9, "007", 1,   6,  3, 2 },
	{  9, "008", 0, 127,  1, 0 },	// RP temperature high
	{  9, "008", 1, 127,  1, 1 },
	{  9, "009", 0, 127,  2, 0 },	// RP cutter problem
	{  9, "009", 1, 127,  2, 1 },
	{  9, "010", 0, 127,  3, 0 },	// RP high head problem
	{  9, "010", 1, 127,  3, 1 },
	{ 31, "101", 0,  17,  2, 0 },	// Antenna communication. Error
	{ 31, "101", 1,  17,  2, 1 },
	{ 31, "102", 0, 127,  4, 0 },	// Antenna writing error
	{ 31, "102", 1, 127,  4, 1 },
	{ 31, "103", 0,  17,  1, 0 },	// Antenna Pertel default
	{ 31, "103", 1,  17,  1, 1 },
	{ 31, "201", 0,  18,  2, 0 },	// TGB communication. Error
	{ 31, "201", 1,  18,  2, 1 },
	{ 31, "202", 0, 127,  6, 0 },	// TGB writing error
	{ 31, "202", 1, 127,  6, 1 },
	{ 31, "203", 0,  18,  1, 0 },	// TGB Pertel default
	{ 31, "203", 1,  18,  1, 1 },
	{ 31, "204", 0, 127,  7, 0 },	// TGB marker default
	{ 31, "204", 1, 127,  7, 1 },
	{ 35, "001", 0,  16,  2, 0 },	// CSC communication. error (low level)
	{ 35, "001", 1,  16,  2, 1 },
	{ 35, "002", 0,  16,  4, 0 },	// CSC writing error (low level)
	{ 35, "002", 1,  16,  4, 1 },
	{ 35, "003", 0,  16,  4, 0 },	// CSC writing and transaction error (low level)
	{ 35, "003", 1,  16,  4, 1 },
	{ 45, "101", 0,  10,  2, 0 },	// AVC communication. Error
	{ 45, "101", 1,  10,  2, 1 },
	{ 45, "107", 0,   5, 11, 0 },	// AVC optical barrier default
	{ 45, "107", 1,   5, 11, 1 },
	{ 45, "110", 0,   5, 11, 0 },	// AVC optical barrier always activated
	{ 45, "110", 1,   5, 11, 2 },
	{ 45, "111", 0,   5, 99, 0 },	// AVC loop default
	{ 45, "111", 1,   5, 99, 1 },
	{ 45, "113", 0,  10,  3, 0 },	// AVC treadles 1 default
	{ 45, "113", 1,  10,  3, 1 },
	{ 45, "114", 0,  10,  4, 0 },	// AVC treadles 2 default
	{ 45, "114", 1,  10,  4, 1 },
	{ 45, "115", 0,  10,  5, 0 },	// AVC treadles 3 default
	{ 45, "115", 1,  10,  5, 1 },
	{ 45, "116", 0,  10,  6, 0 },	// AVC treadles 4 default
	{ 45, "116", 1,  10,  6, 1 },
	{ 45, "121", 0, 127,  8, 0 },	// AVC height detection default
	{ 45, "121", 1, 127,  8, 1 },
	{ 49, "001", 0,  19,  2, 0 },	// EPT communication. error
	{ 49, "001", 1,  19,  2, 1 },
	{ 80, "009", 2, 127,  9, 1 },	// Manual barrier open
	{ 80, "010", 2, 127, 40, 1 },	// Manual barrier closed
	{ 80, "013", 2,   5, 14, 0 },	// Overhead traffic light red
	{ 80, "014", 2,   5, 14, 1 },	// Overhead traffic light green
	{ 80, "015", 2,   4, 16, 1 },	// Reversing
	{ 80, "016", 2, 127, 10, 1 },	// Class discrepancy
	{ 80, "018", 2,   4,  9, 1 },	// Passage simulation
	{ 80, "019", 2,   4, 11, 1 },	// Forced passage
	{ 80, "020", 2, 127, 11, 1 },	// Unspecified class
	{ 80, "022", 2, 127, 12, 1 },	// Emergency
	{ 80, "024", 2, 127, 13, 1 },	// MOP in black list
	{ 80, "025", 2, 127, 14, 1 },	// Operator MOP
	{ 80, "026", 2,   4, 13, 1 },	// MOP cancellation
	{ 80, "028", 2, 127, 15, 1 },	// Plaza contentious
	{ 80, "031", 2,   1,  4, 1 },	// Opening lane
	{ 80, "032", 2,   1,  4, 0 },	// Closing lane
	{ 80, "033", 2, 127, 16, 1 },	// Refusal of remote control
	{ 80, "036", 2,   2,  2, 1 },	// PCS communication; Read the status from the event description
	{ 80, "043", 2, 127, 17, 1 },	// Beginning of break
	{ 80, "044", 2, 127, 18, 1 },	// End of break
	{ 80, "045", 2, 127, 19, 1 },	// Transit ticket problem
	{ 80, "058", 2, 127, 20, 1 },	// Towed vehicle
	{ 80, "060", 2, 127, 21, 1 },	// Vehicle with badge refused on ETC lane
	{ 80, "069", 2, 127, 22, 1 },	// Rest of the magnetic reader in the reading/invalidation sequence
	{ 80, "077", 2, 127, 23, 1 },	// Upgrade of the lane software on the next reboot
	// TFT table
	{ 50, "001", 2,   3,  4, 1 },
	{ 50, "002", 2,   3,  4, 1 },
	{ 50, "003", 2,   3,  4, 3 },
	{ 50, "004", 2,   3,  4, 2 },
	{ 50, "005", 2,   3,  4, 3 },
	{ 50, "006", 2,   3,  4, 3 },
	{ 50, "007", 2,   3,  4, 3 },
	{ 50, "008", 2,   3,  4, 3 },
	// Journey table
	{ 51, "001", 2, 127, 24, 1 },
	{ 51, "002", 2, 127, 24, 1 },
	{ 51, "003", 2, 127, 24, 3 },
	{ 51, "004", 2, 127, 24, 2 },
	{ 51, "005", 2, 127, 24, 3 },
	{ 51, "006", 2, 127, 24, 3 },
	{ 51, "007", 2, 127, 24, 3 },
	{ 51, "008", 2, 127, 24, 3 },
	// CUR table
	{ 53, "001", 2,   3,  7, 1 },
	{ 53, "002", 2,   3,  7, 1 },
	{ 53, "003", 2,   3,  7, 3 },
	{ 53, "004", 2,   3,  7, 2 },
	{ 53, "005", 2,   3,  7, 3 },
	{ 53, "006", 2,   3,  7, 3 },
	{ 53, "007", 2,   3,  7, 3 },
	{ 53, "008", 2,   3,  7, 3 },
	// AUT table
	{ 54, "001", 2,   3,  9, 1 },
	{ 54, "002", 2,   3,  9, 1 },
	{ 54, "003", 2,   3,  9, 3 },
	{ 54, "004", 2,   3,  9, 2 },
	{ 54, "005", 2,   3,  9, 3 },
	{ 54, "006", 2,   3,  9, 3 },
	{ 54, "007", 2,   3,  9, 3 },
	{ 54, "008", 2,   3,  9, 3 },
	// TCI table
	{ 56, "001", 2,   3,  6, 1 },
	{ 56, "002", 2,   3,  6, 1 },
	{ 56, "003", 2,   3,  6, 3 },
	{ 56, "004", 2,   3,  6, 2 },
	{ 56, "005", 2,   3,  6, 3 },
	{ 56, "006", 2,   3,  6, 3 },
	{ 56, "007", 2,   3,  6, 3 },
	{ 56, "008", 2,   3,  6, 3 },
	// Black list for magnetic cards
	{ 57, "001", 2,   3, 13, 1 },
	{ 57, "002", 2,   3, 13, 1 },
	{ 57, "003", 2,   3, 13, 3 },
	{ 57, "004", 2,   3, 13, 2 },
	{ 57, "005", 2,   3, 13, 3 },
	{ 57, "006", 2,   3, 13, 3 },
	{ 57, "007", 2,   3, 13, 3 },
	{ 57, "008", 2,   3, 13, 3 },
	// Black list for CSC
	{ 58, "001", 2,   3, 14, 1 },
	{ 58, "002", 2,   3, 14, 1 },
	{ 58, "003", 2,   3, 14, 3 },
	{ 58, "004", 2,   3, 14, 2 },
	{ 58, "005", 2,   3, 14, 3 },
	{ 58, "006", 2,   3, 14, 3 },
	{ 58, "007", 2,   3, 14, 3 },
	{ 58, "008", 2,   3, 14, 3 },
	// White list
	{ 59, "001", 2, 127, 25, 1 },
	{ 59, "002", 2, 127, 25, 1 },
	{ 59, "003", 2, 127, 25, 3 },
	{ 59, "004", 2, 127, 25, 2 },
	{ 59, "005", 2, 127, 25, 3 },
	{ 59, "006", 2, 127, 25, 3 },
	{ 59, "007", 2, 127, 25, 3 },
	{ 59, "008", 2, 127, 25, 3 },
	// Constant table
	{ 60, "001", 2, 127, 26, 1 },
	{ 60, "002", 2, 127, 26, 1 },
	{ 60, "003", 2, 127, 26, 3 },
	{ 60, "004", 2, 127, 26, 2 },
	{ 60, "005", 2, 127, 26, 3 },
	{ 60, "006", 2, 127, 26, 3 },
	{ 60, "007", 2, 127, 26, 3 },
	{ 60, "008", 2, 127, 26, 3 },
	// PRD table
	{ 62, "001", 2,   3,  5, 1 },
	{ 62, "002", 2,   3,  5, 1 },
	{ 62, "003", 2,   3,  5, 3 },
	{ 62, "004", 2,   3,  5, 2 },
	{ 62, "005", 2,   3,  5, 3 },
	{ 62, "006", 2,   3,  5, 3 },
	{ 62, "007", 2,   3,  5, 3 },
	{ 62, "008", 2,   3,  5, 3 },
	// Authorized media types table
	{ 63, "001", 2, 127, 27, 1 },
	{ 63, "002", 2, 127, 27, 1 },
	{ 63, "003", 2, 127, 27, 3 },
	{ 63, "004", 2, 127, 27, 2 },
	{ 63, "005", 2, 127, 27, 3 },
	{ 63, "006", 2, 127, 27, 3 },
	{ 63, "007", 2, 127, 27, 3 },
	{ 63, "008", 2, 127, 27, 3 },
	// Media table
	{ 64, "001", 2, 127, 28, 1 },
	{ 64, "002", 2, 127, 28, 1 },
	{ 64, "003", 2, 127, 28, 3 },
	{ 64, "004", 2, 127, 28, 2 },
	{ 64, "005", 2, 127, 28, 3 },
	{ 64, "006", 2, 127, 28, 3 },
	{ 64, "007", 2, 127, 28, 3 },
	{ 64, "008", 2, 127, 28, 3 },
	// Authorized reloading value table
	{ 68, "001", 2, 127, 29, 1 },
	{ 68, "002", 2, 127, 29, 1 },
	{ 68, "003", 2, 127, 29, 3 },
	{ 68, "004", 2, 127, 29, 2 },
	{ 68, "005", 2, 127, 29, 3 },
	{ 68, "006", 2, 127, 29, 3 },
	{ 68, "007", 2, 127, 29, 3 },
	{ 68, "008", 2, 127, 29, 3 },
	// General table
	{ 70, "001", 2, 127, 30, 1 },
	{ 70, "002", 2, 127, 30, 1 },
	{ 70, "003", 2, 127, 30, 3 },
	{ 70, "004", 2, 127, 30, 2 },
	{ 70, "005", 2, 127, 30, 3 },
	{ 70, "006", 2, 127, 30, 3 },
	{ 70, "007", 2, 127, 30, 3 },
	{ 70, "008", 2, 127, 30, 3 },
	// MFR start
	// Whitelist for exempts
	{ 65, "001", 2, 127, 31, 1 },
	{ 65, "002", 2, 127, 31, 1 },
	{ 65, "003", 2, 127, 31, 3 },
	{ 65, "004", 2, 127, 31, 2 },
	{ 65, "005", 2, 127, 31, 3 },
	{ 65, "006", 2, 127, 31, 3 },
	{ 65, "007", 2, 127, 31, 3 },
	{ 65, "008", 2, 127, 31, 3 },
	// Journey restriction
	{ 66, "001", 2, 127, 32, 1 },
	{ 66, "002", 2, 127, 32, 1 },
	{ 66, "003", 2, 127, 32, 3 },
	{ 66, "004", 2, 127, 32, 2 },
	{ 66, "005", 2, 127, 32, 3 },
	{ 66, "006", 2, 127, 32, 3 },
	{ 66, "007", 2, 127, 32, 3 },
	{ 66, "008", 2, 127, 32, 3 },	
	// Aditional events
	{ 35, "101", 0,  22,  2, 0 },	// CSC communication error (high level)
	{ 35, "101", 1,  22,  2, 1 },
	{ 35, "102", 0,  22,  4, 0 },	// CSC writing error (high level)
	{ 35, "102", 1,  22,  4, 1 },
	{ 35, "103", 0,  22,  4, 0 },	// CSC writing and transaction error (high level)
	{ 35, "103", 1,  22,  4, 1 },
	{ 23, "001", 0,  23,  2, 0 },	// VPR communication error
	{ 23, "001", 1,  23,  2, 1 },
	{ 31, "104", 0, 127,  5, 0 },	// Antenna marker default
	{ 31, "104", 1, 127,  5, 1 },
	{  1, "003", 0, 127, 33, 0 },	// Disk Full
	{  1, "003", 1, 127, 33, 1 },	
	{  9, "012", 2, 127, 34, 1 },	// RP printing problem
	{  9, "012", 2, 127, 34, 1 },
	{ 80, "021", 2, 127, 35, 1 },	// Class modification
	{ 80, "038", 2, 127, 36, 1 },	// Staff refused
	{ 80, "067", 2, 127, 37, 1 },	// Receipt duplicate
	{ 80, "098", 2, 127, 38, 1 },	// Licence plate discrepancy
	{ 80, "099", 2, 127, 39, 1 },	// Validation of licence plate by operator
	// MFR end  

	{  0, "000", 0,   0, 0, 0 }
};
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertEvent ( IN CMW_PARAMS * pcParams,                     */
/*                             IN char * pMessage,                           */
/*                             OUT char ** pConvertedMsg,                    */
/*                             OUT DWORD * pdwMsgLength )                    */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PROTECTED DWORD ConvertEvent ( IN CMW_PARAMS * pcParams,
							   IN char * pMessage,
							   OUT char ** pConvertedMsg,
							   OUT DWORD * pdwMsgLength )
{
	struct MSG_GEA_EVENT * pGEAEvent;
	struct MSG_EVENT * pEvent;
	DWORD dwMsgSize;
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD dwCat, dwSubCat, dwStatus;
	BOOL bResult = FALSE;

	pGEAEvent = MSG_GEA_EVENT_New ();

	if( pGEAEvent != NULL )
	{
		if( MSG_GEA_EVENT_Read( pGEAEvent, (BYTE *)pMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize ) )
		{
			if( !FindEvent( pGEAEvent->body.event_category,
							pGEAEvent->body.event_sub_category,
							pGEAEvent->body.event_status,
							&dwCat,
							&dwSubCat,
							&dwStatus ) )
			{
				MSG_GEA_EVENT_Delete_All( pGEAEvent );
				*pdwMsgLength = 0;

				return NO_ERROR;
			}

			memset ( szMsgBuff, 0, PCS_MAX_MSG_LENGTH);
			pEvent = MSG_EVENT_New();

			// Fill header
			pEvent->header.id = 6;
			pEvent->header.cd = MSG_EVENT_CD;
			pEvent->header.plaza_number = pGEAEvent->header.plaza_number + pGEAEvent->header.network_number * pcParams->dwNetMult;
			pEvent->header.lane_number = atol( pGEAEvent->header.lane_number );
			pEvent->header.M_number = 1;
			memcpy( &pEvent->header.time_of_message,
					&pGEAEvent->header.time_of_message,
					sizeof( SYSTEMTIME ) );
			pEvent->header.toll_collector_id = pGEAEvent->ref_sfift.collector_id;
			pEvent->header.active_vault_id = 0;

			// Fill message
			memcpy( &pEvent->body.time_of_event,
					&pGEAEvent->header.time_of_message,
					sizeof( SYSTEMTIME ) );

			pEvent->body.event_category = dwCat;
			pEvent->body.event_sub_category = dwSubCat;
			if ( dwCat == 2 && dwSubCat ==2)
			{
				sscanf_s( pGEAEvent->body.event_details, "%d", &pEvent->body.current_event_status );
				pEvent->body.current_event_status = !pEvent->body.current_event_status;
			}
			else
				pEvent->body.current_event_status = dwStatus;

			dwMsgSize = 0;
			bResult = MSG_EVENT_Write ( pEvent,
										(BYTE *)(szMsgBuff),
										PCS_MAX_MSG_LENGTH, 
										&dwMsgSize );

			MSG_EVENT_Delete_All ( pEvent );

			if ( bResult == FALSE )
			{
				MSG_GEA_EVENT_Delete_All( pGEAEvent );
				*pConvertedMsg = NULL;
				*pdwMsgLength = 0;
				// Put the error message in the log file
				return NO_ERROR;
			}

			if ( dwMsgSize >0 )
			{
				*pdwMsgLength = dwMsgSize + MSG_PREFIX_LENGTH;

				// MFR start
				// Bug
//				*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
				*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength  + 1);
				// MFR end

				if ( *pConvertedMsg == NULL )
				{
					MSG_GEA_EVENT_Delete_All( pGEAEvent );
					return GetLastError();
				}

				// Make CSSI message prefix
				memcpy( *pConvertedMsg, &dwMsgSize, sizeof( DWORD ) );				
				// Add converted message
				sprintf_s(&(*pConvertedMsg)[4], *pdwMsgLength, "%s", szMsgBuff);
			}
		}
		else
		{
			MSG_GEA_EVENT_Delete_All( pGEAEvent );
			return GetLastError();
		}

		MSG_GEA_EVENT_Delete_All( pGEAEvent );
	}
	else
	{
		return GetLastError();
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL FindEvent( IN DWORD dwGEACat,                                 */
/*                        IN char * szGEASubCat,                             */
/*                        IN DWORD dwGEAStatus,                              */
/*                        OUT DWORD * pdwCat,                                */
/*                        OUT DWORD * pdwSubCat,                             */
/*                        OUT DWORD * pdwStatus )                            */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PRIVATE BOOL FindEvent( IN DWORD dwGEACat,
						IN char * szGEASubCat,
						IN DWORD dwGEAStatus,
						OUT DWORD * pdwCat,
						OUT DWORD * pdwSubCat,
						OUT DWORD * pdwStatus )
{
	DWORD dwCounter;

	dwCounter = 0;

	while ( TRUE )
	{
		if( sEventConversion[dwCounter].dwGEACategory == 0 )
			return FALSE;

		if ( ( sEventConversion[dwCounter].dwGEACategory == dwGEACat ) &&
			 ( strcmp(sEventConversion[dwCounter].szGEASubCategory, szGEASubCat ) == 0 ) &&
			 ( sEventConversion[dwCounter].dwGEAStatus == dwGEAStatus ) )
		{
			*pdwCat = sEventConversion[dwCounter].dwCategory;
			*pdwSubCat = sEventConversion[dwCounter].dwSubCategory;
			*pdwStatus = sEventConversion[dwCounter].dwStatus;

			return TRUE;
		}

		dwCounter ++;
	}

	return FALSE;
}
/*-------------------------------- END OF FILE ------------------------------*/