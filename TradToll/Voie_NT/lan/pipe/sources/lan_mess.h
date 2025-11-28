/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL LAN
* FICHIER: LAN_MESS.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Prototypes des fonctions traitant les messages recus par
*         LAN.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/includes/lan_mess.h_v  $
 * 
 *    Rev 1.2   Apr 25 2001 16:56:12   HOMO
 *  
 * 
 *    Rev 1.1   Sep 08 2000 14:49:48   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:14   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   02 Oct 1998 11:45:48   bph
 *  
 * 
 *    Rev 1.8   09 Dec 1997 11:25:02   BPH
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef LAN_MESS_H
#define LAN_MESS_H

/*--------------- INCLUDES: ---------------*/
/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
/* -------------- FONCTIONS: --------------*/

PROTECTED void LanAttendBAL(short int ident);

PROTECTED void LanSupprimeBAL(short int ident);

PROTECTED enum_lan_valide LanDebutService(short int /*ident_lecteur*/ , 
      enum_lan_service /*service*/ , noyau_bal_id /* bal_dest */ );

PROTECTED enum_lan_valide LanDebutServiceDop(short int ident, noyau_bal_id 
												bal_demandeur,
												enum_espion_sens sens,
												enum_espion_nature nature);

PROTECTED enum_lan_valide LanFinService(short int /*ident_lecteur*/ , 
      enum_lan_service /*service*/ , noyau_bal_id /* bal_dest*/ );
PROTECTED BOOL LanEnvoiService(short int ident, enum_lan_service service_id, 
      enum_lan_type type_message, void * p_msg);
PROTECTED void LanEnvoiAcquittement(short int /*ident*/ , noyau_bal_id 
      /*bal_dest*/ , enum_lan_service /*service_id*/ , enum_lan_type 
      /*type_message*/ );
PROTECTED enum_lan_valide LanVerifDemande(short int ident, noyau_bal_id bal, 
      enum_lan_service service_id);

PROTECTED BOOL LanEnvoiDop(short int ident, unsigned char * message, unsigned 
      short int longueur, enum_espion_sens sens, enum_lan_msg_dop type);

PROTECTED noyau_enum_retour LanEnvoiEmis(short int /*ident*/ , 
      enum_lan_msg_interne /*type*/ , struct_lan_gestion_msg * /*message*/ );

PROTECTED noyau_enum_retour LanEnvoiFic(short int /*ident*/ , 
      enum_lan_msg_interne /*type*/ , struct_lan_gestion_fic * /*param_fic*/ );

PROTECTED void LanEnvoiAppli(short int /*ident*/ , noyau_bal_id /*bal_dest */ , 
      enum_lan_service /*service*/ , enum_lan_type /*type*/ , void * /*p_msg*/ )
;

/*------------------VARIABLES:-----------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif
