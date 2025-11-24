/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_key.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des clés séquentielles pour génération des handles
 *              de connexions.
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
#include <acom_key.h>
#undef LOC_DEF

#include <memclass.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComKeyGetNextKey ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Une nouvelle clé pour générer un handle
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Génère une clé séquentielle utilisée comme handle de connexion
 *             en se basant sur les principes suivants :
 *                 . jamais nulle
 *                 . partie faible générée d'après l'index de la connexion dans le tableau
 *                   du working set
 *                 . partie forte générée d'après un compteur spécifique au working set
 *                 . utilisation de ACOM_MAX_WKS_CONNECTIONS pour séparer partie forte
 *                   et faible (modulo et division entière).
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComKeyGetNextKey( ACOM_CONNECTION * psCnx )
{
    DWORD dwKey;

    AComDbgInfo( __FILE__, __LINE__, "AComKeyGetNextKey(0x%016X)", psCnx );

    do
    {
        dwKey = InterlockedIncrement( &psCnx->psWks->dwNextKey );
        dwKey = dwKey * ACOM_MAX_WKS_CONNECTIONS;
        dwKey = ( dwKey | psCnx->dwIndex );
    }
    while ( dwKey == 0 );

    AComDbgInfo( __FILE__, __LINE__, "AComKeyGetNextKey return 0x%016X", dwKey );

    return dwKey;
}

/* -------------  FIN DU FICHIER : acom_key.c ------------- */ 
