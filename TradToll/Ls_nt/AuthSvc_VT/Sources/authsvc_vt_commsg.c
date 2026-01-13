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
#include <msg_sv_header.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>

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

#define LOC_DEF
#include <authsvc_vt_commsg.h>
#undef LOC_DEF

// --------------- INTERNALS -----------------

#include <memclass.h>

// --------------- CODE ----------------------

PROTECTED void ROUTE_Start (void)
{
    // Création de la liste des messages reconnu des VOIES
	MSG_LC_AUTH_VT_ENL_REQ_New_Record(&gsSvcWork.hlRouteMsgList);
	MSG_LC_AUTH_VT_EXL_REQ_New_Record(&gsSvcWork.hlRouteMsgList);
	MSG_LC_AUTH_VT_ENL_REQ_V2_New_Record(&gsSvcWork.hlRouteMsgList);
	MSG_LC_AUTH_VT_EXL_REQ_V2_New_Record(&gsSvcWork.hlRouteMsgList);
	MSG_LC_AUTH_VT_ENL_REQ_V3_New_Record(&gsSvcWork.hlRouteMsgList);
	MSG_LC_AUTH_VT_EXL_REQ_V3_New_Record(&gsSvcWork.hlRouteMsgList);

	// Add router messages into the list
	MSG_SV_CON_REP_New_Record( &gsSvcWork.hlRouteMsgList );
}

PROTECTED void ROUTE_Stop (void)
{
    // Destruction de la liste des messages
    MSG_Delete_All_Records  (&gsSvcWork.hlRouteMsgList);
}

PROTECTED BOOL ROUTE_Send_msg_sv_con_req (ACOM_CNX_HANDLE hCnxHandle)
{
    struct MSG_SV_CON_REQ *p_req;
    BOOL bRet;
    
    // Création d'un message de service
    p_req = MSG_SV_CON_REQ_New ();
    if (p_req == NULL)
        return FALSE;
    
    // affectation du message
    memset(p_req, 0, sizeof (struct MSG_SV_CON_REQ));
//    p_req->header.id = SVC_Get_Msg_Id ();
    
    // Envoi du message vers ROUTE sur le réseau
//    bRet = ROUTE_ACOM_Send (hCnxHandle, p_req);
    
    // suppression du message
    bRet &= MSG_SV_CON_REQ_Delete_All (p_req);
    
    return bRet;
}

PROTECTED BOOL ROUTE_Send_msg_sv_filt_dec ()
{
	DWORD dwMsgSize;
    struct MSG_SV_FILT_DEC *p_req;
    struct MSG_SV_FILT_DEC_Idcd *p_Idcd;
	BYTE tbBuffer[100];
    BOOL bRet;
    
    // Boucle do while() utilisée pour la commodité du break. En fait, la condition
    // de bouclage est FALSE, on ne passe donc qu'une fois dedans.
    bRet = FALSE;
    do 
    {
        // Création d'un message de service
        p_req = MSG_SV_FILT_DEC_New ();
        if (p_req == NULL)
            break;
        
        // affectation du message
        memset(p_req, 0, sizeof (struct MSG_SV_CON_REQ));
        p_req->header.id =gsSvcWork.sParmWork.dwAppMsgId;
        
        // filtrage des message de connexion
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_SV_CON_REP_CD;
        
		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_ENL_REQ_CD;

		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New(&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_EXL_REQ_CD;

		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New(&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_ENL_REQ_V2_CD;

		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New(&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_EXL_REQ_V2_CD;

		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New(&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_ENL_REQ_V3_CD;

		// receive authorization ENL message from the lane
		p_Idcd = MSG_SV_FILT_DEC_Idcd_New(&p_req->list_idcd);
		if (p_Idcd == NULL)
			break;
		p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
		p_Idcd->cd = MSG_LC_AUTH_VT_EXL_REQ_V3_CD;

        dwMsgSize = sizeof( tbBuffer );
        if ( ! MSG_SV_FILT_DEC_Write( p_req, tbBuffer, dwMsgSize, &dwMsgSize ) )
        {
            MSG_SV_FILT_DEC_Delete_All (p_req);
            break;
        }

        // Envoi du message vers ROUTE sur le réseau
        if (AComSendMessage( gsSvcWork.hWks, gsSvcWork.hComCnx, 0, dwMsgSize, tbBuffer ) != NO_ERROR)
            break;

        bRet = TRUE;
    }
    while ( FALSE );
    
    // suppression du message
    if (p_req != NULL)
        bRet &= MSG_SV_FILT_DEC_Delete_All (p_req);
    
    return bRet;
}

PROTECTED BOOL ROUTE_ACOM_Send (ACOM_CNX_HANDLE hCnxHandle,
                                HMSG hMsg)
{
	BYTE buffer[AUTHSVC_MSG_BUFFER_SIZE] = {0};
    DWORD buffer_size;

    // conversion du message en "ASCII"
    buffer_size = MSG_Write (hMsg, buffer, sizeof(buffer));
	if (buffer_size == 0UL)
	{
		NTSVCInfo("ROUTE_ACOM_Send(), Error MSG_Write!!");
		return FALSE;
	}

	NTSVCInfo( "ROUTE_ACOM_Send(), message %s", buffer );

    // Envoi du buffer vers un PV sur le réseau
    return ROUTE_ACOM_Send_Buffer (hCnxHandle,
                                   buffer_size,
                                   buffer );
}

PROTECTED BOOL ROUTE_ACOM_Send_Buffer (ACOM_CNX_HANDLE hCnxHandle,
                                       DWORD dwDataSize,
                                       void * pvData )
{
    CHAR *string = pvData;
    string[dwDataSize] = '\0';

    NTSVCInfo( "ROUTE_ACOM_Send_Buffer(%s)", pvData );

    return AComSendMessage (gsSvcWork.hWks, 
                            hCnxHandle,
                            (ULONG) gsSvcWork.hWks,
                            dwDataSize,
                            pvData ) == NO_ERROR;
}