/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_TCH.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"
#include "csr_excpt.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
#define FMT_TACHE  " { tache %lu }"
#define FMT_NOM_TACHE_PRIO " { %s / tache %lu / prio %u }"
#define FMT_NOM_PRIO " { %s / prio %u }"
#define FMT_TACHE_DUREE " { tache %lu / duree %lu }"
#define FMT_TACHE_PRIO  " { tache %lu / prio %u}"
#define FMT_NOM_TACHE_ADR_PRIO " { %s / tache %lu / %p / prio %u }"

/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour LanceTache (struct_tache *p_tache,
*                                               unsigned int Numero_1er_Tache)
* PARAMETRES:
* ENTREE:  - poineur sur la structure
*          - Numero de la premiere tache a lancer
*          - Nombre de taches
* RETOUR:
*          -
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Lancement des differentes taches de l'application.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour LanceTache (struct_tache *p_tache)
{
   WORD TaskRange;
   DWORD ThreadId;
   
   /*** Entree dans une region ***/
   EnterRegion();

   /*** Boucle d'initialisation des taches ***/
   for (TaskRange = 0; p_tache [ TaskRange ].noyau_adresse_tache != NULL; TaskRange++ )
   {
      if (TaskRange >= MAX_THREADS)
      {
#ifdef ERREURS
            RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "LanceTache() : echec nb threads" FMT_NOM_TACHE_PRIO,
                p_tache[TaskRange].nom_tache, GetCurrentThreadId(), p_tache[TaskRange].noyau_priorite_tache);
#endif
            ExitBad();
      }

      /*** Si la tache doit etre lancee ***/
      if (p_tache [ TaskRange ].noyau_tache_active == NOYAU_VRAI )
      {
         p_tache [ TaskRange ].noyau_id_tache = ExcptCreateThread (NULL, // SECURITY_ATTRIBUTES
                                            0,    // initial stack size
                                            p_tache[TaskRange].noyau_adresse_tache,
                                            p_tache [ TaskRange ].parametre, // parameter
                                            CREATE_SUSPENDED,
                                            &ThreadId,
                                            p_tache [ TaskRange ].nom_tache); 
         
         if (p_tache [ TaskRange ].noyau_id_tache == NULL)
         {
#ifdef ERREURS
            RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "LanceTache() : echec CreateThread" FMT_NOM_TACHE_PRIO, FMT_STATUS,
                p_tache[TaskRange].nom_tache, ThreadId, p_tache[TaskRange].noyau_priorite_tache, GetLastError());
#endif
            ExitBad();
         }

         /* on fixe sa priorite */
         if (!SetThreadPriority (p_tache [ TaskRange ].noyau_id_tache, p_tache[TaskRange].noyau_priorite_tache))
         {
#ifdef ERREURS
             RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "LanceTache() : echec SetThreadPriority" FMT_NOM_TACHE_PRIO, FMT_STATUS,
                p_tache[TaskRange].nom_tache, ThreadId, p_tache[TaskRange].noyau_priorite_tache, GetLastError());
#endif
            ExitBad();
         }

         /*** Si l'initialisation d'une tache Ok ***/
#ifdef TRACES
         RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "LanceTache() :" FMT_NOM_TACHE_ADR_PRIO,
               p_tache[TaskRange].nom_tache, 
               ThreadId,
               p_tache[TaskRange].noyau_adresse_tache,
               p_tache[TaskRange].noyau_priorite_tache);
#endif
         
      }
   }

   for (TaskRange = 0; p_tache [ TaskRange ].noyau_adresse_tache != NULL; TaskRange++ )
   {
      // Si la tache doit etre lancee 
      if (p_tache [ TaskRange ].noyau_tache_active == NOYAU_VRAI )
         if (ResumeThread (p_tache [ TaskRange ].noyau_id_tache) == -1)
         {
#ifdef ERREURS
             RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "LanceTache() : echec SetThreadPriority" FMT_NOM_PRIO, FMT_STATUS,
                p_tache[TaskRange].nom_tache, p_tache[TaskRange].noyau_priorite_tache, GetLastError());
#endif
             ExitBad();
         }
   }    

   /*** Sortir de la region ***/
   LeaveRegion();

   /*** Retourner la valeur du lancement des taches ***/
   return NOYAU_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour ArretTaches (struct_tache *p_tache,
*                                                short int Numero_1er_Tache )
* PARAMETRES:
* ENTREE:
*          _ Numero de la premiere tache a arreter
* RETOUR:
*          - NOYAU_ARRET_TACHE_OK
*          - NOYAU_ARRET_TACHE_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Arreter la tache encore operationnelles et rendre la
* memoire (associee a cette tache) au systeme.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour ArretTaches (struct_tache *p_tache)
{
   WORD TaskRange;

   /*** Boucle d'initialisation des taches ***/
   for (TaskRange = 0; p_tache [ TaskRange ].noyau_adresse_tache != NULL; TaskRange++ )
   {
      /*** Si la tache a ete lancee ***/
      if (p_tache [ TaskRange ].noyau_tache_active == NOYAU_VRAI )
      {
         TerminateThread (p_tache [ TaskRange ].noyau_id_tache, 0);

#ifdef TRACES
         RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "ArretTache() :" FMT_NOM_PRIO,
                p_tache[TaskRange].nom_tache, 
                p_tache[TaskRange].noyau_priorite_tache);
#endif
      }
   }

  /*** retourner valeur de l'arret ***/
  return NOYAU_ARRET_TACHE_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour EtatTache (unsigned short int Numero_Tache)
* PARAMETRES:
* ENTREE:
*          _ Numero de la tache dont on veut connaitre l'etat.
* RETOUR:
*            NOYAU_TACHE_COURANTE
*            NOYAU_TACHE_PRETE
*            NOYAU_TACHE_EN_ATTENTE
*            NOYAU_TACHE_NON_PRETE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Tester l'etat de la tache
* --------------------------------------------------------------------
* $F_FCTN
*/
/*
PUBLIC noyau_enum_retour EtatTache (noyau_tache_id Numero_Tache)
{
   noyau_enum_retour etat;
   DWORD dwStatus;

   if (Numero_Tache == GetCurrentThreadId())
      return NOYAU_TACHE_COURANTE;

   etat = NOYAU_NOK;
   if (GetExitCodeThread(Numero_Tache, &dwStatus))
   {
      if (dwStatus == STILL_ACTIVE)
         etat = NOYAU_TACHE_PRETE;
      else
         etat = NOYAU_TACHE_NON_PRETE;
   }
  
#ifdef TRACES
   RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "EtatTache() :" FMT_TACHE FMT_STATUS, Numero_Tache, etat);
#endif

   TestsPeriodiques ();

  // Renvoyer etat de la tache
  return (etat);
}
*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC short int TacheCourante (void)
* PARAMETRES:
* ENTREE:
* RETOUR:
*            Numero de la tache courante
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Renvoie le numero de la tache courante (peut etre negatif)
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_tache_id TacheCourante (void)
{
  /*** Renvoyer handle du thread courant ***/
  return GetCurrentThread();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PUBLIC void DelaiTache (unsigned short int ticks);
* PARAMETRES: nombre de ticks horloge qui donne le temps de blocage de
*             la tache appelante.
* RETOUR:
*             - NOYAU_OK
*             - NOYAU_DELAY_TIMEOUT   : timeOut atteind avant evenement
*             - NOYAU_DELAY_STOP      : Delay arrete par une StopTask
*             - NOYAU_DELAY_IN_REGION : appel dans une region critique
*             - NOYAU_DELAY_TASK      : appel dans une tache de fond
*             - NOYAU_DELAY_NUL       : Nbre de ticks a 0
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction publique
* ROLE: Suspend la tache appelante pendant le temps passe en parametre.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour DelaiTache (noyau_delai ticks)
{
    DWORD thread_id = GetCurrentThreadId();

    if (_NOYAU_.thread_en_region == thread_id ||
        _noyau_.thread_en_region_locale == thread_id)
    {
#ifdef ERREURS
         RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "DelaiTache() : en région " FMT_TACHE_DUREE,
            GetCurrentThreadId(), ticks);
#endif
    }
    
    RESIZE_TIMEOUT(ticks);  // ancien tick de 55 ms
    
    Sleep (ticks);

    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour ChangePriorite (short int num_tache,
*                                                   short int priorite )
* PARAMETRES:
* ENTREE:         numero de la tache : -1 si tache courante
*                 priorite de la tache
* RETOUR:
*          NOYAU_OK
*          NOYAU_MAUVAISE_TACHE : mauvais numero de tache
*          NOYAU_MAUVAISE_PRIORITE:mauvaise priorite
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction.
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour ChangePriorite (noyau_tache_id num_tache,
                                         noyau_priorite_tache priorite )
{
   noyau_enum_retour retour;

   retour = NOYAU_OK;
 
   if (!SetThreadPriority ( num_tache, priorite))
      retour = NOYAU_MAUVAISE_TACHE;

#ifdef TRACES
   RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "ChangePriorite() :" FMT_TACHE_PRIO FMT_STATUS, GetCurrentThreadId(), priorite, GetLastError());
#endif

   TestsPeriodiques ();

   return (retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void Termine ( void )
* PARAMETRES:
* ENTREE:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: arrˆte l'ex‚cution de la tache courante.
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void Termine ( void )
{

#ifdef TRACES
   RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "Termine()", FMT_TACHE, GetCurrentThreadId());
#endif

   TestsPeriodiques ();

   ExitThread(0);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour TermineTache (noyau_tache_id numero_tache)
* PARAMETRES:
* ENTREE:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Elimine une tache de la liste des taches en attentes.
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour TermineTache (noyau_tache_id numero_tache)
{
   /*** Arret de la tache numero_tache ***/
   if (SuspendThread(numero_tache) == -1)
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "TermineTache() :" FMT_TACHE FMT_STATUS, numero_tache, GetLastError());
#endif
      return NOYAU_ARRET_TACHE_NOK;
   }

#ifdef TRACES
   RtcFichierTrace(NOY_TCH, &_noyau_.dbg, "TermineTache() :" FMT_TACHE, numero_tache);
#endif

   TestsPeriodiques ();

   return NOYAU_OK;
}


PROTECTED void TraceTaches (WORD nb_taches)
{
/*
   noyau_tache_id i;
   char *etat;

   EnterRegion();

   // inhibe les erreurs 
   DBG_DebutDebugSansErreurs (&_noyau_.dbg);

   RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "TraceTaches() : nb taches = %02d", nb_taches);

   // teste l'etat de toutes les taches
   for (i=0; i<nb_taches; i++)
	{
      switch (TaskState (i))
      {
         case cCurrent :
              etat = "COURANTE";
              break;

         case cReady :
              etat = "PRETE";
              break;

         case cWaiting :
              etat = "EN ATTENTE";
              break;

         case cNonOperational :
              etat = "NON PRETE";
              break;

         case cBadTaskNumber :
              etat = "NUMERO NOK";
              break;

         default :
              etat = "?";
              break;
       }
       RtcFichierDebug(NOY_TCH, &_noyau_.dbg, "   Tache %02d => %s", i, etat);
	}

   DBG_FinDebugSansErreurs (&_noyau_.dbg);

   LeaveRegion();
*/
}

PROTECTED void TesteTacheCourante (void)
{
/*
  noyau_tache_id tache;
   int x,y;

   tache = CurrentTask();
   if (_NOYAU_->tache_courante != tache)
   {
      _NOYAU_->tache_courante = tache;
#ifdef TRACES
      // si fichier traces present 
      if (DBG_FichierTracesPresent(NOY_TCOU, &_noyau_.dbg) == NOYAU_FAUX)
         return;
      x = wherex();
      y = wherey();
      gotoxy (2,24);
      putch(tache + 'a');
      gotoxy(x,y);
#endif
   }
*/
}
