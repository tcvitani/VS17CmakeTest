/************** (v) 2016 EMOVIS - All rights reserved ***************/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Display Unit
* FICHIER: AFF_MESS.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION: Prototypes des fonctions pour la gestion des messages
*              entre l'application et le module
* --------------------------------------------------------------------
* HISTORIQUE:
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef AFF_MESS_H
#define AFF_MESS_H

/*--------------- INCLUDES: ---------------*/


/*--------------- RESERVED: ---------------*/

#include <protect.h>

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/


/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/
PROTECTED enum_aff_valide AFFDebutService( short int        ident_aff,
                                           enum_aff_service service_id,
                                           noyau_bal_id        bal_dest);
PROTECTED enum_aff_valide AFFDebutServiceEspion( short int         ident_aff,
                                              noyau_bal_id      BalDemandeur,
											  enum_espion_nature      nature,
											  enum_espion_sens		 sens);
PROTECTED enum_aff_valide AFFFinService( short int        ident_aff,
                                         enum_aff_service service_id,
                                         noyau_bal_id        bal_dest);
PROTECTED void AFFEnvoiDeviceInfo(short int		ident_aff,
                                  enum_aff_type	type_message,
								  _AFF_ANI_DEV_INFO_MSG *device_info_msg);
	
/* Fonctions d'envoi de messages par BAL */
PROTECTED void AFFEnvoiAcquittement( short int        ident_aff,
                                     noyau_bal_id        bal_dest,
                                     enum_aff_service service_id,
                                     enum_aff_type    type_message);

PROTECTED void AFFEnvoiEtat(short int			ident_aff,
							noyau_bal_id		bal_dest,
							enum_aff_type		type,
							unsigned int		uiStatus);
PROTECTED void AFFEnvoiVisu( short int     ident_aff,
                             noyau_bal_id     bal_dest,
                             enum_aff_type type);

PROTECTED void AFFEnvoiIos( short int     ident_aff,
                            enum_ani__ios message_type,
                            void          *parametre);

PROTECTED void AFFLibereBAL(noyau_bal_id bal);

/*-- ------------- VARIABLES: ---------------*/

#undef PROTECTED
#undef I
#undef INIT
#endif
