/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_tcp.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des connexions pour les liaisons socket TCP
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_TCP_H
#define ACOM_TCP_H

#include <protect.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpInit ( void )
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie l'API socket
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpInit ( void );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpServerPrepare ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie la mise en écoute d'un serveur tcp.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpServerPrepare( ACOM_INSTANCE * psInst );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComTcpServerFinish ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Termine l'écoute d'un serveur tcp.
 * --------------------------------------------------------------------
 */
PROTECTED void AComTcpServerFinish( ACOM_INSTANCE * psInst );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un serveur tcp. En fait,
 *             tente de créér le serveur et de le mettre en attente
 *             de connexion. Si la création échoue, la connexion reste à l'état
 *             "déconnecté", sinon, elle passe à l'état "en cours de connexion".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpServerConnect( ACOM_CONNECTION * psCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un client tcp. En fait,
 *             tente de créér le client. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpClientConnect( ACOM_CONNECTION * psCnx );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComTcpGetIdentity( ACOM_CONNECTION * psCnx, 
 *                                                struct sockaddr_in * psAddr )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 *             struct sockaddr_in * psAddr : Pointeur sur la structure d'adresse du connecté ou NULL
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (tcp = identifié pour serveur uniquement)
 * REMARQUE  : Suppose que le slot de connexion est bien associé à
 *             une connexion TCP (client ou serveur), dans un état "connecté" et
 *             qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComTcpGetIdentity( ACOM_CONNECTION * psCnx, 
                                   struct sockaddr_in * psAddr );



#endif


/* -------------  FIN DU FICHIER : acom_pipe.h ------------- */ 
