/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
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
#include <todbsvc_text.h>
#undef LOC_DEF

#include <memclass.h>


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


PROTECTED void WINAPI TextUnloadDefinitions( HCOLLECTION hCol )
{
    if ( hCol != NULL ) ColDestroy( hCol );
}
 

PROTECTED char * WINAPI TextFind( HCOLLECTION hCol, char * szTextKey, char * szAlternative )
{
    HCOLLECTIONITEM hItem;

    if ( hCol == NULL ) return szAlternative;

    hItem = ColItemFind( hCol, szTextKey );
    if ( hItem == NULL ) return szAlternative;

    return ( ColItemData( hItem ) );
}



// --------------- CODE ----------------------

