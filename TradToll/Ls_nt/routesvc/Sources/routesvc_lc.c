/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_lc.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : voie
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des messages arrivant des voies
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
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <msg_sv_header.h>
#include <msg_lc_header.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>
#include <routesvc_struct.h>
#include <routesvc_spy.h>
#include <routesvc_ls.h>

#define LOC_DEF
#include <routesvc_lc.h>
#undef LOC_DEF

#include <memclass.h>

// --------------- CODE ----------------------




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void LCReceive( DWORD64 dwLCIndex, DWORD dwMsgSize, BYTE * pbMsg )
 * PARAMETERS: dwLCIndex : Index du slot de voie où le message a été recu
 *             dwMsgSize : Taille du message reçu
 *             pbMsg     : Buffer contenant le message reçu
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message reçu sur l'interface voie
 * --------------------------------------------------------------------
 */
PROTECTED void LCReceive(DWORD64 dwLCIndex, DWORD dwMsgSize, BYTE * pbMsg)
{
    DWORD dwErr = NO_ERROR;
    BOOL  fCnx = FALSE;
    BOOL  fFirst;
    DWORD dwConRepSize;
    DWORD dwLCSubIndex;
    DWORD dwIndex;
    struct MSG_SV_CON_REP * psConRep = NULL;
    ROUTESVC_LC * psLC;

    LCTraceMsg( TRUE, dwLCIndex, dwMsgSize, pbMsg );

    psLC = &gsSvcWork.psList[dwLCIndex].uCnx.sLC;

    __try
    {
        if ( psLC->fQualified && ! psLC->fMultiple )
            __leave;

        // La connexion n'est pas encore identifiée, il peut s'agir d'une connexion
        // multiple. On prépare l'analyse du message de connexion.
        psConRep = MSG_SV_CON_REP_New();
        if ( psConRep == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            SVC_ERR( dwErr, "ERR_LS_MSG_MEMORY" );
            __leave;
        }

        // Décoder le message complet
        if ( ! MSG_SV_CON_REP_Read( psConRep, pbMsg, dwMsgSize, &dwConRepSize ) )
        {
            // Ce n'est pas un message de connexion/déconnexion, il passe dans le 
            // traitement classique.
            __leave;
        }

        fCnx = TRUE;

        // Il faut maintenant rechercher si la voie concernée est déjà connectée
        dwLCSubIndex = 0xFFFFFFFF;
        for ( dwIndex = 0 ; dwIndex < psLC->dwCount ; dwIndex ++ )
        {
            if ( ( psLC->tdwPlaza[dwIndex] == psConRep->body.plazanum ) &&
                 ( psLC->tdwLane[dwIndex] == psConRep->body.lanenum  ) )
            {
                dwLCSubIndex = dwIndex;
                break;
            }
        }

        if ( psConRep->body.state != 0 )
        {
            // Il s'agit d'un message de connexion
            if ( dwLCSubIndex == 0xFFFFFFFF )
            {
                // La voie n'est pas déjà connectée
                if ( psLC->dwCount < MAX_CONCENTRATED_LANES )
                {
                    dwLCSubIndex = psLC->dwCount;
                    fFirst = ! psLC->fQualified;
                    psLC->tdwPlaza[dwLCSubIndex] = psConRep->body.plazanum;
                    psLC->tdwLane[dwLCSubIndex] = psConRep->body.lanenum;
                    psLC->fMultiple = TRUE;
                    psLC->fQualified = TRUE;
                    psLC->dwCount ++;
                    if ( fFirst ) SpyCnxLane( dwLCIndex );
                    LSSendCnxInfo( dwLCIndex, dwLCSubIndex, LS_ALL_SLOTS, FALSE );
                }
                else
                {
                    // Nombre max atteint.
                    dwErr = ERROR_BUFFER_OVERFLOW;
                    SVC_ERR( dwErr, "ERR_LS_MSG_OVERFLOW" );
                    __leave;
                }
            }
        }
        else
        {
            // Il s'agit d'un message de déconnexion
            if ( dwLCSubIndex != 0xFFFFFFFF )
            {
                LSSendCnxInfo( dwLCIndex, dwLCSubIndex, LS_ALL_SLOTS, TRUE );
                // La dernière voie dans la liste vient remplacer le trou
                if ( psLC->dwCount > 1 )
                {
                    psLC->tdwPlaza[dwLCSubIndex] = psLC->tdwPlaza[psLC->dwCount-1];
                    psLC->tdwLane[dwLCSubIndex] = psLC->tdwLane[psLC->dwCount-1];
                }
                psLC->dwCount --;
            }
            else
            {
                // La voie n'était pas enregistrée, rien à faire
                __leave;
            }
        }
    }
    __finally
    {
        if ( psConRep != NULL )
            MSG_Delete_All( &psConRep );
        if ( dwErr != NO_ERROR )
            AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.psList[dwLCIndex].hCnx, FALSE );
    }

    if ( ( dwErr == NO_ERROR ) && ! fCnx )
    {
        // Router le message vers les applications clientes LS
        dwErr = LSSendMessageFromLC( dwLCIndex, dwMsgSize, pbMsg );
        if ( dwErr != NO_ERROR  )
        {
            SVC_ERR( dwErr, "ERR_ROUTE_LC_MSG" );
        }
    }
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void LCSendLSMessageToLC( DWORD dwPlaza, DWORD dwLane, DWORD dwMsgSize, void * pvMsg )
 * PARAMETERS: dwPlaza   : Numéro de gare destinatrice
 *             dwLane    : Numéro de voie destinatrice
 *             dwMsgSize : Taille du message à envoyer
 *             pvMsg     : Buffer contenant le message à envoyer
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoie à une ou des voies un message provenant de l'interface LS
 * --------------------------------------------------------------------
 */
PROTECTED void LCSendLSMessageToLC( DWORD dwPlaza, DWORD dwLane, DWORD dwMsgSize, void * pvMsg )
{
    DWORD dwLCIndex;        // Pour scanner les connexions LC
    DWORD dwLCSubIndex;        // Pour scanner les connexions LC
    ROUTESVC_LC * psLC;

    EnterCriticalSection( &gsSvcWork.sCritical );

    for ( dwLCIndex = gsSvcWork.dwLCStart ; dwLCIndex <= gsSvcWork.dwLCEnd ; dwLCIndex ++ )
    {
        if ( ( gsSvcWork.psList[dwLCIndex].bBusy ) &&
             ( gsSvcWork.psList[dwLCIndex].dwType == ROUTESVC_PIPE_LC_TYPE ) )
        {
            psLC = &gsSvcWork.psList[dwLCIndex].uCnx.sLC;
            if ( ! psLC->fQualified )
                continue;
            for ( dwLCSubIndex = 0 ; dwLCSubIndex < psLC->dwCount ; dwLCSubIndex ++ )
            {
                if ( ( ( psLC->tdwPlaza[dwLCSubIndex] == dwPlaza ) ||
                       ( dwPlaza == MSG_HEADER_PLAZA_BROADCAST   ) ) &&
                     ( ( psLC->tdwLane[dwLCSubIndex]  == dwLane  ) ||
                       ( dwLane  == MSG_HEADER_LANE_BROADCAST    ) ) )
                {
                    AComSendMessage( gsSvcWork.hWks, gsSvcWork.psList[dwLCIndex].hCnx, 0, dwMsgSize, pvMsg );
                    LCTraceMsg( FALSE, dwLCIndex, dwMsgSize, pvMsg );
                    SpySendMessage( dwLCIndex, "SPYOUT", dwMsgSize, pvMsg );
                    if ( psLC->fMultiple )
                        break;
                }
            }       
        }
    }

    LeaveCriticalSection( &gsSvcWork.sCritical );
}




PROTECTED void LCTraceMsg(BOOL fRecv, DWORD64 dwLCIndex, DWORD dwMsgSize, BYTE * pbMsg)
{
    static char   tcXMap[] = "0123456789ABCDEF";
    DWORD         dwIndex;
    DWORD         dwPos;
    BOOL          fTooLong = FALSE;
    ROUTESVC_LC * psLC;
    char          szBuf[1024];

    if ( gsSvcWork.sParmWork.dwTraceLCMsg == 0 )
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

    psLC = &gsSvcWork.psList[dwLCIndex].uCnx.sLC;

    if ( fRecv )
        NTSVCInfo( "LC:RECV - Idx%03u:%s(%03u) [ << %s]",
                   dwLCIndex,
                   psLC->fQualified ? psLC->fMultiple ? "M" : "S" : "U",
                   psLC->dwCount,
                   szBuf );
    else
        NTSVCInfo( "LC:SEND - Idx%03u:%s(%03u) [ >> %s]",
                   dwLCIndex,
                   psLC->fQualified ? psLC->fMultiple ? "M" : "S" : "U",
                   psLC->dwCount,
                   szBuf );
}

    
