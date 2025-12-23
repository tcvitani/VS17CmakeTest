/************** (v) 2006 CSEE-Peage --------- Droits reserves ****************/
/*																			 */
/* $D_HEAD																	 */
/* ------------------------------------------------------------------------- */
/* MODULE: AFFICHEUR														 */
/* FICHIER: AFF_IOS_PRC.H													 */
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

#ifndef AFF_IOS_PRC_H
#define AFF_IOS_PRC_H

/*--------------- INCLUDES: ---------------*/

/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/
#define AFF_NB_ERREURS_MAX	40

#define PRC_INIT_FAIBLE		0x0A	//10 decimal
#define PRC_INIT_FORT		0x0B	//11 decimal
#define PRC_RAM_AFF			0x0C	//12 decimal
#define PRC_ERASE_MEM		0x20	//32 decimal
#define PRC_BIG_FONT		0x21	//33 decimal
#define PRC_RESET_MIC		0x23	//35 decimal
#define PRC_START			0x3C	//< char - char to start the message
#define PRC_STOP			0x3E	//> chat - char to stop the message
#define PRC_LED_ON			0x25
#define PRC_LED_OFF			0x26


/* Definition de la longueur des lignes */
#define PRC_MAX_CAR_POLICE_A  20
#define PRC_MAX_CAR_POLICE_B  15

/*--------------- TYPEDEFS: ---------------*/
typedef enum
{
	PRC_NO_LIGNE,
	PRC_LIGNE_1,
	PRC_LIGNE_2,
}
enum_prc_line;
/*--------------- FUNCTIONS:---------------*/

PROTECTED void AFFEnvoiAni (short int ident_aff,
                            enum_ani__ios message,
                            void *complement);
PROTECTED void AFFEcritMessageSerie(short int ident_aff, short int port,
                                    UCHAR * buffer, short int nb_octets);
PROTECTED void AFFErreurLiaison(short int ident_aff,
                                Ser_enum_type_tampon_t tampon);
PROTECTED void AFFCommande (short int ident_aff,
							unsigned char cmd,
                            void *parametre);
/*--------------- VARIABLES:-----------------*/
PROTECTED UCHAR	PRC_LINE_1[2]	
#ifdef LOC_DEF
=	{0x0A, 0x3C}				//sets the cursor to the first line
#endif
;

PROTECTED UCHAR	PRC_LINE_2[2] 
#ifdef LOC_DEF
=	{0x0A, 0x8C}				//sets the cursor to the second line
#endif
;

PROTECTED UCHAR	PRC_CURSOR_TO_START[2]	
#ifdef LOC_DEF
=	{0x0B, 0x00}				//sets the cursor to the begining of the line
#endif
;

PROTECTED UCHAR	PRC_MODE[2]	
#ifdef LOC_DEF
=	{0x00, 0x30}				//sets the mode to small fonts
#endif
;


PROTECTED UCHAR PRC_ADDRESS[2]	//sets the address of the display 
#ifndef LOC_DEF
=	{'0', '0'}
#endif
;

PROTECTED UCHAR RESET_HW[7]	//resets the device
#ifdef LOC_DEF
=	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x23}
#endif
;
/*-------------------------------------------*/
#undef PROTECTED
#undef I
#undef INIT
#endif