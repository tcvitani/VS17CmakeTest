/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_timer.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions internes de gestion des timers.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_TIMER_H
#define ACOM_TIMER_H

#include <protect.h>


PROTECTED BOOL WINAPI AComTimerStartThread();

PROTECTED void WINAPI AComTimerStopThread();

PROTECTED ACOM_TIMER_ID AComTimerAddTimer(
                        DWORD               dwDelay,
                        HANDLE              hCompletion,
                        ACOM_CONNECTION   * psCnx,
                        ACOM_CNX_HANDLE     hCnx,
                        DWORD               dwType );

PROTECTED void AComTimerRemoveTimer( 
                        ACOM_TIMER_ID       dwId );


#endif
/* -------------  FIN DU FICHIER : acom_timer.h ------------- */ 
