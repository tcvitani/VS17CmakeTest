/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_lane.c
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
#include <ntsvc.h>
#include <dbif.h>
#include <col.h>
#include <csr_list.h>
#include <csr_msg.h>
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <msg_lc_header.h>
#include <msg_lc_perm_rq_send_file.h>
#include <msg_lc_perm_send_file.h>
#include <msg_lc_backup_file.h>
#include <trc.h>

#include <todbsvc_glob.h>
#include <todbsvc_text.h>

#define LOC_DEF
#include <todbsvc_lane.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- CODE ----------------------



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneSendSetFilterMsg()
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoi du message de mise en place de filtre sur la connexion
 *             avec le serveur de com.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneSendSetFilterMsg()
{
    DWORD dwErr;
    DWORD dwMsgSize;
    struct MSG_SV_FILT_DEC * psMsg;
    struct MSG_SV_FILT_DEC_Idcd * psIDCD;
    BYTE tbBuffer[100];

    psMsg = MSG_SV_FILT_DEC_New();
    if ( psMsg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    psMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;

    psIDCD = MSG_SV_FILT_DEC_Idcd_New( &psMsg->list_idcd );
    if ( psIDCD == NULL )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    psIDCD->id = MSG_SV_FILT_DEC_ID_ANY;
    psIDCD->cd = MSG_SV_CON_REP_CD;

    psIDCD = MSG_SV_FILT_DEC_Idcd_New( &psMsg->list_idcd );
    if ( psIDCD == NULL )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    psIDCD->id = MSG_SV_FILT_DEC_ID_ANY;
    psIDCD->cd = MSG_PERM_RQ_SEND_FILE_CD;

    dwMsgSize = sizeof( tbBuffer );
    if ( ! MSG_SV_FILT_DEC_Write( psMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = AComSendMessage( gsSvcWork.hWks, gsSvcWork.hComCnx, 0, dwMsgSize, tbBuffer );
    MSG_Delete_All( &psMsg );

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneSendOkToSend( TODBSVC_LANE * psLane )
 * PARAMETERS: psLane : voie consernée
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoi du message d'autorisation d'émettre à destination d'une voie.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneSendOkToSend( TODBSVC_LANE * psLane )
{
    DWORD dwErr;
    DWORD dwMsgSize;
    struct MSG_PERM_SEND_FILE * psMsg;
    BYTE tbBuffer[100];
    
    psMsg = MSG_PERM_SEND_FILE_New();
    if ( psMsg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    psMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;
    psMsg->header.plaza_number = psLane->sId.dwPlaza;
    psMsg->header.lane_number = psLane->sId.dwLane;

    dwMsgSize = sizeof( tbBuffer );
    if ( ! MSG_PERM_SEND_FILE_Write( psMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = AComSendMessage( gsSvcWork.hWks, gsSvcWork.hComCnx, 0, dwMsgSize, tbBuffer );
    MSG_Delete_All( &psMsg );

    return dwErr;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneSendBackup( TODBSVC_LANE * psLane )
 * PARAMETERS: psLane : voie consernée
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoi du message d'autorisation d'émettre à destination d'une voie.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneSendBackup( TODBSVC_LANE * psLane, DWORD dwFirst, DWORD dwLast )
{
    DWORD dwErr;
    DWORD dwMsgSize;
    struct MSG_BACKUP_FILE * psMsg;
    BYTE tbBuffer[100];

    psMsg = MSG_BACKUP_FILE_New();
    if ( psMsg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    psMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;
    psMsg->header.plaza_number = psLane->sId.dwPlaza;
    psMsg->header.lane_number = psLane->sId.dwLane;
    psMsg->body.begin_file_number = dwFirst;
    psMsg->body.end_file_number = dwLast;

    dwMsgSize = sizeof( tbBuffer );
    if ( ! MSG_BACKUP_FILE_Write( psMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = AComSendMessage( gsSvcWork.hWks, gsSvcWork.hComCnx, 0, dwMsgSize, tbBuffer );
    MSG_Delete_All( &psMsg );

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneSendQueryCnxMsg()
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoi du message de demande d'état de connexion de toutes les voies
 *             sur la connexion avec le serveur de com.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneSendQueryCnxMsg()
{
    DWORD dwErr;
    DWORD dwMsgSize;
    struct MSG_SV_CON_REQ * psMsg;
    BYTE tbBuffer[100];

    psMsg = MSG_SV_CON_REQ_New();
    if ( psMsg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    psMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;

    dwMsgSize = sizeof( tbBuffer );
    if ( ! MSG_SV_CON_REQ_Write( psMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
    {
        MSG_Delete_All( &psMsg );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = AComSendMessage( gsSvcWork.hWks, gsSvcWork.hComCnx, 0, dwMsgSize, tbBuffer );
    MSG_Delete_All( &psMsg );

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneReceiveMsg( DWORD dwMsgSize, void * pvMsg )
 * PARAMETERS: dwMsgSize : Taille des données pointées par pvMsg
 *             pvMsg     : Pointe sur les données du message
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Traitement d'un message reçu sur la connexion avec le
 *             service de communication.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneReceiveMsg( DWORD dwMsgSize, void * pvMsg )
{
    DWORD dwErr = NO_ERROR;
    HCOLLECTIONITEM hItem;
    void * pvHead;
    struct MSG_HEADER * psLCHead;
    struct MSG_HEADER * psLSHead;
    struct MSG_SV_CON_REP * psConMsg;
    struct MSG_PERM_RQ_SEND_FILE * psSndMsg;
    TODBSVC_LANE * psLane;          // Pointe sur la voie connecté
    TODBSVC_LANE_ID sId;            // Id de la voie

    // Décoder l'entête par rapport aux messages attendus
    pvHead = MSG_New_Read_If_Found( gsSvcWork.hInMsgList, pvMsg, dwMsgSize );
    psLCHead = pvHead;
    psLSHead = pvHead;

    if ( pvHead == NULL )
    {
        dwErr = ERROR_INVALID_DATA;
        SVC_ERR( dwErr, "ERR_LC_MSG_INVALID" );
    }
    else
    {
        // Le message a un format attendu

        // Si le message est information de connexion / déconnexion
        if ( psLSHead->cd == MSG_SV_CON_REP_CD )
        {
            psConMsg = (struct MSG_SV_CON_REP *)psLSHead;
            sId.dwPlaza = psConMsg->body.plazanum;
            sId.dwLane = psConMsg->body.lanenum;

            // On utilise la liste des voie, on passe donc par la section critique
            ColLock( gsSvcWork.hLanes );

            // La voie est-elle enregistrée
            hItem = ColItemFind( gsSvcWork.hLanes, &sId );
            if ( hItem != NULL )
            {
                // On la connait, c'est le cas standard
                psLane = ColItemData( hItem );
                psLane->bConnected = ( psConMsg->body.state != 0 );
            }
            else
            {
                // On ne la connait pas, c'est pas bon !
                // Si au moins une voie est enregistrée dans le système, c'est une erreur
                if ( ColCount( gsSvcWork.hLanes ) > 0 )
                {
                    dwErr = ERROR_INVALID_DATA;
                    SVC_ERR( dwErr, "ERR_UNREFERENCED_LC" );
                }
                // Sinon, il est fort probable qu'on ait pas encore eu le temps de se
                // connecter à la base
                else
                    NTSVCInfo( "LaneReceiveMsg(), la voie PZ%04u-LN%04u est connectée, mais la liste des voies du système est vide", sId.dwPlaza, sId.dwLane );
            }

            ColUnlock( gsSvcWork.hLanes );
        }

        else if ( psLCHead->cd == MSG_PERM_RQ_SEND_FILE_CD )
        {
            psSndMsg = (struct MSG_PERM_RQ_SEND_FILE *)psLCHead;
            sId.dwPlaza = psSndMsg->header.plaza_number;
            sId.dwLane = psSndMsg->header.lane_number;

            // On utilise la liste des voie, on passe donc par la section critique
            ColLock( gsSvcWork.hLanes );

            // La voie est-elle enregistrée
            hItem = ColItemFind( gsSvcWork.hLanes, &sId );
            if ( hItem != NULL )
            {
                // On la connait, c'est le cas standard
                psLane = ColItemData( hItem );
                psLane->bRequestToSend = TRUE;
                NTSVCInfo( "LaneReceiveMsg(), la voie PZ%04u-LN%04u demande autorisation à émettre)", psLane->sId.dwPlaza, psLane->sId.dwLane );
                NTSVCError( "AVERTISSEMENT : La voie PZ%04u-LN%04u est en démarrage à froid (demande d'autorisation à émettre)", psLane->sId.dwPlaza, psLane->sId.dwLane );
            }
            else
            {
                // On ne la connait pas, c'est pas bon !
                // Si au moins une voie est enregistrée dans le système, c'est une erreur
                if ( ColCount( gsSvcWork.hLanes ) > 0 )
                {
                    dwErr = ERROR_INVALID_DATA;
                    SVC_ERR( dwErr, "ERR_UNREFERENCED_LC" );
                }
                // Sinon, il est fort probable qu'on ait pas encore eu le temps de se
                // connecter à la base
                else
                    NTSVCInfo( "LaneReceiveMsg(), la voie PZ%04u-LN%04u est connectée, mais la liste des voies du système est vide", sId.dwPlaza, sId.dwLane );
            }

            ColUnlock( gsSvcWork.hLanes );

        }
        else
        {
            dwErr = ERROR_INVALID_DATA;
            SVC_ERR( dwErr, "ERR_LC_MSG_UNEXPECTED" );
        }

        MSG_Delete_All( &pvHead );
    }

    return dwErr;
}



