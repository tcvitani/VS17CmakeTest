/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TAB_ACK.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_TAB_ACK_H
#define MSG_TAB_ACK_H

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

#define MSG_TAB_ACK_CD 51L

#define MSG_TAB_ACK_REFERENCE_NAME_LENGTH	256UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_TAB_ACK
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_TAB_ACK_Body
    {
        SYSTEMTIME  time_of_tab_ack;
        DWORD       type;               // 0-99
		CHAR        reference[MSG_TAB_ACK_REFERENCE_NAME_LENGTH+1];
        DWORD       update_indicator;   // 0-9
    } 
    body;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_TAB_ACK * WINAPI MSG_TAB_ACK_New(void);

EXPORT BOOL WINAPI MSG_TAB_ACK_Delete_All(struct MSG_TAB_ACK *p_tab_ack);

EXPORT BOOL WINAPI MSG_TAB_ACK_Write(struct MSG_TAB_ACK *p_tab_ack, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_TAB_ACK_Read(struct MSG_TAB_ACK *p_tab_ack, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_TAB_ACK_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_TAB_ACK.H */

/*-------------------------------- END OF FILE ------------------------------*/