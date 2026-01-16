/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile_file.h                                            */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKGEAFILE_FILE_H
#define CMWORKGEAFILE_FILE_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED DWORD GEAReadNextMessage( IN CMW_PARAMS * pcParams,
									IN HANDLE * hFile,
									OUT char ** pcMessage );
PROTECTED DWORD GEAAppendMessage( IN HANDLE * hFile, IN char * szBuffer );
PROTECTED DWORD AppendMessage( IN HANDLE * hFile,
							   IN char * szBuffer,
							   IN DWORD dwLength );
PROTECTED DWORD MoveDirContent( IN char * szSourceDir, IN char * szDestDir );
PROTECTED DWORD DeleteDirContent( IN char * szDir );
/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/