/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_status.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de status des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_status.c_v  $
 * 
 *    Rev 1.5   26 Apr 2002 11:44:56   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.3   13 Dec 2000 16:34:50   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:36   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <alarm.h>
#include <plaza.h>
#include <msg_lc_status.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/

PROTECTED BOOL ROUTE_Process_Msg_Status (struct MSG_STATUS *p_stat)
{
    struct LANE_CNF *p_lane;
    struct MSG_STATUS_Event_Information *p_event;
    BOOL bRet = TRUE;

    // recherche de la voie concernée par la transaction
    p_lane = PLAZA_Search_Lane (p_stat->header.lane_number);
    if (p_lane == NULL)
        return FALSE;

    // pour tous les Events du message de status
    p_event = MSG_STATUS_Get_First_Event_Information (p_stat->list_event_info);
    while (p_event != NULL)
    {
		// processing of status modifying the state of the lane: shift and mode
        bRet &= ALARM_Set_Lane_Shift_And_Mode (p_lane, 
                                               p_stat->header.toll_collector_id,
                                               p_stat->header.active_vault_id,
                                               p_event->event_category, 
                                               p_event->event_sub_category,
                                               p_event->current_event_status);

		// processing of status modifying the state of the lane: active vault
        bRet &= ALARM_Set_Lane_Active_Vault (p_lane, 
                                             p_stat->header.active_vault_id,
                                             p_event->event_category);
    
		// test if changed state of lane equipment
        bRet &= LANE_Search_And_Set_Eqt (p_lane, 
                                         p_event->event_category, 
                                         p_event->event_sub_category,
                                         p_event->current_event_status);

		// generate alarm
        bRet &= ALARM_Search_And_Set_List (p_lane, 
                                           p_event->event_category, 
                                           p_event->event_sub_category,
                                           p_event->current_event_status,
                                           &p_stat->body.time_of_event,
										   "");

        p_event = MSG_STATUS_Get_Next_Event_Information (p_stat->list_event_info, p_event);
    }

    return bRet;
}

