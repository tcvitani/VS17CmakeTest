/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SRVCMD_WM
 * FILE       : SRVCMD_WM.H
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

#ifndef CMD_WM_H
#define CMD_WM_H

// Types standards (DWORD, LONG, BOOL etc...)
#include <windows.h>

#include <protect.h>

enum CMD_WM_TYPE 
{
    CMD_WM_DUMMY  = 1,
};

// ------------------ FONCTIONS ---------------------

PROTECTED BOOL CMD_Start (void);
   
PROTECTED BOOL CMD_Stop (DWORD TimeOut);

PROTECTED BOOL CMD_PostMessage (enum CMD_WM_TYPE type, PVOID Param);

#endif
