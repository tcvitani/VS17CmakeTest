/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_lane.h
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

#ifndef TODBSVC_LANE_H
#define TODBSVC_LANE_H

#include <protect.h>

PROTECTED DWORD LaneSendSetFilterMsg(void);
PROTECTED DWORD LaneSendOkToSend( TODBSVC_LANE * psLane );
PROTECTED DWORD LaneSendBackup( TODBSVC_LANE * psLane, DWORD dwFirst, DWORD dwLast );
PROTECTED DWORD LaneSendQueryCnxMsg(void);
PROTECTED DWORD LaneReceiveMsg( DWORD dwMsgSize, void * pvMsg );

#endif
