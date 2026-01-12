/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV_CON_REQ.H                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_SV_CON_REQ_H
#define MSG_SV_CON_REQ_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <msg_sv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_SV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_SV_CON_REQ_CD 102L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_SV_CON_REQ
{
    // Message Header
    struct MSG_SV_HEADER header;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_SV_CON_REQ * WINAPI MSG_SV_CON_REQ_New(void);

EXPORT BOOL WINAPI MSG_SV_CON_REQ_Delete_All(struct MSG_SV_CON_REQ *p_ptr);

EXPORT BOOL WINAPI MSG_SV_CON_REQ_Write(struct MSG_SV_CON_REQ *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_CON_REQ_Read(struct MSG_SV_CON_REQ *p_ptr, 
									   BYTE *p_msg, 
									   DWORD msg_size_max, 
									   DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_CON_REQ_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_SV_CON_REQ_H */

/*-------------------------------- END OF FILE ------------------------------*/