/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_PERM_SEND_FILE.H										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PERM_SEND_FILE_H
#define MSG_PERM_SEND_FILE_H

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

#define MSG_PERM_SEND_FILE_CD 71L //message id, L-long

/*-------------------------------- TYPEDEFS:  -------------------------------*/


struct MSG_PERM_SEND_FILE
{
    // Message Header
    struct MSG_HEADER header;
    
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PERM_SEND_FILE * WINAPI MSG_PERM_SEND_FILE_New(void);

EXPORT BOOL WINAPI MSG_PERM_SEND_FILE_Delete_All(struct MSG_PERM_SEND_FILE *p_perm_send_file);

EXPORT BOOL WINAPI MSG_PERM_SEND_FILE_Write(struct MSG_PERM_SEND_FILE *p_perm_send_file, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PERM_SEND_FILE_Read(struct MSG_PERM_SEND_FILE *p_perm_send_file, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_PERM_SEND_FILE_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_PERM_SEND_FILE.H */

/*-------------------------------- END OF FILE ------------------------------*/