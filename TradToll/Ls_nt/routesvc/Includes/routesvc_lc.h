/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_lc.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : voie
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des messages arrivant des voies
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#ifndef ROUTESVC_LC_H
#define ROUTESVC_LC_H

#include <protect.h>


PROTECTED void LCReceive(DWORD64 dwLCIndex, DWORD dwMsgSize, BYTE * pbMsg);
PROTECTED void LCSendLSMessageToLC( DWORD dwPlaza, DWORD dwLane, DWORD dwMsgSize, void * pvMsg );
PROTECTED void LCTraceMsg(BOOL fRecv, DWORD64 dwLCIndex, DWORD dwMsgSize, BYTE * pbMsg);


#endif
