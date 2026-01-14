/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_SERV_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_SERV_REP_H
#define MSG_PV_SERV_REP_H

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

#define MSG_PV_SERV_REP_CD 218L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_SERV_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
    // Message Body
    struct MSG_PV_SERV_REP_Body
    {
        CHAR message[MSG_PV_MAX_MESSAGE_SIZE];
    } 
    body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_SERV_REP * WINAPI MSG_PV_SERV_REP_New(void);

EXPORT BOOL WINAPI MSG_PV_SERV_REP_Delete_All(struct MSG_PV_SERV_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_SERV_REP_Write(struct MSG_PV_SERV_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_SERV_REP_Read(struct MSG_PV_SERV_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_SERV_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_SERV_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/