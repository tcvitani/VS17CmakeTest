/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_struct.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Manipulation de la liste des connexion et des structures associées
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#ifndef ROUTESVC_STRUCT_H
#define ROUTESVC_STRUCT_H

#include <protect.h>


#define STRUCT_NOT_ALLOCATED (0xFFFFFFFF)

PROTECTED DWORD WINAPI StructOpenList( DWORD dwLCCount, DWORD dwLSCount, DWORD dwCmdCount );
PROTECTED VOID WINAPI StructCloseList();
PROTECTED DWORD64 WINAPI StructAllocate(DWORD64 dwType);
PROTECTED DWORD64 WINAPI StructFree(DWORD64 dwIndex);
PROTECTED DWORD WINAPI StructSetFilter(DWORD64 dwIndex, HLIST hList);
PROTECTED BOOL WINAPI StructInFilter(DWORD64 dwIndex, DWORD dwID, DWORD dwCD);


#endif
