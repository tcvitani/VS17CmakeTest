/*****************(v) 2015 SANEF ITS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_AUTH.H                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_LC_AUTH_VT_H
#define MSG_LC_AUTH_VT_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_AUTH_VT_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED void MSG_LC_AUTH_VT_ENL_REP_Init(void);
PROTECTED void MSG_LC_AUTH_VT_ENL_REQ_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REP_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REQ_Init(void);
PROTECTED void MSG_LC_AUTH_VT_ENL_REP_V2_Init(void);
PROTECTED void MSG_LC_AUTH_VT_ENL_REQ_V2_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REP_V2_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REQ_V2_Init(void);
PROTECTED void MSG_LC_AUTH_VT_ENL_REP_V3_Init(void);
PROTECTED void MSG_LC_AUTH_VT_ENL_REQ_V3_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REP_V3_Init(void);
PROTECTED void MSG_LC_AUTH_VT_EXL_REQ_V3_Init(void);

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/