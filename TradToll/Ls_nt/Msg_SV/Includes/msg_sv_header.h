/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV_HEADER.H                                                 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_SV_HEADER_H
#define MSG_SV_HEADER_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_SV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_SV_HEADER_NB_FIELDS 202

#define MSG_FIELD_SV_HEADER(a, b)\
          MSG_FIELD_DWORD,	MSG_OFFSET(a, header.id),	0UL,	999UL },\
        { MSG_FIELD_CONST,	MSG_OFFSET(a, header.cd),	b,		0UL,	255UL \

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_SV_HEADER
{
	DWORD	id;		// 0-999
	LONG	cd;		// automatique
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT BOOL WINAPI MSG_SV_HEADER_Compare_CD(BYTE *p_msg, LONG CD);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_SV_HEADER_H */

/*-------------------------------- END OF FILE ------------------------------*/