/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC.H                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_LC_H
#define MSG_LC_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED void MSG_BACKUP_DATE_Init(void);
PROTECTED void MSG_BACKUP_FILE_Init(void);
PROTECTED void MSG_BOWL_MOVEMENT_Init(void);
PROTECTED void MSG_BOWL_STAT_Init(void);
PROTECTED void MSG_COIN_LISTING_Init(void);
PROTECTED void MSG_COMMAND_Init(void);
PROTECTED void MSG_COMP_INF_TR_Init(void);
PROTECTED void MSG_END_SHIFT_Init(void);
PROTECTED void MSG_EVENT_Init(void);
PROTECTED void MSG_HEADER_Init(void);
PROTECTED void MSG_HOURLY_Init(void);
PROTECTED void MSG_COUNTER_STATUS_Init(void);
PROTECTED void MSG_PARTLY_Init(void);
PROTECTED void MSG_PAYMENT_Init(void);
PROTECTED void MSG_PERM_RQ_SEND_FILE_Init(void);
PROTECTED void MSG_PERM_SEND_FILE_Init(void);
PROTECTED void MSG_REFERENCE_Init(void);
PROTECTED void MSG_START_SHIFT_Init(void);
PROTECTED void MSG_STATUS_Init(void);
PROTECTED void MSG_TAB_ACK_Init(void);
PROTECTED void MSG_TRACE_Init(void);
PROTECTED void MSG_TRANSACTION_Init(void);
PROTECTED void MSG_TRANSACTION_VES_ALTAIR_Init(void);
PROTECTED void MSG_VAULT_INS_Init(void);
PROTECTED void MSG_VAULT_STAT_Init(void);
PROTECTED void MSG_VLT_WITHDRAW_Init(void);

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/