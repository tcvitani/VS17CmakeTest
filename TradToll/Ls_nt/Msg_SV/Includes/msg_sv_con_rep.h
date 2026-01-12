/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV_CON_REP.H                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_SV_CON_REP_H
#define MSG_SV_CON_REP_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <msg_sv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_SV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_SV_CON_REP_CD 103L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_SV_CON_REP
{
	// Message Header
	struct MSG_SV_HEADER header;

	struct MSG_SV_CON_REP_Body
	{
		DWORD	plazanum;	// 0-9999
		DWORD	lanenum;	// 0-9999
		DWORD	state;		// 0-9
	} 
	body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_SV_CON_REP * WINAPI MSG_SV_CON_REP_New(void);

EXPORT BOOL WINAPI MSG_SV_CON_REP_Delete_All(struct MSG_SV_CON_REP *p_ptr);

EXPORT BOOL WINAPI MSG_SV_CON_REP_Write(struct MSG_SV_CON_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_CON_REP_Read(struct MSG_SV_CON_REP *p_ptr, 
									   BYTE *p_msg, 
									   DWORD msg_size_max, 
									   DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_CON_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_SV_CON_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/