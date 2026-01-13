/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_callback.c
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
#include <col.h>
#include <csr_list.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>
#include <routesvc_struct.h>
#include <routesvc_cmd.h>
#include <routesvc_ls.h>
#include <routesvc_lc.h>
#include <routesvc_spy.h>

#define LOC_DEF
#include <routesvc_callback.h>
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
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. dwInstUsrKey désigne
 *             le type de la connexion ( LS/LC/CMD).
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CallbackConnection(
		DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
	DWORD64 dwIndex;   // Pour l'index de la nouvelle connexion

    if ( ( gsSvcWork.fLicenceIsValid ) || ( dwInstUsrKey == ROUTESVC_PIPE_CMD_TYPE ) )
    {

        // Trouver et allouer un emplacement vide avec le bon type
        dwIndex = StructAllocate( dwInstUsrKey );
        if ( dwIndex == STRUCT_NOT_ALLOCATED )
        {
            // Aucun emplacement libre, c'est pas normal.
            AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
            SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_NEW_CNX" );
        }
        else
        {
            // Un emplacement a pu être alloué
            if      ( ( gsSvcWork.sParmWork.dwTraceLCMsg != 0 ) && ( dwInstUsrKey == ROUTESVC_PIPE_LC_TYPE ) )
                NTSVCInfo( "LC:CNX  - Idx%03u", dwIndex );
            else if ( ( gsSvcWork.sParmWork.dwTraceLSMsg != 0 ) && ( dwInstUsrKey == ROUTESVC_PIPE_LS_TYPE ) )
                NTSVCInfo( "LS:CNX  - Idx%03u", dwIndex );
            else
                NTSVCInfo( "CallbackConnection(), connexion d'un client de type %s, dwIndex=%u, hCnx=0x%08X", ROUTESVC_PIPE_TYPE_TEXT(dwInstUsrKey), dwIndex, hCnxHandle );

            gsSvcWork.psList[dwIndex].hCnx = hCnxHandle;

            SpyCnx( dwIndex, TRUE );
        }
    }
    else
    {
        dwIndex = STRUCT_NOT_ALLOCATED;
        // Aucun emplacement libre, c'est pas normal.
        AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_LICENCE" );
    }

    // Retourner l'index de l'emplacement qui sera donc passé
    // en paramètres pour les prochains callback.
    return dwIndex;
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
	DWORD64 dwIndex = dwCnxUsrKey;   // Index de la connexion consernée

    if      ( ( gsSvcWork.sParmWork.dwTraceLCMsg != 0 ) && ( dwInstUsrKey == ROUTESVC_PIPE_LC_TYPE ) )
        NTSVCInfo( "LC:DCNX - Idx%03u", dwIndex );
    else if ( ( gsSvcWork.sParmWork.dwTraceLSMsg != 0 ) && ( dwInstUsrKey == ROUTESVC_PIPE_LS_TYPE ) )
        NTSVCInfo( "LS:DCNX - Idx%03u", dwIndex );
    else
        NTSVCInfo( "CallbackDisconnection(), déconnexion client de type %s, dwIndex=%u", ROUTESVC_PIPE_TYPE_TEXT(dwInstUsrKey), dwIndex );

    if ( dwIndex != STRUCT_NOT_ALLOCATED )
    {
        SpyCnx( dwCnxUsrKey, FALSE );

        // S'il s'agit d'une voie, l'info de déconnexion doit être
        // propagée coté LS.
        if ( dwInstUsrKey == ROUTESVC_PIPE_LC_TYPE )
        {
             LSSendCnxInfo( dwCnxUsrKey, LS_ALL_SLOTS, LS_ALL_SLOTS, TRUE );
        }

        // S'il s'agit d'un client LS
        else if ( dwInstUsrKey == ROUTESVC_PIPE_LS_TYPE )
        {
            // Rien de spécial à faire
        }

        // S'il s'agit d'un client de contrôle
        else 
        {
            // Rien de spécial à faire
        }
    
        StructFree( dwIndex );
    }
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
	DWORD64 dwIndex = dwCnxUsrKey;   // Index de la connexion consernée
    char * pcCmd;
    
    if ( dwIndex != STRUCT_NOT_ALLOCATED )
    {
        SpySendMessage( dwIndex, "SPYIN", dwMsgSize, pbMsg );

        // S'il s'agit d'une voie, brancher sur le traitement voie
        if ( dwInstUsrKey == ROUTESVC_PIPE_LC_TYPE )
            LCReceive( dwIndex, dwMsgSize, pbMsg );

        // S'il s'agit d'un client LS, brancher sur le traitement LS
        else if ( dwInstUsrKey == ROUTESVC_PIPE_LS_TYPE )
            LSReceive( dwIndex, dwMsgSize, pbMsg );

        // S'il s'agit d'un client commande, brancher sur le traitement commande
        else 
        {
            // Allouer pour pouvoir placer un marqueur de fin de chaine
            pcCmd = HeapAlloc( GetProcessHeap(), 0, dwMsgSize + 1 );
            if ( pcCmd != NULL )
            {
                CopyMemory( pcCmd, pbMsg, dwMsgSize );
                pcCmd[dwMsgSize] = '\0';
                CmdReceive( dwIndex, dwMsgSize, pcCmd );
                HeapFree( GetProcessHeap(), 0, pcCmd );
            }
            else
            {
                AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwIndex].hCnx, FALSE );
                SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_NEW_CMD" );
            }
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
	DWORD64 dwIndex = dwCnxUsrKey;   // Index de la connexion consernée

    // S'il s'agit d'un succès
    if ( dwError == NO_ERROR )
    {
        // Rien de spécial à faire
    }
    else
    {
        // Rien à faire, la connexion va être automatiquement roupue
        NTSVCInfo( "CallbackSent(), erreur %u, impossible d'émettre sur dwIndex=%u", dwError, dwIndex );
    }
}

