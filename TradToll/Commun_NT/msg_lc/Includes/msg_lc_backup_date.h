/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_BACKUP_DATE.H                                            */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_BACKUP_DATE_H
#define MSG_BACKUP_DATE_H

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

#define MSG_BACKUP_DATE_CD 73L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_BACKUP_DATE
{
	// Message Header
	struct MSG_HEADER header;
    
	// Message Body
	struct MSG_BACKUP_DATE_Body
	{
		SYSTEMTIME  begin_date;
		SYSTEMTIME  end_date;
	} 
	body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_BACKUP_DATE * WINAPI MSG_BACKUP_DATE_New(void);

EXPORT BOOL WINAPI MSG_BACKUP_DATE_Delete_All(struct MSG_BACKUP_DATE *p_backup_date);

EXPORT BOOL WINAPI MSG_BACKUP_DATE_Write(struct MSG_BACKUP_DATE *p_backup_date, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_BACKUP_DATE_Read(struct MSG_BACKUP_DATE *p_backup_date, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL MSG_BACKUP_DATE_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_BACKUP_DATE.H */

/*-------------------------------- END OF FILE ------------------------------*/