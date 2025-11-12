/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_LOC.H
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef NOY_LOC_H
#define NOY_LOC_H


/*--------------- INCLUDES ---------------*/
//#include "tcp_ip.h"
#include "noy_cro.h"
#include "debug.h"

/*--------------- RESERVED: ---------------*/
#include "protect.h"

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES STATIQUES : ---------------*/

#define VECTEUR_PEMM 0xFA

/*** S‚curite d'une allocation minimum si n‚cesaire  ***/
#define TAILLE_MINI_MEMOIRE  16       
#define MAX_THREADS 40
#define MAX_MAILSLOTS 100
#define MAX_TAILLE_MESSAGE 0 // pas de limite de taille (< 64Ko en pratique)
   
#define RESIZE_TIMEOUT(t) {if (t > INFINITE/55UL) t = INFINITE; else t *= 55UL; }


/*--------------- DEFINITIONS: ---------------*/

PROTECTED enum index_traces
{
   NOY_INIT,
   NOY_BAL,
   NOY_CRO,
   NOY_EVT,
   NOY_MEM,
   NOY_BLOC,
   NOY_RES,
   NOY_RGN,
   NOY_SEM,
   NOY_TCH,
   NOY_TCOU,
   NOY_PILE,
   NOY_NB_TRACES,
   NOY_ERREUR = NOY_NB_TRACES
};
/*
typedef struct
{
   unsigned char *p_start;
   noyau_taille_bloc taille;
   noyau_taille_bloc profondeur;
}
struct_pile;
*/
/*
typedef struct
{
   unsigned int nb_pages_4ko;
   void * adresse_virtuelle;
   unsigned long adresse_physique;
   unsigned int handler_XMS;
}
noyau_struct_pool_XMS;

typedef struct
{
   unsigned long taille;
   unsigned long adresse;
   noyau_taille_bloc alloue;
   noyau_taille_bloc max_alloue;
}
noyau_struct_pool;

typedef struct
{
   int Index;
   int Count;
} _tObjId;

typedef struct
{
   void *adr;
   noyau_taille_bloc taille;
   noyau_pool_id pool;
   noyau_tache_id tache;
   _tObjId seg_id;
}
noyau_struct_bloc;
*/
typedef struct
{
   LARGE_INTEGER  DueTime;
   noyau_delai    duree;
   noyau_delai    periode;
   HANDLE         hTimer;
   noyau_bal_id   bal_retour;
   HANDLE         hBal;
   DWORD          compteur;
   PVOID          args;
}
noyau_struct_chrono;

typedef struct 
{
    DWORD dwMessageCount;
    DWORD dwSizeCount;
    DWORD dwNbMessagesMax;
    noyau_bal_id bal_id;
    char BalName[MAX_PATH];
} noyau_struct_infos_mailslot;

typedef struct
{
   /* parametres pour regions */
   signed short imbrication_region;
   DWORD thread_en_region;

   /* parametres reserves aux traces et debug */
   unsigned long debug_ticks;

   /* pour trace de la tache courante */
//   noyau_tache_id tache_courante;

    noyau_struct_infos_mailslot sMailslotInfos[MAX_MAILSLOTS];
}
struct_global_noyau;

typedef struct
{
   // partie privée du debug à cause du pointeur vers le tableau de traces
   dbg_struct_debug dbg;
   dbg_struct_trace tab_traces[NOY_NB_TRACES];
   
   HANDLE hRegion; // mutex inter-process
   HANDLE hRegionNoyau; // mutex inter-process
   CRITICAL_SECTION hLocalRegion; // region locale au process
   CRITICAL_SECTION csChrono;
   CRITICAL_SECTION csDbg;

   /* parametres pour regions */
   signed short imbrication_region_locale;
   DWORD thread_en_region_locale;

   HANDLE hMailslotRead[MAX_MAILSLOTS];
   HANDLE hMailslotWrite[MAX_MAILSLOTS];

   /*** parametres reserves aux chronomètres ***/
   noyau_struct_chrono chrono[MAX_CHRONO];
   HANDLE hThreadChrono;
   DWORD ThreadChronoId;
}
struct_local_noyau;

/*--------------- VARIABLES: ---------------*/

// début de section de données partagées
#include "global.h"

   PROTECTED struct_global_noyau _NOYAU_ INIT(0);

   PROTECTED WORD NOYAU_NB_INSTANCES INIT(0);

// fin de section de données partagées
#include "global.h"

PROTECTED struct_local_noyau _noyau_;

/*--------------- FUNCTIONS: ---------------*/

/*** fait appel … toutes les fonctions de traces ***/
PROTECTED void TraceTout (void);

/*** fonction de debug des blocs restant allou‚s dans les pools ***/
PROTECTED void TraceBlocs (void);

/*** fonction de debug des messages restant dans les BAL ***/
PROTECTED void TraceBALs (void);

/*** fonction de debug des ‚tats des taches ***/
PROTECTED void TraceTaches (WORD nb_taches);

/*** fonction de debug des regions ***/
PROTECTED void TraceRegions (short int imbrications);

/*** fonction de debug des regions locales ***/
PROTECTED void TraceRegionsLocales (short int imbrications);

/*** Renvoie VRAI si InitNoyau a ete execute avec retour OK */
PROTECTED noyau_enum_booleen NoyauEstInitialise (void);

PROTECTED unsigned long IncrementeCompteurDebug(void);

PROTECTED noyau_enum_retour InitMem(char *error_file);

PROTECTED noyau_enum_retour InitBAL(void);

PROTECTED noyau_enum_retour InitRegion (void);

PROTECTED void EnterRegion (void);

PROTECTED void LeaveRegion (void);

#undef PROTECTED
#undef I
#undef INIT
#endif
