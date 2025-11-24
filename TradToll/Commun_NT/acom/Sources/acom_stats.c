/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_stats.c
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

#include <windows.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#define LOC_DEF
#include <acom_stats.h>
#undef LOC_DEF

#include <memclass.h>


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
PROTECTED void AComStatsReset( ACOM_CONNECTION * psCnx )
{
    AComDbgInfo( __FILE__, __LINE__, "AComStatsReset(0x%016X)", psCnx );
    GetSystemTimeAsFileTime( (LPFILETIME)&psCnx->ullCnxDate );
    psCnx->ullInBytes = (ULONGLONG)0;
    psCnx->ullOutBytes = (ULONGLONG)0;
    psCnx->ullInMsg = (ULONGLONG)0;
    psCnx->ullOutMsg = (ULONGLONG)0;
}


/* -------------  FIN DU FICHIER : acom_stats.c ------------- */ 
