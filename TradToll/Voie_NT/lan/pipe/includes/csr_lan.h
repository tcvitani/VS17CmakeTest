/*------   (v) 1998 CS-Route  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL LAN
* FICHIER: lan.h
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module LAN
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Includes/csr_lan.h_v  $
 * 
 *    Rev 1.2   Jun 04 2002 14:41:36   hmoreau
 *  
 * 
 *    Rev 1.1   Sep 08 2000 14:48:46   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:12   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   09 Apr 1999 16:27:50   afx
 *  
 * 
 *    Rev 1.3   Jan 20 1999 11:35:46   bph
 *  
 * 
 *    Rev 1.2   Oct 14 1998 14:18:32   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:46   bph
 *  
 * 
 *    Rev 1.12   03 Nov 1997 11:13:50   BPH
 * Passage en ExitAlloue, ExitLibere, ExitEnvoie
 * Utilisation du debug noyau 6
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef LAN_H
#define LAN_H

/*--------------- INCLUDES: ---------------*/
#include <run.h>
#include <module.h>

/*--------------- RESERVED: ---------------*/

#ifdef LAN_DEF
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/
#define LAN_MAX_CAR                 MAX_PATH
//#define LAN_LG_TRAME                2048
#define LAN_LG_TRAME                4096

/*****************************************/
/* parametres de configuration du module */
/* dans la registry                      */
/*****************************************/

#define MOD_REG_KEYv_CLIENT                     "_Client"
#define MOD_REG_KEYv_IP_MASK                    "_IP_Mask"
#define MOD_REG_KEYv_MSG_SERVER                 "_MSG_Server"
#define MOD_REG_KEYv_FILE_SERVER                "_FILE_Server"
#define MOD_REG_KEYv_TIME_SERVER                "_TIME_Server"
#define MOD_REG_KEYv_FILE_USER                  "_File_User"
#define MOD_REG_KEYv_FILE_PASSWORD              "_File_Password"
#define MOD_REG_KEYv_KEEP_ALIVE                 "_Keep_Alive"
#define MOD_REG_KEYv_MAX_KEEP_ALIVE_COUNTER     "_Max_Keep_Alive_Counter"
#define MOD_REG_KEYv_RESPONSE_TO                "_Response_Time_Out"
#define MOD_REG_KEYv_CONNECTION_DELAY           "_Connection_Delay" 
#define MOD_REG_KEYv_MAX_DIFF_TIME              "_Max_Diff_Time"

/*--------------- TYPEDEFS: ---------------*/

/* erreurs possibles pour les retour des fonctions du module  */
typedef enum
{
   LAN_CONFIG_RESEAU_OK,
   LAN_CONFIG_ERR_ADRESSE,
   LAN_CONFIG_ERR_NOM,
   LAN_CONFIG_ERR_PORT,
   LAN_CONFIG_ERR_MASK,
   LAN_CONFIG_ERR_FICHIER,
   LAN_CONFIG_ERR_USER,
   LAN_CONFIG_ERR_PASSWORD,
   LAN_CONFIG_ERR_SERVEUR,
   LAN_CONFIG_PERIODE_OK,
   LAN_CONFIG_ERR_PERIODE

}enum_lan_retour;

/* liste des services disponibles */
typedef enum
{
   PREMIER_LAN_SERVICE = 0,

   M_LAN_ARRET  = PREMIER_LAN_SERVICE,
   M_LAN_ETAT,
   M_LAN_DOP,       // Multi-jetons
   M_LAN_CONNEXION,
   M_LAN_MESSAGE,
   M_LAN_FICHIER,
   M_LAN_HORAIRE,
   M_LAN_VIDER_BAL,

   BUTEE_LAN_SERVICE,
   DERNIER_LAN_SERVICE = BUTEE_LAN_SERVICE - 1,
   NB_LAN_SERVICE = BUTEE_LAN_SERVICE - PREMIER_LAN_SERVICE
} enum_lan_service;

/* liste des types de messages */
typedef enum
{
   PREMIER_LAN_TYPE = 0,

   LAN_DEBUT = PREMIER_LAN_TYPE,    /*************************************/
   LAN_DEBUT_ACQ,                   /* Sens des Messages  :              */
   LAN_DEBUT_NACQ,                  /* de l'application vers le module   */
   LAN_FIN,                         /* LAN_DEBUT     : tous les services */
   LAN_FIN_ACQ,                     /* LAN_FIN       : tous les services */
   LAN_FIN_NACQ,                    /* LAN_DEMANDE   : arret/etat        */
   LAN_DEMANDE,                     /* LAN_EMISSION  : message /fichier  */
   LAN_DEMANDE_ACQ,                 /* LAN_RECEPTION : fichier           */
   LAN_DEMANDE_NACQ,                /*************************************/
   LAN_NOUVEL_ETAT,
   LAN_MESSAGE_DOP,

   LAN_EMISSION,
   LAN_EMISSION_ACQ,
   LAN_EMISSION_NACQ,
   LAN_EMISSION_EFFECTUEE,
   LAN_EMISSION_NON_EFFECTUEE,
   LAN_RECEPTION,
   LAN_RECEPTION_ACQ,
   LAN_RECEPTION_NACQ,
   LAN_RECEPTION_EFFECTUEE,
   LAN_RECEPTION_NON_EFFECTUEE,

   LAN_EMISSION_RENOMME,              /********************************/
   LAN_EMISSION_RENOMME_ACQ,          /*                              */
   LAN_EMISSION_RENOMME_NACQ,         /* propre au service FICHIER    */
   LAN_EMISSION_RENOMME_EFFECTUEE,    /* pour un transfert de fichier */
   LAN_EMISSION_RENOMME_NON_EFFECTUEE,/*  avec renommage              */
   LAN_RECEPTION_RENOMME,             /*                              */
   LAN_RECEPTION_RENOMME_ACQ,         /********************************/
   LAN_RECEPTION_RENOMME_NACQ,
   LAN_RECEPTION_RENOMME_EFFECTUEE,
   LAN_RECEPTION_RENOMME_NON_EFFECTUEE,

   BUTEE_LAN_TYPE,
   DERNIER_LAN_TYPE = BUTEE_LAN_TYPE - 1,
   NB_LAN_TYPE = BUTEE_LAN_TYPE - PREMIER_LAN_TYPE
} enum_lan_type;

/*--------------------------------------------------------------------*/
                     /* REMARQUE IMPORTANTE */
/* TOUTES LES STRUCTURES DEFINIES CI-DESSOUS AYANT LE QUALIFICATIF "EXTERNE" */
/* PERMETTENT A L'APPLICATION D'OPTIMISER LA GESTION DES ALLOCATIONS MEMOIRE */
/* A LA FOIS DANS LA PILE ET DANS LES POOLS CAR CES STRUCTURES DEFINISSENT */
/* LE MINIMUM, ELLES NE PEUVENT ETRES UTILISEES QUE LORS D'UN ENVOI        */
/* D'UN MESSAGE VERS LE MODULE LAN AVEC LA FONCTION LANENVOI() FOURNIE  */
/*--------------------------------------------------------------------*/


/*******************************/
/*      Service  DOP           */
/* type des messages espionnes */
/*******************************/
/* messages attendus */
typedef enum
{
   LAN_DOP_VIE = 0x1,
   LAN_DOP_MSG = 0x2,
   LAN_DOP_FIC = 0x4,
   LAN_DOP_TPS = 0x8
}enum_lan_msg_dop;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
   enum_espion_nature  nature;  /* donnees ou protocole */
   enum_espion_sens    sens;    /* entrant et/ou sortant */
   unsigned short int  taille;
   enum_lan_msg_dop    type;    /* VIE, MSG, FIC ou TPS */
   unsigned char       msg[LAN_LG_TRAME];
}struct_lan_gestion_dop;
#pragma pack(pop)

typedef struct
{
   enum_espion_nature  nature;  /* donnees ou protocole */
   enum_espion_sens    sens;   /* entrant et/ou sortant */
}struct_lan_dop_externe;

/*******************************/
/*      Service  ETAT          */
/* type des messages espionnes */
/*******************************/
/* liste des serveurs attendus */
typedef enum
{
   LAN_SERVEUR_MSG = 0,
   LAN_SERVEUR_FIC,
   LAN_SERVEUR_TPS
}enum_lan_serveur;

/* etat possibles pour les differentes connexions */
typedef enum
{
   LAN_LIAISON_OK = 0,
   LAN_ATTENTE_CONNEXION,
   LAN_CONNEXION,
   LAN_DEGRADE,
   LAN_TRANSFERT_FICHIER_OK,
   LAN_TRANSFERT_FICHIER_NOK,
   LAN_LIAISON_FICHIER_HS,
   LAN_LIAISON_TPS_HS
}enum_etat_liaison;

typedef struct
{
   short int          bal_serv;  /* reserve */
   enum_etat_liaison  serveur_msg_old;
   enum_etat_liaison  serveur_fic_old;
   enum_etat_liaison  serveur_tps_old;
   enum_etat_liaison  serveur_msg;
   enum_etat_liaison  serveur_fic;
   enum_etat_liaison  serveur_tps;
}struct_lan_gestion_etat;


/*******************************/
/*      Service  CONNEXION     */
/* type des messages echanges  */
/*******************************/
typedef struct
{
   int periode_reconnexion;
}struct_lan_gestion_connexion;


/*******************************/
/*      Service MESSAGE        */
/* type des messages echanges  */
/*******************************/
typedef struct
{
   noyau_bal_id bal_serv;    /* reserve */
   int       longueur;
   char      contenu[LAN_LG_TRAME];
}struct_lan_gestion_msg;


typedef struct
{
   int   longueur;
   char  *contenu;
}struct_lan_msg_externe;


/*******************************/
/*      Service  FICHIER       */
/* type des messages echanges  */
/*******************************/
typedef struct
{
   noyau_bal_id bal_serv;              /* reserve */
   char      fichier[_MAX_PATH]; /* chemin complet du fichier a transferer */
   char      path[_MAX_PATH];    /* repertoire de transfert sur le distant        */
   char      hostname[LAN_MAX_CAR];    /* ="0" pour serveur par defaut */
}struct_lan_gestion_fic;

typedef struct
{
   char  *fichier;    /* chemin complet du fichier */
   char  *path;       /* repertoire du distant     */
   char  *hostname;   /* ="\0" pour serveur par defaut */
}struct_lan_fic_externe;

/*******************************/
/*      Service  HORAIRE       */
/* type des messages echanges  */
/*******************************/
typedef struct
{
   noyau_bal_id bal_serv;                 /* reserve */
   char        hostname[LAN_MAX_CAR];    /* ="0" pour serveur par defaut */
   signed long difference_tps;           /* nombre de secondes */
}struct_lan_gestion_tps;

typedef struct
{
   char *hostname;    /* ="0" pour serveur par defaut */
}struct_lan_tps_externe;

/***********************************/
/*      Messages inter-taches      */
/***********************************/
/* structure de l'entete */
typedef struct
{
   struct_neutre       neutre;
   enum_lan_service    service;
   enum_lan_type       type_message;
} struct_lan_entete;

/* structure globale */
typedef struct
{
   struct_lan_entete    entete;
   union
    {
         struct_lan_gestion_connexion   msg_connexion;
         struct_lan_gestion_etat  msg_etat;
         struct_lan_gestion_dop   msg_dop;
         struct_lan_gestion_msg   message;
         struct_lan_gestion_fic   param_fic;
         struct_lan_gestion_tps   msg_tps;
    }u;
}struct_lan_message;

/* structure globale pour l'applicatif pour l'envoi vers le module uniquement*/
typedef struct
{
   struct_lan_entete    entete;
   union
    {
         struct_lan_gestion_connexion   msg_connexion;
         struct_lan_gestion_etat  msg_etat;
         struct_lan_dop_externe   msg_dop;
         struct_lan_msg_externe   message;
         struct_lan_fic_externe   param_fic;
         struct_lan_tps_externe   msg_tps;
    }u;
}struct_lan_message_externe;

/*--------------- FUNCTIONS: ---------------*/

EXPORT enum_instance_result WINAPI LanLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );

EXPORT enum_instance_result WINAPI LanArret(noyau_bal_id iBalId);

EXPORT noyau_enum_retour WINAPI EnvoiLan(noyau_bal_id bal_dest, noyau_bal_id bal_source, noyau_pool_id pool,
                     struct_lan_message_externe *p_message);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* LAN_H */
