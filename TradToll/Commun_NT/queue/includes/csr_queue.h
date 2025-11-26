/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QUEUE
 * FILE       : CSR_QUEUE.H
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

#ifndef CSR_QUEUE_H
#define CSR_QUEUE_H

#ifdef QUEUE_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif



//
// Définition de la structure d'instance (contenu non exporté)
//
typedef struct _QUEUE_INSTANCE QUEUE_INSTANCE;



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
        IN DWORD                dwCount );




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
        IN QUEUE_INSTANCE * psQueue );




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
        IN QUEUE_INSTANCE * psQueue );




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
        IN QUEUE_INSTANCE * psQueue );




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
        IN DWORD            dwTimeout);

        


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
        IN DWORD            dwTimeout );




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
        IN void           * pvItem );




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
        IN QUEUE_INSTANCE * psQueue );




#endif
