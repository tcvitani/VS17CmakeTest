/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
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
* $Log:   T:/MODULO/VoieNt/Aff/AFF/Includes/Aff_mess.h_v  $
 * 
 *    Rev 1.2   Jul 19 2001 16:55:38   sbatiot
 *  
 * 
 *    Rev 1.1   Mar 31 2000 14:43:52   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.3   Nov 17 1999 10:54:38   nbl
 *  
 * 
 *    Rev 1.2   Nov 15 1999 09:37:24   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:26   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:24   nbl
 *  
 * 
 *    Rev 1.0   Mar 27 1998 11:13:08   pgg
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   21 Nov 1997 17:01:50   ANA
 * Version initiale
 *
 * Renamed from SGM to AFF 21.07.1999. sasa - ecsat
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

PROTECTED void AFFEnvoiEtat( short int     ident_aff,
                             noyau_bal_id     bal_dest,
                             enum_aff_type type);
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



