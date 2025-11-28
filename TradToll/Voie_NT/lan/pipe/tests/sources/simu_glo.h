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
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/simu_glo.h_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   02 Oct 1998 11:46:12   bph
 *  
 * 
 *    Rev 1.6   07 Oct 1997 11:32:06   ANA
 * Test version 4.00 avec nouvelle librairie de 
 * decoupage/recollage des fichiers à transferer
 * 
 *    Rev 1.5   Aug 04 1997 17:34:42   ANA
 * Transfert de fichiers avec renommage
 * 
 *    Rev 1.4   Mar 11 1997 16:52:40   ANA
 * Adaptation au fur et a mesure des tests
 * 
 *    Rev 1.3   Mar 07 1997 18:33:36   ANA
 * Evolution du message de vie et Ajout de la tache
 * de gestion du service HORAIRE
 *
 *    Rev 1.2   Feb 20 1997 15:57:00   ANA
 * Version 0
 * 
 *    Rev 1.1   Feb 10 1997 17:15:38   ANA
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
/* retour chariot */
#define CR            13

/* correction */
#define CORRECTION    8

/* espace */
#define ESPACE        255

/*--------------- TYPEDEFS: ----------------*/
typedef struct
{
   noyau_bal_id  bal_id;
   noyau_bal_id  bal_module;
   noyau_priorite_tache  priorite_tache;
   unsigned char  flag_fin;
   unsigned char  boite;
   unsigned char  path[80];
   unsigned char  path_distant[80];
} struct_simu;

/*-----------------FONCTION:--------------------*/

PROTECTED void cadre( int left, int top, int bottom, int right);
PROTECTED void SimuNumBalDefaut(void) ;
PROTECTED void SimuArretModule(noyau_bal_id /* bal_module*/);
PROTECTED void MenuPrincipal(void);
PROTECTED void MessagesModule(noyau_bal_id /* bal_module*/);

PROTECTED void AfficheReception( struct_neutre * /*p_neutre*/);
PROTECTED void AfficheAcquittement( char *, enum_lan_type );
PROTECTED void AfficheMessageRecu( enum_lan_service , enum_lan_type );

PROTECTED void SaisieChaine( char * /*chaine*/ );
PROTECTED void EnvoiMessage(noyau_bal_id /* bal*/);

PROTECTED DWORD WINAPI Simulateur(PVOID param);
PROTECTED DWORD WINAPI SimuRecoit(PVOID param);
PROTECTED DWORD WINAPI SimuSrvPipe (PVOID param);

PROTECTED void SimuEnvoiLan(noyau_bal_id /*bal_dest*/,enum_lan_service,
                            enum_lan_type, char *);


/*----------------VARIABLES:-------------------*/
PROTECTED struct_simu SIMU;

PROTECTED char *etat_serveur[8]
#ifdef LOC_DEF
= {"liaison OK       ",
   "attente connexion",
   "connect‚         ",
   "degrad‚          ",
   "transfert OK     ",
   "transfert NOK    ",
   "liaison HS       ",
   "liaison HS       "};
#else
;
#endif

PROTECTED char boite[]
#ifdef LOC_DEF
 = { 'A', 'B', 'C', 'D'};
#else
;
#endif

#undef PROTECTED
#undef I
#undef INIT
#endif