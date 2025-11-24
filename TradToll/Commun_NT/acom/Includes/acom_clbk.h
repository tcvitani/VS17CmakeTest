/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_clbk.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des callbacks par défaut pour fonctionnement en WM
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_CLBK_H
#define ACOM_CLBK_H

#include <protect.h>


PROTECTED DWORD AComClbkConnection(
                                ACOM_CONNECTION * psCnx );


PROTECTED DWORD AComClbkReceived(
                                ACOM_CONNECTION * psCnx,
                                DWORD dwDataSize );

PROTECTED DWORD AComClbkSent(
                                ACOM_CONNECTION * psCnx,
                                DWORD dwError );

PROTECTED DWORD AComClbkDisconnection(
                                ACOM_CONNECTION * psCnx );

PROTECTED DWORD AComClbkShutdown(
                                ACOM_WORKING_SET * psWks,
                                DWORD dwError );

#endif


/* -------------  FIN DU FICHIER : acom_clbk.h ------------- */ 
