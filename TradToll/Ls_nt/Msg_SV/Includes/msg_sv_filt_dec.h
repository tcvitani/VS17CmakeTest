/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV_FILT_DEC.H                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_SV_FILT_DEC_H
#define MSG_SV_FILT_DEC_H

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

#define MSG_SV_FILT_DEC_CD		101L

#define MSG_SV_FILT_DEC_ID_ANY	999L
#define MSG_SV_FILT_DEC_CD_ANY	999L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_SV_FILT_DEC_Idcd
{
	// Alarm Id
	DWORD	id;	// 0-999
	DWORD	cd;	// 0-999
};

struct MSG_SV_FILT_DEC
{
    // Message Header
	struct MSG_SV_HEADER header;

	// Alarm
	HLIST	list_idcd;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_SV_FILT_DEC * WINAPI MSG_SV_FILT_DEC_New(void);

EXPORT struct MSG_SV_FILT_DEC_Idcd * WINAPI MSG_SV_FILT_DEC_Idcd_New(HLIST *hList);

EXPORT struct MSG_SV_FILT_DEC_Idcd * WINAPI MSG_SV_FILT_DEC_Get_First_Idcd(HLIST hList);

EXPORT struct MSG_SV_FILT_DEC_Idcd * WINAPI MSG_SV_FILT_DEC_Get_Next_Idcd(HLIST hList, 
																		  struct MSG_SV_FILT_DEC_Idcd *p_ptr);

EXPORT BOOL WINAPI MSG_SV_FILT_DEC_Delete_All(struct MSG_SV_FILT_DEC *p_ptr);

EXPORT BOOL WINAPI MSG_SV_FILT_DEC_Write(struct MSG_SV_FILT_DEC *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_FILT_DEC_Read(struct MSG_SV_FILT_DEC *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_SV_FILT_DEC_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_SV_FILT_DEC_H */

/*-------------------------------- END OF FILE ------------------------------*/