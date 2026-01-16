/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DE ROUTE 
* FICHIER: srvroute_aut.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge de faire evoluer l'automate de ROUTE
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_aut.c_v  $
 * 
 *    Rev 1.9   26 Apr 2002 11:44:54   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.7   21 Dec 2001 13:36:14   DSILBERM
 * Mise a jour des nouvelles dll
 * 
 *    Rev 1.6   26 Jan 2001 13:48:54   DSI
 * Ajout du message complémentaire de
 * transaction pour mise à jour temps réel
 * des classes + mise à niveau des dll
 * linkées
 * 
 *    Rev 1.5   19 Dec 2000 15:10:10   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.4   13 Dec 2000 16:34:48   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.1   Dec 08 1999 16:35:32   pgg
 *  
 * 
 *    Rev 1.0   Nov 22 1999 11:00:36   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include "route_aut.h"
#include <acom.h>
#include <ntsvc.h>
#include <plaza.h>
#include <route_msg.h>
#include <route_acom.h>
#include <pv_wm.h>
#include <srvpv_main.h>
#include <msg_sv_con_rep.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_com_req.h>
#include <msg_lc_header.h>
#include <msg_lc_command.h>
#include <msg_lc_comp_inf_tr.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

#define ROUTE_AUT_REG_VAL_MULTI_PLAZA      "MultiPlaza"
#define ROUTE_AUT_REG_VAL_MULTI_PLAZA_COEF "MultiPlazaCoef"

/*--------------- TYPEDEFS: ----------------*/

typedef enum
{
    ROUTE_AUT_ETAT_0,
    ROUTE_AUT_MAX_ETATS
} 
ROUTE_AUT_ETATS;

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

//#pragma option -d-

/* liste des evenements reconnus en entree */
PROTECTED aut_event EVT_ROUTE_CONNECTION          = AUT_EVT_INIT("CONNECTION", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_DISCONNECTION       = AUT_EVT_INIT("DISCONNECTION", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_LANE_CONNECTION     = AUT_EVT_INIT("LANE CONNECTION", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_TRANSACTION     = AUT_EVT_INIT("MSG TRANSACTION", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_INFORMATION     = AUT_EVT_INIT("MSG INFORMATION", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_STATUS          = AUT_EVT_INIT("MSG STATUS", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_EVENT           = AUT_EVT_INIT("MSG EVENT", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_BOWL            = AUT_EVT_INIT("MSG BOWL", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_VAULT           = AUT_EVT_INIT("MSG VAULT", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_PARTLY          = AUT_EVT_INIT("MSG PARTLY", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_DISK	          = AUT_EVT_INIT("MSG DISK", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_COUNTERS        = AUT_EVT_INIT("MSG COUNTERS", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_COMMAND             = AUT_EVT_INIT("COMMAND", AUT_ARGS);
PROTECTED aut_event EVT_ROUTE_MSG_TRACE           = AUT_EVT_INIT("MSG TRACE", AUT_ARGS);

//#pragma option -d

typedef struct
{
    aut_automate_id automate;
    ACOM_CNX_HANDLE hCnxHandle;
} 
ROUTE_AUT;

PRIVATE struct 
{
    IN BOOL bMulti_Plaza;
	IN DWORD bMulti_Plaza_Coef;
}
ROUTE_AUT_PARAMS = {0};

/*--------------- FUNCTIONS: ---------------*/

/****************************/
/* FONCTIONS DE TRANSITIONS */
/****************************/

PRIVATE BOOL Send_msg_lc_status_req (ROUTE_AUT *p_route, DWORD lane_number)
{
    struct MSG_COMMAND * p_req;
    
    // Création d'un message de service
    p_req = MSG_COMMAND_New ();
    if (p_req == NULL)
    {
        NTSVC_ERR ("Send_msg_lc_status_req() => Error memory allocation: MSG_COMMAND_New()!!");
        
        return FALSE;
    }
    
    // affectation du message
    p_req->header.id = SVC_Get_Msg_Id ();
    
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
	{
		p_req->header.plaza_number = lane_number/ROUTE_AUT_PARAMS.bMulti_Plaza_Coef;
		p_req->header.lane_number = lane_number%ROUTE_AUT_PARAMS.bMulti_Plaza_Coef;
	}
	else
	{
		p_req->header.plaza_number = PLAZA_Get_Number();
		// si lane_number vaut 0 alors broadcast a toutes les voies connectées
		p_req->header.lane_number = lane_number;
	}

    // cmd_type de status request
    p_req->body.cmd_type = MSG_COMMAND_LANE_STATUS;
    
    // Envoi vers la voie sur le réseau
    ROUTE_ACOM_Send (p_route->hCnxHandle, p_req);
    
    // suppression du message
    MSG_COMMAND_Delete_All (p_req);

    return TRUE;
}

PRIVATE BOOL ACOM_Connection (ROUTE_AUT *p_route, ACOM_CNX_HANDLE *hCnxHandle)
{
    BOOL bRet = FALSE;

	NTSVCInfo("ACOM_Connection()");

    // on récupère le handle de connexion
    p_route->hCnxHandle = *hCnxHandle;

    // Mettre en place le filtre laissant passer les infos de connexion
    // et de déconnexion, et les messages des voies...
    bRet = ROUTE_Send_msg_sv_filt_dec (p_route->hCnxHandle);
    if (bRet == FALSE)
    {
        NTSVCInfo( "ROUTE_Connection() => impossible to establish filter");
        ROUTE_ACOM_Disconnect ( p_route->hCnxHandle );

        return FALSE;
    }

    // Interroger RouteSvc sur les voies connectées.
    bRet = ROUTE_Send_msg_sv_con_req (p_route->hCnxHandle);
    if (bRet == FALSE)
    {
        NTSVCInfo( "ROUTE_Connection() => impossible to get the list of connected lanes");
        ROUTE_ACOM_Disconnect ( p_route->hCnxHandle );

        return FALSE;
    }

    // envoyer une demande de status a toutes les voies...
    bRet = Send_msg_lc_status_req (p_route, MSG_HEADER_LANE_BROADCAST);
    if (bRet == FALSE)
    {
        NTSVCInfo( "ROUTE_Connection() => impossible to get the status of connected lanes");
        ROUTE_ACOM_Disconnect ( p_route->hCnxHandle );

        return FALSE;
    }

    return bRet;
}

PRIVATE BOOL ACOM_Disconnection (ROUTE_AUT *p_route, ACOM_CNX_HANDLE *hCnxHandle)
{
    // On force l'état de toutes les voies à "déconnecté"
    PLAZA_Disconnect_All_Lanes (*hCnxHandle);

    // On prévient les PV de la déconnection
    PV_PostMessage (PV_WM_SERVER_COM_FAILURE, NULL);

    return TRUE;
}

PRIVATE BOOL Recv_msg_sv_con_rep (ROUTE_AUT *p_route, struct MSG_SV_CON_REP *p_rep)
{
    NTSVCInfo( "Recv_msg_sv_con_rep()" );

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->body.lanenum = p_rep->body.plazanum * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->body.lanenum;

    PLAZA_Set_Lane_Connexion_State (p_rep, p_route->hCnxHandle);

    // envoyer une demande de status a la voie
    Send_msg_lc_status_req (p_route, p_rep->body.lanenum);

    return TRUE;
}

PRIVATE BOOL Recv_msg_lc_transaction (ROUTE_AUT *p_route, struct MSG_TRANSACTION *p_rep)
{
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    return ROUTE_Process_Msg_Transaction (p_rep);
}

PRIVATE BOOL Recv_msg_lc_status (ROUTE_AUT *p_route, struct MSG_STATUS *p_rep)
{
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    return ROUTE_Process_Msg_Status (p_rep);
}

PRIVATE BOOL Recv_msg_lc_event (ROUTE_AUT *p_route, struct MSG_EVENT *p_rep)
{
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    return ROUTE_Process_Msg_Event (p_rep);
}

PRIVATE BOOL Recv_msg_lc_information (ROUTE_AUT *p_route, struct MSG_COMP_INF_TR *p_rep)
{
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    return ROUTE_Process_Msg_Information (p_rep);
}

PRIVATE BOOL Recv_msg_lc_bowl (ROUTE_AUT *p_route, struct MSG_BOWL_STAT *p_rep)
{
    struct MSG_PV_BOWL_REP *p_bowl;

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    p_bowl = ROUTE_Process_Msg_Bowl (p_rep);
    if (p_bowl == NULL)
        return FALSE;

    return PV_PostMessage (PV_WM_BOWL_STATUS, p_bowl);
}

PRIVATE BOOL Recv_msg_lc_vault (ROUTE_AUT *p_route, struct MSG_VAULT_STAT *p_rep)
{
    struct MSG_PV_VAUL_REP *p_vault;

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    p_vault = ROUTE_Process_Msg_Vault (p_rep);
    if (p_vault == NULL)
        return FALSE;

    return PV_PostMessage (PV_WM_VAULT_STATUS, p_vault);
}

PRIVATE BOOL Recv_msg_lc_partly (ROUTE_AUT *p_route, struct MSG_PARTLY *p_rep)
{
    struct MSG_PV_PART_REP *p_part;

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    p_part = ROUTE_Process_Msg_Partly (p_rep);
    if (p_part == NULL)
        return FALSE;

    return PV_PostMessage (PV_WM_PARTLY, p_part);
}

PRIVATE BOOL Recv_msg_lc_disk (ROUTE_AUT *p_route, struct MSG_COIN_LISTING *p_rep)
{
    struct MSG_PV_DISK_REP *p_disk;

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;
    
	p_disk = ROUTE_Process_Msg_Disk (p_rep);
    if (p_disk == NULL)
        return FALSE;

    return PV_PostMessage (PV_WM_DISK, p_disk);
}

PRIVATE BOOL Recv_msg_lc_trace (ROUTE_AUT *p_route, struct MSG_TRACE *p_rep)
{
    struct MSG_PV_TRAC_REP *p_trac;

	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;
	
    p_trac = ROUTE_Process_Msg_Trace (p_rep);
    if (p_trac == NULL)
        return FALSE;
	
    return PV_PostMessage (PV_WM_TRACE, p_trac);
}

PRIVATE BOOL Recv_msg_lc_counters (ROUTE_AUT *p_route, struct MSG_COUNTER_STATUS *p_rep)
{
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
		p_rep->header.lane_number = p_rep->header.plaza_number * ROUTE_AUT_PARAMS.bMulti_Plaza_Coef + p_rep->header.lane_number;

    return ROUTE_Process_Msg_Counters (p_rep);
}

PRIVATE BOOL Recv_msg_pv_com_req (ROUTE_AUT *p_route, EVT_ROUTE_COMMAND_ARGS *p_args)
{
    struct MSG_HEADER	*p_header					= NULL;
	BYTE				msg[SVC_MSG_BUFFER_SIZE]	= {0};
    DWORD				msg_size					= 0;
	DWORD				buffer_size					= 0;

    // créer un header
    p_header = MSG_HEADER_New();
    if (p_header == NULL)
        return FALSE; 

    // le renseigner avec les infos connues...
    memset (p_header, 0, sizeof (struct MSG_HEADER));
    p_header->id = p_args->p_req->body.id;
    p_header->cd = p_args->p_req->body.cd;
	
	if (ROUTE_AUT_PARAMS.bMulti_Plaza)
	{
		p_header->plaza_number = p_args->p_lane->lanenum/ROUTE_AUT_PARAMS.bMulti_Plaza_Coef;
		p_header->lane_number = p_args->p_lane->lanenum%ROUTE_AUT_PARAMS.bMulti_Plaza_Coef;
	}
	else
	{
		p_header->plaza_number = PLAZA_Get_Number();
		p_header->lane_number = p_args->p_lane->lanenum;
    }
	
	SVC_Get_Time (&p_header->time_of_message);

    // le convertir en un buffer ASCII
    MSG_HEADER_Write (p_header, msg, sizeof(msg), &msg_size);

    // y accoler le buffer de commande
    memcpy(&msg[msg_size], p_args->p_req->body.command, p_args->p_req->body.size_of_command);
	buffer_size = msg_size + p_args->p_req->body.size_of_command;
	msg[buffer_size] = 0;

    // envoyer le tout
	ROUTE_ACOM_Send_Buffer(p_route->hCnxHandle, buffer_size, msg);

    // suppression du header
    MSG_HEADER_Delete_All(p_header);

    return TRUE;
}

/*--------------- VARIABLES INITIALISEES: ---------------*/

PRIVATE aut_etat ETAT_0 = {
    { EVT_ROUTE_CONNECTION, ROUTE_AUT_ETAT_0, ACOM_Connection },
    { EVT_ROUTE_MSG_TRANSACTION, ROUTE_AUT_ETAT_0, Recv_msg_lc_transaction },
    { EVT_ROUTE_MSG_INFORMATION, ROUTE_AUT_ETAT_0, Recv_msg_lc_information },
    { EVT_ROUTE_MSG_STATUS, ROUTE_AUT_ETAT_0, Recv_msg_lc_status },
    { EVT_ROUTE_MSG_EVENT, ROUTE_AUT_ETAT_0, Recv_msg_lc_event },
    { EVT_ROUTE_MSG_BOWL, ROUTE_AUT_ETAT_0, Recv_msg_lc_bowl },
    { EVT_ROUTE_MSG_VAULT, ROUTE_AUT_ETAT_0, Recv_msg_lc_vault },
    { EVT_ROUTE_MSG_PARTLY, ROUTE_AUT_ETAT_0, Recv_msg_lc_partly },
    { EVT_ROUTE_MSG_DISK, ROUTE_AUT_ETAT_0, Recv_msg_lc_disk },
    { EVT_ROUTE_MSG_COUNTERS, ROUTE_AUT_ETAT_0, Recv_msg_lc_counters },
    { EVT_ROUTE_DISCONNECTION, ROUTE_AUT_ETAT_0, ACOM_Disconnection },
    { EVT_ROUTE_LANE_CONNECTION, ROUTE_AUT_ETAT_0, Recv_msg_sv_con_rep },
    { EVT_ROUTE_COMMAND, ROUTE_AUT_ETAT_0, Recv_msg_pv_com_req },
	{ EVT_ROUTE_MSG_TRACE, ROUTE_AUT_ETAT_0, Recv_msg_lc_trace },
    { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_automate ROUTE_AUT_TABLE =
{
    { ETAT_0 }
};

/***********************/
/* FONCTIONS PUBLIQUES */
/***********************/

PROTECTED BOOL ROUTE_AUT_New (aut_route_id *id, ACOM_CNX_HANDLE hCnxHandle)
{
    aut_enum_retour retour;
    ROUTE_AUT *aut_route;
    
    if ((*id = malloc (sizeof (ROUTE_AUT))) == NULL)
        return FALSE;
    
    aut_route = *(ROUTE_AUT **) id;
    
    memset (aut_route, 0, sizeof(ROUTE_AUT));
    aut_route->hCnxHandle = hCnxHandle;

    retour = AUT_Lance (&aut_route->automate,
        *id, 
        ROUTE_AUT_ETAT_0,
        ROUTE_AUT_MAX_ETATS,
        ROUTE_AUT_TABLE,
		NULL,
        "ROUTE_AUT");
    
    if (retour != AUT_OK)
        free (*id);
    
    return retour == AUT_OK;
}

PROTECTED BOOL ROUTE_AUT_Delete (aut_route_id *id)
{
    aut_enum_retour retour;
    ROUTE_AUT *aut_route = *(ROUTE_AUT **) id;

    retour = AUT_Arret (&aut_route->automate);
    
    free (*id);
    *id = NULL;
    
    return retour == AUT_OK;
}

PROTECTED BOOL ROUTE_AUT_Send (aut_route_id id, aut_event_id event, PVOID args)
{
    ROUTE_AUT *aut_route = (ROUTE_AUT *) id;
    aut_enum_retour ret;
    
    NTSVCInfo ("ROUTE_AUT_Send (%s)", event);

    ret = AUT_Envoie (aut_route->automate, event, args);
    
    return ret == AUT_OK || ret == AUT_EVT_INCONNU;
}

PROTECTED BOOL ROUTE_AUT_Init (void)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams; // paramètres du registre

    // RAZ
    ZeroMemory( &ROUTE_AUT_PARAMS, sizeof(ROUTE_AUT_PARAMS) );

    // récupération des paramètres dans le registre
    psParams = NTSVCOpenParameters( 
            ROUTE_AUT_REG_VAL_MULTI_PLAZA      , REG_DWORD, 4, 0, &ROUTE_AUT_PARAMS.bMulti_Plaza,
			ROUTE_AUT_REG_VAL_MULTI_PLAZA_COEF , REG_DWORD, 4, 0, &ROUTE_AUT_PARAMS.bMulti_Plaza_Coef,
            NULL );
    if ( psParams == NULL )
        return FALSE;

    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;

    NTSVCCloseParameters( psParams );

    return TRUE;
}
