/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WATCHDOGsvc
 * FILE       : WATCHDOGsvc_cmd.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : commande, controle, service
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des commandes de contrôle du service
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#ifndef WATCHDOGSVC_CMD_H
#define WATCHDOGSVC_CMD_H

#include <protect.h>


PROTECTED void WINAPI CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg );


#endif
