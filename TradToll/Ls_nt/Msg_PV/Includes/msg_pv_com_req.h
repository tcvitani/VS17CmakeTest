/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_COM_REQ.H                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_COM_REQ_H
#define MSG_PV_COM_REQ_H

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

#define MSG_PV_COM_REQ_CD 215L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_COM_REQ_Lanenum
{
	DWORD	lanenum;	// 0-9999
};

struct MSG_PV_COM_REQ
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_COM_REQ_Body
    {
		DWORD	id;	// 0-999
		DWORD	cd; // 0-999
		
		DWORD	size_of_command;
		BYTE	command[MSG_PV_MAX_COMMAND_SIZE];
    } 
    body;

    // Alarm
    HLIST	list_lanenum;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_COM_REQ * WINAPI MSG_PV_COM_REQ_New(void);

EXPORT struct MSG_PV_COM_REQ_Lanenum * WINAPI MSG_PV_COM_REQ_Lanenum_New(HLIST *hList);

EXPORT struct MSG_PV_COM_REQ_Lanenum * WINAPI MSG_PV_COM_REQ_Get_First_Lanenum(HLIST hList);

EXPORT struct MSG_PV_COM_REQ_Lanenum * WINAPI MSG_PV_COM_REQ_Get_Next_Lanenum(HLIST hList, 
																			 struct MSG_PV_COM_REQ_Lanenum *p_ptr);

EXPORT BOOL WINAPI MSG_PV_COM_REQ_Delete_All(struct MSG_PV_COM_REQ *p_ptr);

EXPORT BOOL WINAPI MSG_PV_COM_REQ_Write(struct MSG_PV_COM_REQ *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_COM_REQ_Read(struct MSG_PV_COM_REQ *p_ptr, 
									   BYTE *p_msg, 
									   DWORD msg_size_max, 
									   DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_COM_REQ_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_COM_REQ_H */

/*-------------------------------- END OF FILE ------------------------------*/