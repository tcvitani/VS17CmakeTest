/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_VAUL_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_VAUL_REP_H
#define MSG_PV_VAUL_REP_H

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

#define MSG_PV_VAUL_REP_CD 217L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_VAUL_REP_Coin
{
    CHAR	label[MSG_PV_MAX_COIN_LABEL];
	DWORD	counter;	// 0-99 999 999
};

struct MSG_PV_VAUL_REP_Currency
{
    CHAR	label[MSG_PV_MAX_CURRENCY_LABEL];
	DOUBLE	value;	// 99 999 999.99

	HLIST	list_coin;
};

struct MSG_PV_VAUL_REP_Vault
{
	DWORD		position;	// 0-9
	DWORD		percent;    // 0-999
	DWORD		id;			// 0-999999
	DWORD		state;		// 0-99
	DWORD		type;		// 0-9
	DWORD		rejected;	// 0-99 999 999
	DWORD		activated;  // 0-99
	SYSTEMTIME	date;
	
	HLIST		list_currency;
};


struct MSG_PV_VAUL_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_VAUL_REP_Body
    {
        DWORD       lanenum;	//9999
		SYSTEMTIME	date;
		DWORD		collector_id; //0-999 999
		DWORD		active_vault; //0-999 999
		DWORD		rejected_coin_counter; //0-99 999 999
    } 
    body;

    // vault detail
    HLIST list_vault;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_VAUL_REP * WINAPI MSG_PV_VAUL_REP_New(void);

EXPORT struct MSG_PV_VAUL_REP_Vault * WINAPI MSG_PV_VAUL_REP_Vault_New(HLIST *hList);

EXPORT struct MSG_PV_VAUL_REP_Vault * WINAPI MSG_PV_VAUL_REP_Get_First_Vault(HLIST hList);

EXPORT struct MSG_PV_VAUL_REP_Vault * WINAPI MSG_PV_VAUL_REP_Get_Next_Vault(HLIST hList, 
																			struct MSG_PV_VAUL_REP_Vault *p_ptr);

EXPORT struct MSG_PV_VAUL_REP_Currency * WINAPI MSG_PV_VAUL_REP_Currency_New(HLIST *hList);

EXPORT struct MSG_PV_VAUL_REP_Currency * WINAPI MSG_PV_VAUL_REP_Get_First_Currency(HLIST hList);

EXPORT struct MSG_PV_VAUL_REP_Currency * WINAPI MSG_PV_VAUL_REP_Get_Next_Currency(HLIST hList, 
																				  struct MSG_PV_VAUL_REP_Currency *p_ptr);

EXPORT struct MSG_PV_VAUL_REP_Coin * WINAPI MSG_PV_VAUL_REP_Coin_New(HLIST *hList);

EXPORT struct MSG_PV_VAUL_REP_Coin * WINAPI MSG_PV_VAUL_REP_Get_First_Coin(HLIST hList);

EXPORT struct MSG_PV_VAUL_REP_Coin * WINAPI MSG_PV_VAUL_REP_Get_Next_Coin(HLIST hList, 
																		  struct MSG_PV_VAUL_REP_Coin *p_ptr);

EXPORT BOOL WINAPI MSG_PV_VAUL_REP_Delete_All(struct MSG_PV_VAUL_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_VAUL_REP_Write(struct MSG_PV_VAUL_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_VAUL_REP_Read(struct MSG_PV_VAUL_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_VAUL_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_VAUL_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/