/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_trace.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de trace
* --------------------------------------------------------------------
* HISTORIQUE:
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <plaza.h>
#include <msg_lc_trace.h>
#include <msg_pv_trac_rep.h>
#include <srvpv_main.h>
#include <ntsvc.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/

PROTECTED struct MSG_PV_TRAC_REP *ROUTE_Process_Msg_Trace (struct MSG_TRACE *p_lc_trac)
{
    struct MSG_PV_TRAC_REP *p_rep;
    struct MSG_PV_TRAC_REP_Type *p_type;
    struct MSG_PV_TRAC_REP_Info *p_info;
    struct MSG_TRACE_Info_Type *p_lc_type;
    struct MSG_TRACE_Elem_Info *p_lc_info;
    BOOL bRet = TRUE;

    // allocation d'un message de trace
    p_rep = MSG_PV_TRAC_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("ROUTE_Process_Msg_Trace() => Error memory allocation: MSG_PV_TRAC_REP_New()!");

        return NULL;
    }

    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body.lanenum = p_lc_trac->header.lane_number;
    p_rep->body.date = p_lc_trac->header.time_of_message;
    
    p_lc_type = MSG_TRACE_Get_First_Info_Type (p_lc_trac->list_trace);
    while (p_lc_type != NULL)
    {
		// allocation d'un nouveau type
		if ((p_type = MSG_PV_TRAC_REP_Type_New (&p_rep->list_type)) == NULL)
		{
			NTSVC_ERR ("ROUTE_Process_Msg_Trace() => Error memory allocation: MSG_PV_TRAC_REP_Type_New()!");
    
			return NULL;
		}
		
		p_type->type = p_lc_type->type;

		p_lc_info = MSG_TRACE_Get_First_Elem_Info (p_lc_type->list_info_type);
		
		while (p_lc_info != NULL)
		{
			// allocation d'une nouvelle info
			if ((p_info = MSG_PV_TRAC_REP_Info_New (&p_type->list_info)) == NULL)
			{
				NTSVC_ERR ("ROUTE_Process_Msg_Trace() => Error memory allocation: MSG_PV_TRAC_REP_Info_New()!");
    
				return NULL;
			}

			p_info->code =  p_lc_info->code;

			memcpy(p_info->info,p_lc_info->info_value.String,MSG_PV_MAX_INFO_SIZE);
				
			// type d'info suivante
			p_lc_info = MSG_TRACE_Get_Next_Elem_Info (p_lc_type->list_info_type, p_lc_info);
		}

		p_lc_type = MSG_TRACE_Get_Next_Info_Type (p_lc_trac->list_trace, p_lc_type);
	}
    return p_rep;
}

