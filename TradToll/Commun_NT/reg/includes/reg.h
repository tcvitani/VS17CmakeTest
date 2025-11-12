/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : REG (REG)
 * FICHIER    : REG.H
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



/*---------------------------- DEFINITION DU TYPE DE LIBRAIRIE -------------------------*/

#ifndef REG_H
#define REG_H

// Definition des modificateur déclaratifs (PUBLIC etc.)
#ifdef REG_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


// Pour pouvoir utiliser les types DWORD, HKEY et les
// fonctions RegXXXX. et la macro WINAPI. Windows.H vérifiant
// qu'il n'a pas déjà été inclus, on peut l'insérer sans pb.
#include <windows.h>


/*---------------------------- PROTOTYPES -------------------------*/


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
       REGSAM dwReqAccess );

       
       
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
       HKEY * phCle );



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
EXPORT DWORD WINAPI REG_Fermer( HKEY hCle );



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
	DWORD * pdwTailleValeur );



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
	DWORD * pdwTailleValeur );



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
	DWORD * pdwValeur );



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
	DWORD * pdwTailleValeur );



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
                      DWORD * pdwTailleCle );

	
	
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
                      DWORD * pdwTailleValeur );

	

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
                      DWORD * pdwTailleValeur );
					  


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
                      DWORD * pdwValeur );

					  

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
 *                          (si chaine vide, met a jour la cle par defaut).
 *             dwType     : Type de la cle a stocker.
 *             pvValeur   : Pointe sur la zone de memoire contenant les
 *                          donnees a ecrire.
 *             dwTailleValeur: Taille de la zone pointee par pvValeur.
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
	DWORD dwTailleValeur );



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
	char * pcValeur);



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
	DWORD dwValeur);



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
	DWORD dwTailleValeur );



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
      DWORD dwTailleValeurDefaut );




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
      char * pcValeurDefaut );



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
      DWORD dwValeurDefaut );



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
      DWORD dwTailleValeurDefaut );



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
	char * pcNomValeur );



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
 *                          stoppe son attente.
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
	DWORD dwDelai);



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
EXPORT HANDLE WINAPI REG_Debut_Acces_Exclusif( char * pcJeton, DWORD dwDelai );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD REG_Fin_Acces_Exclusif( HANDLE hJeton )
 * PARAMETRES: hJeton : Handle de l'acces.
 * RETOUR    : ERROR_SUCCES si acces libere, un code Win32 sinon.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ferme un acces exclusif
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI REG_Fin_Acces_Exclusif( HANDLE hJeton );



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
        HANDLE * phNotification );



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
        DWORD dwDelaiMs );



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
	    HANDLE hNotificiation );



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
	    HANDLE hNotificiation );

        

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
EXPORT void WINAPI REG_Terminer_Notification( HANDLE hNotificiation );


#endif

/*---------------------------- FIN DU FICHIER -------------------------*/
