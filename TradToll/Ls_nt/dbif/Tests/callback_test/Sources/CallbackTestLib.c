#include <windows.h>
#include <stdio.h>

#include <dbif.h>


void Trace( char * szString )
{
	FILE * pfTrace = NULL;

	fopen_s(&pfTrace, "c:\\dbif.log", "a+");
    if ( pfTrace )
    {
        fprintf( pfTrace, "%s", szString );
        fclose( pfTrace );
    }
}


__declspec( dllexport ) LONG __cdecl RetourneIntFromVoid( void )
{
    return 12345;
}


__declspec( dllexport ) LONG __cdecl RetourneIntFromInt( long param )
{
    return param;
}


__declspec( dllexport ) LONG __cdecl RetourneIntFromString( const char * szString, long taille )
{
	return (LONG)strlen(szString);
}



__declspec( dllexport ) LONG __cdecl RetourneIntFromContextInt( void * pvContext, LONG a )
{
    DB_CNX * psCnx;
    DB_VAR * psInParam;
    DB_VAR * psOutParam;
    DB_STMT * psStmt;
    LONG b;
    DWORD dwClass;
    DWORD dwCode;
    char szText[1000];

    psCnx = DBConnectCallback( pvContext );
    if ( psCnx == NULL )
        return 0xFFFFFFFF;

    psStmt = DBOpenStatement( psCnx, "BEGIN :outparam := :inparam; END;" );
    if ( psStmt == NULL )
    {
        DBGetLastError( psCnx, &dwClass, &dwCode, szText, sizeof(szText ) );
        DBDisconnect( psCnx );
        Trace( szText );
        Trace( "\n" ); 
        return 100000 + dwCode;
    }

    psInParam = DBBindPlaceHolderVariable( psStmt, ":inparam", DB_TYPE_INT, sizeof(int), 0 );
    psOutParam = DBBindPlaceHolderVariable( psStmt, ":outparam", DB_TYPE_INT, sizeof(int), 0 );
    if ( ( psInParam == NULL ) || ( psOutParam == NULL ) )
    {
        DBGetLastError( psCnx, &dwClass, &dwCode, szText, sizeof(szText ) );
        DBDisconnect( psCnx );
        Trace( szText );
        Trace( "\n" ); 
        return 200000 + dwCode;
    }

    if ( DBSetVariableItemValue( psInParam, 0, &a, 0 ) == NULL )
    {
        DBGetLastError( psCnx, &dwClass, &dwCode, szText, sizeof(szText ) );
        DBDisconnect( psCnx );
        Trace( szText );
        Trace( "\n" ); 
        return 300000 + dwCode;
    }


    if ( ! DBExecuteStatement( psStmt ) )
    {
        DBGetLastError( psCnx, &dwClass, &dwCode, szText, sizeof(szText ) );
        DBDisconnect( psCnx );
        Trace( szText );
        Trace( "\n" ); 
        return 400000 + dwCode;
    }

    DBGetVariableItemValue( psOutParam, 0, &b, NULL );

    DBDisconnect( psCnx );

    return b;
}

/*

__declspec( dllexport ) LONG CallbackTest( void * pvContext, LONG a, LONG * b )
{
    DB_CNX * psCnx;
    DB_VAR * psInParam;
    DB_VAR * psOutParam;
    DB_STMT * psStmt;

    psCnx = DBConnectCallback( pvContext );
    if ( psCnx == NULL )
        return FALSE;

    psStmt = DBOpenStatement( psCnx, "BEGIN DBADMIN.TEST_CALL_DLL.OCITEST2(:inparam,:outparam); END;" );
    if ( psStmt == NULL )
    {
        DBDisconnect( psCnx );
        return FALSE;
    }

    psInParam = DBBindPlaceHolderVariable( psStmt, ":inparam", DB_TYPE_INT, sizeof(int), 0 );
    psOutParam = DBBindPlaceHolderVariable( psStmt, ":outparam", DB_TYPE_INT, sizeof(int), 0 );
    if ( ( psInParam == NULL ) || ( psOutParam == NULL ) )
    {
        DBDisconnect( psCnx );
        return FALSE;
    }

    DBSetVariableItemValue( psInParam, 0, &a, 0 );
    if ( ! DBExecuteStatement( psStmt ) )
    {
        DBDisconnect( psCnx );
        return FALSE;
    }

    DBGetVariableItemValue( psOutParam, 0, b, NULL );

    DBDisconnect( psCnx );

    return TRUE;
}

*/


/*---------------------------- FIN DU FICHIER -------------------------*/


