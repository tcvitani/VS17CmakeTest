/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_worker.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des threads worker
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_WORKER_H
#define ACOM_WORKER_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerThread ( ACOM_WORKING_SET * psWks )
 * PARAMETERS: ACOM_WORKING_SET * psWks : Structure de workings set.
 * RETURN    : NO_ERROR si OK, un code Win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée d'un thread worker.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerThread( ACOM_WORKING_SET * psWks );


#endif


/* -------------  FIN DU FICHIER : acom_worker.h ------------- */ 
