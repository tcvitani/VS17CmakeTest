/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_bowl.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de bol des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_bowl.c_v  $
 * 
 *    Rev 1.5   26 Apr 2002 11:44:54   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.3   13 Dec 2000 16:34:48   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:36   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/


#include <plaza.h>
#include <msg_lc_bowl_stat.h>
#include <msg_pv_bowl_rep.h>
#include <srvpv_main.h>
#include <ntsvc.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/

PROTECTED struct MSG_PV_BOWL_REP *ROUTE_Process_Msg_Bowl (struct MSG_BOWL_STAT *p_stat)
{
    struct MSG_PV_BOWL_REP *p_rep;
    struct MSG_PV_BOWL_REP_Bowl *p_bowl;
    struct MSG_BOWL_STAT_Bowl_Information *p_info;
    BOOL bRet = TRUE;

    // allocation d'un message d'état bol
    p_rep = MSG_PV_BOWL_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("ROUTE_Process_Msg_Bowl() => Error memory allocation: MSG_PV_BOWL_REP_New()!");

        return NULL;
    }

    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body.lanenum = p_stat->header.lane_number;
    p_rep->body.date = p_stat->header.time_of_message;
    p_rep->body.collector_id = p_stat->header.toll_collector_id;
    p_rep->body.active_vault = p_stat->header.active_vault_id;

    p_info = MSG_BOWL_STAT_Get_First_Bowl_Information (p_stat->list_bowl_informations);
    while (p_info != NULL)
    {
        // allocation d'un nouveau bol
        if ((p_bowl = MSG_PV_BOWL_REP_Bowl_New (&p_rep->list_bowl)) == NULL)
        {
            NTSVC_ERR ("ROUTE_Process_Msg_Bowl() => Error memory allocation: MSG_PV_BOWL_REP_Bowl_New()!");
            
            return NULL;
        }

        p_bowl->position = p_info->bowl_position;
	    p_bowl->capacity = p_info->bowl_capacity;
        p_bowl->remaining_coin_counter = p_info->remaining_coin_counter;
        p_bowl->returned_coin_counter = p_info->returned_coin_counter;

        PLAZA_Get_Coin (p_info->coin_id,
                        p_bowl->currency_label,
	                    p_bowl->coin_label,
    	                &p_bowl->value,
	                    &p_bowl->bag);

        p_info = MSG_BOWL_STAT_Get_Next_Bowl_Information (p_stat->list_bowl_informations, p_info);
    }

    return p_rep;
}