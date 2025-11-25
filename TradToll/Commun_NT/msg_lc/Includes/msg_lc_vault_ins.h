/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_VAULT_INS.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_VAULT_INS_H
#define MSG_VAULT_INS_H

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

#define MSG_VAULT_INS_CD 21L

/*-------------------------------- TYPEDEFS:  -------------------------------*/


struct MSG_VAULT_INS
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_VAULT_INS_Body
    {
        SYSTEMTIME  time_of_vault_insertion;	//YYYYMMDDHHMMSS
		DWORD		ins_vault_id;				// 0-999999
		DWORD		vault_type;					// 0-9
		DWORD		vault_pos_lan;				// 0-9
	} 
    body;
 
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_VAULT_INS * WINAPI MSG_VAULT_INS_New(void);


EXPORT BOOL WINAPI MSG_VAULT_INS_Delete_All(struct MSG_VAULT_INS *p_vault_ins);

EXPORT BOOL WINAPI MSG_VAULT_INS_Write(struct MSG_VAULT_INS *p_vault_ins, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_VAULT_INS_Read(struct MSG_VAULT_INS *p_vault_ins, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_VAULT_INS_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_VAULT_INS.H */

/*-------------------------------- END OF FILE ------------------------------*/