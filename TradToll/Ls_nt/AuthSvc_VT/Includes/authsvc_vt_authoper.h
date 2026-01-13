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

#ifndef AUTHSVC_AUTHOPER_H
#define AUTHSVC_AUTHOPER_H

#include <protect.h>

PROTECTED void Auth_VT_EnlReq_Oper(struct MSG_LC_AUTH_VT_ENL_REQ *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);
PROTECTED void Auth_VT_ExlReq_Oper(struct MSG_LC_AUTH_VT_EXL_REQ *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);

PROTECTED void Auth_VT_EnlReq_V2_Oper(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);
PROTECTED void Auth_VT_ExlReq_V2_Oper(struct MSG_LC_AUTH_VT_EXL_REQ_V2 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);

PROTECTED void Auth_VT_EnlReq_V3_Oper(struct MSG_LC_AUTH_VT_ENL_REQ_V3 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);
PROTECTED void Auth_VT_ExlReq_V3_Oper(struct MSG_LC_AUTH_VT_EXL_REQ_V3 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle);

#endif
