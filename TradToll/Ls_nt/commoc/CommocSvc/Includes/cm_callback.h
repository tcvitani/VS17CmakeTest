/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_callback.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions callback pour la gestion de la communication
 *              dans le service.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_CALLBACK_H
#define CM_CALLBACK_H

#include <protect.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackShutdown(
 *                     DWORD64 dwWksUsrKey,
 *                     DWORD dwError )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Appelé lorsque la com va mal. Cela provoque un arrêt du service.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackShutdown(
		DWORD64 dwWksUsrKey,
        DWORD dwError );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 CALLBACK CallbackConnection(
 *                     DWORD64 dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. dwInstUsrKey désigne
 *             le type de la connexion ( LS/LC/CMD).
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CallbackConnection(
		DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackDisconnection(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se déconnecter.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackDisconnection(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey);




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackReceived(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey,
 *                     DWORD dwMsgSize,
 *                     BYTE * pbMsg )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : On vient de recevoir un message d'un client
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackReceived(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg );




/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CallbackSent(
 *                    DWORD64 dwInstUsrKey,
 *                    DWORD64 dwCnxUsrKey,
 *                    DWORD64 dwMsgUsrKey,
 *                    DWORD dwError )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : On vient d'envoyer un message
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackSent(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
		DWORD64 dwMsgUsrKey,
        DWORD dwError );


#endif
