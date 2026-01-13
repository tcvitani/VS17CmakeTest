/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : tolanessvc
 * FILE       : tolanessvc_callback.c
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
#include <csr_list.h>
#include <col.h>

#include <tolanessvc_glob.h>
#include <tolanessvc_text.h>
#include <tolanessvc_lane.h>
#include <tolanessvc_cmd.h>

#define LOC_DEF
#include <tolanessvc_callback.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackShutdown(
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
PROTECTED void CALLBACK CallbackShutdown(
		DWORD64 dwWksUsrKey,
        DWORD dwError )
{
    SVC_ERR( ERROR_INVALID_DATA, "ERR_WKS_SHUTDOWN" );
    RaiseException( 0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 CALLBACK CallbackConnection(
 *                     DWORD64 dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. dwInstUsrKey désigne
 *             le type de la connexion ( LS/LC/CMD).
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CallbackConnection(
		DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD dwErr = NO_ERROR;

    // Il s'agit d'une connexion avec le service de communication (RouteSvc)
    if ( dwInstUsrKey == TOLANESSVC_PIPE_COM_TYPE )
    {
        NTSVCInfo( "CallbackConnection(), connexion avec le service de communication" );

        // Mémoriser le handle de la connexion
        gsSvcWork.hComCnx = hCnxHandle; 

        // Mettre en place le filtre ne laissant passer que les infos de connexion
        // et de déconnexion
        dwErr = LaneSendSetFilterMsg();
        if ( dwErr != NO_ERROR )
        {
            NTSVCInfo( "CallbackConnection(), erreur=%u, impossible d'établir le filtre", dwErr );
            AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
        }
        else
        {
            // Interroger RouteSvc sur les voies connectées.
            dwErr = LaneSendQueryCnxMsg();
            if ( dwErr != NO_ERROR )
            {
                NTSVCInfo( "CallbackConnection(), erreur %u, impossible d'obtenir la liste des voies connectées", dwErr );
                AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
            }
        }
    }
    else
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
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey)
{
    DWORD dwIndex;

    if ( dwInstUsrKey == TOLANESSVC_PIPE_COM_TYPE )
    {
        // Acces à la liste des voie, d'où section critique
        EnterCriticalSection( &gsSvcWork.sCritical );

        // On force la l'état de toutes les voies à "déconnecté"
        for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
            if ( gsSvcWork.psList[dwIndex].bBusy )
                gsSvcWork.psList[dwIndex].bConnected = FALSE;

        LeaveCriticalSection( &gsSvcWork.sCritical );

        // Reset du handle de la connexion avec le service de communication
        gsSvcWork.hComCnx = (ACOM_CNX_HANDLE)0;
        NTSVCInfo( "CallbackDisconnection(), déconnexion du service de communication" );
    }
    else
        NTSVCInfo( "CallbackDisconnection(), déconnexion d'une interface de commande" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackReceived(
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
PROTECTED void CALLBACK CallbackReceived(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    DWORD dwErr;
    char * pcCmd;

    if ( dwInstUsrKey == TOLANESSVC_PIPE_COM_TYPE )
    {
        NTSVCInfo( "CallbackReceived(), réception d'un message du service de communication" );
        dwErr = LaneReceiveMsg( dwMsgSize, pbMsg );
        if ( dwErr != NO_ERROR )
            NTSVCInfo( "CallbackReceived(), erreur %u, traitement du message impossible", dwErr );
        else
            NTSVCInfo( "CallbackReceived(), message traité avec succés" );
    }
    else
    {
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
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CallbackSent(
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
PROTECTED void CALLBACK CallbackSent(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
		DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    NTSVCInfo( "CallbackSent(), acquittement, code=%u", dwError );
}

