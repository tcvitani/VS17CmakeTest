/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_mail.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des "connexions" pour les mailslots
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_MAIL_H
#define ACOM_MAIL_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComMailServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un mailslot serveur. En fait,
 *             tente de créér le mailslot en écoute. Si la création
 *             échoue, la connexion reste à l'état "déconnecté", sinon,
 *             elle passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComMailServerConnect( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComMailClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un mailslot client. En fait,
 *             tente de créér le client pour émission. Si la création
 *             échoue, la connexion reste à l'état "déconnecté", sinon,
 *             elle passe à l'état "connecté".
 *             ATTENTION : En raison d'un bug sur les clients de mailslots distants
 *             ( le bug ne se déclare pas lors de communications loacales par mailslot )
 *             en overlapped i/o, le client de mailslot est ouvert en mode synchrone
 *             et n'est pas associé à un i/o completion port. Les completion packets
 *             seront postés par PostQueuedCompletionStatus().
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComMailClientConnect( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComMailGetIdentity( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (mailslot = non identifié)
 * REMARQUE  : Suppose que le slot de connexion est associé à un mailslot,
 *             dans un état "connecté" et qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComMailGetIdentity( ACOM_CONNECTION * psCnx );


#endif


/* -------------  FIN DU FICHIER : acom_mail.h ------------- */ 
