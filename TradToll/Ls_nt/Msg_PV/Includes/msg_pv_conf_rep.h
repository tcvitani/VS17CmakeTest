/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_CONF_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_CONF_REP_H
#define MSG_PV_CONF_REP_H

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

#define MSG_PV_CONF_REP_CD 209L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_CONF_REP_Equipment
{
    DWORD	id;		// 0-99
};

struct MSG_PV_CONF_REP_Lane
{
    DWORD	lanenum;	// 0-9999
	CHAR	name[MSG_PV_MAX_LANE_NAME];
	DWORD	type;	//0-9

	HLIST	list_equipment;
};

struct MSG_PV_CONF_REP_Lanenum
{
    DWORD	id;			// 0-9999
};

struct MSG_PV_CONF_REP_Zone
{
    DWORD	id;			// 0-99

	HLIST	list_lanenum;
};

struct MSG_PV_CONF_REP_Display
{
    DWORD	id;			// 0-99

	HLIST	list_zone;
};

struct MSG_PV_CONF_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_CONF_REP_Body
    {
        DWORD	plazanum;	//9999
		CHAR	name[MSG_PV_MAX_PLAZA_NAME];
    } 
    body;

    // lane detail
    HLIST	list_lane;

	// display detail
	HLIST	list_display;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_CONF_REP * WINAPI MSG_PV_CONF_REP_New(void);

EXPORT struct MSG_PV_CONF_REP_Lane * WINAPI MSG_PV_CONF_REP_Lane_New(HLIST *hList);

EXPORT struct MSG_PV_CONF_REP_Lane * WINAPI MSG_PV_CONF_REP_Get_First_Lane(HLIST hList);

EXPORT struct MSG_PV_CONF_REP_Lane * WINAPI MSG_PV_CONF_REP_Get_Next_Lane(HLIST hList, 
																		  struct MSG_PV_CONF_REP_Lane *p_ptr);

EXPORT struct MSG_PV_CONF_REP_Equipment * WINAPI MSG_PV_CONF_REP_Equipment_New(HLIST *hList);

EXPORT struct MSG_PV_CONF_REP_Equipment * WINAPI MSG_PV_CONF_REP_Get_First_Equipment(HLIST hList);

EXPORT struct MSG_PV_CONF_REP_Equipment * WINAPI MSG_PV_CONF_REP_Get_Next_Equipment(HLIST hList, 
																					struct MSG_PV_CONF_REP_Equipment *p_ptr);

EXPORT struct MSG_PV_CONF_REP_Display * WINAPI MSG_PV_CONF_REP_Display_New(HLIST *hList);

EXPORT struct MSG_PV_CONF_REP_Display * WINAPI MSG_PV_CONF_REP_Get_First_Display(HLIST hList);

EXPORT struct MSG_PV_CONF_REP_Display * WINAPI MSG_PV_CONF_REP_Get_Next_Display(HLIST hList, 
																				struct MSG_PV_CONF_REP_Display *p_ptr);

EXPORT struct MSG_PV_CONF_REP_Zone * WINAPI MSG_PV_CONF_REP_Zone_New(HLIST *hList);

EXPORT struct MSG_PV_CONF_REP_Zone * WINAPI MSG_PV_CONF_REP_Get_First_Zone(HLIST hList);

EXPORT struct MSG_PV_CONF_REP_Zone * WINAPI MSG_PV_CONF_REP_Get_Next_Zone(HLIST hList, 
																		  struct MSG_PV_CONF_REP_Zone *p_ptr);

EXPORT struct MSG_PV_CONF_REP_Lanenum * WINAPI MSG_PV_CONF_REP_Lanenum_New(HLIST *hList);

EXPORT struct MSG_PV_CONF_REP_Lanenum * WINAPI MSG_PV_CONF_REP_Get_First_Lanenum(HLIST hList);

EXPORT struct MSG_PV_CONF_REP_Lanenum * WINAPI MSG_PV_CONF_REP_Get_Next_Lanenum(HLIST hList, 
																				struct MSG_PV_CONF_REP_Lanenum *p_ptr);

EXPORT BOOL WINAPI MSG_PV_CONF_REP_Delete_All(struct MSG_PV_CONF_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_CONF_REP_Write(struct MSG_PV_CONF_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_CONF_REP_Read(struct MSG_PV_CONF_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_CONF_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_CONF_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/