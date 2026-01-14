/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_ALAR_REQ.H                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_ALAR_REQ_H
#define MSG_PV_ALAR_REQ_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>

#include <msg_pv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_ACK_REQ_CD 214L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_ACK_REQ_Event
{
	// Alarm Id
	DWORD		plazanum;	// 0-9999
	DWORD		lanenum;	// 0-9999
	DWORD		type;		// 0-999
	DWORD		subtype;	// 0-99
	DWORD		id;			// 0-99
	SYSTEMTIME	appdate;
};

struct MSG_PV_ACK_REQ
{
	// Message Header
	struct MSG_PV_HEADER header;
    
	struct MSG_PV_ACK_REQ_Body
	{
		DWORD	lanenum;		// 0-9999
		DWORD	matricule;		// 0-999 999
	} 
	body;

	// Alarm
	HLIST list_event;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_ACK_REQ * WINAPI MSG_PV_ACK_REQ_New(void);

EXPORT struct MSG_PV_ACK_REQ_Event * WINAPI MSG_PV_ACK_REQ_Event_New(HLIST *hList);

EXPORT struct MSG_PV_ACK_REQ_Event * WINAPI MSG_PV_ACK_REQ_Get_First_Event(HLIST hList);

EXPORT struct MSG_PV_ACK_REQ_Event * WINAPI MSG_PV_ACK_REQ_Get_Next_Event(HLIST hList, 
																		  struct MSG_PV_ACK_REQ_Event *p_ptr);

EXPORT BOOL WINAPI MSG_PV_ACK_REQ_Delete_All(struct MSG_PV_ACK_REQ *p_ptr);

EXPORT BOOL WINAPI MSG_PV_ACK_REQ_Write(struct MSG_PV_ACK_REQ *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_ACK_REQ_Read(struct MSG_PV_ACK_REQ *p_ptr,
									  BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_ACK_REQ_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_ACK_REQ_H */

/*-------------------------------- END OF FILE ------------------------------*/