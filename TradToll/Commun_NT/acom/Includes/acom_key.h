/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_key.h
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

#ifndef ACOM_KEY_H
#define ACOM_KEY_H

#include <protect.h>


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
PROTECTED DWORD AComKeyGetNextKey( ACOM_CONNECTION * psCnx );

#endif


/* -------------  FIN DU FICHIER : acom_key.h ------------- */ 
