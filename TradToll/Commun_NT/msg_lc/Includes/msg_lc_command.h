/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COMMAND.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_COMMAND_H
#define MSG_COMMAND_H

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

#define MSG_COMMAND_CD 72L

#define MSG_COMMAND_COMP_INFO_LENGTH	999UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef enum
{
    MSG_COMMAND_INSTRUCTIONS = 1,
    MSG_COMMAND_BOWL_STATUS,
    MSG_COMMAND_VAULT_STATUS,
    MSG_COMMAND_LANE_STATUS
} 
MSG_COMMAND_CMD_TYPE;

struct MSG_COMMAND
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_COMMAND_Body
    {
        DWORD	cmd_type;						// 0-99
		DWORD	compl_information_lenght;		// 0-999
        CHAR	compl_information[MSG_COMMAND_COMP_INFO_LENGTH+1];
    } 
    body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_COMMAND * WINAPI MSG_COMMAND_New(void);

EXPORT BOOL WINAPI MSG_COMMAND_Delete_All(struct MSG_COMMAND *p_command);

EXPORT BOOL WINAPI MSG_COMMAND_Write(struct MSG_COMMAND *p_command, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_COMMAND_Read(struct MSG_COMMAND *p_command, 
                                    BYTE *p_msg, 
                                    DWORD msg_size_max, 
                                    DWORD *final_msg_size);

EXPORT BOOL MSG_COMMAND_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_COMMAND.H */

/*-------------------------------- END OF FILE ------------------------------*/