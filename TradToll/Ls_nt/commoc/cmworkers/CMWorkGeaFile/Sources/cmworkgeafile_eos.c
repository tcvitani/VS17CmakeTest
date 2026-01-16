/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_eos.c                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage conversion from  */
/*             the GEA end of job message to the CSSI end of shift message.  */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>

// CSSI interface
#include <trc.h>
#include <msg_gea_lc_endshift.h>
#include <msg_gea_lc_endshift_ii.h>

#include <msg_lc_endshift.h>
// Module interface
#include <cmworkgeafile.h>
#include <cmworkgeafile_convert.h>
#include <cmworkgeafile_trs.h>
#define LOC_DEF
#include <cmworkgeafile_eos.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertEOS ( IN CMW_PARAMS * pcParams,                       */
/*                           IN char * pMessage,                             */
/*                           OUT char ** pConvertedMsg,                      */
/*                           OUT DWORD * pdwMsgLength )                      */
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

PROTECTED DWORD ConvertEOS ( IN CMW_PARAMS * pcParams,
							 IN char * pMessage,
							 OUT char ** pConvertedMsg,
							 OUT DWORD * pdwMsgLength )
{
	struct MSG_GEA_END_SHIFT * pGEAEos;
	struct MSG_END_SHIFT * pEos;
	DWORD dwMsgSize, dwOldSize;
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD dwError;
	CMW_TYP_DATA sTypData;

	WorkTrace( pcParams, "**** End of Shift message conversion started!" );

	*pdwMsgLength = 0;

	pGEAEos = MSG_GEA_END_SHIFT_New();
	if( pGEAEos != NULL )
	{
		if( MSG_GEA_END_SHIFT_Read( pGEAEos, (BYTE *)pMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize ) )
		{
			memset ( szMsgBuff, 0, PCS_MAX_MSG_LENGTH);

			// If there was TYP payment type
			if ( pGEAEos->counter[23] != 0 )
			{
				sTypData.dwPlaza = pGEAEos->header.plaza_number;
				sTypData.dwNetwork = pGEAEos->header.network_number;
				memcpy(sTypData.szLane, pGEAEos->header.lane_number, 3);
				sTypData.szLane[3] = '\0';
				sTypData.dwLaneMode = pGEAEos->reference.lane_mode;
				sTypData.dwCollectorID = pGEAEos->reference.collector_id;
				memcpy( &sTypData.sTime, &pGEAEos->header.time_of_message, sizeof(SYSTEMTIME) );
				sTypData.dwSOSId = pGEAEos->reference.start_shift_msg_id;
				memcpy( &sTypData.sTimeSOS, &pGEAEos->reference.time_of_start_shift, sizeof(SYSTEMTIME) );
				sTypData.dTyp = ( (double) ( pGEAEos->counter[23] )) / 10;
				dwError = ConvertTipTrs( pcParams, &sTypData, pConvertedMsg, pdwMsgLength );

				if ( dwError != NO_ERROR )
				{
					MSG_GEA_END_SHIFT_Delete_All( pGEAEos );

					return dwError;
				}

				WorkTrace( pcParams, "TYP transaction message returned to ConvertEOS function" );
			}

			pEos = MSG_END_SHIFT_New();

			// Fill header
			pEos->header.id = 6;
			pEos->header.cd = MSG_END_SHIFT_CD;
			pEos->header.plaza_number = pGEAEos->header.plaza_number + pGEAEos->header.network_number * pcParams->dwNetMult;
			pEos->header.lane_number = atol( pGEAEos->header.lane_number );
			pEos->header.M_number = 1;
			memcpy( &pEos->header.time_of_message,
					&pGEAEos->header.time_of_message,
					sizeof( SYSTEMTIME ) );
			pEos->header.toll_collector_id = pGEAEos->reference.collector_id;
			pEos->header.active_vault_id = 0;

			// Fill message
			memcpy( &pEos->body.time_of_end_shift,
					&pGEAEos->header.time_of_message,
					sizeof( SYSTEMTIME ) );   
			pEos->body.close_type = 1;
			pEos->start_ref.start_shift_msg_id = pGEAEos->reference.start_shift_msg_id;
			memcpy( &pEos->start_ref.start_shift_time, 
					&pGEAEos->reference.time_of_start_shift,
					sizeof( SYSTEMTIME ) );

			dwMsgSize = 0;
			MSG_END_SHIFT_Write ( pEos,
								  (BYTE *)(szMsgBuff),
								  PCS_MAX_MSG_LENGTH, 
								  &dwMsgSize );

			MSG_END_SHIFT_Delete_All ( pEos );

			if ( dwMsgSize >0 )
			{
				dwOldSize = *pdwMsgLength;

				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

				if ( *pConvertedMsg == NULL )
					// MFR start
//					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength + 1 );
					// MFR end
				else
					*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

				if ( *pConvertedMsg == NULL )
				{
					MSG_GEA_END_SHIFT_Delete_All( pGEAEos );
					return GetLastError();
				}				

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
			}
		}
		else
		{
			MSG_GEA_END_SHIFT_Delete_All( pGEAEos );
			return GetLastError();
		}

		MSG_GEA_END_SHIFT_Delete_All( pGEAEos );
	}
	else
	{
		return GetLastError();
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertEOSII ( IN CMW_PARAMS * pcParams,                     */
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

PROTECTED DWORD ConvertEOSII ( IN CMW_PARAMS * pcParams,
							   IN char * pMessage,
							   OUT char ** pConvertedMsg,
							   OUT DWORD * pdwMsgLength )
{
	struct MSG_GEA_END_SHIFT_II * pGEAEos;
	struct MSG_END_SHIFT * pEos;
	DWORD dwMsgSize, dwOldSize;
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD dwError;
	CMW_TYP_DATA sTypData;

	WorkTrace( pcParams, "**** End of Shift message conversion started!" );

	*pdwMsgLength = 0;

	pGEAEos = MSG_GEA_END_SHIFT_II_New();
	if( pGEAEos != NULL )
	{
		if( MSG_GEA_END_SHIFT_II_Read( pGEAEos, (BYTE *)pMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize ) )
		{
			memset ( szMsgBuff, 0, PCS_MAX_MSG_LENGTH);

			// If there was TYP payment type
			if ( ( pGEAEos->counter[29] != 0 ) || ( pGEAEos->counter[30] != 0 ) )
			{
				sTypData.dwPlaza = pGEAEos->header.plaza_number;
				sTypData.dwNetwork = pGEAEos->header.network_number;
				memcpy(sTypData.szLane, pGEAEos->header.lane_number, 3);
				sTypData.szLane[3] = '\0';
				sTypData.dwLaneMode = pGEAEos->reference.lane_mode;
				sTypData.dwCollectorID = pGEAEos->reference.collector_id;
				memcpy( &sTypData.sTime, &pGEAEos->header.time_of_message, sizeof(SYSTEMTIME) );
				sTypData.dwSOSId = pGEAEos->reference.start_shift_msg_id;
				memcpy( &sTypData.sTimeSOS, &pGEAEos->reference.time_of_start_shift, sizeof(SYSTEMTIME) );
				sTypData.dTyp = ( (double) ( pGEAEos->counter[29] * 10000 +  pGEAEos->counter[30] )) / 100;
				dwError = ConvertTipTrs( pcParams, &sTypData, pConvertedMsg, pdwMsgLength );

				if ( dwError != NO_ERROR )
				{
					MSG_GEA_END_SHIFT_II_Delete_All( pGEAEos );

					return dwError;
				}

				WorkTrace( pcParams, "TYP transaction message returned to ConvertEOSII function" );
			}

			pEos = MSG_END_SHIFT_New();

			// Fill header
			pEos->header.id = 6;
			pEos->header.cd = MSG_END_SHIFT_CD;
			pEos->header.plaza_number = pGEAEos->header.plaza_number + pGEAEos->header.network_number * pcParams->dwNetMult;
			pEos->header.lane_number = atol( pGEAEos->header.lane_number );
			pEos->header.M_number = 1;
			memcpy( &pEos->header.time_of_message,
					&pGEAEos->header.time_of_message,
					sizeof( SYSTEMTIME ) );
			pEos->header.toll_collector_id = pGEAEos->reference.collector_id;
			pEos->header.active_vault_id = 0;

			// Fill message
			memcpy( &pEos->body.time_of_end_shift,
					&pGEAEos->header.time_of_message,
					sizeof( SYSTEMTIME ) );   
			pEos->body.close_type = 1;
			pEos->start_ref.start_shift_msg_id = pGEAEos->reference.start_shift_msg_id;
			memcpy( &pEos->start_ref.start_shift_time, 
					&pGEAEos->reference.time_of_start_shift,
					sizeof( SYSTEMTIME ) );

			dwMsgSize = 0;
			MSG_END_SHIFT_Write ( pEos,
								  (BYTE *)(szMsgBuff),
								  PCS_MAX_MSG_LENGTH, 
								  &dwMsgSize );

			MSG_END_SHIFT_Delete_All ( pEos );

			if ( dwMsgSize >0 )
			{
				dwOldSize = *pdwMsgLength;

				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

				if ( *pConvertedMsg == NULL )
				{
					// MFR start
//					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength + 1 );
				}// MFR end
				else// MFR start
				{
//					*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );
					*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength + 1 );
				}
				// MFR end

				if ( *pConvertedMsg == NULL )
				{
					MSG_GEA_END_SHIFT_II_Delete_All( pGEAEos );
					return GetLastError();
				}

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
			}
		}
		else
		{
			MSG_GEA_END_SHIFT_II_Delete_All( pGEAEos );
			return GetLastError();
		}

		MSG_GEA_END_SHIFT_II_Delete_All( pGEAEos );
	}
	else
	{
		return GetLastError();
	}

	return NO_ERROR;
}
/*-------------------------------- END OF FILE ------------------------------*/