/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : PV_ACOM
 * FILE       : PV_ACOM.H
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

#ifndef PV_ACOM_H
#define PV_ACOM_H

// Types standards (DWORD, LONG, BOOL etc...)
#include <windows.h>
#include <acom.h>
#include <csr_msg.h>

#include <protect.h>

// ------------------ DEFINES ---------------------

#define PV_ACOM_MAX_CONNECTIONS 20

#define PV_REG_VAL_MAXPVCNX             "MaxPVCnx"
#define PV_REG_VAL_MAXPVCNX_DEFAULT     PV_ACOM_MAX_CONNECTIONS
#define PV_REG_VAL_PVWORKERS            "SrvPVWorkers"
#define PV_REG_VAL_PVWORKERS_DEFAULT    3

// ------------------ FONCTIONS ---------------------

PROTECTED BOOL PV_ACOM_Start (DWORD dwPVThreadId);
   
PROTECTED BOOL PV_ACOM_Terminate (void);

PROTECTED BOOL PV_ACOM_Send (ACOM_CNX_HANDLE hCnxHandle,
                             HMSG hMsg);

PROTECTED BOOL PV_ACOM_Send_Buffer (ACOM_CNX_HANDLE hCnxHandle,
                                    DWORD dwDataSize,
                                    void * pvData);

PROTECTED BOOL PV_ACOM_Disconnect (ACOM_CNX_HANDLE hCnxHandle);

#endif
