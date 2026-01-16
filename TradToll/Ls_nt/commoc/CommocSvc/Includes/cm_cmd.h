/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_cmd.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Traitement des commandes de l'interface de commande du
 *              service.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef COMMOCSVC_CMD_H
#define COMMOCSVC_CMD_H

#include <protect.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg )
 * PARAMETERS: hCnx       : Handle de la connexion
 *             dwMsgSize  : Taille du message de commande
 *             pcMsg      : Message de commande
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite une commande reçue
 * --------------------------------------------------------------------
 */
PROTECTED void CmdReceive( ACOM_CNX_HANDLE hCnx, DWORD dwMsgSize, char * pcMsg );




#endif
