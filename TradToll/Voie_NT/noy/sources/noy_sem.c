/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_SEM.C
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
#define FMT_SEM "{ sem %d }"

/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour AlloueSemaphore (noyau_semaphore_id *sem_id, 
*                            long valeur_init, long valeur_max, char *nom)
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : Creation OK
*           - NOYAU_NOK : Creation NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Création d'un semaphore
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour AlloueSemaphore (noyau_semaphore_id *sem_id, long valeur_init, long valeur_max, char *nom)
{
    
	*sem_id = (noyau_semaphore_id *)CreateSemaphoreEx(NULL, valeur_init, valeur_max, nom, 0, SEMAPHORE_ALL_ACCESS);

    if (*sem_id == NULL)
    {
#ifdef ERREURS
     RtcFichierDebug(NOY_SEM, &_noyau_.dbg, "AlloueSem() : %s en echec " FMT_SEM FMT_STATUS, 
        nom, *sem_id, GetCurrentThreadId(), GetLastError());
#endif
        return NOYAU_NOK;
    }

#ifdef TRACES
    RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "AlloueSem() : %s " FMT_SEM, 
        nom, *sem_id, GetCurrentThreadId());
#endif

    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour LibereSemaphore (noyau_semaphore_id *sem_id)
* PARAMETRES:
* ENTREE:
* RETOUR:
*           - NOYAU_OK : Creation OK
*           - NOYAU_NOK : Creation NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Liberation d'un semaphore
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour LibereSemaphore (noyau_semaphore_id *sem_id)
{
    
    if (CloseHandle(*sem_id) == FALSE)
    {
#ifdef ERREURS
     RtcFichierDebug(NOY_SEM, &_noyau_.dbg, "LibereSem() :"" echec " FMT_SEM FMT_STATUS, 
        *sem_id, GetCurrentThreadId(), GetLastError());
#endif
        return NOYAU_NOK;
    }
    
    *sem_id = NULL; 

#ifdef TRACES
    RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "LibereSem() :" FMT_SEM, 
        *sem_id, GetCurrentThreadId());
#endif

    return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour AttendPriseSemaphore (unsigned short numero_semaphore)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - numero du sempahore … prendre
*
*      Retour: - NOYAU_OK ou NOYAU_SEM_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction de prise de jeton d'un semaphore bloquante.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour AttendPriseSemaphore (noyau_semaphore_id semaphore)
{
   DWORD dwStatus;
   noyau_enum_retour retour;

   /* Attends indefiniment le semaphore */
   dwStatus = WaitForSingleObject (semaphore, INFINITE);

   /*** Si la prise de sempahore s'est bien pass‚e ***/
   switch (dwStatus)
   {
      case WAIT_OBJECT_0:
#ifdef TRACES
           RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "AttendPriseSemaphore() :" FMT_SEM, semaphore);
#endif
           retour = NOYAU_OK;
           break;

      default:
           retour = NOYAU_SEM_NOK;
           break;
   }

   TestsPeriodiques ();

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour AttendPriseSemaphoreTantQue (unsigned short numero_semaphore,
*                                                                unsigned short delai)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - numero du sempahore … prendre
*              - delai d'attente en ticks de la prise du semaphore (>0)
*
*      Retour: - NOYAU_OK ou NOYAU_SEM_DELAI ou NOYAU_SEM_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction de prise de jeton d'un semaphore bloquante sur time out.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour AttendPriseSemaphoreTantQue (noyau_semaphore_id semaphore,
                                                      noyau_delai ticks)
{
   DWORD dwStatus = 0;
   DWORD dwError = 0;
   noyau_enum_retour retour = NOYAU_NOK;

   RESIZE_TIMEOUT(ticks);

   retour = NOYAU_SEM_NOK;

   /* Attends le semaphore jusqu'a echeance du delai */
   dwStatus = WaitForSingleObject (semaphore, ticks);
   
   /*** Si la prise de sempahore s'est bien pass‚e ***/
   switch (dwStatus)
   {
   case WAIT_OBJECT_0:
#ifdef TRACES
       RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "AttendPriseSemaphoreTantQue() :" FMT_SEM, semaphore);
#endif
       retour = NOYAU_OK;
       break;
       
   case WAIT_TIMEOUT:
       retour = NOYAU_SEM_DELAI;
       break;
       
   default:
       retour = NOYAU_SEM_NOK;
	   dwError = GetLastError();
       break;
   }

   TestsPeriodiques ();

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour TestEtPrendSemaphore (unsigned short numero_semaphore)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - numero du sempahore … prendre
*
*      Retour: - NOYAU_OK ou NOYAU_SEM_ABSENT OU NOYAU_SEM_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction de prise de jeton d'un semaphore si disponible (non bloquante).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour TesteEtPrendSemaphore (noyau_semaphore_id semaphore)
{
   DWORD dwStatus;
   noyau_enum_retour retour;

   /* Prends le semaphore si disponible */
   dwStatus= WaitForSingleObject (semaphore, 0);

   /*** Si la prise de sempahore s'est bien pass‚e ***/
   switch (dwStatus)
   {
      case WAIT_OBJECT_0:
#ifdef TRACES
           RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "TesteEtPrendSemaphore() :" FMT_SEM, semaphore);
#endif
           retour = NOYAU_OK;
           break;

      case WAIT_TIMEOUT:
           retour = NOYAU_SEM_ABSENT;
           break;

      default:
           retour = NOYAU_SEM_NOK;
           break;
   }

   TestsPeriodiques ();

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour RendSemaphore (unsigned short numero_semaphore)
* --------------------------------------------------------------------
* PARAMETRES:
*      Entr‚e: - numero du sempahore … relacher
*
*      Retour: - NOYAU_OK ou NOYAU_SEM_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction de rendu de jeton d'un semaphore.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour RendSemaphore(noyau_semaphore_id semaphore, LONG combien)
{
	LONG previous_count;
	DWORD dwError = 0;
	noyau_enum_retour retour;

	if (!ReleaseSemaphore(semaphore, combien, &previous_count))
	{
		dwError = GetLastError();
		retour = NOYAU_SEM_NOK;
	}
   else
   {
#ifdef TRACES
           RtcFichierTrace(NOY_SEM, &_noyau_.dbg, "RendSemaphore() :" FMT_SEM, semaphore);
#endif
           retour = NOYAU_OK;
   }

   TestsPeriodiques ();

   return retour;
}

