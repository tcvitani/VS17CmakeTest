/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_START_SHIFT.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_START_SHIFT_H
#define MSG_START_SHIFT_H

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

#define MSG_START_SHIFT_CD 11L

#define MSG_START_SHIFT_TOLL_FARE_REF_LENGTH	256UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_START_SHIFT
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_START_SHIFT_Body
    {
        SYSTEMTIME  time_of_start_shift;
		DWORD   start_of_shift_msg_id;
		CHAR    toll_fare[MSG_START_SHIFT_TOLL_FARE_REF_LENGTH+1];
		DWORD	fare_point;
		DWORD   fare_strip;
		DWORD   open_type;
		DWORD   util_mode;
		DWORD   exploit_mode;
    } 
    body;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_START_SHIFT * WINAPI MSG_START_SHIFT_New(void);

EXPORT BOOL WINAPI MSG_START_SHIFT_Delete_All(struct MSG_START_SHIFT *p_startshift);

EXPORT BOOL WINAPI MSG_START_SHIFT_Write(struct MSG_START_SHIFT *p_startshift, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_START_SHIFT_Read(struct MSG_START_SHIFT *p_startshift, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_START_SHIFT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_STARTSHIFT.H */

/*-------------------------------- END OF FILE ------------------------------*/