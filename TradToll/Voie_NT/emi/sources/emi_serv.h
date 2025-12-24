 /*------   (v) 1997 CS- Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_SERV.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Prototypes des fonctions traitant les services de la tache
*         d'EMISSION DE FICHIER
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Includes/emi_serv.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:02   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   May 16 1997 13:33:46   ANA
 *  
 * 
 *    Rev 1.0   Mar 21 1997 09:14:30   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef EMI_SERV_H
#define EMI_SERV_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/

/* -------------- FONCTIONS: --------------*/
PROTECTED void ReceptionEmiArret(struct_emi_message * /* p_msg_recu */);

PROTECTED void ReceptionEmiDop(struct_emi_message * /* p_msg_recu */);

PROTECTED void ReceptionEmiEtat(struct_emi_message * /* p_msg_recu */);

PROTECTED void ReceptionEmiRestitution(struct_emi_message * /* p_msg_recu */);

PROTECTED void ReceptionEmiPurge(struct_emi_message * /* p_msg_recu */);

PROTECTED void ReceptionEmiFic(struct_lan_message *);

PROTECTED void ReceptionEmiConfig(struct_emi_message *p_message);

PROTECTED void FichierSuivant(boolean flag_fic,char *path_fichier);

PROTECTED short int EmettreFichier(char * fichier,
                                   char * path_distant,
                                   noyau_bal_id bal_dest,
                                   noyau_bal_id bal_source);

PROTECTED void EmiEnvoiBackup(enum_emi_service service, void *p_msg);
/*------------------VARIABLES:-----------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif