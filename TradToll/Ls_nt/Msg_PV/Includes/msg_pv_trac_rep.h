/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_TRAC_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_TRAC_REP_H
#define MSG_PV_TRAC_REP_H

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

#define MSG_PV_TRAC_REP_CD 221L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_TRAC_REP_Info
{
	DWORD	code;
	CHAR	info[MSG_PV_MAX_INFO_SIZE];
};

struct MSG_PV_TRAC_REP_Type
{
    DWORD	type;
	
	HLIST	list_info;// 0-99 999 999
};

struct MSG_PV_TRAC_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_TRAC_REP_Body
    {
        DWORD		lanenum;	//9999
		SYSTEMTIME	date;
    } 
    body;

    // type detail
    HLIST	list_type;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_TRAC_REP * WINAPI MSG_PV_TRAC_REP_New(void);

EXPORT struct MSG_PV_TRAC_REP_Type * WINAPI MSG_PV_TRAC_REP_Type_New(HLIST *hList);

EXPORT struct MSG_PV_TRAC_REP_Type * WINAPI MSG_PV_TRAC_REP_Get_First_Type(HLIST hList);

EXPORT struct MSG_PV_TRAC_REP_Type * WINAPI MSG_PV_TRAC_REP_Get_Next_Type(HLIST hList, 
																		  struct MSG_PV_TRAC_REP_Type *p_ptr);

EXPORT struct MSG_PV_TRAC_REP_Info * WINAPI MSG_PV_TRAC_REP_Info_New(HLIST *hList);

EXPORT struct MSG_PV_TRAC_REP_Info * WINAPI MSG_PV_TRAC_REP_Get_First_Info(HLIST hList);

EXPORT struct MSG_PV_TRAC_REP_Info * WINAPI MSG_PV_TRAC_REP_Get_Next_Info(HLIST hList, 
																		  struct MSG_PV_TRAC_REP_Info *p_ptr);

EXPORT BOOL WINAPI MSG_PV_TRAC_REP_Delete_All(struct MSG_PV_TRAC_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_TRAC_REP_Write(struct MSG_PV_TRAC_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_TRAC_REP_Read(struct MSG_PV_TRAC_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_TRAC_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_TRAC_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/