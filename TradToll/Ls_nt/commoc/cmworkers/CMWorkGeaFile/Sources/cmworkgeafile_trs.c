/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_trs.c                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions that manage conversion from  */
/*             the GEA transaction message to the CSSI transaction message.  */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>

// CSSI interface
#include <trc.h>
#include <reg.h>
#include <msg_gea_lc_transaction_ii.h>
#include <msg_gea_lc_transaction_iii.h>
#include <msg_gea_lc_endshift.h>

#include <msg_lc_payment.h>
#include <msg_lc_transaction.h>
#include <msg_lc_comp_inf_tr.h>
// Module interface
#include <cmworkgeafile.h>
#include <cmworkgeafile_convert.h>
#include <cmworkgeafile_dbconf.h>
#define LOC_DEF
#include <cmworkgeafile_trs.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE DWORD CreatePmtMessage ( OUT struct MSG_PAYMENT ** pPmtMsg );
PRIVATE void ClosePmtMessage( IN CMW_PARAMS * pcParams,
							  IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							  IN DWORD dwCounter,
							  IN DWORD dwCorrection,
							  IN OUT struct MSG_PAYMENT ** pPmtMsg );
PRIVATE DWORD AddPmtAnomaly( IN CMW_PARAMS * pcParams,
							 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							 IN OUT struct MSG_PAYMENT ** pPmtMsg );
PRIVATE DWORD CreateTrsCompInfo( IN CMW_PARAMS * pcParams,
								 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
								 IN DWORD dwTckNum,
								 OUT struct MSG_COMP_INF_TR ** pTrsInfo );
PRIVATE DWORD CreateTrsMessage( OUT struct MSG_TRANSACTION ** pTrsMsg );
PRIVATE void CloseTrsMessage( IN CMW_PARAMS * pcParams,
							  IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							  IN BOOL bMultiPmt,
							  IN BOOL bFCurrPmt,
							  IN DWORD dwEntryLane,
							  IN OUT struct MSG_TRANSACTION ** pTrsMsg );
PRIVATE DWORD AddTrsAnomaly( IN CMW_PARAMS * pcParams,
							 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							 IN OUT struct MSG_TRANSACTION ** pTrsMsg );
PRIVATE DWORD FindPmtType( IN DWORD dwGEABillingCode,
						   IN char cPassObsCode,
						   IN char cSeqObsCode );
PRIVATE DWORD FindPmtSubType( IN DWORD dwGEABillingCode,
							  IN char * cMopObsCode );
PRIVATE DWORD FindCurrencyID( IN CMW_PARAMS * pcParams,
							  IN char * pcCurr,
							  OUT DWORD * pdwCurr );
PRIVATE DWORD GEAMopToAnomaly( IN char * pcGEAMopObsCode );
PRIVATE DWORD GEAPassToAnomaly( IN char cGEAPassObsCode );
PRIVATE DWORD GEASeqToAnomaly( IN char cGEASeqObsCode );
PRIVATE DWORD GEATTToAnomaly( IN char cGEATTObsCode );
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertTransaction (                                         */
/*                             IN CMW_PARAMS * pcParams,                     */
/*                             IN struct MSG_GEA_TRANSACTION_II * pTrsItems, */
/*                             IN DWORD dwNumTrsItems,                       */
/*                             OUT char ** pConvertedMsg,                    */
/*                             OUT DWORD * pdwMsgLength )                    */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            When the ProcessTransaction function detects the end of        */
/*            GEA transaction it calls this function to convert the GEA      */
/*            transaction in the CSSI transaction.                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams                     - Pointer to the module */
/*                                                     global structure      */
/*      IN struct MSG_GEA_TRANSACTION_II * pTrsItems - Pointer to the array  */
/*                                                     of decoded GEA        */
/*                                                     transaction items     */
/*      IN DWORD dwNumTrsItems                       - Number of the GEA     */
/*                                                     transaction items in  */
/*                                                     the array             */
/*      OUT char ** pConvertedMsg                    - CSSI transaction,     */
/*                                                     payment and           */
/*                                                     complementary info    */
/*                                                     message converted to  */
/*                                                     the string            */
/*      OUT DWORD * pdwMsgLength                     - The string length     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              If the transaction is successfully converted       */
/*  !NO_ERROR             If the conversion fails the function returns the   */
/*                        error code obtained from the GetLastError function.*/
/*****************************************************************************/

PROTECTED DWORD ConvertTransaction ( IN CMW_PARAMS * pcParams,
									 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
									 IN DWORD dwNumTrsItems,
									 OUT char ** pConvertedMsg,
									 OUT DWORD * pdwMsgLength )
{
	DWORD	dwCounter;
	DWORD	dwPartialFare				= 0;
	struct	MSG_TRANSACTION * pTrs		= NULL;
	struct	MSG_COMP_INF_TR * pTrsInfo	= NULL;
	struct	MSG_PAYMENT		* pPmt		= NULL;
	char	szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD	dwMsgSize, dwOldSize;
	BOOL	bMultiPmt = FALSE, bFCurrPmt= FALSE;
	DWORD	dwError, dwTicketNumber		= 0;

	*pdwMsgLength = 0;
	if( dwNumTrsItems == 0 )
		return NO_ERROR;

	WorkTrace( pcParams,
			   "*** Transaction conversion started (transaction has %d items)!!!",
			   dwNumTrsItems );

	dwError = CreatePmtMessage( &pPmt );
	if ( dwError != NO_ERROR )
		return dwError;
	WorkTrace( pcParams, "Payment message initialized" );
	dwError = CreateTrsMessage( &pTrs );
	if ( dwError != NO_ERROR )
	{
		MSG_PAYMENT_Delete_All ( pPmt );

		return dwError;
	}
	WorkTrace( pcParams, "Transaction message initialized" );

	for ( dwCounter = 0; dwCounter < dwNumTrsItems; dwCounter ++ )
	{
		if( pTrsItems[dwCounter].det_trans.billing_code != 0 )
			dwPartialFare = dwPartialFare + pTrsItems[dwCounter].det_trans.partial_fare;
		if(	pTrsItems[dwCounter].ref_trans.index == 9 )
		{
			dwTicketNumber = pTrsItems[dwCounter].det_trans.ticket_number;
		}

		if ( ( ( pTrsItems[dwCounter].det_trans.billing_code == 0 ) &&
			 ( pTrsItems[dwCounter].det_trans.pass_observation_code[0] != '3' ) &&	// Violation
			 ( pTrsItems[dwCounter].det_trans.seq_observation_code[0] != 'C' ) )	// Emergency
			 || 
			 ( ( ( pTrsItems[dwCounter].det_trans.billing_code == 8 ) ||			// Requisition
			 ( pTrsItems[dwCounter].det_trans.billing_code == 27 ) ) &&				// Exempt
			 ( atol( pTrsItems[dwCounter].det_trans.transaction_code ) != 70 ) &&	// Not finished by pass. simulation 
			 ( atol( pTrsItems[dwCounter].det_trans.transaction_code ) != 77 ) ) )	// Not finished by vehicle passage
		{
			// Transaction or payment anomaly

			WorkTrace( pcParams,
					   "Add the anomaly to the transaction mesage (%d. of %d items)",
					   dwCounter + 1,
					   dwNumTrsItems );

			AddPmtAnomaly( pcParams, &pTrsItems[dwCounter], &pPmt );
			AddTrsAnomaly( pcParams, &pTrsItems[dwCounter], &pTrs );
		}
		else if ( ( ( pTrsItems[dwCounter].det_trans.partial_fare != 0 ) && 
				  ( dwPartialFare >= pTrsItems[dwCounter].det_trans.fare ) &&
				  ( pTrsItems[dwCounter].det_trans.billing_code != 0 ) )
				  || // Violation
				  ( ( pTrsItems[dwCounter].det_trans.partial_fare == 0 ) && 
				  ( pTrsItems[dwCounter].det_trans.billing_code == 0 ) &&
				  ( pTrsItems[dwCounter].det_trans.pass_observation_code[0] == '3' ) )
				  || // Emergency
				  ( ( pTrsItems[dwCounter].det_trans.partial_fare == 0 ) && 
				  ( pTrsItems[dwCounter].det_trans.billing_code == 0 ) &&
				  ( pTrsItems[dwCounter].det_trans.seq_observation_code[0] == 'C' ) )
				  || // Non multi-payment transactions  
				  ( ( ( pTrsItems[dwCounter].det_trans.billing_code == 8 ) ||	// Requisition
				  ( pTrsItems[dwCounter].det_trans.billing_code == 27 ) ||		// Exempt
				  ( pTrsItems[dwCounter].det_trans.billing_code != 0 &&			// Specific case when the toll fare = 0
				    pTrsItems[dwCounter].det_trans.partial_fare == 0 &&
					pTrsItems[dwCounter].det_trans.fare == 0)) &&
				  ( ( atol( pTrsItems[dwCounter].det_trans.transaction_code ) == 70 ) ||
				  ( atol( pTrsItems[dwCounter].det_trans.transaction_code ) == 77 ) ) ) )
		{
			// Last transaction item

			WorkTrace( pcParams,
					   "The last transaction item (%d. of %d items)",
					   dwCounter + 1,
					   dwNumTrsItems );

			// Add transaction and payment anomalies
			AddPmtAnomaly( pcParams, &pTrsItems[dwCounter], &pPmt );
			AddTrsAnomaly( pcParams, &pTrsItems[dwCounter], &pTrs );

			if ( pTrsItems[dwCounter].det_trans.billing_code == 9 )
				bFCurrPmt = TRUE;
			// End of payment message
			ClosePmtMessage( pcParams, 
							 &pTrsItems[dwCounter],
							 dwCounter,
							 dwPartialFare - pTrsItems[dwCounter].det_trans.fare,
							 &pPmt );
			dwMsgSize = 0;
			memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
			MSG_PAYMENT_Write ( pPmt,
								(BYTE *)(szMsgBuff),
								PCS_MAX_MSG_LENGTH,
								&dwMsgSize );

			MSG_PAYMENT_Delete_All ( pPmt );
			pPmt = NULL;

			if ( dwMsgSize > 0 )
			{
				dwOldSize = *pdwMsgLength;
				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

				if ( *pConvertedMsg == NULL )
					// MFR start
				{
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
					MSG_TRANSACTION_Delete_All( pTrs );

					return GetLastError();
				}

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
			}
			else
			{
				MSG_TRANSACTION_Delete_All( pTrs );

				return GetLastError();
			}

			// Transaction complementary information
			CreateTrsCompInfo( pcParams, &pTrsItems[dwCounter], dwTicketNumber, &pTrsInfo );
			dwMsgSize = 0;
			memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
			MSG_COMP_INF_TR_Write( pTrsInfo,
								   (BYTE *)(szMsgBuff),
								   PCS_MAX_MSG_LENGTH, 
								   &dwMsgSize );

			MSG_COMP_INF_TR_Delete_All( pTrsInfo );
			pTrsInfo = NULL;

			if ( dwMsgSize > 0 )
			{
				dwOldSize = *pdwMsgLength;
				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;
				*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

				if ( *pConvertedMsg == NULL )
				{
					MSG_TRANSACTION_Delete_All( pTrs );

					return GetLastError();
				}

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize + 4], szMsgBuff, dwMsgSize );
			}
			else
			{
				MSG_TRANSACTION_Delete_All( pTrs );

				return GetLastError();
			}

			// End of transaction message
			CloseTrsMessage( pcParams,
							 &pTrsItems[dwCounter],
							 bMultiPmt,
							 bFCurrPmt,
							 pTrsItems[dwCounter].det_trans.tag_entry_lane,
							 &pTrs );

			dwMsgSize = 0;
			memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
			MSG_TRANSACTION_Write( pTrs,
								   (BYTE *)(szMsgBuff),
								   PCS_MAX_MSG_LENGTH, 
								   &dwMsgSize );

			MSG_TRANSACTION_Delete_All ( pTrs );
			pTrs = NULL;

			if ( dwMsgSize > 0 )
			{
				dwOldSize = *pdwMsgLength;
				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;
				*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

				if ( *pConvertedMsg == NULL )
				{
					return GetLastError();
				}

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize + 4], szMsgBuff, dwMsgSize );
			}
			else
			{
				return GetLastError();
			}

			return NO_ERROR;
		}
		else if ( ( pTrsItems[dwCounter].det_trans.partial_fare != 0 ) && 
				  ( dwPartialFare < pTrsItems[dwCounter].det_trans.fare ) &&
				  ( pTrsItems[dwCounter].det_trans.billing_code != 0 ) &&
				  ( pTrsItems[dwCounter].det_trans.billing_code != 8 ) && 
				  ( pTrsItems[dwCounter].det_trans.billing_code != 27 ) )
		{
			// End of payment message (multi payment)

			WorkTrace( pcParams,
					   "Create the payment message of the multi-payment transaction (%d. of %d items)",
					   dwCounter + 1,
					   dwNumTrsItems );

			// Add payment anomalies
			AddPmtAnomaly( pcParams, &pTrsItems[dwCounter], &pPmt );

			bMultiPmt = TRUE;
			if ( pTrsItems[dwCounter].det_trans.billing_code == 9 )
				bFCurrPmt = TRUE;

			ClosePmtMessage( pcParams, &pTrsItems[dwCounter], dwCounter, 0, &pPmt );
			dwMsgSize = 0;
			memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
			MSG_PAYMENT_Write( pPmt,
							   (BYTE *)(szMsgBuff),
							   PCS_MAX_MSG_LENGTH, 
							   &dwMsgSize );
			MSG_PAYMENT_Delete_All( pPmt );
			pPmt = NULL;

			if ( dwMsgSize > 0 )
			{
				dwOldSize = *pdwMsgLength;
				*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

				if ( *pConvertedMsg == NULL )
					// MFR start
				{
//					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
					*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength +1 );
				}// MFR end
				else// MFR start
				{
//					*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );
					*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength + 1);					
				}
				// MFR end
				


				if ( *pConvertedMsg == NULL )
				{
					MSG_TRANSACTION_Delete_All( pTrs );

					return GetLastError();
				}

				// Add CSSI message prefix
				memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
				// Add converted message
				memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
			}
			else
			{
				MSG_TRANSACTION_Delete_All( pTrs );

				return GetLastError();
			}

			// Create the new payment message
			dwError = CreatePmtMessage( &pPmt );
			if ( dwError != NO_ERROR )
			{
				MSG_TRANSACTION_Delete_All( pTrs );

				return dwError;
			}
		}
		else
		{
			WorkTrace( pcParams,
					   "Undefined transaction item (ignore message)");

			MSG_PAYMENT_Delete_All( pPmt );
			MSG_TRANSACTION_Delete_All( pTrs );

			return GetLastError();
		}
	}

	if ( pTrs != NULL )
	{
		WorkTrace( pcParams,
				   "Undetected transaction end (ignore message)" );

		MSG_TRANSACTION_Delete_All( pTrs );
	}

	if ( pPmt != NULL )
	{
		MSG_PAYMENT_Delete_All( pPmt );
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertFreeModeTrs(                                          */
/*                             IN CMW_PARAMS * pcParams,                     */
/*                             IN struct MSG_GEA_TRANSACTION_II * pTrsItems, */
/*                             OUT char ** pConvertedMsg,                    */
/*                             OUT DWORD * pdwMsgLength )                    */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            When the ProcessTransaction function detects the transaction   */
/*            made in free mode it calls this function to convert the GEA    */
/*            transaction to the CSSI statistic data (trnasaction, payment   */
/*            and complementary information).                                */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams                     - Pointer to the module */
/*                                                     global structure      */
/*      IN struct MSG_GEA_TRANSACTION_II * pTrsItems - Pointer to decoded    */
/*                                                     GEA transaction item  */
/*      OUT char ** pConvertedMsg                    - CSSI transaction,     */
/*                                                     payment and           */
/*                                                     complementary info    */
/*                                                     message converted to  */
/*                                                     the string            */
/*      OUT DWORD * pdwMsgLength                     - The string length     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  NO_ERROR              If the transaction is successfully converted       */
/*  !NO_ERROR             If the conversion fails the function returns the   */
/*                        error code returned from the GetLastError function.*/
/*****************************************************************************/

PROTECTED DWORD ConvertFreeModeTrs( IN CMW_PARAMS * pcParams,
									IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
									OUT char ** pConvertedMsg,
									OUT DWORD * pdwMsgLength )
{
	struct MSG_TRANSACTION * pTrs = NULL;
	struct MSG_COMP_INF_TR * pTrsInfo = NULL;
	struct MSG_PAYMENT * pPmt = NULL;
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD dwMsgSize, dwOldSize;
	DWORD dwError;

	WorkTrace( pcParams,
			   "*** Transaction conversion started (free mode)!!!" );

	*pdwMsgLength = 0;

	dwError = CreatePmtMessage( &pPmt );
	if ( dwError != NO_ERROR )
		return dwError;
	WorkTrace( pcParams, "Payment message initialized" );
	dwError = CreateTrsMessage( &pTrs );
	if ( dwError != NO_ERROR )
	{
		MSG_PAYMENT_Delete_All ( pPmt );
	
		return dwError;
	}
	WorkTrace( pcParams, "Transaction message initialized" );

	// Add transaction and payment anomalies
	AddPmtAnomaly( pcParams, pTrsItems, &pPmt );
	AddTrsAnomaly( pcParams, pTrsItems, &pTrs );

	// End of payment message
	ClosePmtMessage( pcParams, 
					 pTrsItems,
					 0,
					 0,
					 &pPmt );

	pPmt->type.type = 62;
	pPmt->type.sub_type = 1;
	pPmt->value.valid = 0;

	dwMsgSize = 0;
	memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
	MSG_PAYMENT_Write ( pPmt,
						(BYTE *)(szMsgBuff),
						PCS_MAX_MSG_LENGTH,
						&dwMsgSize );

	MSG_PAYMENT_Delete_All ( pPmt );
	pPmt = NULL;

	if ( dwMsgSize > 0 )
	{
		dwOldSize = *pdwMsgLength;
		*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

		// MFR start
//		if ( *pConvertedMsg == NULL )
//			*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
//		else
//			*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );
		
		if ( *pConvertedMsg == NULL )
			*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength + 1 );
		else
			*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength + 1 );
		// MFR end

		if ( *pConvertedMsg == NULL )
		{
			MSG_TRANSACTION_Delete_All( pTrs );

			return GetLastError();
		}

		// Add CSSI message prefix
		memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
		// Add converted message
		memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
	}
	else
	{
		MSG_TRANSACTION_Delete_All( pTrs );

		return GetLastError();
	}

	// Transaction complementary information
	CreateTrsCompInfo( pcParams, pTrsItems, 0, &pTrsInfo );
	dwMsgSize = 0;
	memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
	MSG_COMP_INF_TR_Write( pTrsInfo,
						   (BYTE *)(szMsgBuff),
						   PCS_MAX_MSG_LENGTH, 
						   &dwMsgSize );

	MSG_COMP_INF_TR_Delete_All( pTrsInfo );
	pTrsInfo = NULL;

	if ( dwMsgSize > 0 )
	{
		dwOldSize = *pdwMsgLength;
		*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;
		*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

		if ( *pConvertedMsg == NULL )
		{
			MSG_TRANSACTION_Delete_All( pTrs );
				return GetLastError();
		}

		// Add CSSI message prefix
		memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
		// Add converted message
		memcpy( &(*pConvertedMsg)[dwOldSize + 4], szMsgBuff, dwMsgSize );
	}
	else
	{
		MSG_TRANSACTION_Delete_All( pTrs );

		return GetLastError();
	}

	// End of transaction message
	CloseTrsMessage( pcParams, pTrsItems, FALSE, FALSE, 0, &pTrs );

	pTrs->det_trans.valid_traffic = 1;
	pTrs->det_trans.valid_payment = 0;
	pTrs->det_trans.type_of_payment = 62;
	pTrs->det_trans.sub_type_of_payment = 1;

	dwMsgSize = 0;
	memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
	MSG_TRANSACTION_Write( pTrs,
						   (BYTE *)(szMsgBuff),
						   PCS_MAX_MSG_LENGTH, 
						   &dwMsgSize );

	MSG_TRANSACTION_Delete_All ( pTrs );
	pTrs = NULL;

	if ( dwMsgSize > 0 )
	{
		dwOldSize = *pdwMsgLength;
		*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;
		*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

		if ( *pConvertedMsg == NULL )
		{
			return GetLastError();
		}

		// Add CSSI message prefix
		memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
		// Add converted message
		memcpy( &(*pConvertedMsg)[dwOldSize + 4], szMsgBuff, dwMsgSize );
	}
	else
	{
		return GetLastError();
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD ConvertTipTrs( IN CMW_PARAMS * pcParams,                     */
/*                             IN CMW_TYP_DATA * pTypData,                   */
/*                             OUT char ** pConvertedMsg,                    */
/*                             OUT DWORD * pdwMsgLength )                    */
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

PROTECTED DWORD ConvertTipTrs( IN CMW_PARAMS * pcParams,
							   IN CMW_TYP_DATA * pTypData,
							   OUT char ** pConvertedMsg,
							   OUT DWORD * pdwMsgLength )
{
	struct MSG_TRANSACTION * pTrs = NULL;

	struct MSG_PAYMENT * pPmt = NULL;
	struct MSG_PAYMENT_Sold_Product * pSoldProduct = NULL;
	struct MSG_PAYMENT_Official_Currency * pOffCurrency = NULL;
	struct MSG_PAYMENT_Info_Type * pInfoType = NULL;
	struct MSG_PAYMENT_Elem_Info * pElemInfo = NULL;
	
	char  szMsgBuff[PCS_MAX_MSG_LENGTH];
	DWORD dwMsgSize, dwOldSize;
	DWORD dwError;

	WorkTrace( pcParams,
			   "*** Transaction conversion started (TYP)!!!" );

	*pdwMsgLength = 0;

	dwError = CreatePmtMessage( &pPmt );
	if ( dwError != NO_ERROR )
		return dwError;
	WorkTrace( pcParams, "Payment message initialized" );
	dwError = CreateTrsMessage( &pTrs );
	if ( dwError != NO_ERROR )
	{
		MSG_PAYMENT_Delete_All ( pPmt );

		return dwError;
	}
	WorkTrace( pcParams, "Transaction message initialized" );

	// Fill payment message header
	pPmt->header.id = 6;
	pPmt->header.cd = MSG_PAYMENT_CD;
	pPmt->header.plaza_number = pTypData->dwPlaza + pTypData->dwNetwork * pcParams->dwNetMult;
	pPmt->header.lane_number = atol( pTypData->szLane );
	pPmt->header.M_number = 1;
	memcpy( &pPmt->header.time_of_message,
			&pTypData->sTime,
			sizeof( SYSTEMTIME ) );
	pPmt->header.toll_collector_id = pTypData->dwCollectorID;
	pPmt->header.active_vault_id = 0;
	pPmt->body.time_of_payment = pTypData->sTime;

	// Fill payment message
	pPmt->ref_trs.internal_id = 0;
	pPmt->ref_trs.start_time = pTypData->sTime;

	pSoldProduct = MSG_PAYMENT_Sold_Product_New( &pPmt->list_sold_products );
	pSoldProduct->id = 0;
	pSoldProduct->quantity = 1;
	pSoldProduct->sign = 1;

	pPmt->type.type = 97;
	pPmt->type.sub_type = 1;

	pOffCurrency = MSG_PAYMENT_Official_Currency_New( &pSoldProduct->list_official_currencies );
	pOffCurrency->id = 1;
	pOffCurrency->unit_price = pTypData->dTyp;

	pPmt->value.currency_id = 1;
	pPmt->value.in_currency = pTypData->dTyp;
	pPmt->value.valid = 1;

	// Convert payment message
	dwMsgSize = 0;
	memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
	MSG_PAYMENT_Write ( pPmt,
						(BYTE *)(szMsgBuff),
						PCS_MAX_MSG_LENGTH,
						&dwMsgSize );

	MSG_PAYMENT_Delete_All ( pPmt );
	pPmt = NULL;

	if ( dwMsgSize > 0 )
	{
		dwOldSize = *pdwMsgLength;
		*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;

		// MFR start
//		if ( *pConvertedMsg == NULL )
//			*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength );
//		else
//			*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

		if ( *pConvertedMsg == NULL )
			*pConvertedMsg = HeapAlloc( GetProcessHeap(), 0, *pdwMsgLength + 1 );
		else
			*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength + 1 );
		// MFR end

		if ( *pConvertedMsg == NULL )
		{
			MSG_TRANSACTION_Delete_All( pTrs );

			return GetLastError();
		}

		// Add CSSI message prefix
		memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
		// Add converted message
		memcpy( &(*pConvertedMsg)[dwOldSize+4], szMsgBuff, dwMsgSize );
	}
	else
	{
		MSG_TRANSACTION_Delete_All( pTrs );

		return GetLastError();
	}

	// Fill transaction header
	pTrs->header.id = 6;
	pTrs->header.cd = MSG_PAYMENT_CD;
	pTrs->header.plaza_number = pTypData->dwPlaza + pTypData->dwNetwork * pcParams->dwNetMult;
	pTrs->header.lane_number = atol( pTypData->szLane );
	pTrs->header.M_number = 1;
	memcpy( &pTrs->header.time_of_message,
			&pTypData->sTime,
			sizeof( SYSTEMTIME ) );
	pTrs->header.toll_collector_id = pTypData->dwCollectorID;
	pTrs->header.active_vault_id = 0;

	pTrs->body.time_of_transaction = pTypData->sTime;
	pTrs->det_trans.type_of_payment = 97;
	pTrs->det_trans.sub_type_of_payment = 1;

	pTrs->det_trans.transaction_class = 1;
	pTrs->det_trans.keyed_class = 1;

	pTrs->det_trans.detected_class = 0;
	pTrs->det_trans.currency_id = 1;
	pTrs->det_trans.toll_fare = pTypData->dTyp;
	pTrs->det_trans.tax = 0;
	pTrs->det_trans.toll_fare_ht = pTypData->dTyp;
	pTrs->det_trans.tax_rate = 0;

	pTrs->det_trans.correcting_type = 1;
	pTrs->det_trans.correcting_sign = 1;
	pTrs->det_trans.correcting_value = 0;
	strncpy_s(pTrs->det_trans.transaction_id, sizeof(pTrs->det_trans.transaction_id), "", 10);
	pTrs->det_trans.transaction_id[10]='\0';
	pTrs->ref_trans.start_time = pTypData->sTime;
	pTrs->ref_trans.id = 0;

	pTrs->ref_shift.start_of_msg_id = pTypData->dwSOSId;
	pTrs->ref_shift.start_time = pTypData->sTimeSOS;

	if ( pTypData->dwLaneMode == 3 )
	{
		// Maintenance mode
		pTrs->det_trans.valid_traffic = 0;
		pTrs->det_trans.valid_payment = 0;
	}
	else
	{
		// Normal modes (automatic,manual and free)
		pTrs->det_trans.valid_traffic = 0;
		pTrs->det_trans.valid_payment = 0;
	}

	dwMsgSize = 0;
	memset( szMsgBuff, 0, PCS_MAX_MSG_LENGTH );
	MSG_TRANSACTION_Write( pTrs,
						   (BYTE *)(szMsgBuff),
						   PCS_MAX_MSG_LENGTH, 
						   &dwMsgSize );

	MSG_TRANSACTION_Delete_All ( pTrs );
	pTrs = NULL;

	if ( dwMsgSize > 0 )
	{
		dwOldSize = *pdwMsgLength;
		*pdwMsgLength = *pdwMsgLength + dwMsgSize + MSG_PREFIX_LENGTH;
		*pConvertedMsg = HeapReAlloc( GetProcessHeap(), 0, *pConvertedMsg, *pdwMsgLength );

		if ( *pConvertedMsg == NULL )
		{
			return GetLastError();
		}

		// Add CSSI message prefix
		memcpy( &(*pConvertedMsg)[dwOldSize], &dwMsgSize, sizeof( DWORD ) );
		// Add converted message
		memcpy( &(*pConvertedMsg)[dwOldSize + 4], szMsgBuff, dwMsgSize );
	}
	else
	{
		return GetLastError();
	}

	WorkTrace( pcParams, "TYP transaction message created" );

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD CreatePmtMessage ( OUT struct MSG_PAYMENT ** pPmtMsg )       */
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

PRIVATE DWORD CreatePmtMessage( OUT struct MSG_PAYMENT ** pPmtMsg )
{
	*pPmtMsg = MSG_PAYMENT_New();

	if ( *pPmtMsg == NULL )
		return GetLastError();

	(*pPmtMsg)->value.balance_type = 0;
	(*pPmtMsg)->value.balance_value_after_trs = 0.0;
	(*pPmtMsg)->value.valid = 1;

	(*pPmtMsg)->value.balance_value_after_trs = 0;
	(*pPmtMsg)->value.balance_type = 0;
	(*pPmtMsg)->type.acquisition_mode = 0;
	(*pPmtMsg)->type.primary_product_code[0] = 0;
	(*pPmtMsg)->type.primary_commercial_type = 0;
	(*pPmtMsg)->type.product_code_used[0] = 0;
	(*pPmtMsg)->type.commercial_type_used = 0;

    (*pPmtMsg)->list_anomalies = NULL;
	(*pPmtMsg)->list_comp_inf = NULL;

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: PRIVATE void ClosePmtMessage( IN CMW_PARAMS * pcParams,            */
/*                            IN struct MSG_GEA_TRANSACTION_II * pTrsItems,  */
/*                            IN DWORD dwCounter,                            */
/*                            IN DWORD dwCorrection,                         */
/*                            IN OUT struct MSG_PAYMENT ** pPmtMsg )         */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*                                                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN CMW_PARAMS * pcParams         - Pointer to the module global      */
/*                                         structure                         */
/*      IN struct MSG_GEA_TRANSACTION_II                                     */
/*                           * pTrsItems - GEA transaction item              */
/*      IN DWORD dwCounter               - Counter of the GEA                */
/*                                         transaction items. It is used     */
/*                                         to create the unique M_Number     */
/*                                         field for the payment             */
/*                                         messages joined to the same       */
/*                                         transaction (multi payment)       */
/*      IN DWORD dwCorrection            - Difference between the            */
/*                                         partial fare or sum of the        */
/*                                         partial fares and fare in the     */
/*                                         GEA transaction messages.         */
/*      IN OUT struct MSG_PAYMENT                                            */
/*                            ** pPmtMsg - Converted payment message         */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

PRIVATE void ClosePmtMessage( IN CMW_PARAMS * pcParams,
							  IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							  IN DWORD dwCounter,
							  IN DWORD dwCorrection,
							  IN OUT struct MSG_PAYMENT ** pPmtMsg )
{
	struct MSG_PAYMENT_Sold_Product			* pSoldProduct = NULL;
	struct MSG_PAYMENT_Official_Currency	* pOffCurrency = NULL;
	struct MSG_PAYMENT_Info_Type			* pInfoType = NULL;
	struct MSG_PAYMENT_Elem_Info			* pElemInfo = NULL;
	DWORD dwCurr;
	// MFR start
//	double dExchangeRate;
//	char szAddDigits[10];
	// MFR end  
	DWORD dwDivisor = 10;

	// Fill header
	(*pPmtMsg)->header.id = 6;
	(*pPmtMsg)->header.cd = MSG_PAYMENT_CD;
	(*pPmtMsg)->header.plaza_number = pTrsItems->header.plaza_number + pTrsItems->header.network_number * pcParams->dwNetMult;
	(*pPmtMsg)->header.lane_number = atol( pTrsItems->header.lane_number );
	(*pPmtMsg)->header.M_number = dwCounter + 1;
	memcpy( &(*pPmtMsg)->header.time_of_message,
			&pTrsItems->header.time_of_message,
			sizeof( SYSTEMTIME ) );
	(*pPmtMsg)->header.toll_collector_id = pTrsItems->ref_shift.collector_id;
	(*pPmtMsg)->header.active_vault_id = 0;

	(*pPmtMsg)->body.time_of_payment = pTrsItems->header.time_of_message;

	// Reference Transaction
	(*pPmtMsg)->ref_trs.internal_id = atol( &pTrsItems->det_trans.receipt_number[6] );
	(*pPmtMsg)->ref_trs.start_time = pTrsItems->header.time_of_message;

	pSoldProduct = MSG_PAYMENT_Sold_Product_New( &(*pPmtMsg)->list_sold_products );
	pSoldProduct->id = 0;
	pSoldProduct->quantity = 1;
	pSoldProduct->sign = 1;

	(*pPmtMsg)->type.type = FindPmtType( pTrsItems->det_trans.billing_code,
										 pTrsItems->det_trans.pass_observation_code[0],
										 pTrsItems->det_trans.seq_observation_code[0] );
	(*pPmtMsg)->type.sub_type = FindPmtSubType( pTrsItems->det_trans.billing_code,
												pTrsItems->det_trans.mop_observation_code );

	if ( ( (*pPmtMsg)->type.type == 1 ) ||			// Violation
		( pTrsItems->ref_shift.lane_mode == 3 ) ||	// Maintenance mode
		( (*pPmtMsg)->type.type == 96 ) )			// Emergency
		(*pPmtMsg)->value.valid = 0;

	// If it is the new transaction message
	// MFR start
//	if ( pTrsItems->det_trans.reserved6[5] == '+' )
	if ( pTrsItems->det_trans.flag_format[0] == '+' )
		dwDivisor = 100;
	// MFR end  
	

	pOffCurrency = MSG_PAYMENT_Official_Currency_New( &pSoldProduct->list_official_currencies );
	pOffCurrency->id = 1;
	if ( (*pPmtMsg)->type.type == 2 )				// Free way
	{
		pOffCurrency->unit_price = 0;
	}
	else
	{
		pOffCurrency->unit_price = ( (double) ( pTrsItems->det_trans.partial_fare - dwCorrection )) / dwDivisor;
	}

	if ( (*pPmtMsg)->type.type == 9 )
	{
		// Paid by foreign currency
		if( FindCurrencyID( pcParams, pTrsItems->det_trans.billing_code_abb, &dwCurr ) == NO_ERROR )
		{
			(*pPmtMsg)->value.currency_id = dwCurr;
			(*pPmtMsg)->value.in_currency = ( (double) pTrsItems->det_trans.amount_in_foreign_cur ) / dwDivisor;

			pInfoType = MSG_PAYMENT_Info_Type_New(&(*pPmtMsg)->list_comp_inf);
			pInfoType->type = 6;

			pElemInfo = MSG_PAYMENT_Elem_Info_New(&pInfoType->list_info_type);
			pElemInfo->code = 1;
			pElemInfo->info_value.type = 3;
			pElemInfo->info_value.Float = ( (double) pTrsItems->det_trans.amount_in_foreign_cur ) / dwDivisor;

			// MFR start
/*
			// Check if the exchange rate has additional digits
			if ( pTrsItems->det_trans.reserved6[5] == '+' )
			{
				strncpy( szAddDigits, pTrsItems->det_trans.reserved6, 5 );
				szAddDigits[5] = '\0';
				dExchangeRate = ( (double) pTrsItems->det_trans.exchange_rate ) / 10 +
					( atof(szAddDigits) / 1000000 );
			}
			else
			{
				dExchangeRate = ( (double) pTrsItems->det_trans.exchange_rate ) / 10000;
			}
*/

			// MFR end  
			

			pElemInfo=MSG_PAYMENT_Elem_Info_New(&pInfoType->list_info_type);
			pElemInfo->code = 2;
			pElemInfo->info_value.type = 3;
			
			// MFR start
//			pElemInfo->info_value.Float = ( ( (double) pTrsItems->det_trans.amount_in_foreign_cur ) / dwDivisor )*
//				( dExchangeRate ) -
//				( ( (double) ( pTrsItems->det_trans.partial_fare - dwCorrection ) ) / dwDivisor );

			pElemInfo->info_value.Float = ( ( (double) pTrsItems->det_trans.amount_in_foreign_cur ) / dwDivisor )*
				(  pTrsItems->det_trans.exchange_rate_double ) -
				( ( (double) ( pTrsItems->det_trans.partial_fare - dwCorrection ) ) / dwDivisor );
			// MFR end  
		}
		else
		{
			(*pPmtMsg)->value.currency_id = 1;
			(*pPmtMsg)->value.in_currency = ( (double) pTrsItems->det_trans.partial_fare ) / dwDivisor;
		}
	}
	else
	{
		(*pPmtMsg)->value.currency_id = 1;
		if ( (*pPmtMsg)->type.type == 2 )
			(*pPmtMsg)->value.in_currency = 0;
		else
			(*pPmtMsg)->value.in_currency = ( (double) pTrsItems->det_trans.partial_fare ) / dwDivisor;
	}

	if ( ( pTrsItems->det_trans.billing_code == 8 ) ||
		 ( pTrsItems->det_trans.billing_code == 37 ) ||
		 ( pTrsItems->det_trans.billing_code == 38 ) )
	{
		// Keyed in free payments
		pInfoType = MSG_PAYMENT_Info_Type_New(&(*pPmtMsg)->list_comp_inf);
		pInfoType->type = 5;

		pElemInfo = MSG_PAYMENT_Elem_Info_New(&pInfoType->list_info_type);
		pElemInfo->code = 5;

		pElemInfo->info_value.type = 4;
		strncpy_s(pElemInfo->info_value.String, sizeof(pElemInfo->info_value.String), pTrsItems->det_trans.mop_data, 14);
		pElemInfo->info_value.String[14] = '\0';
		StrTrim( pElemInfo->info_value.String, " " );
	}
	else if ( pTrsItems->det_trans.billing_code == 27 )
	{
		// CSC free payments
		pInfoType = MSG_PAYMENT_Info_Type_New(&(*pPmtMsg)->list_comp_inf);
		pInfoType->type = 5;

		pElemInfo = MSG_PAYMENT_Elem_Info_New(&pInfoType->list_info_type);
		pElemInfo->code = 5;

		pElemInfo->info_value.type = 4;
		strncpy_s(pElemInfo->info_value.String, sizeof(pElemInfo->info_value.String), &pTrsItems->det_trans.mop_data[22], 17);
		pElemInfo->info_value.String[17] = '\0';
		StrTrim( pElemInfo->info_value.String, " " );
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD AddPmtAnomaly( IN CMW_PARAMS * pcParams,                     */
/*                             IN struct MSG_GEA_TRANSACTION_II * pTrsItems, */
/*                             IN OUT struct MSG_PAYMENT ** pPmtMsg )        */
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

PRIVATE DWORD AddPmtAnomaly( IN CMW_PARAMS * pcParams,
							 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							 IN OUT struct MSG_PAYMENT ** pPmtMsg )
{
	struct MSG_PAYMENT_Anomaly * pPmtAnomaly = NULL;
	DWORD dwAnomalyId = 0;

	dwAnomalyId = GEAMopToAnomaly( pTrsItems->det_trans.mop_observation_code );

	if ( dwAnomalyId != 0 )
	{
		WorkTrace( pcParams, "Payment anomaly %d detected", dwAnomalyId );
		if ( (*pPmtMsg)->list_anomalies != NULL )
		{
			pPmtAnomaly = MSG_PAYMENT_Get_First_Anomaly( (*pPmtMsg)->list_anomalies );
			while( pPmtAnomaly != NULL && pPmtAnomaly->id != dwAnomalyId )
			{
				pPmtAnomaly = MSG_PAYMENT_Get_Next_Anomaly( (*pPmtMsg)->list_anomalies,  pPmtAnomaly );
			}
		}

		if( pPmtAnomaly == NULL )
		{
			pPmtAnomaly = MSG_PAYMENT_Anomaly_New( &(*pPmtMsg)->list_anomalies );
			if ( pPmtAnomaly != NULL )
			{
				pPmtAnomaly->id = dwAnomalyId;
				pPmtAnomaly->value = 1;
			}
		}
		else
			pPmtAnomaly->value += 1;
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD CreateTrsCompInfo(                                           */
/*                           IN CMW_PARAMS * pcParams,                       */
/*                           IN struct MSG_GEA_TRANSACTION_II * pTrsItems,   */
/*                           IN DWORD dwTckNum,                              */
/*                           OUT struct MSG_COMP_INF_TR ** pTrsInfo )        */
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

PRIVATE DWORD CreateTrsCompInfo( IN CMW_PARAMS * pcParams,
								 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
								 IN DWORD dwTckNum,
								 OUT struct MSG_COMP_INF_TR ** pTrsInfo )
{
	struct MSG_COMP_INF_TR_Info_Type * pListInfo;
	struct MSG_COMP_INF_TR_Elem_Info * pSublistInfo;

	*pTrsInfo = MSG_COMP_INF_TR_New();

	// Fill header
	(*pTrsInfo)->header.id = 6;
	(*pTrsInfo)->header.cd = MSG_COMP_INF_TR_CD;
	(*pTrsInfo)->header.plaza_number = pTrsItems->header.plaza_number + pTrsItems->header.network_number * pcParams->dwNetMult;
	(*pTrsInfo)->header.lane_number = atol( pTrsItems->header.lane_number );
	(*pTrsInfo)->header.M_number = 1;
	memcpy( &(*pTrsInfo)->header.time_of_message,
			&pTrsItems->header.time_of_message,
			sizeof( SYSTEMTIME ) );
	(*pTrsInfo)->header.toll_collector_id = pTrsItems->ref_shift.collector_id;
	(*pTrsInfo)->header.active_vault_id = 0;

	(*pTrsInfo)->body.time_of_comp_inf = pTrsItems->header.time_of_message;
	(*pTrsInfo)->ref_trs.internal_id = atol( &(*pTrsItems).det_trans.receipt_number[6] );
	(*pTrsInfo)->ref_trs.start_time = pTrsItems->header.time_of_message;

	// Add reference to the VES image
	pListInfo = MSG_COMP_INF_TR_Info_Type_New( &(*pTrsInfo)->list_comp_inf );
	pListInfo->type = 7;

	pSublistInfo = MSG_COMP_INF_TR_Elem_Info_New( &pListInfo->list_info_type );
	pSublistInfo->code = 2;
	pSublistInfo->info_value.type = 4;
	sprintf_s(pSublistInfo->info_value.String,
			sizeof(pSublistInfo->info_value.String),
			 "%02d%05d",
			 pTrsItems->ref_shift.start_of_msg_id,
			 pTrsItems->ref_trans.id );

	// Add entry ticket serial number
	if( dwTckNum != 0 )
	{
		pListInfo = MSG_COMP_INF_TR_Info_Type_New( &(*pTrsInfo)->list_comp_inf );
		pListInfo->type = 9;

		pSublistInfo = MSG_COMP_INF_TR_Elem_Info_New( &pListInfo->list_info_type );
		pSublistInfo->code = 1;
		pSublistInfo->info_value.type = 4;
		sprintf_s(pSublistInfo->info_value.String,
				sizeof(pSublistInfo->info_value.String),
				"%05d",
				dwTckNum );
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD CreateTrsMessage( OUT struct MSG_TRANSACTION ** pTrsMsg )    */
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

PRIVATE DWORD CreateTrsMessage( OUT struct MSG_TRANSACTION ** pTrsMsg )
{
	*pTrsMsg = MSG_TRANSACTION_New();

	if( *pTrsMsg == NULL )
		return GetLastError();

    // Transaction Reference
	(*pTrsMsg)->ref_trans.total_traffic = 0;
    (*pTrsMsg)->ref_trans.total_violation = 0;

	(*pTrsMsg)->det_trans.correcting_type = 1;
    (*pTrsMsg)->det_trans.correcting_sign = 1;
    (*pTrsMsg)->det_trans.correcting_value = 0;

    // Entry Information
	(*pTrsMsg)->entry_info = NULL;
	(*pTrsMsg)->list_anomalies = NULL;

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: void CloseTrsMessage( IN CMW_PARAMS * pcParams,                    */
/*                              IN struct MSG_GEA_TRANSACTION_II * pTrsItems,*/
/*                              IN BOOL bMultiPmt,                           */
/*                              IN BOOL bFCurrPmt,                           */
/*                              IN DWORD dwEntryLane,                        */
/*                              IN OUT struct MSG_TRANSACTION ** pTrsMsg )   */
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

PRIVATE void CloseTrsMessage( IN CMW_PARAMS * pcParams,
							  IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							  IN BOOL bMultiPmt,
							  IN BOOL bFCurrPmt,
							  IN DWORD dwEntryLane,
							  IN OUT struct MSG_TRANSACTION ** pTrsMsg )
{
	struct MSG_TRANSACTION_Entry_Info *pEntryInfo = NULL;
	SYSTEMTIME sTime;
	DWORD dwDivisor = 10;
	// MFR start
//	char  szEntryNetId[5];
//	DWORD dwEntryNetId;
	// MFR end  

	GetLocalTime(&sTime);

	// Fill header
	(*pTrsMsg)->header.id = 6;
	(*pTrsMsg)->header.cd = MSG_PAYMENT_CD;
	(*pTrsMsg)->header.plaza_number = pTrsItems->header.plaza_number+ pTrsItems->header.network_number * pcParams->dwNetMult;
	(*pTrsMsg)->header.lane_number = atol( pTrsItems->header.lane_number );
	(*pTrsMsg)->header.M_number = 1;
	memcpy( &(*pTrsMsg)->header.time_of_message,
			&pTrsItems->header.time_of_message,
			sizeof( SYSTEMTIME ) );
	(*pTrsMsg)->header.toll_collector_id = pTrsItems->ref_shift.collector_id;
	(*pTrsMsg)->header.active_vault_id = 0;

	(*pTrsMsg)->body.time_of_transaction = pTrsItems->header.time_of_message;
	if ( ( bMultiPmt == TRUE ) && ( bFCurrPmt == TRUE ) )
		(*pTrsMsg)->det_trans.type_of_payment = 9;
	else if ( ( bMultiPmt == TRUE ) && ( bFCurrPmt != TRUE ) )
		(*pTrsMsg)->det_trans.type_of_payment = 99;
	else
		(*pTrsMsg)->det_trans.type_of_payment = FindPmtType( pTrsItems->det_trans.billing_code,
															 pTrsItems->det_trans.pass_observation_code[0],
															 pTrsItems->det_trans.seq_observation_code[0] );
	(*pTrsMsg)->det_trans.sub_type_of_payment = FindPmtSubType( pTrsItems->det_trans.billing_code,
																pTrsItems->det_trans.mop_observation_code );

	// If it is the new type of transaction message
	if ( pTrsItems->det_trans.flag_format[0] == '+' )
		dwDivisor = 100;

	if ( (*pTrsItems).det_trans.keyed_class == 0 )
	{
		(*pTrsMsg)->det_trans.transaction_class = 1;
		(*pTrsMsg)->det_trans.keyed_class = 1;
	}
	else
	{
		(*pTrsMsg)->det_trans.transaction_class = (*pTrsItems).det_trans.keyed_class;
		(*pTrsMsg)->det_trans.keyed_class = (*pTrsItems).det_trans.keyed_class;
	}

	(*pTrsMsg)->det_trans.detected_class = (*pTrsItems).det_trans.detected_class;
	(*pTrsMsg)->det_trans.currency_id = 1;
	(*pTrsMsg)->det_trans.toll_fare = ( ( double )(*pTrsItems).det_trans.fare )/dwDivisor;
	(*pTrsMsg)->det_trans.tax = ( ( double )(*pTrsItems).det_trans.tax )/dwDivisor;
	if ( (*pTrsMsg)->det_trans.toll_fare != 0 )
	{
		(*pTrsMsg)->det_trans.toll_fare_ht = (*pTrsMsg)->det_trans.toll_fare - (*pTrsMsg)->det_trans.tax;
		(*pTrsMsg)->det_trans.tax_rate = 100*(*pTrsMsg)->det_trans.tax/(*pTrsMsg)->det_trans.toll_fare;
	}
	else
	{
		(*pTrsMsg)->det_trans.toll_fare_ht = 0;
		(*pTrsMsg)->det_trans.tax_rate = 0;
	}
	(*pTrsMsg)->det_trans.correcting_type = 1;
	(*pTrsMsg)->det_trans.correcting_sign = 1;
	(*pTrsMsg)->det_trans.correcting_value = 0;
	strncpy_s((*pTrsMsg)->det_trans.transaction_id,
			sizeof((*pTrsMsg)->det_trans.transaction_id),
			 (*pTrsItems).det_trans.receipt_number,
			 10 );
	(*pTrsMsg)->det_trans.transaction_id[10]='\0';
	(*pTrsMsg)->ref_trans.start_time = (*pTrsItems).header.time_of_message;
	(*pTrsMsg)->ref_trans.id = atol( &(*pTrsMsg)->det_trans.transaction_id[6] );

	(*pTrsMsg)->ref_shift.start_of_msg_id = (*pTrsItems).ref_shift.start_of_msg_id;
	(*pTrsMsg)->ref_shift.start_time = (*pTrsItems).ref_shift.start_time;

	if ( (*pTrsItems).ref_shift.lane_mode == 3 )
	{
		// Maintenance mode
		(*pTrsMsg)->det_trans.valid_traffic = 0;
		(*pTrsMsg)->det_trans.valid_payment = 0;
	}
	else
	{
		// Normal modes (automatic, manual and free)
		(*pTrsMsg)->det_trans.valid_traffic = 1;
		(*pTrsMsg)->det_trans.valid_payment = 1;
	}

	if ( ( (*pTrsMsg)->det_trans.type_of_payment == 1 ) ||	// Violation
		( (*pTrsMsg)->det_trans.type_of_payment == 96 ) )	// Emergency
		(*pTrsMsg)->det_trans.valid_payment = 0;

	// Entry information
	if ( (*pTrsItems).det_trans.keyed_entry_plaza_id != 0 )
	{
		if ( (*pTrsMsg)->entry_info == NULL )
			pEntryInfo = MSG_TRANSACTION_Entry_Info_New( &(*pTrsMsg)->entry_info );
		else
			pEntryInfo = MSG_TRANSACTION_Get_First_Entry_Info( &(*pTrsMsg)->entry_info );
		if( pEntryInfo != NULL )
		{
			memset( pEntryInfo, 0, sizeof( struct MSG_TRANSACTION_Entry_Info ) );

			// MFR start
			// Entry network ID decoding
//			strncpy(szEntryNetId, &(*pTrsItems).det_trans.reserved6[18], 2);
//			szEntryNetId[2] = '\0';
//			dwEntryNetId = atol(szEntryNetId);
//			if(dwEntryNetId == 0)
//				dwEntryNetId = pTrsItems->header.network_number;
			// MFR end  

			if ( pTrsItems->det_trans.entry_network == 0 )
				pTrsItems->det_trans.entry_network = pTrsItems->header.network_number;

			// MFR start
//			pEntryInfo->plaza_number = (*pTrsItems).det_trans.keyed_entry_plaza_id + dwEntryNetId * pcParams->dwNetMult;
			pEntryInfo->plaza_number = (*pTrsItems).det_trans.keyed_entry_plaza_id + pTrsItems->det_trans.entry_network * pcParams->dwNetMult;
			// MFR end  

			// If U-turn is detected
			if((*pTrsItems).det_trans.keyed_entry_plaza_id == 96 || (*pTrsItems).det_trans.keyed_entry_plaza_id == 97)
				pEntryInfo->lane_number = 0;
			else
				pEntryInfo->lane_number = dwEntryLane;

			pEntryInfo->collector_id = 0;
			pEntryInfo->destination_fare_point = pTrsItems->header.plaza_number + pTrsItems->header.network_number * pcParams->dwNetMult;
			pEntryInfo->destination_plaza_number = pTrsItems->header.plaza_number + pTrsItems->header.network_number * pcParams->dwNetMult;
			pEntryInfo->detected_entry_class = 1;
			// MFR start
//			pEntryInfo->entry_fare_point = (*pTrsItems).det_trans.keyed_entry_plaza_id + dwEntryNetId * pcParams->dwNetMult;
			pEntryInfo->entry_fare_point = (*pTrsItems).det_trans.keyed_entry_plaza_id + pTrsItems->det_trans.entry_network * pcParams->dwNetMult;
			// MFR end  
			
			pEntryInfo->keyed_entry_class = 1;
			pEntryInfo->time_of_entry_trans = (*pTrsItems).det_trans.entry_time;
			pEntryInfo->transaction_entry_class = 1;
			pEntryInfo->distribution_level = ( (*pTrsItems).det_trans.issuing_level == 2 );
		}
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD AddTrsAnomalyID( IN CMW_PARAMS * pcParams,                   */
/*                               IN DWORD dwAnomalyID,                       */
/*                               IN OUT struct MSG_TRANSACTION ** pTrsMsg )  */
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

PRIVATE DWORD AddTrsAnomalyID( IN CMW_PARAMS * pcParams,
							   IN DWORD dwAnomalyID,
							   IN OUT struct MSG_TRANSACTION ** pTrsMsg )
{
	struct MSG_TRANSACTION_Anomaly * pTrsAno = NULL;

	if ( (*pTrsMsg)->list_anomalies != NULL )
	{
		pTrsAno = MSG_TRANSACTION_Get_First_Anomaly( (*pTrsMsg)->list_anomalies );
		while( pTrsAno != NULL && pTrsAno->id != dwAnomalyID )
		{
			pTrsAno = MSG_TRANSACTION_Get_Next_Anomaly( (*pTrsMsg)->list_anomalies, pTrsAno );
		}
	}

	if ( pTrsAno == NULL )
	{
		pTrsAno = MSG_TRANSACTION_Anomaly_New( &(*pTrsMsg)->list_anomalies );
		if ( pTrsAno != NULL )
		{
			pTrsAno->id = dwAnomalyID;
			pTrsAno->value = 1;
		}
	}
	else
		pTrsAno->value += 1;

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD AddTrsAnomaly( IN CMW_PARAMS * pcParams,                     */
/*                             IN struct MSG_GEA_TRANSACTION_II * pTrsItems, */
/*                             IN OUT struct MSG_TRANSACTION ** pTrsMsg )    */
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

PRIVATE DWORD AddTrsAnomaly( IN CMW_PARAMS * pcParams,
							 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
							 IN OUT struct MSG_TRANSACTION ** pTrsMsg )
{
	DWORD dwAnomalyID = 0;

	dwAnomalyID = GEAPassToAnomaly( (*pTrsItems).det_trans.pass_observation_code[0] );
	if ( dwAnomalyID != 0 )
	{
		WorkTrace( pcParams, "Anomaly %d detected", dwAnomalyID );
		AddTrsAnomalyID( pcParams, dwAnomalyID, pTrsMsg );
	}

	dwAnomalyID = GEASeqToAnomaly( (*pTrsItems).det_trans.seq_observation_code[0] );
	if ( dwAnomalyID != 0 )
	{
		AddTrsAnomalyID( pcParams, dwAnomalyID, pTrsMsg );
	}

	dwAnomalyID = GEATTToAnomaly( (*pTrsItems).det_trans.tt_observation_code[0] );
	if ( dwAnomalyID != 0 )
	{
		AddTrsAnomalyID( pcParams, dwAnomalyID, pTrsMsg );
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD FindPmtType( IN DWORD dwGEABillingCode,                      */
/*                           IN char cPassObsCode )                          */
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

PRIVATE DWORD FindPmtType( IN DWORD dwGEABillingCode,
						   IN char cPassObsCode,
						   IN char cSeqObsCode )
{
	switch ( dwGEABillingCode )
	{
		case 0:
			if ( cPassObsCode == '3' )
				return 1;	// Violation
			else
			{
				if ( cSeqObsCode == 'C' )
					return 96;	// Emergency
				else
					return 0;	// Undefined
			}
		case 1:
			return 3;	// Cash
		case 5:
			return 7;	// Check
		case 6:
			return 98;	// Coupon
		case 7:
			return 10;	// IOU
		case 8:
			return 2;	// Requisition => Free way
		case 9:
			return 9;	// Foreign currency
		case 12:
			return 4;	// Credit card
		case 15:
			return 14;	// Pre-paid CSC
		case 16:
			return 6;	// Pre-paid TAG
		case 27:
			return 2;	// Exempt => Free way
		case 34:
			return 11;	// Voucher
		case 37:
			return 2;	// Monthly subscription
		case 38:
			return 2;	// Annual subscription
		// MFR start
		case 45:
			return 2;	// Monthly and Yearly subscriptions
		case 18:
			return 6;	// Postpaid subscriptions
		// MFR end
		default:
			return 0;	// Undefined
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD FindPmtSubType( IN DWORD dwGEABillingCode,                   */
/*                              IN char * cMopObsCode )                      */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function maps the GEA billing and observation code in the  */
/*            CS payment subtype ID                                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*       IN DWORD dwGEABillingCode  - Billing code from the GEA transaction  */
/*                                    message                                */
/*       IN char * cMopObsCode      - Observation code from the GEA          */
/*                                    transaction message                    */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 Payment subtype ID.                                */
/*****************************************************************************/

PRIVATE DWORD FindPmtSubType( IN DWORD dwGEABillingCode,
							  IN char * cMopObsCode )
{
	switch ( dwGEABillingCode )
	{
		case 27:
		case 8:
			if ( atol( cMopObsCode ) == 49 )
				return 12;
			else if ( atol( cMopObsCode ) == 50 )
				return 10;
			else if ( atol( cMopObsCode ) == 52 )
				return 2;
			else if ( atol( cMopObsCode ) == 53 )
				return 3;
			else if ( atol( cMopObsCode ) == 54 )
				return 4;
			else if ( atol( cMopObsCode ) == 55 )
				return 5;
			else if ( atol( cMopObsCode ) == 56 )
				return 14;
			else if ( atol( cMopObsCode ) == 57 )
				return 7;
			else if ( atol( cMopObsCode ) == 58 )
				return 12;
			else if ( atol( cMopObsCode ) == 59 )
				return 11;
			else if ( atol( cMopObsCode ) == 60 )
				return 10;
			else if ( atol( cMopObsCode ) == 61 )
				return 13;
			else if ( atol( cMopObsCode ) == 62 )
				return 15;
			else
				return 1;
		case 37:
			return 8;
		case 38:
			return 9;
		// MFR start
		case 45:
			return 16;
		// MFR end
		default:
			return 1;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD FindCurrencyID( IN CMW_PARAMS * pcParams,                    */
/*                              IN char * pcCurr,                            */
/*                              OUT DWORD * pdwCurr )                        */
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

PRIVATE DWORD FindCurrencyID( IN CMW_PARAMS * pcParams,
							  IN char * pcCurr,
							  OUT DWORD * pdwCurr )
{
	DWORD dwTemp, dwError;
	char szKey[MAX_PATH];
	char szCurr[4] = {0};

	// Remove blanks
	sscanf_s(pcCurr, "%s", szCurr, _countof(szCurr));

	sprintf_s(szKey,
			sizeof(szKey),
			 "%s\\%s\\%s\\",
			 "SYSTEM\\CurrentControlSet\\Services\\CommocSvc\\Parameters\\Workers",
			 pcParams->szWrkName,
			 "CurrencyTable");
	dwError = REG_Lire_Entier( HKEY_LOCAL_MACHINE, szKey, szCurr, &dwTemp );

	if ( dwError != NO_ERROR )
	{
		if ( strlen( pcParams->szDbInst ) == 0 )
			return dwError;
#ifdef CMWORK_DB_CONNECTION
		else
		{
			// Connect to database and refresh the currency mapping table
			dwError = RefreshCurrTable( pcParams );
			if( dwError == NO_ERROR )
			{
				dwError = REG_Lire_Entier( HKEY_LOCAL_MACHINE, szKey, szCurr, &dwTemp );
				if ( dwError == NO_ERROR )
					*pdwCurr = dwTemp;
			}
		}
#endif
	}
	else
	{
		*pdwCurr = dwTemp;

		return NO_ERROR;
	}

	return NO_ERROR;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEAMopToAnomaly( IN char * pcGEAMopObsCode )                 */
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

PRIVATE DWORD GEAMopToAnomaly( IN char * pcGEAMopObsCode )
{
	if ( atol( pcGEAMopObsCode ) == 1 )
		return 15;	// Black listed TAG or CSC or card
	else if ( atol( pcGEAMopObsCode ) == 2 )
		return 18;	// Expired date
	else if ( atol( pcGEAMopObsCode ) == 3 )
		return 19;	// EPM unreadable
	else if ( atol( pcGEAMopObsCode ) == 4 )
		return 16;	// Card issuer not authorised
	else if ( atol( pcGEAMopObsCode ) == 7 )
		return 22;	// Usage code not authorised
	else if ( atol( pcGEAMopObsCode ) == 14 )
		return 16;	// Issuer code not accepted
	else if ( atol( pcGEAMopObsCode ) == 15 )
		return 25;	// TAG presented twice
	else if ( atol( pcGEAMopObsCode ) == 20 )
		return 13;	// Keyed in TAG or card ID
	else if ( atol( pcGEAMopObsCode ) == 21 )
		return 13;	// Keyed in TAG or card ID
	else
		return 0;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEAPassToAnomaly( IN char cGEAPassObsCode )                  */
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

PRIVATE DWORD GEAPassToAnomaly( IN char cGEAPassObsCode )
{
	if ( cGEAPassObsCode == '6' )
		return 6;	// Class discrepancy
	else if ( cGEAPassObsCode == '7' )
		return 5;	// Passage simulation
	else
		return 0;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEASeqToAnomaly( IN char cGEASeqObsCode )                    */
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

PRIVATE DWORD GEASeqToAnomaly( IN char cGEASeqObsCode )
{
	if ( cGEASeqObsCode == '5' )
		return 32;	// Payment cancellation
	else
		return 0;
}

/**/
/*****************************************************************************/
/*SYNTAX: DWORD GEATTToAnomaly( IN char cGEATTObsCode )                      */
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

PRIVATE DWORD GEATTToAnomaly( IN char cGEATTObsCode )
{

	return 0;
}
/*-------------------------------- END OF FILE ------------------------------*/