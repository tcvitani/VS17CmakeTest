/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_ALAR_REP.H                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_ALAR_REP_H
#define MSG_PV_ALAR_REP_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>

#include <msg_pv_define.h>
#include <msg_pv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_ALAR_REP_CD 213L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_ALAR_REP_Event
{
	// Alarm Id
	DWORD		plazanum;	// 0-9999
	DWORD		lanenum;	// 0-9999
	DWORD		type;		// 0-999
	DWORD		subtype;	// 0-99
	DWORD		id;			// 0-99
	SYSTEMTIME	appdate;
	//End of Alarm Id

	SYSTEMTIME	ackdate;
	DWORD		matricule;	// 0-999 999
	DWORD		level;		// 0-9
	DWORD		state;		// 0-9
	CHAR		label[MSG_PV_MAX_ALARM_LABEL];
	CHAR		instruction[MSG_PV_MAX_DETAIL_SIZE];
	CHAR		description[MSG_PV_MAX_ALARM_DESCRIPTION];
};

struct MSG_PV_ALAR_REP
{
	// Message Header
	struct MSG_PV_HEADER header;
    
	struct MSG_PV_ALAR_REP_Body
	{
		DWORD	lanenum;	//0-9999
		DWORD	unacked;	// 0-999
	} 
	body;

	// Alarm detail
	HLIST list_event;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_ALAR_REP * WINAPI MSG_PV_ALAR_REP_New(void);

EXPORT struct MSG_PV_ALAR_REP_Event * WINAPI MSG_PV_ALAR_REP_Event_New(HLIST *hList);

EXPORT struct MSG_PV_ALAR_REP_Event * WINAPI MSG_PV_ALAR_REP_Get_First_Event(HLIST hList);

EXPORT struct MSG_PV_ALAR_REP_Event * WINAPI MSG_PV_ALAR_REP_Get_Next_Event(HLIST hList, 
																			struct MSG_PV_ALAR_REP_Event *p_ptr);

EXPORT BOOL WINAPI MSG_PV_ALAR_REP_Delete_All(struct MSG_PV_ALAR_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_ALAR_REP_Write(struct MSG_PV_ALAR_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_ALAR_REP_Read(struct MSG_PV_ALAR_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_ALAR_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_ALAR_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/