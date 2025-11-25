/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRACE.H													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_TRACE_H
#define MSG_TRACE_H

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

#define MSG_TRACE_CD 61L

#define MSG_TRACE_INFO_VALUE_LENGTH	4000UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_TRACE_Elem_Info	// Elementary Information Detail
{
    DWORD       code;					// 0-99
	MSG_VARIANT(MSG_TRACE_INFO_VALUE_LENGTH)		info_value;
};

struct MSG_TRACE_Info_Type	// Information Type
{
    DWORD       type;                   // 0-99
    HLIST       list_info_type;			// >= 1
};


struct MSG_TRACE				// Trace
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_TRACE_Body
    {
        SYSTEMTIME  time_of_trace;
    } 
    body;

    // Trace               
    HLIST           list_trace;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_TRACE * WINAPI MSG_TRACE_New(void);

EXPORT struct MSG_TRACE_Info_Type * WINAPI MSG_TRACE_Info_Type_New(HLIST *hList);

EXPORT struct MSG_TRACE_Info_Type * WINAPI MSG_TRACE_Get_First_Info_Type(HLIST hList);

EXPORT struct MSG_TRACE_Info_Type * WINAPI MSG_TRACE_Get_Next_Info_Type(HLIST hList, struct MSG_TRACE_Info_Type *p_type);


EXPORT struct MSG_TRACE_Elem_Info * WINAPI MSG_TRACE_Elem_Info_New(HLIST *hList);

EXPORT struct MSG_TRACE_Elem_Info * WINAPI MSG_TRACE_Get_First_Elem_Info(HLIST hList);

EXPORT struct MSG_TRACE_Elem_Info * WINAPI MSG_TRACE_Get_Next_Elem_Info(HLIST hList, struct MSG_TRACE_Elem_Info *p_elem);


EXPORT BOOL WINAPI MSG_TRACE_Delete_All(struct MSG_TRACE *p_trace);

EXPORT BOOL WINAPI MSG_TRACE_Write(struct MSG_TRACE *p_trace, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_TRACE_Read(struct MSG_TRACE *p_trace, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);
EXPORT BOOL MSG_TRACE_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_TRACE.H */

/*-------------------------------- END OF FILE ------------------------------*/