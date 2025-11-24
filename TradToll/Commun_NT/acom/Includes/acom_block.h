/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_block.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des blocs de commande
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_BLOCK_H
#define ACOM_BLOCK_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_BLOCK * AComBlockOpenWithHeader ( 
 *                      DWORD dwBlockType,
 *                      ACOM_CONNECTION * psCnx,
 *                      ACOM_CNX_HANDLE hCnxHandle, 
 *                      DWORD64 dwCnxUsrKey, 
 *                      DWORD64 dwMsgUsrKey, 
 *                      DWORD bDisable, 
 *                      DWORD dwHeaderSize,
 *                      void  * pvHeader,
 *                      DWORD dwDataSize, 
 *                      void  * pvData )
 * PARAMETERS: DWORD dwBlockType           : Type du bloc de commande (ACOM_COMMAND_XXXX) 
 *             ACOM_CONNECTION * psCnx     : Référence à la connexion concernée
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion
 *             DWORD64 dwCnxUsrKey           : Clé utilisateur de connexion
 *             DWORD64 dwMsgUsrKey           : Clé utilisateur de message
 *             DWORD bDisable              : Cas des déconnexion : TRUE la connexion tentera de se
 *                                           reconnecter. FALSE : la connexion est désactivée.
 *             DWORD dwHeaderSize          : Taille des données pointée par pvHeader
 *             void  * pvHeader            : Données de l'entête. Si pvHeader == NULL et dwHeaderSize != 0,
 *                                           les données du bloc seront allouée selon dwHeaderSize et
 *                                           initialisées à 0.
 *             DWORD dwDataSize            : Taille des données pointée par pvData
 *             void  * pvData              : Données. Si pvData == NULL et dwDataSize != 0, les
 *                                           données du bloc seront allouée selon dwDataSize et
 *                                           initialisées à 0.
 * RETURN    : NULL si erreur, un pointeur de bloc sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue l'allocation et l'initialisation d'un bloc de commande
 *             avec les données contenant un entête
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_BLOCK * AComBlockOpenWithHeader( 
    DWORD dwBlockType,
    ACOM_CONNECTION * psCnx,
    ACOM_CNX_HANDLE hCnxHandle,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
    DWORD bDisable,
    DWORD dwHeaderSize,
    void  * pvHeader,
    DWORD dwDataSize,
    void  * pvData );

    
/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComBlockClose ( ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_BLOCK * psBlock : Bloc à désallouer
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la désallocation d'un bloc alloué avec AComBlockOpen().
 * --------------------------------------------------------------------
 */
PROTECTED void AComBlockClose( ACOM_BLOCK * psBlock );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void * AComBlockDataPtr ( ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_BLOCK * psBlock : Bloc concerné
 * RETURN    : Un pointeur sur la zone de données du bloc.
 *             Attention, si le bloc ne contient pas de données, le pointeur
 *             retourné pointe juste après le bloc.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère un pointeur sur la zone de données d'un bloc de commande
 * --------------------------------------------------------------------
 */
PROTECTED void * AComBlockDataPtr( ACOM_BLOCK * psBlock );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComBlockPost ( 
 *                      HANDLE hCompletion, 
 *                      DWORD dwBlockType, 
 *                      ACOM_CONNECTION * psCnx, 
 *                      ACOM_CNX_HANDLE hCnxHandle, 
 *                      DWORD64 dwCnxUsrKey, 
 *                      DWORD64 dwMsgUsrKey, 
 *                      BOOL  bDisable, 
 *                      DWORD dwHeaderSize,
 *                      void  * pvHeader,
 *                      DWORD dwDataSize, 
 *                      void  * pvData )
 * PARAMETERS: HANDLE hCompletion          : I/O completion port où le bloc de commande doit
 *             DWORD dwBlockType           : Type du bloc de commande (ACOM_COMMAND_XXXX) 
 *             ACOM_CONNECTION * psCnx     : Référence à la connexion concernée
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion
 *             DWORD64 dwCnxUsrKey           : Clé utilisateur de connexion
 *             DWORD64 dwMsgUsrKey           : Clé utilisateur de message
 *             DWORD bDisable              : Cas des déconnexion : TRUE la connexion tentera de se
 *                                           reconnecter. FALSE : la connexion est désactivée.
 *             DWORD dwHeaderSize          : Taille des données pointée par pvHeader
 *             void  * pvHeader            : Données de l'entête. Si pvHeader == NULL et dwHeaderSize != 0,
 *                                           les données du bloc seront allouée selon dwHeaderSize et
 *                                           initialisées à 0.
 *             DWORD dwDataSize            : Taille des données pointée par pvData
 *             void  * pvData              : Données. Si pvData == NULL et dwDataSize != 0, les
 *                                           données du bloc seront allouée selon dwDataSize et
 *                                           initialisées à 0.
 * RETURN    : NO_ERROR si posté, sinon, un code d'erreur win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Alloue, rempli et post un bloc de commande dans un i/o completion port.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComBlockPost( 
    HANDLE hCompletion,
    DWORD dwBlockType,
    ACOM_CONNECTION * psCnx,
    ACOM_CNX_HANDLE hCnxHandle,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
    BOOL  bDisable,
    DWORD dwHeaderSize,
    void  * pvHeader,
    DWORD dwDataSize,
    void  * pvData );




#endif


/* -------------  FIN DU FICHIER : acom_block.h ------------- */ 
