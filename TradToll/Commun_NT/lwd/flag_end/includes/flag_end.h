/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : LWD (Lauch/Watch Dog)
 * FICHIER    : FLAG_END.H
 * LANGAGE    : C (VC++ 6.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : process, program
 * -----------------------------------------------------------------
 * RESUME     : Lanceur et surveillance d'application (de programmes)
 * --------------------------------------------------------------------
 * DESCRIPTION: Ce programme va lire dans la base de registre la cle 'startup' 
 *				et lance tous les  programmes qui se trouvent dans cette cle.
 *				Il surveille ensuite si l'un des programmes s'arrete. Si c'est le cas et que 
 *				l'arret n'est pas volontaire par l'application, il arrete alors 
 *				tous les programmes et les relance a nouveau.
 *				Ce module fournit une fonction (DLL) qui permet a une application de 
 *              signaler son arret, afin d'eviter que ce module ne redemarre l'application!
 *				Si le module detecte la fin d'un process(programme) alors que le signal d'arret
 *				n'a pas ete communique, le module arrete (violemment) les autres process et les
 *              relance tous.
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





/*---------------------------- DEFINITION DU TYPE DE LIBRAIRIE -------------------------*/

#ifndef LWD_H
#define LWD_H


#ifdef LWD_DEF
#    include <public.h>
#else
#    include <export.h>
#endif


// Pour pouvoir utiliser les types DWORD, HKEY et le fonctions RegXXXX.
#ifndef _WINDOWS_
	#include <windows.h>
#endif


/*---------------------------- PROTOTYPES -------------------------*/


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD WINAPI LWD_SignaleFin(void)
 * PARAMETRES: AUCUN
 * RETOUR    : ERROR_SUCCESS : le signal de fin a pu etre envoye au 
 *             process 'startup'.
 *			   Autre code d’erreur, il s’agit d’un code d’erreur standard Win32
 *             dont le texte peut être obtenu avec FormatMessage.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Signale au process 'Startup' que l'application lancee par lui
 *             va s'arreter volontairement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI LWD_SignaleFin(void);
