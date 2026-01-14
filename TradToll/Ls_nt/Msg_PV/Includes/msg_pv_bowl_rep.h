/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_BOWL_REP.H                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_BOWL_REP_H
#define MSG_PV_BOWL_REP_H

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

#define MSG_PV_BOWL_REP_CD 216L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_BOWL_REP_Bowl
{
	DWORD		position;	// 0-9
	DWORD		capacity; // 0-9999
	CHAR		currency_label[MSG_PV_MAX_CURRENCY_LABEL];
	CHAR		coin_label[MSG_PV_MAX_COIN_LABEL];
	DWORD		remaining_coin_counter; // 0-99 999 999
	DOUBLE		value; // 99999.99
	DWORD		bag;	// 0-99 999 999
	DWORD       returned_coin_counter;  // 0-99 999 999
};

struct MSG_PV_BOWL_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_BOWL_REP_Body
    {
		DWORD		lanenum;	//0-9999
		SYSTEMTIME	date;
		DWORD		collector_id;	// 0-999 999
		DWORD		active_vault;	//0-999 999
    } 
    body;

    // Bowl
    HLIST	list_bowl;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_BOWL_REP * WINAPI MSG_PV_BOWL_REP_New(void);

EXPORT struct MSG_PV_BOWL_REP_Bowl * WINAPI MSG_PV_BOWL_REP_Bowl_New(HLIST *hList);

EXPORT struct MSG_PV_BOWL_REP_Bowl * WINAPI MSG_PV_BOWL_REP_Get_First_Bowl(HLIST hList);

EXPORT struct MSG_PV_BOWL_REP_Bowl * WINAPI MSG_PV_BOWL_REP_Get_Next_Bowl(HLIST hList, 
																		  struct MSG_PV_BOWL_REP_Bowl *p_ptr);

EXPORT BOOL WINAPI MSG_PV_BOWL_REP_Delete_All(struct MSG_PV_BOWL_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_BOWL_REP_Write(struct MSG_PV_BOWL_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_BOWL_REP_Read(struct MSG_PV_BOWL_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_BOWL_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_BOWL_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/