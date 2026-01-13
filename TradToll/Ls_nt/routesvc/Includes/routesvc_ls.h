/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_ls.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des messages arrivant des clients du LS
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#ifndef ROUTESVC_LS_H
#define ROUTESVC_LS_H

#include <protect.h>


#define LS_ALL_SLOTS (0xFFFFFFFF)


PROTECTED void LSReceive(DWORD64 dwLSIndex, DWORD dwMsgSize, BYTE * pbMsg);
PROTECTED DWORD LSSendCnxInfo(DWORD64 dwLCIndex, DWORD dwLCSubIdx, DWORD64 dwLSIndex, BOOL bDisconnected);
PROTECTED DWORD LSSendMessageFromLC(DWORD64 dwLCIndex, DWORD dwMsgSize, void * pvMsg);
PROTECTED void LSTraceMsg(BOOL fRecv, DWORD64 dwLSIndex, DWORD dwMsgSize, BYTE * pbMsg);


#endif
