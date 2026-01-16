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
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/includes/pv_aut.h_v  $
 * 
 *    Rev 1.6   26 Apr 2002 11:44:46   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.4   26 Jan 2001 13:47:38   DSI
 * Ajout du message complémentaire de
 * transaction pour mise à jour temps réel
 * des classes + mise à niveau des dll
 * linkées
 * 
 *    Rev 1.3   13 Dec 2000 16:34:40   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:26   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef PV_AUT_H
#define PV_AUT_H

/*--------------- INCLUDES: ----------------*/
#include "csr_aut.h"
#include <pv_acom.h>
#include <srvpv_main.h>

#include <protect.h>

/*--------------- DEFINES: -----------------*/
/*--------------- TYPEDEFS: ----------------*/

typedef PVOID aut_pv_id;

/* liste des evenements reconnus en entree */
PROTECTED aut_event EVT_PV_LOGIN_REQ;
PROTECTED aut_event EVT_PV_LOGIN_REP;
PROTECTED aut_event EVT_PV_PROFILE_REQ;
PROTECTED aut_event EVT_PV_PROFILE_REP;
PROTECTED aut_event EVT_PV_PLAZA_CONF;
PROTECTED aut_event EVT_PV_GENERAL_STATE;
PROTECTED aut_event EVT_PV_PLAZA_STATE;         // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_ALARM_STATE;         // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_LOGOUT;
PROTECTED aut_event EVT_PV_ABORT;
PROTECTED aut_event EVT_PV_COMMAND;
PROTECTED aut_event EVT_PV_CMD_REP;
PROTECTED aut_event EVT_PV_AUTH_REQ;
PROTECTED aut_event EVT_PV_AUTH_REP;
PROTECTED aut_event EVT_PV_COM_SERVER_FAILURE;
PROTECTED aut_event EVT_PV_ACK_ALARM;
PROTECTED aut_event EVT_PV_ALARM_ACKED;
PROTECTED aut_event EVT_PV_DATA_BASE_FAILURE;
PROTECTED aut_event EVT_PV_RELOAD_SERVER;
PROTECTED aut_event EVT_PV_VAULT_STATUS;        // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_PARTLY;			    // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_DISK;			    // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_BOWL_STATUS;         // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_TRACE;			    // ARG_EVT_PV_BUFFER
PROTECTED aut_event EVT_PV_COMP_INFO;
PROTECTED aut_event EVT_PV_STATS;
PROTECTED aut_event EVT_PV_CONN_STAT;
PROTECTED aut_event EVT_PV_CONN_STAT_REQ;
PROTECTED aut_event EVT_PV_CONN_STAT;			// ARG_EVT_PV_BUFFER

// Définition des arguments

typedef struct 
{
    BYTE buffer[SVC_MSG_BUFFER_SIZE];
    DWORD buffer_size;
}
ARG_EVT_PV_BUFFER;

/*--------------- FUNCTIONS: ---------------*/

PROTECTED BOOL PV_AUT_Init(void);
PROTECTED BOOL PV_AUT_New(aut_pv_id *id, ACOM_CNX_HANDLE hCnxHandle);
PROTECTED BOOL PV_AUT_Delete(aut_pv_id *aut_pv);
PROTECTED BOOL PV_AUT_Send(aut_pv_id id, aut_event_id event, PVOID args);

PROTECTED struct MSG_PV_USER_REQ_Body *PV_AUT_Get_Current_User(aut_pv_id id);

#endif