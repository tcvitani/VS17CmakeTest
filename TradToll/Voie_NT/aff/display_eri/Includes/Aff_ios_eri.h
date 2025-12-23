/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*																			 */
/* $D_HEAD																	 */
/* ------------------------------------------------------------------------- */
/* MODULE: AFFICHEUR														 */
/* FICHIER: AFF_IOS_ERI.H													 */
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

#ifndef AFF_IOS_ERI_H
#define AFF_IOS_ERI_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
#define AFF_NB_ERREURS_MAX		40

#define ERI_INDICATE_ADDR_MODE	0x0F	// Indicate Address Mode
#define ERI_UNIT_ADDRESS		0x20	// Address to send = 0x20 + 'Address of Unit'; Where 'Address of Unit' is in the range 0=Global to 200
#define ERI_TERMINATE_ADDR_MODE	0x0E	// Terminate Address Mode
#define ERI_TEXT_START			0x02	// Indicate text start
#define ERI_DATE_TIME_START		0x04	// Indicate text start
#define ERI_1ST_CHAR_POS		0x1F	// 1st character position. 1F + Position of the first character. Ex: To write from 1st char -> 1F + 01 = 20;
#define ERI_TEXT_END			0x03	// Indicate text end
#define ERI_TIME_END1			0x30	// Indicate time end
#define ERI_TIME_END2			0x34	// Indicate time end
#define ERI_DATE_END			0x35	// Indicate date end
#define ERI_END					0x09	// End of communication

#define ERI_1ST_ROW_START		1		// 1st row start character
#define ERI_2ND_ROW_START		11		// 2nd row start character
#define ERI_3RD_ROW_START		21		// 3rd row start character

// Definition of line length
#define ERI_MAX_CHAR_POLICE_A  10

/*--------------- TYPEDEFS: ---------------*/
typedef enum
{
	ERI_NO_LINE,
	ERI_LINE_FIRST,
	
	ERI_LINE_1 = ERI_LINE_FIRST,
	ERI_LINE_2,
	ERI_LINE_3,

	ERI_LINE_LAST = ERI_LINE_3,

	ERI_LINE_END
}
enum_eri_line;
/*--------------- FUNCTIONS:---------------*/

PROTECTED void AFFEnvoiAni (short int ident_aff,
                            enum_ani__ios message,
                            void *complement);
PROTECTED Ser_enum_cpt_erreur_t AFFEcritMessageSerie(short int ident_aff, short int port,
                                    UCHAR * buffer, short int nb_octets);

PROTECTED Ser_enum_cpt_erreur_t AFFReadMessageFromPort(short ident_aff, BYTE *byDataBuffer, int iLength, int *iReadData);
PROTECTED void AFFErreurLiaison(short int ident_aff,
                                Ser_enum_type_tampon_t tampon);
PROTECTED void AFFCommande (short int ident_aff,
							unsigned char cmd,
                            void *parametre);

/*--------------- VARIABLES:-----------------*/

/*-------------------------------------------*/
#undef PROTECTED
#undef I
#undef INIT
#endif