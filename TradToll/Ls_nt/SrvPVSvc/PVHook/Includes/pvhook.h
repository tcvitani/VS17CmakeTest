/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     pvhook.h														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef PVHOOK_H
#define PVHOOK_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef LOC_DEF
	#include <public.h>
#else
	#include <export.h>
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef enum
{
	Pvhook_Ok,
	Pvhook_NoName,
	Pvhook_NoAcces,
	Pvhook_GrosBug,
	Pvhook_OpenLimit,
	Pvhook_CannotOpenReg,
	Pvhook_CannotReadReg,
	Pvhook_CannotWriteReg,
	Pvhook_WriteDisabled,
	Pvhook_CannotDelete,
}
PvhookError;

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT PvhookError WINAPI Pvhook_DataWrite(DWORD size, BYTE *data);

EXPORT PvhookError WINAPI Pvhook_NTSVC_Inherit_Handle(void *pvContext);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
