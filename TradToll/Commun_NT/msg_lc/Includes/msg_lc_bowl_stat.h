/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_BOWL_STAT.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_BOWL_STAT_H
#define MSG_BOWL_STAT_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <msg_lc_header.h>
#include <time.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_BOWL_STAT_CD 32L //message id, L-long

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_BOWL_STAT_Bowl_Information
{
	DWORD       bowl_position;          // 0-9
	DWORD       coin_id;                // 0-99
	DWORD		bowl_capacity;			// 0-9999
	DWORD		remaining_coin_counter; // 0-99 999 999
	DWORD       returned_coin_counter;  // 0-99 999 999
};

struct MSG_BOWL_STAT
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_BOWL_STAT_Body
    {
        SYSTEMTIME  time_of_last_fill;
    } 
    body;

    // Filling Information
    HLIST	list_bowl_informations;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_BOWL_STAT * WINAPI MSG_BOWL_STAT_New(void);

EXPORT struct MSG_BOWL_STAT_Bowl_Information * WINAPI MSG_BOWL_STAT_Bowl_Information_New(HLIST *hList);

EXPORT struct MSG_BOWL_STAT_Bowl_Information * WINAPI MSG_BOWL_STAT_Get_First_Bowl_Information(HLIST hList);

EXPORT struct MSG_BOWL_STAT_Bowl_Information * WINAPI MSG_BOWL_STAT_Get_Next_Bowl_Information(HLIST hList, 
																							 struct MSG_BOWL_STAT_Bowl_Information *p_bowl_inf);

EXPORT BOOL WINAPI MSG_BOWL_STAT_Delete_All(struct MSG_BOWL_STAT *p_bowl_stat);

EXPORT BOOL WINAPI MSG_BOWL_STAT_Write(struct MSG_BOWL_STAT *p_bowl_stat, 
									   BYTE *p_msg, 
									   DWORD msg_size_max, 
									   DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_BOWL_STAT_Read(struct MSG_BOWL_STAT *p_bowl_stat, 
									  BYTE *p_msg, 
									  DWORD msg_size_max, 
									  DWORD *final_msg_size);

EXPORT BOOL MSG_BOWL_STAT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_BOWL_STAT.H */

/*-------------------------------- END OF FILE ------------------------------*/