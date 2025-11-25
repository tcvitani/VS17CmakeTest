#include <windows.h>
#include <stdio.h>

#include <crypt.h>

int __cdecl main( int iArgc, char * * ppcArgv )
{
    char szString[1000];

    if ( iArgc != 2 )
    {
        printf( "ERREUR : Un paramètre attendu : CRYPTE ou DECRYPTE\n" );
    }

    else if ( _stricmp( ppcArgv[1], "CRYPTE" ) == 0 )
    {
        for ( ; TRUE ; )
        {
            printf( "ENTRER LA CHAINE A CRYPTER : " );
			if (gets_s(szString, sizeof(szString)) != NULL)
            {
                fnEncrypt( szString );
                printf( "RESULTAT DU CRYPTAGE : [%s]\n", szString );
            }
            else
                break;
        }
    }
    else if ( _stricmp( ppcArgv[1], "DECRYPTE" ) == 0 )
    {
        for ( ; TRUE ; )
        {
            printf( "ENTRER LA CHAINE A DECRYPTER : " );
			if (gets_s(szString, sizeof(szString)) != NULL)
            {
                fnDecrypt( szString );
                printf( "RESULTAT DU CRYPTAGE : [%s]\n", szString );
            }
            else
                break;
        }
    }
    else
    {
        printf( "ERREUR : Un paramètre attendu : CRYPTE ou DECRYPTE\n" );
    }
    return 0;
}


