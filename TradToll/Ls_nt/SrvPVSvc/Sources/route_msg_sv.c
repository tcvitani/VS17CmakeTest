/* --------------------------------------------------------------------
* (C) 1999 CS Route - All rights reserved
* --------------------------------------------------------------------
* MODULE     : SRVPVSVC
* FILE       : msg_route.c
* LANGAGE    : C
* --------------------------------------------------------------------
* KEYWORDS   : 
* --------------------------------------------------------------------
* SUMMARY    : 
* --------------------------------------------------------------------
* DESCRIPTION: 
* --------------------------------------------------------------------
*/

#include <route_acom.h>
#include <srvpv_main.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_con_req.h>
#include <msg_sv_filt_dec.h>
#include <msg_lc_status.h>
#include <msg_lc_event.h>
#include <msg_lc_transaction.h>
#include <msg_lc_bowl_stat.h>
#include <msg_lc_vault_stat.h>
#include <msg_lc_counter_status.h>
#include <msg_lc_partly.h>
#include <msg_lc_coin_listing.h>
#include <msg_lc_comp_inf_tr.h>
#include <msg_lc_trace.h>

// --------------- INTERNALS -----------------

#include <memclass.h>

// Valeurs jockers pour les ID et CD

// --------------- CODE ----------------------

PROTECTED BOOL ROUTE_Send_msg_sv_con_req (ACOM_CNX_HANDLE hCnxHandle)
{
    struct MSG_SV_CON_REQ *p_req;
    BOOL bRet = FALSE;
    
    // Création d'un message de service
    p_req = MSG_SV_CON_REQ_New ();
    if (p_req == NULL)
        return FALSE;
    
    // affectation du message
    memset(p_req, 0, sizeof (struct MSG_SV_CON_REQ));
    p_req->header.id = SVC_Get_Msg_Id ();
    
    // Envoi du message vers ROUTE sur le réseau
    bRet = ROUTE_ACOM_Send (hCnxHandle, p_req);
    
    // suppression du message
    bRet &= MSG_SV_CON_REQ_Delete_All (p_req);
    
    return bRet;
}

PROTECTED BOOL ROUTE_Send_msg_sv_filt_dec (ACOM_CNX_HANDLE hCnxHandle)
{
    struct MSG_SV_FILT_DEC *p_req;
    struct MSG_SV_FILT_DEC_Idcd *p_Idcd;
    BOOL bRet = FALSE;
    
    // 
    
    bRet = FALSE;
    do 
    {
        // Création d'un message de service
        p_req = MSG_SV_FILT_DEC_New ();
        if (p_req == NULL)
            break;
        
        // affectation du message
        memset(p_req, 0, sizeof (struct MSG_SV_CON_REQ));
        p_req->header.id = SVC_Get_Msg_Id ();
        
        // filtrage des message de connexion
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_SV_CON_REP_CD;
        
        // ne recevoir que les messages trs, status, event, bowl, vault des voies
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_TRANSACTION_CD;

		p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_COMP_INF_TR_CD;

        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_STATUS_CD;
        
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_EVENT_CD;
        
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_BOWL_STAT_CD;
        
        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_VAULT_STAT_CD;

        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_COUNTER_STATUS_CD;

		p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_PARTLY_CD;

        p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_COIN_LISTING_CD;

		p_Idcd = MSG_SV_FILT_DEC_Idcd_New (&p_req->list_idcd);
        if (p_Idcd == NULL)
            break;
        p_Idcd->id = MSG_SV_FILT_DEC_ID_ANY;
        p_Idcd->cd = MSG_TRACE_CD;
        
        // Envoi du message vers ROUTE sur le réseau
        if (ROUTE_ACOM_Send (hCnxHandle, p_req) == FALSE)
            break;

        bRet = TRUE;
    }
    while ( FALSE );
    
    // suppression du message
    if (p_req != NULL)
        bRet &= MSG_SV_FILT_DEC_Delete_All (p_req);
    
    return bRet;
}
