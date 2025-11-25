/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_BOWL_MOVEMENT.H                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_BOWL_MOVEMENT_H
#define MSG_BOWL_MOVEMENT_H

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

#define MSG_BOWL_MOVEMENT_CD 31L //message id, L-long

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_BOWL_MOVEMENT_Filling_Information
{
	DWORD	bowl_position;		// 0-9
	DWORD	coin_id;			// 0-99
	LONG	sign;				// -1 ou 1
	DWORD	coin_counter;		// 0-99 999 999
};

struct MSG_BOWL_MOVEMENT
{
	// Message Header
	struct MSG_HEADER header;
    
	// Message Body
	struct MSG_BOWL_MOVEMENT_Body
	{
		SYSTEMTIME	time_of_movement;
		DWORD		agent_identifier;	//0-999999
		DWORD		movement_type;		//0-9
	} 
	body;

	// Filling Information
	HLIST	list_fill_informations;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_BOWL_MOVEMENT * WINAPI MSG_BOWL_MOVEMENT_New(void);

EXPORT struct MSG_BOWL_MOVEMENT_Filling_Information * WINAPI MSG_BOWL_MOVEMENT_Filling_Information_New(HLIST *hList);

EXPORT struct MSG_BOWL_MOVEMENT_Filling_Information * WINAPI MSG_BOWL_MOVEMENT_Get_First_Filling_Information(HLIST hList);

EXPORT struct MSG_BOWL_MOVEMENT_Filling_Information * WINAPI MSG_BOWL_MOVEMENT_Get_Next_Filling_Information(HLIST hList, 
																											struct MSG_BOWL_MOVEMENT_Filling_Information *p_movement);

EXPORT BOOL WINAPI MSG_BOWL_MOVEMENT_Delete_All(struct MSG_BOWL_MOVEMENT *p_bowl_movement);

EXPORT BOOL WINAPI MSG_BOWL_MOVEMENT_Write(struct MSG_BOWL_MOVEMENT *p_bowl_movement, 
										   BYTE *p_msg, 
										   DWORD msg_size_max, 
										   DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_BOWL_MOVEMENT_Read(struct MSG_BOWL_MOVEMENT *p_bowl_movement, 
										  BYTE *p_msg, 
										  DWORD msg_size_max, 
										  DWORD *final_msg_size);

EXPORT BOOL MSG_BOWL_MOVEMENT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_BOWL_MOVEMENT.H */

/*-------------------------------- END OF FILE ------------------------------*/