/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DB_WM
 * FILE       : DB_WM.H
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

#ifndef ICO_H
#define ICO_H

// Types standards (DWORD, LONG, BOOL etc...)
#include <windows.h>

#include <protect.h>

// ------------------ FONCTIONS ---------------------

PROTECTED VOID ICO_Init (void);

PROTECTED BOOL ICO_Start (void);
   
PROTECTED BOOL ICO_Stop (DWORD TimeOut);

#endif
