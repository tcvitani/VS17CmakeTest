/************** (v) 2006 CSEE-Peage --------- Droits reserves ****************/
/*																			 */
/* $D_HEAD																	 */
/* ------------------------------------------------------------------------- */
/* MODULE: AFFICHEUR														 */
/* FICHIER: AFF_IOS_MATCO.H													 */
/* LANGAGE: C																 */
/* ------------------------------------------------------------------------- */
/* MOT-CLE:																	 */
/* ------------------------------------------------------------------------- */
/* RESUME: Declarations de constantes et variables globales a la tache		 */
/*         AFF_IOS.															 */
/* ------------------------------------------------------------------------- */
/* DESCRIPTION:																 */
/* ------------------------------------------------------------------------- */
/* HISTORIQUE:																 */
/*****************************************************************************/

#ifndef AFF_IOS_MATCO_H
#define AFF_IOS_MATCO_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
#define AFF_NB_ERREURS_MAX	40

#define MATCO_LINE_1		0x01	// char "1" - for the first line
#define MATCO_LINE_2		0x02	// char "2" - for the second line
#define MATCO_LINE_3		0x03	// char "3" - for the third line

#define ACK	0xFA	
#define NAK	0xFE
/*--------------- TYPEDEFS: ---------------*/
typedef enum
{
	MATCO_NO_LIGNE,
	MATCO_LIGNE_1,
	MATCO_LIGNE_2,
	MATCO_LIGNE_3,
}
enum_matco_line;

/*--------------- FUNCTIONS:---------------*/

PROTECTED void AFFEnvoiAni (short int ident_aff,
                            enum_ani__ios message,
                            void *complement);
PROTECTED void AFFEcritMessageSerie(short int ident_aff, short int port,
                                    unsigned char * buffer, short int nb_octets);
PROTECTED void AFFErreurLiaison(short int ident_aff,
                                Ser_enum_type_tampon_t tampon);
PROTECTED void AFFCommande (short int ident_aff,
							unsigned char cmd,
                            void *parametre);
/*-------------------------------------------*/
#undef PROTECTED
#undef I
#undef INIT
#endif
