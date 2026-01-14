/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_DEFINE.H                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_MAX_ALARM_LABEL			50UL + 1UL
#define MSG_PV_MAX_USER_NAME			50UL + 1UL
#define MSG_PV_MAX_USER_FIRSTNAME		50UL + 1UL
#define MSG_PV_MAX_USER_PROFILE_NAME	50UL + 1UL
#define MSG_PV_MAX_USER_PASSWORD		8UL + 1UL
#define MSG_PV_MAX_USER_REPLY			1UL + 1UL
#define MSG_PV_MAX_CURRENCY_LABEL		50UL + 1UL
#define MSG_PV_MAX_COIN_LABEL			50UL + 1UL
#define MSG_PV_MAX_COMMAND_SIZE			200UL + 1UL
#define MSG_PV_MAX_COMMAND_LABEL		50UL + 1UL
#define MSG_PV_MAX_LANE_NAME			50UL + 1UL
#define MSG_PV_MAX_PLAZA_NAME			50UL + 1UL
#define MSG_PV_MAX_ANOMALY_LABEL		50UL + 1UL
#define MSG_PV_MAX_OTHER_LABEL			50UL + 1UL
#define MSG_PV_MAX_PAYMENT_LABEL		4UL + 1UL
#define MSG_PV_MAX_CLASS_LABEL			6UL + 1UL
#define MSG_PV_MAX_MESSAGE_SIZE			100UL + 1UL
#define MSG_PV_MAX_DETAIL_SIZE			99UL + 1UL
#define MSG_PV_MAX_INFO_SIZE			4000UL + 1UL
#define MSG_PV_MAX_ALARM_DESCRIPTION	256UL + 1UL
#define MSG_PV_MAX_LANE_LIC_PLATE_LEN	15UL + 1UL

#endif /* MSG_DEFINE_H */

/*-------------------------------- END OF FILE ------------------------------*/