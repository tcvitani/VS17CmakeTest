/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : authsvc_vt
 * FILE       : authsvc_vt_callback.c
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

#include <msg_lc_header.h>
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
#include <authsvc_vt_authoper.h>
#include <authsvc_vt_text.h>
#include <authsvc_vt_cmd.h>
#include <authsvc_vt_commsg.h>

#define LOC_DEF
#include <authsvc_vt_callback.h>
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
        DWORD64 dwWksUsrKey,
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
PROTECTED DWORD64 CALLBACK CallbackConnection(
        DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    if ( dwInstUsrKey == AUTHSVC_PIPE_SERVER_TYPE )
        NTSVCInfo( "ComCallbackConnection(), authorization client connected" );
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CLIENT_TYPE )
    {
        NTSVCInfo( "ComCallbackConnection(), connected to authorization server" );

        // Memorize the connection handle of the authorization server
        gsSvcWork.hSrvCnx = hCnxHandle;
    }
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CMD_TYPE )
    {
        // Interface de commande, rien de spécial à faire.
        NTSVCInfo( "CallbackConnection(), connexion avec une interface de commande" );
    }
    else
    {
        NTSVCInfo( "CallbackConnection(), Unknown connection" );
    }

    // Return connection handle for later use in call back functions
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
        DWORD64 dwCnxUsrKey )
{
    if ( dwInstUsrKey == AUTHSVC_PIPE_SERVER_TYPE )
        NTSVCInfo( "CallbackDisconnection(), client disconnected" );
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CLIENT_TYPE )
	{
		gsSvcWork.hSrvCnx = (ACOM_CNX_HANDLE)0;
        NTSVCInfo( "CallbackDisconnection(), disconnected from server" );
	}
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CMD_TYPE )
        NTSVCInfo( "CallbackDisconnection(), command interface disconnected" );
	else
       NTSVCInfo( "CallbackDisconnection(), unknown connection type" );
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
        DWORD64 dwInstUsrKey,
        DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    char * pcCmd;
    struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_ENL_REQ * psAuthEnlReq = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ * psAuthExlReq = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ_V2 * psAuthEnlReqV2 = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ_V2 * psAuthExlReqV2 = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ_V3 * psAuthEnlReqV3 = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ_V3 * psAuthExlReqV3 = NULL;

    if ( dwInstUsrKey == AUTHSVC_PIPE_SERVER_TYPE )
    {
        NTSVCInfo( "CallbackReceived(), message received from the client" );

		// DB action + return the message to client if necessary
        p_header = MSG_New_Read_If_Found (gsSvcWork.hlRouteMsgList, pbMsg, dwMsgSize);
        if (p_header != NULL)
        {
            switch (p_header->cd)
            {
				case MSG_LC_AUTH_VT_ENL_REQ_CD:
				{
					psAuthEnlReq = (struct MSG_LC_AUTH_VT_ENL_REQ *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request ENL received from plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s]",
						psAuthEnlReq->header.plaza_number,
						psAuthEnlReq->header.lane_number,
						psAuthEnlReq->body.entry_plaza_id,
						psAuthEnlReq->body.entry_lane_id,
						psAuthEnlReq->body.entry_trs_id,
						psAuthEnlReq->body.entry_vrn);

					Auth_VT_EnlReq_Oper(psAuthEnlReq, 0);
				}
				break;
				case MSG_LC_AUTH_VT_EXL_REQ_CD:
				{
					psAuthExlReq = (struct MSG_LC_AUTH_VT_EXL_REQ *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request EXL received from plaza %d, lane %d, exit_plaza_id:%d exit_lane_id:%d exit_trs_id:[%s] exit_vrn:[%s]",
						psAuthExlReq->header.plaza_number,
						psAuthExlReq->header.lane_number,
						psAuthExlReq->body.exit_plaza_id,
						psAuthExlReq->body.exit_lane_id,
						psAuthExlReq->body.exit_trs_id,
						psAuthExlReq->body.exit_vrn);

					Auth_VT_ExlReq_Oper(psAuthExlReq, 0);

				}
				break;
				case MSG_LC_AUTH_VT_ENL_REQ_V2_CD:
				{
					psAuthEnlReqV2 = (struct MSG_LC_AUTH_VT_ENL_REQ_V2 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request ENL_V2 received from plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s]",
						psAuthEnlReqV2->header.plaza_number,
						psAuthEnlReqV2->header.lane_number,
						psAuthEnlReqV2->body.entry_plaza_id,
						psAuthEnlReqV2->body.entry_lane_id,
						psAuthEnlReqV2->body.entry_trs_id,
						psAuthEnlReqV2->body.entry_vrn);

					Auth_VT_EnlReq_V2_Oper(psAuthEnlReqV2, 0);
				}
				break;
				case MSG_LC_AUTH_VT_EXL_REQ_V2_CD:
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
				{
					psAuthEnlReqV3 = (struct MSG_LC_AUTH_VT_ENL_REQ_V3 *) p_header;

					NTSVCInfo("ComCallbackReceived(), Authorization request ENL_V3 received from plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s]",
						psAuthEnlReqV3->header.plaza_number,
						psAuthEnlReqV3->header.lane_number,
						psAuthEnlReqV3->body.entry_plaza_id,
						psAuthEnlReqV3->body.entry_lane_id,
						psAuthEnlReqV3->body.entry_trs_id,
						psAuthEnlReqV3->body.entry_vrn);

					Auth_VT_EnlReq_V3_Oper(psAuthEnlReqV3, 0);
				}
				break;
				case MSG_LC_AUTH_VT_EXL_REQ_V3_CD:
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
                    NTSVCInfo( "CallbackReceived(), Message not used" );
                    break;
            }
            MSG_Delete_All ((HMSG *)&p_header);
        }
    }
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CLIENT_TYPE )
    {
        NTSVCInfo( "CallbackReceived(), message received from the server" );

        // Forward the result to lane through the route service
        ROUTE_ACOM_Send_Buffer ( gsSvcWork.hComCnx, dwMsgSize, pbMsg );
    }
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CMD_TYPE )
    {
        NTSVCInfo( "CallbackReceived(), message received from the command interface" );

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
	else
       NTSVCInfo( "CallbackReceived(), message received from unknown connection type" );
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
    if ( dwInstUsrKey == AUTHSVC_PIPE_SERVER_TYPE )
        NTSVCInfo( "CallbackSent(), message from the AUTHSVC_PIPE_SERVER_TYPE, code=%u", dwError );
    else if ( dwInstUsrKey == AUTHSVC_PIPE_CLIENT_TYPE )
        NTSVCInfo( "CallbackSent(), message from the AUTHSVC_PIPE_CLIENT_TYPE, code=%u", dwError );
	else if (dwInstUsrKey == AUTHSVC_PIPE_CMD_TYPE)
		NTSVCInfo("CallbackSent(),  message from the AUTHSVC_PIPE_CMD_TYPE, code=%u", dwError);
	else
		NTSVCInfo("CallbackSent(), message sent to unknown connection type, code=%u", dwError);
}

