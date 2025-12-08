/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CMDL.H                                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMDL_H
#define CMDL_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef CMDL_EXPORTS
	#include <public.h>
#else
	#include <export.h>
#endif

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define CMDL_MAX_PARAM_SIZE 1024

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT char ** WINAPI CMDLOpenArgv(IN char *pcCommand, OUT DWORD *pdwArgc);

EXPORT void WINAPI CMDLCloseArgv(IN char **ppcArgv);

EXPORT BOOL WINAPI CMDLParamAsList(IN char *pcName, 
								   IN char *pcParamVal, 
								   IN char cSeparator, 
								   IN DWORD dwMaxItemSize, 
								   IN OUT DWORD *pdwItems, 
								   OUT ...);

EXPORT BOOL WINAPI CMDLParamAsString(IN char *pcName, 
									 IN char *pcParamVal, 
									 IN DWORD dwMaxValueSize,
									 OUT char *pcValue);

EXPORT BOOL WINAPI CMDLParamAsNumber(IN char *pcName, 
									 IN char *pcParamVal, 
									 OUT LONG *plValue);

EXPORT BOOL WINAPI CMDLParamAsEnum(IN char *pcName, 
								   IN char *pcParamVal, 
								   OUT LONG *plValue, 
								   IN DWORD dwCount, 
								   IN ...);

EXPORT BOOL WINAPI CMDLParamAsBoolean(IN char *pcName, 
									  IN char *pcParamVal, 
									  OUT BOOL *pbValue);

EXPORT BOOL WINAPI CMDLParamAsFlag(IN char *pcName ,
								   IN char *pcParamVal);

#endif

/*-------------------------------- END OF FILE ------------------------------*/