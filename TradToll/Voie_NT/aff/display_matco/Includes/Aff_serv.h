/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
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
* $Log:   T:/MODULO/VoieNt/Aff/AFF/Includes/Aff_serv.h_v  $
 * 
 *    Rev 1.2   Jul 19 2001 16:55:38   sbatiot
 *  
 * 
 *    Rev 1.1   Mar 31 2000 14:43:52   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.2   Nov 17 1999 10:54:38   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:26   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:24   nbl
 *  
 * 
 *    Rev 1.1   09 Apr 1998 11:39:40   HMO
 *  
 * 
 *    Rev 1.0   Mar 27 1998 11:13:08   pgg
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   11 Dec 1997 15:19:36   ANA
 * Ajout de la commande "ParametresFiscaux",
 * Tests du module en page etendue.
 * 
 *    Rev 1.1   02 Dec 1997 14:29:56   ANA
 * Gestion de la liste chainee et de la section #STRINGS
 * 
 *    Rev 1.0   21 Nov 1997 17:01:52   ANA
 * Version initiale
 *
 * Renamed from SGM to AFF 21.07.1999. sasa - ecsat
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
#endif	/* AFF_SERV_H */
