/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_MESS.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code des fonctions traitant les types de messages recus par
*         la tache LAN
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_mess.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:58   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* 
*    Rev 1.2   Jan 20 1999 11:36:04   bph
*  
* 
*    Rev 1.1   02 Oct 1998 11:46:00   bph
*  
* 
*    Rev 1.16   05 Feb 1998 10:38:48   BPH
* Modif de traces
* 
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <string.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <lan_glob.h>

#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/

typedef struct 
{
    short int ident;
    enum_lan_service  service_id;
    enum_lan_type     type_message;
    void *p_msg;
}
struct_lan_envoi_service;

typedef struct 
{
    short int ident;
    struct_lan_message *p_msg;
}
struct_lan_envoi_service_dop;

typedef struct 
{
   enum_espion_nature     nature;  /* donnees ou protocole */
   enum_espion_sens       sens;   /* entrant et/ou sortant */
}
struct_lan_service_dop;

/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/


PROTECTED void LanAttendBAL (short int ident)
{
    // Attente publication de la BAL LAN
    LAN[ident].lan_bal = AttendBAL (LAN[ident].nom_lan_bal);
    
    // Attente publication de la BAL EMI
    LAN[ident].emis_bal = AttendBAL (LAN[ident].nom_emis_bal);
    
    // Attente publication de la BAL FIC
    LAN[ident].fic_bal = AttendBAL (LAN[ident].nom_fic_bal);
}

PROTECTED void LanSupprimeBAL (short int ident)
{
    // Suppression de la BAL LAN
    SupprimeBAL (LAN[ident].nom_lan_bal);
    
    // Suppression de la BAL EMI
    SupprimeBAL (LAN[ident].nom_emis_bal);
    
    // Suppression de la BAL FIC
    SupprimeBAL (LAN[ident].nom_fic_bal);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int DebutService(short int ident_lecteur,
*                                           short int service_id,
*                                           short int bal_dest)
* PARAMETRES:
*     entree: numero du lecteur
*           : identificateur du service
*           : boite aux lettres de la tache utilisatrice du service
*     retour: demande valide ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache LAN_ANI
* ROLE: Acceptation ou refus de la demande de debut de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_valide LanDebutService(short int ident,
                                          enum_lan_service service_id,
                                          noyau_bal_id bal_demandeur)
{
    enum_lan_type ret;
    
    ret = LAN_DEBUT_NACQ;
    
    // le demandeur possède deja le service ?
    if (SrvEstDemandeur (LAN[ident].service[service_id], bal_demandeur) == FALSE)
    {
        // le nombre de jetons max est atteind ?
        if (SrvAjouteDemandeur (LAN[ident].service[service_id], bal_demandeur, 0L) != NULL)
            ret = LAN_DEBUT_ACQ;
    }
        
    LanEnvoiAcquittement(ident, bal_demandeur, service_id, ret);
    
    if (ret == LAN_DEBUT_NACQ)
        return LAN_NON_VALIDE;
    
    return LAN_VALIDE;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int FinService( short int service_id,
*                                       unsigned char bal_dest)
* PARAMETRES:
*     entree: identificateur du service
*           : boite aux lettres de la tache utilisatrice du service
*     retour: demande valide ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache LAN_ANI
* ROLE: Acceptation ou refus de la demande de fin de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_type LanFinService(short int ident,
                                      enum_lan_service service_id,
                                      noyau_bal_id bal_demandeur)
{
    PVOID pSrvData;
    
    // recherche le demandeur de ce service
    pSrvData = SrvRechercheDemandeur (LAN[ident].service[service_id], 
        bal_demandeur,
        NULL,
        NULL);
    if (pSrvData == NULL)
    {
        LanEnvoiAcquittement(ident, bal_demandeur, service_id, LAN_FIN_NACQ);
        
        return LAN_NON_VALIDE;
    }
    
    SrvEnleveDemandeur (LAN[ident].service[service_id], &pSrvData);
    
    LanEnvoiAcquittement(ident,bal_demandeur, service_id, LAN_FIN_ACQ);
    
    return LAN_VALIDE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  PROTECTED BOOL LanEnvoiService (zip_ident_lecteur ident, 
*                                enum_zip_service  service,
*                                enum_zip_type     type,
*                                struct_zip_message *p_msg)
* PARAMETRES:
*     entree: numero du lecteur
*           : service du message
*           : type du message
*           : message
*     retour: TRUE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message contenant les donnees d'une carte
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE BOOL WINAPI EnvoiDemandeur (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
    struct_lan_envoi_service *p_data = (PVOID) Param;
    
    LanEnvoiAppli (p_data->ident, BalDemandeur, p_data->service_id, p_data->type_message, p_data->p_msg);
    
    // en renvoyant TRUE, on passe au demandeur suivant
    return TRUE;
}

PROTECTED BOOL LanEnvoiService (short int ident, 
                                enum_lan_service  service_id,
                                enum_lan_type     type_message,
                                void *p_msg)
{
    struct_lan_envoi_service data = { ident, service_id, type_message, p_msg };
    
    // Envoie un message à chaque demandeur du service
    SrvPourChaqueDemandeur(LAN[ident].service[service_id], EnvoiDemandeur, &data);
    
    return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void LanEnvoiAppli(short int ident,
*                          unsigned char bal_dest,
short int msg_etat)
* PARAMETRES:
*     entree: numero du socket
*           : Boite aux lettres destinataire du message
*           : message d'etat
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message vers l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanEnvoiAppli(short int ident,
                             noyau_bal_id bal_dest,
                             enum_lan_service service_id,
                             enum_lan_type type,
                             void *p_msg)
{
    struct_lan_message  *p_msg_emis = (struct_lan_message *)NULL;
    int taille_alloue;
    
    /* en fonction du service */
    switch(service_id )
    {
    case M_LAN_MESSAGE:
        taille_alloue = sizeof(struct_lan_entete) + sizeof(struct_lan_gestion_msg)
            - LAN_LG_TRAME + ((struct_lan_gestion_msg *)p_msg)->longueur;
        ExitAlloue((struct_neutre **)(&p_msg_emis),taille_alloue,
            LAN[ident].lan_pool);
        
        p_msg_emis->entete.service = service_id;
        p_msg_emis->entete.type_message = type;
        p_msg_emis->u.message.longueur = ((struct_lan_gestion_msg *)p_msg)->longueur;
        memcpy(&p_msg_emis->u.message.contenu,
            &((struct_lan_gestion_msg *)p_msg)->contenu,
            ((struct_lan_gestion_msg *)p_msg)->longueur);

        // test si le demandeur est encore valide
        if (Envoie(bal_dest,LAN[ident].lan_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK)
        {
            // on supprime ce demandeur du service !
            LanFinService (ident, service_id, bal_dest);

            ExitLibere((struct_neutre **)(&p_msg_emis));
        }
        break;
        
    case M_LAN_ETAT:
        taille_alloue = sizeof(struct_lan_entete) + sizeof(struct_lan_gestion_etat);
        ExitAlloue((struct_neutre **)(&p_msg_emis),taille_alloue,
            LAN[ident].lan_pool);
        
        p_msg_emis->entete.service = service_id;
        p_msg_emis->entete.type_message = type;
        memcpy(&p_msg_emis->u.msg_etat,(struct_lan_gestion_etat *)p_msg,
            sizeof(struct_lan_gestion_etat));
        // test si le demandeur est encore valide
        if (Envoie(bal_dest,LAN[ident].lan_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK)
        {
            // on supprime ce demandeur du service !
            LanFinService (ident, service_id, bal_dest);

            ExitLibere((struct_neutre **)(&p_msg_emis));
        }
        break;
        
    case M_LAN_FICHIER:
        taille_alloue = sizeof(struct_lan_entete) + sizeof(struct_lan_gestion_fic);
        ExitAlloue((struct_neutre **)(&p_msg_emis),taille_alloue,
            LAN[ident].lan_pool);
        
        p_msg_emis->entete.service = service_id;
        p_msg_emis->entete.type_message = type;
        memcpy(&p_msg_emis->u.param_fic,(struct_lan_gestion_fic *)p_msg,
            sizeof(struct_lan_gestion_fic));
        // test si le demandeur est encore valide
        if (Envoie(bal_dest,LAN[ident].lan_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK)
        {
            // on supprime ce demandeur du service !
            LanFinService (ident, service_id, bal_dest);

            ExitLibere((struct_neutre **)(&p_msg_emis));
        }
        break;
        
    case M_LAN_HORAIRE:
        taille_alloue = sizeof(struct_lan_entete) + sizeof(struct_lan_gestion_tps);
        ExitAlloue((struct_neutre **)(&p_msg_emis),taille_alloue,
            LAN[ident].lan_pool);
        
        p_msg_emis->entete.service = service_id;
        p_msg_emis->entete.type_message = type;
        memcpy(&p_msg_emis->u.msg_tps,(struct_lan_gestion_tps *)p_msg,
            sizeof(struct_lan_gestion_tps));
        // test si le demandeur est encore valide
        if (Envoie(bal_dest,LAN[ident].lan_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK)
        {
            // on supprime ce demandeur du service !
            LanFinService (ident, service_id, bal_dest);

            ExitLibere((struct_neutre **)(&p_msg_emis));
        }
        break;
        
    case M_LAN_ARRET:
        LanEnvoiAcquittement(ident, bal_dest, service_id, type);
        break;

    default:
        break;
    }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int DebutServiceDop(short int ident_lecteur,
*                                           short int service_id,
*                                           short int bal_dest)
* PARAMETRES:
*     entree: numero du lecteur
*           : identificateur du service
*           : boite aux lettres de la tache utilisatrice du service
*     retour: demande valide ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache LAN_ANI
* ROLE: Acceptation ou refus de la demande de debut de service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_valide LanDebutServiceDop(short int ident,
                                             noyau_bal_id bal_demandeur,
                                             enum_espion_sens sens,
                                             enum_espion_nature nature)
{
    enum_lan_type ret;
    struct_lan_service_dop *pSrvData;
    
    ret = LAN_DEBUT_NACQ;
    
    // le demandeur possède deja le service ?
    if (SrvEstDemandeur (LAN[ident].service[M_LAN_DOP], bal_demandeur) == FALSE)
    {
        // le nombre de jetons max est atteind ?
        pSrvData = SrvAjouteDemandeur (LAN[ident].service[M_LAN_DOP], 
            bal_demandeur, 
            sizeof(struct_lan_service_dop));
        if (pSrvData != NULL)
        {
            pSrvData->nature =  nature;
            pSrvData->sens = sens;
            ret = LAN_DEBUT_ACQ;
        }
    }
    
    LanEnvoiAcquittement(ident, bal_demandeur, M_LAN_DOP, ret);
    
    if (ret == LAN_DEBUT_NACQ)
        return LAN_NON_VALIDE;
    
    return LAN_VALIDE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  PROTECTED BOOL LanEnvoiServiceDop (short int ident, 
*                                enum_lan_service  service,
*                                enum_lan_type     type,
*                                struct_lan_message *p_msg)
*
* PARAMETRES:
*     entree: numero du lecteur
*           : service du message
*           : type du message
*           : message
*     retour: TRUE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message contenant les donnees d'une carte
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE BOOL WINAPI EnvoiDemandeurDop (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
    struct_lan_envoi_service_dop *p_param = Param;
    struct_lan_service_dop *p_data = DataDuDemandeur;
    struct_lan_message *p_msg_emis;
    noyau_taille_bloc taille_allouee;
    
    // test si demandeur interesse ou non
    if ( (((p_data->nature) & (p_param->p_msg->u.msg_dop.nature)) != 0) &&
        (p_data->sens == SRV_ESPION_ENTRANT_SORTANT ||
        p_data->sens == p_param->p_msg->u.msg_dop.sens) )
    {
        /* calcul de la taille reelle du message a allouer */
        /* longueur reelle du message et non LAN_LG_TRAME  */
        taille_allouee = sizeof(struct_lan_entete) + sizeof(struct_lan_gestion_dop)
            - LAN_LG_TRAME + p_param->p_msg->u.msg_dop.taille;
        
        ExitAlloue((struct_neutre **)(&p_msg_emis),
            taille_allouee,LAN[p_param->ident].lan_pool);
        
        // recopie du message
        memcpy (p_msg_emis, p_param->p_msg, taille_allouee);
        
        // test si le demandeur est encore valide
        if (Envoie(BalDemandeur,LAN[p_param->ident].lan_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK)
        {
            // on supprime ce demandeur du service !
            LanFinService (p_param->ident, p_param->p_msg->entete.service, BalDemandeur);

            ExitLibere((struct_neutre **)(&p_msg_emis));
        }
    }
    
    // en renvoyant TRUE, on passe au demandeur suivant
    return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:void LanEnvoiDop(short int ident_lecteur,
*                       unsigned char *message,
*                       short int longueur,
*                       unsigned char sens);
* PARAMETRES:
*     entree: numero du lecteur
*           : message a transmettre
*           : longueur du message
*           : sens du message
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Fonction qui transmet les messages du dialogue operateur
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED BOOL LanEnvoiDop(short int ident,
                           unsigned char *message,
                           unsigned short int longueur,
                           enum_espion_sens sens,
                           enum_lan_msg_dop type)
{
    struct_lan_message p_msg;
    struct_lan_envoi_service_dop data = { ident, &p_msg };
    
    // préparation du message
    p_msg.entete.service = M_LAN_DOP;
    p_msg.entete.type_message = LAN_MESSAGE_DOP;
    p_msg.u.msg_dop.type   = type;
    p_msg.u.msg_dop.sens   = sens;

    // determination de la nature en fonction du type
    switch (type)
    {
    case LAN_DOP_MSG:
    case LAN_DOP_FIC:
    case LAN_DOP_TPS:
        p_msg.u.msg_dop.nature = SRV_ESPION_MESSAGE_SERVICE;
        break;
    
    case LAN_DOP_VIE:
    default:
        p_msg.u.msg_dop.nature = SRV_ESPION_MESSAGE_DONNEES;
        break;
    }

    // ajout du type a la taille des données utiles (pour station de maintenance)
    p_msg.u.msg_dop.taille = longueur + sizeof (p_msg.u.msg_dop.type);

    memcpy( p_msg.u.msg_dop.msg, message, longueur);
    
    // Envoie un message à chaque demandeur du service
    SrvPourChaqueDemandeur(LAN[ident].service[M_LAN_DOP], EnvoiDemandeurDop, &data);
    
    return TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: static unsigned char Verif_demande( short int ident_lecteur,short int bal,
*                                              enum_ddm_ident service,
*                                              enum_ltm_type type)
* PARAMETRES:
*     entree: numero de boite aux lettres de la tache demandant une reponse
*           : service demande : dop / etat / carte / message / visualisation
*           : type de demande : debut ou fin de service
*     retour: boleen donnant autorisant ou non la demande
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_valide LanVerifDemande(short int ident,
                                          noyau_bal_id bal,
                                          enum_lan_service service_id)
{
    enum_lan_valide etat = LAN_NON_VALIDE;
    
    /* teste si le demandeur de fin du service est le meme que celui
    qui a ouvert ce service  et si le service est effectivement pris */
    if( SrvEstDemandeur(LAN[ident].service[service_id], bal) == TRUE )    
        etat = LAN_VALIDE;
    
    return(etat);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void LanEnvoiAcquittement(short int ident,
*                                           unsigned char bal_dest,
*                                           short int service_id,
*                                           short int type_message)
* PARAMETRES:
*     entree: numero du socket
*           : boite aux lettres destinataire
*           : identificateur du service
*           : type du message d'acquittement a renvoyer
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message d'acquittement vers l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanEnvoiAcquittement(short int ident,noyau_bal_id bal_dest,
                                    enum_lan_service service_id,enum_lan_type type_message)
{
    struct_lan_entete  *p_entete = (struct_lan_entete *)(0);
    
    ExitAlloue((struct_neutre **)(&p_entete),
        sizeof(struct_lan_entete),LAN[ident].lan_pool);
    
    p_entete->service = service_id;
    p_entete->type_message = type_message;
    
    // test si le demandeur est encore valide
    if (Envoie(bal_dest,LAN[ident].lan_bal,(struct_neutre *)(p_entete)) != NOYAU_OK)
    {
        // pas de FinService ici, sinon on entre en récursion !

        ExitLibere((struct_neutre **)(&p_entete));
    }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void LanEnvoiEmis(short int ident,short int message)
* PARAMETRES:
*     entree: numero du socket
*           : message a transmettre a lan_ios.
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message a la tache LAN_EMISSION
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour LanEnvoiEmis(short int ident,enum_lan_msg_interne type,
                                         struct_lan_gestion_msg *p_msg)
{
    struct_lan_message  *p_message;
    int taille_alloue;
    noyau_enum_retour retour;
    
    retour = NOYAU_BAL_PLEINE;
    
    switch(type)
    {
    case LAN_EMISSION :
        /* pour minimiser l'occipation memoire */
        /* taille de l'entete + longueur du msg + octets de longueur */
        taille_alloue = sizeof(struct_lan_entete) + p_msg->longueur +sizeof(p_msg->longueur)
            + sizeof(p_msg->bal_serv);
        
        ExitAlloue((struct_neutre **)(&p_message),taille_alloue,
            LAN[ident].lan_pool);
        
        p_message->entete.type_message = type;
        p_message->u.message.longueur = p_msg->longueur;
        memcpy(&p_message->u.message.contenu,
            &p_msg->contenu,
            p_msg->longueur);
        
        /* envoie dans la BAL (limit‚e) d‚di‚es aux messages applicatifs */
        retour = Envoie(LAN[ident].emis_bal,LAN[ident].lan_bal,(struct_neutre *)(p_message));
        if (retour == NOYAU_BAL_PLEINE )
        {
            ExitLibere ((struct_neutre **) &p_message);
            LanFichierDebug(ident, "LanEnvoiEmis => BAL Pleine");
        }
        break;
        
    default:
        break;
    }
    
    return retour;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED void LanEnvoiFic(short int ident,
*                                    char *fichier)
* PARAMETRES:
*     entree: numero du socket
*           : nom du fichier a transmettre
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Fonction qui transmet un message a la tache FIC
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour LanEnvoiFic(short int ident,enum_lan_msg_interne type,
                                        struct_lan_gestion_fic *p_param)
{
    struct_interne_fic  *p_msg_fic = (struct_interne_fic *)(0);
    noyau_enum_retour retour;
    
    ExitAlloue((struct_neutre **)(&p_msg_fic),
        sizeof(struct_interne_fic),LAN[ident].lan_pool);
    
    p_msg_fic->entete.type_message = type;
    
    /* parametre concernant le serveur FIC  */
    memcpy(&p_msg_fic->param,p_param, sizeof(struct_lan_gestion_fic));

    retour = Envoie(LAN[ident].fic_bal,LAN[ident].lan_bal,(struct_neutre *)(p_msg_fic));
    if (retour == NOYAU_BAL_PLEINE)
    {
        ExitLibere ((struct_neutre **) &p_msg_fic);
        //      LanFichierDebug(ident, "LanEnvoiFic => BAL Pleine");
    }
    
    return retour;
}



