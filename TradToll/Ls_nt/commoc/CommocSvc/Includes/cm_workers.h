/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_workers.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des workers par l'intermédiaire des librairie
 *              d'extension, et définition des comportements par
 *              défaut (equivalent d'une libraire d'extension).
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_WORKERS_H
#define CM_WORKERS_H

#include <protect.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED  BOOL CMWorkerInit( CM_WORKER * psWorker, char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker   : Pointe sur la structure de gestion du worker à initialiser
 *             szName     : Pointe sur le nom attribué au worker
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres du worker
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure de gestion d'un worker
 * --------------------------------------------------------------------
 */
PROTECTED  BOOL CMWorkerInit( CM_WORKER * psWorker, char * szName, HKEY hKeyConfig );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerTerminate( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un worker
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerTerminate( CM_WORKER * psWorker );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerLock( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerLock( CM_WORKER * psWorker );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMWorkerLock( CM_WORKER * psWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: psWorker     : Pointe sur la structure de gestion d'un worker initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMWorkerUnlock( CM_WORKER * psWorker );




#endif