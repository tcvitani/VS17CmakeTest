/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : REG (REG)
 * FICHIER    : REG.C
 * LANGAGE    : C (VC++ 5.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Registre, configuration
 * -----------------------------------------------------------------
 * RESUME     : Lecture, modification et creation de valeurs dans la
 *              base de registre Win32
 * --------------------------------------------------------------------
 * DESCRIPTION: Fournit une serie de fonctions facilitant l'acces aux
 *              donnees du registre Win32 (Acces en une fonction au lieu
 *              de 3 minimum avec l'API Win32 directement.
 *              Contrairement aux fonctions de l'API WIN32, les valeurs
 *              réservées HKEY_XXXXX sont directement utilisables. Toutefois,
 *              par soucis de performances, dans le cas d'accés de nombreuses
 *              valeurs sous la même clé, il est préférable de travailler
 *              avec un handle obtenu par RegOpenKeyEx, puis de le refermer
 *              avec RegCloseKey.
 *              De plus, dans le cas de modifications de valeurs, un appel
 *              a RegFlushKey permettra de s'assurer du bon stockage des
 *              informations sur le disque.
  * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * Revision   : 1.0.0
 * Date       : 07/1998
 * Auteur     : NBL
 * Evolution  : creation
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */


#include <windows.h>
//#include <VS6Migration.h>
#include <memclass.h>
#include "reg.h"

#define REG_ACCESS_HEADER "REGCSR_"

typedef struct _REG_NOTIFICATION
{
    HKEY    hKey;
    HANDLE  hEvent;

    BOOL    bSubTree;
}
    REG_NOTIFICATION;


/*--------------- CODE: ---------------*/


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Ouvrir_Access(
 *      char * pcMachine,
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      HKEY * phCle,
 *      REGSAM dwReqAccess )
 * PARAMETRES: pcMachine : pointe sur une chaîne de caractères terminée
 *                   par un ‘\0’ donnant le nom de la machine sur
 *                   laquelle se connecter sous la forme :
 *                   "\\NomMachine" (attention, en C, "\\"
 *                   devient "\\\\"). Si cette pcMachine est NULL,
 *                   alors il s’agit de la machine locale.
 *             hRacine : handle prédéfini d’une entrée de registre :
 *                        + HKEY_CLASSES_ROOT
 *                        + HKEY_CURRENT_USER
 *                        + HKEY_LOCAL_MACHINE
 *                        + HKEY_USERS
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                   par un ‘\0’ donnant le chemin de la clé servant de
 *                   base à la connexion sous la forme
 *                   "cle\sous-cle\sous-sous-cle" (attention, en C, "\"
 *                   devient "\\").
 *             phCle : pointe sur une variable récupérant le handle de
 *                   connexion.
 *             dwReqAccess : Acces requis pour l'ouverture : KEY_ALL_ACCESS,
 *                   KEY_READ, KEY_WRITE, KEY_QUERY_VALUE, KEY_SET_VALUE etc.
 *                   ou toute autre combinaison bit à bit de ces valeurs.
 * RETOUR    : ERROR_SUCCESS : la connexion a réussi et (*phCle) est
 *             mis à jour. Autre code d’erreur, il s’agit d’un code
 *             d’erreur standard Win32 dont le texte peut être obtenu
 *             avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Se connecte à un registre distant ou ouvre le registre
 *             local à un endroit précis.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ouvrir_Access(
       char * pcMachine,
       HKEY hRacine,
       char * pcSousCle,
       HKEY * phCle,
       REGSAM dwReqAccess )
{
	DWORD dwErreur = ERROR_SUCCESS;
	HKEY hCle = NULL;
	HKEY hCnx = hRacine;

	if ( pcMachine != NULL )
		if ( strlen(pcMachine) != 0 && strcmp(pcMachine,"\\\\.") != 0 )
			dwErreur = RegConnectRegistry( pcMachine, hRacine, &hCnx );

	if ( dwErreur == ERROR_SUCCESS )
	{
		dwErreur = RegOpenKeyEx(
				hCnx,
				pcSousCle,
				0,
				dwReqAccess,
				&hCle );
		if ( hCnx != hRacine )
			if ( dwErreur == ERROR_SUCCESS )
			{
				dwErreur = RegCloseKey( hCnx );
				if ( dwErreur != ERROR_SUCCESS )
					RegCloseKey( hCle );
			}
			else
				RegCloseKey( hCnx );

	}
	(*phCle) = hCle;
	return dwErreur;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Ouvrir(
 *      char * pcMachine,
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      HKEY * phCle )
 * PARAMETRES: pcMachine : pointe sur une chaîne de caractères terminée
 *                   par un ‘\0’ donnant le nom de la machine sur
 *                   laquelle se connecter sous la forme :
 *                   "\\NomMachine" (attention, en C, "\\"
 *                   devient "\\\\"). Si cette pcMachine est NULL,
 *                   alors il s’agit de la machine locale.
 *             hRacine : handle prédéfini d’une entrée de registre :
 *                        + HKEY_CLASSES_ROOT
 *                        + HKEY_CURRENT_USER
 *                        + HKEY_LOCAL_MACHINE
 *                        + HKEY_USERS
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                   par un ‘\0’ donnant le chemin de la clé servant de
 *                   base à la connexion sous la forme
 *                   "cle\sous-cle\sous-sous-cle" (attention, en C, "\"
 *                   devient "\\").
 *             phCle : pointe sur une variable récupérant le handle de
 *                   connexion.
 * RETOUR    : ERROR_SUCCESS : la connexion a réussi et (*phCle) est
 *             mis à jour. Autre code d’erreur, il s’agit d’un code
 *             d’erreur standard Win32 dont le texte peut être obtenu
 *             avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Se connecte à un registre distant ou ouvre le registre
 *             local à un endroit précis.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ouvrir(
       char * pcMachine,
       HKEY hRacine,
       char * pcSousCle,
       HKEY * phCle )
{
	return REG_Ouvrir_Access( pcMachine, hRacine, pcSousCle, phCle, KEY_READ | KEY_WRITE );
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Fermer( HKEY hCle )
 * PARAMETRES: hCle : clé ouverte avec REG_Ouvrir.
 * RETOUR    : ERROR_SUCCESS : la déconnexion a réussi. Autre code
 *             d’erreur, il s’agit d’un code d’erreur standard Win32
 *             dont le texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Se déconnecte d'un registre distant ou ferme le registre
 *             local ouvert à un endroit précis.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Fermer( HKEY hCle )
{
	return RegCloseKey( hCle );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Lire(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      DWORD * pdwType,
 *                      void * pvValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             pcNomValeur: Nom de la valeur à lire dans la cle (si chaine vide,
 *                          lit la cle par defaut).
 *             pdwType    : Pointe sur une variable retournant le type de la
 *                          cle lue.
 *             pvValeur   : Pointe sur le buffer recevant la valeur.
 *             pdwTailleValeur: Pointe sur une variable contenant en
 *                          entree la taille du buffer (en octets)
 *                          et en sortie, la taille occupee dans le buffer
 *                          par la valeur lue.
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) dans la base de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Lire(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	DWORD * pdwType,
	void * pvValeur,
	DWORD * pdwTailleValeur )
{
	HKEY hCle;
	DWORD dwErreur = ERROR_SUCCESS;

	if ( ( dwErreur = RegOpenKeyEx(
				hRacine,
				pcSousCle,
				0,
				KEY_READ,
				&hCle )
		 ) == ERROR_SUCCESS )
 	{
		if ( ( dwErreur = RegQueryValueEx(
					hCle,
					pcNomValeur,
					NULL,
					pdwType,
					pvValeur,
					pdwTailleValeur )
				) == ERROR_SUCCESS )
			dwErreur = RegCloseKey( hCle );
		else
			RegCloseKey( hCle );
	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Lire_Chaine(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      char * pcValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             pcNomValeur: Nom de la valeur à lire dans la cle (si chaine vide,
 *                          lit la cle par defaut).
 *             pcValeur   : Pointe sur le tableau de caractere (chaine) destine
 *                          a recevoir la valeur.
 *             pdwTailleValeur: Pointe sur une variable contenant, en entree,
 *                          la taille maximum de la chaine (en caracteres)
 *                          y compris le caractere de terminaison ('\0'),
 *                          et en sortie, la taille occupee par la chaine lue
 *                          y compris le caractere de terminaison ('\0').
 * RETOUR    : ERROR_SUCCESS si la chaine a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 *             Si le type de la valeur n'est pas REG_SZ ou REG_EXPAND_SZ,
 *             la fonction retourne ERROR_INVALID_DATA.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type REG_SZ (chaine simple)
 *             ou REG_EXPAND_SZ (chaine avec variables d'environnement)
 *             dans la base de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Lire_Chaine(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	char * pcValeur,
	DWORD * pdwTailleValeur )
{
	DWORD dwErreur;
	DWORD dwType;

	if ( ( dwErreur = REG_Lire( 
				hRacine,
				pcSousCle,
				pcNomValeur,
				&dwType,
				pcValeur,
				pdwTailleValeur )
			) == ERROR_SUCCESS )
		dwErreur = ( ( dwType == REG_SZ || dwType == REG_EXPAND_SZ ) ?
						ERROR_SUCCESS :
						ERROR_INVALID_DATA
					);

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Lire_Entier(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      DWORD * pdwValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             pcNomValeur: Nom de la valeur à lire dans la cle (si chaine vide,
 *                          lit la cle par defaut).
 *             pdwValeur  : Pointe sur la variable entiere destinee
 *                          a recevoir la valeur.
 * RETOUR    : ERROR_SUCCESS si la valeur entiere a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 *             Si le type de la valeur n'est pas REG_DWORD,
 *             REG_DWORD_BIG_ENDIAN, ou REG_DWORD_SMALL_ENDIAN, la
 *             fonction retourne ERROR_INVALID_DATA.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type REG_DWORD,
 *             REG_DWORD_BIG_ENDIAN, ou REG_DWORD_SMALL_ENDIAN
 *             dans la base de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Lire_Entier(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	DWORD * pdwValeur )
{
	DWORD dwErreur;
	DWORD dwType;
	DWORD dwTaille;

	dwTaille = sizeof(*pdwValeur);

	if ( ( dwErreur = REG_Lire(
				hRacine,
				pcSousCle,
				pcNomValeur,
				&dwType,
				pdwValeur,
				&dwTaille )
			) == ERROR_SUCCESS )
		dwErreur = ( ( dwType == REG_DWORD || dwType == REG_DWORD_LITTLE_ENDIAN || dwType == REG_DWORD_BIG_ENDIAN ) ?
						ERROR_SUCCESS :
						ERROR_INVALID_DATA );
	else
		if ( dwErreur == ERROR_MORE_DATA )
			dwErreur = ERROR_INVALID_DATA;

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Lire_Donnees(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      void * pvValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             pcNomValeur: Nom de la valeur à lire dans la cle (si chaine vide,
 *                          lit la cle par defaut).
 *             pvValeur   : Pointe sur la zone de memoire destinee a recevoir
 *                          les donnees lues.
 *             pdwTailleValeur: Pointe sur une variable contenant, en entree,
 *                          la taille de la zone pointee par pvValeur,
 *                          et en sortie, la taille occupee par les donnees
 *                          lues.
  * RETOUR    : ERROR_SUCCESS si les donnees ont ete lues correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 *             Si le type de la valeur n'est pas REG_BINARY, ou
 *             REG_NONE, la fonction retourne ERROR_INVALID_DATA.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type REG_BINARY ou REG_NONE
 *             dans la base de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Lire_Donnees(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	void * pvValeur,
	DWORD * pdwTailleValeur )
{
	DWORD dwErreur;
	DWORD dwType;

	if ( ( dwErreur = REG_Lire( 
				hRacine,
				pcSousCle,
				pcNomValeur,
				&dwType,
				pvValeur,
				pdwTailleValeur )
			) == ERROR_SUCCESS )
		dwErreur = ( dwType == REG_BINARY || dwType == REG_NONE ) ? ERROR_SUCCESS : ERROR_INVALID_DATA;

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Enum_Cles(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      DWORD dwIndex,
 *                      char * pcNomCle,
 *                      DWORD * pdwTailleCle )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             dwIndex    : Index de la clé à interroger
 *             pcNomCle   : Pointe sur le buffer recevant le nom de la clé 
 *                          indexée par dwIndex.
 *             pdwTailleCle:Pointe sur l'entier contenant en entrée la taille
 *                          du buffer pointé par pcNomCle et en sortie la
 *                          taille mise à jour dans pcNomCle
 *                          y compris le caractere de terminaison ('\0').
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit le nom d'une clé dans la base
 *             de registre Win32 en se basant sur un index
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Enum_Cles(
                      HKEY hRacine,
                      char * pcSousCle,
                      DWORD dwIndex,
                      char * pcNomCle,
                      DWORD * pdwTailleCle )
{
	HKEY hCle;
	FILETIME sTime;
	DWORD dwErreur = ERROR_SUCCESS;

	if ( ( dwErreur = RegOpenKeyEx(
				hRacine,
				pcSousCle,
				0,
				KEY_READ,
				&hCle )
		 ) == ERROR_SUCCESS )
 	{
		if ( ( dwErreur = RegEnumKeyEx(
					hCle,
					dwIndex,
					pcNomCle,
					pdwTailleCle,
					NULL,
					NULL,
					NULL,
					&sTime )
				) == ERROR_SUCCESS )
			dwErreur = RegCloseKey( hCle );
		else
			RegCloseKey( hCle );
	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Enum_Valeurs(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      DWORD dwIndex,
 *                      char * pcNomValeur,
 *                      DWORD * pdwTailleNom,
 *                      DWORD * pdwType,
 *                      void * pvValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             dwIndex    : Index de la valeur à interroger
 *             pcNomValeur: Pointe sur le buffer recevant le nom de la valeur
 *                          à lire dans la cle.
 *             pdwTailleNom:Pointe sur l'entier contenant en entrée la taille
 *                          du buffer pointé par pcNomValeur et en sortie la
 *                          taille mise à jour dans pcNomValeur
 *                          y compris le caractere de terminaison ('\0').
 *             pdwType    : Pointe sur une variable retournant le type de la
 *                          valeur lue.
 *             pvValeur   : Pointe sur le buffer recevant la valeur.
 *             pdwTailleValeur: Pointe sur une variable contenant en
 *                          entree la taille du buffer (en octets)
 *                          et en sortie, la taille occupee dans le buffer
 *                          par la valeur lue.
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) dans la base de registre Win32
 *             en se basant sur un index
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Enum_Valeurs(
                      HKEY hRacine,
                      char * pcSousCle,
                      DWORD dwIndex,
                      char * pcNomValeur,
                      DWORD * pdwTailleNom,
                      DWORD * pdwType,
                      void * pvValeur,
                      DWORD * pdwTailleValeur )
{
	HKEY hCle;
	DWORD dwErreur = ERROR_SUCCESS;

	if ( ( dwErreur = RegOpenKeyEx(
				hRacine,
				pcSousCle,
				0,
				KEY_READ,
				&hCle )
		 ) == ERROR_SUCCESS )
 	{
		if ( ( dwErreur = RegEnumValue(
					hCle,
					dwIndex,
					pcNomValeur,
					pdwTailleNom,
					NULL,
					pdwType,
					pvValeur,
					pdwTailleValeur )
				) == ERROR_SUCCESS )
			dwErreur = RegCloseKey( hCle );
		else
			RegCloseKey( hCle );
	}

	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Enum_Valeurs_Chaine(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      DWORD dwIndex,
 *                      char * pcNomValeur,
 *                      DWORD * pdwTailleNom,
 *                      char * pcValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             dwIndex    : Index de la valeur à interroger
 *             pcNomValeur: Pointe sur le buffer recevant le nom de la valeur
 *                          à lire dans la cle.
 *             pdwTailleNom:Pointe sur l'entier contenant en entrée la taille
 *                          du buffer pointé par pcNomValeur et en sortie la
 *                          taille mise à jour dans pcNomValeur
 *                          y compris le caractere de terminaison ('\0').
 *             pcValeur   : Pointe sur le buffer recevant la valeur.
 *             pdwTailleValeur: Pointe sur une variable contenant en
 *                          entree la taille du buffer (en octets)
 *                          et en sortie, la taille occupee dans le buffer
 *                          par la valeur lue.
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type SZ, ou EXPAND_SZ dans la base de
 *             registre Win32 en se basant sur un index
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Enum_Valeurs_Chaine(
                      HKEY hRacine,
                      char * pcSousCle,
                      DWORD dwIndex,
                      char * pcNomValeur,
                      DWORD * pdwTailleNom,
                      char * pcValeur,
                      DWORD * pdwTailleValeur )
{
	DWORD dwErreur;
	DWORD dwType;

	if ( ( dwErreur = REG_Enum_Valeurs( 
				hRacine,
				pcSousCle,
				dwIndex,
				pcNomValeur,
				pdwTailleNom,
				&dwType,
				pcValeur,
				pdwTailleValeur )
			) == ERROR_SUCCESS )
		dwErreur = ( ( dwType == REG_SZ || dwType == REG_EXPAND_SZ ) ?
						ERROR_SUCCESS :
						ERROR_INVALID_DATA
					);

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Enum_Valeurs_Entier(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      DWORD dwIndex,
 *                      char * pcNomValeur,
 *                      DWORD * pdwTailleNom,
 *                      DWORD * pdwValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             dwIndex    : Index de la valeur à interroger
 *             pcNomValeur: Pointe sur le buffer recevant le nom de la valeur
 *                          à lire dans la cle.
 *             pdwTailleNom:Pointe sur l'entier contenant en entrée la taille
 *                          du buffer pointé par pcNomValeur et en sortie la
 *                          taille mise à jour dans pcNomValeur
 *                          y compris le caractere de terminaison ('\0').
 *             pdwValeur  : Valeur entiére associée à la clé
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type entier dans la base 
 *             de registre Win32 en se basant sur un index
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Enum_Valeurs_Entier(
                      HKEY hRacine,
                      char * pcSousCle,
                      DWORD dwIndex,
                      char * pcNomValeur,
                      DWORD * pdwTailleNom,
                      DWORD * pdwValeur )
{
	DWORD dwErreur;
	DWORD dwType;
	DWORD dwTaille;

	dwTaille = sizeof(*pdwValeur);

	if ( ( dwErreur = REG_Enum_Valeurs(
				hRacine,
				pcSousCle,
				dwIndex,
				pcNomValeur,
				pdwTailleNom,
				&dwType,
				pdwValeur,
				&dwTaille )
			) == ERROR_SUCCESS )
		dwErreur = ( ( dwType == REG_DWORD || dwType == REG_DWORD_LITTLE_ENDIAN || dwType == REG_DWORD_BIG_ENDIAN ) ?
						ERROR_SUCCESS :
						ERROR_INVALID_DATA );
	else
		if ( dwErreur == ERROR_MORE_DATA )
			dwErreur = ERROR_INVALID_DATA;


	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Enum_Valeurs_Donnees(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      DWORD dwIndex,
 *                      char * pcNomValeur,
 *                      DWORD * pdwTailleNom,
 *                      void * pvValeur,
 *                      DWORD * pdwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en lecture sur cette cle.
 *             dwIndex    : Index de la valeur à interroger
 *             pcNomValeur: Pointe sur le buffer recevant le nom de la valeur
 *                          à lire dans la cle.
 *             pdwTailleNom:Pointe sur l'entier contenant en entrée la taille
 *                          du buffer pointé par pcNomValeur et en sortie la
 *                          taille mise à jour dans pcNomValeur
 *                          y compris le caractere de terminaison ('\0').
 *             pvValeur   : Pointe sur le buffer recevant la valeur.
 *             pdwTailleValeur: Pointe sur une variable contenant en
 *                          entree la taille du buffer (en octets)
 *                          et en sortie, la taille occupee dans le buffer
 *                          par la valeur lue.
 * RETOUR    : ERROR_SUCCESS si la valeur a ete lue correctement.
 *             Un code d'erreur type Win32 (cf winerror.h)
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit une entree (valeur) de type DATA ou NONE dans la base
 *             de registre Win32 en se basant sur un index
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Enum_Valeurs_Donnees(
                      HKEY hRacine,
                      char * pcSousCle,
                      DWORD dwIndex,
                      char * pcNomValeur,
                      DWORD * pdwTailleNom,
                      void * pvValeur,
                      DWORD * pdwTailleValeur )
{
	DWORD dwErreur;
	DWORD dwType;

	if ( ( dwErreur = REG_Enum_Valeurs( 
				hRacine,
				pcSousCle,
				dwIndex,
				pcNomValeur,
				pdwTailleNom,
				&dwType,
				pvValeur,
				pdwTailleValeur )
			) == ERROR_SUCCESS )
		dwErreur = ( dwType == REG_BINARY || dwType == REG_NONE ) ? ERROR_SUCCESS : ERROR_INVALID_DATA;

	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Ecrire(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      DWORD dwType,
 *                      void * pvValeur,
 *                      DWORD dwTailleValeur )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine. Si la cle n'existe pas, elle
 *                          est cree.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en ecriture sur cette cle.
 *             pcNomValeur: Nom de la valeur à creer/modifier dans la cle
 *                          Si chaine vide, met a jour la cle par defaut.
 *                          Si NULL, seule la clé est créée.
 *             dwType     : Type de la cle a stocker. Sans importance si
 *                          pcNomValeur est NULL.
 *             pvValeur   : Pointe sur la zone de memoire contenant les
 *                          donnees a ecrire.  Sans importance si
 *                          pcNomValeur est NULL.
 *             dwTailleValeur: Taille de la zone pointee par pvValeur.
 *                          Sans importance si pcNomValeur est NULL.
 * RETOUR    : ERROR_SUCCESS si les donnees ont ete ecrites correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrit une entree (valeur) dans la base de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ecrire(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	DWORD dwType,
	void * pvValeur,
	DWORD dwTailleValeur )
{
	HKEY hCle;
	DWORD dwErreur = ERROR_SUCCESS;
	DWORD dwDispo;

    if ( pcSousCle != NULL )
    {
        dwErreur = RegCreateKeyEx(
				hRacine,
				pcSousCle,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_READ|KEY_WRITE,
				NULL,
				&hCle,
				&dwDispo );
    }
    else
    {
        dwErreur = RegOpenKeyEx(
				hRacine,
				pcSousCle,
				0,
				KEY_READ|KEY_WRITE,
				&hCle );
    }

	if ( dwErreur == ERROR_SUCCESS )
	{
		if ( pcNomValeur != NULL )
		{
			if ( ( dwErreur = RegSetValueEx(
						hCle,
						pcNomValeur,
						0,
						dwType,
						pvValeur,
						dwTailleValeur )
					) == ERROR_SUCCESS )
				dwErreur = RegCloseKey( hCle );
			else
				RegCloseKey( hCle );
		}
		else
			dwErreur = RegCloseKey( hCle );
	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Ecrire_Chaine(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      char * pcValeur)
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine. Si la cle n'existe pas, elle
 *                          est cree.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en ecriture sur cette cle.
 *             pcNomValeur: Nom de la valeur à creer/modifier dans la cle
 *                          (si chaine vide, met a jour la cle par defaut).
 *             pcValeur   : Pointe sur la chaine a ecrire
  * RETOUR    : ERROR_SUCCESS si la chaine a ete ecrite correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrit une chaine (valeur de type REG_SZ) dans la base de
 *             registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ecrire_Chaine(
	HKEY hRacine,
	char * pcSousCle,
	char *pcNomValeur,
	char * pcValeur)
{
	return REG_Ecrire(
				hRacine,
				pcSousCle,
				pcNomValeur,
				REG_SZ,
				pcValeur,
				(DWORD)strlen(pcValeur)+1 );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Ecrire_Entier(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      char * pcNomValeur,
 *                      DWORD dwValeur)
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine. Si la cle n'existe pas, elle
 *                          est cree.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en ecriture sur cette cle.
 *             pcNomValeur: Nom de la valeur à creer/modifier dans la cle
 *                          (si chaine vide, met a jour la cle par defaut).
 *             dwValeur   : Entier a ecrire
  * RETOUR    : ERROR_SUCCESS si l'entier a ete ecrit correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrit un entier (valeur de type REG_DWORD) dans la base
 *             de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ecrire_Entier(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	DWORD dwValeur)
{
	return REG_Ecrire(
				hRacine,
				pcSousCle,
				pcNomValeur,
				REG_DWORD,
				&dwValeur,
				sizeof(DWORD) );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : REG_Ecrire_Donnees(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      pcNomValeur,
 *                      pvValeur,
 *                      dwTailleValeur)
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine. Si la cle n'existe pas, elle
 *                          est cree.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits en ecriture sur cette cle.
 *             pcNomValeur: Nom de la valeur à creer/modifier dans la cle
 *                          (si chaine vide, met a jour la cle par defaut).
 *             pvValeur   : Pointe sur les donnees a ecrire
 *             dwTailleValeur: Taille de la zone pointee par pvValeur.
 * RETOUR    : ERROR_SUCCESS si la chaine a ete ecrite correctement.
 *             Un code d'erreur type Win32 (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrit des donnees (valeur de type REG_BINARY) dans la base
 *             de registre Win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Ecrire_Donnees(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur,
	void * pvValeur,
	DWORD dwTailleValeur )
{
	return REG_Ecrire(
		hRacine,
		pcSousCle,
		pcNomValeur,
		REG_BINARY,
		pvValeur,
		dwTailleValeur );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Defaut(
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      char * pcNomValeur,
 *      DWORD * pdwType,
 *      void * pvValeurLue,
 *      DWORD * pdwTailleValeurLue,
 *      void * pvValeurDefaut,
 *      DWORD dwTailleValeurDefaut )
 * PARAMETRES: hRacine : handle d'une clé deja ouverte (avec REG_Ouvrir
 *                 par exemple) ou handle prédéfini d’une entrée
 *                 de registre :
 *                   HKEY_CLASSES_ROOT, HKEY_CURRENT_USER,
 *                   HKEY_LOCAL_MACHINE, HKEY_USERS
 *                 dans ce dernier cas, REG_Ouvrir est inutile, mais
 *                 les performances plus faibles.
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le chemin de la clé (depuis hRacine)
 *                 sous la forme "cle\sous-cle\sous-sous-cle" (attention,
 *                 en C, "\" devient "\\"). Peut être une chaîne vide ou
 *                 NULL si hRacine a été obtenu avec REG_Ouvrir.
 *                 Attention, l'utilisateur associe au process doit avoir
 *                 les droits en lecture et en écriture sur cette clé.
 *             pcNomValeur : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le nom de la valeur à lire ou à
 *                 créer dans la clé (si chaîne vide ou NULL, lit la clé
 *                 par défaut).
 *             pdwType : pointe sur une variable contenant en entrée le
 *                 type de la valeur par défaut et retournant le type de
 *                 la clé lue ou écrite : REG_BINARY, REG_DWORD,
 *                 REG_DWORD_LITTLE_ENDIAN, REG_DWORD_BIG_ENDIAN,
 *                 REG_EXPAND_SZ, REG_LINK, REG_MULTI_SZ, REG_NONE,
 *                 REG_RESOURCE_LIST, REG_SZ
 *             pvValeurLue : pointe sur la zone de mémoire destinée à
 *                 stocker la valeur lue (si trouvée), la valeur par défaut
 *                 si créée.
 *             pdwTailleValeurLue : pointe sur une variable contenant en
 *                 entrée la taille de la zone pointée par pvValeurLue (en
 *                 octets) et en sortie, la taille occupée dans cette zone
 *                 par la valeur.
 *             pvValeurDefaut : pointe sur la zone de mémoire où se trouve
 *                 la valeur par défaut.
 *             dwTailleValeurDefaut : taille de la zone pointée par
 *                 pvValeurDefaut.
 * RETOUR    : ERROR_SUCCESS : la lecture/création a réussie.
 * Autre code d’erreur, il s’agit d’un code d’erreur standard Win32 dont le
 * texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit le contenu d’une valeur dans une clé donnée sans type
 *             prédéfini, ou effectue la création
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Defaut(
      HKEY hRacine,
      char * pcSousCle,
      char * pcNomValeur,
      DWORD * pdwType,
      void * pvValeurLue,
      DWORD * pdwTailleValeurLue,
      void * pvValeurDefaut,
      DWORD dwTailleValeurDefaut )
{
	DWORD dwErreur = ERROR_SUCCESS;
	DWORD dwType, dwTaille;

	if ( (*pdwTailleValeurLue) >= dwTailleValeurDefaut )
	{
		dwTaille = *pdwTailleValeurLue;
		dwErreur = REG_Lire( hRacine, pcSousCle, pcNomValeur, &dwType, pvValeurLue, &dwTaille );

		if ( dwErreur == ERROR_SUCCESS )
		{
			(*pdwTailleValeurLue) = dwTaille;
			(*pdwType) = dwType;
		}
		else if ( dwErreur == ERROR_FILE_NOT_FOUND )
		{
			dwErreur = REG_Ecrire( hRacine, pcSousCle, pcNomValeur, *pdwType, pvValeurDefaut, dwTailleValeurDefaut );
			if ( dwErreur == ERROR_SUCCESS )
			{
				memcpy( pvValeurLue, pvValeurDefaut, dwTailleValeurDefaut );
				(*pdwTailleValeurLue) = dwTailleValeurDefaut;
			}
		}
	}
	else
		dwErreur = ERROR_INVALID_PARAMETER;

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Defaut_Chaîne(
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      char * pcNomValeur,
 *      char * pcValeurLue,
 *      DWORD * pdwTailleValeurLue,
 *      char * pcValeurDefaut )
 * PARAMETRES: hRacine : handle d'une clé deja ouverte (avec REG_Ouvrir
 *                 par exemple) ou handle prédéfini d’une entrée
 *                 de registre :
 *                   HKEY_CLASSES_ROOT, HKEY_CURRENT_USER,
 *                   HKEY_LOCAL_MACHINE, HKEY_USERS
 *                 dans ce dernier cas, REG_Ouvrir est inutile, mais
 *                 les performances plus faibles.
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le chemin de la clé (depuis hRacine)
 *                 sous la forme "cle\sous-cle\sous-sous-cle" (attention,
 *                 en C, "\" devient "\\"). Peut être une chaîne vide ou
 *                 NULL si hRacine a été obtenu avec REG_Ouvrir.
 *                 Attention, l'utilisateur associe au process doit avoir
 *                 les droits en lecture et en écriture sur cette clé.
 *             pcNomValeur : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le nom de la valeur à lire ou à
 *                 créer dans la clé (si chaîne vide ou NULL, lit la clé
 *                 par défaut).
 *             pcValeurLue : pointe sur le tableau de caractere (chaîne)
 *                 destiné à recevoir la valeur lue / créée.
 *             pdwTailleValeurLue : pointe sur une variable contenant,
 *                 en entrée la taille maximum de la zone pointée par
 *                 pcValeurLue (y compris le caractere de terminaison '\0'),
 *                 et en sortie, la taille occupée par la valeur lue (y
 *                 compris le caractère de terminaison '\0').
 *             pcValeurDefaut : pointe sur une chaîne de caractères
 *                 terminée par un ‘\0’ constituant la chaîne par défaut.
 * RETOUR    : ERROR_SUCCESS : la lecture / création a réussi.
 *             Autre code d’erreur, il s’agit d’un code d’erreur standard
 *             Win32 dont le texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit le contenu d’une valeur dans une clé donnée de type
 *             REG_SZ ou REG_EXPAND_SZ, ou effectue la création.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Defaut_Chaine(
      HKEY hRacine,
      char * pcSousCle,
      char * pcNomValeur,
      char * pcValeurLue,
      DWORD * pdwTailleValeurLue,
      char * pcValeurDefaut )
{
	DWORD dwErreur = ERROR_SUCCESS;
	DWORD dwTaille;

	if ( (*pdwTailleValeurLue) >= ( strlen(pcValeurDefaut) + 1 ) )
	{
		dwTaille = *pdwTailleValeurLue;
		dwErreur = REG_Lire_Chaine( hRacine, pcSousCle, pcNomValeur, pcValeurLue, &dwTaille );

		if ( dwErreur == ERROR_SUCCESS )
			(*pdwTailleValeurLue) = dwTaille;
		else if ( dwErreur == ERROR_FILE_NOT_FOUND )
		{
			dwErreur = REG_Ecrire_Chaine( hRacine, pcSousCle, pcNomValeur, pcValeurDefaut );
			if ( dwErreur == ERROR_SUCCESS )
			{
				strcpy_s( pcValeurLue, dwTaille, pcValeurDefaut );
				(*pdwTailleValeurLue) = (DWORD)strlen(pcValeurDefaut) + 1;
			}
		}
	}
	else
		dwErreur = ERROR_INVALID_PARAMETER;

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Defaut_Entier(
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      char * pcNomValeur,
 *      DWORD * pdwValeurLue,
 *      DWORD dwValeurDefaut )
 * PARAMETRES: hRacine : handle d'une clé deja ouverte (avec REG_Ouvrir
 *                 par exemple) ou handle prédéfini d’une entrée
 *                 de registre :
 *                   HKEY_CLASSES_ROOT, HKEY_CURRENT_USER,
 *                   HKEY_LOCAL_MACHINE, HKEY_USERS
 *                 dans ce dernier cas, REG_Ouvrir est inutile, mais
 *                 les performances plus faibles.
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le chemin de la clé (depuis hRacine)
 *                 sous la forme "cle\sous-cle\sous-sous-cle" (attention,
 *                 en C, "\" devient "\\"). Peut être une chaîne vide ou
 *                 NULL si hRacine a été obtenu avec REG_Ouvrir.
 *                 Attention, l'utilisateur associe au process doit avoir
 *                 les droits en lecture et en écriture sur cette clé.
 *             pcNomValeur : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le nom de la valeur à lire ou à
 *                 créer dans la clé (si chaîne vide ou NULL, lit la clé
 *                 par défaut).
 *             pdwTailleValeurLue : pointe sur une variable contenant, en
 *                 entrée la taille maximum de la zone pointée par
 *                 pcValeurLue (y compris le caractere de terminaison '\0')
 *                 ou pvValeurLue, et en sortie, la taille occupée par la
 *                 valeur lue (y compris le caractère de terminaison '\0'
 *                 pour pcValeurLue).
 *             dwValeurDefaut : valeur entière par défaut.
 * RETOUR    : ERROR_SUCCESS : la lecture / création a réussi.
 *             Autre code d’erreur, il s’agit d’un code d’erreur standard
 *             Win32 dont le texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit le contenu d’une valeur dans une clé donnée de type
 *             REG_DWORD ou REG_DWORD_XXX, ou effectue la création.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Defaut_Entier(
      HKEY hRacine,
      char * pcSousCle,
      char * pcNomValeur,
      DWORD * pdwValeurLue,
      DWORD dwValeurDefaut )
{
	DWORD dwErreur = ERROR_SUCCESS;

	dwErreur = REG_Lire_Entier( hRacine, pcSousCle, pcNomValeur, pdwValeurLue );

	if ( dwErreur == ERROR_FILE_NOT_FOUND )
	{
		dwErreur = REG_Ecrire_Entier( hRacine, pcSousCle, pcNomValeur, dwValeurDefaut );
		if ( dwErreur == ERROR_SUCCESS )
			(*pdwValeurLue) = dwValeurDefaut;
	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI REG_Defaut_Donnees(
 *      HKEY hRacine,
 *      char * pcSousCle,
 *      char * pcNomValeur,
 *      void * pvValeurLue,
 *      DWORD * pdwTailleValeurLue,
 *      void * pvValeurDefaut,
 *      DWORD dwTailleValeurDefaut )
 * PARAMETRES: hRacine : handle d'une clé deja ouverte (avec REG_Ouvrir
 *                 par exemple) ou handle prédéfini d’une entrée
 *                 de registre :
 *                   HKEY_CLASSES_ROOT, HKEY_CURRENT_USER,
 *                   HKEY_LOCAL_MACHINE, HKEY_USERS
 *                 dans ce dernier cas, REG_Ouvrir est inutile, mais
 *                 les performances plus faibles.
 *             pcSousCle : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le chemin de la clé (depuis hRacine)
 *                 sous la forme "cle\sous-cle\sous-sous-cle" (attention,
 *                 en C, "\" devient "\\"). Peut être une chaîne vide ou
 *                 NULL si hRacine a été obtenu avec REG_Ouvrir.
 *                 Attention, l'utilisateur associe au process doit avoir
 *                 les droits en lecture et en écriture sur cette clé.
 *             pcNomValeur : pointe sur une chaîne de caractères terminée
 *                 par un ‘\0’ donnant le nom de la valeur à lire ou à
 *                 créer dans la clé (si chaîne vide ou NULL, lit la clé
 *                 par défaut).
 *             pvValeurLue : pointe sur la zone de memoire destinee a
 *                 recevoir les donnees lues / créées.
 *             pdwTailleValeurLue : pointe sur une variable contenant,
 *                 en entrée la taille maximum de la zone pointée par
 *                 pvValeurLue, et en sortie, la taille occupée
 *                 par la valeur lue.
 *             pvValeurDefaut : pointe sur la zone de mémoire où se
 *                 trouvent les données par défaut.
 *             dwTailleValeurDefaut : taille de la zone pointée par
 *                 pvValeurDefaut.
  * RETOUR    : ERROR_SUCCESS : la lecture / création a réussi.
 *             Autre code d’erreur, il s’agit d’un code d’erreur standard
 *             Win32 dont le texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Lit le contenu d’une valeur dans une clé donnée de type
 *             REG_BINARY ou REG_NONE, ou effectue la création.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Defaut_Donnees(
      HKEY hRacine,
      char * pcSousCle,
      char * pcNomValeur,
      void * pvValeurLue,
      DWORD * pdwTailleValeurLue,
      void * pvValeurDefaut,
      DWORD dwTailleValeurDefaut )
{
	DWORD dwErreur = ERROR_SUCCESS;
	DWORD dwTaille;

	if ( (*pdwTailleValeurLue) >= dwTailleValeurDefaut )
	{
		dwTaille = *pdwTailleValeurLue;
		dwErreur = REG_Lire_Donnees( hRacine, pcSousCle, pcNomValeur, pvValeurLue, &dwTaille );

		if ( dwErreur == ERROR_SUCCESS )
			(*pdwTailleValeurLue) = dwTaille;
		else if ( dwErreur == ERROR_FILE_NOT_FOUND )
		{
			dwErreur = REG_Ecrire_Donnees( hRacine, pcSousCle, pcNomValeur, pvValeurDefaut, dwTailleValeurDefaut );
			if ( dwErreur == ERROR_SUCCESS )
			{
				memcpy( pvValeurLue, pvValeurDefaut, dwTailleValeurDefaut );
				(*pdwTailleValeurLue) = dwTailleValeurDefaut;
			}
		}
	}
	else
		dwErreur = ERROR_INVALID_PARAMETER;

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : REG_Efface_Cle(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      pcNomValeur)
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits pour effacer cette cle.
 *             pcNomValeur: Nom de la valeur à effacer. Si NULL, efface pcSousCle.
 *                          Si non NULL, seule la valeur pcNomValeur est effacee,
 *                          et pcSousCle est conservee.
 * RETOUR    : ERROR_SUCCESS si effacement effectue.
 *             Un code d'erreur type Win32 (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Efface une cle / valeur de la base de registre Win32
 *             Si des sous-cles sont présentes, elles sont également effacées
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Efface_Cle(
	HKEY hRacine,
	char * pcSousCle,
	char * pcNomValeur )
{
	HKEY hCle;
	DWORD dwErreur = ERROR_SUCCESS;
	FILETIME sFt;
	DWORD dwSize;
	char pcNom[MAX_PATH+1];
	
	if ( ( dwErreur = RegOpenKeyEx(
				hRacine,
				pcSousCle,
				0,
				KEY_READ|KEY_WRITE,
				&hCle )
			) == ERROR_SUCCESS )
	{
		if ( pcNomValeur != NULL )
		{
			if ( ( dwErreur = RegDeleteValue( hCle, pcNomValeur ) ) == ERROR_SUCCESS )
				dwErreur = RegFlushKey( hCle );

			if ( dwErreur == ERROR_SUCCESS )
				dwErreur = RegCloseKey( hCle );
			else
				RegCloseKey( hCle );
		}
		else
		{
			while (TRUE)
			{
				// Appel recursif pour toutes les sous-cles
				// On repart a la sous-cle 0 a chaque fois, car elles sont
				// reindexees chaque fois que l'une d'elles est effacee
				dwSize = sizeof(pcNom);
				if ( ( dwErreur = RegEnumKeyEx(
							hCle,
							0,
							pcNom,
							&dwSize,
							NULL,
							NULL,
							NULL,
							&sFt ) ) != ERROR_SUCCESS )
					break;
				if ( ( dwErreur = REG_Efface_Cle( hCle, pcNom, NULL ) ) != ERROR_SUCCESS )
					break;
			}

			if ( dwErreur == ERROR_NO_MORE_ITEMS )
				dwErreur = RegCloseKey( hCle );
			else
				RegCloseKey( hCle );

			if ( dwErreur == ERROR_SUCCESS )
				dwErreur = RegDeleteKey( hRacine, pcSousCle );

		}

	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : REG_Attendre_Changement(
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *	                    BOOL bSousArbre,
 *                      DWORD dwDelai )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits de notification sur cette clé.
 *             bSousArbre : Si TRUE, la notification comprend également toutes
 *                          les sous-clés.
 *             dwDelai    : Delai imparti en ms au dela duquel la fonction
 *                          stoppe son attente. INFINITE pour une attente
 *                          infinie.
 * RETOUR    : ERROR_SUCCESS si un changement a eu lieu. ERROR_TIMEOUT en cas
 *             de dépassement de delai.
 *             Un code d'erreur type Win32 sinon (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Attend qu'une des valeurs d'une clé change (comprend :
 *             modification, effacement, suppression).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Attendre_Changement(
	HKEY hRacine,
	char * pcSousCle,
	BOOL bSousArbre,
	DWORD dwDelai )
{
	HKEY hCle;
	HANDLE hEvent;
	DWORD dwErreur = ERROR_SUCCESS;
	DWORD dwWait;

	// Creer l'evenement qui va permettre de limiter l'attende sur la cle
	if ( ( hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) != NULL )
	{
		// L'evenement a ete cree

		if ( ( dwErreur = RegOpenKeyEx(
					hRacine,
					pcSousCle,
					0,
					KEY_READ,
					&hCle )
				) == ERROR_SUCCESS )
		{
			if ( ( dwErreur = RegNotifyChangeKeyValue(
					hCle,
					bSousArbre,
					REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_NAME,
					hEvent,
					TRUE )
				 ) == ERROR_SUCCESS )
			{
				dwWait = WaitForSingleObject( hEvent, dwDelai );
				if ( dwWait != WAIT_OBJECT_0 )
					if ( dwWait != WAIT_TIMEOUT )
						dwErreur = GetLastError();
					else
						dwErreur = ERROR_TIMEOUT;
				else
					dwErreur = ERROR_SUCCESS;
			}

			if ( dwErreur == ERROR_SUCCESS )
				dwErreur = RegCloseKey( hCle );
			else
				RegCloseKey( hCle );
		}
		if ( ! CloseHandle( hEvent ) )
			if ( dwErreur == ERROR_SUCCESS )
				dwErreur = GetLastError();
	}
	else
		dwErreur = GetLastError();

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE REG_Debut_Acces_Exclusif( char * pcJeton, DWORD dwDelai )
 * PARAMETRES: pcJeton : nom du jeton d'acces
 * RETOUR    : NULL si erreur, sinon, l'acces est obtenu. Si erreur,
 *             GetLastError() identifie l'erreur. ERROR_TIMEOUT sur depassement
 *             de delai.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Obtient un acces exclusif
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT HANDLE WINAPI REG_Debut_Acces_Exclusif( char * pcJeton, DWORD dwDelai )
{
	DWORD dwRes;
	DWORD dwErreur = ERROR_SUCCESS;
	HANDLE hJeton;
	DWORD dwIdx;
	char pcAcces[MAX_PATH+1] = REG_ACCESS_HEADER;

	strncat_s( pcAcces, MAX_PATH+1, pcJeton, sizeof(pcAcces) );
	pcAcces[sizeof(pcAcces)-1] = '\0';

	for ( dwIdx = 0 ; pcAcces[dwIdx] != '\0'; dwIdx ++ )
		if ( ! isalnum( pcAcces[dwIdx] ) )
			pcAcces[dwIdx] = '_';
		else
			pcAcces[dwIdx] = toupper(pcAcces[dwIdx]);
	
	if ( ( hJeton = CreateMutex( NULL, FALSE, pcAcces ) ) != NULL )
	{
		dwRes = WaitForSingleObject( hJeton, dwDelai );
		switch ( dwRes )
		{
		case WAIT_TIMEOUT :
			dwErreur = ERROR_TIMEOUT;
			CloseHandle( hJeton );
			hJeton = NULL;
			break;
		case WAIT_FAILED :
			dwErreur = GetLastError();
			CloseHandle( hJeton );
			hJeton = NULL;
			break;
		case WAIT_ABANDONED :
		case WAIT_OBJECT_0 :
			dwErreur = ERROR_SUCCESS;
			break;
		default :
			dwErreur = ERROR_INVALID_DATA;
			CloseHandle( hJeton );
			hJeton = NULL;
			break;
		}
	}
	else
		dwErreur = GetLastError();

	SetLastError( dwErreur );

	return hJeton;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Fin_Acces_Exclusif( HANDLE hJeton )
 * PARAMETRES: hJeton : Handle de l'acces.
 * RETOUR    : ERROR_SUCCESS si acces libere, un code Win32 sinon.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ferme un acces exclusif
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Fin_Acces_Exclusif( HANDLE hJeton )
{

	DWORD dwErreur = ERROR_SUCCESS;

	if ( ReleaseMutex( hJeton ) )
		if ( CloseHandle( hJeton ) )
			dwErreur = ERROR_SUCCESS;
		else
			dwErreur = GetLastError();
	else
	{
		dwErreur = GetLastError();
		CloseHandle( hJeton );
	}

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI REG_Demarrer_Notification( 
 *                      HKEY hRacine,
 *                      char * pcSousCle,
 *                      BOOL bSousArbre,
 *                      HANDLE * phNotification )
 * PARAMETRES: hRacine    : handle d'une cle deja ouverte (avec RegOpenKeyEx
 *                          par exemple) ou constante parmi HKEY_CLASSES_ROOT,
 *                          HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE ou HKEY_USERS 
 *             pcSousCle  : "chemin d'acces" a la cle sous la forme d'une
 *                          chaine : "cle\souscle\soussouscle". Si chaine
 *                          vide, se base sur la cle correspondant au
 *                          handle hRacine.
 *                          Attention, l'utilisateur associe au process doit
 *                          avoir les droits de notification sur cette clé.
 *             bSousArbre : Si TRUE, la notification comprend également toutes
 *                          les sous-clés.
 *             phNotification : Récupère un handle à utiliser pour les notification
 * RETOUR    : ERROR_SUCCESS si la notification est démarée.
 *             Un code d'erreur type Win32 sinon (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Prépare le test des changement opérés dans une clé de registre
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Demarrer_Notification( 
	    HKEY hRacine,
	    char * pcSousCle,
	    BOOL bSousArbre,
        HANDLE * phNotification )
{
    DWORD dwErr = NO_ERROR;
    REG_NOTIFICATION * psNot = NULL;

    __try
    {
        psNot =  HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psNot) );
        if ( psNot == NULL )            { dwErr = ERROR_NOT_ENOUGH_MEMORY; __leave; }
    
        psNot->bSubTree = bSousArbre;

        psNot->hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	    if ( psNot->hEvent == NULL )    { dwErr = GetLastError(); __leave; }

		dwErr = RegOpenKeyEx(
			hRacine,
			pcSousCle,
			0,
			KEY_READ,
			&psNot->hKey );
        if ( dwErr != ERROR_SUCCESS )   __leave;

        dwErr = RegNotifyChangeKeyValue(
			psNot->hKey,
			psNot->bSubTree,
			REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_NAME,
			psNot->hEvent,
			TRUE );
        if ( dwErr != ERROR_SUCCESS )   __leave;
    }
    __finally
    {
        if ( dwErr != ERROR_SUCCESS )
        {
            if ( psNot != NULL )
            {
                if ( psNot->hEvent != NULL ) CloseHandle( psNot->hEvent );
                if ( psNot->hKey != NULL ) RegCloseKey( psNot->hKey );
                HeapFree( GetProcessHeap(), 0, psNot );
            }
            psNot = NULL;
        }

        (*phNotification) = (HANDLE)psNot;
//        return dwErr;
    }

	return dwErr;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT BOOL WINAPI REG_Tester_Notification( 
 *                      HANDLE hNotificiation,
 *                      DWORD dwDelaiMs )
 * PARAMETRES: hNotification : Handle obtenu avec REG_Demarrer_Notification
 *             dwDelaiMs     : Delai d'attente de la notification. Si 0, la fonction
 *                             ne fait qu'un test.
 * RETOUR    : TRUE si un changement a eu lieu. FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Test/attend un changement dans une clé. Il suffit de réappeler
 *             REG_Tester_Notification en boucle pour tester les changements
 *             suivants.
 * REMARQUE  : Si un changement est survenu, l'acquitement est effectué
 *             automatiquement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI REG_Tester_Notification( 
	    HANDLE hNotificiation,
        DWORD dwDelaiMs )
{
    DWORD dwResult;
    BOOL bReturn = FALSE;
    REG_NOTIFICATION * psNot = hNotificiation;

    dwResult = WaitForSingleObject( psNot->hEvent, dwDelaiMs );
    if ( dwResult == WAIT_OBJECT_0 )
        RegNotifyChangeKeyValue(
			psNot->hKey,
			psNot->bSubTree,
			REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_NAME,
			psNot->hEvent,
			TRUE );
    return ( dwResult == WAIT_OBJECT_0 );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT HANDLE WINAPI REG_Evt_Notification( 
 *                      HANDLE hNotificiation )
 * PARAMETRES: hNotification : Handle obtenu avec REG_Demarrer_Notification
 * RETOUR    : Un handle d'événement passant à l'état signalé lorsqu'une
 *             notification survient. Il faut ensuite appeler REG_Acquite_Notification()
 *             pour acquiter la notification
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Obtenir un handle d'événememt associé à la notification.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT HANDLE WINAPI REG_Evt_Notification( 
	    HANDLE hNotificiation )
{
    REG_NOTIFICATION * psNot = hNotificiation;
    return psNot->hEvent;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI REG_Acquiter_Notification( 
 *                      HANDLE hNotificiation )
 * PARAMETRES: hNotification : Handle obtenu avec REG_Demarrer_Notification
 * RETOUR    : ERROR_SUCCESS si la notification est démarée.
 *             Un code d'erreur type Win32 sinon (cf winerror.h).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Acquiter une notification.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Acquiter_Notification( 
	    HANDLE hNotificiation )
{
    REG_NOTIFICATION * psNot = hNotificiation;

    return RegNotifyChangeKeyValue(
		psNot->hKey,
		psNot->bSubTree,
		REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_NAME,
		psNot->hEvent,
		TRUE );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI REG_Terminer_Notification( HANDLE hNotificiation )
 * PARAMETRES: hNotification : Handle obtenu avec REG_Demarrer_Notification
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Arréte une notification lancée avec REG_Demarrer_Notification.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI REG_Terminer_Notification( HANDLE hNotificiation )
{
    REG_NOTIFICATION * psNot = hNotificiation;
    if ( psNot->hEvent != NULL ) CloseHandle( psNot->hEvent );
    if ( psNot->hKey != NULL ) RegCloseKey( psNot->hKey );
    HeapFree( GetProcessHeap(), 0, psNot );
}




/*---------------------------- FIN DU FICHIER -------------------------*/


