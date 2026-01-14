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

#ifndef TRFSVC_CMD_CALLBACK_H
#define TRFSVC_CMD_CALLBACK_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CmdCallbackShutdown(
 *                     DWORD64 dwWksUsrKey,
 *                     DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Appelé lorsque la com va mal. Cela provoque un arrêt du service.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CmdCallbackShutdown(
		DWORD64 dwWksUsrKey,
        DWORD dwError );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 CALLBACK CmdCallbackConnection(
 *                     DWORD64 dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. 
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CmdCallbackConnection(DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CmdCallbackDisconnection(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se déconnecter.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CmdCallbackDisconnection(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey);



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CmdCallbackReceived(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey,
 *                     DWORD dwMsgSize,
 *                     BYTE * pbMsg )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : On vient de recevoir un message d'un client
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CmdCallbackReceived(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
        DWORD dwDataSize,
        void * pvData );



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CmdCallbackSent(
 *                    DWORD64 dwInstUsrKey,
 *                    DWORD64 dwCnxUsrKey,
 *                    DWORD64 dwMsgUsrKey,
 *                    DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : On vient d'envoyer un message
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CmdCallbackSent(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
        DWORD dwError );


#endif
