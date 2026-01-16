/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_acom.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef ROUTE_ACOM_H
#define ROUTE_ACOM_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <acom.h>
#include <csr_msg.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define ROUTE_ACOM_MAX_CONNECTIONS 10

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD ROUTE_ACOM_Get_Nb_Com_Server(void);

PROTECTED BOOL ROUTE_ACOM_Start(DWORD dwPVThreadId);

PROTECTED BOOL ROUTE_ACOM_Terminate(void);

PROTECTED BOOL ROUTE_ACOM_Send(ACOM_CNX_HANDLE hCnxHandle, HMSG hMsg);

PROTECTED BOOL ROUTE_ACOM_Send_Buffer(ACOM_CNX_HANDLE hCnxHandle, DWORD dwDataSize, void *pvData);

PROTECTED BOOL ROUTE_ACOM_Disconnect(ACOM_CNX_HANDLE hCnxHandle);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
