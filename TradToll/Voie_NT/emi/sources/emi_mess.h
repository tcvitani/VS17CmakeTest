/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: EMI_MESS.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Prototypes des fonctions traitant les messages recus par
*         EMI.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Includes/emi_mess.h_v  $
 * 
 *    Rev 1.1   Apr 12 2001 16:56:10   sbatiot
 *  
 * 
 *    Rev 1.0   14 Dec 1999 14:09:02   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   May 13 1997 17:09:18   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 * 
 *    Rev 1.1   Apr 07 1997 11:50:42   ANA
 * Nouvelle Gestion des "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:14:30   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef EMI_MESS_H
#define EMI_MESS_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/

/* -------------- FONCTIONS: --------------*/

PROTECTED short int EmiDebutService(enum_emi_service /*service*/,
                                    noyau_bal_id /* bal_dest */);
PROTECTED short int EmiFinService(enum_emi_service /*service*/,
                                  noyau_bal_id /* bal_dest*/);
PROTECTED void EmiEnvoiAcquittement(noyau_bal_id /*bal_dest*/,
                                    enum_emi_service /*service_id*/,
                                    enum_emi_type /*type_message*/);
PROTECTED short int EmiVerifDemande(noyau_bal_id /*bal*/,
                                         enum_emi_service /* service_id*/,
                                         enum_emi_type /*type*/);
PROTECTED void EmiEnvoiDop(char * /*message*/,
                           short int /*longueur*/,
                           short int /*sens*/);

PROTECTED void EmiEnvoiAppli(enum_emi_service /*service*/,
                             enum_emi_type /*type*/,
                             void * /*p_msg*/);

PROTECTED short int EmiEnvoiFic(enum_lan_service service,
                                enum_lan_type type,
                                struct_lan_gestion_fic *);

PROTECTED void EmiPurge(struct_emi_gestion_purge *);

PROTECTED noyau_enum_retour EmiEnvoiLan(noyau_bal_id bal_dest,
                                        noyau_bal_id bal_source,
                                        noyau_pool_id pool,
                                        struct_lan_message_externe *p_msg);


/*------------------VARIABLES:-----------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif