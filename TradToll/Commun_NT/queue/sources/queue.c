/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QUEUE
 * FILE       : QUEUE.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : File d'attente, queue
 * --------------------------------------------------------------------
 * SUMMARY    : Librairie de gestion événementielle d'une file d'attente.
 * --------------------------------------------------------------------
 * DESCRIPTION: La file d'attente est du type FIFO à taille limitée.
 *              Les éléments insérés sont des valeurs 32 bits (comme des
 *              pointeurs. A tout moment, il est possible de retirer un
 *              élément de la file.
 *              A la file sont associés deux objets événements. L'un
 *              étant signalé lorsque la file contient au moins un élément.
 *              L'autre étant signalé tant que la file n'est pas pleine.
 *              Le premier est utilisable de manière externe, le second
 *              est uniquement utilisé en interne.
 *              La librairie est 100% réentrante, c'est à dire qu'une
 *              même instance de file d'attente est utilisable simultanément
 *              par plusieurs threads en écriture (empilement dans la file),
 *              plusieurs threads en lecture (dépilement de la file), voire
 *              plusieurs thread en annulation (retrait de la file).
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#ifndef QUEUE_EXPORTS
#define QUEUE_EXPORTS
#endif

#include "csr_queue.h"
#undef LOC_DEF

#include <memclass.h>

//
// Encapsulation de l'utilisation de la proctection par
// section critique. Si la macro QUEUE_PROTECTION_ACTIVATED,
// le code généré est 100% réentrant. Si la macro n'est pas
// définie, le code généré ne supporte pas les appels
// concurents sur certaines de ses fonctions, mais convient
// parfaitement pour une utilisation mono-thread.
//
#ifdef QUEUE_PROTECTION_ACTIVATED
#define QUEUE_PROTECTION_INITIALIZE(s)   InitializeCriticalSection(s)
#define QUEUE_PROTECTION_ENTER(s)        EnterCriticalSection(s)
#define QUEUE_PROTECTION_LEAVE(s)        LeaveCriticalSection(s)
#define QUEUE_PROTECTION_DELETE(s)       DeleteCriticalSection(s)
#pragma message( "Protection réentrance activée" )
#else
#define QUEUE_PROTECTION_INITIALIZE(s)
#define QUEUE_PROTECTION_ENTER(s)
#define QUEUE_PROTECTION_LEAVE(s)
#define QUEUE_PROTECTION_DELETE(s)
#pragma message( "Protection réentrance désactivée" )
#endif


//
// Définition de la structure d'instance
//
typedef struct _QUEUE_INSTANCE
{
    //Nombre d'éléments max dans la file
    DWORD               dwCount;

    // Handle de l'événement signalant qu'au moins un emplacement est dispo
    HANDLE              hFreeEvt;

    // Handle de l'événement signalant qu'au moins un emplacement est rempli
    HANDLE              hItemEvt;

    // Section critique permettant de sérialiser les accés à l'objet
#ifdef QUEUE_PROTECTION_ACTIVATED
    CRITICAL_SECTION sCritical; // Protection des accés concurents
#endif

    // Position du premier élément dans la liste circulaire
    DWORD               dwFirst;

    // Position du dernier élément dans la liste circulaire
    DWORD               dwLast;

    // Liste circulaire
    void              * tpvQueue[];
}
    QUEUE_INSTANCE;




PRIVATE void WINAPI _QueueRefresh( 
        IN QUEUE_INSTANCE * psQueue );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI QueueOpen( 
 *                      OUT QUEUE_INSTANCE   ** ppsQueue, 
 *                      IN DWORD                dwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: ppsQueue : Renvoie un handle de file d'attente
 *             dwCount  : Nombre d'éléments max dans la file
 * --------------------------------------------------------------------
 * RETURN    : NO_ERROR si succes, une erreur WIN32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une nouvelle instance de file d'attente.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI QueueOpen( 
        OUT QUEUE_INSTANCE   ** ppsQueue, 
        IN DWORD                dwCount )
{
    DWORD               dwErr = NO_ERROR;
    QUEUE_INSTANCE    * psQueue = NULL;

    __try
    {
        (*ppsQueue) = NULL;

        //
        // Du fait de la gestion de la file en anneau fermé, il faut un élément
        // de plus dans la file, de le nombre max d'élément empilable.
        //
        dwCount ++;

        psQueue = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psQueue ) + sizeof( void* ) * dwCount );
        if ( psQueue == NULL )
        {
            //
            // L'allocation du bloc contenant les données de l'objet a échoué
            //
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Initialiser ce qui doit l'être
        //
        psQueue->dwCount = dwCount;
        QUEUE_PROTECTION_INITIALIZE( &psQueue->sCritical );

        //
        // Créer les événements qui vont permettre des attentes blocantes sans polling
        // selon les états de la file d'attente.
        //

        //
        // Evénement passant à l'état signalé lorsque la file a au moins
        // un emplacement de libre (file non pleine)
        //
        psQueue->hFreeEvt = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( psQueue->hFreeEvt == NULL )
        {
            //
            // Echec de création de l'événement
            //
            dwErr = GetLastError();
            __leave;
        }
        
        //
        // Evénement passant à l'état signalé lorsque la file contient au
        // moins un éléments.
        //
        psQueue->hItemEvt = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psQueue->hItemEvt == NULL )
        {
            //
            // Echec de création de l'événement
            //
            dwErr = GetLastError();
            __leave;
        }

        (*ppsQueue) = psQueue;
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
            QueueClose( psQueue );

    }
	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC void WINAPI QueueClose( 
 *                      IN QUEUE_INSTANCE * psQueue )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue : Handle de file d'attente renvoyé par QueueOpen 
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'une file d'attente créée avec QueueOpen().
 * --------------------------------------------------------------------
 */
PUBLIC void WINAPI QueueClose( 
        IN QUEUE_INSTANCE * psQueue )
{
    //
    // On s'assure d'abord que le bloc de l'objet a effectivement été alloué
    //
    if ( psQueue != NULL )
    {
        QUEUE_PROTECTION_ENTER( &psQueue->sCritical );

        //
        // Cette fonction peut être appelé aussi bien par l'utilisateur
        // de la libraire que par SERL1Open lorsque l'ouverture a échoué
        // en cours d'initialisation. Il ne faut donc libérer que les
        // ressources réellement allouées.
        //

        if ( psQueue->hFreeEvt != NULL )
            CloseHandle( psQueue->hFreeEvt );

        if ( psQueue->hItemEvt != NULL )
            CloseHandle( psQueue->hItemEvt );

        QUEUE_PROTECTION_DELETE( &psQueue->sCritical );

        //
        // Finalement libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psQueue );
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC BOOL WINAPI QueueIsEmpty( 
 *                      IN QUEUE_INSTANCE * psQueue )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue : Handle de file d'attente renvoyé par QueueOpen 
 * --------------------------------------------------------------------
 * RETURN    : TRUE si la file est vide, FALSE sinon
 * --------------------------------------------------------------------
 * ROLE      : Détermine si une file d'attente est vide.
 * --------------------------------------------------------------------
 */
PUBLIC BOOL WINAPI QueueIsEmpty( 
        IN QUEUE_INSTANCE * psQueue )
{
    BOOL fEmpty;

    QUEUE_PROTECTION_ENTER( &psQueue->sCritical );

    fEmpty = ( psQueue->dwFirst == psQueue->dwLast );

    QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );

    return fEmpty;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC BOOL WINAPI QueueIsFull( 
 *                      IN QUEUE_INSTANCE * psQueue )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue : Handle de file d'attente renvoyé par QueueOpen 
 * --------------------------------------------------------------------
 * RETURN    : TRUE si la file est vide, FALSE sinon
 * --------------------------------------------------------------------
 * ROLE      : Détermine si une file d'attente est pleine.
 * --------------------------------------------------------------------
 */
PUBLIC BOOL WINAPI QueueIsFull( 
        IN QUEUE_INSTANCE * psQueue )
{
    BOOL fFull;

    QUEUE_PROTECTION_ENTER( &psQueue->sCritical );

    fFull = ( psQueue->dwFirst == ( ( psQueue->dwLast + 1 ) % psQueue->dwCount ) );

    QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );

    return fFull;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI QueueWriteItem( 
 *                      IN QUEUE_INSTANCE * psQueue,
 *                      IN void           * pvItem,
 *                      IN DWORD            dwTimeout)
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue   : Handle de file d'attente renvoyé par QueueOpen 
 *             pvItem    : Pointeur à placer en file d'attente
 *             dwTimeout : Délai maximum autorisé en millisecondes.
 * --------------------------------------------------------------------
 * RETURN    : NO_ERROR si l'élément a pu être placé en file, WAIT_TIMEOUT
 *             en cas de dépassement de délai, un autre code Win32 sinon.
 * --------------------------------------------------------------------
 * ROLE      : Tente d'ajouter un élément dans une file d'attente. Si la
 *             file est pleine, la fonction reste bloquée jusqu'à ce que
 *             l'élément ait pu être rajouté ou que dwTimeout millisecondes
 *             se soient écoulées.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI QueueWriteItem( 
        IN QUEUE_INSTANCE * psQueue,
        IN void           * pvItem,
        IN DWORD            dwTimeout)
{
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    BOOL  fFull;
    BOOL  fInCritical = FALSE;

    __try
    {
        do
        {
            //
            // Attente blocante sur l'événement indiquant qu'au moins un
            // emplacement de la file est vide.
            //
            dwRes = WaitForSingleObject( psQueue->hFreeEvt, dwTimeout );
            if ( dwRes == WAIT_TIMEOUT )
            {
                dwErr = WAIT_TIMEOUT;
                __leave;
            }

            if ( dwRes != WAIT_OBJECT_0 )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            //
            // L'événement étant auto reset, on limite les risques de contention,
            // mais on ne les évite pas. Il nous faut donc entrer en section
            // critique et s'assurer qu'on peu placer l'élément, sinon, on
            // reboucle.
            //
            QUEUE_PROTECTION_ENTER( &psQueue->sCritical );
            fInCritical = TRUE;

            fFull = QueueIsFull( psQueue );
            if ( fFull )
            {
                //
                // Malgré le signal de l'événement, la file est pleine. Il y avait
                // donc contention, et un des thread en attente nous est passé devant.
                // On doit recommencer l'attente  (avec un petit délai entre temps,
                // histoire de ne pas tout bloquer)
                //
                QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );
                fInCritical = FALSE;

                Sleep( 1 );
            }
        }
        while ( fFull );

        //
        // On est en accés protégé sur la file d'attente et elle n'est pas
        // pleine. On peut donc ajouter notre élément sans problème.
        //
        psQueue->tpvQueue[psQueue->dwLast] = pvItem;
        psQueue->dwLast = ( ( psQueue->dwLast + 1 ) % psQueue->dwCount );
          
        //
        // Remettre à jour l'état des événements en fonction de l'état de la file
        //
        _QueueRefresh( psQueue );

    }
    __finally
    {
        //
        // Si on est pas encore sortie de la section critique, on en sort.
        //
        if ( fInCritical ) 
            QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );

    }
	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI QueueReadItem( 
 *                      IN QUEUE_INSTANCE * psQueue,
 *                      IN void          ** ppvItem,
 *                      IN DWORD            dwTimeout )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue   : Handle de file d'attente renvoyé par QueueOpen 
 *             ppvItem   : Retourne le pointeur placé précédemment en file
 *             dwTimeout : Délai maximum autorisé en millisecondes.
 * --------------------------------------------------------------------
 * RETURN    : NO_ERROR si l'élément a pu être retiré de la file, WAIT_TIMEOUT
 *             en cas de dépassement de délai, un autre code Win32 sinon.
 * --------------------------------------------------------------------
 * ROLE      : Tente de retirer le prochain élément d'une file d'attente. Si la
 *             file est vide, la fonction reste bloquée jusqu'à ce que
 *             qu'un élément y soit placé ou que dwTimeout millisecondes
 *             se soient écoulées.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI QueueReadItem( 
        IN QUEUE_INSTANCE * psQueue,
        IN void          ** ppvItem,
        IN DWORD            dwTimeout )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwRes;
    BOOL  fEmpty;
    BOOL  fInCritical = FALSE;

    __try
    {
        //
        // Par défaut, on retourne NULL
        //
        (*ppvItem) = NULL;

        do
        {
            //
            // Attente blocante sur l'événement indiquant qu'au moins un
            // élément est disponible dans la file est vide.
            //
            dwRes = WaitForSingleObject( psQueue->hItemEvt, dwTimeout );
            if ( dwRes == WAIT_TIMEOUT )
            {
                dwErr = WAIT_TIMEOUT;
                __leave;
            }

            if ( dwRes != WAIT_OBJECT_0 )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            //
            // L'événement étant auto reset, on limite les risques de contention,
            // mais on ne les évite pas. Il nous faut donc entrer en section
            // critique et s'assurer qu'on peu retirer l'élément, sinon, on
            // reboucle.
            //
            QUEUE_PROTECTION_ENTER( &psQueue->sCritical );
            fInCritical = TRUE;

            fEmpty = QueueIsEmpty( psQueue );
            if ( fEmpty )
            {
                //
                // Malgré le signal de l'événement, la file est vide. Il y avait
                // donc contention, et un des thread en attente nous est passé devant.
                // On doit recommencer l'attente  (avec un petit délai entre temps,
                // histoire de ne pas tout bloquer)
                //
                QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );
                fInCritical = FALSE;
                Sleep( 1 );
            }
        }
        while ( fEmpty );

        //
        // On est en accés protégé sur la file d'attente et elle n'est pas
        // vide. On peut donc retire le premier élément sans problème.
        //
        (*ppvItem) = psQueue->tpvQueue[psQueue->dwFirst];
        psQueue->dwFirst = ( ( psQueue->dwFirst + 1 ) % psQueue->dwCount );

        //
        // Remettre à jour l'état des événements en fonction de l'état de la file
        //
        _QueueRefresh( psQueue );
    }
    __finally
    {
        //
        // Si on est pas encore sortie de la section critique, on en sort.
        //
        if ( fInCritical ) 
            QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );

    }
	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC DWORD WINAPI QueueCancelItem( 
 *                      IN QUEUE_INSTANCE * psQueue,
 *                      IN void           * pvItem )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue   : Handle de file d'attente renvoyé par QueueOpen 
 *             pvItem    : Elément à retire de la file d'attente.
 * --------------------------------------------------------------------
 * RETURN    : NO_ERROR si l'élément a pu être retiré de la file, un autre code Win32 sinon.
 * --------------------------------------------------------------------
 * ROLE      : Tente de retirer un élément d'une file d'attente, quelle que
 *             soit sa position.
 * --------------------------------------------------------------------
 */
PUBLIC DWORD WINAPI QueueCancelItem( 
        IN QUEUE_INSTANCE * psQueue,
        IN void           * pvItem )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwCurrent;
    DWORD dwNext;
    DWORD dwCount = 0;

    __try
    {
        QUEUE_PROTECTION_ENTER( &psQueue->sCritical );

        //
        // Si la file est vide, il est inutile de faire une annulation
        //
        if ( QueueIsEmpty( psQueue ) )
        {
            dwErr = ERROR_NO_MORE_ITEMS;
            __leave;
        }

        //
        // Scanner la liste pour trouver les éléments à supprimer
        //
        dwNext = dwCurrent = psQueue->dwFirst;
        while ( dwCurrent != psQueue->dwLast )
        {
            if ( psQueue->tpvQueue[dwCurrent] != pvItem )
            {
                psQueue->tpvQueue[dwNext] = psQueue->tpvQueue[dwCurrent];
                dwNext = ( dwNext + 1 ) % psQueue->dwCount;
            }
            dwCurrent = ( dwCurrent + 1 ) % psQueue->dwCount;
            dwCount ++;
        }
        psQueue->dwLast = dwNext;

        if ( dwCount == 0 )
            dwErr = ERROR_NO_MORE_ITEMS;

        _QueueRefresh( psQueue );
    }
    __finally
    {
        QUEUE_PROTECTION_LEAVE( &psQueue->sCritical );

    }
	return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PUBLIC HANDLE WINAPI QueueGetWaitableHandle( 
 *                      IN QUEUE_INSTANCE * psQueue )
 * --------------------------------------------------------------------
 * PARAMETERS: psQueue   : Handle de file d'attente renvoyé par QueueOpen 
 * --------------------------------------------------------------------
 * RETURN    : Un handle d'événement sur lequel on peut effectuer les operations
 *             d'attente du type WaitForXXX.
 * --------------------------------------------------------------------
 * ROLE      : Obtient un handle d'événement passant à l'état signalé lorsque
 *             la file contient au moins un élément, et passant à l'état non
 *             signalé lorsqu'un n'en a plus.
 * --------------------------------------------------------------------
 */
PUBLIC HANDLE WINAPI QueueGetWaitableHandle( 
        IN QUEUE_INSTANCE * psQueue )
{
    return psQueue->hItemEvt;
}




//
// PROJECT WIDE FUNCTIONS CODE
//




//
// PRIVATE FUNCTIONS CODE
//


PRIVATE void WINAPI _QueueRefresh( 
        IN QUEUE_INSTANCE * psQueue )
{
    if ( ! QueueIsFull( psQueue ) )
        SetEvent( psQueue->hFreeEvt );
    else
        ResetEvent( psQueue->hFreeEvt );

    if ( ! QueueIsEmpty( psQueue ) )
        SetEvent( psQueue->hItemEvt );
    else
        ResetEvent( psQueue->hItemEvt );
}

