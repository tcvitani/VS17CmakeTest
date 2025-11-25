/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_EVENT.H													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_EVENT_H
#define MSG_EVENT_H

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

#define MSG_EVENT_CD 2L
#define MSG_EVENT_ANOMALY_DESCRIPTION_LENGTH 256UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_EVENT_Event_Description
{
	MSG_VARIANT(MSG_EVENT_ANOMALY_DESCRIPTION_LENGTH)		description;
};

struct MSG_EVENT
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_EVENT_Body
    {
        SYSTEMTIME  time_of_event;	
		DWORD		event_category;				// 0-999
		DWORD		event_sub_category;			// 0-99
		DWORD		current_event_status;		// 0-99
		HLIST		list_event_descritpion;		// >= 0 		
    } 
    body;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_EVENT * WINAPI MSG_EVENT_New(void);

EXPORT BOOL WINAPI MSG_EVENT_Delete_All(struct MSG_EVENT *p_event);

EXPORT struct MSG_EVENT_Event_Description * WINAPI MSG_EVENT_Event_Description_New(HLIST *list_anomaly_descritpion);

EXPORT struct MSG_EVENT_Event_Description * WINAPI MSG_EVENT_Get_First_Anomaly_Description(HLIST list_anomaly_descritpion);

EXPORT struct MSG_EVENT_Event_Description * WINAPI MSG_EVENT_Get_Next_Anomaly_Description(HLIST list_anomaly_descritpion, 
																						  struct MSG_EVENT_Event_Description *p_anomaly_descritpion);

EXPORT BOOL WINAPI MSG_EVENT_Write(struct MSG_EVENT *p_event,
									BYTE *p_msg,
									DWORD msg_size_max,
									DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_EVENT_Read(struct MSG_EVENT *p_event,
									BYTE *p_msg, 
									DWORD msg_size_max,
									DWORD *final_msg_size);

EXPORT BOOL MSG_EVENT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_EVENT.H */

/*-------------------------------- END OF FILE ------------------------------*/