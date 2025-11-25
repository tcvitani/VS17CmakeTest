/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_END_SHIFT.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_END_SHIFT_H
#define MSG_END_SHIFT_H

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

#define MSG_END_SHIFT_CD 12L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_END_SHIFT_Coin_Information
{
    DWORD       coin_id;                     // 0-99
    DWORD       coin_counter;             // 0-99 999 999
};

struct MSG_END_SHIFT_Bowl_Information
{
    DWORD       bowl_position;         // 0-9
    DWORD       coin_id;               // 0-99
    DWORD       coin_change_counter;   // 0-99 999 999
};

struct MSG_END_SHIFT
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_END_SHIFT_Body
    {
        SYSTEMTIME  time_of_end_shift;
		DWORD	close_type;				//0-9
    } 
    body;

    // Start of Shift Reference
    struct MSG_END_SHIFT_Start_Ref
    {
        DWORD       start_shift_msg_id;   //0-99 999
        SYSTEMTIME  start_shift_time;
    } 
    start_ref;

    // End of Shift detail
    HLIST           list_coin_information;

    HLIST           list_bowl_information;   
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_END_SHIFT * WINAPI MSG_END_SHIFT_New(void);

EXPORT struct MSG_END_SHIFT_Coin_Information * WINAPI MSG_END_SHIFT_Coin_Information_New(HLIST *hList);

EXPORT struct MSG_END_SHIFT_Coin_Information * WINAPI MSG_END_SHIFT_Get_First_Coin_Information(HLIST hList);

EXPORT struct MSG_END_SHIFT_Coin_Information * WINAPI MSG_END_SHIFT_Get_Next_Coin_Information(HLIST hList, struct MSG_END_SHIFT_Coin_Information *p_coin);

EXPORT struct MSG_END_SHIFT_Bowl_Information * WINAPI MSG_END_SHIFT_Bowl_Information_New(HLIST *hList);

EXPORT struct MSG_END_SHIFT_Bowl_Information * WINAPI MSG_END_SHIFT_Get_First_Bowl_Information(HLIST hList);

EXPORT struct MSG_END_SHIFT_Bowl_Information * WINAPI MSG_END_SHIFT_Get_Next_Bowl_Information(HLIST hList, struct MSG_END_SHIFT_Bowl_Information *p_bowl);

EXPORT BOOL WINAPI MSG_END_SHIFT_Delete_All(struct MSG_END_SHIFT *p_endshift);

EXPORT BOOL WINAPI MSG_END_SHIFT_Write(struct MSG_END_SHIFT *p_endshift, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_END_SHIFT_Read(struct MSG_END_SHIFT *p_endshift, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_END_SHIFT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_END_SHIFT.H */

/*-------------------------------- END OF FILE ------------------------------*/