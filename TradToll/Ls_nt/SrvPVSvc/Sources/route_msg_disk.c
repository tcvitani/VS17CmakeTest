/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_disk.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de disque des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_disk.c_v  $
 * 
 *    Rev 1.5   26 Apr 2002 11:44:54   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.3   19 Dec 2000 15:10:10   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.2   13 Dec 2000 16:34:48   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:38   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <plaza.h>
#include <msg_lc_coin_listing.h>
#include <msg_pv_disk_rep.h>
#include <srvpv_main.h>
#include <ntsvc.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/


PRIVATE struct MSG_PV_DISK_REP_Coin *Get_Coin (HLIST *list_coin,
                                               CHAR coin_label[MSG_PV_MAX_COIN_LABEL])
{
    struct MSG_PV_DISK_REP_Coin *p_coin;

    // recherche si la monnaie existe deja dans la liste
    p_coin = MSG_PV_DISK_REP_Get_First_Coin (*list_coin);
    while (p_coin != NULL)
    {
        if (strncmp (p_coin->label, coin_label, sizeof(p_coin->label)) == 0)
            break;
        
        p_coin = MSG_PV_DISK_REP_Get_Next_Coin (*list_coin, p_coin);
    }
    
    // si pas trouvé, crée un item avec cette nouvelle monnaie
    if (p_coin == NULL)
    {
        p_coin = MSG_PV_DISK_REP_Coin_New (list_coin);
        if (p_coin == NULL)
        {
            NTSVC_ERR ("Get_Coin() => Error memory allocation: MSG_PV_DISK_REP_Coin_New()!");

            return NULL;
        }

        // init du label
        strzcpy (p_coin->label, MSG_PV_MAX_COIN_LABEL, coin_label, sizeof(p_coin->label));
    }

    return p_coin;
}

PROTECTED struct MSG_PV_DISK_REP *ROUTE_Process_Msg_Disk (struct MSG_COIN_LISTING *p_lc_stat)
{
    struct MSG_PV_DISK_REP *p_rep;
    struct MSG_PV_DISK_REP_Coin *p_coin;
    struct MSG_COIN_LISTING_Coin_Information *p_lc_coin;
    CHAR currency_label[MSG_PV_MAX_CURRENCY_LABEL];
	CHAR coin_label[MSG_PV_MAX_COIN_LABEL];
    DOUBLE coin_value;
	DWORD bag_count;
    BOOL bRet = TRUE;

    // allocation d'un message d'état disk
    p_rep = MSG_PV_DISK_REP_New ();
    
	if (p_rep == NULL)
    {
        NTSVC_ERR ("ROUTE_Process_Msg_Disk() => Error memory allocation: MSG_PV_DISK_REP_New()!");

        return NULL;
    }

    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body.lanenum = p_lc_stat->header.lane_number;
    p_rep->body.date = p_lc_stat->header.time_of_message;
    p_rep->body.collector_id = p_lc_stat->header.toll_collector_id;
    p_rep->body.active_vault = p_lc_stat->header.active_vault_id;
    p_rep->body.rejected_coin_counter = p_lc_stat->body.rejected_coins;
	
	// recherche du libellé de la monnaie
    bRet &= PLAZA_Get_Currency (p_lc_stat->body.currency_id, p_rep->body.label);

	p_lc_coin = MSG_COIN_LISTING_Get_First_Coin_Information (p_lc_stat->list_coin_info);
    
	while (p_lc_coin != NULL)
        {
            // on teste si le disk contient des pièces avant de continuer
            if (p_lc_coin->coin_counter > 0)
            {
                // on recherche la piece courante dans la configuration
                if (PLAZA_Get_Coin (p_lc_coin->coin_id,
                                    currency_label,
	                                coin_label,
    	                            &coin_value,
	                                &bag_count) == FALSE)
                {
                    // si pas trouvé, on passe a la suivante
                    p_lc_coin = MSG_COIN_LISTING_Get_Next_Coin_Information (p_lc_stat->list_coin_info, p_lc_coin);
                    continue;
                }
            
              
                // on ajoute la valeur des pieces pour cette monnaie
                p_rep->body.total_cash += coin_value * p_lc_coin->coin_counter;

                // on recherche la valeur faciale de cette piece dans cette monnaie
                p_coin = Get_Coin (&p_rep->list_coin, coin_label);
        
                // mise a jour du compteur de pièces du coffre
                p_coin->counter += p_lc_coin->coin_counter;
            }

            // pieces suivantes
            p_lc_coin = MSG_COIN_LISTING_Get_Next_Coin_Information (p_lc_stat->list_coin_info, p_lc_coin);
        }

	p_rep->body.left_to_pay = p_lc_stat->body.toll_fare - p_rep->body.total_cash;

    return p_rep;
}

