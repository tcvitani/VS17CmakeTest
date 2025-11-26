/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TIMER
 * FILE       : CSR_TIMER.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des timers
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

#ifndef CSR_TIMER_H
#define CSR_TIMER_H

#ifdef TIMER_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif



#define TIMER_TYPE_SELECT              0xF0000000
#define TIMER_TYPE_WAITABLE            0x10000000
#define TIMER_TYPE_SPECIFIC_THREAD     0x20000000

#define TIMER_RESOLUTION_SELECT        0x0FF00000
#define TIMER_MAKE_RESOLUTION(x)       ((((DWORD)(x))&0x000000FF)<<20)
#define TIMER_GET_RESOLUTION(x)        ((((DWORD)(x))&0x0FF00000)>>20)


//
// Définition de la structure d'instance (contenu non exporté)
//
typedef struct _TIMER_INSTANCE TIMER_INSTANCE;




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
        OUT TIMER_INSTANCE   ** ppsTimer );



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
        OUT TIMER_INSTANCE   ** ppsTimer );



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
        IN TIMER_INSTANCE * psTimer );



        
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
        IN TIMER_INSTANCE * psTimer );



        
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
        IN DWORD            dwDelayMs );




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
        IN TIMER_INSTANCE * psTimer );




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
        IN TIMER_INSTANCE * psTimer );




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
        IN TIMER_INSTANCE * psTimer );



#endif
