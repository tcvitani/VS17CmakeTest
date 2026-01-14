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

#include <windows.h>
#include <stdio.h>
#include <acom.h>
#include <col.h>
#include <ntsvc.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>

#define LOC_DEF
#include <trfsvc_cmd_callback.h>
#undef LOC_DEF
#include <trfsvc_cmd.h>

#include <memclass.h>


// --------------- CODE ----------------------




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK CmdCallbackConnection(
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
PROTECTED DWORD64 CALLBACK CmdCallbackConnection(
	DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    NTSVCInfo( "CmdCallbackConnection(0x%08X)", hCnxHandle );
    return hCnxHandle;
}


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
	DWORD64 dwCnxUsrKey)
{
    NTSVCInfo( "CmdCallbackDisconnection(0x%08X)", dwCnxUsrKey );
    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CmdCallbackReceived(
 *                     DWORD dwInstUsrKey,
 *                     DWORD dwCnxUsrKey,
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
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    NTSVCInfo( "CmdCallbackReceived(0x%08X)", dwCnxUsrKey );

    CmdReceive( (ACOM_CNX_HANDLE)dwCnxUsrKey, dwMsgSize, pbMsg );

    return;
}



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
        DWORD dwError )
{
    NTSVCInfo( "CmdCallbackSent(0x%08X)", dwCnxUsrKey );
    // Rien à faire
    return;
}


