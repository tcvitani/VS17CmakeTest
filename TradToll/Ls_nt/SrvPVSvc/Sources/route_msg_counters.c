/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_counter_status.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message de transaction des voies
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_counters.c_v  $
 * 
 *    Rev 1.4   26 Apr 2002 11:44:54   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.2   19 Dec 2000 15:10:10   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.1   Dec 08 1999 16:35:32   pgg
 *  
 * 
 *    Rev 1.0   Nov 22 1999 11:00:36   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <plaza.h>
#include <msg_lc_counter_status.h>

/*--------------- EXTERNALS: ---------------*/

#include <memclass.h>

/*--------------- FUNCTIONS -----------------*/

PROTECTED BOOL ROUTE_Process_Msg_Counters (struct MSG_COUNTER_STATUS *p_counters)
{
    struct LANE_CNF *p_lane_cnf;
    BOOL bRet = TRUE;
    
    // recherche de la voie concernée par les compteurs
    p_lane_cnf = PLAZA_Search_Lane (p_counters->header.lane_number);
    if (p_lane_cnf == NULL)
        return FALSE;
    
    //
    // RAZ des compteurs de trafic des voies déconnectées du réseau
    //

    bRet &= PLAZA_Reset_Traffic_Counters_For_All_Disconnected_Lanes (p_lane_cnf, p_counters->body.reset);
    
	bRet &= LANE_Set_Traffic (p_lane_cnf,
                              p_counters->body.total_traffic, 
                              p_counters->body.total_violation);

    return bRet;
}
