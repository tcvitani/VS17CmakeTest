/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : extfilessvc
 * FILE       : extfilessvc_files.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module de gestion des fichier
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef EXTFILESSVC_FILES_H
#define EXTFILESSVC_FILES_H

#include <protect.h>

PROTECTED WIN32_FIND_DATA * FileListOpen( char * szPath, char * szMask, DWORD * pdwCount );
PROTECTED void FileListClose( WIN32_FIND_DATA * psList );

#endif
