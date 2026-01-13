/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_files.h
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

#ifndef TODBSVC_FILES_H
#define TODBSVC_FILES_H

#include <protect.h>

typedef char FILE_LIST_ITEM[MAX_PATH];

PROTECTED FILE_LIST_ITEM * FileListOpen( char * szPath, char * szMask, DWORD * pdwCount );
PROTECTED void FileListClose( FILE_LIST_ITEM * psList );
PROTECTED BOOL FileSplitName( char * pcFile, DWORD * pdwFile );
PROTECTED BOOL FileAccessCheck( char * pcFile );

#endif
