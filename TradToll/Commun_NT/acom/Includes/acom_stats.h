/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_stats.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des statistiques
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_STATS_H
#define ACOM_STATS_H

#include <protect.h>

// MACROS DE MISE A JOUR DES STATISTIQUES

// Mise à jour des statistiques sur réception
#define ACOM_STATS_RECEIVED(psCnx,dwBytes) ((psCnx->ullInBytes+=(ULONGLONG)dwBytes ),(psCnx->ullInMsg++))

// Mise à jour des statistiques sur émission
#define ACOM_STATS_SENT(psCnx,dwBytes)     ((psCnx->ullOutBytes+=(ULONGLONG)dwBytes ),(psCnx->ullOutMsg++))


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComStatsReset ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Met à zéro les statistiques d'une connexion
 * --------------------------------------------------------------------
 */
PROTECTED void AComStatsReset( ACOM_CONNECTION * psCnx );


#endif


/* -------------  FIN DU FICHIER : acom_stats.h ------------- */ 
