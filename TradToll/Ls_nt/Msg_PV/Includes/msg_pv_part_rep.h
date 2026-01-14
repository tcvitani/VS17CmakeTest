/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_PART_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_PART_REP_H
#define MSG_PV_PART_REP_H

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

#define MSG_PV_PART_REP_CD 220L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_PART_REP_Class
{
    DWORD	value;
	DWORD	counter;	// 0-99 999 999
};

struct MSG_PV_PART_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_PART_REP_Body
    {
        DWORD       lanenum;	//9999
		SYSTEMTIME	date;
		DWORD		collector_id; //0-999 999
		DWORD		active_vault; //0-999 999
		SYSTEMTIME	lower_date;
		SYSTEMTIME	upper_date;
		DWORD		total_traffic; //0-99 999
		DWORD		total_violation; //0-99 999
		DWORD		total_maintenance;	//0-99 999
    } 
    body;

    // class detail
    HLIST	list_class;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_PART_REP * WINAPI MSG_PV_PART_REP_New(void);

EXPORT struct MSG_PV_PART_REP_Class * WINAPI MSG_PV_PART_REP_Class_New(HLIST *hList);

EXPORT struct MSG_PV_PART_REP_Class * WINAPI MSG_PV_PART_REP_Get_First_Class(HLIST hList);

EXPORT struct MSG_PV_PART_REP_Class * WINAPI MSG_PV_PART_REP_Get_Next_Class(HLIST hList, 
																			struct MSG_PV_PART_REP_Class *p_ptr);

EXPORT BOOL WINAPI MSG_PV_PART_REP_Delete_All(struct MSG_PV_PART_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_PART_REP_Write(struct MSG_PV_PART_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_PART_REP_Read(struct MSG_PV_PART_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_PART_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_PART_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/