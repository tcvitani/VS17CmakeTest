/************** (v) 2017 EMOVIS - All rights reserved ***************/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Display Unit
* FICHIER: AFF_SERV.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION: Prototypes des fonctions pour la gestion des services
*              du module Display Unit
* --------------------------------------------------------------------
* HISTORIQUE:
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef AFF_SERV_H
#define AFF_SERV_H

/*--------------- INCLUDES: ---------------*/


/*--------------- RESERVED: ---------------*/

#include <protect.h>

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/


/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/

/* Fonctions pour la gestion des services */
PROTECTED void AFFReceptionAffichage( short int          ident_aff,
                                      struct_aff_message *p_msg_recu);
PROTECTED void AFFReceptionVisu( short int          ident_aff,
                                 struct_aff_message *p_msg_recu);
PROTECTED void AFFReceptionEtat( short int          ident_aff,
                                 struct_aff_message *p_msg_recu);
PROTECTED void AFFReceptionEspion( short int          ident_aff,
                                struct_aff_message *p_msg_recu);
PROTECTED void AFFReceptionArret( short int          ident_aff,
                                  struct_aff_message *p_msg_recu);
PROTECTED void AFFGestionAffichage( short int          ident_aff,
                                    struct_aff_message *p_message,
                                    aff_infos          *buffer);
PROTECTED void AFFGestionReset( short int          ident_aff,
                                struct_aff_message *p_message);
PROTECTED void AFFGestionDimming( short int          ident_aff,
                                  struct_aff_message *p_message);
PROTECTED void AFFReceptionDeviceInfo(short int          ident_aff,
									struct_aff_message *p_message);
/*-- ------------- VARIABLES: ---------------*/

#undef PROTECTED
#undef INIT
#undef I
#endif
