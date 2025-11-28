/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : FSEARCH
 * FILE       : FSEARCH_LOC.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Local definitions for FSEARCH. The definitions
 *              in this file are visible from any source belonging
 *              to the project FSEARCH.DSP
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    :
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

//
// The macro FSEARCH_LOC_H is defined to avoid multiple file inclusion
//
#ifndef FSEARCH_LOC_H
#define FSEARCH_LOC_H


//
// The header file PROTECT.H will behave differently if the macro
// LOC_DEF is defined or not. In a project, this file should only
// be included once with LOC_DEF defined.
//
#include <protect.h>




#ifdef LOC_DEF
#define     SEARCH_INIT(x)     =x
#else
#define     SEARCH_INIT(x)
#endif



//
// Définition du context de recherche passé aux fonctions
// de comparaison à l'aide d'un slot TLS.
//
typedef struct _SEARCH_CONTEXT
{
    // Nombre d'octets du bloc dans lequel effectuer la recherche
    DWORD               dwBufferBytes;

    // Pointeur sur le début de la rechercher
    BYTE              * pbBufferBytes;
    
    // Nombre d'octets dans le record courant (pour un fichier à
    // taille de record variable) ou dans tous les records (pour
    // un fichier à taille de record fixe).
    DWORD               dwRecordBytes;

    // Position du début de la clé dans un record
    DWORD               dwKeyOffset;

    // Taille de la clé dans un record
    DWORD               dwKeyBytes;

	//Position du curseur sur un bloc de mémoire ou fichier
	DWORD        dwRecordBytesPosition;

    // Pointe sur le bloc de paramétrage de la recherche
    SEARCH_PARAMS     * psParams;
}
    SEARCH_CONTEXT;



//
// Ce flag est utilisé pour indiquer si les données globales des fonctions
// de recherche sont initialisée.
// Les valeurs sont :
//      0x00000000 : Non initialisée
//      0x00000001 : En cours d'initialisation
//      .......... : En cours d'initialisation avec contension
//      0x80000000 : Initialisée
//
PROTECTED DWORD    gdwSearchInit      SEARCH_INIT(0);

//
// Identifiant du slot TLS alloué pour communique le contexte
// aux fonctions de comparaison appelées en callback par la
// fonction de recherche.
//
PROTECTED DWORD    gdwSearchContext   SEARCH_INIT(0xFFFFFFFF);



//
// In reference to FSEARCH_LOC_H
//
#endif
