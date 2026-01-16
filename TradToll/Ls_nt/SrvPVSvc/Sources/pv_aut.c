/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DE PV 
* FICHIER: srvpv_aut.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge de faire evoluer l'automate de PV
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/pv_aut.c_v  $
 * 
 *    Rev 1.8   26 Apr 2002 11:44:52   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.6   21 Dec 2001 13:36:12   DSILBERM
 * Mise a jour des nouvelles dll
 * 
 *    Rev 1.5   26 Jan 2001 13:48:40   DSI
 * Ajout du message complémentaire de
 * transaction pour mise à jour temps réel
 * des classes + mise à niveau des dll
 * linkées
 * 
 *    Rev 1.4   19 Dec 2000 15:10:08   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.3   13 Dec 2000 16:34:46   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:34   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include "pv_aut.h"
#include <srvpv_main.h>
#include <ntsvc.h>
#include <csr_msg.h>
#include <pv_acom.h>
#include <pv_wm.h>
#include <route_wm.h>
#include <pv_conf.h>
#include <db_wm.h>
#include <msg_pv_user_req.h>
#include <msg_pv_user_rep.h>
#include <msg_pv_serv_rep.h>
#include <msg_pv_log_req.h>
#include <msg_pv_log_rep.h>
#include <msg_pv_conf_req.h>
#include <msg_pv_conf_rep.h>
#include <msg_pv_gal_req.h>
#include <msg_pv_plaz_rep.h>
#include <msg_pv_dec_req.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_com_req.h>
#include <msg_pv_auth_req.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

#define PV_AUT_REG_VAL_COM_FAILURE      "Com_Failure_Message"
#define PV_AUT_REG_VAL_DB_FAILURE       "DB_Failure_Message"
#define PV_AUT_REG_VAL_RELOAD_SERVER    "Com_Reload_Server"

/*--------------- TYPEDEFS: ----------------*/

typedef enum
{
    PV_AUT_ETAT_0,
    PV_AUT_ETAT_1,
    PV_AUT_ETAT_2,
    PV_AUT_ETAT_3,
    PV_AUT_MAX_ETATS
} 
PV_AUT_ETATS;

typedef struct
{
    aut_automate_id automate;
    ACOM_CNX_HANDLE hCnxHandle;
    struct MSG_PV_USER_REQ_Body user;
} 
PV_AUT;

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

//#pragma option -d-

/* liste des evenements reconnus en entree */
PROTECTED aut_event EVT_PV_LOGIN_REQ = AUT_EVT_INIT("EVT_PV_LOGIN_REQ", AUT_ARGS);
PROTECTED aut_event EVT_PV_LOGIN_REP = AUT_EVT_INIT("EVT_PV_LOGIN_REP", AUT_ARGS);
PROTECTED aut_event EVT_PV_PROFILE_REQ = AUT_EVT_INIT("EVT_PV_PROFILE_REQ", AUT_ARGS);
PROTECTED aut_event EVT_PV_PROFILE_REP = AUT_EVT_INIT("EVT_PV_PROFILE_REP", AUT_ARGS);
PROTECTED aut_event EVT_PV_PLAZA_CONF = AUT_EVT_INIT("EVT_PV_PLAZA_CONF", AUT_ARGS);
PROTECTED aut_event EVT_PV_GENERAL_STATE = AUT_EVT_INIT("EVT_PV_GENERAL_STATE", AUT_ARGS);
PROTECTED aut_event EVT_PV_PLAZA_STATE = AUT_EVT_INIT("EVT_PV_PLAZA_STATE", AUT_ARGS);
PROTECTED aut_event EVT_PV_ALARM_STATE = AUT_EVT_INIT("EVT_PV_ALARM_STATE", AUT_ARGS);
PROTECTED aut_event EVT_PV_LOGOUT = AUT_EVT_INIT("EVT_PV_LOGOUT", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_ABORT = AUT_EVT_INIT("EVT_PV_ABORT", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_COMMAND = AUT_EVT_INIT("EVT_PV_COMMAND", AUT_ARGS);
PROTECTED aut_event EVT_PV_CMD_REP = AUT_EVT_INIT("EVT_PV_CMD_REP", AUT_ARGS);
PROTECTED aut_event EVT_PV_AUTH_REQ = AUT_EVT_INIT("EVT_PV_AUTH_REQ", AUT_ARGS);
PROTECTED aut_event EVT_PV_AUTH_REP = AUT_EVT_INIT("EVT_PV_AUTH_REP", AUT_ARGS);
PROTECTED aut_event EVT_PV_COM_SERVER_FAILURE = AUT_EVT_INIT("EVT_PV_COM_SERVER_FAILURE", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_DATA_BASE_FAILURE = AUT_EVT_INIT("EVT_PV_DATA_BASE_FAILURE", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_RELOAD_SERVER = AUT_EVT_INIT("EVT_PV_RELOAD_SERVER", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_ACK_ALARM = AUT_EVT_INIT("EVT_PV_ACK_ALARM", AUT_ARGS);
PROTECTED aut_event EVT_PV_ALARM_ACKED = AUT_EVT_INIT("EVT_PV_ALARM_ACKED", AUT_ARGS);
PROTECTED aut_event EVT_PV_VAULT_STATUS = AUT_EVT_INIT("EVT_PV_VAULT_STATUS", AUT_ARGS);
PROTECTED aut_event EVT_PV_PARTLY = AUT_EVT_INIT("EVT_PV_PARTLY", AUT_ARGS);
PROTECTED aut_event EVT_PV_TRACE = AUT_EVT_INIT("EVT_PV_TRACE", AUT_ARGS);
PROTECTED aut_event EVT_PV_DISK = AUT_EVT_INIT("EVT_PV_DISK", AUT_ARGS);
PROTECTED aut_event EVT_PV_BOWL_STATUS = AUT_EVT_INIT("EVT_PV_BOWL_STATUS", AUT_ARGS);
PROTECTED aut_event EVT_PV_COMP_INFO = AUT_EVT_INIT("EVT_PV_COMP_INFO", AUT_ARGS);
PROTECTED aut_event EVT_PV_CONN_STAT_REQ = AUT_EVT_INIT("EVT_PV_CONN_STAT_REQ", AUT_NO_ARGS);
PROTECTED aut_event EVT_PV_CONN_STAT = AUT_EVT_INIT("EVT_PV_CONN_STAT", AUT_ARGS);

//#pragma option -d

PRIVATE struct 
{
    CHAR szCom_Failure[MSG_PV_MAX_MESSAGE_SIZE];
    CHAR szDB_Failure[MSG_PV_MAX_MESSAGE_SIZE];
    CHAR szReload_Server[MSG_PV_MAX_MESSAGE_SIZE];
}
PV_AUT_PARAMS = {0};

/*--------------- FUNCTIONS: ---------------*/

/****************************/
/* FONCTIONS DE TRANSITIONS */
/****************************/

PRIVATE BOOL Recv_msg_pv_log_req (PV_AUT *p_pv, struct MSG_PV_LOG_REQ *p_req)
{
    struct MSG_PV_LOG_REQ *p_dup;

    // on duplique le message (car il va etre efface apres cette fonction)
    p_dup = MSG_Duplicate (p_req);

    // pour identifier le PV expéditeur
    p_dup->header.id = p_pv->hCnxHandle;

    DB_PostMessage (DB_WM_USER_LOGIN_REQ, p_dup);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_log_rep (PV_AUT *p_pv, struct PV_WM_PARAM_USER_LOGIN_REP *p_login)
{
    struct MSG_PV_LOG_REP * p_rep;
    
	// create a service message
    p_rep = MSG_PV_LOG_REP_New();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("Send_msg_pv_log_rep() => Error memory allocation: MSG_PV_LOG_REP_New()!");      
        return FALSE;
    }
    
    // assigne message
    p_rep->header.id = SVC_Get_Msg_Id();
    p_rep->body = p_login->user;

	// calculation of the response
    if (p_login->user_ok == 0)
    {
        // "NO"
        strzcpy (p_rep->body.reply, MSG_PV_MAX_USER_REPLY, "N", sizeof (p_rep->body.reply));
    }
	else if (p_login->user_ok == 2)
	{
		// "EXPIRED (DEACTIVATED)"
		strzcpy(p_rep->body.reply, MSG_PV_MAX_USER_REPLY, "D", sizeof(p_rep->body.reply));
	}
    else if (p_login->user_ok == 3)
    {
        // "ERROR" -> DB HS
        strzcpy (p_rep->body.reply, MSG_PV_MAX_USER_REPLY, "E", sizeof (p_rep->body.reply));
    }
    else
    {
		// the user is accepted by the database
		// it must now be verified that it has not reached its quota
		// connections allowed simultaneously in its profile

		// if the PV requesting the connection is of the same profile
		// if the maximum number of connections allowed is not reached
        if (strncmp(p_login->user.profilename, p_pv->user.profilename, sizeof(p_login->user.profilename)) == 0 ||
            PV_Count_User_Connections (p_login->user.profilename) < p_login->profile_max_connections)
        {
            // "YES"
            strzcpy (p_rep->body.reply, MSG_PV_MAX_USER_REPLY, "Y", sizeof (p_rep->body.reply));
			dwNbOfPVConnection = dwNbOfPVConnection + 1;	// Increase PV connection for one
        }
        else
        {
            // "FULL"
            strzcpy (p_rep->body.reply, MSG_PV_MAX_USER_REPLY, "F", sizeof (p_rep->body.reply));
        }
    }

    NTSVCInfo("Send_msg_pv_log_rep() => user: %s, first name: %s, password: %d, reply: %s", 
			p_rep->body.name,
			p_rep->body.firstname,
			p_rep->body.matricule,
			p_rep->body.reply);

    // send message to the PV
    PV_ACOM_Send(p_pv->hCnxHandle, p_rep);
    
    // free message
    MSG_PV_LOG_REP_Delete_All(p_rep);

    return TRUE;
}

PRIVATE BOOL Recv_msg_pv_user_req (PV_AUT *p_pv, struct MSG_PV_USER_REQ *p_req)
{
    struct MSG_PV_USER_REQ *p_dup;

    // on duplique le message (car il va etre efface apres cette fonction)
    p_dup = MSG_Duplicate (p_req);

    // pour identifier le PV expéditeur
    p_dup->header.id = p_pv->hCnxHandle;

    DB_PostMessage (DB_WM_USER_PROFILE_REQ, p_dup);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_user_rep (PV_AUT *p_pv, struct PV_WM_PARAM_USER_PROFILE_REP *p_rep)
{
    // Envoi vers le PV sur le réseau
    PV_ACOM_Send (p_pv->hCnxHandle, p_rep->p_profile);

    // sauvegarder le user courant
    p_pv->user = p_rep->req;
    

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_conf_rep (PV_AUT *p_pv, void *args)
{
    struct MSG_PV_CONF_REP *p_rep;
    
    // on récupère l'image du message de configuration de la gare
    p_rep = PV_Build_msg_pv_conf_rep ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("Send_msg_pv_conf_rep() => Error execute: PV_Build_msg_pv_conf_rep()!");                    

        return FALSE;
    }

    // Envoi vers le PV sur le réseau
    PV_ACOM_Send (p_pv->hCnxHandle, p_rep);
        
    // suppression du message
    MSG_PV_CONF_REP_Delete_All (p_rep);

    return TRUE;
}

PRIVATE BOOL Recv_msg_pv_gal_req (PV_AUT *p_pv, struct MSG_PV_GAL_REQ *p_req)
{
    // on envoie une demande de rafraichissement de l'état complet
    // de la gare (REM :  tous les PV vont la recevoir)
    ROUTE_PostMessage (ROUTE_WM_TOTAL_PLAZA_STATE, NULL);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_buffer_rep (PV_AUT *p_pv, ARG_EVT_PV_BUFFER *p_rep)
{
    // on recoit directement un buffer pour des raisons d'efficacité
    // Envoi vers le PV sur le réseau
    PV_ACOM_Send_Buffer (p_pv->hCnxHandle, p_rep->buffer_size, p_rep->buffer);

    return TRUE;
}

PRIVATE BOOL Recv_msg_pv_conn_status_req(PV_AUT *p_pv, PVOID pNull)
{
	DB_PostMessage(DB_WM_CONNECT_STAUS_REQ, NULL);

	return TRUE;
}

PRIVATE BOOL Send_msg_pv_conn_status(PV_AUT *p_pv, ARG_EVT_PV_BUFFER *p_rep)
{
	PV_ACOM_Send_Buffer(p_pv->hCnxHandle, p_rep->buffer_size, p_rep->buffer);

	return TRUE;
}

PRIVATE BOOL User_logout (PV_AUT *p_pv, struct MSG_PV_DEC_REQ *p_req)
{
	if (dwNbOfPVConnection != 0)
		dwNbOfPVConnection = dwNbOfPVConnection - 1;

    // RAZ du user courant
    memset (&p_pv->user, 0, sizeof(p_pv->user));

    return TRUE;
}

PRIVATE BOOL Disconnect_PV (PV_AUT *p_pv, struct MSG_PV_DEC_REQ *p_req)
{
    // RAZ du user courant
    memset (&p_pv->user, 0, sizeof(p_pv->user));

    // déconnection du PV
    PV_ACOM_Disconnect (p_pv->hCnxHandle);

    return TRUE;
}

PRIVATE BOOL Ack_alarm (PV_AUT *p_pv, struct MSG_PV_ACK_REQ *p_req)
{
    struct MSG_PV_ACK_REQ *p_dup;

    // on duplique le message (car il va etre efface apres cette fonction)
    p_dup = MSG_Duplicate (p_req);

    // pour identifier le PV expéditeur
    p_dup->header.id = p_pv->hCnxHandle;

    ROUTE_PostMessage (ROUTE_WM_ACK_ALARM, p_dup);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_serv_rep (PV_AUT *p_pv, CHAR *message)
{
    struct MSG_PV_SERV_REP *p_rep;
    
    // Création d'un message de service
    p_rep = MSG_PV_SERV_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("Send_msg_pv_serv_rep() => Error memory allocation: MSG_PV_SERV_REP_New()!");                    

        return FALSE;
    }
    
    // affectation du message
    p_rep->header.id = SVC_Get_Msg_Id ();
    strzcpy (p_rep->body.message, MSG_PV_MAX_MESSAGE_SIZE,  message, sizeof (p_rep->body.message));
    
    // Envoi vers le PV sur le réseau
    PV_ACOM_Send (p_pv->hCnxHandle, p_rep);
    
    // suppression du message
    MSG_PV_SERV_REP_Delete_All (p_rep);

    return TRUE;
}

PRIVATE BOOL Com_server_failure (PV_AUT *p_pv, void *args)
{
    // informer le PV que la com. avec les voies est HS
    Send_msg_pv_serv_rep (p_pv, PV_AUT_PARAMS.szCom_Failure);

    return TRUE;
}

PRIVATE BOOL Data_base_failure (PV_AUT *p_pv, void *args)
{
    // informer le PV que la com. avec la base est HS
    Send_msg_pv_serv_rep (p_pv, PV_AUT_PARAMS.szDB_Failure);

    return TRUE;
}

PRIVATE BOOL Reload_server (PV_AUT *p_pv, void *args)
{
    // informer le PV que la communication avec le serveur va etre coupée
    Send_msg_pv_serv_rep (p_pv, PV_AUT_PARAMS.szReload_Server);

    return TRUE;
}

PRIVATE BOOL Recv_msg_pv_com_req (PV_AUT *p_pv, struct MSG_PV_COM_REQ *p_req)
{
    struct MSG_PV_COM_REQ *p_dup;

    // on duplique le message (car il va etre efface apres cette fonction)
    p_dup = MSG_Duplicate (p_req);

    // pour identifier le PV expéditeur
    p_dup->header.id = p_pv->hCnxHandle;

    // on teste si la commande est destinée aux voies ou a la base 
    // selon la valeur de l'ID
    if (p_req->body.id == 0)
        DB_PostMessage (DB_WM_CMD_REQ, p_dup);
    else
        ROUTE_PostMessage (ROUTE_WM_COMMAND, p_dup);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_com_rep (PV_AUT *p_pv, struct PV_WM_PARAM_DB_CMD_REP *rep)
{
    // informer le PV du résultat de la commande
    Send_msg_pv_serv_rep (p_pv, rep->string);

    return TRUE;
}

PRIVATE BOOL Recv_msg_pv_auth_req (PV_AUT *p_pv, struct MSG_PV_AUTH_REQ *p_req)
{
    struct MSG_PV_AUTH_REQ *p_dup;

    // on duplique le message (car il va etre efface apres cette fonction)
    p_dup = MSG_Duplicate (p_req);

    // pour identifier le PV expéditeur
    p_dup->header.id = p_pv->hCnxHandle;

    DB_PostMessage (DB_WM_USER_AUTH_REQ, p_dup);

    return TRUE;
}

PRIVATE BOOL Send_msg_pv_auth_rep (PV_AUT *p_pv, struct PV_WM_PARAM_USER_AUTH_REP *p_auth)
{
    struct MSG_PV_AUTH_REP * p_rep;
    
    // Création d'un message de service
    p_rep = MSG_PV_AUTH_REP_New ();
    if (p_rep == NULL)
    {
        NTSVC_ERR ("Send_msg_pv_auth_re() => Error memory allocation: MSG_PV_AUTH_REP_New()!");
        
        return FALSE;
    }
    
    // affectation du message
    p_rep->header.id = SVC_Get_Msg_Id ();
    p_rep->body = p_auth->user;

    NTSVCInfo ("Send_msg_pv_auth_rep() => user: %s, password: %d, reply: %s", 
                p_rep->body.name,
                p_rep->body.matricule,
                p_rep->body.reply);

    // Envoi vers le PV sur le réseau
    PV_ACOM_Send (p_pv->hCnxHandle, p_rep);
    
    // suppression du message
    MSG_PV_AUTH_REP_Delete_All (p_rep);

    return TRUE;
}

/*--------------- VARIABLES INITIALISEES: ---------------*/

PRIVATE aut_etat ETAT_0 =
{
    { EVT_PV_LOGIN_REQ,				PV_AUT_ETAT_0,	Recv_msg_pv_log_req		},
    { EVT_PV_LOGIN_REP,				PV_AUT_ETAT_0,	Send_msg_pv_log_rep		},
    { EVT_PV_PROFILE_REQ,			PV_AUT_ETAT_0,	Recv_msg_pv_user_req	},
    { EVT_PV_PROFILE_REP,			PV_AUT_ETAT_1,	Send_msg_pv_user_rep	},
    { EVT_PV_DATA_BASE_FAILURE,		PV_AUT_ETAT_0,	Data_base_failure		},
    { EVT_PV_LOGOUT,				PV_AUT_ETAT_0,	User_logout				},
    { EVT_PV_ABORT,					PV_AUT_ETAT_0,	Disconnect_PV			},
    { EVT_PV_RELOAD_SERVER,			PV_AUT_ETAT_0,	Reload_server			},

    { AUT_EVT_NULL,					AUT_ETAT_NULL,	AUT_ACTION_NULL			}
};

PRIVATE aut_etat ETAT_1 = 
{
    { EVT_PV_PLAZA_CONF,			PV_AUT_ETAT_2,	Send_msg_pv_conf_rep	},
    { EVT_PV_LOGOUT,				PV_AUT_ETAT_0,	User_logout				},
    { EVT_PV_ABORT,					PV_AUT_ETAT_0,	Disconnect_PV			},
    { EVT_PV_RELOAD_SERVER,			PV_AUT_ETAT_0,	Reload_server			},

    { AUT_EVT_NULL,					AUT_ETAT_NULL,	AUT_ACTION_NULL			}
};

PRIVATE aut_etat ETAT_2 = 
{
    { EVT_PV_GENERAL_STATE,			PV_AUT_ETAT_3,	Recv_msg_pv_gal_req		},
    { EVT_PV_LOGOUT,				PV_AUT_ETAT_0,	User_logout				},
    { EVT_PV_ABORT,					PV_AUT_ETAT_0,	Disconnect_PV			},
    { EVT_PV_RELOAD_SERVER,			PV_AUT_ETAT_0,	Reload_server			},
    { AUT_EVT_NULL,					AUT_ETAT_NULL,	AUT_ACTION_NULL			}
};

PRIVATE aut_etat ETAT_3 = 
{
    { EVT_PV_PLAZA_STATE,			PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_ALARM_STATE,			PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_ACK_ALARM,				PV_AUT_ETAT_3,	Ack_alarm				},
    { EVT_PV_VAULT_STATUS,			PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_PARTLY,				PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
	{ EVT_PV_TRACE,					PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_DISK,					PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_BOWL_STATUS,			PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
	{ EVT_PV_COMP_INFO,				PV_AUT_ETAT_3,	Send_msg_pv_buffer_rep	},
    { EVT_PV_COMMAND,				PV_AUT_ETAT_3,	Recv_msg_pv_com_req		},
    { EVT_PV_CMD_REP,				PV_AUT_ETAT_3,	Send_msg_pv_com_rep		},
    { EVT_PV_AUTH_REQ,				PV_AUT_ETAT_3,	Recv_msg_pv_auth_req	},
    { EVT_PV_AUTH_REP,				PV_AUT_ETAT_3,	Send_msg_pv_auth_rep	},
    { EVT_PV_LOGIN_REQ,				PV_AUT_ETAT_3,	Recv_msg_pv_log_req		},
    { EVT_PV_LOGIN_REP,				PV_AUT_ETAT_3,	Send_msg_pv_log_rep		},
    { EVT_PV_LOGOUT,				PV_AUT_ETAT_0,	User_logout				},
    { EVT_PV_ABORT,					PV_AUT_ETAT_0,	Disconnect_PV			},
    { EVT_PV_COM_SERVER_FAILURE,	PV_AUT_ETAT_3,	Com_server_failure		},
    { EVT_PV_DATA_BASE_FAILURE,		PV_AUT_ETAT_3,	Data_base_failure		},
    { EVT_PV_RELOAD_SERVER,			PV_AUT_ETAT_0,	Reload_server			},
	{ EVT_PV_CONN_STAT_REQ,			PV_AUT_ETAT_3,	Recv_msg_pv_conn_status_req },
	{ EVT_PV_CONN_STAT,				PV_AUT_ETAT_3,	Send_msg_pv_conn_status },

    { AUT_EVT_NULL,					AUT_ETAT_NULL, AUT_ACTION_NULL }
};

PRIVATE aut_automate PV_AUT_TABLE =
{
    { ETAT_0 },
    { ETAT_1 },
    { ETAT_2 },
    { ETAT_3 }
};

/***********************/
/* FONCTIONS PUBLIQUES */
/***********************/

PROTECTED BOOL PV_AUT_New (aut_pv_id *id, ACOM_CNX_HANDLE hCnxHandle)
{
    aut_enum_retour retour;
    PV_AUT *aut_pv;
    
    if ((*id = malloc (sizeof (PV_AUT))) == NULL)
        return FALSE;
    
    aut_pv = *(PV_AUT **) id;
    
    memset (aut_pv, 0, sizeof(PV_AUT));
    aut_pv->hCnxHandle = hCnxHandle;
    
    retour = AUT_Lance (&aut_pv->automate,
        *id, 
        PV_AUT_ETAT_0,
        PV_AUT_MAX_ETATS,
        PV_AUT_TABLE,
		NULL,
        "PV_AUT");
    
	if (retour != AUT_OK)
	{
		free(*id);
		*id = NULL;
	}
    
    return retour == AUT_OK;
}

PROTECTED BOOL PV_AUT_Delete (aut_pv_id *id)
{
    aut_enum_retour retour;
    PV_AUT *aut_pv = *(PV_AUT **) id;
    
    retour = AUT_Arret (&aut_pv->automate);
    
    free (*id);
    *id = NULL;
    
    return retour == AUT_OK;
}

PROTECTED BOOL PV_AUT_Send (aut_pv_id id, aut_event_id event, PVOID args)
{
    PV_AUT *aut_pv = (PV_AUT *) id;
    aut_enum_retour ret;
	int iEtatPrev, iEtatNext;
	DWORD Cnx;

	if (event == AUT_EVT_NULL)
		return FALSE;

	Cnx = ACOM_GET_SLOT_NUMBER(aut_pv->hCnxHandle);

	iEtatPrev = AUT_DonneEtatCourant(aut_pv->automate);
	NTSVCInfo("PV_AUT_Send(%u)  START - event: %s - state[%d]", Cnx, event, iEtatPrev);

    ret = AUT_Envoie (aut_pv->automate, event, args);
	
	if (ret == AUT_OK)
	{
		iEtatNext = AUT_DonneEtatCourant(aut_pv->automate);
		NTSVCInfo("PV_AUT_Send(%u) END - event: %s - from [%d] to [%d] complete!", Cnx,
			event,
			iEtatPrev,
			iEtatNext);
	}
	else
	{
		NTSVCInfo("PV_AUT_Send(%u)- ERROR: AUT_Envoie: event: %s return:%u", Cnx, event, ret);
	}

    return ret == AUT_OK || ret == AUT_EVT_INCONNU;
}

PROTECTED struct MSG_PV_USER_REQ_Body *PV_AUT_Get_Current_User (aut_pv_id id)
{
    PV_AUT *aut_pv = (PV_AUT *) id;
    
    return &aut_pv->user;
}

PROTECTED BOOL PV_AUT_Init (void)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams; // paramètres du registre

    // RAZ
    ZeroMemory( &PV_AUT_PARAMS, sizeof(PV_AUT_PARAMS) );

    // récupération des paramètres dans le registre
    psParams = NTSVCOpenParameters( 
            PV_AUT_REG_VAL_COM_FAILURE , REG_SZ, sizeof (PV_AUT_PARAMS.szCom_Failure), "COM SERVER COMMUNICATION FAILURE", &PV_AUT_PARAMS.szCom_Failure,
            PV_AUT_REG_VAL_DB_FAILURE  , REG_SZ, sizeof (PV_AUT_PARAMS.szDB_Failure),  "DATA BASE COMMUNICATION FAILURE", &PV_AUT_PARAMS.szDB_Failure,
            PV_AUT_REG_VAL_RELOAD_SERVER  , REG_SZ, sizeof (PV_AUT_PARAMS.szReload_Server),  "SERVER SHUTDOWN IN 10 SECONDS !", &PV_AUT_PARAMS.szReload_Server,
            NULL );
    if ( psParams == NULL )
        return FALSE;

    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;

    NTSVCCloseParameters( psParams );

    return TRUE;
}