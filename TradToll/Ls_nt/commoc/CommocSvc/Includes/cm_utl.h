/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_utl.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions d'intéret général
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_UTL_H
#define CM_UTL_H

#include <protect.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * CMMakePath( char * szDir, char * szFile, char * szPath, DWORD dwSize )
 * --------------------------------------------------------------------
 * PARAMETERS: szDir  : Chemin complet d'un répertoire (forme "X:\\PATH\\SUBPATH" ou "\\\\COMPUTER\\SHARE\\PATH")
 *             szFile : Nom de fichier
 *             szPath : Récupère le chemin complet d'accés au fichier
 *             dwSize : Taille du buffer pointé par szPath
 * --------------------------------------------------------------------
 * RETURN    : szPath
 * --------------------------------------------------------------------
 * ROLE      : Construit un chemin d'accés à un fichier.
 * --------------------------------------------------------------------
 */
PROTECTED char * CMMakePath( char * szDir, char * szFile, char * szPath, DWORD dwSize );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * CMTrim( char * pcStr )
 * --------------------------------------------------------------------
 * PARAMETERS: pcStr : Pointe sur la chaine à épurer
 * --------------------------------------------------------------------
 * RETURN    : pcStr
 * --------------------------------------------------------------------
 * ROLE      : Effectue une épuration de chaine en éliminant les
 *             espaces, tabulations et autres caractères de contrôle
 *             placés en début et fin de chaine.
 * --------------------------------------------------------------------
 */
PROTECTED char * CMTrim( char * pcStr );




#endif