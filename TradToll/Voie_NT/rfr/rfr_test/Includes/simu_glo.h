/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU_GLO.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Referenc/test/Includes/simu_glo.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:54   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jun 04 1997 19:11:40   DPI
 *  
 * 
 *    Rev 1.1   Jun 03 1997 17:38:40   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:44:54   DPI
 *  
*
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef SIMU_GLO_H
#define SIMU_GLO_H

/*--------------- INCLUDES: ---------------*/

#include <protect.h>

/*--------------- DEFINE :-----------------*/

/* define pour utiliser le simulateur avec le vrai module horodate */
#define VRAI_HORODATE

/* retour chariot */
#define CR            13

/* correction */
//#define CORRECTION    8

/* espace */
#define ESPACE        255

#define SIMU_ARRET    -1
/*--------------- TYPEDEFS: ----------------*/
typedef struct
{
	noyau_bal_id bal_simu; /* BAL du simulateur (=BAL applicative pour module ReFeRence) */
	noyau_bal_id bal_rfr; /* BAL module ReFeRence */
	noyau_bal_id bal_lan; /* BAL module LAN (publiee par ce simulateur pour module RFR) */
	noyau_bal_id bal_msg; /* BAL module COM_LS (publiee par ce simulateur pour module RFR) */
	noyau_bal_id bal_hrd; /* BAL module Horodate */
	short int  priorite_tache;
	unsigned char  flag_fin;
} struct_simu;

/*-----------------FONCTION:--------------------*/

//PROTECTED void cadre( int, int);
PROTECTED void MenuPrincipal(void);
PROTECTED void SendReference( void);
PROTECTED void SendManual( void);
PROTECTED void ProcessStop( void);
PROTECTED void ProcessMessages( void);
PROTECTED void AfficheReception( struct_neutre * p_neutre);


//PROTECTED void SaisieChaine( char * chaine );
//BOOLEAN SaisieChaine( char * chaine, short lg_max, int xpos, int ypos );


PROTECTED DWORD WINAPI Simulateur (void *param);
PROTECTED DWORD WINAPI SimuRecoit (LPVOID p);

/*----------------VARIABLES:-------------------*/
PROTECTED struct_simu SIMU;
PROTECTED char Lan_Start[3];
PROTECTED char Lan_End[3];
PROTECTED char Lan_Copy[3];
PROTECTED BOOLEAN   Lan_Started 
#ifdef LOC_DEF
= FALSE
#endif
;

#undef PROTECTED
#undef I
#undef INIT
#endif