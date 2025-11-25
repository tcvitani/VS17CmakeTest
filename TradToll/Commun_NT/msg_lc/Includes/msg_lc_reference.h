/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_REFERENCE.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_REFERENCE_H
#define MSG_REFERENCE_H

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

#define MSG_REFERENCE_CD 70L //message id, L-long

#define MSG_REFERENCE_REFERENCE_NAME_LENGTH	999UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_REFERENCE
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_REFERENCE_Body
    {
		CHAR	reference_name[MSG_REFERENCE_REFERENCE_NAME_LENGTH+1];
    } 
    body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_REFERENCE * WINAPI MSG_REFERENCE_New(void);

EXPORT BOOL WINAPI MSG_REFERENCE_Delete_All(struct MSG_REFERENCE *p_reference);

EXPORT BOOL WINAPI MSG_REFERENCE_Write(struct MSG_REFERENCE *p_reference, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_REFERENCE_Read(struct MSG_REFERENCE *p_reference, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_REFERENCE_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_REFERENCE.H */

/*-------------------------------- END OF FILE ------------------------------*/