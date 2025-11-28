/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DE LAN
* FICHIER: atm_lan.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge de faire evoluer l'automate de
*         lan (gestion connexion, messages de vie, synchronisation)
* --------------------------------------------------------------------
* ENTREES :
* SORTIES :
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/Atm_lan.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:50   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:14   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/
#include <string.h>

#include "atm_lan.h"
#include "csr_lan.h"
#include "lan_glob.h"
#include "lan_mess.h"

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

/*--------------- TYPEDEFS: ----------------*/

typedef enum
{
    ATM_LAN_ETAT_0,
    ATM_LAN_ETAT_1,
    ATM_LAN_ETAT_2,
    ATM_LAN_ETAT_3,
    ATM_LAN_MAX_ETATS
} ATM_LAN_ETATS;

/*--------------- VARIABLES STATIQUES ---------------*/

typedef struct {
    noyau_pool_id pool;
    aut_automate_id  automate;

    short int ident;
    LONG diff_temps;
} ATM_LAN;

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

//#pragma option -d-

/* liste des evenements reconnus en entree */
PRIVATE aut_event EVT_LAN_CONNEXION = AUT_EVT_INIT("CONNEXION", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_SHUTDOWN = AUT_EVT_INIT("SHUTDOWN", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_CONNECTE = AUT_EVT_INIT("CONNECTE", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_ECHEC_CONNEXION = AUT_EVT_INIT("ECHEC CONNECTION", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_DECONNEXION = AUT_EVT_INIT("DECONNEXION", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_MESSAGE = AUT_EVT_INIT("MESSAGE", AUT_ARGS);
PRIVATE aut_event EVT_LAN_REPONDRE_VIE = AUT_EVT_INIT("REPONDRE VIE", AUT_ARGS);
PRIVATE aut_event EVT_LAN_DEMANDE_VIE = AUT_EVT_INIT("DEMANDE VIE", AUT_NO_ARGS);
PRIVATE aut_event EVT_LAN_DEMANDE_SYNCHRO = AUT_EVT_INIT("DEMANDE SYNCHRO", AUT_NO_ARGS);

//#pragma option -d

/*--------------- FUNCTIONS: ---------------*/

/**********************/
/* FONCTIONS INTERNES */
/**********************/

PRIVATE aut_enum_retour ATM_LAN_Envoie (HANDLE id, aut_event_id event, PVOID args);
PRIVATE void Synchroniser (HANDLE id, LONGLONG *MsgDate);

/****************************/
/* FONCTIONS DE TRANSITIONS */
/****************************/

PRIVATE BOOL Connecte (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].cpt_timeout_msg = 0;

    LAN[atm_lan->ident].msg_etat.serveur_msg = LAN_CONNEXION;
    LAN[atm_lan->ident].msg_etat.serveur_tps = LAN_LIAISON_OK;
    
    LanceChrono (LAN[atm_lan->ident].chrono_attente_msg, 
                 LAN[atm_lan->ident].duree_chrono_attente_msg, 
                 LAN[atm_lan->ident].lan_bal);

	return TRUE;
}

PRIVATE BOOL Echec_Connexion (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].msg_etat.serveur_msg = LAN_DEGRADE;
    LAN[atm_lan->ident].msg_etat.serveur_tps = LAN_LIAISON_TPS_HS;
    
    LanceChrono (LAN[atm_lan->ident].chrono_reconnexion,
                 LAN[atm_lan->ident].duree_chrono_reconnexion, 
                 LAN[atm_lan->ident].lan_bal);

	return TRUE;
}

PRIVATE BOOL Deconnexion (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].msg_etat.serveur_msg = LAN_DEGRADE;
    LAN[atm_lan->ident].msg_etat.serveur_tps = LAN_LIAISON_TPS_HS;

    LanDisconnect (atm_lan->ident, &LAN[atm_lan->ident].param);
 
    ArretChrono (LAN[atm_lan->ident].chrono_attente_msg);
    LanceChrono (LAN[atm_lan->ident].chrono_reconnexion,
                 LAN[atm_lan->ident].duree_chrono_reconnexion, 
                 LAN[atm_lan->ident].lan_bal);
	return TRUE;
}

PRIVATE BOOL Shutdown (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].msg_etat.serveur_msg = LAN_ATTENTE_CONNEXION;
    LAN[atm_lan->ident].msg_etat.serveur_tps = LAN_LIAISON_TPS_HS;
    
    LanDisconnect (atm_lan->ident, &LAN[atm_lan->ident].param);

    ArretChrono (LAN[atm_lan->ident].chrono_attente_msg);
    ArretChrono (LAN[atm_lan->ident].chrono_reconnexion);

	return TRUE;
}

PRIVATE BOOL Connexion (ATM_LAN *atm_lan, PVOID args)
{
   enum_lan_erreur result;

   result = LanConnect(atm_lan->ident, &LAN[atm_lan->ident].param);

   if (result == LAN_OK)
      ATM_LAN_Envoie (atm_lan, EVT_LAN_CONNECTE, NULL);  
   else
      ATM_LAN_Envoie (atm_lan, EVT_LAN_ECHEC_CONNEXION, NULL);  

   return TRUE;
}

PRIVATE BOOL Message (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].cpt_timeout_msg = 0;    

    ArretChrono (LAN[atm_lan->ident].chrono_attente_msg);
    LanceChrono (LAN[atm_lan->ident].chrono_attente_msg, 
                 LAN[atm_lan->ident].duree_chrono_attente_msg, 
                 LAN[atm_lan->ident].lan_bal);

    Synchroniser (atm_lan, args);

	return TRUE;
}

PRIVATE BOOL Repondre_Vie (ATM_LAN *atm_lan, PVOID args)
{
    LAN[atm_lan->ident].cpt_timeout_msg = 0;    

    ArretChrono (LAN[atm_lan->ident].chrono_attente_msg);
    LanceChrono (LAN[atm_lan->ident].chrono_attente_msg, 
                 LAN[atm_lan->ident].duree_chrono_attente_msg, 
                 LAN[atm_lan->ident].lan_bal);

    // réponse au message de vie
    LanEnvoiMessage (atm_lan->ident, 0, PROT_REPONSE_VIE, NULL);

    Synchroniser (atm_lan, args);

	return TRUE;
}

PRIVATE BOOL Demande_Vie (ATM_LAN *atm_lan, PVOID args)
{
    if (LAN[atm_lan->ident].cpt_timeout_msg++ >= LAN[atm_lan->ident].max_cpt_timeout_msg)
    {
        ATM_LAN_Envoie (atm_lan, EVT_LAN_DECONNEXION, NULL);  
    }
    else
    {
        ArretChrono (LAN[atm_lan->ident].chrono_attente_msg);
        LanceChrono (LAN[atm_lan->ident].chrono_attente_msg, 
                     LAN[atm_lan->ident].duree_chrono_attente_msg, 
                     LAN[atm_lan->ident].lan_bal);

        // message de demande de vie
        LanEnvoiMessage (atm_lan->ident, 0, PROT_DEMANDE_VIE, NULL);
    }

	return TRUE;
}

PRIVATE BOOL Demande_Synchro (ATM_LAN *atm_lan, PVOID args)
{
    struct_lan_gestion_tps p_msg;
    
    p_msg.difference_tps = atm_lan->diff_temps;

    LanEnvoiService(atm_lan->ident,
                    M_LAN_HORAIRE, LAN_RECEPTION, &p_msg);

	return TRUE;
}

PRIVATE void Synchroniser (ATM_LAN *atm_lan, LONGLONG *MsgDate)
{
    struct_lan_gestion_tps p_msg;
    LONGLONG now;
    LONG diff_temps;

    GetSystemTimeAsFileTime ((FILETIME *) &now);

    // différence de temps en secondes (conversion de 100ns d'unité)
    now -= *MsgDate;
    now /= 10000000UL;
    diff_temps = -(LONG) now;

    // pour éviter de bombarder le service a chaque message
    // on tolere une différence de temps...
    if (labs(atm_lan->diff_temps - diff_temps) > LAN[atm_lan->ident].max_diff_time)
    {
        p_msg.difference_tps = atm_lan->diff_temps = diff_temps;

        LanEnvoiService(atm_lan->ident,
                    M_LAN_HORAIRE, LAN_RECEPTION, &p_msg);

        LanEnvoiDop(atm_lan->ident, (unsigned char *)&p_msg, sizeof(p_msg), 
               SRV_ESPION_ENTRANT, LAN_DOP_TPS);

    }
}
/*--------------- VARIABLES INITIALISEES: ---------------*/

// AU REPOS DECONNECTE
PRIVATE aut_etat ETAT_0 = {
    { EVT_LAN_SHUTDOWN,          ATM_LAN_ETAT_0, Shutdown },
    { EVT_LAN_CONNEXION,         ATM_LAN_ETAT_1, Connexion }, 
    { AUT_EVT_NULL,              AUT_ETAT_NULL,  AUT_ACTION_NULL }
};

// EN COURS DE CONNEXION
PRIVATE aut_etat ETAT_1 = {
    { EVT_LAN_SHUTDOWN,          ATM_LAN_ETAT_0, Shutdown },
    { EVT_LAN_CONNECTE,          ATM_LAN_ETAT_2, Connecte },
    { EVT_LAN_ECHEC_CONNEXION,   ATM_LAN_ETAT_3, Echec_Connexion },
    { AUT_EVT_NULL,              AUT_ETAT_NULL,  AUT_ACTION_NULL }
};

// CONNECTE / EN ATTENTE DE MSG
PRIVATE aut_etat ETAT_2 = {
    { EVT_LAN_SHUTDOWN,          ATM_LAN_ETAT_0, Shutdown },
    { EVT_LAN_DECONNEXION,       ATM_LAN_ETAT_3, Deconnexion },
    { EVT_LAN_MESSAGE,           ATM_LAN_ETAT_2, Message },
    { EVT_LAN_REPONDRE_VIE,      ATM_LAN_ETAT_2, Repondre_Vie },
    { EVT_LAN_DEMANDE_VIE,       ATM_LAN_ETAT_2, Demande_Vie },
    { EVT_LAN_DEMANDE_SYNCHRO,   ATM_LAN_ETAT_2, Demande_Synchro },
    { AUT_EVT_NULL,              AUT_ETAT_NULL,  AUT_ACTION_NULL }
};

// DECONNECTE / EN ATTENTE DE RECONNEXION
PRIVATE aut_etat ETAT_3 = {
    { EVT_LAN_SHUTDOWN,          ATM_LAN_ETAT_0, Shutdown },
    { EVT_LAN_CONNEXION,         ATM_LAN_ETAT_1, Connexion },
    { AUT_EVT_NULL,              AUT_ETAT_NULL,  AUT_ACTION_NULL }
};

PRIVATE aut_automate AUT_LAN_TABLE =
{
    { ETAT_0	, NULL	, NULL },
    { ETAT_1	, NULL	, NULL },
    { ETAT_2	, NULL	, NULL },
    { ETAT_3	, NULL	, NULL },
};

/***********************/
/* FONCTIONS PUBLIQUES */
/***********************/

PUBLIC aut_enum_retour ATM_LAN_Lance (HANDLE *id, 
                                      short int ident)
{
    aut_enum_retour retour;
    ATM_LAN *atm_lan;
    
    /* L'automate de lan peut gerer plusieurs instances de lui-meme.
    Chaque fct recoit donc en parametres un id de l'automate
    que l'on souhaite manipuler. Si l'on ne veut qu'une instance
    (ou si une seule suffit), on peut simplifier l'interface en
    supprimant cet id et ses allocations dynamiques
    en les remplacant par une declaration statique de ATM_LAN */
    
    ExitAlloue ((struct_neutre **) id, sizeof (ATM_LAN), LAN[ident].lan_pool);
    
    atm_lan = *(ATM_LAN **) id;
    memset (atm_lan, 0, sizeof(ATM_LAN));

    retour = AUT_Lance (&atm_lan->automate,
                        atm_lan,
                        ATM_LAN_ETAT_0,
                        ATM_LAN_MAX_ETATS,
                        AUT_LAN_TABLE,
						NULL,
						"ATM_LAN");
    
    if (retour != AUT_OK)
        ExitLibere ((struct_neutre **) id);
        
    return retour;
}

PUBLIC aut_enum_retour ATM_LAN_Arret (HANDLE *id)
{
    aut_enum_retour retour;
    ATM_LAN *atm_lan;
    
    atm_lan = *(ATM_LAN **) id;
    
    retour = AUT_Arret (&atm_lan->automate);
    
    ExitLibere ((struct_neutre **) id);
    
    return retour;
}

PRIVATE aut_enum_retour ATM_LAN_Envoie (HANDLE id, aut_event_id event, PVOID args)
{
    ATM_LAN *atm_lan = (ATM_LAN *) id;
    
    return AUT_Envoie (atm_lan->automate, event, args);
}

PUBLIC aut_enum_retour ATM_LAN_Reset (HANDLE id)
{
    ATM_LAN *atm_lan  = (ATM_LAN *) id;
    
    return AUT_Reset (atm_lan->automate);
}

PUBLIC BOOL ATM_LAN_Reception_Message_BAL (HANDLE id, struct_neutre *p_neutre)
{
    ATM_LAN *atm_lan  = (ATM_LAN *) id;
    struct_chrono *p_chro = (struct_chrono *) p_neutre;
    BOOL retour = FALSE;
    
    // si message de chrono
    if (p_chro->neutre.bl_retour == NOYAU_BAL_HORLOGE_ID)
    {
        if (p_chro->numero == LAN[atm_lan->ident].chrono_attente_msg)
        {
            ATM_LAN_Envoie (id, EVT_LAN_DEMANDE_VIE, NULL);
            retour = TRUE;
        }
        else if (p_chro->numero == LAN[atm_lan->ident].chrono_reconnexion)
        {
            ATM_LAN_Envoie (id, EVT_LAN_CONNEXION, NULL);
            retour = TRUE;
        }
    }

    return retour;
}

PUBLIC BOOL ATM_LAN_Reception_Message_Reseau (HANDLE id, struct_prot *p_prot)
{
    ATM_LAN *atm_lan  = (ATM_LAN *) id;
    BOOL retour = FALSE;
    
    // message de protocol ou applicatif ?
    switch (p_prot->MsgType)
    {
    case PROT_APPLICATIF:
        ATM_LAN_Envoie (id, EVT_LAN_MESSAGE, &p_prot->MsgDate);
        retour = FALSE;
    break;

    case PROT_DEMANDE_VIE:
        ATM_LAN_Envoie (id, EVT_LAN_REPONDRE_VIE, &p_prot->MsgDate);
        retour = TRUE;
    break;

    case PROT_REPONSE_VIE:
        ATM_LAN_Envoie (id, EVT_LAN_MESSAGE, &p_prot->MsgDate);
        retour = TRUE;
    break;

    default:
        LanFichierDebug(atm_lan->ident, "MsgType inconnu = %lu", p_prot->MsgType);
    break;
    }

    return retour;
}

PUBLIC void ATM_LAN_Connexion(HANDLE id)
{
    ATM_LAN_Envoie (id, EVT_LAN_CONNEXION, NULL);
}

PUBLIC void ATM_LAN_Shutdown(HANDLE id)
{
    ATM_LAN_Envoie (id, EVT_LAN_SHUTDOWN, NULL);
}

PUBLIC void ATM_LAN_Break(HANDLE id)
{
    ATM_LAN_Envoie (id, EVT_LAN_DECONNEXION, NULL);
}

PUBLIC void ATM_LAN_Synchronisation (HANDLE id)
{
    ATM_LAN_Envoie (id, EVT_LAN_DEMANDE_SYNCHRO, NULL);
}