/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : trctst
 * FILE       : trctst.c
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

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <windows.h>
#include <csr_excpt.h>
#include "trc.h"

#define MAX_THREADS 10
#define MAX_INSTANCES 10

DWORD gdwOptions = 0;
BOOL gbChrono = FALSE;
BOOL gbTerm = FALSE;
BOOL gbFullTrace = FALSE;
BOOL gbDirect = FALSE;
DWORD gdwBoucle = 1;
DWORD gdwDelai = 0xFFFFFFFF;
DWORD gdwNbInstances=1;
char gpcFichier[MAX_PATH+1] = "";
HANDLE gphThreads[10] = {NULL};
DWORD gdwNbThreads = 1;
BOOL gbEmetteur;
BOOL gbReceveur;
DWORD gdwNbParm;
PCHAR gppcParms[10];
TRC_EMETTEUR gpsInst[MAX_INSTANCES];
TRC_RECEVEUR gsEcoute;
char pcFichier[MAX_PATH+1] = "";

DWORD WINAPI TraceThread( DWORD dwThr );


/*^L*/
/*
 * $D_FCTN
 * -----------------------------------------------------------------
 * SYNTAX    : BOOL AnalyseSwitch( char * pcSwitch )
 * PARAMETERS: pcSwitch : parametre switch à analyser
 * RETURN    : TRUE si ok, FALSE si erreur
 * -----------------------------------------------------------------
 * OVERVIEW  : 
 * -----------------------------------------------------------------
 * $F_FCTN
 */
BOOL AnalyseSwitch( char * pcSwitch )
{
	if ( _stricmp( pcSwitch, "/EVENTLOG" ) == 0 )
	{
		gdwOptions |= TRC_OPT_EVENTLOG;
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/CONSOLE" ) == 0 )
	{
		gdwOptions |= TRC_OPT_CONSOLE;
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/TEXTESEUL" ) == 0 )
	{
		gdwOptions |= TRC_OPT_TEXTE_SEUL;
		return TRUE;
	}

    if ( _stricmp( pcSwitch, "/MAILSLOTLOCAL" ) == 0 )
	{
		gdwOptions |= TRC_OPT_MAILSLOT_LOCAL;
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/MAILSLOTDISTANT" ) == 0 )
	{
		gdwOptions |= TRC_OPT_MAILSLOT_DISTANT;
		return TRUE;
	}

	if ( _strnicmp( pcSwitch, "/FICHIER:", 9 ) == 0 )
	{
		gdwOptions |= TRC_OPT_FICHIER | TRC_OPT_CREER_FICHIER;
		strcpy_s(gpcFichier, MAX_PATH, pcSwitch + 9);
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/NUMEROTATION" ) == 0 )
	{
		gdwOptions |= TRC_OPT_NUMEROTATION;
		return TRUE;
	}

    if ( _stricmp( pcSwitch, "/FULLTRACE" ) == 0 )
	{
		gbFullTrace = TRUE;
		return TRUE;
	}

    if ( _stricmp( pcSwitch, "/TERM" ) == 0 )
	{
		gbTerm = TRUE;
		return TRUE;
	}

    if ( _stricmp( pcSwitch, "/DIRECT" ) == 0 )
	{
		gbDirect = TRUE;
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/CHRONO" ) == 0 )
	{
		gbChrono = TRUE;
		return TRUE;
	}

	if ( _stricmp( pcSwitch, "/BOUCLE" ) == 0 )
	{
		gdwBoucle = 0xFFFFFFFF;
		return TRUE;
	}

	if ( _strnicmp( pcSwitch, "/BOUCLE:", 8 ) == 0 )
	{
		gdwBoucle = atol( pcSwitch + 8 );
		return TRUE;
	}

    if ( _strnicmp( pcSwitch, "/INSTANCES:", 11 ) == 0 )
	{
		gdwNbInstances = atol( pcSwitch + 11 );
        if ( gdwNbInstances == 0 || gdwNbInstances > MAX_INSTANCES )
            return FALSE;
		return TRUE;
	}

	if ( _strnicmp( pcSwitch, "/DELAI:", 7 ) == 0 )
	{
		gdwDelai = atol( pcSwitch + 7 );
		return TRUE;
	}

	if ( _strnicmp( pcSwitch, "/THREADS:", 9 ) == 0 )
	{
		gdwNbThreads = atol( pcSwitch + 9 );
		return TRUE;
	}

	return FALSE;
}



void EcrireErreur( char * pcDebut, DWORD dwErr )
{
	if ( dwErr == ERROR_SUCCESS )
    {
        if ( gbFullTrace )
            printf( "%s . Opération effectuée avec succes.\n", pcDebut );
    }
	else
	{
		char pcMessage[1000];
		if ( ! FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErr,
			0,
			pcMessage,
			sizeof(pcMessage),
			NULL ) )
			fprintf( stderr, "%s . Impossible de trouver le texte associe a l'erreur %d\n", pcDebut, dwErr );
		else
			fprintf( stderr, "%s . Erreur %d : %s\n", pcDebut, dwErr, pcMessage );
	}
}






/*^L*/
/*
 * $D_FCTN
 * -----------------------------------------------------------------
 * SYNTAX    : int main( int iArgc, char * * ppcArgv )
 * PARAMETERS: Cf ANSI specs
 * RETURN    : Cf ANSI spec
 * -----------------------------------------------------------------
 * OVERVIEW  : 
 * -----------------------------------------------------------------
 * $F_FCTN
 */
int main( int iArgc, char ** ppcArgv )
{
	DWORD dwIdx, dwErr, dwFoo, dwTick1, dwTick2;
    char pcName[MAX_PATH];

	gdwNbParm = 0;

	for ( dwIdx = 1 ; dwIdx < (DWORD)iArgc && dwIdx < 10 ; dwIdx ++ )
		if ( ppcArgv[dwIdx][0] == '/' )
		{
			if ( ! AnalyseSwitch( ppcArgv[dwIdx] ) )
			{
				fprintf( stderr, "Invalid switch" );
				return ERROR_INVALID_PARAMETER;
			}
		}
		else
		{
			gppcParms[gdwNbParm] = ppcArgv[dwIdx];
			gdwNbParm ++ ;
		}

	if ( gdwNbParm == 0 )
    {
		printf("Ligne de commande : \n\
TRACETST EMETTEUR  cle\n\
         [/CONSOLE]\n\
         [/EVENTLOG]\n\
         [/MAILSLOTLOCAL]\n\
         [/MAILSLOTDISTANT]\n\
         [/FICHIER:fichier]\n\
         [/NUMEROTATION]\n\
         [/CHRONO]\n\
         [/TERM]\n\
         [/FULLTRACE]\n\
         [/BOUCLE:nnn]\n\
         [/DELAI:nnn]\n\
         [/DIRECT]\n\
         [/TEXTESEUL]\n\
         [/THREADS:n]\n\n\
TRACETST RECEVEUR  cle\n\
         [/CHRONO]\n\
         [/BOUCLE:nnn]\n\
         [/DELAI:nnn]\n\
"
			);
        return NO_ERROR;
    }
	else
	{

		gbEmetteur = ( _stricmp(  gppcParms[0], "EMETTEUR" ) == 0 && gdwNbParm == 2 );
		gbReceveur = ( _stricmp(  gppcParms[0], "RECEVEUR" ) == 0 && gdwNbParm == 2 );
		if ( gbReceveur ) gdwNbThreads = 1;

		if ( gbEmetteur )
		{

            for ( dwIdx = 0 ; dwIdx < gdwNbInstances ; dwIdx ++ )
            {
                if ( gdwNbInstances != 1 )
					sprintf_s(pcName, MAX_PATH, "%s%d", gppcParms[1], dwIdx);
                else
					strcpy_s(pcName, MAX_PATH, gppcParms[1]);

			    dwErr = TRC_Initialise_Trace(
				    pcName,
				    (strlen(gpcFichier)==0?"":gpcFichier),
					TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_TEXTE_SEUL,
				    &gpsInst[dwIdx] );
                EcrireErreur( "TRC_Initialise_Trace", dwErr );

			    if ( dwErr == NO_ERROR && ( gdwOptions & TRC_OPT_FICHIER ) != 0 )
                {
 				    dwErr = TRC_Taille_Max_Fichier( gpsInst[dwIdx], 10000 );
                    EcrireErreur( "TRC_Taille_Max_Fichier", dwErr );
                }
			    if ( dwErr != NO_ERROR )
                    return dwErr;
            }
		}
		else if ( gbReceveur )
        {
			dwErr = TRC_Initialise_Ecoute(
				gppcParms[1],
				&gsEcoute );
            EcrireErreur( "TRC_Initialise_Ecoute", dwErr );
			if ( dwErr != NO_ERROR )
                return dwErr;
        }

		for ( dwIdx = 0 ; dwIdx < gdwNbThreads ; dwIdx ++ )
		{
			gphThreads[dwIdx] = ExcptCreateThread(
				NULL,
				0, 
				(LPTHREAD_START_ROUTINE)TraceThread,
				(LPVOID)dwIdx,
				CREATE_SUSPENDED,
				&dwFoo,
                "TRC_TEST_THREAD" );
		}

        dwTick1 = GetTickCount();
		for ( dwIdx = 0 ; dwIdx < gdwNbThreads ; dwIdx ++ )
			ResumeThread( gphThreads[dwIdx] );

		WaitForMultipleObjects( gdwNbThreads, gphThreads, TRUE, INFINITE );
        dwTick2 = GetTickCount();
        printf( "Temps d'execution : %d ms\n", dwTick2-dwTick1 );

        if ( gbTerm )
        {
            EcrireErreur( "Forcage de fin avant libération", NO_ERROR );
            exit( NO_ERROR );
        }

		if ( gbEmetteur )
		{
            for ( dwIdx = 0 ; dwIdx < gdwNbInstances ; dwIdx ++ )
            {
				dwErr = TRC_Termine_Trace( gpsInst[dwIdx] );
                EcrireErreur( "TRC_Termine_Trace", dwErr );
            }
		}
		else if ( gbReceveur )
		{
			dwErr = TRC_Termine_Ecoute( gsEcoute );
            EcrireErreur( "TRC_Termine_Ecoute", dwErr );
		}

        if ( gbEmetteur )
        {
			dwErr = TRC_Vider_Traces(INFINITE);
            EcrireErreur( "TRC_Vider_Traces", dwErr );
        }
	}
    _getch();
	return NO_ERROR;
}


DWORD WINAPI TraceThread( DWORD dwThr )
{
		DWORD dwChrono[10];
        DWORD dwIdx;
		DWORD dwErr;
		TRC_MESSAGE psMsg;
		DWORD dwBoucle = gdwBoucle;
        char szMsg[3000];

		dwErr = NO_ERROR;

        for ( dwIdx = 0 ; dwIdx < sizeof(szMsg) ; dwIdx ++ )
            szMsg[dwIdx] = 'A' + (char)( dwIdx % 26 );
        szMsg[sizeof(szMsg)-1] = '\0';

		if ( (! gbEmetteur) && (! gbReceveur) )
			dwErr = ERROR_INVALID_PARAMETER;

		dwChrono[0] = GetTickCount();

		while ( dwErr == ERROR_SUCCESS && dwBoucle != 0 )
		{
			dwBoucle --;
			if ( gbEmetteur )
			{
                for ( dwIdx = 0 ; dwIdx < gdwNbInstances ; dwIdx ++ )
                {
                    if ( gbDirect )
				        dwErr = TRC_Direct_Trace(
					        gpsInst[dwIdx],
					        gdwOptions,
					        "0123456789ABCDEF",
					        ( rand()%15 ) + 1,
					        "[%s]:%d:[%s]", gppcParms[1], dwBoucle, szMsg );
                    else
				        dwErr = TRC_Trace(
					        gpsInst[dwIdx],
					        gdwOptions,
					        "0123456789ABCDEF",
					        ( rand()%15 ) + 1,
					        "[%s]:%d=0x%08X", gppcParms[1], dwBoucle, dwBoucle );
				    EcrireErreur( "TRC_Trace", dwErr );
                }

				if ( gdwDelai != 0xFFFFFFFF )
					Sleep( gdwDelai );
			}
				
			else if ( gbReceveur )
			{
				dwErr = TRC_Ecoute_Message(
					gsEcoute,
					&psMsg );
    		    EcrireErreur( "TRC_Ecoute_Message", dwErr );
				if ( dwErr == NO_ERROR && psMsg != NULL )
				{
					printf( "%d %s\n", TRC_Message_Num(psMsg), TRC_Message_Texte(psMsg) );
					dwErr = TRC_Libere_Message( psMsg );
        		    EcrireErreur( "TRC_Libere_Message", dwErr );
				}
				else
					if ( gdwDelai != 0xFFFFFFFF )
						Sleep( gdwDelai );
			}
		}
	
		dwChrono[1] = GetTickCount();

		if ( gbChrono && dwErr == NO_ERROR )
			printf( "Temps d'execution Messages      : %d ms\n", dwChrono[1] - dwChrono[0] );

		ExitThread( dwErr );
		return dwErr;
}


