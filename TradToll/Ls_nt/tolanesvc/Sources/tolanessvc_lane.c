/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : tolanessvc
 * FILE       : tolanessvc_lane.c
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
#include <csr_list.h>
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_reference.h>
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <col.h>

#include <tolanessvc_glob.h>

#include <tolanessvc_text.h>
#define LOC_DEF
#include <tolanessvc_lane.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- CODE ----------------------



// --------------- CODE ----------------------

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneListCount( TOLANESSVC_LANE * psList )
 * PARAMETERS: psList  : Pointe sur le premier élément d'un tableau de voies
 * RETURN    : Nombre de voies dans la liste
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Compte le nombre de voies enregistrées dans une liste
 *             Remarque : cette fonction utilise comme taille maximum de la liste
 *             la valeur gsSvcWork.sParmWork.dwMaxLanes.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneListCount( TOLANESSVC_LANE * psList )
{
    DWORD dwIndex;   // Indice de boucle
    DWORD dwCount;   // Nombre à retourner

    EnterCriticalSection( &gsSvcWork.sCritical );
    for ( dwCount = 0, dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
        if ( psList[dwIndex].bBusy )
            dwCount ++;
    LeaveCriticalSection( &gsSvcWork.sCritical );

    return dwCount;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD LaneListFindLane( TOLANESSVC_LANE * psList, DWORD dwPlaza, DWORD dwLane )
 * PARAMETERS: psList  : Pointe sur le premier élément d'un tableau de voies
 *             dwPlaza : Identifiant de la gare à trouver
 *             dwLane  : Identifiant de la voie à trouver
 * RETURN    : 0xFFFFFFFF si non trouvée, l'index (0 based) de la voie dans le tableau sinon
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Recherche une voie particulière dans une liste de voies
 *             Remarque : cette fonction utilise comme taille maximum de la liste
 *             la valeur gsSvcWork.sParmWork.dwMaxLanes.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneListFindLane( TOLANESSVC_LANE * psList, DWORD dwPlaza, DWORD dwLane )
{
    DWORD dwIndex;   // Indice de boucle

    for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
    {
        // Attention, l'élément doit être occupé par une voie (bBusy = TRUE)
        if ( ( psList[dwIndex].dwLane == dwLane ) && 
             ( psList[dwIndex].dwPlaza == dwPlaza ) &&
             psList[dwIndex].bBusy )
        {
            // C'est bon, on l'a trouvée
            return dwIndex;
        }
    }

    // Pas trouvé
    return 0xFFFFFFFF;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void LaneListMerge( TOLANESSVC_LANE * psList, TOLANESSVC_LANE * psListNew )
 * PARAMETERS: psList    : Pointe sur le premier élément de l'ancienne liste
 *             psListNew : Pointe sur le premier élément de la nouvelle liste
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Effectue une synthèse entre une liste de voie ancienne (A) et une 
 *             liste nouvelle (B). La synthèse remplace la liste A, alors que la
 *             liste B est utilisée comme liste de travail (donc potentiellement
 *             modifiée).
 *             Le traitement effectué est le suivant : 
 *             Pour les voies de A figurant dans B, on reporte les attributs
 *             de A dans B.
 *             Pour les voies de A ne figurant pas dans B, on ne fait rien
 *             (ces voies on disparues).
 *             Pour les voies de B ne figurant pas dans A, on ne fait rien
 *             non plus (elle viennent d'apparaitre).
 *             Enfin, la totalité de la liste B (nouvelle liste avec report
 *             des attributs pour les voies n'ayant pas disparues) est recopiée
 *             dans la liste A.
 * --------------------------------------------------------------------
 */
PROTECTED void LaneListMerge( TOLANESSVC_LANE * psList, TOLANESSVC_LANE * psListNew )
{
    DWORD dwIndex;
    DWORD dwNewIndex;
    DWORD dwErr;
    BOOL bDiff = FALSE;  // Indique un changement dans le contenu de la liste

    EnterCriticalSection( &gsSvcWork.sCritical );

    // Faire l'inventaire des voies qui ont apparues
    // C'est uniquement pour les infos de debug, sinon, ca ne sert à rien
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
    {
        if ( psListNew[dwIndex].bBusy )
        {
            dwNewIndex = LaneListFindLane( psList, psListNew[dwIndex].dwPlaza, psListNew[dwIndex].dwLane );
            if ( dwNewIndex == 0xFFFFFFFF )
            {
                NTSVCInfo( "LaneListMerge(), la voie PZ%04u-LN%04u est nouvelle", psListNew[dwIndex].dwPlaza, psListNew[dwIndex].dwLane );
                bDiff = TRUE;
            }
        }
    }

    // Faire l'inventaire de celles qui sont maintenue.
    // Ici, c'est réellement utile puisqu'on reporte les attributs des voies qui sont
    // maintenues.
    // Accessoirement, ca permet aussi de faire l'inventaire des voies qui ont disparues.
    for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
    {
        if ( psList[dwIndex].bBusy )
        {
            dwNewIndex = LaneListFindLane( psListNew, psList[dwIndex].dwPlaza, psList[dwIndex].dwLane );
            if ( dwNewIndex != 0xFFFFFFFF )
            {
                // Reprendre la contenu de l'ancienne structure dans la nouvelle
                // pour les champs qui consernent l'état courant uniquement
                psListNew[dwNewIndex].bBusy = TRUE;
                psListNew[dwNewIndex].bConnected = psList[dwNewIndex].bConnected;
                psListNew[dwNewIndex].bMustSend = psList[dwNewIndex].bMustSend;
                psListNew[dwNewIndex].dwPlaza = psList[dwNewIndex].dwPlaza;
                psListNew[dwNewIndex].dwLane = psList[dwNewIndex].dwLane;
                psListNew[dwNewIndex].llLastSent = psList[dwNewIndex].llLastSent;

                // Si le fichier de référence a changé, on force l'envoie d'un nouveau
                // message de référence
                if ( _stricmp( psListNew[dwNewIndex].szRefFile, psList[dwNewIndex].szRefFile ) != 0 )
                    psListNew[dwNewIndex].bMustSend = TRUE;
            }
            else
            {
                bDiff = TRUE;
                NTSVCInfo( "LaneListMerge(), la voie PZ%04u-LN%04u a disparue", psList[dwIndex].dwPlaza, psList[dwIndex].dwLane );
            }
        }
    }

    // Reporter le contenue de la nouvelle liste dans la liste courante
    CopyMemory( psList, psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*psList) );

    LeaveCriticalSection( &gsSvcWork.sCritical );

    // En cas de changement dans la liste des voies du système, on interroge
    // le service de com pour connaitre les états de connexion
    if ( bDiff )
    {
        dwErr = LaneSendQueryCnxMsg();
        if ( dwErr != NO_ERROR )
        {
            NTSVCInfo( "LaneListMerge(), erreur %u, impossible d'obtenir la liste des voies connectées", dwErr );
            AComDisconnectPeer( gsSvcWork.hWks, gsSvcWork.hComCnx, FALSE );
        }
    }
}
                


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
 * SYNTAX    : PROTECTED DWORD LaneSendRefMsg( DWORD dwIndex )
 * PARAMETERS: dwIndex : numéro de la voie consernée dans la table courante
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoi du message de référence pour une voie données.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD LaneSendRefMsg( DWORD dwIndex )
{
    DWORD dwErr;
    DWORD dwMsgSize;
    struct MSG_REFERENCE * psMsg;
    BYTE tbBuffer[1000];

    psMsg = MSG_REFERENCE_New();
    if ( psMsg == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    psMsg->header.id = gsSvcWork.sParmWork.dwAppMsgId;
    psMsg->header.plaza_number = gsSvcWork.psList[dwIndex].dwPlaza;
    psMsg->header.lane_number = gsSvcWork.psList[dwIndex].dwLane;
    strncpy_s( psMsg->body.reference_name, MSG_REFERENCE_REFERENCE_NAME_LENGTH+1, gsSvcWork.psList[dwIndex].szRefFile, sizeof(psMsg->body.reference_name) );
    psMsg->body.reference_name[sizeof(psMsg->body.reference_name)-1] = '\0';

    dwMsgSize = sizeof( tbBuffer );
    if ( ! MSG_REFERENCE_Write( psMsg, tbBuffer, dwMsgSize, &dwMsgSize ) )
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
    DWORD dwIndex;
    void * pvHead;
    struct MSG_HEADER * psLSHead;
    struct MSG_SV_CON_REP * psConMsg;
    TOLANESSVC_LANE * psLane;       // Pointe sur la voie consernée, dans la liste courante des voies

    // Décoder l'entête par rapport aux messages attendus
    pvHead = MSG_New_Read_If_Found( gsSvcWork.hInMsgList, pvMsg, dwMsgSize );
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

            // On utilise la liste des voie, on passe donc par la section critique
            EnterCriticalSection( &gsSvcWork.sCritical );

            // La voie est-elle enregistrée
            dwIndex = LaneListFindLane( gsSvcWork.psList, psConMsg->body.plazanum, psConMsg->body.lanenum );
            if ( dwIndex != 0xFFFFFFFF )
            {
                // On la connait, c'est le cas standard
                psLane = &gsSvcWork.psList[dwIndex];
                if ( ( psConMsg->body.state != 0 ) && ! psLane->bConnected )
                {
                    psLane->bMustSend = TRUE;
                    psLane->bConnected = ( psConMsg->body.state != 0 );
                    SetEvent( gsSvcWork.hWakeupEvent );
                }
                else
                    psLane->bConnected = ( psConMsg->body.state != 0 );
            }
            else
            {
                // On ne la connait pas, c'est pas bon !
                // Si au moins une voie est enregistrée dans le système, c'est une erreur
                if ( LaneListCount( gsSvcWork.psList ) > 0 )
                {
                    dwErr = ERROR_INVALID_DATA;
                    SVC_ERR( dwErr, "ERR_UNREFERENCED_LC" );
                }
                // Sinon, il est fort probable qu'on ait pas encore eu le temps de se
                // connecter à la base
                else
                    NTSVCInfo( "LaneReceiveMsg(), la voie PZ%04u-LN%04u est connectée, mais la liste des voies du système est vide", psConMsg->body.plazanum, psConMsg->body.lanenum  );
            }

            LeaveCriticalSection( &gsSvcWork.sCritical );
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




