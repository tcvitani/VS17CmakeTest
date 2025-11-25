/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_VAULT_STAT.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_VAULT_STAT_H
#define MSG_VAULT_STAT_H

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

#define MSG_VAULT_STAT_CD 23L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

	// Coin information
struct MSG_VAULT_STAT_Coin_Info
	{
		DWORD       coin_id;                // 0-99
		DWORD       coin_counter;			// 0 99 999 999
	};
    
	
	// Vault Information 
struct MSG_VAULT_STAT_Vault_Info
	{
	    SYSTEMTIME  time_of_insert;			//YYYYMMDDHHMMSS
		DWORD		position;				//9
		DWORD		type;					//9
		DWORD       id;                     // 0-999999
		DWORD		fill_percent;			// 0-100
		DWORD		rejected_coins_since_insertion; // 0-99999999
		DWORD		nb_of_activation_since_insertion; // 0-9999
		DWORD		vault_state;					  // 0-9999
		HLIST       list_coin_info;			// >= 1
	};

	


struct MSG_VAULT_STAT
{
    // Message Header
	struct MSG_HEADER header;
    
    // Message Body
    struct MSG_VAULT_STAT_Body
    {
        DWORD		rejected_coin_counter;	// 0-99 999 999
    } 
    body;

   HLIST list_vaults_in_lane;

};


/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_VAULT_STAT * WINAPI MSG_VAULT_STAT_New(void);

EXPORT struct MSG_VAULT_STAT_Vault_Info * WINAPI MSG_VAULT_STAT_Vault_Info_New(HLIST *list_vaults_in_lane);

EXPORT struct MSG_VAULT_STAT_Vault_Info * WINAPI MSG_VAULT_STAT_Get_First_Vault_Info(HLIST hList);

EXPORT struct MSG_VAULT_STAT_Vault_Info * WINAPI MSG_VAULT_STAT_Get_Next_Vault_Info(HLIST hList, struct MSG_VAULT_STAT_Vault_Info *p_vault);


EXPORT struct MSG_VAULT_STAT_Coin_Info * WINAPI MSG_VAULT_STAT_Coin_Info_New(HLIST *hList);

EXPORT struct MSG_VAULT_STAT_Coin_Info * WINAPI MSG_VAULT_STAT_Get_First_Coin_Info(HLIST hList);

EXPORT struct MSG_VAULT_STAT_Coin_Info * WINAPI MSG_VAULT_STAT_Get_Next_Coin_Info(HLIST hList, struct MSG_VAULT_STAT_Coin_Info *p_coin);


EXPORT BOOL WINAPI MSG_VAULT_STAT_Delete_All(struct MSG_VAULT_STAT *p_vault_stat);

EXPORT BOOL WINAPI MSG_VAULT_STAT_Write(struct MSG_VAULT_STAT *p_vault_stat, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_VAULT_STAT_Read(struct MSG_VAULT_STAT *p_vault_stat, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_VAULT_STAT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_VAULT_STAT.H */

/*-------------------------------- END OF FILE ------------------------------*/