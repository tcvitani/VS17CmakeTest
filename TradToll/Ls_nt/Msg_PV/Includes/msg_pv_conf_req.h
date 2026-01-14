/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_CONF_REQ.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_CONF_REQ_H
#define MSG_PV_CONF_REQ_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>

#include <msg_pv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_CONF_REQ_CD 208L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_CONF_REQ
{
    // Message Header
    struct MSG_PV_HEADER header;
    
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_CONF_REQ * WINAPI MSG_PV_CONF_REQ_New(void);

EXPORT BOOL WINAPI MSG_PV_CONF_REQ_Delete_All(struct MSG_PV_CONF_REQ *p_ptr);

EXPORT BOOL WINAPI MSG_PV_CONF_REQ_Write(struct MSG_PV_CONF_REQ *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_CONF_REQ_Read(struct MSG_PV_CONF_REQ *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_CONF_REQ_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_CONF_REQ_H */

/*-------------------------------- END OF FILE ------------------------------*/