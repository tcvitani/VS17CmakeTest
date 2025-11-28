/*------   (v) 1998 CS- Route  -----------   Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL LAN
* FICHIER: LAN_SERV.H
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Prototypes des fonctions traitant les services du LAN.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/includes/lan_serv.h_v  $
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
 *    Rev 1.3   Mar 21 1997 15:07:34   ANA
 * Gestion du multi-jeton pour les services suivants :
 * ETAT, MESSAGE, FICHIER
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef LAN_SERV_H
#define LAN_SERV_H

/*--------------- INCLUDES: ---------------*/
/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
/* -------------- FONCTIONS: --------------*/
PROTECTED void ReceptionLanArret(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanDop(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanEtat(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanMessage(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanConnexion(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanFichier(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );
PROTECTED void ReceptionLanHoraire(short int /*ident*/ , struct_lan_message * 
      /* p_msg_recu */ );


/*------------------VARIABLES:-----------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif
