/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     aff_util.h												         */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef AFF_UTIL_H
#define AFF_UTIL_H

/*--------------------------- INCLUDES:  --------------------------*/

/*--------------------------- RESERVED:  --------------------------*/

#ifdef AFF_UTIL_DEF
	#include <public.h>
#else
	#include <export.h>
#endif

/*--------------------------- EXTERNALS: --------------------------*/

/*--------------------------- DEFINES:   --------------------------*/

#define LGN_CLE				'#'
#define LGN_LABEL			'$'
#define LGN_COMMENTS		'*'
#define LGN_INVISIBLE		'^'

#define AFF_LIGNE_MAX		80

#define NB_MAX_AFF_PRIO		8

#define AFF_NB_POLICES		3

#define MAX_VARIABLES		10

#define LGN_MAX_CAR			255

/* format de l'ann‚e */
#define AFF_DIGIT2			0
#define AFF_DIGIT4			1

#define NO_BAL				0xFFFF

// !!!!!! Ne Pas modifier l'ordre des mots cl‚s !!!!!
// autant de mots cl‚s que de champs dans la structure aff_inf !!
// AFF_YEAR doit ˆtre suivi de AFF_YEAR_2_DIGIT, ceci … cause de l'analyse du format ann‚e
#define NB_MAX_MOT_CLE (sizeof(aff_inf) / sizeof(struct_aff_donnee_affichage))

/*--------------------------- TYPEDEFS:  --------------------------*/

typedef enum
{
	MSG_ERREUR = 0,
	MSG_MOT_CLE,
	MSG_LABEL
} 
enum_type_message;

typedef enum
{
	LGN_ERREUR = 0,
	LGN_MOT_CLE,
	LGN_MOT_LABEL,
	LGN_LIGNE,
} 
enum_type_ligne;

typedef struct
{
	char *nom;
}
struct_mot_cle;

/* Informations remplies sur reception d'un message affichage */
typedef struct
{
	struct_aff_donnee_affichage	soft_revision;
	struct_aff_donnee_affichage	payment_type;
	struct_aff_donnee_affichage	veh_class;
	struct_aff_donnee_affichage	libelle1;
	struct_aff_donnee_affichage	libelle2;
	struct_aff_donnee_affichage	libelle3;
	struct_aff_donnee_affichage	fare_int;      /* Champ calcul‚ */
	struct_aff_donnee_affichage	paid_int;      /* Champ calcul‚ */
	struct_aff_donnee_affichage	sale_fare_int; /* Champ calcul‚ */
	struct_aff_donnee_affichage	sale_paid_int; /* Champ calcul‚ */
	struct_aff_donnee_affichage	fare_dec;      /* Champ calcul‚ */
	struct_aff_donnee_affichage	paid_dec;      /* Champ calcul‚ */
	struct_aff_donnee_affichage	sale_fare_dec; /* Champ calcul‚ */
	struct_aff_donnee_affichage	sale_paid_dec; /* Champ calcul‚ */
	struct_aff_donnee_affichage	year;          /* Champs calcul‚s par HRD */
	struct_aff_donnee_affichage	year_2_digit;
	struct_aff_donnee_affichage	month;
	struct_aff_donnee_affichage	day;
	struct_aff_donnee_affichage	hour;
	struct_aff_donnee_affichage	minute;
	struct_aff_donnee_affichage	second;
	struct_aff_donnee_affichage	precision_tarif;
	struct_aff_donnee_affichage	amount_due_int;
	struct_aff_donnee_affichage	amount_due_dec;
	struct_aff_donnee_affichage	overpay_int;
	struct_aff_donnee_affichage	overpay_dec;
}
aff_inf;

typedef union
{
	aff_inf infos;
	
	struct_aff_donnee_affichage var[sizeof(aff_inf) / sizeof(struct_aff_donnee_affichage)];
}
aff_infos;

/* Informations sur la ligne lue */
typedef struct
{
	long	no_ligne;
	char	ligne[LGN_MAX_CAR];
} 
struct_line_info;

/* Contenu de la ligne identifie */
typedef struct
{
	long			no_ligne;
	enum_type_ligne	type_ligne;
	char			ligne[LGN_MAX_CAR];
} 
struct_line_id;

// list of lines
typedef struct struct_line_list
{
	char					police;               /* Police utilis‚e */
	char					*txt_fmt;             /* Format utilise pour la ligne */

	unsigned char			nb_var;      /* Nombre de variables pour cette ligne */
	int						motcle[MAX_VARIABLES]; /* Indices dans le tableau des mots-cles (max=10) */

	struct struct_line_list *suiv;
} 
struct_line_list;

// label definition
typedef struct
{
	enum_type_message	type;
	char				label[AFF_MAX_LABEL + 1];
	struct_line_list	*ligne;
} 
struct_label;

typedef struct struct_list_text
{
	char					police;
	unsigned char			texte[AFF_LIGNE_MAX];
	boolean					suiv_valide;
	struct struct_list_text	*suiv;
} 
struct_list_text;

typedef struct
{
	noyau_bal_id		bal;
	struct_label		*p_label;
	struct_list_text	*texte;
} 
struct_request_screen;

typedef struct
{
	struct_request_screen tab_request[NB_MAX_AFF_PRIO];
} 
struct_screen;

/*--------------------------- FUNCTIONS: --------------------------*/

EXPORT void WINAPI AFF_MsgRequestFreeTab(struct_screen *pScreen);
EXPORT void WINAPI AFF_MsgRequestFreeLine(struct_request_screen *pLigne);
EXPORT int WINAPI AFF_MsgRequestCurrentPriority(struct_screen *pScreen);
EXPORT int WINAPI AFF_MsgRequestLinePriority(short int iNumber, struct_screen *pScreen);
EXPORT struct_list_text* WINAPI AFF_MsgRequestLineNumber(struct_request_screen *pLine, short int iNumber);
EXPORT struct_list_text* WINAPI AFF_MsgResearchLineNumber(struct_request_screen *pLine, short int iNumber);

EXPORT boolean WINAPI AFF_ReadFile(char *pFile, struct_label *pLabel, int iNbLabel, long *pLine, int *pPosition);

EXPORT boolean WINAPI FreeTabLabel(struct_label *pLabel, int iNbLabel);

EXPORT boolean WINAPI AFF_InitFile(TGereFic *pGereFic, char *pFileName, char *pAttributes, struct_line_info *pItem, struct_line_id *pId, long *pVal);

EXPORT boolean WINAPI AFF_CloseFile(TGereFic *pGereFic);

EXPORT FIC_enum_retour WINAPI ReadTextPosition(TGereFic *pGereFic, long lPosition, struct_line_id  **ppId, long **ppVal);

EXPORT boolean WINAPI ReadMessage(TGereFic *pGereFic, struct_label *pLabel, long *pLine, int *pPosition);

EXPORT void WINAPI AFF_CreateText(char *pText, size_t iTextSize, struct_line_list *pLine, aff_infos *pBuffer);

EXPORT void WINAPI AFF_DataConversion(aff_infos *pDataInfo, struct_aff_demande *pAffichage);

EXPORT void WINAPI AFF_CreateLine(unsigned char *pConversionArray, struct_request_screen *pScreen, struct_label *pLabel, aff_infos *pBuffer);

/*--------------------------- VARIABLES: --------------------------*/

#undef EXPORT
#undef I
#undef INIT
#endif

/*--------------------------- END OF FILE -------------------------*/