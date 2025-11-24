/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_dbg.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions de trace et de debug
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_DBG_H
#define ACOM_DBG_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgInit()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise la section critique d'accès aux handle de trace
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgInit();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComDbgOpen( BOOL bInfo, BOOL bError, char * pcFileName )
 * PARAMETERS: BOOL bInfo        : Etat d'activation des traces informatives élémentaires
 *             BOOL bError       : Etat d'activation des traces d'erreur
 *             char * pcFileName : Nom du fichier de trace
 * RETURN    : NO_ERROR si ok
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Active la trace. Si elle est déjà active, retourne une erreur
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComDbgOpen( BOOL bInfo, BOOL bError, char * pcFileName );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgClose()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Désactive la trace si elle est active
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgClose();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgError ( DWORD dwErr, char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
 * PARAMETERS: DWORD dwErr       : Code de l'erreur win32, 0 si autre que win32 ou inconnu
 *             char * pcFilePath : fichier source
 *             DWORD dwLine      : ligne de code
 *             char * pcFormat   : Texte de la trace
 *             ...               : 
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace d'erreur
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgError( DWORD dwErr, char * pcFilePath, DWORD dwLine, char * pcFormat, ... );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgInfo ( char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
 * PARAMETERS: char * pcFilePath : fichier source
 *             DWORD dwLine      : ligne de code
 *             char * pcFormat   : Texte de la trace
 *             ...               : 
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace informative élémentaire
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgInfo( char * pcFilePath, DWORD dwLine, char * pcFormat, ... );

PROTECTED void WINAPI AComDbgTraceHexBuffer(char * szText, DWORD dwLen, BYTE * pBuffer);

PROTECTED const char * GetNameForConnectionState(DWORD dwConnectionState);
PROTECTED const char * GetNameForBlockType(DWORD dwBlockType);


#endif


/* -------------  FIN DU FICHIER : acom_dbg.h ------------- */ 
