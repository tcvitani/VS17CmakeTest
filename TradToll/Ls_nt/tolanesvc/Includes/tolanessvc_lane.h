/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : tolanessvc
 * FILE       : tolanessvc_lane.h
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

#ifndef TOLANESSVC_LANE_H
#define TOLANESSVC_LANE_H

#include <protect.h>

PROTECTED DWORD LaneListCount( TOLANESSVC_LANE * psList );
PROTECTED DWORD LaneListFindLane( TOLANESSVC_LANE * psList, DWORD dwPlaza, DWORD dwLane );
PROTECTED void LaneListMerge( TOLANESSVC_LANE * psList, TOLANESSVC_LANE * psListNew );
PROTECTED DWORD LaneSendSetFilterMsg(void);
PROTECTED DWORD LaneSendQueryCnxMsg(void);
PROTECTED DWORD LaneSendRefMsg( DWORD dwIndex);
PROTECTED DWORD LaneReceiveMsg( DWORD dwMsgSize, void * pvMsg );

#endif
