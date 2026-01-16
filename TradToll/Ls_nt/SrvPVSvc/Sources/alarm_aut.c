/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DE ALAR
* FICHIER: atm_alar.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge de faire evoluer l'automate de
*         alar (comptage des esieux simples et doubles en
*         marche avant et marche arriere)
* --------------------------------------------------------------------
* ENTREES :
* SORTIES :
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/alarm_aut.c_v  $
 * 
 *    Rev 1.6   26 Apr 2002 11:44:50   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.4   21 Dec 2001 13:36:08   DSILBERM
 * Mise a jour des nouvelles dll
 * 
 *    Rev 1.3   19 Dec 2000 15:10:06   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.2   Dec 17 1999 15:02:06   pgg
 *  
 * 
 *    Rev 1.1   Dec 08 1999 16:35:28   pgg
 *  
 * 
 *    Rev 1.0   Nov 22 1999 11:00:28   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <alarm_aut.h>
#include <lane.h>
#include <csr_aut.h>
#include <msg_pv_alar_rep.h>
#include <srvpv_main.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

/*--------------- TYPEDEFS: ----------------*/

typedef enum
{
   ALARM_AUT_ETAT_0,
   ALARM_AUT_ETAT_1,
   ALARM_AUT_ETAT_2,
   ALARM_AUT_ETAT_3,
   ALARM_AUT_MAX_ETATS
} 
ALARM_AUT_ETATS;

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

//#pragma option -d-

/* liste des evenements reconnus en entree */
PROTECTED aut_event EVT_ALARM_ON = AUT_EVT_INIT("APPARITION", AUT_ARGS);
PROTECTED aut_event EVT_ALARM_OFF = AUT_EVT_INIT("DISPARITION", AUT_NO_ARGS);
PROTECTED aut_event EVT_ALARM_ACKED = AUT_EVT_INIT("ACQUITTEMENT", AUT_ARGS);

//#pragma option -d

typedef struct ALARM_AUT
{
    BOOL to_delete;
    struct LANE_CNF *p_lane_cnf;
    struct MSG_PV_ALAR_REP_Event event;
    DWORD ack_mode;

    aut_automate_id automate;
} 
ALARM_AUT;

/*--------------- FUNCTIONS: ---------------*/

/****************************/
/* FONCTIONS DE TRANSITIONS */
/****************************/

PRIVATE BOOL Alarm_New (struct ALARM_AUT *p_alar, struct ARG_EVT_ALARM_ON *event)
{
    // nouvelle alarme
    p_alar->event.state = p_alar->ack_mode >= ALARM_MODE_2;
    p_alar->event.matricule = 0;
    memset (&p_alar->event.ackdate, 0, sizeof(p_alar->event.ackdate));

    // date d'apparition 
    p_alar->event.appdate = *event->appdate;

    // liste des alarmes modifiée...
    LANE_Set_Alarm_Flag (p_alar->p_lane_cnf, TRUE);

    return TRUE;
}

PRIVATE BOOL Alarm_Acked (struct ALARM_AUT *p_alar, struct ARG_EVT_ALARM_ACKED *ack)
{
    // alarme acquittée
    p_alar->event.state = 2;
    p_alar->event.ackdate = ack->ackdate;
    p_alar->event.matricule = ack->matricule;

    ack->acked = TRUE;

    // liste des alarmes modifiée...
    LANE_Set_Alarm_Flag (p_alar->p_lane_cnf, TRUE);

    return TRUE;
}

PRIVATE BOOL Alarm_To_Delete (struct ALARM_AUT *p_alar, PVOID args)
{
    // sera effacee au prochain envoi de msg d'etat alarmes
    p_alar->to_delete = TRUE;
    
    // liste des alarmes modifiée...    
    LANE_Set_Alarm_Flag (p_alar->p_lane_cnf, TRUE);
    
    return TRUE;
}

PRIVATE BOOL Alarm_Acked_To_Delete (struct ALARM_AUT *p_alar, struct ARG_EVT_ALARM_ACKED *ack)
{
    // alarme a acquittée
    Alarm_Acked (p_alar, ack);

    // marquer l'alarme comme a effacer
    Alarm_To_Delete (p_alar, NULL);

    return TRUE;
}

/*--------------- VARIABLES INITIALISEES: ---------------*/

PRIVATE aut_etat ETAT_0_MODE_0 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_0, Alarm_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_0 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_1_MODE_1 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_0, Alarm_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_1_MODE_2 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_3, AUT_ACTION_NULL },
   { EVT_ALARM_ACKED, ALARM_AUT_ETAT_2, Alarm_Acked },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_1_MODE_3 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_0, Alarm_To_Delete },
   { EVT_ALARM_ACKED, ALARM_AUT_ETAT_2, Alarm_Acked },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_1_MODE_4 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_3, AUT_ACTION_NULL },
   { EVT_ALARM_ACKED, ALARM_AUT_ETAT_0, Alarm_Acked_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_1_MODE_5 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_0, Alarm_To_Delete },
   { EVT_ALARM_ACKED, ALARM_AUT_ETAT_0, Alarm_Acked_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_2 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_OFF, ALARM_AUT_ETAT_0, Alarm_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_etat ETAT_3 = {
   { EVT_ALARM_ON, ALARM_AUT_ETAT_1, Alarm_New },
   { EVT_ALARM_ACKED, ALARM_AUT_ETAT_0, Alarm_Acked_To_Delete },
   { AUT_EVT_NULL, AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_automate ALARM_AUT_MODE_0 =
{
	{ ETAT_0_MODE_0 },
	{ NULL },
	{ NULL },
	{ NULL }
};

PRIVATE aut_automate ALARM_AUT_MODE_1 =
{
	{ ETAT_0 },
	{ ETAT_1_MODE_1 },
	{ NULL },
	{ NULL }
};

PRIVATE aut_automate ALARM_AUT_MODE_2 =
{
	{ ETAT_0 },
	{ ETAT_1_MODE_2 },
	{ ETAT_2 },
	{ ETAT_3 }
};

PRIVATE aut_automate ALARM_AUT_MODE_3 =
{
	{ ETAT_0 },
    { ETAT_1_MODE_3 },
	{ ETAT_2 },
    { NULL }
};

PRIVATE aut_automate ALARM_AUT_MODE_4 =
{
	{ ETAT_0 },
	{ ETAT_1_MODE_4 },
	{ NULL },
	{ ETAT_3 }
};

PRIVATE aut_automate ALARM_AUT_MODE_5 =
{
	{ ETAT_0 },
    { ETAT_1_MODE_5 },
	{ NULL },
    { NULL }
};

PRIVATE  aut_automate *ALARM_AUT_TABLE[] =
{
    (PVOID) ALARM_AUT_MODE_0,
    (PVOID) ALARM_AUT_MODE_1,
    (PVOID) ALARM_AUT_MODE_2,
    (PVOID) ALARM_AUT_MODE_3,
    (PVOID) ALARM_AUT_MODE_4,
    (PVOID) ALARM_AUT_MODE_5
};

/***********************/
/* FONCTIONS PUBLIQUES */
/***********************/

PROTECTED BOOL ALARM_AUT_New (HLIST hList,
							  aut_alarm_id *id, 
                              struct LANE_CNF *p_lane_cnf,
                              DWORD plaza_number,
                              DWORD lane_number,
                              DWORD alarm_type,
                              DWORD alarm_sub_type,
                              DWORD event_id,
                              DWORD ack_mode,
                              DWORD event_level,
                              CHAR *event_label,
                              CHAR *event_directive,
							  CHAR *pDescription)
{
    aut_enum_retour retour;
    ALARM_AUT *aut_alarm;

    if (ack_mode >= ALARM_MAX_MODE)
        return FALSE;
    
    if ((*id = List_ItemNew (hList, sizeof (ALARM_AUT))) == NULL)
        return FALSE;
    
    aut_alarm = *(ALARM_AUT **) id;
    
    memset (aut_alarm, 0, sizeof(ALARM_AUT));
    aut_alarm->p_lane_cnf = p_lane_cnf;
    aut_alarm->event.plazanum = plaza_number;
    aut_alarm->event.lanenum = lane_number;
    aut_alarm->event.type = alarm_type;
    aut_alarm->event.subtype = alarm_sub_type;
    aut_alarm->event.id = event_id;
    aut_alarm->ack_mode = ack_mode;
    aut_alarm->event.level = event_level;
    strzcpy (aut_alarm->event.label, MSG_PV_MAX_ALARM_LABEL, event_label, sizeof(aut_alarm->event.label));
    strzcpy (aut_alarm->event.instruction, MSG_PV_MAX_DETAIL_SIZE, event_directive, sizeof(aut_alarm->event.instruction));
	strzcpy(aut_alarm->event.description, MSG_PV_MAX_ALARM_DESCRIPTION, pDescription, sizeof(aut_alarm->event.description));

    retour = AUT_Lance (&aut_alarm->automate,
                        *id, 
                        ALARM_AUT_ETAT_0,
                        ALARM_AUT_MAX_ETATS,
                        *ALARM_AUT_TABLE[ack_mode],
						NULL,
                        "ALARM_AUT");
    
    if (retour != AUT_OK)
        free (id);
    
    return retour == AUT_OK;
}

PROTECTED BOOL ALARM_AUT_Delete (HLIST hList, aut_alarm_id *id)
{
    aut_enum_retour retour;
    ALARM_AUT *aut_alarm = *(ALARM_AUT **) id;
    
    retour = AUT_Arret (&aut_alarm->automate);
    
    List_ItemDelete (hList, id);
    *id = NULL;
    
    return retour == AUT_OK;
}

PROTECTED BOOL ALARM_AUT_Send (aut_alarm_id id, aut_event_id event, PVOID args)
{
    ALARM_AUT *aut_alarm = (ALARM_AUT *) id;
    
//    NTSVCInfo ("ALARM_AUT_Send (%s)", event);

    return AUT_Envoie (aut_alarm->automate, event, args) == AUT_OK;
}

PROTECTED struct MSG_PV_ALAR_REP_Event *ALARM_AUT_Get_Event (aut_alarm_id id)
{
    ALARM_AUT *aut_alarm = (ALARM_AUT *) id;

    return &aut_alarm->event;
}

PROTECTED BOOL ALARM_AUT_To_Be_Deleted (aut_alarm_id id)
{
    ALARM_AUT *aut_alarm = (ALARM_AUT *) id;

    return aut_alarm->to_delete;
}