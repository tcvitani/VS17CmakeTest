/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : authsvc_vt
 * FILE       : authsvc_vt_comcallback.c
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
#include <dbif.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>

#include <csr_msg.h>
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>

#include <msg_lc_auth_vt_enl_rep.h>
#include <msg_lc_auth_vt_enl_req.h>
#include <msg_lc_auth_vt_exl_rep.h>
#include <msg_lc_auth_vt_exl_req.h>
#include <msg_lc_auth_vt_enl_rep_v2.h>
#include <msg_lc_auth_vt_enl_req_v2.h>
#include <msg_lc_auth_vt_exl_rep_v2.h>
#include <msg_lc_auth_vt_exl_req_v2.h>
#include <msg_lc_auth_vt_enl_rep_v3.h>
#include <msg_lc_auth_vt_enl_req_v3.h>
#include <msg_lc_auth_vt_exl_rep_v3.h>
#include <msg_lc_auth_vt_exl_req_v3.h>


#include <authsvc_vt_glob.h>
#include <authsvc_vt_text.h>
#include <authsvc_vt_cmd.h>
#include <authsvc_vt_commsg.h>
#include <authsvc_vt_db.h>
#include <authsvc_vt_authoper.h>

#define LOC_DEF
#include <authsvc_vt_comcallback.h>
#undef LOC_DEF

#include <memclass.h>


// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK ComCallbackShutdown(
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
PROTECTED void CALLBACK ComCallbackShutdown(
        DWORD dwWksUsrKey,
        DWORD dwError )
{
    SVC_ERR( ERROR_INVALID_DATA, "ERR_WKS_SHUTDOWN" );
    NTSVCSignalEnd();
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK ComCallbackConnection(
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
PROTECTED DWORD64 CALLBACK ComCallbackConnection(
        DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    BOOL bSent = FALSE;

    // Il s'agit d'une connexion avec le service de communication (RouteSvc)
    if ( dwInstUsrKey == AUTHSVC_PIPE_COM_TYPE )
    {
        NTSVCInfo( "ComCallbackConnection(), connected to route service" );

        // Memorize the connection handle of the route service
        gsSvcWork.hComCnx = hCnxHandle; 

        // Mettre en place le filtre ne laissant passer que les infos de connexion
        // et de déconnexion
        bSent = ROUTE_Send_msg_sv_filt_dec();
        if ( bSent == FALSE )
        {
            NTSVCInfo( "ComCallbackConnection(), impossible d'établir le filtre" );
            AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
        }
    }
    else
        // Interface de commande, rien de spécial à faire.
        NTSVCInfo( "ComCallbackConnection(), unknown connection" );

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
PROTECTED void CALLBACK ComCallbackDisconnection(
        DWORD64 dwInstUsrKey,
        DWORD64 dwCnxUsrKey )
{
	gsSvcWork.hComCnx = (ACOM_CNX_HANDLE)0;
    NTSVCInfo( "ComCallbackDisconnection(), disconnected from the route servcie" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK ComCallbackReceived(
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
PROTECTED void CALLBACK ComCallbackReceived(
        DWORD64 dwInstUsrKey,
        DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_ENL_REQ * psAuthEnlReq = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ * psAuthExlReq = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ_V2 * psAuthEnlReqV2 = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ_V2 * psAuthExlReqV2 = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ_V3 * psAuthEnlReqV3 = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ_V3 * psAuthExlReqV3 = NULL;
	DWORD dwErr = NO_ERROR;

    pbMsg[dwMsgSize] = 0;
    NTSVCInfo( "ComCallbackReceived(), Message received: %s", pbMsg );
 
    p_header = MSG_New_Read_If_Found (gsSvcWork.hlRouteMsgList, pbMsg, dwMsgSize);
    if (p_header != NULL)
    {
        switch (p_header->cd)
        {
            case MSG_SV_CON_REP_CD:
				NTSVCInfo( "ComCallbackReceived(), Message received from the route service" );
                break;

			case MSG_LC_AUTH_VT_ENL_REQ_CD:
                if ( gsSvcWork.sParmWork.dwIsAuthClient )
                {
                     NTSVCInfo( "ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_ENL_REQ_CD to the authorization server" );
					// Forward message to authorization server
                    dwErr = AComSendMessage (gsSvcWork.hAuthWks, 
                                gsSvcWork.hSrvCnx,
                                (ULONG) gsSvcWork.hAuthWks,
                                dwMsgSize,
                                pbMsg );

                    if ( dwErr != NO_ERROR )
					    NTSVCInfo( "ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_ENL_REQ_CD to the authorization server failed, error %d", dwErr );
                }
                else
                {
					psAuthEnlReq = (struct MSG_LC_AUTH_VT_ENL_REQ *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization ENL request received from plaza %d, lane %d, entry_trs_ref_num:%d entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] entry_provider:[%s]",
						psAuthEnlReq->header.plaza_number,
						psAuthEnlReq->header.lane_number,
						psAuthEnlReq->body.entry_trs_ref_num,
						psAuthEnlReq->body.entry_plaza_id,
						psAuthEnlReq->body.entry_lane_id,
						psAuthEnlReq->body.entry_trs_id,
						psAuthEnlReq->body.entry_vrn,
						psAuthEnlReq->body.entry_vrn_country,
						psAuthEnlReq->body.entry_provider);

					Auth_VT_EnlReq_Oper(psAuthEnlReq, 0);

                 }
				break;

			case MSG_LC_AUTH_VT_EXL_REQ_CD:
				if (gsSvcWork.sParmWork.dwIsAuthClient)
				{
					NTSVCInfo("ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_EXL_REQ_CD to the authorization server");
					// Forward message to authorization server
					dwErr = AComSendMessage(gsSvcWork.hAuthWks,
						gsSvcWork.hSrvCnx,
						(ULONG)gsSvcWork.hAuthWks,
						dwMsgSize,
						pbMsg);

					if (dwErr != NO_ERROR)
						NTSVCInfo("ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_EXL_REQ_CD to the authorization server failed, error %d", dwErr);
				}
				else
				{
					psAuthExlReq = (struct MSG_LC_AUTH_VT_EXL_REQ *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization EXL request received from plaza %d, lane %d, exit_trs_ref_num:%d ,exit_plaza_id:%d exit_lane_id:%d exit_trs_id:[%s] exit_vrn:[%s] exit_vrn_country:[%s] exit_provider:[%s]",
						psAuthExlReq->header.plaza_number,
						psAuthExlReq->header.lane_number,
						psAuthExlReq->body.exit_trs_ref_num,
						psAuthExlReq->body.exit_plaza_id,
						psAuthExlReq->body.exit_lane_id,
						psAuthExlReq->body.exit_trs_id,
						psAuthExlReq->body.exit_vrn,
						psAuthExlReq->body.exit_vrn_country,
						psAuthExlReq->body.exit_provider);

					Auth_VT_ExlReq_Oper(psAuthExlReq, 0);

				}
				break;
			case MSG_LC_AUTH_VT_ENL_REQ_V2_CD:
				if (gsSvcWork.sParmWork.dwIsAuthClient)
				{
					NTSVCInfo("ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_ENL_REQ_V2_CD to the authorization server");
					// Forward message to authorization server
					dwErr = AComSendMessage(gsSvcWork.hAuthWks,
						gsSvcWork.hSrvCnx,
						(ULONG)gsSvcWork.hAuthWks,
						dwMsgSize,
						pbMsg);

					if (dwErr != NO_ERROR)
						NTSVCInfo("ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_ENL_REQ_V2_CD to the authorization server failed, error %d", dwErr);
				}
				else
				{
					psAuthEnlReqV2 = (struct MSG_LC_AUTH_VT_ENL_REQ_V2 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization ENL_V2 request received from plaza %d, lane %d, entry_trs_ref_num:%d entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] entry_provider:[%s]",
						psAuthEnlReqV2->header.plaza_number,
						psAuthEnlReqV2->header.lane_number,
						psAuthEnlReqV2->body.entry_trs_ref_num,
						psAuthEnlReqV2->body.entry_plaza_id,
						psAuthEnlReqV2->body.entry_lane_id,
						psAuthEnlReqV2->body.entry_trs_id,
						psAuthEnlReqV2->body.entry_vrn,
						psAuthEnlReqV2->body.entry_vrn_country,
						psAuthEnlReqV2->body.entry_provider);

					Auth_VT_EnlReq_V2_Oper(psAuthEnlReqV2, 0);

				}
				break;

			case MSG_LC_AUTH_VT_EXL_REQ_V2_CD:
				if (gsSvcWork.sParmWork.dwIsAuthClient)
				{
					NTSVCInfo("ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_EXL_REQ_V2_CD to the authorization server");
					// Forward message to authorization server
					dwErr = AComSendMessage(gsSvcWork.hAuthWks,
						gsSvcWork.hSrvCnx,
						(ULONG)gsSvcWork.hAuthWks,
						dwMsgSize,
						pbMsg);

					if (dwErr != NO_ERROR)
						NTSVCInfo("ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_EXL_REQ_V2_CD to the authorization server failed, error %d", dwErr);
				}
				else
				{
					psAuthExlReqV2 = (struct MSG_LC_AUTH_VT_EXL_REQ_V2 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request EXL_V2 received from plaza %d, lane %d, exit_plaza_id:%d exit_lane_id:%d exit_trs_id:[%s] exit_vrn:[%s]",
						psAuthExlReqV2->header.plaza_number,
						psAuthExlReqV2->header.lane_number,
						psAuthExlReqV2->body.exit_plaza_id,
						psAuthExlReqV2->body.exit_lane_id,
						psAuthExlReqV2->body.exit_trs_id,
						psAuthExlReqV2->body.exit_vrn);

					Auth_VT_ExlReq_V2_Oper(psAuthExlReqV2, 0);

				}
				break;
			case MSG_LC_AUTH_VT_ENL_REQ_V3_CD:
				if (gsSvcWork.sParmWork.dwIsAuthClient)
				{
					NTSVCInfo("ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_ENL_REQ_V3_CD to the authorization server");
					// Forward message to authorization server
					dwErr = AComSendMessage(gsSvcWork.hAuthWks,
						gsSvcWork.hSrvCnx,
						(ULONG)gsSvcWork.hAuthWks,
						dwMsgSize,
						pbMsg);

					if (dwErr != NO_ERROR)
						NTSVCInfo("ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_ENL_REQ_V3_CD to the authorization server failed, error %d", dwErr);
				}
				else
				{
					psAuthEnlReqV3 = (struct MSG_LC_AUTH_VT_ENL_REQ_V3 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization ENL_V3 request received from plaza %d, lane %d, entry_trs_ref_num:%d, entry_plaza_id:%d, entry_lane_id:%d, entry_trs_id:[%s], entry_vrn:[%s], entry_vrn_country:[%s], entry_provider:[%s], ticket_type %d, flag_delete %d, flag_no_insert %d, request_id %d",
						psAuthEnlReqV3->header.plaza_number,
						psAuthEnlReqV3->header.lane_number,
						psAuthEnlReqV3->body.entry_trs_ref_num,
						psAuthEnlReqV3->body.entry_plaza_id,
						psAuthEnlReqV3->body.entry_lane_id,
						psAuthEnlReqV3->body.entry_trs_id,
						psAuthEnlReqV3->body.entry_vrn,
						psAuthEnlReqV3->body.entry_vrn_country,
						psAuthEnlReqV3->body.entry_provider,
						psAuthEnlReqV3->body.ticket_type,
						psAuthEnlReqV3->body.flag_delete,
						psAuthEnlReqV3->body.flag_no_insert,
						psAuthEnlReqV3->body.request_id);

					Auth_VT_EnlReq_V3_Oper(psAuthEnlReqV3, 0);

				}
				break;

			case MSG_LC_AUTH_VT_EXL_REQ_V3_CD:
				if (gsSvcWork.sParmWork.dwIsAuthClient)
				{
					NTSVCInfo("ComCallbackReceived(), Forward message MSG_LC_AUTH_VT_EXL_REQ_V3_CD to the authorization server");
					// Forward message to authorization server
					dwErr = AComSendMessage(gsSvcWork.hAuthWks,
						gsSvcWork.hSrvCnx,
						(ULONG)gsSvcWork.hAuthWks,
						dwMsgSize,
						pbMsg);

					if (dwErr != NO_ERROR)
						NTSVCInfo("ComCallbackReceived(), Forwarding of message MSG_LC_AUTH_VT_EXL_REQ_V3_CD to the authorization server failed, error %d", dwErr);
				}
				else
				{
					psAuthExlReqV3 = (struct MSG_LC_AUTH_VT_EXL_REQ_V3 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request EXL_V3 received from plaza %d, lane %d, exit_plaza_id:%d exit_lane_id:%d exit_trs_id:[%s] exit_vrn:[%s]",
						psAuthExlReqV3->header.plaza_number,
						psAuthExlReqV3->header.lane_number,
						psAuthExlReqV3->body.exit_plaza_id,
						psAuthExlReqV3->body.exit_lane_id,
						psAuthExlReqV3->body.exit_trs_id,
						psAuthExlReqV3->body.exit_vrn);

					Auth_VT_ExlReq_V3_Oper(psAuthExlReqV3, 0);

				}
				break;

			default:
                NTSVCInfo( "ComCallbackReceived(), Message not used" );
                break;
		}

		MSG_Delete_All ((HMSG *)&p_header);
    }
    else
        NTSVCInfo( "ComCallbackReceived(), Message not found in the list" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK ComCallbackSent(
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
PROTECTED void CALLBACK ComCallbackSent(
        DWORD64 dwInstUsrKey,
        DWORD64 dwCnxUsrKey,
        DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    NTSVCInfo( "ComCallbackSent(), acquittement, code=%u", dwError );
}

