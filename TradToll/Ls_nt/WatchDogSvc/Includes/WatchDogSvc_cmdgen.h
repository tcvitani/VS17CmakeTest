/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : xxxsvc
 * FILE       : xxx_cmdgen.h
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
#ifndef CMDGEN_H
#define CMDGEN_H

#include <protect.h>

#define CMD_MAX_SIZE 1024
#define CMD_MAX_PARAMS 4


PROTECTED void WINAPI CmdMessage( ACOM_CNX_HANDLE hCnx, char * pcRsp, ... );
PROTECTED BOOL WINAPI CmdCheck( char * pcCmdName, char * pcCmdLine, DWORD dwCmdSize, DWORD dwParams, char ** ppcParams );
PROTECTED char * WINAPI CmdTrim( char * pcStr );


#endif
