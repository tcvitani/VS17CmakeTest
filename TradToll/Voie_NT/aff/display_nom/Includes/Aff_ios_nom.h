/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*																			 */
/* $D_HEAD																	 */
/* ------------------------------------------------------------------------- */
/* MODULE: AFFICHEUR														 */
/* FICHIER: AFF_IOS_NOM.H													 */
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

#ifndef AFF_IOS_NOM_H
#define AFF_IOS_NOM_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
#define AFF_NB_ERREURS_MAX	40

#define NOM_START_CHAR			0x02
#define NOM_END_CHAR			0x03
#define NOM_CMD_POLL			0x00
#define NOM_CMD_NEW_PRGM_DWNLD	0x0D
#define NOM_CMD_FIRMWARE_RQST	0x0F
#define NOM_CMD_LCD_PARAMETER	0x3B
#define NOM_CMD_DISP_LARGE_CHAR	0x32
#define NOM_CMD_DISP_SMALL_CHAR	0x33
#define NOM_CMD_DISP_DUAL_CHAR	0x34
#define NOM_CMD_OUTPUTS			0x3F

#define NOM_LENGTH_POSITION_IN_MSG	1
#define NOM_FIRMWARE_RESPONSE_POSITION_IN_MSG	1


// Definition of line length
#define NOM_MAX_CHAR_POLICE_A  20
#define NOM_MAX_CHAR_POLICE_B  15
#define NOM_MAX_CHAR_POLICE_C  30

/*--------------- TYPEDEFS: ---------------*/
typedef enum
{
	NOM_NO_LINE,
	NOM_LINE_1,
	NOM_LINE_2
}
enum_nom_line;
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