/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_serial.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des com sur port serie
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_SERIAL_H
#define ACOM_SERIAL_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComSerialConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un port série. En fait,
 *             tente de créér le handle. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComSerialConnect( ACOM_CONNECTION * psCnx );


#endif


/* -------------  FIN DU FICHIER : acom_pipe.h ------------- */ 
