/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : WATCHDOGsvc
 * FILE       : WATCHDOGsvc_callback.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : messagerie, routage
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des évènements de la com.
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
#include <col.h>
#include <acom.h>
#include <ntsvc.h>

#include <WATCHDOGsvc_glob.h>
#include <WATCHDOGsvc_text.h>
#define LOC_DEF
#include <WATCHDOGsvc_callback.h>
#undef LOC_DEF
#include <WATCHDOGsvc_cmdgen.h>
#include <WATCHDOGsvc_cmd.h>

#include <memclass.h>


// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackShutdown(
 *                     DWORD dwWksUsrKey,
 *                     DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Appelé lorsque la com va mal. Cela provoque un arrêt du service.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackShutdown(
        DWORD dwWksUsrKey,
        DWORD dwError )
{
    SVC_ERR( ERROR_INVALID_DATA, "ERR_WKS_SHUTDOWN" );
    RaiseException( 0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK CallbackConnection(
 *                     DWORD dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. 
 * --------------------------------------------------------------------
 */
PROTECTED DWORD CALLBACK CallbackConnection(
        DWORD dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    NTSVCInfo( "CallbackConnection(0x%08X), connexion interface de commande", hCnxHandle );

    return hCnxHandle;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackDisconnection(
 *                     DWORD dwInstUsrKey,
 *                     DWORD dwCnxUsrKey )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se déconnecter.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackDisconnection(
        DWORD dwInstUsrKey,
        DWORD dwCnxUsrKey )
{
    NTSVCInfo( "CallbackDisconnection(0x%08X), déconnexion interface commande", dwCnxUsrKey );

    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackReceived(
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
PROTECTED void CALLBACK CallbackReceived(
        DWORD dwInstUsrKey,
        DWORD dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    NTSVCInfo( "CallbackReceived(0x%08X), réception message commande", dwCnxUsrKey );
    CmdReceive( (ACOM_CNX_HANDLE)dwCnxUsrKey, dwMsgSize, pbMsg );

    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CallbackSent(
 *                    DWORD dwInstUsrKey,
 *                    DWORD dwCnxUsrKey,
 *                    DWORD dwMsgUsrKey,
 *                    DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : On vient d'envoyer un message
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackSent(
        DWORD dwInstUsrKey,
        DWORD dwCnxUsrKey,
        DWORD dwMsgUsrKey,
        DWORD dwError )
{
    NTSVCInfo( "CallbackSent(0x%08X)", dwCnxUsrKey );
    return;
}





