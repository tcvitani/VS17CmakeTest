/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : CM_TEXT.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des chaines de caractère localisables
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef FIND_TEXT_H
#define FIND_TEXT_H

#include <protect.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI TextLoadDefinitions( char * szKey, HCOLLECTION * phCol )
 * --------------------------------------------------------------------
 * PARAMETERS: szKey : Chemin de la clé du registre dans laquelle sont les définitions de texte
 *             phCol : Retourne la collection des chaines de caractère indexée par les clés texte.
 * --------------------------------------------------------------------
 * RETURN    : NO_ERROR en cas de succés. Un code win32 sinon.
 * --------------------------------------------------------------------
 * ROLE      : Charge en mémoire une collection de chaines de caractère localisables.
 *             Le handle obtenu par phCol doit être libéré à l'aide de TextUnloadDefinitions
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI TextLoadDefinitions( char * szKey, HCOLLECTION * phCol );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void WINAPI TextUnloadDefinitions( HCOLLECTION hCol )
 * --------------------------------------------------------------------
 * PARAMETERS: hCol : Collection des chaines de caractère retournée par TextLoadDefinitions
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère le handle obtenu à l'aide de TextLoadDefinitions
 * --------------------------------------------------------------------
 */
PROTECTED void WINAPI TextUnloadDefinitions( HCOLLECTION hCol );
 



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED char * WINAPI TextFind( HCOLLECTION hCol, char * szTextKey, char * szAlternative )
 * --------------------------------------------------------------------
 * PARAMETERS: hCol          : Collection des chaines de caractère retournée par TextLoadDefinitions
 *             szTextKey     : Clé identifiant la chaine localisable.
 *             szAlternative : Pointe sur le texte à retourner si la clé n'existe pas.
 * --------------------------------------------------------------------
 * RETURN    : Un pointeur sur la chaine localisée si la clé existe, szAlternative si la clé
 *             n'a pas été trouvée.
 * --------------------------------------------------------------------
 * ROLE      : Recherche la valeur d'une chaine localisable.
 * --------------------------------------------------------------------
 */
PROTECTED char * WINAPI TextFind( HCOLLECTION hCol, char * szTextKey, char * szAlternative );




#endif
