/*------    ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOYAU.H
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef NOYAU_H
#define NOYAU_H

/*--------------- EXTERNALS: ---------------*/
#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include "csrlc32.h"

#ifdef NOY_DEF
   #include <public.h>
#else
   #include <export.h>
#endif

/*--------------- DEFINES: ---------------*/

#define NOYAU_VALEUR_TICK  55       /* duree d'un tick : 55 ms */
#define NOYAU_NB_TICK_PAR_SEC   18  /* 18 ticks/seconde (base 55 ms) */

#define NOYAU_BAL_HORLOGE_ID 0

#define NOYAU_ATTENTE_INFINIE INFINITE   // utilisé avec l'enum noyau_delai
#define NOYAU_SANS_ATTENTE 0             // utilisé avec l'enum noyau_delai

#define NOYAU_BAL_ILLIMITEE 0            // pas de limte de messages pendings dans une BAL 

/* debug dans un fichier texte et/ou a l'ecran */
#define NOYAU_EXTENSION_TRACES      ".TRC"
#define NOYAU_EXTENSION_ECRAN       ".ECR"
#define NOYAU_EXTENSION_ERREURS     ".ERR"
  
// 
// LISTE DES FICHIERS DE TRACES DU NOYAU
//

// NOY_INIT.TRC : nbre d'instances du noyau
// NOY_BAL.TRC  : fct PublieBAL, AttendBAL, SupprimeBAL, ExitEnvoie, Recoit, TestRecoit
// NOY_CRO.TRC  : fct AlloueChrono, LibereChrono, LanceChrono, ArretChrono, TestChrono
// NOY_EVT.TRC  : fct AlloueEvt, LibereEvt, SignalEvt, AttenteEvt, TEstEvt, EffaceEvt
// NOY_MEM.TRC  : fct ExitAlloue, ExitLibere
// NOY_BLOC.TRC : non implémenté sous NT (liste les blocs mémoire alloués)
// NOY_RES.TRC  : toutes fct du réseau (socket)
// NOY_RGN.TRC  : fct DebutRegion, FinRegion
// NOY_SEM.TRC  : fct AlloueSem, LibereSem, AttendPriseSemaphore, RendSemaphore...
// NOY_TCH.TRC  : fct LanceTache, ArretTache, EtatTache, TermineTache, ChangePriorite
// NOY_TCOU.ECR : non implémenté sous NT (affiche tache courante a l'écran)
// NOY_PILE.TRC : non implémenté sous NT (teste les débordements de pile)

#define NOYAU_REPERTOIRE_APP		CSR_REG_KEYv_APPPATH
#define NOYAU_REPERTOIRE_TRACES     CSR_REG_KEYv_TRACEPATH
#define NOYAU_FICHIER_TRACES_ECRAN  "NOYAU"

#define NOYAU_REPERTOIRE_ERREURS    CSR_REG_KEYv_ERRORPATH
#define NOYAU_FICHIER_ERREURS       "NOYAU"

#define NOYAU_FICHIER_NO_EXIT_BAD   NOYAU_REPERTOIRE_ERREURS "\\NO_EXIT.BAD"

#define NOYAU_FICHIER_TAILLE_LIMITE CSR_REG_KEYv_FILEMAXSIZE

#define NOYAU_REG_KEYv_PRIO_INIT	"PriorityInit"				// Dword
#define NOYAU_REG_KEYv_PRIO_MAX		"PriorityMax"				// Dword
#define NOYAU_REG_KEYv_POOL			"Pool"						// Dword

// Fonction de récupération d'un n° de pool (Ici, atoi retourne 0)
#define NOYAU_GetPoolId(x)  ((noyau_pool_id)(atoi ("Réserve pour extension future !")))

#define	NOYAU_MapPriority(x)  ((x==1) ? THREAD_PRIORITY_IDLE			\
			   				: ((x==2) ? THREAD_PRIORITY_LOWEST			\
							: ((x==3) ? THREAD_PRIORITY_BELOW_NORMAL	\
							: ((x==4) ? THREAD_PRIORITY_NORMAL			\
							: ((x==5) ? THREAD_PRIORITY_ABOVE_NORMAL	\
							: ((x==6) ? THREAD_PRIORITY_HIGHEST			\
							: ((x==7) ? THREAD_PRIORITY_TIME_CRITICAL	\
							: THREAD_PRIORITY_NORMAL					\
							)))))))

#define NOYAU_VIDE_TACHE(s)					ZeroMemory(&(s),sizeof(s))

#define NOYAU_INIT_TACHE(s,a,b,c,d,e,f,g)   { s.noyau_tache_active=a;	\
										      s.noyau_priorite_tache=b;	\
										      s.noyau_taille_pile=c;	\
										      s.noyau_adresse_tache=d;	\
										      s.parametre=e;			\
										      s.noyau_id_tache=f;       \
                                              strcpy_s (s.nom_tache, MAX_PATH, g); }

// Some C99 compatibility defines
#if _MSC_VER <= 1200
#define snprintf _snprintf
#endif

/*--------------- TYPEDEFS: ---------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*** Valeurs de retour des fonctions de la librairie ***/

typedef enum
{
    NOYAU_NOK,
    NOYAU_OK,

    NOYAU_BAL_MESS,
    NOYAU_BAL_PLEINE,
    NOYAU_BAL_VIDE,
    NOYAU_BAL_STOP,
    NOYAU_BAL_TIME,
    NOYAU_INIT_BAL_NOK,

    NOYAU_IOS_OK,
    NOYAU_IOS_INIT_NOK,
    NOYAU_IOS_INSTALL_NOK,
    NOYAU_IOS_RES_NOK,

    NOYAU_PB_MALLOC,

    NOYAU_FICHIER_ERREURS_NOK,
    NOYAU_FICHIER_POOLS_NOK,
    NOYAU_KERNEL_NOK,
    NOYAU_PEMM_INIT_NOK,
    NOYAU_POOL_NOK,

    NOYAU_INIT_TACHE_NOK,
    NOYAU_START_TACHE_NOK,
    NOYAU_ARRET_TACHE_OK,
    NOYAU_ARRET_TACHE_NOK,

    NOYAU_EXEC_TACHE_NOK,

    NOYAU_REGION_MAX,

    NOYAU_DELAY_TIMEOUT,
    NOYAU_DELAY_STOP,
    NOYAU_DELAY_IN_REGION,
    NOYAU_DELAY_TASK,
    NOYAU_DELAY_NULL,

    NOYAU_EVT_TIMEOUT,
    NOYAU_EVT_EXCEPTION,
    NOYAU_EVT_REGION,
    NOYAU_EVT_TASK,

    NOYAU_SIGNAL_TACHE,
    NOYAU_MAUVAISE_TACHE,
    NOYAU_MAUVAISE_PRIORITE,

    NOYAU_TACHE_COURANTE,
    NOYAU_TACHE_PRETE,
    NOYAU_TACHE_EN_ATTENTE,
    NOYAU_TACHE_NON_PRETE,

    NOYAU_SEM_DELAI,
    NOYAU_SEM_ABSENT,
    NOYAU_SEM_NOK
}
noyau_enum_retour;

typedef enum
{
    NOYAU_OBJET_0           = WAIT_OBJECT_0,
    NOYAU_OBJET_ABANDON_0   = WAIT_ABANDONED_0,
    NOYAU_OBJET_TIMEOUT     = WAIT_TIMEOUT,
    NOYAU_OBJET_ERREUR      = WAIT_FAILED
}
noyau_objet_retour;

/*-------------------------------------------------------------
 * Structure de communication entre les taches
 * (par boites aux lettres) : base commune a tout message.
 --------------------------------------------------------------*/

typedef DWORD noyau_bal_id;
typedef HANDLE noyau_tache_id;
typedef UINT noyau_chrono_id; 
typedef INT noyau_priorite_tache;
typedef DWORD  noyau_delai;
typedef HANDLE noyau_pool_id;
typedef HANDLE noyau_event_id;
typedef HANDLE noyau_semaphore_id;
typedef DWORD noyau_taille_bloc; 
typedef HANDLE noyau_objet;
typedef DWORD noyau_nb_objets;

#define NOYAU_ID_NOK -1L

typedef struct
{
   noyau_bal_id bl_id;
   noyau_bal_id bl_retour;
}
struct_neutre;

typedef struct
{
   struct_neutre   neutre;
   noyau_chrono_id numero;
   DWORD           compteur;
   PVOID           args;
}
struct_chrono;

typedef struct
{
    DWORD    dwMessageCount;
    DWORD    dwSizeCount;
    DWORD    dwNbMessagesMax;
    noyau_bal_id bal_id;
}
noyau_bal_stats;

/*-------------------------------------------------------------
 * Structure permettant d'initialiser le noyau.
 --------------------------------------------------------------*/

typedef enum
{
   NOYAU_FAUX = 0,
   NOYAU_VRAI
}
noyau_enum_booleen;

/*-------------------------------------------------------------
 * Structure propre … une tƒche.
 --------------------------------------------------------------*/

typedef DWORD WINAPI ThreadFunction (LPVOID param);

typedef struct
{
   noyau_enum_booleen   noyau_tache_active;     /* faut-il ou non d‚marrer la tƒche */
   noyau_priorite_tache noyau_priorite_tache;   /* priorite de la tache */
   noyau_taille_bloc    noyau_taille_pile;      /* taille de la pile de cette tache.       */
   ThreadFunction       *noyau_adresse_tache;   /* Point d'entree de la tache.             */
   void                 *parametre;             /* Paramètre de la tache */
   noyau_tache_id       noyau_id_tache;         /* renseigné par LanceTache() */
   char                 nom_tache[MAX_PATH +1]; /* nom en claire de la tache pour trace */
}
struct_tache;


/*--------------- FUNCTIONS: ---------------*/

/*** Lancement des diff‚rentes taches de l'application ***/
EXPORT noyau_enum_retour WINAPI LanceTache (struct_tache *p_tache);

/*** Arret de la tache courante ***/
EXPORT noyau_enum_retour WINAPI ArretTaches (struct_tache *p_tache);

/*** Etat d'une tache ***/
//EXPORT noyau_enum_retour WINAPI EtatTache (noyau_tache_id Numero_Tache);

/*** Numero tache courante ***/
EXPORT noyau_tache_id WINAPI TacheCourante (void);

/*** Liberer le temps CPU ***/
EXPORT noyau_enum_retour WINAPI DelaiTache (noyau_delai ticks);

/*** Changement de priorite d'une tache ***/
EXPORT noyau_enum_retour WINAPI ChangePriorite (noyau_tache_id num_tache,
                                                noyau_priorite_tache priorite );

/*** Arret d'une tache ***/
EXPORT noyau_enum_retour WINAPI TermineTache (noyau_tache_id numero_tache);

/*** Arret de la tache courante ***/
EXPORT void WINAPI Termine (void);

/*** Allocation m‚moire pour les boŒtes aux lettres ***/
EXPORT noyau_enum_retour WINAPI Alloue (struct_neutre ** pp_neutre,
                                        noyau_taille_bloc longueur,
                                        noyau_pool_id numero_pool);

#define ExitAlloue(a, b, c) DEFINE_ExitAlloue(__FILE__, __LINE__, a, b, c)
EXPORT void WINAPI DEFINE_ExitAlloue (const char *file,
                                      int line,
                                      struct_neutre **pp_neutre,
                                      noyau_taille_bloc longueur,
                                      noyau_pool_id numero_pool);

/*** LibŠre de la m‚moire des boŒtes aux lettres ***/
EXPORT noyau_enum_retour WINAPI Libere (struct_neutre ** pp_neutre);


#define ExitLibere(a) DEFINE_ExitLibere(__FILE__, __LINE__, a)
EXPORT void WINAPI DEFINE_ExitLibere (const char *file,
                                      int line,
                                      struct_neutre **pp_neutre);

/* alloue de la memoire dans un pool */
EXPORT noyau_enum_retour WINAPI AlloueBloc (void **adresse,
                                            noyau_taille_bloc longueur,
                                            noyau_pool_id numero_pool);

/* libere de la m‚moire allou‚e dans un pool */
EXPORT noyau_enum_retour WINAPI LibereBloc (void **adresse);

/* renvoie la taille d'un bloc */
EXPORT noyau_taille_bloc WINAPI DonneTailleBloc (struct_neutre *p_neutre);

/*** Création d'une Boite Aux Lettres ***/
EXPORT noyau_bal_id WINAPI PublieBAL (char *nom_bal, unsigned long nb_messages_en_attente_max); // 0 message = illimité

/*** connexion vers une boite aux lettres ***/
EXPORT noyau_bal_id WINAPI AttendBAL (char *nom_bal);
EXPORT noyau_bal_id WINAPI AttendBALTantQue (char *nom_bal, noyau_delai delai);

EXPORT noyau_bal_id WINAPI DonneIdBAL (char *nom_bal);

EXPORT noyau_enum_retour WINAPI DonneNomBAL (noyau_bal_id bl_id, char *nom_bal);

/*** suppresion d'une boite aux lettres ***/
EXPORT noyau_enum_retour WINAPI SupprimeBAL (char *nom_bal);

EXPORT noyau_enum_retour WINAPI StatistiquesBAL (noyau_bal_id id_bal,
                                                 size_t size,
                                                 noyau_bal_stats *stats_bal);

/*** Envoi un message … une autre tƒche ***/
EXPORT noyau_enum_retour WINAPI Envoie (noyau_bal_id num_bal_destinataire,
                                        noyau_bal_id bal_retour,
                                        struct_neutre * p_neutre);

#define ExitEnvoie(a, b, c) DEFINE_ExitEnvoie(__FILE__, __LINE__, a, b, c)
EXPORT void WINAPI DEFINE_ExitEnvoie (const char *file,
                                      int line,
                                      noyau_bal_id bal,
                                      noyau_bal_id bal_retour,
                                      struct_neutre *p_neutre);

/*** Test si message pr‚sent dans une boŒtes aux lettres ***/
EXPORT noyau_enum_retour WINAPI TestRecoit (noyau_bal_id num_bal,
                                            struct_neutre ** pp_neutre);

/*** Attend un message dans une boŒte aux lettres ***/
EXPORT noyau_enum_retour WINAPI Recoit (noyau_bal_id num_bal,
                                        struct_neutre ** pp_neutre,
                                        noyau_delai timeout);

EXPORT noyau_enum_retour WINAPI RecoitMs (noyau_bal_id num_bal,
                                        struct_neutre ** pp_neutre,
                                        int iTimeoutMs);

// ATTENTION contrairement à la fonction Envoie classique :
// - pas de region impliquée 
// - p_neutre n'est plus un bloc dynamique libéré par la fonction
// - le nombre de messages envoyé dans une BAL n'est plus limité
// - il n'y a plus de traces, ni de debug
EXPORT noyau_enum_retour FastEnvoie (HANDLE hBalEcriture,
                                     noyau_bal_id id_bal,
                                     noyau_bal_id bal_retour,
                                     struct_neutre *p_neutre,
                                     noyau_taille_bloc taille);

// Renvoie un handle en écriture sur la BAL à utiliser avec 
// la fonction FastEnvoie()
EXPORT HANDLE DonneHandleEcritureBAL (noyau_bal_id id_bal);

/*** Debut region critique ***/
EXPORT noyau_enum_retour WINAPI DebutRegion (void);

/*** Fin region critique ***/
EXPORT noyau_enum_retour WINAPI FinRegion (void);

/*** Teste region critique ***/
EXPORT noyau_enum_booleen WINAPI EstEnRegion (void);

/*** Debut region critique ***/
EXPORT noyau_enum_retour WINAPI DebutRegionLocale (void);

/*** Fin region critique ***/
EXPORT noyau_enum_retour WINAPI FinRegionLocale (void);

/*** Teste region critique ***/
EXPORT noyau_enum_booleen WINAPI EstEnRegionLocale (void);

/*** Crée un evenement ***/
EXPORT noyau_enum_retour WINAPI AlloueEvent (noyau_event_id *event_id, char *nom);
EXPORT noyau_enum_retour WINAPI LibereEvent (noyau_event_id *event_id);

/*** Signal un evenement ***/
EXPORT noyau_enum_retour WINAPI SignalEvt (noyau_event_id Evenement);

/*** Attente d'un evenement ***/
EXPORT noyau_enum_retour WINAPI AttenteEvt (noyau_event_id Evenement,
                                            noyau_delai Timeout);

/*** Teste si un evenement est arrivee */
EXPORT noyau_enum_retour WINAPI TestEvt (noyau_event_id Evenement);

/*** Effacement d'un evenement ***/
EXPORT noyau_enum_retour WINAPI EffaceEvt (noyau_event_id Evenement);


/*** Reservation d'un numéro de chronometre ***/
EXPORT noyau_enum_retour WINAPI AlloueChrono (noyau_chrono_id *numero_chrono, char *nom);
EXPORT noyau_enum_retour WINAPI AlloueChronoEx (noyau_chrono_id *numero_chrono, char *nom, PVOID args);
EXPORT noyau_enum_retour WINAPI LibereChrono (noyau_chrono_id *numero_chrono);

/*** Lancement d'un chronometre ***/
EXPORT noyau_enum_retour WINAPI LanceChrono (noyau_chrono_id numero_chrono,
                                             noyau_delai duree_chrono,
                                             noyau_bal_id numero_bal);

EXPORT noyau_enum_retour WINAPI LanceChronoMs (noyau_chrono_id numero_chrono,
                                               int iTimeDelayMs,
                                               noyau_bal_id numero_bal);

/*** Lancement d'un chronometre cyclique ***/
EXPORT noyau_enum_retour WINAPI LanceChronoCyclique (noyau_chrono_id numero_chrono,
                                                     noyau_delai duree_chrono,
                                                     noyau_delai periode,
                                                     noyau_bal_id numero_bal);

EXPORT noyau_enum_retour WINAPI LanceChronoCycliqueMs (noyau_chrono_id numero_chrono,
                                                     int iTimeDelayMs,
                                                     int iPeriodeMs,
                                                     noyau_bal_id numero_bal);

/*** renvoie le compteur de lancement courant du chrono qu'on retrouve dans struct_chrono ***/
EXPORT DWORD WINAPI DonneCompteurChrono (noyau_chrono_id numero_chrono);

/*** Arret d'un chronometre ***/
EXPORT noyau_enum_retour WINAPI ArretChrono (noyau_chrono_id numero_chrono);

/*** Test l'etat d'un chronometre ***/
EXPORT noyau_enum_booleen WINAPI TestChrono (noyau_chrono_id numero_chrono);

/*** Création d'un sémaphore ***/
EXPORT noyau_enum_retour WINAPI AlloueSemaphore (noyau_semaphore_id *sem_id, long valeur_init, long valeur_max, char *nom);

/*** Libération d'un sémaphore ***/
EXPORT noyau_enum_retour WINAPI LibereSemaphore (noyau_semaphore_id *sem_id);

/*** Prise bloquante d'un semaphore ***/
EXPORT noyau_enum_retour WINAPI AttendPriseSemaphore (noyau_semaphore_id semaphore);

/*** Prise d'un semaphore avec delai d'attente ***/
EXPORT noyau_enum_retour WINAPI AttendPriseSemaphoreTantQue (noyau_semaphore_id semaphore,
                                                             noyau_delai delai);

/*** Prise d'un semaphore si disponible ***/
EXPORT noyau_enum_retour WINAPI TesteEtPrendSemaphore (noyau_semaphore_id semaphore);

/*** Rendu d'un semaphore ***/
EXPORT noyau_enum_retour WINAPI RendSemaphore (noyau_semaphore_id semaphore, LONG combien);

EXPORT noyau_objet_retour WINAPI AttendObjets (noyau_enum_booleen attendre_tous, noyau_nb_objets nb_objets, noyau_objet objet_0, ...);

EXPORT noyau_objet_retour WINAPI AttendObjetsTantQue (noyau_delai delai, noyau_enum_booleen attendre_tous, noyau_nb_objets nb_objets, noyau_objet objet_0, ...);

/*** Test profondeur des piles ***/
//EXPORT noyau_taille_bloc WINAPI TestePileTacheCourante (void);

/*** teste si ‚crasement des blocs alloues ***/
//EXPORT noyau_enum_retour WINAPI TesteBlocs(void);

/*** fonction de debug du scheduling des taches ***/
//EXPORT void WINAPI TesteTacheCourante (void);

/*** En cas d'erreur fatale ( fonctions Exit...() ) ***/
#define ExitBad() DEFINE_ExitBad(__FILE__, __LINE__)
EXPORT void WINAPI DEFINE_ExitBad(const char *file, int line);

EXPORT noyau_enum_booleen WINAPI InterditDebugNoyau (void);
EXPORT noyau_enum_booleen WINAPI AutoriseDebugNoyau (void);
EXPORT enum index_traces WINAPI TesteTailleFichiersTracesNoyau (void);

EXPORT void WINAPI NoyGetInstallPath(char *pszPath);
EXPORT void WINAPI NoyGetTracesPath(char *pszPath);
EXPORT void WINAPI NoyGetErrorsPath(char *pszPath);


/*|*/
/*****************************************************************************/
/*SYNTAX: 	char * NoyCompleteWithInstallPath(char *pszPath)				 */
/*===========================================================================*/
/*TYPE:		Exported function.												 */
/*===========================================================================*/
/*DESCRIPTION:	Insert the installation path at the begining of a string if	 */
/*	it begins with character '\'											 */
/*	Warning : The size of pszPath must be at least MAX_PATH					 */
/*===========================================================================*/
/*PARAMETERS:	pszPath - a relative path									 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  Completed pszPath if success											 */
/*	NULL			if error												 */
/*****************************************************************************/
EXPORT char * NoyCompleteWithInstallPath(char *pszPath);

#ifdef __cplusplus
}
#endif

#undef PUBLIC
#undef EXPORT
#undef I
#undef INIT
#endif
