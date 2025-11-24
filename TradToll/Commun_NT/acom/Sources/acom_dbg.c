/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_dbg.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions de trace et de debug
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <trc.h>

#define LOC_DEF
#include <acom_dbg.h>
#undef LOC_DEF

#include <memclass.h>


// BOOLEENS D'INDICATION DE L'ETAT DES TRACES

// Etat d'activation des traces informatives
PRIVATE gbInfoIsActive = FALSE;

// Etat d'activation des traces d'erreur
PRIVATE gbErrorIsActive = FALSE;

// Handle de l'instance de trace
PRIVATE TRC_EMETTEUR ghTrace = NULL;

// Section critique d'accès à l'instance de trace (pour création et destruction)
PRIVATE CRITICAL_SECTION gsTrcInit;

// PROTOTYPES PRIVES
PRIVATE char * AComDbgRemoveCtrl( char * pcText );
PRIVATE char * AComDbgFindName( char * pcFilePath );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgInit()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise la section critique d'accès aux handle de trace
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgInit()
{
    InitializeCriticalSection( &gsTrcInit );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComDbgOpen( BOOL bInfo, BOOL bError, char * pcFileName )
 * PARAMETERS: BOOL bInfo        : Etat d'activation des traces informatives élémentaires
 *             BOOL bError       : Etat d'activation des traces d'erreur
 *             char * pcFileName : Nom du fichier de trace
 * RETURN    : NO_ERROR si ok
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Active la trace. Si elle est déjà active, retourne une erreur
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComDbgOpen( BOOL bInfo, BOOL bError, char * pcFileName )
{
    DWORD dwErr = NO_ERROR;
    EnterCriticalSection( &gsTrcInit );
    if ( ghTrace == NULL )
    {
        gbInfoIsActive = FALSE;
        gbErrorIsActive = FALSE;
        if ( bInfo || bError )
        {
            dwErr = TRC_Initialise_Trace( 
                    "ACom", 
                    pcFileName, 
                    TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_CREER_FICHIER,
                    &ghTrace );
            if ( dwErr == ERROR_SUCCESS )
            {
                gbInfoIsActive = bInfo;
                gbErrorIsActive = bError;

				TRC_Taille_Max_Fichier(ghTrace, 1024*1024 * 10); //set 10MB trace 
            }
        }
    }
    else 
        dwErr = ERROR_ALREADY_EXISTS;
    LeaveCriticalSection( &gsTrcInit );

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgClose()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Désactive la trace si elle est active
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgClose()
{
    EnterCriticalSection( &gsTrcInit );
    if ( ghTrace != NULL )
    {
        TRC_Vider_Traces( 10000 );
        TRC_Termine_Trace( ghTrace );
        ghTrace = NULL;
    }
    LeaveCriticalSection( &gsTrcInit );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgError ( DWORD dwErr, char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
 * PARAMETERS: DWORD dwErr       : Code de l'erreur win32, 0 si autre que win32 ou inconnu
 *             char * pcFilePath : fichier source
 *             DWORD dwLine      : ligne de code
 *             char * pcFormat   : Texte de la trace
 *             ...               : 
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace d'erreur
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgError( DWORD dwErr, char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
{
    DWORD dwSize;
    va_list sMarker;
    FILE * pfFile = stderr;
    char szErr[256];

    if ( gbErrorIsActive )
    {
        if ( dwErr == NO_ERROR )
            strcpy_s( szErr, 256, "ERR=N/C" ); 
        else
        {
            dwSize = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErr,
                0,
                szErr,
                sizeof(szErr),
                NULL );
            if ( dwSize == 0 )
                sprintf_s( szErr, 256, "ERR=%d", dwErr );
            else
                AComDbgRemoveCtrl( szErr );
        }

        TRC_Trace_Texte(
            ghTrace, 
            TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_CREER_FICHIER,
            "[%s(%u)/TId=0x%016X] - ERROR%d:\"%s\"",
            AComDbgFindName( pcFilePath ),
            dwLine,
            GetCurrentThreadId(),
            dwErr, 
            szErr );
        va_start( sMarker, pcFormat );     
        TRC_Trace_Texte_V(
            ghTrace, 
            TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_CREER_FICHIER,
            pcFormat,
            sMarker );
    }
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComDbgInfo ( char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
 * PARAMETERS: char * pcFilePath : fichier source
 *             DWORD dwLine      : ligne de code
 *             char * pcFormat   : Texte de la trace
 *             ...               : 
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une trace informative
 * --------------------------------------------------------------------
 */
PROTECTED void AComDbgInfo( char * pcFilePath, DWORD dwLine, char * pcFormat, ... )
{
    FILE * pfFile = stderr;
	char szString1[2000] = { 0 };
	char szString2[2000] = { 0 };
	va_list args;
	

    if ( gbInfoIsActive )
    {
		sprintf_s(szString1, sizeof(szString2), "[%s(%u)/TId=0x%016X] - INFO -", AComDbgFindName(pcFilePath), dwLine, GetCurrentThreadId());

		va_start(args, pcFormat);
		_vsnprintf_s(szString2, sizeof(szString2), sizeof(szString2), pcFormat, args);
		va_end(args);

		strcat_s(szString1, sizeof(szString2), szString2);
  
        TRC_Trace_Texte_V(
            ghTrace, 
            TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_CREER_FICHIER,
			szString1,
            NULL );
    }
}

PROTECTED void WINAPI AComDbgTraceHexBuffer(char * szText, DWORD dwLen, BYTE * pBuffer)
{
	char szBuffer[2000] = {0};
	char szHexByte[4];
	DWORD i;
	DWORD iMaxBytesLen;

	if (gbInfoIsActive)
	{
		strcpy_s(szBuffer, sizeof(szBuffer), szText);
		iMaxBytesLen = (int)(sizeof(szBuffer) - strlen(szBuffer)) >> 2;

		for (i = 0; i < dwLen && i < iMaxBytesLen; i++)
		{
			sprintf_s(szHexByte, sizeof(szHexByte), " %02x", pBuffer[i]);
			strcat_s(szBuffer, sizeof(szBuffer), szHexByte);
		}

		TRC_Trace_Texte_V(
			ghTrace,
			TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_CREER_FICHIER,
			szBuffer,
			NULL);
	}
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * AComDbgRemoveCtrl ( char * pcText )
 * PARAMETERS: char * pcText : Chaine de caractère à traiter
 * RETURN    : pcText
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Supprime les caractères de control dans une chaine en les
 *             remplaçant par des espaces
 * --------------------------------------------------------------------
 */
PRIVATE char * AComDbgRemoveCtrl( char * pcText )
{
    char * pcCurrent = pcText;  
    while ( *pcCurrent != '\0' )
    {
        if ( *pcCurrent < ' ' )
            *pcCurrent = ' ';
        pcCurrent ++;
    }
    return pcText;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * AComDbgFindName ( char * pcFilePath )
 * PARAMETERS: char * pcFilePath : Chaine contenant un chemin d'accès à un
 *                                 fichier
 * RETURN    : Un pointeur sur le nom du fichier
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Recherche un nom de fichier dans un chemin.
 * --------------------------------------------------------------------
 */
PRIVATE char * AComDbgFindName( char * pcFilePath )
{
    char * pcLast = pcFilePath;
    char * pcScan;
    while ( ( pcScan = strchr(pcLast,':') ) != NULL )
        pcLast = pcScan + 1;
    while ( ( pcScan = strchr(pcLast,'\\') ) != NULL )
        pcLast = pcScan + 1;
    return pcLast;
}




PROTECTED const char * GetNameForBlockType(DWORD dwBlockType)
{
	switch (dwBlockType)
	{
	case 0x00030001:return "COMMAND_SEND      ";
	case 0x00030002:return "COMMAND_DISCONNECT";
	case 0x00030003:return "COMMAND_LIFE      ";
	default:
		return "Unknown Command";
	}
}


PROTECTED const char * GetNameForConnectionState(DWORD dwConnectionState)
{
	switch (dwConnectionState)
	{
	case 0x00010001:return "CONNECTION_STATE_FREE        ";
	case 0x00010002:return "CONNECTION_STATE_INIT        ";
	case 0x00010003:return "CONNECTION_STATE_CONNECTING  ";
	case 0x00010004:return "CONNECTION_STATE_CONNECTED   ";
	case 0x00010005:return "CONNECTION_STATE_DISCONNECTED";
	case 0x00010006:return "CONNECTION_STATE_ERROR       ";
	case 0x00010007:return "CONNECTION_STATE_SHUTDOWN    ";
	default:
		return "CONNECTION_STATE_Unknown ";
	}
}



/* -------------  FIN DU FICHIER : acom_dbg.c ------------- */ 
