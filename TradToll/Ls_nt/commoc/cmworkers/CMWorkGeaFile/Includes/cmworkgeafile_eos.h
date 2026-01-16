/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_eos.h                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKGEAFILE_EOS_H
#define CMWORKGEAFILE_EOS_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED DWORD ConvertEOS ( IN CMW_PARAMS * pcParams,
							 IN char * pMessage,
							 OUT char ** pConvertedMsg,
							 OUT DWORD * pdwMsgLength );
PROTECTED DWORD ConvertEOSII ( IN CMW_PARAMS * pcParams,
							   IN char * pMessage,
							   OUT char ** pConvertedMsg,
							   OUT DWORD * pdwMsgLength );
/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/