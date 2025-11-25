/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COUNTER_STATUS.H										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_COUNTER_STATUS_H
#define MSG_COUNTER_STATUS_H

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

#define MSG_COUNTER_STATUS_CD 53L //message id, L-long

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_COUNTER_STATUS
{
    // Message Header
    struct MSG_HEADER header;
    
	// Message Body
    struct MSG_COUNTER_STATUS_Body
    {
        DWORD       total_traffic;   //0-99999999
        DWORD       total_violation; //0-99999999
		BOOL		reset;			 //0-1
    } 
    body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_COUNTER_STATUS * WINAPI MSG_COUNTER_STATUS_New(void);

EXPORT BOOL WINAPI MSG_COUNTER_STATUS_Delete_All(struct MSG_COUNTER_STATUS *p_perm_rq_send_file);

EXPORT BOOL WINAPI MSG_COUNTER_STATUS_Write(struct MSG_COUNTER_STATUS *p_perm_rq_send_file, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_COUNTER_STATUS_Read(struct MSG_COUNTER_STATUS *p_perm_rq_send_file, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_COUNTER_STATUS_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_COUNTER_STATUS.H */

/*-------------------------------- END OF FILE ------------------------------*/