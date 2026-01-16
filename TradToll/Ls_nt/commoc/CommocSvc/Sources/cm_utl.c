/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_utl.c
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

#include <windows.h>
#include <stdio.h>
#include <ntsvc.h>
#include <col.h>
#include <acom.h>

#include <cmhost.h>
#include <cmwork.h>
#include <cm_glob.h>

#define LOC_DEF
#include <cm_utl.h>
#undef LOC_DEF

#include <memclass.h>




//
// CODE DES FONCTIONS PROTEGEES
//

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
PROTECTED char * CMMakePath( char * szDir, char * szFile, char * szPath, DWORD dwSize )
{
    _snprintf( szPath, dwSize, "%s\\%s", szDir, szFile );
    szPath[dwSize-1] = 0;

    return szPath;
}
    



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
PROTECTED char * CMTrim( char * pcStr )
{
    unsigned char * pcSrc = pcStr;
    unsigned char * pcDst = pcStr;
    unsigned char * pcStop = NULL;

    while ( ( *pcSrc != '\0' ) && ( *pcSrc <= ( unsigned char)' ' ) ) pcSrc ++;
    while ( *pcSrc != '\0' )
    {
        *pcDst = *pcSrc;
        if ( ( pcStop == NULL ) && ( *pcDst <=(unsigned char)' ' ) )
            pcStop = pcDst;
        else if ( *pcDst > (unsigned char)' ' )
            pcStop = NULL;
        pcDst ++;
        pcSrc ++;
    }
    if ( pcStop != NULL ) 
        *pcStop = '\0';
    else
        *pcDst = '\0';

    return pcStr;
}


