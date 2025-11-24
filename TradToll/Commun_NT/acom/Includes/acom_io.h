/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_io.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des i/o + déconnexion
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_IO_H
#define ACOM_IO_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerDisconnectOnError ( ACOM_CONNECTION * psCnx, DWORD dwErr )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 *             DWORD dwErr             : Erreur ayant provoqué la déconnexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la déconnexion d'une connexion lorsqu'une erreur est survenue
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerDisconnectOnError( ACOM_CONNECTION * psCnx, DWORD dwErr );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerReceiveNext ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie une i/o de lecture, sauf si l'attribut bIsOverlapped de la
 *             structure de connexion est à FALSE. Dans ce dernier cas, la fonction
 *             est bloquée jusqu'à reception, un i/o completion packet est alors
 *             envoyé comme si la lecture avait été faite de manière assynchrone.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerReceiveNext( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerSendNext ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie une i/o d'écriture, sauf si l'attribut bIsOverlapped de la
 *             structure de connexion est à FALSE. Dans ce dernier cas, la fonction
 *             est bloquée jusqu'à émission, un i/o completion packet est alors
 *             envoyé comme si l'écriture avait été faite de manière assynchrone.
 *             REMARQUE : L'i/o d'écriture n'est lancée que lorsqu'il y a un
 *             bloc dans la file des envois. Sinon, la fonction sort immédiatement
 *             sans rien lancer.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerSendNext( ACOM_CONNECTION * psCnx );


#endif


/* -------------  FIN DU FICHIER : acom_io.h ------------- */ 
