/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_DISK_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_DISK_REP_H
#define MSG_PV_DISK_REP_H

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

#define MSG_PV_DISK_REP_CD 219L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_DISK_REP_Coin
{
    CHAR	label[MSG_PV_MAX_COIN_LABEL];
	DWORD	counter;	// 0-99 999 999
};

struct MSG_PV_DISK_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_DISK_REP_Body
    {
        DWORD       lanenum;	//9999
		SYSTEMTIME	date;
		DWORD		collector_id; //0-999 999
		DWORD		active_vault; //0-999 999
		DWORD		rejected_coin_counter; //0-99 999 999
		CHAR		label[MSG_PV_MAX_CURRENCY_LABEL];
		DOUBLE		total_cash;	// 9 999 999.99
		DOUBLE		left_to_pay;	// 9 999 999.99
    } 
    body;

    // vault detail
    HLIST	list_coin;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_DISK_REP * WINAPI MSG_PV_DISK_REP_New(void);

EXPORT struct MSG_PV_DISK_REP_Coin * WINAPI MSG_PV_DISK_REP_Coin_New(HLIST *hList);

EXPORT struct MSG_PV_DISK_REP_Coin * WINAPI MSG_PV_DISK_REP_Get_First_Coin(HLIST hList);

EXPORT struct MSG_PV_DISK_REP_Coin * WINAPI MSG_PV_DISK_REP_Get_Next_Coin(HLIST hList, 
																		  struct MSG_PV_DISK_REP_Coin *p_ptr);

EXPORT BOOL WINAPI MSG_PV_DISK_REP_Delete_All(struct MSG_PV_DISK_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_DISK_REP_Write(struct MSG_PV_DISK_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_DISK_REP_Read(struct MSG_PV_DISK_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_DISK_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_DISK_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/