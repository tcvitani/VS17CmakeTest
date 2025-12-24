/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions utiles pour l'application
* --------------------------------------------------------------------
* DESCRIPTION: Fonctions EmiRessource(), EmiLance() et EmiArret()
*              qui permettent a une application utilisatrice du module
*              de l'initialiser, de le lancer et de l'arreter.
*              Fonctions EmiDebutDebug() et EmiFinDebug() pour ecrire
*              dans un fichier les erreurs survenues.
*              Fonctions EmiDebutTrace() et EmiFinTrace() pour ecrire
*              dans un fichier les traces utiles lors de l'integration
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emi_res.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:06   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.5   Jan 22 1999 15:19:28   FR
 * Mise à jour du numéro de version
 * 
 *    Rev 1.4   03 Nov 1997 16:13:50   DPI
 * Ajout de 'horodate.h'
 * 
 *    Rev 1.3   03 Nov 1997 16:10:14   DPI
 * Version 4.00
 * 
 *    Rev 1.2   Aug 01 1997 17:44:36   HMO
 *  
 * 
 *    Rev 1.1   Aug 01 1997 17:30:58   HMO
 * Gestion de la version
 * 
 *    Rev 1.0   Apr 15 1997 15:23:30   HMO
 * Initial revision
 * 
 *    Rev 1.2   Apr 15 1997 10:03:36   ANA
 * Correction du bug sur le service FICHIER deTCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:48   ANA
 * Nouvelle gestion du "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:24:56   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <dos.h>
#include <stdarg.h>
#include <io.h>
#include <conio.h>
#include <string.h>


//#include "stdcsee.h"
#include "err.h"
/* module NOYAU */
#include <noyau.h>
//#include <tcp_ip.h>

#include <csr_lan.h>

#include "horodate.h"
#include "emi_fic.h"

#define LOC_DEF
#include <emi_glob.h>
#undef LOC_DEF

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/
#define MAXCHAINE 127

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

/*--------------- VARIABLES: --------------*/
//PRIVATE char *what = DEFINIR_WHAT_VERSION( "EMI_RES", EMI_VERSION, "Version 7.02 Noyau");

/*--------------- CODE: -------------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC short int EmiRessource( struct_noyau *p_noyau )
* PARAMETRES:
*     entree: pointeur sur une structure du type struct_noyau
*     retour: numero de boite aux lettres permettant a l'application
*             de dialoguer avec le module EMI
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale
* ROLE: fonction qui permet de definir au systeme les ressources
*       noyau utilisees par le module Emission de fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
//PUBLIC short int EmiRessource(struct_noyau *p_noyau )
//{
//   short int retour;
//
//   NO_WARNING( what);
//
//   retour = p_noyau->noyau_nb_bal;
//
//   /* nombre de tache et de BAL du module EMI */
//   p_noyau->noyau_nb_tache += EMI_NB_TACHE;
//   p_noyau->noyau_nb_bal   += EMI_NB_BAL;
//   p_noyau->noyau_nb_chrono += EMI_NB_CHRONO;
//
//   /*  retourne le numero de BAL de EMI */
//   return( retour);
//}
