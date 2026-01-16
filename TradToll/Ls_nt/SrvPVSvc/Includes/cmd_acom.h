/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMD_ACOM
 * FILE       : CMD_ACOM.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : communication asynchrone mailslot named pipe
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

#ifndef CMD_ACOM_H
#define CMD_ACOM_H

// Types standards (DWORD, LONG, BOOL etc...)
#include <windows.h>
#include <acom.h>

#include <protect.h>

// ------------------ FONCTIONS ---------------------

PROTECTED BOOL CMD_ACOM_Start (DWORD dwCMDThreadId);
   
PROTECTED BOOL CMD_ACOM_Terminate (void);

PROTECTED BOOL CMD_ACOM_Send_Buffer(DWORD hCnxHandle,
                                     DWORD dwDataSize,
                                     void * pvData);

PROTECTED BOOL CMD_ACOM_Disconnect (ACOM_CNX_HANDLE hCnxHandle);

#endif
