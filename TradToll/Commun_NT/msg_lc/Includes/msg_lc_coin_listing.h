/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COIN_LISTING.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_COIN_LISTING_H
#define MSG_COIN_LISTING_H

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

#define MSG_COIN_LISTING_CD 27L

/*-------------------------------- TYPEDEFS:  -------------------------------*/


struct MSG_COIN_LISTING_Coin_Information
{
    DWORD	coin_id;		// 0-99
    DWORD	coin_counter;   // 0-99 999 999
};

struct MSG_COIN_LISTING
{
    // Message Header
    struct MSG_HEADER header;

	struct MSG_COIN_LISTING_Body
	{
		DWORD	rejected_coins;		// 0 - 99 999 999
		DWORD	currency_id;		// 0 - 99
		DOUBLE	toll_fare;			// 10 FLOAT
	}
	body;
    
	//Event information
	HLIST	list_coin_info;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_COIN_LISTING * WINAPI MSG_COIN_LISTING_New(void);

EXPORT struct MSG_COIN_LISTING_Coin_Information * WINAPI MSG_COIN_LISTING_Coin_Information_New(HLIST *hList);

EXPORT struct MSG_COIN_LISTING_Coin_Information * WINAPI MSG_COIN_LISTING_Get_First_Coin_Information(HLIST hList);

EXPORT struct MSG_COIN_LISTING_Coin_Information * WINAPI MSG_COIN_LISTING_Get_Next_Coin_Information(HLIST hList, 
																									struct MSG_COIN_LISTING_Coin_Information *p_coin_info);

EXPORT BOOL WINAPI MSG_COIN_LISTING_Delete_All(struct MSG_COIN_LISTING *p_coin_listing);

EXPORT BOOL WINAPI MSG_COIN_LISTING_Write(struct MSG_COIN_LISTING *p_coin_listing, 
										  BYTE *p_msg, 
										  DWORD msg_size_max, 
										  DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_COIN_LISTING_Read(struct MSG_COIN_LISTING *p_coin_listing, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL MSG_COIN_LISTING_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_COIN_LISTING.H */

/*-------------------------------- END OF FILE ------------------------------*/