/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : extfilessvc
 * FILE       : extfilessvc_callback.c
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
#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <col.h>
#include <csr_evtlog.h>

#include <extfilessvc_glob.h>
#include <extfilessvc_text.h>
#include <extfilessvc_cmd.h>

#define LOC_DEF
#include <extfilessvc_callback.h>
#undef LOC_DEF

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
    NTSVCSignalEnd();
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK CallbackConnection(
 *                     DWORD dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD CALLBACK CallbackConnection(
        DWORD dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    // Interface de commande, rien de spécial à faire.
    NTSVCInfo( "CallbackConnection(), connexion avec une interface de commande" );
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
    NTSVCInfo( "CallbackDisconnection(), déconnexion d'une interface de commande" );
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
    char * pcCmd;

    NTSVCInfo( "CallbackReceived(), réception d'un message d'une interface de commande" );
    
    // Allouer un bloc suffisament grand pour pouvoir placer un marqueur de fin de chaine
    pcCmd = HeapAlloc( GetProcessHeap(), 0, dwMsgSize + 1 );
    if ( pcCmd != NULL )
    {
        // Recoupier le bloc
        CopyMemory( pcCmd, pbMsg, dwMsgSize );
        // Forcer la présence d'un marqueur de fin de chaines
        pcCmd[dwMsgSize] = '\0';
        // Donner le bloc à manger au traitement des commandes
        CmdReceive( (ACOM_CNX_HANDLE)dwCnxUsrKey, dwMsgSize, pcCmd );
        // Un fois traité, le libérer
        HeapFree( GetProcessHeap(), 0, pcCmd );
    }
    else
    {
        // Erreur d'allocation
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_NEW_CMD" );
    }
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
    NTSVCInfo( "CallbackSent(), acquittement, code=%u", dwError );
}

