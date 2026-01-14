/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
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

#ifndef FIND_TEXT_H
#define FIND_TEXT_H

#include <protect.h>


PROTECTED DWORD WINAPI TextLoadDefinitions( char * szKey, HCOLLECTION * phCol );
PROTECTED void WINAPI TextUnloadDefinitions( HCOLLECTION hCol );
PROTECTED char * WINAPI TextFind( HCOLLECTION hCol, char * szTextKey, char * szAlternative );


#endif
