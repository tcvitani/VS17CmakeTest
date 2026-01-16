/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_vault.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de coffre des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_vault.c_v  $
 * 
 *    Rev 1.6   26 Apr 2002 11:44:58   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.4   19 Dec 2000 15:10:12   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.3   13 Dec 2000 16:34:50   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:38   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <plaza.h>
#include <msg_lc_vault_stat.h>
#include <msg_pv_vaul_rep.h>
#include <srvpv_main.h>
#include <ntsvc.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/

PRIVATE struct MSG_PV_VAUL_REP_Currency *Get_Currency (HLIST *list_currency,
                                                       CHAR currency_label[MSG_PV_MAX_CURRENCY_LABEL])
{
    struct MSG_PV_VAUL_REP_Currency *p_cur;

    // recherche si la monnaie existe deja dans la liste
    p_cur = MSG_PV_VAUL_REP_Get_First_Currency (*list_currency);
    while (p_cur != NULL)
    {
        if (strncmp (p_cur->label, currency_label, sizeof(p_cur->label)) == 0)
            break;
        
        p_cur = MSG_PV_VAUL_REP_Get_Next_Currency (*list_currency, p_cur);
    }
    
    // si pas trouvé, crée un item avec cette nouvelle monnaie
    if (p_cur == NULL)
    {
        p_cur = MSG_PV_VAUL_REP_Currency_New (list_currency);
        if (p_cur == NULL)
        {
            NTSVC_ERR ("Get_Currency() => Error memory allocation: MSG_PV_VAUL_REP_Currency_New()!");

            return NULL;
        }

        // init du label
        strzcpy (p_cur->label, MSG_PV_MAX_CURRENCY_LABEL, currency_label, sizeof(p_cur->label));
    }

    return p_cur;
}

PRIVATE struct MSG_PV_VAUL_REP_Coin *Get_Coin (HLIST *list_coin,
                                               CHAR coin_label[MSG_PV_MAX_COIN_LABEL])
{
    struct MSG_PV_VAUL_REP_Coin *p_coin;

    // recherche si la monnaie existe deja dans la liste
    p_coin = MSG_PV_VAUL_REP_Get_First_Coin (*list_coin);
    while (p_coin != NULL)
    {
        if (strncmp (p_coin->label, coin_label, sizeof(p_coin->label)) == 0)
            break;
        
        p_coin = MSG_PV_VAUL_REP_Get_Next_Coin (*list_coin, p_coin);
    }
    
    // si pas trouvé, crée un item avec cette nouvelle monnaie
    if (p_coin == NULL)
    {
        p_coin = MSG_PV_VAUL_REP_Coin_New (list_coin);
        if (p_coin == NULL)
        {
            NTSVC_ERR ("Get_Coin() => Error memory allocation: MSG_PV_VAUL_REP_Coin_New()!");

            return NULL;
        }

        // init du label
        strzcpy (p_coin->label, MSG_PV_MAX_COIN_LABEL, coin_label, sizeof(p_coin->label));
    }

    return p_coin;
}

PROTECTED struct MSG_PV_VAUL_REP *ROUTE_Process_Msg_Vault (struct MSG_VAULT_STAT *p_lc_stat)
{
    struct MSG_PV_VAUL_REP *p_rep;
    struct MSG_PV_VAUL_REP_Vault *p_vault;
    struct MSG_PV_VAUL_REP_Currency *p_cur;
    struct MSG_PV_VAUL_REP_Coin *p_coin;
    struct MSG_VAULT_STAT_Vault_Info *p_lc_info;
    struct MSG_VAULT_STAT_Coin_Info *p_lc_coin;
    CHAR currency_label[MSG_PV_MAX_CURRENCY_LABEL];
    CHAR coin_label[MSG_PV_MAX_COIN_LABEL];
    DOUBLE coin_value;
    DWORD bag_count;
    BOOL bRet = TRUE;

    // allocation d'un message d'état bol
    p_rep = MSG_PV_VAUL_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("ROUTE_Process_Msg_Vault() => Error memory allocation: MSG_PV_VAUL_REP_New()!");

        return NULL;
    }

    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body.lanenum = p_lc_stat->header.lane_number;
    p_rep->body.date = p_lc_stat->header.time_of_message;
    p_rep->body.collector_id = p_lc_stat->header.toll_collector_id;
    p_rep->body.active_vault = p_lc_stat->header.active_vault_id;
    p_rep->body.rejected_coin_counter = p_lc_stat->body.rejected_coin_counter;

    p_lc_info = MSG_VAULT_STAT_Get_First_Vault_Info (p_lc_stat->list_vaults_in_lane);
    while (p_lc_info != NULL)
    {
        // allocation d'un nouveau bol
        if ((p_vault = MSG_PV_VAUL_REP_Vault_New (&p_rep->list_vault)) == NULL)
        {
            NTSVC_ERR ("ROUTE_Process_Msg_Vault() => Error memory allocation: MSG_PV_VAUL_REP_Vault_New()!");
            
            return NULL;
        }

        p_vault->position = p_lc_info->position;
	    p_vault->percent = p_lc_info->fill_percent;
        p_vault->id = p_lc_info->id;
        
		if (p_lc_info->vault_state == 0)
			p_vault->state = (p_lc_info->id == p_lc_stat->header.active_vault_id);
        else
			p_vault->state = p_lc_info->vault_state;

		p_vault->type = p_lc_info->type;
		p_vault->rejected = p_lc_info->rejected_coins_since_insertion;
		p_vault->activated = p_lc_info->nb_of_activation_since_insertion;
        p_vault->date = p_lc_info->time_of_insert;

        // type de monnaie et pièces du coffre

        p_lc_coin = MSG_VAULT_STAT_Get_First_Coin_Info (p_lc_info->list_coin_info);
        while (p_lc_coin != NULL)
        {
            // on teste si le coffre contient des pièces avant de continuer
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
                    p_lc_coin = MSG_VAULT_STAT_Get_Next_Coin_Info (p_lc_info->list_coin_info, p_lc_coin);
                    continue;
                }
            
                // on recherche la monnaie concernant cette piece
                p_cur = Get_Currency (&p_vault->list_currency, currency_label);
            
                // on ajoute la valeur des pieces pour cette monnaie
                p_cur->value += coin_value * p_lc_coin->coin_counter;

                // on recherche la valeur faciale de cette piece dans cette monnaie
                p_coin = Get_Coin (&p_cur->list_coin, coin_label);
        
                // mise a jour du compteur de pièces du coffre
                p_coin->counter += p_lc_coin->coin_counter;
            }

            // pieces suivantes
            p_lc_coin = MSG_VAULT_STAT_Get_Next_Coin_Info (p_lc_info->list_coin_info, p_lc_coin);
        }

        p_lc_info = MSG_VAULT_STAT_Get_Next_Vault_Info (p_lc_stat->list_vaults_in_lane, p_lc_info);
    }

    return p_rep;
}

