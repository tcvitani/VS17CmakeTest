/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_VLT_WITHDRAW.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_VLT_WITHDRAW_H
#define MSG_VLT_WITHDRAW_H

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

#define MSG_VLT_WITHDRAW_CD 22L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_VLT_WITHDRAW_Coin_Information
{
    DWORD       coin_id;                     // 0-99
    DWORD       coin_counter;             // 0-99 999 999
};

struct MSG_VLT_WITHDRAW_Bowl_Information
{
    DWORD       bowl_pos;                     // 0-9
	DWORD		coin_id;					  //0-99	
    DWORD       coin_change_counter;          // 0-99 999 999
};

struct MSG_VLT_WITHDRAW
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_VLT_WITHDRAW_Body
    {
        SYSTEMTIME  time_of_vault_withdraw;
		SYSTEMTIME	time_of_vault_insertion;
		DWORD		withdrawn_vault_identifier;  //0-999999
    } 
    body;


    // Coin information
    HLIST           list_coin_information;

	//Bowl information
	HLIST			list_bowl_information;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_VLT_WITHDRAW * WINAPI MSG_VLT_WITHDRAW_New(void);

EXPORT struct MSG_VLT_WITHDRAW_Coin_Information * WINAPI MSG_VLT_WITHDRAW_Coin_Information_New(HLIST *hList);

EXPORT struct MSG_VLT_WITHDRAW_Coin_Information * MSG_VLT_WITHDRAW_Get_First_Coin_Information(HLIST hList);

EXPORT struct MSG_VLT_WITHDRAW_Coin_Information * WINAPI MSG_VLT_WITHDRAW_Get_Next_Coin_Information(HLIST hList, struct MSG_VLT_WITHDRAW_Coin_Information *p_coin);


EXPORT struct MSG_VLT_WITHDRAW_Bowl_Information * WINAPI MSG_VLT_WITHDRAW_Bowl_Information_New(HLIST *hList);

EXPORT struct MSG_VLT_WITHDRAW_Bowl_Information * MSG_VLT_WITHDRAW_Get_First_Bowl_Information(HLIST hList);

EXPORT struct MSG_VLT_WITHDRAW_Bowl_Information * WINAPI MSG_VLT_WITHDRAW_Get_Next_Bowl_Information(HLIST hList, struct MSG_VLT_WITHDRAW_Bowl_Information *p_bowl);


EXPORT BOOL WINAPI MSG_VLT_WITHDRAW_Delete_All(struct MSG_VLT_WITHDRAW *p_vlt_withdraw);

EXPORT BOOL WINAPI MSG_VLT_WITHDRAW_Write(struct MSG_VLT_WITHDRAW *p_vlt_withdraw, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_VLT_WITHDRAW_Read(struct MSG_VLT_WITHDRAW *p_vlt_withdraw, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_VLT_WITHDRAW_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_VLT_WITHDRAW.H */

/*-------------------------------- END OF FILE ------------------------------*/