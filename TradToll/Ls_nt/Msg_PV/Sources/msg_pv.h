/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV.H                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_H
#define MSG_PV_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include "csr_list.h"

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED void MSG_PV_ACK_REQ_Init(void);
PROTECTED void MSG_PV_ALAR_REP_Init(void);
PROTECTED void MSG_PV_AUTH_REP_Init(void);
PROTECTED void MSG_PV_AUTH_REQ_Init(void);
PROTECTED void MSG_PV_BOWL_REP_Init(void);
PROTECTED void MSG_PV_COM_REQ_Init(void);
PROTECTED void MSG_PV_CONF_REP_Init(void);
PROTECTED void MSG_PV_CONF_REQ_Init(void);
PROTECTED void MSG_PV_DEC_REQ_Init(void);
PROTECTED void MSG_PV_DISK_REP_Init(void);
PROTECTED void MSG_PV_GAL_REQ_Init(void);
PROTECTED void MSG_PV_LANE_REP_Init(void);
PROTECTED void MSG_PV_LOG_REP_Init(void);
PROTECTED void MSG_PV_LOG_REQ_Init(void);
PROTECTED void MSG_PV_PART_REP_Init(void);
PROTECTED void MSG_PV_PLAZ_REP_Init(HLIST msg_pv_record);
PROTECTED void MSG_PV_SERV_REP_Init(void);
PROTECTED void MSG_PV_TRAC_REP_Init(void);
PROTECTED void MSG_PV_USER_REP_Init(void);
PROTECTED void MSG_PV_USER_REQ_Init(void);
PROTECTED void MSG_PV_VAUL_REP_Init(void);

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/