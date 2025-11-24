/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_pipe.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des connexions pour les pipes
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_PIPE_H
#define ACOM_PIPE_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComPipeServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un pipe serveur. En fait,
 *             tente de créér le serveur de pipe et de le mettre en attente
 *             de connexion. Si la création échoue, la connexion reste à l'état
 *             "déconnecté", sinon, elle passe à l'état "en cours de connexion".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComPipeServerConnect( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComPipeClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un pipe client. En fait,
 *             tente de créér le client de pipe. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComPipeClientConnect( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComPipeGetIdentity( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (pipe = identifié pour serveur uniquement)
 * REMARQUE  : Suppose que le slot de connexion est bien associé à
 *             un pipe (client ou serveur), dans un état "connecté" et
 *             qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComPipeGetIdentity( ACOM_CONNECTION * psCnx );


#endif


/* -------------  FIN DU FICHIER : acom_pipe.h ------------- */ 
