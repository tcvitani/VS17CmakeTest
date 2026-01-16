/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DECISIONNEL DE COMPTAGES PV
* FICHIER: aut_pv.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/includes/alarm_aut.h_v  $
 * 
 *    Rev 1.5   26 Apr 2002 11:44:44   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.3   21 Dec 2001 13:35:58   DSILBERM
 * Mise a jour des nouvelles dll
 * 
 *    Rev 1.2   Dec 17 1999 15:01:58   pgg
 *  
 * 
 *    Rev 1.1   Dec 08 1999 16:35:14   pgg
 *  
 * 
 *    Rev 1.0   Nov 22 1999 11:00:24   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ALARM_AUT_H
#define ALARM_AUT_H

/*--------------- INCLUDES: ----------------*/
#include <csr_aut.h>
#include <csr_list.h>
#include <msg_pv_define.h>

#include <protect.h>

/*--------------- DEFINES: -----------------*/
/*--------------- TYPEDEFS: ----------------*/

typedef PVOID aut_alarm_id;

typedef enum {
    ALARM_MODE_0, // invisible
    ALARM_MODE_1, // non acquittable
    ALARM_MODE_2, // disparition et acquittement
    ALARM_MODE_3, // disparition
    ALARM_MODE_4, // acquittement
    ALARM_MODE_5, // disparition ou acquittement
    ALARM_MAX_MODE
} aut_alarm_mode;

/* liste des evenements reconnus en entree */
PROTECTED aut_event EVT_ALARM_ACKED;    // ARG_EVT_ALARM_ACKED      
PROTECTED aut_event EVT_ALARM_ON;       // ARG_EVT_ALARM_ON
PROTECTED aut_event EVT_ALARM_OFF;

struct ARG_EVT_ALARM_ON
{
    // date d'apparition
    IN LPSYSTEMTIME appdate;
};

struct ARG_EVT_ALARM_ACKED
{
    // date d'acquittement
    IN SYSTEMTIME ackdate;

    // matricule de l'utilisateur
    IN DWORD matricule;

    // alarme rellement acquittée
    OUT BOOL acked;
};

/*--------------- FUNCTIONS: ---------------*/

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
							  CHAR *pDescription);

PROTECTED BOOL ALARM_AUT_Delete (HLIST hList, aut_alarm_id *aut_alarm);
PROTECTED BOOL ALARM_AUT_Send (aut_alarm_id id, aut_event_id event, PVOID args);

PROTECTED struct MSG_PV_ALAR_REP_Event *ALARM_AUT_Get_Event (aut_alarm_id id);
PROTECTED BOOL ALARM_AUT_To_Be_Deleted (aut_alarm_id id);

#endif