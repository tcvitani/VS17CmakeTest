/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_STATUS.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_STATUS_H
#define MSG_STATUS_H

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

#define MSG_STATUS_CD 1L

/*-------------------------------- TYPEDEFS:  -------------------------------*/


struct MSG_STATUS_Event_Information
{
    DWORD       event_category;                   // 0-127
    DWORD       event_sub_category;               // 0-99
    DWORD       current_event_status;             // 0-99
};

struct MSG_STATUS
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_STATUS_Body
    {
        SYSTEMTIME  time_of_event;
    } 
    body;


	//Event information
	HLIST           list_event_info;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_STATUS * WINAPI MSG_STATUS_New(void);

EXPORT struct MSG_STATUS_Event_Information * WINAPI MSG_STATUS_Event_Information_New(HLIST *hList);

EXPORT struct MSG_STATUS_Event_Information * WINAPI MSG_STATUS_Get_First_Event_Information(HLIST hList);

EXPORT struct MSG_STATUS_Event_Information * WINAPI MSG_STATUS_Get_Next_Event_Information(HLIST hList, struct MSG_STATUS_Event_Information *p_event_info);

EXPORT BOOL WINAPI MSG_STATUS_Delete_All(struct MSG_STATUS *p_status);

EXPORT BOOL WINAPI MSG_STATUS_Write(struct MSG_STATUS *p_status, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_STATUS_Read(struct MSG_STATUS *p_status, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_STATUS_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_STATUS.H */

/*-------------------------------- END OF FILE ------------------------------*/