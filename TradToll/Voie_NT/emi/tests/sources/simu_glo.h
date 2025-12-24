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
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Includes/simu_glo.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:08   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   May 27 1997 16:32:20   ANA
 *  
 * 
 *    Rev 1.2   May 16 1997 13:35:24   ANA
 * Test de la creation  et changement de fichier
 * avec emission en parallèle
 * 
 *    Rev 1.1   Apr 07 1997 13:53:36   ANA
 * Mise au point pour avoir deux taches distinctes :
 * emission et reception de messages vers le module
 * 
 *    Rev 1.0   Mar 21 1997 09:29:38   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef SIMU_GLO_H
#define SIMU_GLO_H

/*--------------- INCLUDES: ---------------*/

#include <fic_conf.h>
#include <fic_def.h>
#include <fic_gere.h>

#include "emi_fic.h"

#include <protect.h>

/*--------------- DEFINE :-----------------*/

#define VRAI_HORODATE

/* retour chariot */
#define CR            13

/* correction */
#define CORRECTION    8

/* espace */
#define ESPACE        255

#define SIMU_ARRET    -1
/*--------------- TYPEDEFS: ----------------*/
typedef struct
{
   noyau_bal_id  bal_lan;
   noyau_bal_id  bal_hrd;
   noyau_bal_id  bal_simu;
   noyau_bal_id  bal_emi;
   unsigned char  flag_fin;
   unsigned char  boite;
   unsigned char  path[80];
   unsigned char  path_distant[80];
   short int  priorite_tache;
   boolean			FileStatus;	

   TGereFic  DateFile;
   long      num_fichier;
   FILE *    MsgFile;

} struct_simu;

/*-----------------FONCTION:--------------------*/

PROTECTED void cadre( int, int);
PROTECTED void SendRestitution(void);
PROTECTED void SendEtat(void);
PROTECTED void SendConfig(void);
PROTECTED void SendArret(void);
PROTECTED void NotifyChange(void);
PROTECTED void MenuPrincipal(void);
PROTECTED void AfficheReception( struct_neutre * /*p_neutre*/);

PROTECTED void SimuEnvoie(noyau_bal_id /*bal_dest*/,noyau_bal_id /*bal_src*/, struct_emi_message * /* *p_msg*/);


PROTECTED void SaisieChaine( char * /*chaine*/ );

PROTECTED DWORD WINAPI Simulateur (void *param);
PROTECTED DWORD WINAPI SimuRecoit(void *param);

/*----------------VARIABLES:-------------------*/
PROTECTED struct_simu SIMU;

#undef PROTECTED
#undef I
#undef INIT
#endif