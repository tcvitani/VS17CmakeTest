/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_partly.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de comptage partiel borne des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_partly.c_v  $
 * 
 *    Rev 1.4   26 Apr 2002 11:44:56   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.2   13 Dec 2000 16:34:50   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:38   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <plaza.h>
#include <msg_lc_partly.h>
#include <msg_pv_part_rep.h>
#include <srvpv_main.h>
#include <ntsvc.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/


PROTECTED struct MSG_PV_PART_REP *ROUTE_Process_Msg_Partly (struct MSG_PARTLY *p_lc_part)
{
    struct MSG_PV_PART_REP *p_rep;
    struct MSG_PV_PART_REP_Class *p_class;
    struct MSG_PARTLY_Detailed_Traffic *p_lc_detail;
    struct MSG_PARTLY_Traffic_Det_Payment *p_lc_pay;
    BOOL bRet = TRUE;

    // allocation d'un message de comptage partiel
    p_rep = MSG_PV_PART_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("ROUTE_Process_Msg_Partly() => Error memory allocation: MSG_PV_PART_REP_New()!");

        return NULL;
    }

    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body.lanenum = p_lc_part->header.lane_number;
    p_rep->body.date = p_lc_part->header.time_of_message;
    p_rep->body.collector_id = p_lc_part->header.toll_collector_id;
    p_rep->body.active_vault = p_lc_part->header.active_vault_id;
	p_rep->body.lower_date = p_lc_part->body.time_of_inferior_limit;
    p_rep->body.upper_date = p_lc_part->body.time_of_superior_limit;
	p_rep->body.total_traffic = p_lc_part->general_traffic.total_traffic;
    p_rep->body.total_violation = p_lc_part->general_traffic.total_violation;
	p_rep->body.total_maintenance = p_lc_part->general_traffic.maintenance_traffic;

    p_lc_detail = MSG_PARTLY_Get_First_Detailed_Traffic (p_lc_part->detailed_traffic);
    while (p_lc_detail != NULL)
    {
        // allocation d'une nouvelle classe
        if ((p_class = MSG_PV_PART_REP_Class_New (&p_rep->list_class)) == NULL)
        {
            NTSVC_ERR ("ROUTE_Process_Msg_Partly() => Error memory allocation: MSG_PV_PART_REP_Class_New()!");
            
            return NULL;
        }

        p_class->value = p_lc_detail->class_id;
	    
        // type de paiement et trafic

        p_lc_pay = MSG_PARTLY_Get_First_Traffic_Det_Payment (p_lc_detail->detailed_payment);
        while (p_lc_pay != NULL)
        {
            // on ajoute la valeur des pieces pour cette monnaie
            p_class->counter +=  p_lc_pay->traffic;
    
            // type de paiement suivant
            p_lc_pay = MSG_PARTLY_Get_Next_Traffic_Det_Payment (p_lc_detail->detailed_payment, p_lc_pay);
        }

        p_lc_detail = MSG_PARTLY_Get_Next_Detailed_Traffic (p_lc_part->detailed_traffic, p_lc_detail);
    }

    return p_rep;
}

