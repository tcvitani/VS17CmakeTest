/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_PLAZ_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_PLAZ_REP_H
#define MSG_PV_PLAZ_REP_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>

#include <msg_pv_header.h>
#include <msg_pv_lane_rep.h>
#include <msg_pv_alar_rep.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_PLAZ_REP_CD 211L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_PLAZ_REP_Lane
{
	struct MSG_PV_LANE_REP *msg_pv_lane_rep;
};

struct MSG_PV_PLAZ_REP_Alarm
{
	struct MSG_PV_ALAR_REP *msg_pv_alar_rep;
};

struct MSG_PV_PLAZ_REP_Display
{
	DWORD		id;	// 0-99
	DWORD		total1; // 0-99 999 999
	DWORD		total2; // 0-99 999 999
	DWORD		total3; // 0-99 999 999
	DWORD		total4; // 0-99 999 999
	DWORD		total5; // 0-99 999 999
};

struct MSG_PV_PLAZ_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_PLAZ_REP_Body
    {
		DWORD	plazanum;	//0-9999
    } 
    body;
	
	//Lane
	HLIST	list_lane;

	//Alarm
	HLIST	list_alarm;
	
    // Display
    HLIST	list_display;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_PLAZ_REP * WINAPI MSG_PV_PLAZ_REP_New(void);

EXPORT struct MSG_PV_PLAZ_REP_Display * WINAPI MSG_PV_PLAZ_REP_Display_New(HLIST *hList);

EXPORT struct MSG_PV_PLAZ_REP_Display * WINAPI MSG_PV_PLAZ_REP_Get_First_Display(HLIST hList);

EXPORT struct MSG_PV_PLAZ_REP_Display * WINAPI MSG_PV_PLAZ_REP_Get_Next_Display(HLIST hList, 
																				struct MSG_PV_PLAZ_REP_Display *p_ptr);

EXPORT struct MSG_PV_PLAZ_REP_Lane * WINAPI MSG_PV_PLAZ_REP_Lane_New(HLIST *hList);

EXPORT struct MSG_PV_PLAZ_REP_Lane * WINAPI MSG_PV_PLAZ_REP_Get_First_Lane(HLIST hList);

EXPORT struct MSG_PV_PLAZ_REP_Lane * WINAPI MSG_PV_PLAZ_REP_Get_Next_Lane(HLIST hList, 
																		  struct MSG_PV_PLAZ_REP_Lane *p_ptr);

EXPORT struct MSG_PV_PLAZ_REP_Alarm * WINAPI MSG_PV_PLAZ_REP_Alarm_New(HLIST *hList);

EXPORT struct MSG_PV_PLAZ_REP_Alarm * WINAPI MSG_PV_PLAZ_REP_Get_First_Alarm(HLIST hList);

EXPORT struct MSG_PV_PLAZ_REP_Alarm * WINAPI MSG_PV_PLAZ_REP_Get_Next_Alarm(HLIST hList, 
																			struct MSG_PV_PLAZ_REP_Alarm *p_ptr);

EXPORT BOOL WINAPI MSG_PV_PLAZ_REP_Delete_All(struct MSG_PV_PLAZ_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_PLAZ_REP_Write(struct MSG_PV_PLAZ_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_PLAZ_REP_Read(struct MSG_PV_PLAZ_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_PLAZ_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_PLAZ_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/