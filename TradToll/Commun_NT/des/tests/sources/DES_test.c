/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DES
 * FILE       : DES_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for DES
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
#include <stdio.h>

#include <CSR_DES.h>

#define LOC_DEF
#include <DES_test.h>
#undef LOC_DEF

#include <memclass.h>




char * DumpKey( BYTE * pbBuffer, DWORD dwLen )
{
    DWORD dwIndex;
    DWORD dwCursor = 0;
    static char szString[1000];

    for ( dwIndex = 0; dwIndex < dwLen; dwIndex ++ )
    {
        if ( dwCursor > 990 )
        {
			dwCursor += sprintf_s(szString + dwCursor, (sizeof(szString) - dwCursor), "...");
            break;
        }
		dwCursor += sprintf_s(szString + dwCursor, (sizeof(szString) - dwCursor), "%02X", (DWORD)pbBuffer[dwIndex]);
    }

    return szString;
}


DWORD BuildKey( BYTE ** ppbBuffer, char * szDef )
{
    DWORD dwIndex;
    char cCur;
    BYTE bVal;
    DWORD dwLen;

    dwLen = (DWORD)strlen(szDef);
    *ppbBuffer = HeapAlloc( GetProcessHeap(), 0, dwLen * 2 + 8);
    if ( *ppbBuffer == NULL )
        return 0;

    for ( dwIndex = 0 ; dwIndex < dwLen ; dwIndex ++ )
    {
        cCur = toupper( szDef[dwIndex] );
        if ( cCur == 0 )
            break;
        else if ( ( cCur >= '0' ) && ( cCur <= '9' ) )
            bVal = (BYTE)cCur - (BYTE)'0';
        else if ( ( cCur >= 'A' ) && ( cCur <= 'F' ) )
            bVal = (BYTE)cCur - (BYTE)'A' + (BYTE)10;
        else
        {
            dwIndex = 0;
            break;
        }

        if ( ( dwIndex % 2 ) == 0 )
            (*ppbBuffer)[dwIndex/2] = bVal << 4;
        else
            (*ppbBuffer)[dwIndex/2] += bVal;
    }
    
    dwIndex = dwIndex / 2;
    if ( dwIndex == 0 )
    {
        HeapFree( GetProcessHeap(), 0, *ppbBuffer );
        *ppbBuffer = NULL;
    }

    return dwIndex;
}





int __cdecl main( int iArgc, char ** ppcArgv )
{
    int iIndex;
    BOOL bDES = FALSE;
    BOOL bNotDES = FALSE;
    BOOL b3DES = FALSE;
    BOOL bNot3DES = FALSE;
    BOOL bCheckKey = FALSE;
    BOOL bECB = FALSE;
    BOOL bCBC = FALSE;
    char * pcParm;
    HDESKEY hKey;
    ULONGLONG ullStart;
    ULONGLONG ullStop;
    ULONGLONG ullFreq;
    ULONGLONG ullDelay;
    DWORD dwLen;
    DWORD dwKeyLen = 0;
    DWORD dwDataLen = 0;
    BYTE * pbKey = NULL;
    BYTE * pbData = NULL;
    BYTE * pbData2 = NULL;
    BYTE   tbKey[24];

    if ( iArgc < 2 )
    {
        printf( "ERREUR : Parametre absent, utiliser /HELP pour afficher l'aide\n" );
        return ERROR_INVALID_PARAMETER;
    }

    for ( iIndex = 1 ; iIndex < iArgc ; iIndex ++ )
    {
        pcParm = ppcArgv[iIndex];

        if ( _stricmp( pcParm, "/HELP" ) == 0 )
        {
            printf( "Test de codage DES/3DES en mode ECD et CBC\n" );
            printf( "Parametres :\n" );
            printf( "    /HELP     : Affiche ce text.\n" );
            printf( "    /DES      : Pour effectuer un codage DES.\n" );
            printf( "    /DES-1    : Pour effectuer un decodage DES.\n" );
            printf( "    /3DES     : Pour effectuer un codage 3DES.\n" );
            printf( "    /3DES-1   : Pour effectuer un decodage 3DES.\n" );
            printf( "    /ECB      : Effectuer un codage ECB.\n" );
            printf( "    /CBC      : Effectuer un codage CBC.\n" );
            printf( "    /KEY=Hex  : Definition de la clef DES 64 bits.\n" );
            printf( "                Attention, en fonction des parametres /CHECKKEY et /FORCEODD,\n" );
            printf( "                la clef peut etre refusee. La valeur de la clef est definie par\n" );
            printf( "                une serie de 16 digits hexadecimaux.\n" );
            printf( "    /DATA=Hex : Definition du bloc de donnees de 64 bits. Il est defini par une\n" );
            printf( "                serie de 16 digits hexadecimaux.\n" );
            printf( "    /CHECKKEY : Effectue une verification de la validite de la clef.\n" );
            printf( "                Permet de verifier que la cle fournie a la bonne parite et\n" );
            printf( "                qu'elle ne fait pas partie des cles a problemes.\n" );
            printf( "Exemple :\n" );
            printf( "DESTEST /DES /ECB /KEY=0162A30FD45273FC /DATA=CC67F50836EA653B\n" );
            return NO_ERROR;
        }

        if ( _stricmp( pcParm, "/DES" ) == 0 )
        {
            bDES = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/DES-1" ) == 0 )
        {
            bNotDES = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/3DES" ) == 0 )
        {
            b3DES = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/3DES-1" ) == 0 )
        {
            bNot3DES = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/CHECKKEY" ) == 0 )
        {
            bCheckKey = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/ECB" ) == 0 )
        {
            bECB = TRUE;
            continue;
        }

        else if ( _stricmp( pcParm, "/CBC" ) == 0 )
        {
            bCBC = TRUE;
            continue;
        }

        else if ( _strnicmp( pcParm, "/KEY=", 5 ) == 0 )
        {
            dwKeyLen = BuildKey( &pbKey, pcParm + 5 );
            continue;
        }

        else if ( _strnicmp( pcParm, "/DATA=", 6 ) == 0 )
        {
            dwDataLen = BuildKey( &pbData, pcParm + 6 );
            continue;
        }

        else
        {
            printf( "ERREUR : Parametre inconnu \"%s\"\n", pcParm );
            return ERROR_INVALID_PARAMETER;
        }
    }

    if ( ( ( bDES ? 1 : 0 ) + ( bNotDES ? 1 : 0 ) + ( b3DES ? 1 : 0 ) + ( bNot3DES ? 1 : 0 ) ) != 1 )
    {
        printf( "ERREUR : Une unique option doit etre choisie parmi /DES, /DES-1, /3DES, et /3DES-1\n" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( ( ( bCBC ? 1 : 0 ) + ( bECB ? 1 : 0 ) ) != 1 )
    {
        printf( "ERREUR : Une unique option doit etre choisie parmi /CBC, /ECB\n" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( b3DES || bNot3DES )
    {
        if ( dwKeyLen == 16 )
        {
            memcpy( tbKey, pbKey, 16 );
            memcpy( tbKey+16, pbKey, 8 );
            dwKeyLen = 24;
        }
        else if ( dwKeyLen == 24 )
        {
            memcpy( tbKey, pbKey, 24 );
            dwKeyLen = 24;
        }
        else
            dwKeyLen = 0;
    }
    else if ( bDES || bNotDES )
    {
        if ( dwKeyLen == 8 )
        {
            memcpy( tbKey, pbKey, 8 );
            dwKeyLen = 8;
        }
        else
            dwKeyLen = 0;
    }
    if ( dwKeyLen == 0 )
    {
        printf( "ERREUR : Definition de la clef non valide ou absente\n" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( dwDataLen == 0 )
    {
        printf( "ERREUR : Definition des donnees non valide ou absente\n" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( b3DES || bNot3DES )
        hKey = DESOpenTripleKey( (unsigned char (*)[8])tbKey, bCheckKey );
    else
        hKey = DESOpenKey( tbKey, bCheckKey );

    if ( hKey == NULL )
    {
        printf( "ERREUR : Clef non valide\n" );
        return ERROR_INVALID_DATA;
    }

    dwLen = dwDataLen + 8;
    pbData2 = HeapAlloc(GetProcessHeap(), 0, dwLen );
    if ( pbData2 != NULL )
    {
        QueryPerformanceCounter( (LARGE_INTEGER*)&ullStart );

        if ( bECB )
        {
            if ( ! DESEncryptEcb( b3DES || bNot3DES, b3DES || bDES, pbData, dwDataLen, pbData2, &dwLen, hKey ) )
            {
                printf( "ERREUR : Impossible d'encrypter\n" );
                return ERROR_INVALID_DATA;
            }
        }
        else
        {
            if ( ! DESEncryptCbc( b3DES || bNot3DES, b3DES || bDES, pbData, dwDataLen, pbData2, &dwLen, hKey ) )
            {
                printf( "ERREUR : Impossible d'encrypter\n" );
                return ERROR_INVALID_DATA;
            }
        }
        QueryPerformanceCounter( (LARGE_INTEGER*)&ullStop );
        QueryPerformanceFrequency( (LARGE_INTEGER*)&ullFreq );

        ullDelay = ( ( ullStop - ullStart ) * 1000000 ) / ullFreq;

        printf( "Clef utilisee  : %s\n", DumpKey( tbKey, dwKeyLen ) );
        printf( "Bloc source    : %s\n", DumpKey( pbData, dwDataLen ) );
        printf( "Bloc resultat  : %s\n", DumpKey( pbData2, dwLen ) );
        printf( "Durée          : %I64u us\n", ullDelay );
    }
    else
    {
        printf( "ERREUR : Erreur d'allocation mémoire\n" );
        return ERROR_INVALID_DATA;
    }

    if ( pbKey != NULL )
        HeapFree( GetProcessHeap(), 0, pbKey );
    if ( pbData != NULL )
        HeapFree( GetProcessHeap(), 0, pbData );
    if ( pbData2 != NULL )
        HeapFree( GetProcessHeap(), 0, pbData2 );

    return NO_ERROR;

}


