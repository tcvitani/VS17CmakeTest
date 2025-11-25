/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_BACKUP_FILE.H                                            */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_BACKUP_FILE_H
#define MSG_BACKUP_FILE_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <msg_lc_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_BACKUP_FILE_CD 74L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_BACKUP_FILE
{
	// Message Header
	struct MSG_HEADER header;
    
	// Message Body
	struct MSG_BACKUP_FILE_Body
	{
		DWORD  begin_file_number;	// 0 - 99999999
		DWORD  end_file_number;		// 0 - 99999999			
	} 
	body;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_BACKUP_FILE * WINAPI MSG_BACKUP_FILE_New(void);

EXPORT BOOL WINAPI MSG_BACKUP_FILE_Delete_All(struct MSG_BACKUP_FILE *p_backup_file);

EXPORT BOOL WINAPI MSG_BACKUP_FILE_Write(struct MSG_BACKUP_FILE *p_backup_file, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_BACKUP_FILE_Read(struct MSG_BACKUP_FILE *p_backup_file, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL MSG_BACKUP_FILE_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_BACKUP_FILE.H */

/*-------------------------------- END OF FILE ------------------------------*/