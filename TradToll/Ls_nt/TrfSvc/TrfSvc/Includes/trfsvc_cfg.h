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

#ifndef TRFSVC_CFG_H
#define TRFSVC_CFG_H

#include <protect.h>

PROTECTED DWORD CfgLoad();
PROTECTED void CfgUnload();
PROTECTED DWORD CfgExtensionsStart();
PROTECTED void CfgExtensionsStop();
PROTECTED BOOL CfgExtensionTestStatus( char * szFaulty, DWORD dwBytes );

#endif
