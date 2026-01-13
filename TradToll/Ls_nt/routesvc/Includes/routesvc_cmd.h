/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_cmd.h
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
#ifndef ROUTESVC_CMD_H
#define ROUTESVC_CMD_H

#include <protect.h>

PROTECTED void CmdReceive(DWORD64 dwCmdIndex, DWORD dwMsgSize, char * pcMsg);


#endif
