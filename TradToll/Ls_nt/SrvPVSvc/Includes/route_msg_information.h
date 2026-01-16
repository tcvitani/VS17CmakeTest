/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_msg_information.h										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef ROUTE_MSG_INFORMAITON_H
#define ROUTE_MSG_INFORMAITON_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

// complementary info. types (form complementay info. message)
#define CMPL_INFO_MSG_TYPE_VEHICLE			5	// complementary info. type used for vehicle
#define CMPL_INFO_MSG_TYPE_FARE_INFO		13	// fare information
#define CMPL_INFO_MSG_TYPE_PV				18	// complementary info. type used for PV additional info

// complementary info. codes (form complementay info. message)
#define CMPL_INFO_MSG_CODE_CLS_ON_FLY_STYPE	3
#define	CMPL_INFO_MSG_CODE_CLS_RESET_STYPE	4
#define CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE	5
#define CMPL_INFO_MSG_CODE_CLS_MOTO_A		13
#define CMPL_INFO_MSG_CODE_CLS_MOTO_B		14

#define CMPL_INFO_MSG_CODE_PV_FARE			2
#define CMPL_INFO_MSG_CODE_PV_PAYMENT_TYPE	3
#define CMPL_INFO_MSG_CODE_PV_CLASS			4
#define CMPL_INFO_MSG_CODE_PV_TRAFFIC_LIGHT	5

#define CMPL_INFO_MSG_CODE_FARE_AMOUNT_PAID	3

// mapped codes used on PV application (form complementay info. message)
#define PV_CMPL_INFO_TRS_ID					1
#define PV_CMPL_INFO_PV_FARE				2
#define PV_CMPL_INFO_CLS_ON_FLY_STYPE		3
#define PV_CMPL_INFO_PV_PAYMENT_TYPE		3
#define PV_CMPL_INFO_CLS_RESET_STYPE		4
#define PV_CMPL_INFO_PV_CLASS				4
#define PV_CMPL_INFO_CLS_KEYIN_STYPE		5
#define PV_CMPL_INFO_PV_TRAFFIC_LIGHT		5
#define PV_CMPL_INFO_PV_AMOUNT_PAID			6
#define PV_CMPL_INFO_CLS_MOTO_A				13
#define PV_CMPL_INFO_CLS_MOTO_B				14

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
