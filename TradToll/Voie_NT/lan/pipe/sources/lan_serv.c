/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_SERV.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code des fonctions de demande de service
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_serv.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:50:00   bph
 *  
* 
*    Rev 1.0   Nov 22 1999 14:55:16   PGG
* Checked in from initial workfile by PVCS Version Manager Project Assistant.
* 
*    Rev 1.1   02 Oct 1998 11:46:04   bph
*  
* 
*    Rev 1.19   05 Feb 1998 10:39:16   BPH
* Correction bug NAK sur bal pleine
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

#include <lan_serv.h>

#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>


/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanConnexion(struct_lan_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages applicatifs
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanConnexion(short int ident, struct_lan_message *
                                     p_message)
{
    short int cr_service;
    
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        cr_service = LanDebutService(ident, M_LAN_CONNEXION, p_message->entete.
            neutre.bl_retour);
        
        if (cr_service == LAN_VALIDE)
        {
            LanFichierTrace(ident, "DEBUT CONNEXION recu de BAL %d", p_message->
                entete.neutre.bl_retour);
            
            /* demande de connexion */
            ATM_LAN_Connexion(LAN[ident].atm_lan);
        }
        break;
        
    case LAN_FIN:
        cr_service = LanFinService(ident, M_LAN_CONNEXION, p_message->entete.
            neutre.bl_retour);
        
        if (cr_service == LAN_VALIDE)
        {
            LanFichierTrace(ident, "FIN CONNEXION recu de BAL %d", p_message->
                entete.neutre.bl_retour);
            
            /* deconnexion */
            ATM_LAN_Shutdown(LAN[ident].atm_lan);
        }
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "recept CONNEXION type inconnu %d", p_message->
            entete.type_message);
        break;
    }
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ReceptionLanArret(short int ident,
*                                 struct_lan_message *p_message)
* PARAMETRES:
*     entree: numero de socket
*             pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant l'arret du module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanArret(short int ident, struct_lan_message *p_message)
{
    switch (p_message->entete.type_message)
    {
    case LAN_DEMANDE:
        LanFichierTrace(ident, "Demande ARRET recu de BAL %d", p_message->
            entete.neutre.bl_retour);
        
        /* sauvegarde bal du demandeur */
        LanDebutService(ident, M_LAN_ARRET, p_message->entete.neutre.bl_retour)
            ;
        
        /* deconnexion automatique */
        LanShutDown(ident);
        break;
        
    case LAN_DEMANDE_ACQ:
        LAN[ident].nb_tache_arret++;
        
        /* toutes les taches internes du module ont repondu a l'arret */
        /* sauf la tache courante !!!!                                */
        if (LAN[ident].nb_tache_arret == LAN_NB_TACHE - 1)
        {
            LanEnvoiService(ident, M_LAN_ARRET, LAN_DEMANDE_ACQ, NULL);
            
            LanFinService(ident, M_LAN_ARRET, p_message->entete.neutre.bl_retour
                );
            
            LAN[ident].temoin_arret = TRUE;
        }
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception ARRET type inconnu %d", p_message->
            entete.type_message);
        
        break;
    }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanEtat(struct_lan_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant les demandes d'etat du socket
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanEtat(short int ident, struct_lan_message *p_message)
{
    short int cr_service;
    
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        LanFichierTrace(ident, "DEBUT ETAT recu de BAL %d", p_message->entete.
            neutre.bl_retour);
        
        cr_service = LanDebutService(ident, M_LAN_ETAT, p_message->entete.
            neutre.bl_retour);
        
        if (cr_service == LAN_VALIDE)
        {
            /* remonte l'etat des connexions */
            LanEnvoiAppli(ident, p_message->entete.neutre.bl_retour, M_LAN_ETAT, 
                LAN_NOUVEL_ETAT, &LAN[ident].msg_etat);
        }
        break;
        
    case LAN_FIN:
        LanFichierTrace(ident, "FIN ETAT recu de BAL %d", p_message->entete.
            neutre.bl_retour);
        LanFinService(ident, M_LAN_ETAT, p_message->entete.neutre.bl_retour);
        break;
        
    case LAN_DEMANDE:
        LanFichierTrace(ident, "DEMANDE ETAT recu de BAL %d", p_message->entete
            .neutre.bl_retour);
        
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_ETAT);
        
        if (cr_service == LAN_VALIDE)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_ETAT, LAN_DEMANDE_ACQ);
            
            LanEnvoiAppli(ident, p_message->entete.neutre.bl_retour, M_LAN_ETAT, 
                LAN_NOUVEL_ETAT, &LAN[ident].msg_etat);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_ETAT, LAN_DEMANDE_NACQ);
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception ETAT type inconnu %d", p_message->
            entete.type_message);
        break;
    }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanMessage(struct_lan_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages applicatifs
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanMessage(short int ident, struct_lan_message *
                                   p_message)
{
    short int cr_service;
    
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        LanFichierTrace(ident, "DEBUT MESSAGE recu de BAL %d", p_message->
            entete.neutre.bl_retour);
        cr_service = LanDebutService(ident, M_LAN_MESSAGE, p_message->entete.
            neutre.bl_retour);
        break;
        
    case LAN_FIN:
        LanFichierTrace(ident, "FIN MESSAGE recu de BAL %d", p_message->entete.
            neutre.bl_retour);
        LanFinService(ident, M_LAN_MESSAGE, p_message->entete.neutre.bl_retour)
            ;
        break;
        
    case LAN_EMISSION:
        /* si le service a ‚t‚ ouvert et que la connexion est OK */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_MESSAGE);
        
        /* transmission vers la tache EMISSION */
        p_message->u.message.bal_serv = p_message->entete.neutre.bl_retour;
        
        if (cr_service == LAN_VALIDE && LanEnvoiEmis(ident, LAN_EMISSION, &
            p_message->u.message) == NOYAU_OK)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_MESSAGE, LAN_EMISSION_ACQ);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_MESSAGE, LAN_EMISSION_NACQ);
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception MESSAGE type inconnu %d", p_message->
            entete.type_message);
        break;
    }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanFichier(struct_lan_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Transmettre le messages d'envoi de fichier a la tache FIC
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanFichier(short int ident, struct_lan_message *
                                   p_message)
{
    short int cr_service;
    
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        LanFichierTrace(ident, "DEBUT FICHIER recu de BAL %d", p_message->
            entete.neutre.bl_retour);
        cr_service = LanDebutService(ident, M_LAN_FICHIER, p_message->entete.
            neutre.bl_retour);
        break;
        
    case LAN_FIN:
        LanFichierTrace(ident, "FIN FICHIER recu de BAL %d", p_message->entete.
            neutre.bl_retour);
        cr_service = LanFinService(ident, M_LAN_FICHIER, p_message->entete.
            neutre.bl_retour);
        break;
        
    case LAN_EMISSION:
        /* si le service a ‚t‚ ouvert */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER);
        
        /* message vers la tache FIC */
        p_message->u.param_fic.bal_serv = p_message->entete.neutre.bl_retour;
        
        if (cr_service == LAN_VALIDE && LanEnvoiFic(ident, LAN_ENVOI_FIC, &
            p_message->u.param_fic) == NOYAU_OK)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_FICHIER, LAN_EMISSION_ACQ);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER, LAN_EMISSION_NACQ);
        break;
        
    case LAN_EMISSION_RENOMME:
        /* si le service a ‚t‚ ouvert */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER);
        
        /* message vers la tache FIC */
        p_message->u.param_fic.bal_serv = p_message->entete.neutre.bl_retour;
        
        if (cr_service == LAN_VALIDE && LanEnvoiFic(ident, 
            LAN_ENVOI_RENOMME_FIC, &p_message->u.param_fic) == NOYAU_OK)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_FICHIER, LAN_EMISSION_RENOMME_ACQ);
            
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER, LAN_EMISSION_RENOMME_NACQ);
        break;
        
    case LAN_RECEPTION:
        /* si le service a ‚t‚ ouvert */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER);
        
        /* message vers la tache FIC */
        p_message->u.param_fic.bal_serv = p_message->entete.neutre.bl_retour;
        
        if (cr_service == LAN_VALIDE && LanEnvoiFic(ident, LAN_RECEPTION_FIC, &
            p_message->u.param_fic) == NOYAU_OK)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_FICHIER, LAN_RECEPTION_ACQ);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER, LAN_RECEPTION_NACQ);
        break;
        
    case LAN_RECEPTION_RENOMME:
        /* si le service a ‚t‚ ouvert */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER);
        
        /* message vers la tache FIC */
        p_message->u.param_fic.bal_serv = p_message->entete.neutre.bl_retour;
        
        if (cr_service == LAN_VALIDE && LanEnvoiFic(ident, 
            LAN_RECEPTION_RENOMME_FIC, &p_message->u.param_fic) == NOYAU_OK)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_FICHIER, LAN_RECEPTION_RENOMME_ACQ);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_FICHIER, LAN_RECEPTION_RENOMME_NACQ);
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception FICHIER type inconnu %d", p_message->
            entete.type_message);
        
        break;
    }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanHoraire(struct_lan_message *p_message)
* PARAMETRES:
*     entree: pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Transmettre une demande d'heure a la tache TEMPS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionLanHoraire(short int ident, struct_lan_message *
                                   p_message)
{
    short int cr_service;
    
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        LanFichierTrace(ident, "DEBUT HORAIRE recu de BAL %d", 
            p_message->entete.neutre.bl_retour);
        cr_service = LanDebutService(ident, M_LAN_HORAIRE, 
            p_message->entete.neutre.bl_retour);
        
        if (cr_service == LAN_VALIDE)
        {
            /* transmission vers la tache TPS */
            ATM_LAN_Synchronisation(LAN[ident].atm_lan);
        }
        break;
        
    case LAN_FIN:
        LanFichierTrace(ident, "FIN HORAIRE recu de BAL %d", 
            p_message->entete.neutre.bl_retour);
        cr_service = LanFinService(ident, M_LAN_HORAIRE, 
            p_message->entete.neutre.bl_retour);
        break;
        
    case LAN_DEMANDE:
        /* si le service a ‚t‚ ouvert */
        cr_service = LanVerifDemande(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_HORAIRE);
        
        if (cr_service == LAN_VALIDE)
        {
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
                M_LAN_HORAIRE, LAN_DEMANDE_ACQ);
            
            ATM_LAN_Synchronisation(LAN[ident].atm_lan);
        }
        else
            LanEnvoiAcquittement(ident, p_message->entete.neutre.bl_retour, 
            M_LAN_HORAIRE, LAN_DEMANDE_NACQ);
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception HORAIRE type inconnu %d", 
            p_message->entete.type_message);
        
        break;
    }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void ReceptionLanDop(short int ident,
*                                struct_lan_message *p_message)
* PARAMETRES:
*     entree: numero du socket
*             pointeur sur le message recu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant le Dialogue Operateur
* --------------------------------------------------------------------
* $F_FCTN
(*/
PROTECTED void ReceptionLanDop(short int ident, struct_lan_message *p_message)
{
    switch (p_message->entete.type_message)
    {
    case LAN_DEBUT:
        LanFichierTrace(ident, "DEBUT DOP recu de BAL %d, liaison '%d'", 
            p_message->entete.neutre.bl_retour, p_message->u.msg_dop.nature);
        LanDebutServiceDop(ident, p_message->entete.neutre.bl_retour, 
            p_message->u.msg_dop.sens, p_message->u.msg_dop.nature);
        break;
        
    case LAN_FIN:
        LanFichierTrace(ident, "FIN DOP recu de BAL %d", 
            p_message->entete.neutre.bl_retour);
        LanFinService(ident, M_LAN_DOP, p_message->entete.neutre.bl_retour);
        break;
        
    default:
        /* reception d un message errone : message_id inconnu ... */
        LanFichierDebug(ident, "reception DOP type inconnu %d", 
            p_message->entete.type_message);
        
        break;
    }
}
