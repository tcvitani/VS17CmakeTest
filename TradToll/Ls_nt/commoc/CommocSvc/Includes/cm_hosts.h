/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_hosts.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des hosts par l'intermédiaire des librairie
 *              d'extension, et définition des comportements par
 *              défaut (equivalent d'une libraire d'extension pour les
 *              fichiers locaux).
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_HOSTS_H
#define CM_HOSTS_H

#include <protect.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED  BOOL CMHostInit( CM_HOST * psHost, char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion de l'host à initialiser
 *             szName     : Pointe sur le nom attribué à l'host
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres de l'host
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure de gestion d'un host
 * --------------------------------------------------------------------
 */
PROTECTED  BOOL CMHostInit( CM_HOST * psHost, char * szName, HKEY hKeyConfig );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostTerminate( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostTerminate( CM_HOST * psHost );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostLock( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostLock( CM_HOST * psHost );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostLock( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostUnlock( CM_HOST * psHost );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxOpen( CM_HOST * psHost, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 *             hKeyConfig : Handle de la clé de registre où trouver les paramètres de l'host
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur. GetLastError() permet
 *             de préciser l'origine de l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostOpen dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxOpen( CM_HOST * psHost, HKEY hKeyConfig );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxStatus( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'host est connecté, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de la non connexion.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostQueryStatus dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxStatus( CM_HOST * psHost );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostCnxClose( CM_HOST * psHost )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost     : Pointe sur la structure de gestion d'un host initialisé
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostClose dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostCnxClose( CM_HOST * psHost );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxPut( CM_HOST * psHost, char * pcLocalPath , char * pcRemotePath )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcLocalPath : Chemin local du fichier source
 *             pcRemotePath: Chemin distant du fichier destination
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostPutFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxPut( CM_HOST * psHost, char * pcLocalPath , char * pcRemotePath );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxGet( CM_HOST * psHost, char * pcRemotePath, char * pcLocalPath )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemotePath: Chemin distant du fichier source
 *             pcLocalPath : Chemin local du fichier destination
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostGetFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxGet( CM_HOST * psHost, char * pcRemotePath, char * pcLocalPath );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxMove( CM_HOST * psHost, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemoteSrc : Chemin distant du fichier source
 *             pcRemoteDst : Chemin distant du fichier destination (si NULL, il s'agit d'un effacement)
 *             bCopy       : Si TRUE, effectue une copie.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostMoveFile dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxMove( CM_HOST * psHost, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMHostCnxEnum( CM_HOST * psHost, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             pcRemoteMask: Chemin distant du masque à utiliser pour l'énumération.
 *             ppsFound    : En cas de succés, retourne un pointeur sur un tableau de
 *                           structures de type WIN32_FIND_DATA contenant les informations
 *                           sur les fichiers énumérés.
 *             pdwCount    : En cas de succés, retourne le nombre de fichiers énumérés.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si l'opération a réussie, FALSE sinon. GetLastError() permet
 *             de préciser l'origine de l'échec.
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostEnumFiles dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMHostCnxEnum( CM_HOST * psHost, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMHostCnxFree( CM_HOST * psHost, WIN32_FIND_DATA * psFind )
 * --------------------------------------------------------------------
 * PARAMETERS: psHost      : Pointe sur la structure de gestion d'un host initialisé
 *             psFind : Pointe sur un tableau d'énumération de fichiers
 *                      retourné par HostEnumFiles.
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Encapsulation de l'appel à HostFreeEnum dans la DLL
 * --------------------------------------------------------------------
 */
PROTECTED void CMHostCnxFree( CM_HOST * psHost, WIN32_FIND_DATA * psFind );




#endif