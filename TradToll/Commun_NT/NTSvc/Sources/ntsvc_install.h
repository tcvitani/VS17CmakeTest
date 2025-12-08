/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     NTSVC_INSTALL.H                                                 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef NTSVC_INSTALL_H
#define NTSVC_INSTALL_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD WINAPI NTSVCInstall(IN char *pcFullPath,
									IN char *pcServiceName,
									IN DWORD dwStartType,
									IN char *pcDependencies,
									IN char *pcUser,
									IN char *pcPassword,
									IN BOOL bInteract );

PROTECTED DWORD WINAPI NTSVCRemove(IN char *pcServiceName);

#endif

/*-------------------------------- END OF FILE ------------------------------*/