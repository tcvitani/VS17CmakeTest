/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COMP_INF_TR.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_COMP_INF_TR_H
#define MSG_COMP_INF_TR_H

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

#define MSG_COMP_INF_TR_CD 43L

#define MSG_COMP_INF_TR_INFO_VALUE_LENGTH	4000UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_COMP_INF_TR_Elem_Info	// Elementary Information Detail
{
    DWORD												code;		// 0-99
	MSG_VARIANT(MSG_COMP_INF_TR_INFO_VALUE_LENGTH)		info_value;
};

struct MSG_COMP_INF_TR_Info_Type	// Informatin Type
{
	DWORD	type;				// 0-99
	HLIST	list_info_type;		// >= 1
};

// Complementary Information Transaction
struct MSG_COMP_INF_TR
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_COMP_INF_TR_Body
    {
        SYSTEMTIME  time_of_comp_inf;
    } 
    body;

    // Transaction Reference
    struct MSG_COMP_INF_TR_Ref_Trs 
    {
        DWORD       internal_id;   
        SYSTEMTIME  start_time;
    } 
    ref_trs;

    // Complementary Information               
    HLIST	list_comp_inf;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_COMP_INF_TR * WINAPI MSG_COMP_INF_TR_New(void);

EXPORT struct MSG_COMP_INF_TR_Info_Type * WINAPI MSG_COMP_INF_TR_Info_Type_New(HLIST *hList);

EXPORT struct MSG_COMP_INF_TR_Info_Type * WINAPI MSG_COMP_INF_TR_Get_First_Info_Type(HLIST hList);

EXPORT struct MSG_COMP_INF_TR_Info_Type * WINAPI MSG_COMP_INF_TR_Get_Next_Info_Type(HLIST hList, 
																					struct MSG_COMP_INF_TR_Info_Type *p_type);

EXPORT struct MSG_COMP_INF_TR_Elem_Info * WINAPI MSG_COMP_INF_TR_Elem_Info_New(HLIST *hList);

EXPORT struct MSG_COMP_INF_TR_Elem_Info * WINAPI MSG_COMP_INF_TR_Get_First_Elem_Info(HLIST hList);

EXPORT struct MSG_COMP_INF_TR_Elem_Info * WINAPI MSG_COMP_INF_TR_Get_Next_Elem_Info(HLIST hList, 
																					struct MSG_COMP_INF_TR_Elem_Info *p_elem);

EXPORT BOOL WINAPI MSG_COMP_INF_TR_Delete_All(struct MSG_COMP_INF_TR *p_comp_inf_tr);

EXPORT BOOL WINAPI MSG_COMP_INF_TR_Write(struct MSG_COMP_INF_TR *p_comp_inf_tr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_COMP_INF_TR_Read(struct MSG_COMP_INF_TR *p_comp_inf_tr,  
										BYTE *p_msg,
										DWORD msg_size_max,
										DWORD *final_msg_size);

EXPORT BOOL MSG_COMP_INF_TR_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_COMP_INF_TR.H */

/*-------------------------------- END OF FILE ------------------------------*/