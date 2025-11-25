/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_HEADER.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_HEADER_H
#define MSG_HEADER_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

#include <csr_msg.h>
#include <time.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

#define MSG_LC_MESSAGE_ID	11L

/*-------------------------------- DEFINES:   -------------------------------*/

// Numéro de voie et de gare à utiliser dans les entètes des messages
// à destination des voie pour effectuer un broadcast.
#define MSG_HEADER_LANE_BROADCAST  0
#define MSG_HEADER_PLAZA_BROADCAST 0

#define MSG_HEADER_NB_FIELDS       8

#define MSG_FIELD_HEADER(a, b)\
		  MSG_FIELD_DWORD, MSG_OFFSET(a, header.id), 0UL, 999UL }, \
		{ MSG_FIELD_CONST, MSG_OFFSET(a, header.cd), b, 0UL, 999UL }, \
		{ MSG_FIELD_DWORD, MSG_OFFSET(a, header.plaza_number), 0UL, 9999UL }, \
		{ MSG_FIELD_DWORD, MSG_OFFSET(a, header.lane_number), 0UL, 9999UL }, \
		{ MSG_FIELD_DWORD, MSG_OFFSET(a, header.M_number), 0UL, 99999UL }, \
		{ MSG_FIELD_NOW, MSG_OFFSET(a, header.time_of_message) }, \
		{ MSG_FIELD_DWORD, MSG_OFFSET(a, header.toll_collector_id), 0UL, 999999UL }, \
		{ MSG_FIELD_DWORD, MSG_OFFSET(a, header.active_vault_id), 0UL, 999999UL  \

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_HEADER
{
    DWORD       id;                 // 0-999
    LONG        cd;                 // automatique
    DWORD       plaza_number;       // 0-9 999
    DWORD       lane_number;        // 0-9 999
	DWORD		M_number;			// 0-99 999
    SYSTEMTIME  time_of_message;    // automatique
    DWORD       toll_collector_id;  // 0-999 999
    DWORD       active_vault_id;    // 0-999 999
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_HEADER * WINAPI MSG_HEADER_New(void);

EXPORT BOOL WINAPI MSG_HEADER_Delete_All(struct MSG_HEADER *p_header);

EXPORT BOOL WINAPI MSG_HEADER_Write(struct MSG_HEADER *p_header, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_HEADER_Read(struct MSG_HEADER *p_header, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_HEADER_Compare_CD(BYTE *p_msg, LONG CD);


/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_HEADER.H */

/*-------------------------------- END OF FILE ------------------------------*/