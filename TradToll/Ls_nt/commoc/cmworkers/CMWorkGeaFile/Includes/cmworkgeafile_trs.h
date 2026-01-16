/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_trs.h                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKGEAFILE_TRS_H
#define CMWORKGEAFILE_TRS_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct
{
	DWORD dwPlaza;
	DWORD dwNetwork;
	char szLane[3+1];
	DWORD dwLaneMode;
	DWORD dwCollectorID;
	SYSTEMTIME sTime;
	DWORD dwSOSId;
	SYSTEMTIME sTimeSOS;
	double dTyp;
}
CMW_TYP_DATA;
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED DWORD ConvertTransaction( IN CMW_PARAMS	* pcParams,
								    IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
									IN DWORD		dwNumTrsItems,
									OUT char		** pConvertedMsg,
									OUT DWORD		* pdwMsgLength );

PROTECTED DWORD ConvertFreeModeTrs ( IN CMW_PARAMS * pcParams,
									 IN struct MSG_GEA_TRANSACTION_III * pTrsItems,
									 OUT char ** pConvertedMsg,
									 OUT DWORD * pdwMsgLength );
PROTECTED DWORD ConvertTipTrs( IN CMW_PARAMS * pcParams,
							   IN CMW_TYP_DATA * pTypData,
							   OUT char ** pConvertedMsg,
							   OUT DWORD * pdwMsgLength );
/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/