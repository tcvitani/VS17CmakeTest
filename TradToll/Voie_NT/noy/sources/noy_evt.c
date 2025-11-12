/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_EVT.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
#define FMT_EVT " { evt 0x%X / tache %lu }"

/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour AlloueEvent (noyau_event_id *event_id)
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : Creation OK
*           - NOYAU_NOK : Creation NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Création d'un evenement
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour AlloueEvent (noyau_event_id *event_id, char *nom)
{
    
    *event_id = CreateEvent (NULL, TRUE, FALSE, nom);

    if (*event_id == NULL)
    {
#ifdef ERREURS
     RtcFichierDebug(NOY_EVT, &_noyau_.dbg, "AlloueEvt() : %s en echec " FMT_EVT FMT_STATUS, 
        nom, *event_id, GetCurrentThreadId(), GetLastError());
#endif
        return NOYAU_NOK;
    }

#ifdef TRACES
    RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "AlloueEvt() : %s " FMT_EVT, nom, *event_id, GetCurrentThreadId());
#endif

    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour LibereEvent (noyau_event_id *event_id)
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : Creation OK
*           - NOYAU_NOK : Creation NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Création d'un evenement
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour LibereEvent (noyau_event_id *event_id)
{
    
    if (CloseHandle(*event_id) == FALSE)
    {
#ifdef ERREURS
     RtcFichierDebug(NOY_EVT, &_noyau_.dbg, "LibereEvt() :"" echec " FMT_EVT FMT_STATUS, 
        *event_id, GetCurrentThreadId(), GetLastError());
#endif
        return NOYAU_NOK;
    }
    
    *event_id = NULL; 

#ifdef TRACES
    RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "LibereEvt() :" FMT_EVT, *event_id, GetCurrentThreadId());
#endif

    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour SignalEvt (short int Numero_Tache, short int Evenement )
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : operation Ok
*           - NOYAU_SIGNAL_TACHE : Mauvais numero de tache
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Signalement d'un evenement a une tache
* --------------------------------------------------------------------
* $F_FCTN
*/
  
PUBLIC noyau_enum_retour SignalEvt(noyau_event_id Evenement)
{
   noyau_enum_retour retour;

   if (SetEvent (Evenement) )
  {
#ifdef TRACES
    RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "SignalEvt() :" FMT_EVT, Evenement, GetCurrentThreadId());
#endif
    retour = NOYAU_OK;
  }
  else
  {
#ifdef ERREURS
     RtcFichierDebug(NOY_EVT, &_noyau_.dbg, "SignalEvt() :"" echec " FMT_EVT FMT_STATUS, 
        Evenement, GetCurrentThreadId(), GetLastError());
#endif
     /*** Si mauvais numero de tache ***/
     retour = NOYAU_NOK;
  }

   TestsPeriodiques ();

  /*** Valeur de retour de proc‚dure ***/
  return (retour);

}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour AttenteEvt (unsigned short int Evenement,
*                                               unsigned long int Timeout,
*                                               unsigned short int *Liste_evt)
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : operation Ok
*           - NOYAU_EVT_TIMEOUT: Sortie sur TimeOut
*           - NOYAU_EVT_EXCEPTION:Tache arrete par un StopTask
*           - NOYAU_EVT_REGION: Appel dans une region
*           - NOYAU_EVT_TASK: Appel par une tache de fond
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Attente d'un evenement dans une BAL
*       Cette proc‚dure ne doit pas ˆtre appelee dans une region critique
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour AttenteEvt (noyau_event_id Evenement,
                                     noyau_delai Timeout)
{
   DWORD dwStatus;
   noyau_enum_retour Status;
   
   RESIZE_TIMEOUT(Timeout);
   
   /*** Attente d'un evenement ***/
   dwStatus = WaitForSingleObject ( Evenement, Timeout);

   /*** Suivant le retour de la fonction ***/
   switch (dwStatus)
   {
      /*** Attente Ok ***/
      case WAIT_OBJECT_0 :
#ifdef TRACES
         RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "AttenteEvt() :" FMT_EVT, Evenement, GetCurrentThreadId());
#endif
         Status = NOYAU_OK;
         break;

      /*** Sortie sur timeout ***/
      case WAIT_TIMEOUT :
         Status = NOYAU_EVT_TIMEOUT;
         break;

      /*** Tache arretee par un StopTask ***/
      case WAIT_ABANDONED :
      default:
         Status = NOYAU_EVT_EXCEPTION;
         break;
   }

   TestsPeriodiques ();

   /*** Retour de la proc‚dure ***/
   return (Status);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour TestEvt (unsigned short int Evenement)
* PARAMETRES:
* ENTREE: Liste d'evenement(s) (1 bit par evenement).
* RETOUR:
*           - NOYAU_OK : operation Ok
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Teste si la liste d'evenement(s) est arrivee pour la tache appelante.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour TestEvt(noyau_event_id Evenement)
{
   DWORD dwStatus;
   noyau_enum_retour Status;

   /*** Teste si toute la liste "Evenement" est arrivee ***/
   dwStatus = WaitForSingleObject (Evenement, 0);
   
   /*** Suivant le retour de la fonction ***/
   switch (dwStatus)
   {
      /*** Attente Ok ***/
      case WAIT_OBJECT_0 :
#ifdef TRACES
         RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "TestEvt() :" FMT_EVT, Evenement, GetCurrentThreadId());
#endif
         Status = NOYAU_OK;
         break;

      /*** Sortie sur timeout ***/
      case WAIT_TIMEOUT :
         Status = NOYAU_EVT_TIMEOUT;
         break;

      /*** Tache arretee par un StopTask ***/
      case WAIT_ABANDONED :
      default:
         Status = NOYAU_EVT_EXCEPTION;
         break;
   }

   TestsPeriodiques ();

   return (NOYAU_NOK);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void EffaceEvt (unsigned short int Evenement )
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Effacement d'un evenement
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour EffaceEvt(noyau_event_id Evenement )
{
   /*** Appel … la proc‚dure d'effacement d'evenements ***/
   if (!ResetEvent (Evenement))
      return NOYAU_EVT_EXCEPTION;

#ifdef TRACES
   RtcFichierTrace(NOY_EVT, &_noyau_.dbg, "EffaceEvt() :" FMT_EVT,Evenement, GetCurrentThreadId());
#endif
   
   TestsPeriodiques ();

   return NOYAU_OK;
}

