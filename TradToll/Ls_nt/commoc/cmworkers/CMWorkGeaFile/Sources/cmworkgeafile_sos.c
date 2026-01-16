/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_sos.c                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage conversion from  */
/*             the GEA beginning of job message to the CSSI start of shift   */
/*             message.                                                      */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>

// CSSI interface
#include <trc.h>
#include <msg_gea_lc_startshift.h>

#include <msg_lc_startshift.h>
// Module interface
#include <cmworkgeafile.h>
#define LOC_DEF
#include <cmworkgeafile_sos.h>
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
/*SYNTAX: DWORD ConvertSOS ( IN CMW_PARAMS * pcParams,                       */
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

PROTECTED DWORD ConvertSOS ( IN CMW_PARAMS * pcParams,
							 IN char * pMessage,
							 OUT char ** pConvertedMsg,
							 OUT DWORD * pdwMsgLength )
{
	struct MSG_GEA_START_SHIFT * pGEASos;
	struct MSG_START_SHIFT * pSos;
	DWORD dwMsgSize;
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];

	pGEASos = MSG_GEA_START_SHIFT_New();

	if( pGEASos != NULL )
	{
		if( MSG_GEA_START_SHIFT_Read( pGEASos, (BYTE *)pMessage, MAX_GEA_LINE_LENGTH, &dwMsgSize ) )
		{
			memset ( szMsgBuff, 0, PCS_MAX_MSG_LENGTH);
			pSos = MSG_START_SHIFT_New();

			// Fill header
			pSos->header.id = 6;
			pSos->header.cd = MSG_START_SHIFT_CD;
			pSos->header.plaza_number = pGEASos->header.plaza_number + pGEASos->header.network_number * pcParams->dwNetMult;
			pSos->header.lane_number = atol( pGEASos->header.lane_number );
			pSos->header.M_number = 1;
			memcpy( &pSos->header.time_of_message,
					&pGEASos->header.time_of_message,
					sizeof( SYSTEMTIME ) );
			pSos->header.toll_collector_id = pGEASos->body.collector_id;
			pSos->header.active_vault_id = 0;

			// Fill message
			memcpy( &pSos->body.time_of_start_shift,
					&pGEASos->header.time_of_message,
					sizeof( SYSTEMTIME ) );   
			pSos->body.start_of_shift_msg_id = pGEASos->body.shift_id;
			strcpy_s(pSos->body.toll_fare, sizeof(pSos->body.toll_fare), "GEA.TFT.V01");
			pSos->body.fare_point = pGEASos->header.plaza_number + pGEASos->header.network_number * pcParams->dwNetMult;
			pSos->body.fare_strip = 1;

			if( pGEASos->body.open_type == 9 )
				pSos->body.open_type = 5;
			else
				pSos->body.open_type = pGEASos->body.open_type;

			if ( pGEASos->body.lane_mode == 3 )
			{
				// Maintenance
				pSos->body.util_mode = 1;
				pSos->body.exploit_mode = 1;
			}
			else if ( ( pGEASos->body.lane_mode == 0 ) || ( pGEASos->body.lane_mode == 9 ) )
			{
				// Closed
				pSos->body.util_mode = 0;
				pSos->body.exploit_mode = 0;
			}
			else if ( pGEASos->body.lane_mode == 1 )
			{
				// Manual mode
				pSos->body.util_mode = 0;
				pSos->body.exploit_mode = 1;
			}
			else if ( pGEASos->body.lane_mode == 2 )
			{
				// Free mode
				pSos->body.util_mode = 0;
				pSos->body.exploit_mode = 9;
			}
			else if ( pGEASos->body.lane_mode == 7 )
			{
				// Automatic mode
				pSos->body.util_mode = 0;
				// In the automatic mode HAC has to make reconciliation
				// to create specific statistics based on the accounting
				// day
				pSos->body.exploit_mode = 1; //2;
			}

			dwMsgSize = 0;
			MSG_START_SHIFT_Write ( pSos,
									(BYTE *)(szMsgBuff),
									PCS_MAX_MSG_LENGTH, 
									&dwMsgSize );

			MSG_START_SHIFT_Delete_All ( pSos );

			if ( dwMsgSize >0 )
			{
				*pdwMsgLength = dwMsgSize + MSG_PREFIX_LENGTH;				

				// MFR start 8/3/2007
//				*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
				*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength + 1 );
				// MFR end   8/3/2007

				if ( *pConvertedMsg == NULL )
				{
					MSG_GEA_START_SHIFT_Delete_All( pGEASos );
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
			MSG_GEA_START_SHIFT_Delete_All( pGEASos );
			return GetLastError();
		}

		MSG_GEA_START_SHIFT_Delete_All( pGEASos );
	}
	else
	{
		return GetLastError();
	}

	return NO_ERROR;
}
/*-------------------------------- END OF FILE ------------------------------*/