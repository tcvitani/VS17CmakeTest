/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_treads.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Thread de gestion des transferts
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_THREADS_H
#define CM_THREADS_H

#include <protect.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadLink( DWORD dwLink )
 * --------------------------------------------------------------------
 * PARAMETERS: dwLink : Index du lien que le thread doit gérer
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion d'un lien COMMOC.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadLink( DWORD dwLink );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadDispatch( DWORD dwFoo )
 * --------------------------------------------------------------------
 * PARAMETERS: dwFoo : Non utilisé
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion du répertoire de
 *             dispatching.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadDispatch( DWORD dwFoo );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadWorker( DWORD dwWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: dwWorker : Index du worker que le thread doit gérer
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion d'un worker.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadWorker( DWORD dwWorker );




#endif