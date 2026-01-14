/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_USER_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_USER_REP_H
#define MSG_PV_USER_REP_H

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

#define MSG_PV_USER_REP_CD 204L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_USER_REP_Lanetype
{
    DWORD	id;		// 0-9
};

struct MSG_PV_USER_REP_Command
{
    DWORD	id;		// 0-99
	CHAR	label[MSG_PV_MAX_COMMAND_LABEL];
	DWORD	mode;   // 0-1
    HLIST	list_lanetype;
};

struct MSG_PV_USER_REP_Function
{
    DWORD	id;		// 0-99
	DWORD	mode;   // 0-1
};

struct MSG_PV_USER_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
    // Payment detail
    HLIST	list_function;

	HLIST	list_command;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_USER_REP * WINAPI MSG_PV_USER_REP_New(void);

EXPORT struct MSG_PV_USER_REP_Function * WINAPI MSG_PV_USER_REP_Function_New(HLIST *hList);

EXPORT struct MSG_PV_USER_REP_Function * WINAPI MSG_PV_USER_REP_Get_First_Function(HLIST hList);

EXPORT struct MSG_PV_USER_REP_Function * WINAPI MSG_PV_USER_REP_Get_Next_Function(HLIST hList, 
																				  struct MSG_PV_USER_REP_Function *p_ptr);

EXPORT struct MSG_PV_USER_REP_Command * WINAPI MSG_PV_USER_REP_Command_New(HLIST *hList);

EXPORT struct MSG_PV_USER_REP_Command * WINAPI MSG_PV_USER_REP_Get_First_Command(HLIST hList);

EXPORT struct MSG_PV_USER_REP_Command * WINAPI MSG_PV_USER_REP_Get_Next_Command(HLIST hList, 
																				struct MSG_PV_USER_REP_Command *p_ptr);

EXPORT struct MSG_PV_USER_REP_Lanetype * WINAPI MSG_PV_USER_REP_Lanetype_New(HLIST *hList);

EXPORT struct MSG_PV_USER_REP_Lanetype * WINAPI MSG_PV_USER_REP_Get_First_Lanetype(HLIST hList);

EXPORT struct MSG_PV_USER_REP_Lanetype * WINAPI MSG_PV_USER_REP_Get_Next_Lanetype(HLIST hList, 
																				  struct MSG_PV_USER_REP_Lanetype *p_ptr);

EXPORT BOOL WINAPI MSG_PV_USER_REP_Delete_All(struct MSG_PV_USER_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_USER_REP_Write(struct MSG_PV_USER_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_USER_REP_Read(struct MSG_PV_USER_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_USER_REP_New_Record (HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_USER_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/