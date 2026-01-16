/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_convert.h                                         */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKGEAFILE_CONVERT_H
#define CMWORKGEAFILE_CONVERT_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED DWORD ConvertFile( IN CMW_PARAMS * pcParams, IN HANDLE * hFile );
PROTECTED DWORD FreeMessage( IN OUT char ** pcMessage );
PROTECTED DWORD SaveContexts( IN CMW_PARAMS * pcParams );
PROTECTED DWORD LoadContexts( IN CMW_PARAMS * pcParams );
PROTECTED void WorkTrace( IN HANDLE hWork, IN char * szFormat, ... );
PROTECTED void WorkError( IN HANDLE hWork, IN char * szFormat, ... );
/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/