/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     cmdl.C                                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project						 */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>

#include <cmdl.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


#define CMDL_IS_SEPARATOR(c)  (((c)==' ')||((c)=='\t'))
#define CMDL_IS_TERMINATOR(c) ((c)=='\0')
#define CMDL_IS_STRING(c)     (((c)=='\"')||((c)=='\''))

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE char * WINAPI CMDLGetParam(IN char *pcCommand, 
								   IN char *pcParam, 
								   IN OUT DWORD *pdwParamSize );

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT char ** WINAPI CMDLOpenArgv( IN char  * pcCommand, OUT DWORD * pdwArgc )
 * PARAMETERS: pcCommand : Chaine à analyser
 *             pdwArgc : Récupère le nombre d'arguments contenus dans pcCommand
 * RETURN    : Un pointeur sur un tableau de pointeurs de caractère donnant la liste des arguments
 *             composant la chaine pcCommand. Le nombre d'éléments dans le tableau est donné par
 *             pdwArgc. Ce tableau s'utilise de la même manière que le paramètre argv d'une
 *             fonction main().
 *             En cas d'erreur (pas assez de mémoire), la fonction renvoie NULL.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Décomposition d'une chaine de caractère en liste d'arguments.
 * --------------------------------------------------------------------
 */
EXPORT char ** WINAPI CMDLOpenArgv(
        IN      char  * pcCommand,
        OUT     DWORD * pdwArgc )
{
    char szParam[CMDL_MAX_PARAM_SIZE];
    char ** ppcArgv;
    DWORD dwCount = 0;
    DWORD dwSize;

    ppcArgv = HeapAlloc( GetProcessHeap(), 0, sizeof( *ppcArgv ) );

    while ( pcCommand != NULL )
    {
        dwSize = sizeof( szParam );
        pcCommand = CMDLGetParam( pcCommand, szParam, &dwSize );
        if ( pcCommand != NULL )
        {
            ppcArgv[dwCount] = HeapAlloc( GetProcessHeap(), 0, dwSize + 1 );
            strcpy_s( ppcArgv[dwCount], CMDL_MAX_PARAM_SIZE, szParam );
            dwCount ++;
            ppcArgv = HeapReAlloc( GetProcessHeap(), 0, ppcArgv, ( dwCount + 1 ) * sizeof( *ppcArgv ) );
        }
    }
    ppcArgv[dwCount] = NULL;

    *pdwArgc = dwCount;
    return ppcArgv;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CMDLCloseArgv( IN char ** ppcArgv )
 * PARAMETERS: ppcArgv : Pointeur retourné par CMDLOpenArgv()
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme un tableau d'argument ouvert avec CMDLOpenArgv
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CMDLCloseArgv(
        IN      char ** ppcArgv )
{
    DWORD dwCount = 0;

    while ( ppcArgv[dwCount] != NULL )
        HeapFree( GetProcessHeap(), 0, ppcArgv[dwCount++] );
    HeapFree( GetProcessHeap(), 0, ppcArgv );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsList( 
 *                   IN      char * pcName, 
 *                   IN      char * pcParamVal, 
 *                   IN      char cSeparator, 
 *                   IN      DWORD dwMaxItemSize, 
 *                   IN OUT  DWORD * pdwItems, 
 *                   OUT     ... )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 *             cSeparator : Caractère utilisé comme séparateur
 *             dwMaxItemSize : Taille du buffer de chaque élément de la liste.
 *             pdwItems : En entrée, nombre maximum d'éléments dans la liste, en sortie, nombre
 *                        d'éléments utilisés dans la liste
 *             ... : Pointeurs sur les buffers de taille dwMaxItemSize destinés à recevoir les
 *                   valeurs de chaque élément de la liste.
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type paramètre liste. Si c'est le cas, la fonction 
 *             extrait les différents éléments de la liste.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsList( 
        IN      char * pcName, 
        IN      char * pcParamVal, 
        IN      char cSeparator, 
        IN      DWORD dwMaxItemSize, 
        IN OUT  DWORD * pdwItems, 
        OUT     ... )
{
    DWORD dwCount;
    DWORD dwCursor;
    DWORD dwLen = (DWORD)(strlen(pcName));
    va_list sList;
    char * pcScan;
    char * pcVal;
    char szItem[CMDL_MAX_PARAM_SIZE];

    // Vérifier que le nom du paramètre est le bon
    if ( _strnicmp( pcName, pcParamVal, dwLen ) != 0 )
        return FALSE;

    // On pointe directement au niveau du premier élément de la liste
    va_start( sList, pdwItems );
    pcVal = va_arg( sList, char * );
    pcScan = pcParamVal + dwLen;

    // Scanner le paramètre pour trouver les items de la liste
    dwCount = *pdwItems;
    dwCursor = 0;
    while ( dwCount > 0  )
    {
        if ( ( *pcScan == cSeparator ) || CMDL_IS_TERMINATOR( *pcScan ) )
        {
            szItem[dwCursor] = '\0';
            strcpy_s( pcVal, CMDL_MAX_PARAM_SIZE, szItem );
            dwCursor = 0;
            dwCount --;
            if ( dwCount > 0 )
                pcVal = va_arg( sList, char * );
            if ( CMDL_IS_TERMINATOR( *pcScan ) )
                break;
        }

        else
            if ( dwCursor < dwMaxItemSize )
                szItem[dwCursor++] = *pcScan;
        pcScan ++;
    }
    *pdwItems = *pdwItems - dwCount;

    return TRUE;
}


           
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsString( 
 *                   IN      char * pcName, 
 *                   IN      char * pcParamVal, 
 *                   IN      DWORD dwMaxValueSize,
 *                   OUT     char * pcValue )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 *             dwMaxValueSize : Taille du buffer destiné à recevoir la chaine.
 *             pcValue : Buffer destiné à recevoir la valeur de la chaine.
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type chaine. Si c'est le cas, la fonction 
 *             extrait la valeur de la chaine.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsString( 
        IN      char * pcName, 
        IN      char * pcParamVal, 
        IN      DWORD dwMaxValueSize,
        OUT     char * pcValue )
{
    DWORD dwLen = (DWORD)(strlen(pcName));

    if ( _strnicmp( pcName, pcParamVal, dwLen ) != 0 )
        return FALSE;
    strncpy_s( pcValue, dwMaxValueSize, pcParamVal + dwLen, dwMaxValueSize );
    pcValue[dwMaxValueSize-1] = '\0';

    return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsNumber( 
 *                   IN      char * pcName, 
 *                   IN      char * pcParamVal, 
 *                   OUT     LONG * plValue )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 *             plValue : Récupère la valeur numérique.
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type nombre. Si c'est le cas, la fonction 
 *             extrait la valeur du nombre.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsNumber( 
        IN      char * pcName, 
        IN      char * pcParamVal, 
        OUT     LONG * plValue )
{
    DWORD dwLen = (DWORD)(strlen(pcName));
    char szVal[100];

    if ( _strnicmp( pcName, pcParamVal, dwLen ) != 0 )
        return FALSE;

    strncpy_s( szVal, 100, pcParamVal + dwLen, sizeof(szVal) );
    szVal[sizeof(szVal)-1] = '\0';
    *plValue = atol( szVal );

    return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsEnum( 
 *                   IN      char * pcName, 
 *                   IN      char * pcParamVal, 
 *                   OUT     LONG * plValue, 
 *                   IN      DWORD dwCount, 
 *                   IN      ... )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 *             plValue : Récupère la valeur de l'énumération
 *             dwCount : Nombre de valeurs possibles pour l'énumération.
 *             … : Composé d'une série de dwCount paires de paramètres :
 *                  pcEnum : Chaine de la valeur énumérée (de type char *)
 *                  lEnum : Valeur d'énumération correspondante (de type LONG)
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type énumération. Si c'est le cas, la fonction 
 *             extrait la valeur de l'énumération.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsEnum( 
        IN      char * pcName, 
        IN      char * pcParamVal, 
        OUT     LONG * plValue, 
        IN      DWORD dwCount, 
        IN      ... )
{
    DWORD dwLen = (DWORD)(strlen(pcName));
    char szVal[100];
    char * pcVal;
    LONG lVal;
    va_list sList;

    if ( _strnicmp( pcName, pcParamVal, dwLen ) != 0 )
        return FALSE;
    strncpy_s( szVal, 100, pcParamVal + dwLen, sizeof(szVal) );
    szVal[sizeof(szVal)-1] = '\0';
    va_start( sList, dwCount );

    while ( dwCount > 0 )
    {
        pcVal = va_arg( sList, char * );
        lVal = va_arg( sList, LONG );

        if ( _stricmp( szVal, pcVal ) == 0 )
        {
            *plValue = lVal;
            return TRUE;
        }
        dwCount --;
    }
    return FALSE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsBoolean( 
 *                   IN      char * pcName, 
 *                   IN      char * pcParamVal, 
 *                   OUT     BOOL * pbValue )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 *             pbValue : Récupère la valeur du booléen.
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type booléen. Si c'est le cas, la fonction 
 *             extrait la valeur du booleen.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsBoolean( 
        IN      char * pcName, 
        IN      char * pcParamVal, 
        OUT     BOOL * pbValue )
{
    LONG lValue;
    
    if ( CMDLParamAsEnum( pcName, pcParamVal, &lValue, 2, "FALSE", 0, "TRUE", 1 ) )
    {
        (*pbValue) = ( lValue != 0 );
        return TRUE;
    }

    return FALSE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CMDLParamAsFlag(
 *                   IN      char * pcName ,
 *                   IN      char * pcParamVal )
 * PARAMETERS: pcName : Nom du paramètre
 *             pcParamVal : Argument à analyser
 * RETURN    : FALSE si le paramètre ne correspond pas.
 *             TRUE si le paramètre correspond
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si un argument est de type flag. Si c'est le cas, la fonction le signal.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CMDLParamAsFlag(
        IN      char * pcName ,
        IN      char * pcParamVal )
{
    return ( _stricmp( pcName, pcParamVal ) == 0 );
}



PRIVATE char * WINAPI CMDLGetParam( 
        IN      char * pcCommand, 
        IN      char * pcParam, 
        IN OUT  DWORD * pdwParamSize )
{
    char * pcStart = pcCommand;
    char * pcCurrent;
    char cCurrent;
    char cString = '\0';
    BOOL bIsInString;
    DWORD dwCursor;

    // Suppression des séparateurs de début
    cCurrent = *pcStart;
    while ( CMDL_IS_SEPARATOR( cCurrent ) )
        cCurrent = *(++pcStart);
    if ( CMDL_IS_TERMINATOR( cCurrent ) )
        return NULL;

    // Rechercher la fin tout en recopiant le param
    pcCurrent = pcStart;
    bIsInString = FALSE;
    dwCursor = 0;
    while ( ( ! CMDL_IS_TERMINATOR( cCurrent ) ) && ( dwCursor < ( (*pdwParamSize) - 1 ) ) )
    {
        // Le caractère courant est un séparateur
        if ( CMDL_IS_SEPARATOR( cCurrent ) )
        {
            // Si on est dans une chaine, il en fait partie
            if ( ! bIsInString )
                break;
            // Sinon, c'est la fin du param
            else
                pcParam[dwCursor++] = cCurrent;
        }

        // Le caractère courant est un délimiteur de chaine
        else if ( CMDL_IS_STRING( cCurrent ) )
        {
            // On est à l'intérieur d'une chaine
            if ( bIsInString )
                // La chaine a été ouverte avec le même délimiteur, c'est la fin de la chaine
                if ( cString == cCurrent )
                    bIsInString = FALSE;
                // La chaine a été ouverte avec un autre délimiteur, le caractère fait partie de la chaine
                else
                    pcParam[dwCursor++] = cCurrent;
            // On commence une chaine
            else
            {
                cString = cCurrent;
                bIsInString = TRUE;
            }
        }

        // C'est un caractère normal
        else
            pcParam[dwCursor++] = cCurrent;

        // Caractère suivant
        cCurrent = *(++pcCurrent);
    }
    pcParam[dwCursor] = '\0';
    *pdwParamSize = dwCursor;
    if ( pcCurrent == pcStart )
        return NULL;

    return pcCurrent;
}

/*-------------------------------- END OF FILE ------------------------------*/
