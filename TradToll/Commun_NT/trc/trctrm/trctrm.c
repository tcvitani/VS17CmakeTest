/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : TRACETRM
 * FICHIER    : TRACETRM.C
 * LANGAGE    : C (VC++ 5.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Trace, log
 * -----------------------------------------------------------------
 * RESUME     : Terminal de trace pour le module de trace distante (via réseau)
 *              basé sur les mail slots
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * $Log : $
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */



/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "trc.h"

#define TAILLE_MAX 255

#define INTENSE_TEXTE(coul,yesno) (WORD)(coul|(yesno?FOREGROUND_INTENSITY:0))
#define INTENSE_FOND(coul,yesno) (WORD)(coul|(yesno?BACKGROUND_INTENSITY:0))
#define COULEUR_NUM (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|FOREGROUND_INTENSITY|FOREGROUND_BLUE)
#define COULEUR_TICK (FOREGROUND_GREEN)
#define COULEUR_DATE (FOREGROUND_GREEN)
#define COULEUR_HEURE (FOREGROUND_GREEN)
#define COULEUR_TEXTE (FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED)
#define COULEUR_DONNEES (FOREGROUND_GREEN|FOREGROUND_BLUE)


#define PARAM_CHAINE(arg,nom,var) \
	if(_strnicmp(arg,nom,strlen(nom))==0)\
		{strncpy_s(var, TAILLE_MAX, arg + strlen(nom), sizeof(var)-1); var[sizeof(var)-1] = '\0'; continue; }

#define PARAM_DWORD(arg,nom,var) \
	if(_strnicmp(arg,nom,strlen(nom))==0)\
		{var=(DWORD)(atol(arg+strlen(nom)));continue;}


#define PARAM_BOOL(arg,nom,var) \
	if(_strnicmp(arg,nom,strlen(nom))==0)\
		{var=TRUE;continue;}



BOOL gbNoDonnees = FALSE;
BOOL gbNoTexte = FALSE;
BOOL gbNoNum = FALSE;
BOOL gbNoDate = FALSE;
BOOL gbNoHeure = FALSE;
BOOL gbNoTick = FALSE;
BOOL gbCouleur = FALSE;
DWORD gdwDelai = 40;
char gpcId[TAILLE_MAX+1] = "";



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void Afficher_Entete()
 *             void Afficher_Aide()
 *             void Afficher_Erreur( char * pcTexte, DWORD dwErr )
 * PARAMETRES: pcTexte : Texte à rajouter en plus de celui de l'erreur
 *             dwErr   : code de l'erreur
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Affiche les différentes portions de texte du programme
 * --------------------------------------------------------------------
 * $F_FCTN
 */

void Afficher_Entete()
{
	printf( "TRACETRM - Terminal de trace a distance\n" );
}

void Afficher_Aide()
{
	printf( "\
Ligne de commande :\n\
   TRACETRM  ID=traceId\n\
            [DELAI=nnn]\n\
            [-DONNEES]\n\
            [-TEXTE]\n\
            [-NUM]\n\
            [-DATE]\n\
            [-HEURE]\n\
			[-TICK]\n\
            [+COULEUR]\n\
"
		);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void Afficher_Erreur ( char * pcTexte, DWORD dwErr )
 * PARAMETERS: char * pcTexte : 
 *             DWORD dwErr    : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
void Afficher_Erreur( char * pcTexte, DWORD dwErr )
{
	DWORD dwLen;
	char pcMessage[TAILLE_MAX+1];

	dwLen = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErr,
		0,
		pcMessage,
		TAILLE_MAX+1,
		NULL );
	if ( dwLen == 0 )
		strcpy_s(pcTexte, TAILLE_MAX, "Code inconnu");

	fprintf( stderr, "ERREUR: %s (0x%08X/%d/%s)\n", pcTexte, dwErr, dwErr, pcMessage );
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD Analyser_Parametres( int iArgc, char * * ppcArgv )
 * PARAMETRES: iArgc   : nombre de paramètres à analyser
 *             ppcArgv : liste des paramètre à analyser (commence au premier
 *                       paramètre et non au nom du programme).
 * RETOUR    : Code erreur Win32, NO_ERROR si pas d'erreur
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Analyse les paramètres de ligne de commande
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD Analyser_Parametres( int iArgc, char * * ppcArgv )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwIdx;
	char * pcArg;

	for ( dwIdx = 0 ; dwIdx < (DWORD)iArgc ; dwIdx ++ )
	{
		pcArg = ppcArgv[dwIdx];

		PARAM_CHAINE( pcArg, "ID=",      gpcId)
		PARAM_DWORD(  pcArg, "DELAI=",   gdwDelai)
		PARAM_BOOL(   pcArg, "-DONNEES", gbNoDonnees)
		PARAM_BOOL(   pcArg, "-TEXTE",   gbNoTexte)
		PARAM_BOOL(   pcArg, "-NUM",     gbNoNum)
		PARAM_BOOL(   pcArg, "-DATE",    gbNoDate)
		PARAM_BOOL(   pcArg, "-HEURE",   gbNoHeure)
		PARAM_BOOL(   pcArg, "-TICK",    gbNoTick)
		PARAM_BOOL(   pcArg, "+COULEUR", gbCouleur)

		dwErr = ERROR_INVALID_PARAMETER;
		Afficher_Erreur( pcArg, dwErr );
	}

	return dwErr;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD Terminal()
 * PARAMETRES: Aucun
 * RETOUR    : Code erreur Win32, NO_ERROR si pas d'erreur
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Initialise la trace, attent les trache et les affiche.
 *             Sortir si touche ESC frappée.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD Terminal()
{
	BOOL bFin = FALSE;
    BOOL bDirect;
	DWORD dwErr;
	DWORD dwErr2;
	DWORD dwNum;
	DWORD dwTick;
	DWORD dwSpc;
	DWORD dwTailleDonnees;
	BYTE * pbDonnees;
	char * pcTexte;
	TRC_MESSAGE psMsg;
	SYSTEMTIME sDate;
	TRC_RECEVEUR psRec;
	HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO sConsole;

	if ( strlen( gpcId ) > 0 ||
		 ( gbNoTexte && gbNoDonnees )
		)
	{
		dwErr = TRC_Initialise_Ecoute( gpcId, &psRec );
		if ( dwErr == NO_ERROR )
		{
			if ( gbCouleur )
				GetConsoleScreenBufferInfo( hConsole, &sConsole );
			printf( "Mise en ecoute de la console sur \"%s\"\n", gpcId );
			printf( "Appuyez sur ESC pour arreter ...\n" );
			while ( dwErr == NO_ERROR && ! bFin )
			{
				dwErr = TRC_Ecoute_Message( psRec, &psMsg );
				if ( dwErr == NO_ERROR )
					if ( psMsg != NULL )
					{
						dwNum = TRC_Message_Num( psMsg );
						bDirect = TRC_Message_Direct( psMsg );
						TRC_Message_Date_Locale( psMsg, &sDate );
						pcTexte = TRC_Message_Texte( psMsg );
						pbDonnees = TRC_Message_Donnees( psMsg );
						dwTailleDonnees = TRC_Message_Taille_Donnees( psMsg );
						dwTick = TRC_Message_Tick( psMsg );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_FOND(COULEUR_NUM,dwNum) );
						if ( ! gbNoNum )
                            if ( bDirect )
								printf( "%-10s;", "DIRECT" );
							else if ( dwNum != 0 )
								printf( "0x%08X;", dwNum );
							else
								printf( "          ;" );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_TEXTE(COULEUR_DATE,dwNum) );
						if ( ! gbNoDate )
							printf( "%04d/%02d/%02d;",
									(int)(sDate.wYear),
									(int)(sDate.wMonth),
									(int)(sDate.wDay) );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_TEXTE(COULEUR_HEURE,dwNum) );
						if ( ! gbNoHeure )
							printf( "%02d:%02d:%02d.%03d;",
									(int)(sDate.wHour),
									(int)(sDate.wMinute),
									(int)(sDate.wSecond),
									(int)(sDate.wMilliseconds) );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_TEXTE(COULEUR_TICK,dwNum) );
						if ( ! gbNoTick )
							printf( "%010u;", dwTick );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_TEXTE(COULEUR_TEXTE,dwNum) );
						if ( ! gbNoTexte && pcTexte[0] != '\0' )
							printf( "\"%s\";", pcTexte );

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, INTENSE_TEXTE(COULEUR_DONNEES,dwNum) );
						if ( ! gbNoDonnees && dwTailleDonnees > 0 )
						{
							dwSpc = 0;
							printf( "[ " );
							while ( dwTailleDonnees > 0 )
							{
								dwTailleDonnees --;
								dwSpc = ( dwSpc + 1 ) % 4;
								printf( "%02x %s",
									(int)(*pbDonnees),
									( dwSpc == 0 && dwTailleDonnees > 0 )? "- " : "" );
								pbDonnees ++;
							}
							printf( "]" );
						}

						if ( gbCouleur )
							SetConsoleTextAttribute( hConsole, sConsole.wAttributes );

						printf( "\n" );

						dwErr = TRC_Libere_Message( psMsg );
						if ( dwErr != NO_ERROR )
							Afficher_Erreur( "Echec sur liberation de message", dwErr );
					}
				else
					Sleep( gdwDelai );
				if ( _kbhit() )
					bFin = ( _getch() == 27 );

			}

			while ( _kbhit() ) _getch();

			dwErr2 = TRC_Termine_Ecoute( psRec );
			if ( dwErr2 != NO_ERROR )
				Afficher_Erreur( "Echec sur fermeture de l'ecoute", dwErr2 );
			if ( dwErr == NO_ERROR )
				dwErr = dwErr2;
		}
		else
		{
			Afficher_Erreur( "Impossible d'initialiser l'ecoute", dwErr );
		}
	}
	else
	{
		dwErr = ERROR_INVALID_PARAMETER;
		Afficher_Erreur( "Parametre manquant ou incompatibles", dwErr );
	}

	return dwErr;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : int main( int iArgc, char * * ppcArgv )
 * PARAMETRES: Cf. ANSI Spec
 * RETOUR    : Code erreur Win32, NO_ERROR si pas d'erreur
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Cf. ANSI spec
 * --------------------------------------------------------------------
 * $F_FCTN
 */
int main( int iArgc, char * * ppcArgv )
{
	DWORD dwErr = NO_ERROR;
	DWORD dwLen;
	char pcTitre[TAILLE_MAX+1];

	dwLen = GetConsoleTitle( pcTitre, TAILLE_MAX+1 );
	if ( dwLen > 0 )
		SetConsoleTitle( "TraceTrm - Terminal de trace a distance" );

	Afficher_Entete();

	if ( iArgc <= 1 )
		Afficher_Aide();
	else
	{
		dwErr = Analyser_Parametres( iArgc - 1, ppcArgv + 1 );
		if ( dwErr == NO_ERROR )
			dwErr = Terminal();
	}
	
	if ( dwLen > 0 )
		SetConsoleTitle( pcTitre );

	return (int)dwErr;
}