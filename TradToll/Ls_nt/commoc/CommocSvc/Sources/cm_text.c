/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : CM_TEXT.C
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

#include <windows.h>
#include <col.h>
#include <reg.h>

#define LOC_DEF
#include <cm_text.h>
#undef LOC_DEF

#include <memclass.h>




//
// CODE DES FONCTION PROTEGEES
//

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
PROTECTED DWORD WINAPI TextLoadDefinitions( char * szKey, HCOLLECTION * phCol )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwValueType;
    HCOLLECTION hCol = NULL;
    HCOLLECTIONITEM hItem;
    char szName[256];
    char szValue[1024];

    __try
    {
        hCol = ColCreate( 
            COL_INDEX_ZSTRING,
            256,
            TRUE,
            TRUE );
        if ( hCol == NULL )             { dwErr = ERROR_NOT_ENOUGH_MEMORY;  __leave; }

        for ( dwIndex = 0; dwErr == NO_ERROR; dwIndex ++ )
        {
            dwValueSize = sizeof( szValue );
            dwNameSize = sizeof( szName );
            dwErr = REG_Enum_Valeurs(
                HKEY_LOCAL_MACHINE,
                szKey,
                dwIndex,
                szName,
                &dwNameSize,
                &dwValueType,
                szValue,
                &dwValueSize );
            if ( ( dwErr == NO_ERROR ) && ( dwValueType == REG_SZ ) )
            {
                hItem = ColItemAdd( hCol, szName, szValue, (DWORD)strlen( szValue ) + 1 );
                if ( hItem == NULL )    { dwErr = ERROR_NOT_ENOUGH_MEMORY;  __leave; }
            }
        }
        if ( dwErr != ERROR_NO_MORE_ITEMS )                                 __leave;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( hCol != NULL ) ColDestroy( hCol );
            hCol = NULL;
        }

        (*phCol) = hCol; 
    }

	return dwErr;
}




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
PROTECTED void WINAPI TextUnloadDefinitions( HCOLLECTION hCol )
{
    if ( hCol != NULL ) ColDestroy( hCol );
}
 



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
PROTECTED char * WINAPI TextFind( HCOLLECTION hCol, char * szTextKey, char * szAlternative )
{
    HCOLLECTIONITEM hItem;

    if ( hCol == NULL ) return szAlternative;

    hItem = ColItemFind( hCol, szTextKey );
    if ( hItem == NULL ) return szAlternative;

    return ( ColItemData( hItem ) );
}



