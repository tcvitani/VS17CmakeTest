/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_ls.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des messages arrivant des clients du LS
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
#include <csr_msg.h>
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <msg_lc_header.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>
#include <routesvc_struct.h>
#include <routesvc_lc.h>
#include <routesvc_spy.h>

#define LOC_DEF
#include <routesvc_ls.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- CODE ----------------------




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void LSReceive( DWORD64 dwLSIndex, DWORD dwMsgSize, BYTE * pbMsg )
 * PARAMETERS: dwLSIndex : Index de l'emplacement de la connexion de type LS
 *             dwMsgSize : Taille du message reçu
 *             pbMsg     : Pointe sur le buffer du message
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message reçu sur l'interface LS
 * --------------------------------------------------------------------
 */
PROTECTED void LSReceive(DWORD64 dwLSIndex, DWORD dwMsgSize, BYTE * pbMsg)
{
    DWORD dwErr;   // Récupération des codes d'erreur
    DWORD dwHeadSize;
    struct MSG_SV_HEADER * psLSHead;
    struct MSG_HEADER * psLCHead;
    struct MSG_SV_FILT_DEC * psFilter;

    LSTraceMsg( TRUE, dwLSIndex, dwMsgSize, pbMsg );

    // Décoder l'entête par rapport aux messages attendus
    psLSHead = MSG_New_Read_If_Found( gsSvcWork.hInMsgList, pbMsg, dwMsgSize );
    if ( psLSHead != NULL )
    {
        // Le message a un format attendu

        // Si le message est une configuration de filtre
        if ( psLSHead->cd == MSG_SV_FILT_DEC_CD )
        {
            NTSVCInfo( "LSReceive(), message FILT_DEC sur dwLSIndex=%u", dwLSIndex );

            psFilter = (struct MSG_SV_FILT_DEC *)psLSHead;

            // La fonction suivante protège son accès à la structure,
            // inutile de le surprotéger
            dwErr = StructSetFilter( dwLSIndex, psFilter->list_idcd );
            if ( dwErr != NO_ERROR )
            {
                AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, FALSE );
                SVC_ERR( dwErr, "ERR_NEW_FILTER" );
            }

        }

        // Si le message est une demande d'état de connexion
        else if ( psLSHead->cd == MSG_SV_CON_REQ_CD )
        {
            NTSVCInfo( "LSReceive(), message QUERY_CNX sur dwLSIndex=%u", dwLSIndex );
            dwErr = LSSendCnxInfo( LS_ALL_SLOTS, LS_ALL_SLOTS, dwLSIndex, FALSE );
            if ( dwErr != NO_ERROR )
            {
                AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, FALSE );
                SVC_ERR( dwErr, "ERR_SEND_CNX_LIST" );
            }
        }

        // Sinon, c'est une erreur
        else
        {
            SVC_ERR( ERROR_INVALID_DATA, "ERR_UNEXP_LS_MSG" );
        }

        MSG_Delete_All( &psLSHead );
    }
    // Sinon, le message est vraissemblablement à destination des voies
    else
    {
        psLCHead = MSG_HEADER_New();
        if ( psLCHead != NULL )
        {
            if ( MSG_HEADER_Read( psLCHead, pbMsg, dwMsgSize, &dwHeadSize ) )
            {
                LCSendLSMessageToLC( psLCHead->plaza_number, psLCHead->lane_number, dwMsgSize, pbMsg );
            }
            else
            {
                AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, FALSE );
                SVC_ERR( ERROR_INVALID_DATA, "ERR_INVALID_LS_MSG" );
            }
            MSG_Delete_All( &psLCHead );
        }
        else
        {
            AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, FALSE );
            SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_LS_MSG_MEMORY" );
        }
    }
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LSSendCnxInfo( DWORD64 dwLCIndex, DWORD dwLSIndex, BOOL bDisconnected )
 * PARAMETERS: dwLCIndex : Index de l'emplacement de la connexion de type voie. Si LS_ALL_SLOTS,
 *                         un message est envoyé pour chaque voie connectée.
 *             dwLCSubIdx: Sous index de voie. Si LS_ALL_SLOTS, un message est envoyé pour chaque
 *                         voie regroupée dans ce slot.
 *             dwLSIndex : Index de l'emplacement de la connexion de type LS. Si LS_ALL_SLOTS,
 *                         chaque message est envoyé à tous les client LS connectés et demandeurs.
 *             bDisconnected: Etat à signaler (si TRUE, on signal un état déconnecté, si FALSE,
 *                         on signale un état connecté).
 * RETURN    : Code d'erreur Win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie un message de connexion / déconnexion
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LSSendCnxInfo(DWORD64 dwLCIndex, DWORD dwLCSubIdx, DWORD64 dwLSIndex, BOOL bDisconnected)
{
	DWORD64 dwFirstLC;
	DWORD64 dwLastLC;
	DWORD64 dwFirstLS;
	DWORD64 dwLastLS;
    DWORD dwFirstSubLC;
    DWORD dwLastSubLC;
    DWORD dwIndex;
    DWORD dwMsgSize;
    struct MSG_SV_CON_REP * psLSMsg;
    BYTE tbBuffer[100];

    NTSVCInfo( "LSSendCnxInfo(), dwLCIndex=%u, dwLSIndex=%u", dwLCIndex, dwLSIndex );

    psLSMsg = MSG_SV_CON_REP_New();
    if ( psLSMsg == NULL )
    {
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_LS_MSG_MEMORY" );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwFirstLC = ( dwLCIndex == LS_ALL_SLOTS ? gsSvcWork.dwLCStart : dwLCIndex );
    dwLastLC =  ( dwLCIndex == LS_ALL_SLOTS ? gsSvcWork.dwLCEnd   : dwLCIndex );
    dwFirstLS = ( dwLSIndex == LS_ALL_SLOTS ? gsSvcWork.dwLSStart : dwLSIndex );
    dwLastLS =  ( dwLSIndex == LS_ALL_SLOTS ? gsSvcWork.dwLSEnd   : dwLSIndex );

    EnterCriticalSection( &gsSvcWork.sCritical );

    // Pour toutes les connexions LS concernées
    for ( dwLSIndex = dwFirstLS ; dwLSIndex <= dwLastLS ; dwLSIndex ++ )
    {
        // La fonction StructInFilter si la connexion LS n'est pas utilisée
        // on a pas besoin de tester l'état du slot
        if ( StructInFilter( dwLSIndex, gsSvcWork.sParmWork.dwAppMsgId, MSG_SV_CON_REP_CD ) )
        {
            // Pour tous les slots de voie consernés
            for ( dwLCIndex = dwFirstLC ; dwLCIndex <= dwLastLC ; dwLCIndex ++ )
            {
                if ( ( ! gsSvcWork.psList[dwLCIndex].bBusy      ) ||
                     ( ! gsSvcWork.psList[dwLCIndex].uCnx.sLC.fQualified ) ||
                     ( gsSvcWork.psList[dwLCIndex].uCnx.sLC.dwCount == 0 ) )
                    continue;

                dwFirstSubLC = ( dwLCSubIdx == LS_ALL_SLOTS
                                 ? 0
                                 : dwLCSubIdx );
                dwLastSubLC =  ( dwLCSubIdx == LS_ALL_SLOTS 
                                 ? gsSvcWork.psList[dwLCIndex].uCnx.sLC.dwCount - 1
                                 : dwLCSubIdx );

                for ( dwIndex = dwFirstSubLC ; dwIndex <= dwLastSubLC ; dwIndex ++ )
                {
                    // Composer le message
                    psLSMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;
                    psLSMsg->body.plazanum = gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwPlaza[dwIndex];
                    psLSMsg->body.lanenum = gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwLane[dwIndex];
                    psLSMsg->body.state = bDisconnected ? 0 : 1;
                    dwMsgSize = sizeof( tbBuffer );

                    // Le placer dans le buffer
                    if ( MSG_SV_CON_REP_Write( psLSMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
                    {
                        // L'envoyer
                        AComSendMessage( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, 0, dwMsgSize, tbBuffer );
                        LSTraceMsg( FALSE, dwLSIndex, dwMsgSize, tbBuffer );
                        SpySendMessage( dwLSIndex, "SPYOUT", dwMsgSize, tbBuffer );
                    }
                    else
                    {
                        AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, FALSE );
                        SVC_ERR( ERROR_INVALID_DATA, "ERR_LS_MSG_WRITE" );
                    }
                }
            }
        }
    }

    LeaveCriticalSection( &gsSvcWork.sCritical );

    MSG_Delete_All( &psLSMsg );

    return NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LSSendMessageFromLC( DWORD64 dwLCIndex, DWORD dwMsgSize, void * pvMsg )
 * PARAMETERS: dwLCIndex : Index de l'emplacement de la connexion de type voie.
 *             dwMsgSize : Taille du message à envoyer
 *             pvMsg     : Pointe sur le buffer du message
 * RETURN    : Code d'erreur Win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie un message de voie sur les interface LS demandeuses
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LSSendMessageFromLC(DWORD64 dwLCIndex, DWORD dwMsgSize, void * pvMsg)
{
    DWORD dwHeadSize;
    DWORD dwLSIndex;
    struct MSG_HEADER * psLCHead;

    NTSVCInfo( "LSSendMessageFromLC()" );

    // Préparer la structure pour décomposer l'entête
    psLCHead = MSG_HEADER_New();
    if ( psLCHead == NULL )
    {
        MSG_Delete_All( &psLCHead );
        AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLCIndex].hCnx, FALSE );
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_LS_MSG_MEMORY" );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Décoder l'entête
    if ( ! MSG_HEADER_Read( psLCHead, pvMsg, dwMsgSize, &dwHeadSize ) )
    {
        MSG_Delete_All( &psLCHead );
        AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLCIndex].hCnx, FALSE );
        SVC_ERR( ERROR_INVALID_DATA, "ERR_INVALID_LS_MSG" );
        return ERROR_INVALID_DATA;
    }

    // La connexion n'était pas encore identifiée
    if ( ! gsSvcWork.psList[dwLCIndex].uCnx.sLC.fQualified )
    {
        // Effectuer modification
        gsSvcWork.psList[dwLCIndex].uCnx.sLC.fQualified = TRUE;
        gsSvcWork.psList[dwLCIndex].uCnx.sLC.fMultiple = FALSE;
        gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwPlaza[0] = psLCHead->plaza_number;
        gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwLane[0] = psLCHead->lane_number;
        gsSvcWork.psList[dwLCIndex].uCnx.sLC.dwCount = 1;
        SpyCnxLane( dwLCIndex );

        // Envoyer l'info de connexion
        LSSendCnxInfo( dwLCIndex, 0, LS_ALL_SLOTS, FALSE );
        NTSVCInfo( "LSSendMessageFromLC(), identification de dwLCIndex=%u comme PZ%04u-LN%04u", dwLCIndex, psLCHead->plaza_number, psLCHead->lane_number );
    }

    EnterCriticalSection( &gsSvcWork.sCritical );

    for ( dwLSIndex = gsSvcWork.dwLSStart ; dwLSIndex <= gsSvcWork.dwLSEnd ; dwLSIndex ++ )
    {
        if ( StructInFilter( dwLSIndex, psLCHead->id, psLCHead->cd ) )
        {
            // Envoyer le message
            AComSendMessage( gsSvcWork.hWks, gsSvcWork.psList[dwLSIndex].hCnx, 0, dwMsgSize, pvMsg );
            LSTraceMsg( FALSE, dwLSIndex, dwMsgSize, pvMsg );
            SpySendMessage( dwLSIndex, "SPYOUT", dwMsgSize, pvMsg );
        }
    }

    MSG_Delete_All( &psLCHead );

    LeaveCriticalSection( &gsSvcWork.sCritical );

    return NO_ERROR;
}


PROTECTED void LSTraceMsg(BOOL fRecv, DWORD64 dwLSIndex, DWORD dwMsgSize, BYTE * pbMsg)
{
    static char   tcXMap[] = "0123456789ABCDEF";
    DWORD         dwIndex;
    DWORD         dwPos;
    BOOL          fTooLong = FALSE;
    char          szBuf[1024];

    if ( gsSvcWork.sParmWork.dwTraceLSMsg == 0 )
        return;

    if ( dwMsgSize > ( ( sizeof(szBuf) - 2 ) / 3 ) )
    {
        fTooLong = TRUE;
        dwMsgSize = ( sizeof(szBuf) - 5 ) / 3;
    }

    for ( dwIndex = 0, dwPos = 0 ; dwIndex < dwMsgSize ; dwIndex ++ )
    {
        szBuf[dwPos++] = tcXMap[ pbMsg[dwIndex] >> 4 ];
        szBuf[dwPos++] = tcXMap[ pbMsg[dwIndex] & 15 ];
        szBuf[dwPos++] = ' ';
    }

    if ( fTooLong )
    {
        szBuf[dwPos++] = '.';
        szBuf[dwPos++] = '.';
        szBuf[dwPos++] = '.';
    }
        
    szBuf[dwPos++] = 0;

    if ( fRecv )
        NTSVCInfo( "LS:RECV - Idx%03u [ << %s]",
                   dwLSIndex,
                   szBuf );
    else
        NTSVCInfo( "LS:SEND - Idx%03u [ >> %s]",
                   dwLSIndex,
                   szBuf );

}
