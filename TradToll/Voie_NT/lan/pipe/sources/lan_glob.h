/*------   (v) 1998 CS-Route  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL LAN
* FICHIER: lan_glob.h
* LANGAGE: C 
* --------------------------------------------------------------------
* RESUME: Fichier interne au module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations de constantes et variables globales
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/includes/lan_glob.h_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:14   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:35:48   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:46   bph
 *  
 * 
 *    Rev 1.22   05 Feb 1998 10:34:56   BPH
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef LAN_GLOB_H
#define LAN_GLOB_H

/*--------------- INCLUDES: ---------------*/

#include <debug.h>
#include <csr_srv.h>
#include <atm_lan.h>

#include <protect.h>

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

/* version */
#define LAN_VERSION                 "1.00"        //RTC "5.02"
#define LAN_COMMENT                 "NOYAU 1.00"

#define LAN_NOM_FICHIER_TRACES  "LAN"

#define LanFichierDebug LAN_FILE=__FILE__,\
                        LAN_LINE=__LINE__,\
                        DEFINE_LanFichierDebug

#define LAN_NB_TACHE                      5

#define LAN_MAX_SERVEUR                   5
#define LAN_NB_IDENT_MAX                  3


#define LAN_NO_PIPE_OPENED                INVALID_HANDLE_VALUE

#define LAN_TIMEOUT_RECOIT                5   /* ticks */

#define MOD_REG_KEYv_MAX_SLEEP_TIME_MS          "MaxSleepTimeMs"

typedef enum
{
   LAN_NON_VALIDE = 0,
   LAN_VALIDE
}
enum_lan_valide;

/*----------------TYPEDEF:----------------*/

/* pour les traces du module */
enum index_traces
{
   LAN_TRC,
   LAN_NB_TRACES
};

typedef enum
{
   LAN_NOK = 0,
   LAN_TIMEOUT,
   LAN_OK
}enum_lan_erreur;

typedef enum
{
   LAN_MSG_INCONNU = 0,
   LAN_ENVOI_FIC,
   LAN_ENVOI_RENOMME_FIC,
   LAN_RECEPTION_FIC,
   LAN_RECEPTION_RENOMME_FIC,
} enum_lan_msg_interne;

/* Erreurs FIC */
typedef enum
{
  FIC_OK,
  FIC_ERR_TRANSFERT_PUT,
  FIC_ERR_TRANSFERT_GET
}fic_enum_retour;

typedef struct
{
   struct_lan_entete      entete;
   struct_lan_gestion_fic param;
}struct_interne_fic;

typedef struct
{
   char nom_distant[LAN_MAX_SERVEUR][LAN_MAX_CAR];
   HANDLE pipe;
   OVERLAPPED rOver, wOver;
   HANDLE rEvent, wEvent;
   char user_fic[LAN_MAX_CAR];
   char password_fic[LAN_MAX_CAR];
   char num_msg;     /* pour reconnaitre chaque serveur parmi la liste */
   char num_fic;     /* des serveurs possibles LAN_MAX_SERVEUR */
}struct_lan_parametre;

typedef struct
{
   long msg_vie;   /* periode pour l'emission du msg de vie */
   long rep_vie;   /* delai de non reponse au message de vie */
}struct_lan_periode;

typedef struct
{
    struct_prot prot;
    BYTE data[LAN_LG_TRAME];
} struct_lan_msg;

/* structure globale associee au reseau */
typedef struct
{
	noyau_priorite_tache lan_priorite_max;
	noyau_pool_id lan_pool;
	char nom_lan_bal[MAX_PATH + 1];
	char nom_emis_bal[MAX_PATH + 1];
	char nom_fic_bal[MAX_PATH + 1];
	noyau_bal_id lan_bal;
	noyau_bal_id emis_bal;
	noyau_bal_id fic_bal;

	short int nb_tache_arret;
	boolean temoin_arret;

	struct_lan_gestion_etat msg_etat;

	HANDLE service[NB_LAN_SERVICE];
	struct_tache taches[LAN_NB_TACHE + 1];

	struct_lan_parametre param;
	struct_lan_periode  periode;
	dbg_struct_debug dbg;
	dbg_struct_trace tab_traces[LAN_NB_TRACES];

    struct_lan_msg read_message;

    HANDLE atm_lan;

    DWORD cpt_timeout_msg;
    DWORD max_cpt_timeout_msg;

    noyau_chrono_id chrono_attente_msg;
    noyau_delai duree_chrono_attente_msg;
    
    noyau_chrono_id chrono_reconnexion;
    noyau_delai duree_chrono_reconnexion;

    LONG max_diff_time;

	DWORD dwSleepTimeMs;

}struct_globale_lan;

/*-------------------- variables ---------------- */

#include "global.h"

    /* tableau de structure */
    PROTECTED struct_globale_lan LAN[LAN_NB_IDENT_MAX] INIT(0);

    PROTECTED WORD LAN_NB_INSTANCES INIT(0);

#include "global.h"

/* pour le mode DEBUG */
PROTECTED char *LAN_FILE;
PROTECTED int LAN_LINE;

/* pour une region locale au module */
CRITICAL_SECTION CS;

/* -------------- FONCTIONS: --------------*/

PROTECTED DWORD WINAPI Lan_ani( LPVOID ); 
PROTECTED DWORD WINAPI LanEmission( LPVOID ); 
PROTECTED DWORD WINAPI LanReception( LPVOID ); 
PROTECTED DWORD WINAPI LanFic( LPVOID ); 

PROTECTED enum_instance_result LanInitTrace (char * pcBal, short int cpt_ressource );
PROTECTED void LanFichierTrace (short int ident_lecteur,char *fmt,...);
PROTECTED void DEFINE_LanFichierDebug (short int ident_lecteur,char *fmt,...);

PROTECTED enum_lan_erreur LanConnect(short int ident, struct_lan_parametre *);
PROTECTED enum_lan_erreur LanDisconnect(short int ident, struct_lan_parametre *);
PROTECTED enum_lan_erreur LanShutDown(short int /*ident*/);
PROTECTED void LanBreak( short int /*ident*/);

PROTECTED enum_lan_erreur LanTransmit(short int ident,
                                  HANDLE /*pipe*/,
                                  struct_lan_msg * /*buffer*/);
                                  
PROTECTED enum_lan_erreur LanReceive(short int ident,
                                 HANDLE /*pipe*/,
                                 struct_lan_msg * /*buffer*/,
                                 DWORD /*longueur*/,
                                 DWORD /* delai */);

PROTECTED enum_lan_erreur LanEnvoiMessage (short int ident, DWORD MsgSize, enum_prot_type MsgType, void *data);

PROTECTED void LanTraitementErreur(void);

PROTECTED void _DebutRegion(void);
PROTECTED void _FinRegion(void);

/*------------------VARIABLES:-----------------*/


#undef LOC_DEF
#undef PROTECTED
#undef I
#undef INIT
#endif