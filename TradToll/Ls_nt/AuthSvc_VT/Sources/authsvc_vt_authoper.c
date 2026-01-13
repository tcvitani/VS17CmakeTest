/* --------------------------------------------------------------------
* (C) 1999 CS Route - All rights reserved
* --------------------------------------------------------------------
* MODULE     : 
* FILE       : 
* LANGAGE    : C
* --------------------------------------------------------------------
* KEYWORDS   : 
* --------------------------------------------------------------------
* SUMMARY    : 
* --------------------------------------------------------------------
* DESCRIPTION: 
* --------------------------------------------------------------------
*/

#include <windows.h>
#include <stdio.h>
#include <dbif.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>

#include <csr_msg.h>
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
#include <authsvc_vt_commsg.h>
#include <authsvc_vt_db.h>

#define LOC_DEF
#include <authsvc_vt_authoper.h>
#undef LOC_DEF

// --------------- INTERNALS -----------------

#include <memclass.h>


PRIVATE BOOL AuthVT_EnlReq_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP *psAuthResp);
PRIVATE BOOL AuthVT_EnlReq_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);

PRIVATE BOOL AuthVT_ExlReq_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP *psAuthResp);
PRIVATE BOOL AuthVT_ExlReq_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);

PRIVATE BOOL AuthVT_EnlReqV2_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthResp);
PRIVATE BOOL AuthVT_EnlReqV2_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);

PRIVATE BOOL AuthVT_ExlReqV2_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthResp);
PRIVATE BOOL AuthVT_ExlReqV2_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);

PRIVATE BOOL AuthVT_EnlReqV3_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthResp);
PRIVATE BOOL AuthVT_EnlReqV3_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);

PRIVATE BOOL AuthVT_ExlReqV3_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthResp);
PRIVATE BOOL AuthVT_ExlReqV3_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle);


// --------------- CODE ----------------------
PROTECTED void Auth_VT_EnlReq_Oper(struct MSG_LC_AUTH_VT_ENL_REQ *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_ENL_REP_New();
	//memcpy(&psAuthRsp->header, &psAuthReq->header, sizeof(struct MSG_HEADER));
	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999


	psAuthRsp->body.dte_ref_entry = psAuthReq->body.dte_ref_entry;
	psAuthRsp->body.entry_trs_ref_num = psAuthReq->body.entry_trs_ref_num;
	psAuthRsp->body.entry_plaza_id = psAuthReq->body.entry_plaza_id;
	psAuthRsp->body.entry_lane_id = psAuthReq->body.entry_lane_id;
	memcpy_s(psAuthRsp->body.entry_trs_id, sizeof(psAuthRsp->body.entry_trs_id), psAuthReq->body.entry_trs_id, sizeof(psAuthReq->body.entry_trs_id));
	psAuthRsp->body.dte_entry = psAuthReq->body.dte_entry;
	memcpy_s(psAuthRsp->body.entry_vrn, sizeof(psAuthRsp->body.entry_vrn), psAuthReq->body.entry_vrn, sizeof(psAuthReq->body.entry_vrn));
	memcpy_s(psAuthRsp->body.entry_vrn_country, sizeof(psAuthRsp->body.entry_vrn_country), psAuthReq->body.entry_vrn_country, sizeof(psAuthReq->body.entry_vrn_country));
	memcpy_s(psAuthRsp->body.entry_provider, sizeof(psAuthRsp->body.entry_provider), psAuthReq->body.entry_provider, sizeof(psAuthReq->body.entry_provider));
	
	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_EnlReq(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_EnlReq(psAuthReq, psAuthRsp);

	NTSVCInfo("Auth_VT_EnlReq_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] entry_provider:[%s] flag_result:%u reason_id:%u",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id);

	if ( gsSvcWork.sParmWork.dwIsAuthServer )
	{
	    // Forward result to the authorization client
		AuthVT_EnlReq_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
	    // Send response to lane
		AuthVT_EnlReq_OperSendResp(psAuthRsp);
	}	

	 MSG_LC_AUTH_VT_ENL_REP_Delete_All(psAuthRsp);

}

PROTECTED void Auth_VT_ExlReq_Oper(struct MSG_LC_AUTH_VT_EXL_REQ *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_EXL_REP_New();

	//memcpy(&psAuthRsp->header, &psAuthReq->header, sizeof(struct MSG_HEADER));
	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999

	psAuthRsp->body.dte_ref_exit = psAuthReq->body.dte_ref_exit;
	psAuthRsp->body.exit_trs_ref_num = psAuthReq->body.exit_trs_ref_num;
	psAuthRsp->body.exit_plaza_id = psAuthReq->body.exit_plaza_id;
	psAuthRsp->body.exit_lane_id = psAuthReq->body.exit_lane_id;
	memcpy_s(psAuthRsp->body.exit_trs_id, sizeof(psAuthRsp->body.exit_trs_id), psAuthReq->body.exit_trs_id, sizeof(psAuthReq->body.exit_trs_id));
	psAuthRsp->body.dte_exit = psAuthReq->body.dte_exit;
	memcpy_s(psAuthRsp->body.exit_vrn, sizeof(psAuthRsp->body.exit_vrn), psAuthReq->body.exit_vrn, sizeof(psAuthReq->body.exit_vrn));
	memcpy_s(psAuthRsp->body.exit_vrn_country, sizeof(psAuthRsp->body.exit_vrn_country), psAuthReq->body.exit_vrn_country, sizeof(psAuthReq->body.exit_vrn_country));
	memcpy_s(psAuthRsp->body.exit_provider, sizeof(psAuthRsp->body.exit_provider), psAuthReq->body.exit_provider, sizeof(psAuthReq->body.exit_provider));
	psAuthRsp->body.flag_eticket = psAuthReq->body.flag_eticket;

	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_ExlReq(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_ExlReq(psAuthReq, psAuthRsp);

	NTSVCInfo("Auth_VT_ExlReq_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] entry_provider:[%s] flag_result:%u reason_id:%u",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id);

	if (gsSvcWork.sParmWork.dwIsAuthServer)
	{
		// Forward result to the authorization client
		AuthVT_ExlReq_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
		// Send response to lane
		AuthVT_ExlReq_OperSendResp(psAuthRsp);
	}

	MSG_LC_AUTH_VT_EXL_REP_Delete_All(psAuthRsp);

}



PRIVATE BOOL AuthVT_EnlReq_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP *psAuthResp)
{
	struct MSG_LC_AUTH_VT_ENL_REP * p_msg;
    
    // Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_New();
    if (p_msg == NULL)
    {
        NTSVC_ERR ("AuthVT_EnlReq_OperSendResp() : error MSG_LC_AUTH_VT_ENL_REP_New !!");
        
        return FALSE;
    }

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP));

	if( gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0 )
	{
        if ( ROUTE_ACOM_Send ( gsSvcWork.hComCnx, p_msg ) == FALSE )
            NTSVCInfo ( "AuthVT_EnlReq_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!" );
	}
	else
		NTSVCInfo ( "AuthVT_EnlReq_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!" );

	MSG_LC_AUTH_VT_ENL_REP_Delete_All(p_msg);

    return TRUE;
}

PRIVATE BOOL AuthVT_EnlReq_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP * p_msg;
    BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
    DWORD buffer_size;
	DWORD dwErr = NO_ERROR;
    
    // Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_New();
    if (p_msg == NULL)
    {
        NTSVC_ERR ("AuthVT_EnlReq_OperFwdResp() : error MSG_LC_AUTH_VT_ENL_REP_New !!");
        
        return FALSE;
    }

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP));

	// conversion du message en "ASCII"
    buffer_size = MSG_Write (p_msg, buffer, sizeof(buffer));
    if (buffer_size == 0UL)
    {
        NTSVC_ERR ("AuthVT_EnlReq_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_ENL_REP_Delete_All(p_msg);

        return FALSE;
    }

    dwErr = AComSendMessage (gsSvcWork.hAuthWks, 
                             hCnxHandle,
                             (ULONG) gsSvcWork.hAuthWks,
                             buffer_size,
                             buffer );

    if ( dwErr != NO_ERROR )
    {
        NTSVCInfo( "AuthVT_EnlReq_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr );
		MSG_LC_AUTH_VT_ENL_REP_Delete_All(p_msg);
        return FALSE;
	}

	MSG_LC_AUTH_VT_ENL_REP_Delete_All(p_msg);

    return TRUE;
}


PRIVATE BOOL AuthVT_ExlReq_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP *psAuthResp)
{
	struct MSG_LC_AUTH_VT_EXL_REP * p_msg;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReq_OperSendResp() : error MSG_LC_AUTH_VT_EXL_REP_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP));

	if (gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0)
	{
		if (ROUTE_ACOM_Send(gsSvcWork.hComCnx, p_msg) == FALSE)
			NTSVCInfo("AuthVT_ExlReq_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!");
	}
	else
		NTSVCInfo("AuthVT_ExlReq_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!");

	MSG_LC_AUTH_VT_EXL_REP_Delete_All(p_msg);

	return TRUE;
}


PRIVATE BOOL AuthVT_ExlReq_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP * p_msg;
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
	DWORD buffer_size;
	DWORD dwErr = NO_ERROR;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReq_OperFwdResp() : error MSG_LC_AUTH_VT_EXL_REP_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP));

	// conversion du message en "ASCII"
	buffer_size = MSG_Write(p_msg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVC_ERR("AuthVT_ExlReq_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_EXL_REP_Delete_All(p_msg);
		return FALSE;
	}

	dwErr = AComSendMessage(gsSvcWork.hAuthWks,
		hCnxHandle,
		(ULONG)gsSvcWork.hAuthWks,
		buffer_size,
		buffer);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("AuthVT_ExlReq_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr);
		MSG_LC_AUTH_VT_EXL_REP_Delete_All(p_msg);
		return FALSE;
	}

	MSG_LC_AUTH_VT_EXL_REP_Delete_All(p_msg);

	return TRUE;
}


//---------------------------------------------------------------------------------
// V2 messages ----
//---------------------------------------------------------------------------------


PROTECTED void Auth_VT_EnlReq_V2_Oper(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V2 * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_ENL_REP_V2_New();
	//memcpy(&psAuthRsp->header, &psAuthReq->header, sizeof(struct MSG_HEADER));
	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999


	psAuthRsp->body.dte_ref_entry = psAuthReq->body.dte_ref_entry;
	psAuthRsp->body.entry_trs_ref_num = psAuthReq->body.entry_trs_ref_num;
	psAuthRsp->body.entry_plaza_id = psAuthReq->body.entry_plaza_id;
	psAuthRsp->body.entry_lane_id = psAuthReq->body.entry_lane_id;
	memcpy_s(psAuthRsp->body.entry_trs_id, sizeof(psAuthRsp->body.entry_trs_id), psAuthReq->body.entry_trs_id, sizeof(psAuthReq->body.entry_trs_id));
	psAuthRsp->body.dte_entry = psAuthReq->body.dte_entry;
	memcpy_s(psAuthRsp->body.entry_vrn, sizeof(psAuthRsp->body.entry_vrn), psAuthReq->body.entry_vrn, sizeof(psAuthReq->body.entry_vrn));
	memcpy_s(psAuthRsp->body.entry_vrn_country, sizeof(psAuthRsp->body.entry_vrn_country), psAuthReq->body.entry_vrn_country, sizeof(psAuthReq->body.entry_vrn_country));
	psAuthRsp->body.ticket_type = psAuthReq->body.ticket_type;

	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_EnlReqV2(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_EnlReqV2(psAuthReq, psAuthRsp);

	NTSVCInfo("Auth_VT_EnlReq_V2_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] ticket_type:[%u] entry_provider:[%s] payment_subtype:%u vehicle_class:%u accepted_days:[%s] flag_result:%u reason_id:%u",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.ticket_type,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.payment_subtype,
		psAuthRsp->body.vehicle_class,
		psAuthRsp->body.accepted_days,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id);

	if (gsSvcWork.sParmWork.dwIsAuthServer)
	{
		// Forward result to the authorization client
		AuthVT_EnlReqV2_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
		// Send response to lane
		AuthVT_EnlReqV2_OperSendResp(psAuthRsp);
	}

	MSG_LC_AUTH_VT_ENL_REP_V2_Delete_All(psAuthRsp);

}

PROTECTED void Auth_VT_ExlReq_V2_Oper(struct MSG_LC_AUTH_VT_EXL_REQ_V2 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V2 * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_EXL_REP_V2_New();

	//memcpy(&psAuthRsp->header, &psAuthReq->header, sizeof(struct MSG_HEADER));
	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999

	psAuthRsp->body.dte_ref_exit = psAuthReq->body.dte_ref_exit;
	psAuthRsp->body.exit_trs_ref_num = psAuthReq->body.exit_trs_ref_num;
	psAuthRsp->body.exit_plaza_id = psAuthReq->body.exit_plaza_id;
	psAuthRsp->body.exit_lane_id = psAuthReq->body.exit_lane_id;
	memcpy_s(psAuthRsp->body.exit_trs_id, sizeof(psAuthRsp->body.exit_trs_id), psAuthReq->body.exit_trs_id, sizeof(psAuthReq->body.exit_trs_id));
	psAuthRsp->body.dte_exit = psAuthReq->body.dte_exit;
	memcpy_s(psAuthRsp->body.exit_vrn, sizeof(psAuthRsp->body.exit_vrn), psAuthReq->body.exit_vrn, sizeof(psAuthReq->body.exit_vrn));
	memcpy_s(psAuthRsp->body.exit_vrn_country, sizeof(psAuthRsp->body.exit_vrn_country), psAuthReq->body.exit_vrn_country, sizeof(psAuthReq->body.exit_vrn_country));
	psAuthRsp->body.ticket_type = psAuthReq->body.ticket_type;
	psAuthRsp->body.flag_eticket = psAuthReq->body.flag_eticket;

	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_ExlReqV2(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_ExlReqV2(psAuthReq, psAuthRsp);

	NTSVCInfo("Auth_VT_ExlReq_V2_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] ticket_type[%u] entry_provider:[%s] accepted_days[%s] vehicle_class:%u payment_subtype:%u exempt_acccount_number:%u exempt_user_number:%u exempt_renewal_number:%u flag_result:%u reason_id:%u",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.ticket_type,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.accepted_days,
		psAuthRsp->body.vehicle_class,
		psAuthRsp->body.payment_subtype,
		psAuthRsp->body.exempt_acccount_number,
		psAuthRsp->body.exempt_user_number,
		psAuthRsp->body.exempt_renewal_number,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id);

	if (gsSvcWork.sParmWork.dwIsAuthServer)
	{
		// Forward result to the authorization client
		AuthVT_ExlReqV2_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
		// Send response to lane
		AuthVT_ExlReqV2_OperSendResp(psAuthRsp);
	}

	MSG_LC_AUTH_VT_EXL_REP_V2_Delete_All(psAuthRsp);

}



PRIVATE BOOL AuthVT_EnlReqV2_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthResp)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V2 * p_msg;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_V2_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_EnlReqV2_OperSendResp() : error MSG_LC_AUTH_VT_ENL_REP_V2_New !!");

		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V2));

	if (gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0)
	{
		if (ROUTE_ACOM_Send(gsSvcWork.hComCnx, p_msg) == FALSE)
			NTSVCInfo("AuthVT_EnlReqV2_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!");
	}
	else
		NTSVCInfo("AuthVT_EnlReqV2_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!");

	MSG_LC_AUTH_VT_ENL_REP_V2_Delete_All(p_msg);

	return TRUE;
}

PRIVATE BOOL AuthVT_EnlReqV2_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V2 * p_msg;
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
	DWORD buffer_size;
	DWORD dwErr = NO_ERROR;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_V2_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_EnlReqV2_OperFwdResp() : error MSG_LC_AUTH_VT_ENL_REP_V2_New !!");

		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP));

	// conversion du message en "ASCII"
	buffer_size = MSG_Write(p_msg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVC_ERR("AuthVT_EnlReqV2_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_ENL_REP_V2_Delete_All(p_msg);

		return FALSE;
	}

	dwErr = AComSendMessage(gsSvcWork.hAuthWks,
		hCnxHandle,
		(ULONG)gsSvcWork.hAuthWks,
		buffer_size,
		buffer);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("AuthVT_EnlReqV2_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr);
		MSG_LC_AUTH_VT_ENL_REP_V2_Delete_All(p_msg);
		return FALSE;
	}

	MSG_LC_AUTH_VT_ENL_REP_V2_Delete_All(p_msg);

	return TRUE;
}


PRIVATE BOOL AuthVT_ExlReqV2_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthResp)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V2 * p_msg;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_V2_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReqV2_OperSendResp() : error MSG_LC_AUTH_VT_EXL_REP_V2_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP_V2));

	if (gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0)
	{
		if (ROUTE_ACOM_Send(gsSvcWork.hComCnx, p_msg) == FALSE)
			NTSVCInfo("AuthVT_ExlReqV2_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!");
	}
	else
		NTSVCInfo("AuthVT_ExlReqV2_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!");

	MSG_LC_AUTH_VT_EXL_REP_V2_Delete_All(p_msg);

	return TRUE;
}


PRIVATE BOOL AuthVT_ExlReqV2_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V2 * p_msg;
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
	DWORD buffer_size;
	DWORD dwErr = NO_ERROR;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_V2_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReqV2_OperFwdResp() : error MSG_LC_AUTH_VT_EXL_REP_V2_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP_V2));

	// conversion du message en "ASCII"
	buffer_size = MSG_Write(p_msg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVC_ERR("AuthVT_ExlReqV2_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_EXL_REP_V2_Delete_All(p_msg);
		return FALSE;
	}

	dwErr = AComSendMessage(gsSvcWork.hAuthWks,
		hCnxHandle,
		(ULONG)gsSvcWork.hAuthWks,
		buffer_size,
		buffer);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("AuthVT_ExlReqV2_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr);
		MSG_LC_AUTH_VT_EXL_REP_V2_Delete_All(p_msg);
		return FALSE;
	}

	MSG_LC_AUTH_VT_EXL_REP_V2_Delete_All(p_msg);

	return TRUE;
}


//---------------------------------------------------------------------------------
// V3 messages ----
//---------------------------------------------------------------------------------


PROTECTED void Auth_VT_EnlReq_V3_Oper(struct MSG_LC_AUTH_VT_ENL_REQ_V3 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V3 * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_ENL_REP_V3_New();
	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999


	psAuthRsp->body.dte_ref_entry = psAuthReq->body.dte_ref_entry;
	psAuthRsp->body.entry_trs_ref_num = psAuthReq->body.entry_trs_ref_num;
	psAuthRsp->body.entry_plaza_id = psAuthReq->body.entry_plaza_id;
	psAuthRsp->body.entry_lane_id = psAuthReq->body.entry_lane_id;
	memcpy_s(psAuthRsp->body.entry_trs_id, sizeof(psAuthRsp->body.entry_trs_id), psAuthReq->body.entry_trs_id, sizeof(psAuthReq->body.entry_trs_id));
	psAuthRsp->body.dte_entry = psAuthReq->body.dte_entry;
	memcpy_s(psAuthRsp->body.entry_vrn, sizeof(psAuthRsp->body.entry_vrn), psAuthReq->body.entry_vrn, sizeof(psAuthReq->body.entry_vrn));
	memcpy_s(psAuthRsp->body.entry_vrn_country, sizeof(psAuthRsp->body.entry_vrn_country), psAuthReq->body.entry_vrn_country, sizeof(psAuthReq->body.entry_vrn_country));
	psAuthRsp->body.ticket_type = psAuthReq->body.ticket_type;
	psAuthRsp->body.flag_delete = psAuthReq->body.flag_delete;
	psAuthRsp->body.flag_no_insert = psAuthReq->body.flag_no_insert;
	psAuthRsp->body.request_id = psAuthReq->body.request_id;

	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_EnlReqV3(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_EnlReqV3(psAuthReq, psAuthRsp);


	NTSVCInfo("Auth_VT_EnlReq_V3_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] ticket_type:[%u] flag_delete:[%u] flag_no_insert:[%u]  request_id:[%u]  entry_provider:[%s] payment_subtype:%u vehicle_class:%u accepted_days:[%s] flag_result:%u reason_id:%u account_balance:[%s] user_balance:[%s] contract_type:%u",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.ticket_type,
		psAuthRsp->body.flag_delete,
		psAuthRsp->body.flag_no_insert,
		psAuthRsp->body.request_id,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.payment_subtype,
		psAuthRsp->body.vehicle_class,
		psAuthRsp->body.accepted_days,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id,
		psAuthRsp->body.account_balance,
		psAuthRsp->body.user_balance,
		psAuthRsp->body.contract_type);

	if (gsSvcWork.sParmWork.dwIsAuthServer)
	{
		// Forward result to the authorization client
		AuthVT_EnlReqV3_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
		// Send response to lane
		AuthVT_EnlReqV3_OperSendResp(psAuthRsp);
	}

	MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(psAuthRsp);

}

PROTECTED void Auth_VT_ExlReq_V3_Oper(struct MSG_LC_AUTH_VT_EXL_REQ_V3 *psAuthReq, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V3 * psAuthRsp;
	psAuthRsp = MSG_LC_AUTH_VT_EXL_REP_V3_New();

	psAuthRsp->header.id = psAuthReq->header.id;
	psAuthRsp->header.plaza_number = psAuthReq->header.plaza_number;       // 0-9 999
	psAuthRsp->header.lane_number = psAuthReq->header.lane_number;       // 0-9 999

	psAuthRsp->body.dte_ref_exit = psAuthReq->body.dte_ref_exit;
	psAuthRsp->body.exit_trs_ref_num = psAuthReq->body.exit_trs_ref_num;
	psAuthRsp->body.exit_plaza_id = psAuthReq->body.exit_plaza_id;
	psAuthRsp->body.exit_lane_id = psAuthReq->body.exit_lane_id;
	memcpy_s(psAuthRsp->body.exit_trs_id, sizeof(psAuthRsp->body.exit_trs_id), psAuthReq->body.exit_trs_id, sizeof(psAuthReq->body.exit_trs_id));
	psAuthRsp->body.dte_exit = psAuthReq->body.dte_exit;
	memcpy_s(psAuthRsp->body.exit_vrn, sizeof(psAuthRsp->body.exit_vrn), psAuthReq->body.exit_vrn, sizeof(psAuthReq->body.exit_vrn));
	memcpy_s(psAuthRsp->body.exit_vrn_country, sizeof(psAuthRsp->body.exit_vrn_country), psAuthReq->body.exit_vrn_country, sizeof(psAuthReq->body.exit_vrn_country));
	psAuthRsp->body.ticket_type = psAuthReq->body.ticket_type;
	psAuthRsp->body.flag_eticket = psAuthReq->body.flag_eticket;
	psAuthRsp->body.flag_delete = psAuthReq->body.flag_delete;
	psAuthRsp->body.flag_open_system = psAuthReq->body.flag_open_system;
	psAuthRsp->body.flag_no_update = psAuthReq->body.flag_no_update;
	psAuthRsp->body.request_id = psAuthReq->body.request_id;

	//flag_result and reason_id are updated from the database procedure call

	//// Insert data into database
	if (DBIfShouldRetry(DBAuthVT_ExlReqV3(psAuthReq, psAuthRsp)) == TRUE)
		DBAuthVT_ExlReqV3(psAuthReq, psAuthRsp);

	NTSVCInfo("Auth_VT_ExlReq_V3_Oper(), Authorization response for plaza %d, lane %d, entry_plaza_id:%d entry_lane_id:%d entry_trs_id:[%s] entry_vrn:[%s] entry_vrn_country:[%s] ticket_type[%u] flag_delete[%u] flag_open_system[%u] flag_no_update[%u] request_id[%u] entry_provider:[%s] accepted_days[%s] vehicle_class:%u payment_subtype:%u exempt_acccount_number:%u exempt_user_number:%u exempt_renewal_number:%u flag_result:%u reason_id:%u account_balance:[%s] user_balance:[%s] contract_type:%u  discount_group:[%s]",
		psAuthRsp->header.plaza_number,
		psAuthRsp->header.lane_number,
		psAuthRsp->body.entry_plaza_id,
		psAuthRsp->body.entry_lane_id,
		psAuthRsp->body.entry_trs_id,
		psAuthRsp->body.entry_vrn,
		psAuthRsp->body.entry_vrn_country,
		psAuthRsp->body.ticket_type,
		psAuthRsp->body.flag_delete,
		psAuthRsp->body.flag_open_system,
		psAuthRsp->body.flag_no_update,
		psAuthRsp->body.request_id,
		psAuthRsp->body.entry_provider,
		psAuthRsp->body.accepted_days,
		psAuthRsp->body.vehicle_class,
		psAuthRsp->body.payment_subtype,
		psAuthRsp->body.exempt_acccount_number,
		psAuthRsp->body.exempt_user_number,
		psAuthRsp->body.exempt_renewal_number,
		psAuthRsp->body.flag_result,
		psAuthRsp->body.reason_id,
		psAuthRsp->body.account_balance,
		psAuthRsp->body.user_balance,
		psAuthRsp->body.contract_type,
		psAuthRsp->body.discount_group);

	if (gsSvcWork.sParmWork.dwIsAuthServer)
	{
		// Forward result to the authorization client
		AuthVT_ExlReqV3_OperFwdResp(psAuthRsp, hCnxHandle);
	}
	else
	{
		// Send response to lane
		AuthVT_ExlReqV3_OperSendResp(psAuthRsp);
	}

	MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(psAuthRsp);

}



PRIVATE BOOL AuthVT_EnlReqV3_OperSendResp(struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthResp)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V3 * p_msg;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_V3_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_EnlReqV3_OperSendResp() : error MSG_LC_AUTH_VT_ENL_REP_V3_New !!");

		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3));

	if (gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0)
	{
		if (ROUTE_ACOM_Send(gsSvcWork.hComCnx, p_msg) == FALSE)
			NTSVCInfo("AuthVT_EnlReqV3_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!");
	}
	else
		NTSVCInfo("AuthVT_EnlReqV3_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!");

	MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(p_msg);

	return TRUE;
}

PRIVATE BOOL AuthVT_EnlReqV3_OperFwdResp(struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_ENL_REP_V3 * p_msg;
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
	DWORD buffer_size;
	DWORD dwErr = NO_ERROR;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_ENL_REP_V3_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_EnlReqV3_OperFwdResp() : error MSG_LC_AUTH_VT_ENL_REP_V3_New !!");

		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3));

	// conversion du message en "ASCII"
	buffer_size = MSG_Write(p_msg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVC_ERR("AuthVT_EnlReqV3_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(p_msg);

		return FALSE;
	}

	dwErr = AComSendMessage(gsSvcWork.hAuthWks,
		hCnxHandle,
		(ULONG)gsSvcWork.hAuthWks,
		buffer_size,
		buffer);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("AuthVT_EnlReqV3_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr);
		MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(p_msg);
		return FALSE;
	}

	MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(p_msg);

	return TRUE;
}


PRIVATE BOOL AuthVT_ExlReqV3_OperSendResp(struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthResp)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V3 * p_msg;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_V3_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReqV3_OperSendResp() : error MSG_LC_AUTH_VT_EXL_REP_V3_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP_V3));

	if (gsSvcWork.hComCnx != (ACOM_CNX_HANDLE)0)
	{
		if (ROUTE_ACOM_Send(gsSvcWork.hComCnx, p_msg) == FALSE)
			NTSVCInfo("AuthVT_ExlReqV3_OperSendResp: ROUTE_ACOM_Send() : Error sending message !!");
	}
	else
		NTSVCInfo("AuthVT_ExlReqV3_OperSendResp: ROUTE_ACOM_Send() : Error not connected !!");

	MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(p_msg);

	return TRUE;
}


PRIVATE BOOL AuthVT_ExlReqV3_OperFwdResp(struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthResp, ACOM_CNX_HANDLE hCnxHandle)
{
	struct MSG_LC_AUTH_VT_EXL_REP_V3 * p_msg;
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE];
	DWORD buffer_size;
	DWORD dwErr = NO_ERROR;

	// Création d'un message de service
	p_msg = MSG_LC_AUTH_VT_EXL_REP_V3_New();
	if (p_msg == NULL)
	{
		NTSVC_ERR("AuthVT_ExlReqV3_OperFwdResp() : error MSG_LC_AUTH_VT_EXL_REP_V3_New !!");
		return FALSE;
	}

	memcpy(p_msg, psAuthResp, sizeof(struct MSG_LC_AUTH_VT_EXL_REP_V3));

	// conversion du message en "ASCII"
	buffer_size = MSG_Write(p_msg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVC_ERR("AuthVT_ExlReqV3_OperFwdResp() : error MSG_Write !!");
		MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(p_msg);
		return FALSE;
	}

	dwErr = AComSendMessage(gsSvcWork.hAuthWks,
		hCnxHandle,
		(ULONG)gsSvcWork.hAuthWks,
		buffer_size,
		buffer);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("AuthVT_ExlReqV3_OperFwdResp(), Forwarding of message to the authorization client failed, error %d", dwErr);
		MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(p_msg);
		return FALSE;
	}

	MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(p_msg);

	return TRUE;
}