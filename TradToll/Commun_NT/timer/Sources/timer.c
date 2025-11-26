/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TIMER
 * FILE       : TIMER.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des timer
 * --------------------------------------------------------------------
 * DESCRIPTION: Ce module permet d'encapsuler les objets "waitable timers"
 *              de WIN32. Les fonctions apportées en plus de la création,
 *              l'armement et l'attente (WIN32 pur) sont les possibilités de :
 *                - annuler un timer armé
 *                - tester si un timer a été armé
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <stdio.h>

#ifndef TIMER_EXPORTS
#define TIMER_EXPORTS
#endif

#include "csr_timer.h"

#include <memclass.h>




//
// Encapsulation de l'utilisation de la proctection par
// section critique. Si la macro TIMER_PROTECTION_ACTIVATED,
// le code généré est 100% réentrant. Si la macro n'est pas
// définie, le code généré ne supporte pas les appels
// concurents sur certaines de ses fonctions, mais convient
// parfaitement pour une utilisation mono-thread.
//
#ifdef TIMER_PROTECTION_ACTIVATED
#define TIMER_PROTECTION_INITIALIZE(s)   InitializeCriticalSection(s)
#define TIMER_PROTECTION_ENTER(s)        EnterCriticalSection(s)
#define TIMER_PROTECTION_LEAVE(s)        LeaveCriticalSection(s)
#define TIMER_PROTECTION_DELETE(s)       DeleteCriticalSection(s)
#pragma message( "Timer protection activated" )
#else
#define TIMER_PROTECTION_INITIALIZE(s)
#define TIMER_PROTECTION_ENTER(s)
#define TIMER_PROTECTION_LEAVE(s)
#define TIMER_PROTECTION_DELETE(s)
#pragma message( "Timer protection deactivated" )
#endif




//
// Définition de l'objet timer, seul le nom du type de la structure est
// exporté, la définition complète reste privée
//
typedef struct _TIMER_INSTANCE
{
    DWORD dwType;               // Type du timer
    DWORD dwResolution;         // Résolution du timer

    BOOL fActivated;            // Booléen indiquant si le timer est activé

    union
    {
        struct
        {

            DWORD  dwTick;               // Compteur pour timer interne
            BOOL   fTerminate;           // Flag de fin pour timer interne
            HANDLE hEvent;               // Handle de l'événement associé
            HANDLE hThread;              // Thread implémentant les timers
        }
            sSpecific;

        struct
        {
            HANDLE hTimer;              // Handle du timer de type WaitableTimer
        }
            sWaitable;
    };
  
#ifdef TIMER_PROTECTION_ACTIVATED
    CRITICAL_SECTION sCritical; // Protection des accés concurents
#endif
}
TIMER_INSTANCE;




PRIVATE DWORD TimerThread( TIMER_INSTANCE * psTimer );





/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC DWORD WINAPI TimerOpen( 
 *                      OUT TIMER_INSTANCE   ** ppsTimer )
 * PARAMETRES: ppsTimer : Récupère un "handle" de l'objet créé
 * RETOURNE  : NO_ERROR si le timer a été crée, sinon, un code Win32
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une instance de l'objet timer. Ce même objet
 *             pourra être détruit avec la fonction TimerClose().
 *             Pour obtenir un handle sur lequel peuvent être effectuée
 *             des opérations du type WaitForXXXX(), utiliser
 *             la fonction TimerGetWaitableHandle().
 *             Note : ne permet de travailler qu'avec un waitable timer.
 *             Pour les timer specifiques, utilser TimerOpenEx().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC DWORD WINAPI TimerOpen( 
        OUT TIMER_INSTANCE   ** ppsTimer )
{
    return TimerOpenEx( TIMER_TYPE_WAITABLE, ppsTimer );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC DWORD WINAPI TimerOpenEx( 
 *                      IN  DWORD               dwFlags,
 *                      OUT TIMER_INSTANCE   ** ppsTimer )
 * PARAMETRES: dwFlags  : Caractérisiques du timer
 *             ppsTimer : Récupère un "handle" de l'objet créé
 * RETOURNE  : NO_ERROR si le timer a été crée, sinon, un code Win32
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une instance de l'objet timer. Ce même objet
 *             pourra être détruit avec la fonction TimerClose().
 *             Pour obtenir un handle sur lequel peuvent être effectuée
 *             des opérations du type WaitForXXXX(), utiliser
 *             la fonction TimerGetWaitableHandle().
 *             Les flags possibles sont :
 *                 TIMER_TYPE_WAITABLE ou TIMER_TYPE_SPECIFIC_THREAD
 *                 TIMER_MAKE_RESOLUTION()
 *             Le type TIMER_TYPE_SPECIFIC_THREAD n'est utilisable que
 *             dans la version ré-entrante de la librairie.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC DWORD WINAPI TimerOpenEx( 
        IN  DWORD               dwFlags,
        OUT TIMER_INSTANCE   ** ppsTimer )
{
    DWORD               dwErr   = NO_ERROR; // Code d'erreur courant
    DWORD               dwType;             // Type de timer
    TIMER_INSTANCE    * psTimer = NULL;     // Pointeur de travail
    DWORD               dwFoo;

    __try
    {
        //
        // Vérifier que le type est bon
        //
        dwType = ( dwFlags & TIMER_TYPE_SELECT );
        if ( ( dwType != TIMER_TYPE_WAITABLE        ) &&
             ( dwType != TIMER_TYPE_SPECIFIC_THREAD ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

#ifndef TIMER_PROTECTION_ACTIVATED
        //
        // Le type TIMER_TYPE_SPECIFIC_THREAD n'est utilisable que
        // dans la version ré-entrante de la librairie.
        //
        if ( dwType == TIMER_TYPE_SPECIFIC_THREAD )
        {
            dwErr = ERROR_CALL_NOT_IMPLEMENTED;
            __leave;
        }
#endif

        //
        // Allouer un bloc de mémoire pour stocker les données de la nouvelle
        // instance de timer, puis le remplir avec les données nécessaire
        //
        psTimer = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psTimer ) );
        if ( ! psTimer ) 
        { 
            //
            // La tentative d'allocation d'un bloc a échoué
            //
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            __leave; 
        }

        psTimer->dwType = dwType;
        psTimer->dwResolution = TIMER_GET_RESOLUTION( dwFlags );
        if ( psTimer->dwResolution == 0 )
            psTimer->dwResolution = 10;

        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_INITIALIZE( &psTimer->sCritical );

        if ( dwType == TIMER_TYPE_WAITABLE )
        {
            psTimer->sWaitable.hTimer = CreateWaitableTimer( NULL, FALSE, NULL );
            if ( psTimer->sWaitable.hTimer == NULL ) 
            { 
                //
                // Le timer proprement dit n'a pas pu être créé
                //
                dwErr = GetLastError(); 
                __leave; 
            }
        }
        else
        {
            psTimer->sSpecific.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
            if ( psTimer->sSpecific.hEvent == NULL ) 
            { 
                //
                // L'événement n'a pas pu être créé
                //
                dwErr = GetLastError(); 
                __leave; 
            }

            //
            // Lancer le thread
            //
            psTimer->sSpecific.hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&TimerThread, psTimer, 0, &dwFoo );
            if ( psTimer->sSpecific.hThread == NULL )
            { 
                //
                // Le thread n'a pas pu être créé
                //
                dwErr = GetLastError(); 
                __leave; 
            }
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            //
            // En cas d'erreur, on libère ce qui a été alloué
            //
            TimerClose( psTimer );
            psTimer = NULL;
        }

        //
        // Retourner le handle du timer créé ou NULL
        //
        (*ppsTimer) = psTimer;
    }
	return dwErr;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC void WINAPI TimerClose( 
 *                      IN TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Destruction d'une instance de l'objet timer créé
 *             avec TimerOpen(). Si un handle a été obtenu par l'intermédiaire
 *             de l'objet avec la fontion TimerGetWaitableHandle(), celui-ci
 *             est également fermé.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC void WINAPI TimerClose( 
        IN TIMER_INSTANCE * psTimer )
{
    //
    // On s'assure d'abord que le bloc de l'objet a effectivement été alloué
    //
    if ( psTimer != NULL )
    {
        //
        // Cette fonction peut être appelée aussi bien par l'utilisateur
        // de la libraire que par TimerOpen lorsque l'ouverture a échoué
        // en cours d'initialisation. Il ne faut donc libérer que les
        // ressources réellement allouées.
        //

        if ( psTimer->dwType == TIMER_TYPE_WAITABLE )
        {
            if ( psTimer->sWaitable.hTimer != NULL )
                CloseHandle( psTimer->sWaitable.hTimer );
        }
        else
        {
            //
            // Forcer le flag d'arret
            //
            psTimer->sSpecific.fTerminate = TRUE;

            //
            // S'assurer que le thread ne tourne plus
            //
            if ( psTimer->sSpecific.hThread != NULL )
            {
                //
                // Attendre le temps de 3 boucles que le thread s'arrête
                //
                WaitForSingleObject( psTimer->sSpecific.hThread, 3 * psTimer->dwResolution );

                //
                // Au cas où le thread ne s'est pas arrété, on tente de forcer l'arret de
                // ce thread.
                //
                TerminateThread( psTimer->sSpecific.hThread, 0xDEADBEEF );

                CloseHandle( psTimer->sSpecific.hThread );
            }

            if ( psTimer->sSpecific.hEvent != NULL )
                CloseHandle( psTimer->sSpecific.hEvent );
        }

        //
        // La section critique est présente dès le moment où
        // l'allocation du bloc est réussie (création sans échec).
        // On peut donc l'effacer sans autre vérification.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_DELETE( &psTimer->sCritical );

        //
        // Finalement libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psTimer );
    }
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC HANDLE WINAPI TimerGetWaitableHandle( 
 *                      IN TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 * RETOURNE  : Un handle d'objet utilisable avec les fontions WaitForXXX
 * --------------------------------------------------------------------
 * ROLE      : Obtenir un handle lié au timer sur lequel on peut
 *             utiliser les fontions WaitForXXX
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC HANDLE WINAPI TimerGetWaitableHandle( 
        IN TIMER_INSTANCE * psTimer )
{
    if ( psTimer->dwType == TIMER_TYPE_WAITABLE )
        return psTimer->sWaitable.hTimer;
    else
        return psTimer->sSpecific.hEvent;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC DWORD WINAPI TimerActivate( 
 *                      IN TIMER_INSTANCE * psTimer,
 *                      IN DWORD            dwDelayMs )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 *             dwDelayMs : Délai de déclenchement du timer en millisecondes
 * RETOURNE  : NO_ERROR en cas d'armement effectué. Si le timer est déjà
 *             armé, ou qu'il est expiré mais n'a pas encore été
 *             acquité, la fonction renvoi ERROR_SIGNAL_PENDING.
 *             Si une autre erreur survient, un code d'erreur Win32 est renvoyé
 * --------------------------------------------------------------------
 * ROLE      : Activation du timer. Au bout du temps définit dans dwDelayMs, l'objet
 *             passe à l'état signalé. Pour tester cet état, utilise le handle
 *             retourné par TimerGetWaitableHandle().
 *             Pour pouvoir réarmer un timer, celui-ci doit être acquité avec
 *             la fonction TimerAcknowledge().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC DWORD WINAPI TimerActivate( 
        IN TIMER_INSTANCE * psTimer,
        IN DWORD            dwDelayMs )
{
    DWORD           dwErr = NO_ERROR;   // Code d'erreur courant
    BOOL            fResult;            // Résultat d'appels systèmes
    LARGE_INTEGER   sDelay;             // Conversion du délai en centaines de nanosecondes sur un mot 64 bits

    __try
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_ENTER( &psTimer->sCritical );

        //
        // S'assurer que les paramètres sont un minimul valides
        //
        if ( dwDelayMs == 0 )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        //
        // S'assurer que le timer n'est pas déjà activé (qu'il soit ou ne
        // soit pas arrivé à expiration)
        //
        if ( psTimer->fActivated )
        { 
            //
            // Le timer est déjà activé, on part en erreur
            //
            dwErr = ERROR_SIGNAL_PENDING; 
            __leave; 
        }

        if ( psTimer->dwType == TIMER_TYPE_WAITABLE )
        {
            //
            // Convertir la valeur 32 bits en ms en une valeur signée
            // négative (temps relatif) 64 bits en centaines de ns
            //
            sDelay.QuadPart = - (signed __int64)dwDelayMs * (signed __int64)10000;

            //
            // Armer le timer
            //
            fResult = SetWaitableTimer(
                psTimer->sWaitable.hTimer,
                &sDelay,
                0,
                NULL,
                NULL,
                FALSE );
            if ( ! fResult ) 
            { 
                //
                //  L'armement du timer a échoué
                //
                dwErr = GetLastError();
                __leave; 
            }
        }
        else
        {
            //
            // S'assurer que l'événement n'est pas déjà signalé
            //
            ResetEvent( psTimer->sSpecific.hEvent );

            //
            // Armer le compteur en fonction de la résolution attendue. On s'assure que la valeur
            // du compteur soit strictement supérieure à 1.
            //
            psTimer->sSpecific.dwTick = ( dwDelayMs + psTimer->dwResolution - 1 ) / psTimer->dwResolution;
        }

        //
        //  Désormais, le timer est activé
        //
        psTimer->fActivated = TRUE;
    }
    __finally
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_LEAVE( &psTimer->sCritical );
    }
	return dwErr;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC DWORD WINAPI TimerCancel( 
 *                      IN TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 * RETOURNE  : NO_ERROR si le timer était activé et pas encore arrivé à expiration.
 *             ERROR_INVALID_PARAMETER si le timer n'était pas activé.
 *             ERROR_TIMEOUT si le timer était arrivé à expiration.
 *             Dans tous les cas précédent, le timer n'est plus actif.
 *             Si une autre erreur survient, un code Win32 est renvoyé.
 * --------------------------------------------------------------------
 * ROLE      : Annuler l'armement d'un timer.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC DWORD WINAPI TimerCancel( 
        IN TIMER_INSTANCE * psTimer )
{
    DWORD   dwErr = NO_ERROR;   // Code d'erreur courant

    __try
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_ENTER( &psTimer->sCritical );

        //
        // S'assurer que le timer est pas activé (qu'il soit ou ne
        // soit pas arrivé à expiration)
        //
        if ( ! psTimer->fActivated ) 
        { 
            //
            // Le timer n'est pas activé, on ne peut donc pas
            // l'annuler
            //
            dwErr = ERROR_INVALID_PARAMETER;
            __leave; 
        }

        //
        // Désactivation
        //
        psTimer->fActivated = FALSE;

        //
        // Annuler le timer.
        //
        if ( psTimer->dwType == TIMER_TYPE_WAITABLE )
        {
            //
            // L'annulation d'un waitable timer ne change pas son
            // état de signalement
            //
            CancelWaitableTimer( psTimer->sWaitable.hTimer );

            //
            // Pour le remettre à un état non signalé, on effectue
            // une attente (sans délai).
            //
            WaitForSingleObject( psTimer->sWaitable.hTimer, 0 );
        }
        else
        {
            //
            // On force l'événement à un état non signalé
            //
            ResetEvent( psTimer->sSpecific.hEvent );

            //
            // Le compteur est remis à zéro
            //
            psTimer->sSpecific.dwTick = 0;
        }

        //
        // Quel que soit l'état dans lequel était le timer,
        // maintenant, il est complètement désactivé 
        //
        psTimer->fActivated = FALSE;
    }
    __finally
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_LEAVE( &psTimer->sCritical );
    }
	return dwErr;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC DWORD WINAPI TimerAcknowledge( 
 *                      IN TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 * RETOURNE  : NO_ERROR si le timer était activé et qu'il est arrivé à expiration.
 *             ERROR_INVALID_PARAMETER si le timer n'était pas activé.
 *             ERROR_INVALID_DATA si le timer était activé sans être arrivé à expiration.
 *             Dans tous les cas précédents, le timer n'est plus actif.
 *             Si une autre erreur survient, un code Win32 est renvoyé.
 * --------------------------------------------------------------------
 * ROLE      : Acquiter l'expiration d'un timer.
 *             L'action de cette fonction est similaire à TimerCancel(), seules
 *             les valeurs de retour changent.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC DWORD WINAPI TimerAcknowledge( 
        IN TIMER_INSTANCE * psTimer )
{
    DWORD   dwErr = NO_ERROR;   // Code d'erreur courant

    __try
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_ENTER( &psTimer->sCritical );

        //
        // S'assurer que le timer est pas activé (qu'il soit ou ne
        // soit pas arrivé à expiration)
        //
        if ( ! psTimer->fActivated ) 
        { 
            //
            // Le timer n'est pas activé, on ne peut donc pas
            // l'annuler
            //
            dwErr = ERROR_INVALID_PARAMETER;
            __leave; 
        }

        //
        // Désactivation
        //
        psTimer->fActivated = FALSE;

        //
        // Annuler le timer.
        // ATTENTION, dans le 2 cas, ceci n'a pas d'inflence sur son état de signalement
        //
        if ( psTimer->dwType == TIMER_TYPE_WAITABLE )
        {
            //
            // L'annulation d'un waitable timer ne change pas son
            // état de signalement
            //
            CancelWaitableTimer( psTimer->sWaitable.hTimer );

            //
            // Pour le remettre à un état non signalé, on effectue
            // une attente (sans délai).
            //
            if ( WaitForSingleObject( psTimer->sWaitable.hTimer, 0 ) != WAIT_TIMEOUT )
            {
                //
                // Il n'était pas dans un état signalé, ce qui signifie que
                // l'acquitement est fait sur un timer non échu
                //
                dwErr = ERROR_INVALID_DATA;
            }
        }
        else
        {
            //
            // On force l'événement à un état non signalé
            //
            ResetEvent( psTimer->sSpecific.hEvent );

            if ( psTimer->sSpecific.dwTick != 0 )
            {
                //
                // Si l'acquitement est fait sur un timer non échu, c'est une erreur
                //
                dwErr = ERROR_INVALID_DATA;
            }

            //
            // Le compteur est remis à zéro
            //
            psTimer->sSpecific.dwTick = 0;
        }

    }
    __finally
    {
        //
        // Section critique pour la protection des accés concurents.
        // REMARQUE : Rendu conditionnel par l'utilisation de la macro.
        //
        TIMER_PROTECTION_LEAVE( &psTimer->sCritical );
    }
	return dwErr;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PUBLIC BOOL WINAPI TimerIsActive( 
 *                      IN TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet créé avec TimerOpen().
 * RETOURNE  : TRUE si un timer est actif (soit pending, soit arrivé à terme
 *             et non acquité. FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Déterminer l'état d'un timer
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PUBLIC BOOL WINAPI TimerIsActive( 
        IN TIMER_INSTANCE * psTimer )
{
    return psTimer->fActivated;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE DWORD TimerThread( TIMER_INSTANCE * psTimer )
 * PARAMETRES: psTimer : "handle" d'objet timer à gérer par ce thread
 * RETOURNE  : TRUE si un timer est actif (soit pending, soit arrivé à terme
 *             et non acquité. FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Déterminer l'état d'un timer
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE DWORD TimerThread( TIMER_INSTANCE * psTimer )
{
    //
    // Faire en sorte d'avoir une priorité maximale
    //
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
    SetThreadPriorityBoost( GetCurrentThread(), TRUE );

    //
    // Boucler sur le flag d'arret
    //
    while ( ! psTimer->sSpecific.fTerminate )
    {
        //
        // Marquer une pause correspondant à la résolution du timer
        //
        Sleep( psTimer->dwResolution );

        TIMER_PROTECTION_ENTER( &psTimer->sCritical );

        if ( psTimer->fActivated )
        {
            //
            // Le timer est actif
            //
            if ( psTimer->sSpecific.dwTick != 0 )
            {
                //
                // Il n'est pas encore arrivé à échéance. On décrémente
                // le compteur.
                //
                psTimer->sSpecific.dwTick --;
                if ( psTimer->sSpecific.dwTick == 0 )
                {
                    //
                    // Le compteur a atteint 0, on est arrivé à échéance, il
                    // faut signaler l'objet événement.
                    //
                    SetEvent( psTimer->sSpecific.hEvent );
                }
            }
        }

        TIMER_PROTECTION_LEAVE( &psTimer->sCritical );
    }

    ExitThread( 0 );
    return 0;
}

