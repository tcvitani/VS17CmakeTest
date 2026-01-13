/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_spy.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des espions de com
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#ifndef ROUTESVC_SPY_H
#define ROUTESVC_SPY_H

#include <protect.h>


PROTECTED DWORD WINAPI SpyCnxAdd(DWORD64 dwCmdIndex, BOOL bState);
PROTECTED DWORD WINAPI SpyAdd(DWORD64 dwCmdIndex, DWORD dwSpyIndex);
PROTECTED BOOL WINAPI SpyCnxIsActive( DWORD dwCmdIndex );
PROTECTED DWORD WINAPI SpyRemove(DWORD64 dwCmdIndex, DWORD dwSpyIndex);
PROTECTED BOOL WINAPI SpyIsActive(DWORD64 dwCmdIndex, DWORD64 dwSpyIndex);
PROTECTED void SpySendMessage(DWORD64 dwIndex, char * pcText, DWORD dwMsgSize, void * pvMsg);
PROTECTED void SpyCnx(DWORD64 dwIndex, BOOL bState);
PROTECTED void SpyCnxLane(DWORD64 dwLCIndex);


#endif
