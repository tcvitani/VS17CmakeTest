/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef AUTHSVC_COMMSG_H
#define AUTHSVC_COMMSG_H

#include <protect.h>

PROTECTED void ROUTE_Start (void);
PROTECTED void ROUTE_Stop (void);
PROTECTED BOOL ROUTE_Send_msg_sv_con_req (ACOM_CNX_HANDLE hCnxHandle);
PROTECTED BOOL ROUTE_Send_msg_sv_filt_dec ();
PROTECTED BOOL ROUTE_ACOM_Send (ACOM_CNX_HANDLE hCnxHandle,
                                HMSG hMsg);
PROTECTED BOOL ROUTE_ACOM_Send_Buffer (ACOM_CNX_HANDLE hCnxHandle,
                                       DWORD dwDataSize,
                                       void * pvData );

#endif
