/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : LWD (Launcher/Watch Dog)
 * FICHIER    : TEST.C
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : process, program, key
 * -----------------------------------------------------------------
 * RESUME     : Test du Lanceur et surveillance d'application (de programmes)
 * --------------------------------------------------------------------
 * DESCRIPTION: Ce programme teste le module LWD (Launcher/Watch Dog)
 *              Le programme applicatif doit deja tourne (lance via startup)
 *
 * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * Revision   : 1.0.0
 * Date       : 09/1998
 * Auteur     : AFX
 * Evolution  : creation
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */


/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include <csrlc32.h>

#include <lwd.h>

/*--------------- RESERVED: ---------------*/
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void main(void)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal : point d'entree dans l'application
* ROLE: Lancer le soft VM
* --------------------------------------------------------------------
* $F_FCTN
*/

void main(int argc, char *argv[])
{
	char ch;
	DWORD code;
	char pcErreur[513];

	printf ("\n ** Test du signal 'fin application' pour voie NT **");
	printf ("\n");
	printf ("\n");
//	printf ("\nLancer le Startup (qui lancera tous les programmes lus dans la cle 'Startup').");
	printf ("\nAppuyez sur 'e' pour envoyer le signal 'fin application'...");

	do   
	{      
		ch = _getch();
		ch = toupper( ch );
	}
	while( ch != 'E' );

	/* Signaler au process Startup la fin volontaire de l'application */
	code = LWD_Signale_Fin_Application ();

	if (code == ERROR_SUCCESS)
		printf ("\nFin programme bien signalee ('Startup' doit se terminer apres x secondes max).");
	else
	{
		if ( FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			code,
			0,
			pcErreur,
			512,
			NULL ) != 0 )
			pcErreur[512] = '\0';
		else
			strcpy( pcErreur, "Texte erreur impossible a avoir");
		printf ("\n\nErreur %lu lors du signal 'fin application'", code);
		printf ("\nLibelle erreur = %s", pcErreur);
	}
	printf ("\n\nAppuyez sur une touche pour terminer ...");
	_getch();
}