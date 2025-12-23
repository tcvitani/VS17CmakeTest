/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_GLOB.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Aff/AFF/Includes/Aff_glob.h_v  $
 * 
 *    Rev 1.3   Jul 19 2001 16:54:58   sbatiot
 *  
 * 
 *    Rev 1.2   Oct 06 2000 11:54:44   CL
 * Ajout "unit_address" dans le registre (pour GIT dans un premier temps)
 * Premières versions des modules ERIdan et AAU
 * 
 *    Rev 1.1   Mar 31 2000 14:43:52   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.3   Nov 17 1999 10:54:36   nbl
 *  
 * 
 *    Rev 1.2   Nov 15 1999 09:37:22   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:26   nbl
 *  
 * 
* NBL : 2 display lines in service VISU. Now sends the displaid text
*       when asked and when it changes.
*       Sorry, not very clean, but is this library really clean ?
*
 *    Rev 1.3   Feb 19 1999 18:53:20   afx
 *  
 * 
 *    Rev 1.2   Feb 15 1999 13:53:28   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:24   nbl
 *  
 * 
 *    Rev 1.1   06 Apr 1998 10:43:10   HMO
 *  
 * 
 *    Rev 1.0   Mar 27 1998 11:13:06   pgg
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 *
 * Renamed from SGM to AFF 21.07.1999. sasa - ecsat
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef AFF_GLOB_H
#define AFF_GLOB_H

/*--------------- INCLUDES: ---------------*/
#include <run.h>
#include <csr_srv.h>

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/

#define AFF_NB_TACHE             3

#define NB_AFFICHEURS_MAX        3
#define NB_MAX_LABEL             64

/* timeout d'attente sur la BAL = 5s */
#define AFF_TIMEOUT_INFINI      NOYAU_ATTENTE_INFINIE
#define AFF_TIMEOUT_RAPIDE      180  // 10s
#define AFF_TIMEOUT_LENT        540  // 30s

/* Constante relative … la gestion des interruptions sur liaisons series */
#define IO_EVENT_INT  0

#define AFF_PRESENT_VALIDE    2
#define AFF_VALIDE_PRATIEL    4

#define AFF_EN_COURS          1

// valeur de depart des semaphores lies aux services
// Le code des services arret et dop n'est momentanement prevu que pour
// gerer un seul demandeur de ces services (changement a relaiser lors
// d'une version ulterieure du module).
// C'est pour cela que les deux semaphores ci-dessous de doivent pas avoir
// une valeur superieure a 1.
#define SEM_ARRET       1
#define SEM_ESPION      1

#define SEM_ETAT        8
#define SEM_AFFICHAGE   8
#define SEM_VISU        8

// Valeur du registre pour configuration
#define AFF_REG_KEYv_FICHIER_FORMAT		"FormatFile"
#define AFF_REG_KEYv_UNIT_ADDRESS		"Unit_address"
#define AFF_REG_KEYv_CHARSET_CONVERSION "CharsetConversion"
#define AFF_REG_KEYv_DELAY_BETWEEN_CMD  "DelayCommandMs"
#define AFF_REG_KEYv_TEXT_FONT			"TextFont"

#define MOD_REG_KEYv_STOP_MAILBOX_TIMEOUT_MS "StopModuleReadMailboxTimeoutMs"

/*******************************************/
/*      Gestion du mode DEBUG/TRACES      */
/*******************************************/
#define AFF_NOM_FICHIER_TRACES  "AFF_MATCO"

#define AffFichierDebug AFF_FILE=__FILE__,\
                        AFF_LINE=__LINE__,\
                        DEFINE_AffFichierDebug

/* pour les traces du module */
enum index_traces
{
   AFF_TRC,
   AFF_NB_TRACES
};


/**/
/*--------------- TYPEDEFS ENUM: ---------------*/

typedef enum
{
   PREMIER_TYPE_TACHE = 0,

   AFF_TYPE_TACHE_ANI  = PREMIER_TYPE_TACHE,
   AFF_TYPE_TACHE_IOS,

   BUTEE_TYPE_TACHE,
   DERNIER_TYPE_TACHE = BUTEE_TYPE_TACHE - 1,
   NB_TYPE_TACHE = BUTEE_TYPE_TACHE - PREMIER_TYPE_TACHE
}
enum_type_tache;

/*****************************************************/
/*          gestion du fichier de formatage          */
/*****************************************************/
typedef enum
{
   AFF_IGNORE = -1,
   AFF_CHAINE,
   AFF_NB_FORMAT
}
enum_aff_format;

/*******************************************/
/*          gestion des services           */
/*******************************************/

typedef enum
{
   AFF_AUCUN = -1,  /* pour ne pas confondre avec la BAL 0 */
   AFF_NON_VALIDE = 0,
   AFF_VALIDE,

   AFF_LIBRE = 0, 
   AFF_NON_LIBRE = 1 

}enum_aff_valide;

typedef enum
{
   ACQ = 0,    // envoi d'un acquittement vers l'appli
   NACQ        // envoi d'un non acquittement vers l'appli
}enum_type_acq;

typedef enum
{
   PAS_D_ERREUR = 0,
   ELEMENT_ABSENT,
   ELEMENT_PRESENT,
   LISTE_VIDE,
   ERREUR_D_ALLOC,
   NB_JETONS_DEPASSE
}enum_erreur_liste;


// Structure servant de retour a la fonction RechercheListe.
// Permet de connaitre l'etat d'un service et son demandeur, s'il est present:
// erreur = ELEMENT_PRESENT.
// Sinon, l'erreur seule est retournee dans le champ erreur et la valeur
// des champs etat et demandeur n'est pas significative.
typedef struct
{
   enum_erreur_liste erreur;
   short int         etat;
   noyau_bal_id      demandeur;
} struct_erreur_etat_service;


typedef enum
{
   M_ARRET = 0,

   /* Messages to AFF_IOS */
   M_ETAT = 1,
   M_AFFICHAGE,
   M_ALLUME,

   /* Messages to AFF_ANI */
   M_CHANGEMENT_ETAT = 1,
   M_ACK_AFFICHAGE,
   M_ERR_AFFICHAGE,
// <-- NBL
   M_CHANGEMENT_VISU,
// NBL -->
   M_RESET_HW,
   M_VOYANT,
   M_ARRET_EFFECTUE,
   M_DEVICE_INFO_COMPLETE	 // Device info successfully get from device
}
enum_ani__ios;

typedef struct
{
   short modif_priorite;
}struct_aff_item;

typedef struct
{
	struct_neutre					neutre;
	struct_device_info_item_list	sDeviceInfoList;
}
_AFF_ANI_DEV_INFO_MSG;

/* structures des messages a destination de AFF_IOS */
typedef struct
{
   union
   {
      struct_aff_item   contenu;
      struct_aff_allume allume;
	  struct_aff_voyant voyant;
	  _AFF_ANI_DEV_INFO_MSG	sDevInfoMsg;
   }u;
} struct_message_ani_vers_ios;

typedef struct
{
// <-- NBL
   // unsigned char numero_dernier_ticket/*[CP_LG_NUMERO_TICKET +1]*/;
    char visu_en_cours[AFF_MAX_LIGNES][AFF_MAX_DONNEES_AFFICHAGE];
// NBL -->
	_AFF_ANI_DEV_INFO_MSG	sDevInfoMsg;
}
struct_message_ios_vers_ani;

typedef struct
{
   struct_neutre     neutre;
   enum_ani__ios message_id;
   union
   {
      struct_message_ani_vers_ios  ani_ios;
      struct_message_ios_vers_ani  ios_ani;
   }u;
}struct_ani__ios;



/*************************************************/
/*    Structure caracterisant le p‚ripherique    */
/*************************************************/
typedef struct
{
  unsigned short port;                /* numero de port     */
  struct_b_etat_aff b_etat;           /* Etat de la liaison */
  unsigned short longueur_cmd;        /* longueur d'une commande */
  unsigned short longueur_rep;        /* longueur de la r‚ponse associ‚e */
  unsigned short nb_tentatives;       /* tentatives d'envoi de commande */
  boolean commande;                   /* Commande en cours  */
  boolean ack;                        /* acquittement de la derniere commande ‚mise */
//J  unsigned char num_sequence;         /* numero de sequence */
  unsigned char err_rec;              /* echec en reception */
  unsigned char err_emi;              /* echec en emission  */
  unsigned char bufemi[LG_TRAME_MAX]; /* Buffer d'‚mission   */
  unsigned char bufrec[LG_TRAME_MAX]; /* Buffer de r‚ception */

}
struct_serie_peripherique;

typedef struct
{
	short int inst_id;
	struct_aff_message *p_msg;
}struct_aff_envoi_service;

/**********************************************/
/*  structure globale associ‚e a un afficheur */
/**********************************************/
typedef struct
{
   BOOL						  bAniThreadStopRequest;	// Be cautious to accesss this variable only from ANI thread!
   BOOL						  bIosThreadStopRequest;	// Be cautious to accesss this variable only from IOS thread!

   noyau_priorite_tache       priorite_max;
   noyau_bal_id               ani_bal;
   noyau_bal_id               ios_bal;
   char                       nom_bal_ani[MAX_PATH+1];
   char                       nom_bal_ios[MAX_PATH+1];
   noyau_delai                timeout_bal;
   noyau_pool_id              pool;
   HANDLE		              service [NB_AFF_SERVICE];
   char                       sem_service[NB_AFF_SERVICE];
   dbg_struct_debug           dbg;
   dbg_struct_trace           tab_traces[AFF_NB_TRACES];
   struct_label               tab_label[NB_MAX_LABEL];
   struct_screen			  sScreen;
// <-- NBL
   char                       tab_lignes_courantes[AFF_MAX_LIGNES][AFF_MAX_DONNEES_AFFICHAGE];
// NBL -->
   struct_serie_peripherique  gestion_peripherique;
   struct_tache               taches[AFF_NB_TACHE+1];
   noyau_bal_id               arret_bal;
   DWORD					  unit_address;
   DWORD					  dwTextFormat;
   unsigned char			  char_conv_array[256];
   unsigned char			  inv_char_conv_array[256];
   char						  pcKey[MAX_PATH];
   noyau_delai                delay_command;
   noyau_chrono_id            uiChronoTest;

    /******* MSU**********/
   unsigned char			  pcFichierFormat[MAX_PATH];
   int						  position;
   /*********************/

   unsigned long			  tab_credit[NB_MAX_AFF_PRIO];
   unsigned long			  tab_paid[NB_MAX_AFF_PRIO];
   DWORD					  warning_level;
   DWORD					  alarm_level;

   HWND                       hDlg; //BCH 13/01/2009 In case of a Winaff display, the dialog handle
   
   unsigned int               stopModuleReadMailboxTimeoutMs;
   struct_device_info_item_list device_info;
}
struct_globale_aff;

/**/
/*--------------- VARIABLES: ---------------*/
/* tableau de structure pour NB_AFFICHEURS_MAX modules utilises */
PROTECTED struct_globale_aff AFF[NB_AFFICHEURS_MAX];

/* pour le mode DEBUG */
PROTECTED char *AFF_FILE;
PROTECTED int AFF_LINE;

// buffer circulaire des demandes d'impression
//JPROTECTED struct_aff_buffer aff_buf_recu[TAILLE_BUFFER+1] ;
// indice de lecture dans le buffer d'impression
PROTECTED unsigned char iL_aff_buf_recu
#ifdef LOC_DEF
= 0 ;
#else
;
#endif

// indice d'ecriture dans le buffer d'impression
PROTECTED unsigned char iE_aff_buf_recu
#ifdef LOC_DEF
= 0 ;
#else
;
#endif


// !!!!!! Ne Pas modifier l'ordre des mots cl‚s !!!!!
// autant de mots cl‚s que de champs dans la structure aff_inf !!
// AFF_YEAR doit ˆtre suivi de AFF_YEAR_2_DIGIT, ceci … cause de l'analyse du format ann‚e
PROTECTED struct_mot_cle mots_cles[NB_MAX_MOT_CLE]
#ifdef LOC_DEF
= {
   {"AFF_SOFT_REV"},
   {"AFF_PAYMENT_TYPE"},
   {"AFF_CLASS"},
   {"AFF_LIBELLE1"},
   {"AFF_LIBELLE2"},
   {"AFF_LIBELLE3"},

   {"AFF_FARE_INT"},
   {"AFF_PAID_INT"},
   {"AFF_SALEFARE_INT"},
   {"AFF_SALEPAID_INT"},
   {"AFF_FARE_DEC"},
   {"AFF_PAID_DEC"},
   {"AFF_SALEFARE_DEC"},
   {"AFF_SALEPAID_DEC"},
   {"AFF_YEAR"},             // infos sur la date et l'heure
   {"AFF_YEAR_2_DIGIT"},     // champ ann‚e sur 2 digits, doit ˆtre plac‚ aprŠs AFF_YEAR
   {"AFF_MONTH"},
   {"AFF_DAY"},
   {"AFF_HOUR"},
   {"AFF_MINUTE"},
   {"AFF_SECOND"},
   {"PRECISION_TARIF"},
   {"AFF_AMOUNT_DUE_INT"},
   {"AFF_AMOUNT_DUE_DEC"},
   {"AFF_OVERPAY_INT"},
   {"AFF_OVERPAY_DEC"},
};
#else
;
#endif

HINSTANCE HINSTGLOBAL;

/**/
/*------------------FUNCTIONS:-----------------*/
PROTECTED DWORD WINAPI AffAni(void * pvFoo);
PROTECTED DWORD WINAPI AffIos(void * pvFoo);

PROTECTED enum_instance_result AffInitTrace( char * pcBal, short int cpt_ressource );
PROTECTED void AffFichierTrace (short int ident_aff, char *fmt,...);
PROTECTED void DEFINE_AffFichierDebug (short int ident_aff, char *fmt,...);

PROTECTED void AFFEnvoiMessageEspion( short int ident_aff,
                                   unsigned char *msg_Espion,
                                   short int longueur,
								   enum_espion_nature nature,
                                   enum_espion_sens sens);

/*------------------------------------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif
