/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_CRO.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"

#include "noy_cro.h"

/*--------------- RESERVED: ---------------*/
#include <memclass.h>
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
#define FMT_CHR_CYC  " { tache %lu / chro %d / duree %ld / periode %ld / bl_ret %ld }"
#define FMT_CHR_ECH  " { tache %lu / chro %d / cycl %ld / bl_ret %ld }"

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

PRIVATE DWORD WINAPI ThreadChrono( LPVOID param);
PRIVATE void APIENTRY APCEcheanceChrono(LPVOID Chrono_id,
                                          DWORD dwTimerLowValue,   
                                          DWORD dwTimerHighValue);
PRIVATE void APIENTRY APCArmeChrono (DWORD Chrono_id);


/*--------------- VARIABLES: ---------------*/


/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC short int Noyau_Init_Chrono ( short int ch_1,
*                                        short int nb_ch )
* PARAMETRES:
* RETOUR:  NOYAU_OK  : l'init s'est correctement deroulee
*          NOYAU_NOK  : l'init des chronos a echoue 
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction .
* ROLE: Fonction d'initialisation de tous les chronometres :
* Cette fonction doit obligatoirement etre lancee avant
* l'utilisation des autres.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED noyau_enum_retour Noyau_Init_Chrono (void)
{
   memset (_noyau_.chrono, 0, sizeof(noyau_struct_chrono)*MAX_CHRONO);

   _noyau_.hThreadChrono = CreateThread (NULL, 0, ThreadChrono, NULL, 0, &_noyau_.ThreadChronoId);

   if (_noyau_.hThreadChrono == NULL)
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "InitChrono() : Echec CreateThread");
#endif
      return(NOYAU_NOK);
   }

   /*** Retourner init chrono Ok ***/
   return(NOYAU_OK);
}

PUBLIC noyau_enum_retour AlloueChrono (noyau_chrono_id *chrono_id, char *nom)
{
    return AlloueChronoEx (chrono_id, nom, NULL);
}

PUBLIC noyau_enum_retour AlloueChronoEx (noyau_chrono_id *chrono_id, char *nom, PVOID args)
{
    noyau_struct_chrono *chrono;

    // recherche prochain numéro de chrono libre
    EnterCriticalSection (&_noyau_.csChrono);
    *chrono_id = 1; // BPH 20/11/00
    while (*chrono_id < MAX_CHRONO && _noyau_.chrono[*chrono_id].hTimer != NULL)
        (*chrono_id)++; 

    // plus de chronos dispos
    if (*chrono_id >= MAX_CHRONO)
    { 
        *chrono_id = -1;
        
        LeaveCriticalSection (&_noyau_.csChrono);
        return NOYAU_NOK;
    }
    
    chrono = &_noyau_.chrono[*chrono_id];
    
    // Associe un timer à ce chrono
    chrono->hTimer = CreateWaitableTimer( NULL, FALSE, NULL);
    if (chrono->hTimer == NULL)
    {
#ifdef ERREURS
        RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "AlloueChrono() : %s Echec CreateWaitableTimer code %d", 
            nom, GetLastError());
#endif
        *chrono_id = -1;
        
        LeaveCriticalSection (&_noyau_.csChrono);
        return NOYAU_NOK;
    }
    
    chrono->duree = 0;
    chrono->periode = 0;
    chrono->bal_retour = 0;
    chrono->args = args;

#ifdef TRACES
   RtcFichierTrace (NOY_CRO, &_noyau_.dbg, "AlloueChrono() : %s" FMT_CHR_ECH,
                    nom,
                    GetCurrentThreadId(),
                    *chrono_id,
                    chrono->periode,
                    chrono->bal_retour);
#endif
    
    LeaveCriticalSection (&_noyau_.csChrono);
    
    return NOYAU_OK;
}

PUBLIC noyau_enum_retour LibereChrono (noyau_chrono_id *chrono_id)
{
   noyau_struct_chrono *chrono;

   EnterCriticalSection (&_noyau_.csChrono);
   
   /*** Si numero de chrono non autorise ***/
   // BPH 20/11/00
   if ( *chrono_id <= 0 || *chrono_id >= MAX_CHRONO || _noyau_.chrono[*chrono_id].hTimer == NULL )
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LibereChrono() : { tache %lu / chro %d} N° chrono invalide",
                    GetCurrentThreadId(),
                    *chrono_id);
#endif

      LeaveCriticalSection (&_noyau_.csChrono);
      return NOYAU_NOK;
   }
   
   chrono = &_noyau_.chrono[*chrono_id];
   
   // Libere le timer associé à ce chrono
   if (CloseHandle (chrono->hTimer) == FALSE)
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LibereChrono() : Echec CloseHandle du WaitableTimer" FMT_CHR_ECH,
                    GetCurrentThreadId(),
                    *chrono_id,
                    chrono->periode,
                    chrono->bal_retour);
#endif
      LeaveCriticalSection (&_noyau_.csChrono);

      return NOYAU_NOK;
   }

#ifdef TRACES
   RtcFichierTrace (NOY_CRO, &_noyau_.dbg, "LibereChrono() :" FMT_CHR_ECH,
                    GetCurrentThreadId(),
                    *chrono_id,
                    chrono->periode,
                    chrono->bal_retour);
#endif
      
   *chrono_id = 0; 
   chrono->hTimer = NULL; 
   chrono->duree = 0;
   chrono->periode = 0;
   chrono->bal_retour = 0;
   chrono->DueTime.QuadPart = 0L;
   chrono->compteur = 0;
   
   LeaveCriticalSection (&_noyau_.csChrono);
   
   return NOYAU_OK;
}

PRIVATE DWORD WINAPI ThreadChrono( LPVOID param)
{
   while (1)
      SleepEx(INFINITE, TRUE);

   return 0;
} 

PRIVATE void APIENTRY APCEcheanceChrono(LPVOID Chrono_id,
                                        DWORD dwTimerLowValue,   
                                        DWORD dwTimerHighValue)
{
   struct_chrono *pMessage = NULL;
   noyau_chrono_id chrono_fini = (noyau_chrono_id) Chrono_id;
   noyau_struct_chrono *chrono;


   chrono = &_noyau_.chrono[chrono_fini];

   EnterCriticalSection (&_noyau_.csChrono);

#ifdef TRACES
   RtcFichierTrace (NOY_CRO, &_noyau_.dbg, "Chrono :" FMT_CHR_ECH,
                    GetCurrentThreadId(),
                    chrono_fini,
                    chrono->periode,
                    chrono->bal_retour);
#endif

   // teste si le chrono n'a pas été désactivé entre temps
   if (chrono->duree == 0)
   {  
      LeaveCriticalSection (&_noyau_.csChrono);
      return;
   }

   ExitAlloue ((struct_neutre**)&pMessage, sizeof(struct_chrono), GetProcessHeap());

   pMessage->numero = chrono_fini;
   pMessage->compteur = chrono->compteur;
   pMessage->args = chrono->args;

//This FastEnvoie makes problems when mailbox is closed and reopened. 
//This is because the client (write pipes) are not verified when using FastEnvoie.
//Envoie does the mailbox reopen if the first write fails.
//    if (FastEnvoie(chrono->hBal,
//                   chrono->bal_retour,
//                   NOYAU_BAL_HORLOGE_ID, 
//                   (struct_neutre*)(&message),
// 					sizeof(struct_chrono)) != NOYAU_OK) 
   if (Envoie(chrono->bal_retour, NOYAU_BAL_HORLOGE_ID, (struct_neutre*)(pMessage)) != NOYAU_OK) 
       ExitBad();

   /* si le chrono n'est pas cyclique, on le libère */
   if (chrono->periode == 0)
   {
      chrono->duree = 0;
      chrono->periode = 0;
      chrono->bal_retour = 0;
   }

   LeaveCriticalSection (&_noyau_.csChrono);
}


PRIVATE void APIENTRY APCArmeChrono (DWORD Chrono_id)
{
   noyau_chrono_id numero_chrono = (noyau_chrono_id) Chrono_id;
   noyau_struct_chrono *chrono;

   chrono = &_noyau_.chrono[numero_chrono];

   EnterCriticalSection (&_noyau_.csChrono);
   
   // teste si le chrono n'a pas été désactivé entre temps
   if (chrono->duree == 0)
   {  
      LeaveCriticalSection (&_noyau_.csChrono);
      return;
   }

   /*** Lancement du chrono ***/
   chrono->DueTime.QuadPart = -(LONGLONG) chrono->duree * 1000 * 10; // duree en ticks (55 ms)
   
   if (chrono->hTimer == NULL || 
      !SetWaitableTimer (chrono->hTimer, 
                         &chrono->DueTime, 
                         chrono->periode, 
                         APCEcheanceChrono, 
                         (LPVOID) numero_chrono, FALSE) ) // periode en ticks (55 ms)
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LanceChrono() :" FMT_CHR_CYC FMT_STATUS,
               GetCurrentThreadId(), numero_chrono, chrono->duree, chrono->periode,
               chrono->bal_retour, GetLastError());
#endif
      ExitBad();
   }

#ifdef TRACES
   RtcFichierTrace(NOY_CRO, &_noyau_.dbg, "LanceChrono() :" FMT_CHR_CYC,
                   GetCurrentThreadId(), numero_chrono, chrono->duree, 
                   chrono->periode, chrono->bal_retour);
#endif

   LeaveCriticalSection (&_noyau_.csChrono);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PUBLIC noyau_enum_retour LanceChronoCyclique (short int numero_chrono,
*                                               unsigned long int duree_chrono,
*                                               short int periode,
*                                               unsigned char numero_bal)
* PARAMETRES:
* << numero_chrono >> : numero du  chrono a demarrer.
* << duree_chrono >> : duree durant laquelle le chrono doit tourner.
* << numero_bal >> : boite au lettre dans laquelle sera poste un message
*                    indiquant que la duree est ecoulee.
*
* RETOUR : NOYAU_OK : Le chrono a demarre
*          NOYAU_NOK : Le chrono n'a pu etre lance
*              (eventu. il l'est deja).
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction
* ROLE: fonction de demarrage d'un chronometre
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour LanceChronoCyclique (noyau_chrono_id numero_chrono,
                                              noyau_delai duree_chrono,
                                              noyau_delai periode,
                                              noyau_bal_id numero_bal)
{
   RESIZE_TIMEOUT(duree_chrono);
   RESIZE_TIMEOUT(periode);
	
   return LanceChronoCycliqueMs (numero_chrono,duree_chrono,periode,numero_bal);

}

PUBLIC noyau_enum_retour LanceChronoCycliqueMs (noyau_chrono_id numero_chrono,
                                                     int iTimeDelayMs,
                                                     int iPeriodeMs,
                                                     noyau_bal_id numero_bal)
{
   noyau_struct_chrono *chrono;
   
   EnterCriticalSection (&_noyau_.csChrono);     

   /*** Si numero de chrono non autorise ***/
   // BPH 20/11/00
   if ( numero_chrono <= 0 || numero_chrono >= MAX_CHRONO || iTimeDelayMs == NOYAU_ATTENTE_INFINIE || iPeriodeMs == NOYAU_ATTENTE_INFINIE)
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LanceChronoCycliqueMs :" FMT_CHR_CYC " -> un parametre est incorrect",
         GetCurrentThreadId(), numero_chrono, iTimeDelayMs, iPeriodeMs, numero_bal);
#endif
      LeaveCriticalSection (&_noyau_.csChrono);
      return NOYAU_NOK;
   }

   chrono = &_noyau_.chrono[numero_chrono];   
   
   /*** si chrono en cours ***/
   if ( chrono->duree > 0 )
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LanceChrono :" FMT_CHR_CYC " deja lancé",
         GetCurrentThreadId(), numero_chrono, iTimeDelayMs, iPeriodeMs, numero_bal);
#endif
      LeaveCriticalSection (&_noyau_.csChrono);

      return NOYAU_NOK;
   }

   chrono->duree = iTimeDelayMs;
   chrono->periode = iPeriodeMs;
   chrono->bal_retour = numero_bal;
   //chrono->hBal = DonneHandleEcritureBAL (numero_bal);
   chrono->compteur++;
   PAPCFUNC pfnAPC = (PAPCFUNC)&APCArmeChrono;
   /*** Lancement du chrono ***/   
   if (!QueueUserAPC(pfnAPC, _noyau_.hThreadChrono, numero_chrono))
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "LanceChronoCycliqueMs :" FMT_CHR_CYC " erreur armement",
         GetCurrentThreadId(), numero_chrono, iTimeDelayMs, iPeriodeMs, numero_bal);
#endif
      LeaveCriticalSection (&_noyau_.csChrono);

      return NOYAU_NOK;
   }

   LeaveCriticalSection (&_noyau_.csChrono);

   TestsPeriodiques ();

   return NOYAU_OK;
}




PUBLIC DWORD DonneCompteurChrono (noyau_chrono_id numero_chrono)
{
    return _noyau_.chrono[numero_chrono].compteur;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PUBLIC noyau_enum_retour LanceChrono (short int numero_chrono,
*                                       unsigned long int duree_chrono,
*                                       unsigned char numero_bal)
* PARAMETRES:
* << numero_chrono >> : numero du  chrono a demarrer.
* << duree_chrono >> : duree durant laquelle le chrono doit tourner.
* << numero_bal >> : boite au lettre dans laquelle sera poste un message
*                    indiquant que la duree est ecoulee.
*
* RETOUR : NOYAU_OK : Le chrono a demarre
*          NOYAU_NOK : Le chrono n'a pu etre lance
*              (eventu. il l'est deja).
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction
* ROLE: fonction de demarrage d'un chronometre
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour LanceChrono (noyau_chrono_id numero_chrono,
                                      noyau_delai duree_chrono,
                                      noyau_bal_id numero_bal)
{
   return LanceChronoCyclique (numero_chrono, duree_chrono, 0, numero_bal);
}

PUBLIC noyau_enum_retour LanceChronoMs (noyau_chrono_id numero_chrono,
                                               int iTimeDelayMs,
                                               noyau_bal_id numero_bal)
{
   return LanceChronoCycliqueMs (numero_chrono, iTimeDelayMs, 0, numero_bal);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour ArretChrono ( short int numero_chrono )
* PARAMETRES:
* << numero_chrono >> : numero du chrono a arreter.
*
* RETOUR:  NOYAU_OK : Le chrono est arrete
*          NOYAU_NOK : Le chrono n'a pu etre arrete (eventu. il l'est deja).
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction externe.
* ROLE: fonction qui permet d'arreter un chronometre
*       en cours de fonctionnement.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_retour ArretChrono (noyau_chrono_id numero_chrono)
{
   noyau_struct_chrono *chrono;

   EnterCriticalSection (&_noyau_.csChrono);   

   /*** Si numero de chrono non autorise ***/
   // BPH 20/11/00   
   if ( numero_chrono <= 0 || numero_chrono >= MAX_CHRONO )
   {
#ifdef ERREURS
      RtcFichierDebug (NOY_CRO, &_noyau_.dbg, "ArretChrono() : { tache %lu / chro %d} N° chrono invalide",
                   GetCurrentThreadId(), 
                   numero_chrono);
#endif

      LeaveCriticalSection (&_noyau_.csChrono);
      return NOYAU_NOK;
   }
   
   chrono = &_noyau_.chrono[numero_chrono];
   
   // si le chrono a deja ete arrete ou n'a jamais ete lancé
   if ( chrono->duree == 0 )
   {

// si pas en commentaire alors simu_zip n'accepte plus les cartes !? 
#ifdef TRACES
   RtcFichierTrace(NOY_CRO, &_noyau_.dbg, "ArretChrono () :" FMT_CHR_ECH " deja arrete",
                   GetCurrentThreadId(), 
                   numero_chrono,
                   chrono->periode,
                   chrono->bal_retour);
#endif
      
      LeaveCriticalSection (&_noyau_.csChrono);

      return NOYAU_NOK;
   }

   /*** Chrono arreter ***/
   chrono->duree = 0;
   chrono->periode = 0;
   chrono->bal_retour = 0;

   /*** Arret du chrono ***/
   if (chrono->hTimer == NULL || 
         !CancelWaitableTimer (chrono->hTimer) ) 
   {
#ifdef ERREURS
      RtcFichierDebug(NOY_CRO, &_noyau_.dbg, "ArretChrono() :" FMT_CHR_CYC FMT_STATUS,
               GetCurrentThreadId(), numero_chrono, chrono->duree, chrono->periode,
               chrono->bal_retour, GetLastError());
#endif
      ExitBad();
   }
   
#ifdef TRACES
   RtcFichierTrace(NOY_CRO, &_noyau_.dbg, "ArretChrono () :" FMT_CHR_ECH,
                   GetCurrentThreadId(), 
                   numero_chrono,
                   chrono->periode,
                   chrono->bal_retour);
#endif
   
   LeaveCriticalSection (&_noyau_.csChrono);

   TestsPeriodiques ();   
   
   return NOYAU_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour TestChrono ( short int numero_chrono )
* PARAMETRES:
* << numero_chrono >> : numero du chrono a tester.
*
* RETOUR:  NOYAU_OK : Le chrono est en route.
*          NOYAU_NOK : Le chrono est arrete.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction externe
* ROLE: fonction qui permet de tester l'etat d'un
*       chronometre (en route ou non).
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC noyau_enum_booleen TestChrono (noyau_chrono_id numero_chrono)
{
   noyau_struct_chrono *chrono;
   noyau_enum_booleen retour;
   
   retour = NOYAU_FAUX;

   EnterCriticalSection (&_noyau_.csChrono);

   /*** Si le numero du chrono est autorise ***/
   // BPH 20/11/00
   if ( numero_chrono <= 0 || numero_chrono >= MAX_CHRONO )
   {
#ifdef ERREURS
      RtcFichierDebug (NOY_CRO, &_noyau_.dbg, "TestChrono() : { tache %lu / chro %d} N° chrono invalide",
                   GetCurrentThreadId(), 
                   numero_chrono);
#endif
      LeaveCriticalSection (&_noyau_.csChrono);

      return retour;
   }

   chrono = &_noyau_.chrono[numero_chrono];

   if (chrono->duree > 0)
   {
      retour = NOYAU_VRAI;

#ifdef TRACES
      RtcFichierTrace (NOY_CRO, &_noyau_.dbg, "TestChrono() :" FMT_CHR_ECH,
                       GetCurrentThreadId(), 
                       numero_chrono,
                       chrono->periode,
                       chrono->bal_retour);
#endif
   }

   LeaveCriticalSection (&_noyau_.csChrono);

   TestsPeriodiques ();

   /*** retour 0: chrono arrete 1: chrono en cours ***/
   return retour;
}

