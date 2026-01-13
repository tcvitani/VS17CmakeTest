/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_callback.c
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
#include <csr_list.h>
#include <trc.h>

#include <todbsvc_glob.h>
#include <todbsvc_text.h>
#include <todbsvc_lane.h>
#include <todbsvc_cmd.h>

#define LOC_DEF
#include <todbsvc_callback.h>
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
    if ( dwInstUsrKey == TODBSVC_PIPE_COM_TYPE )
    {
        NTSVCInfo( "CallbackConnection(), connexion avec le service de communication" );

        // Mémoriser le handle de la connexion
        gsSvcWork.hComCnx = hCnxHandle; 

        // Mettre en place le filtre ne laissant passer que les infos de connexion,
        // de déconnexion et de demande d'autorisation d'émission
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
PROTECTED void CALLBACK CallbackDisconnection(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey)
{
    HCOLLECTIONITEM hItem;
    TODBSVC_LANE * psLane;

    if ( dwInstUsrKey == TODBSVC_PIPE_COM_TYPE )
    {
        // Acces à la liste des voie, d'où section critique
        ColLock( gsSvcWork.hLanes );

        // On force la l'état de toutes les voies à "déconnecté"
        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hLanes, &hItem ) )
        {
            psLane = ColItemData( hItem );
            psLane->bConnected = FALSE;
        }

        ColUnlock( gsSvcWork.hLanes );

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

    if ( dwInstUsrKey == TODBSVC_PIPE_COM_TYPE )
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
        
        // Donner le bloc à manger au traitement des commandes
        CmdReceive( (ACOM_CNX_HANDLE)dwCnxUsrKey, dwMsgSize, pbMsg );
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
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
		DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    NTSVCInfo( "CallbackSent(), acquittement, code=%u", dwError );
}

