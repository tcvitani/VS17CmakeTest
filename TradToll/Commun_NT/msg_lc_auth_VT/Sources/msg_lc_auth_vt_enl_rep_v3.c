/*------   (v) 2014 Sanef ITS Technologies   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: MSG LC AUTH VT ENL REP V3
* FICHIER: 
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <msg_lc_auth_vt_enl_rep_v3.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"
 
/*--------------- DEFINES: ---------------*/

#define MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS MSG_HEADER_NB_FIELDS + 22 + 1 //header + num of body elements + stop

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_LC_AUTH_VT_ENL_REP_V3, MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS)
MSG_DECLARE_REF(MSG_LC_AUTH_VT_ENL_REP_V3_InfoType, MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS)
MSG_DECLARE_REF(MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo, MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_LC_AUTH_VT_ENL_REP_V3_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

	union MSG_Field LOCAL_MSG_LC_AUTH_VT_ENL_REP_V3[MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS] =
    {
		{ MSG_FIELD_HEADER(MSG_LC_AUTH_VT_ENL_REP_V3, MSG_LC_AUTH_VT_ENL_REP_V3_CD) },

		{ MSG_FIELD_TIME, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.dte_ref_entry) },

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_trs_ref_num),					0UL,			99999UL											},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_plaza_id),						0UL,			9999UL											},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_lane_id),						0UL,			9999UL											},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_trs_id ),						0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_TRS_ID_LENGTH			},

		{ MSG_FIELD_TIME,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.dte_entry)																							},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_vrn ),							0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_VRN_LENGTH			},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_vrn_country ),					0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_VRN_COUNTRY_LENGTH	},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.entry_provider ),					0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_PROVIDER_LENGTH		},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.flag_result),						0UL,			9UL												},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.reason_id),							0UL,			999999UL										},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.ticket_type),						0UL,			99UL											},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.flag_delete),						0UL,			9UL												},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.flag_no_insert),						0UL,			9UL												},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.vehicle_class),						0UL,			99UL											},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.payment_subtype),					0UL,			99UL											},

		{ MSG_FIELD_VARSTR,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.accepted_days),						0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_ACCEPTED_DAYS_LENGTH	},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.account_balance),					0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_ACC_BALANCE			},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.user_balance),						0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_USER_ACC				},

		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.contract_type),						0UL,			99UL											},

		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, body.request_id),							0UL,			999999UL										},

		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3, list_comp_info), MSG_LC_AUTH_VT_ENL_REP_V3_InfoType, MSG_SIZEOF(MSG_LC_AUTH_VT_ENL_REP_V3_InfoType), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_INFO_TYPE[MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS] =
	{
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3_InfoType, type),							0UL,			99UL											},
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3_InfoType, list_info_type), MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo, MSG_SIZEOF(MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo), 0UL, 99UL },

		{ MSG_FIELD_STOP }

	};

	union MSG_Field LOCAL_ELEM_INFO[MSG_LC_AUTH_VT_ENL_REP_V3_NB_FIELDS] =
	{
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo, code),							0UL,			99UL											},
		{ MSG_FIELD_VARIANT,MSG_OFFSET(MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo, info_value),						0UL,			MSG_LC_AUTH_VT_ENL_REP_V3_INFO_VALUE_LENGTH		},
		
		{ MSG_FIELD_STOP }
	};
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_LC_AUTH_VT_ENL_REP_V3,				LOCAL_MSG_LC_AUTH_VT_ENL_REP_V3);
	MSG_INIT_REF (MSG_LC_AUTH_VT_ENL_REP_V3_InfoType,		LOCAL_INFO_TYPE);
	MSG_INIT_REF (MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo,		LOCAL_ELEM_INFO);

}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3 *MSG_LC_AUTH_VT_ENL_REP_V3_New(void)
{
    // la variable MSG_LC_AUTH_VT_ENL_REP de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
	return MSG_New(MSG_LC_AUTH_VT_ENL_REP_V3, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3), "MSG_LC_AUTH_VT_ENL_REP_V3");
}


// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(struct MSG_LC_AUTH_VT_ENL_REP_V3 *p_auth_vt_enl_rep)
{
    return MSG_Delete_All (&p_auth_vt_enl_rep);
}

PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3_InfoType *MSG_LC_AUTH_VT_ENL_REP_V3_InfoType_New(HLIST *list_info_type)
{
	return MSG_New_List(list_info_type, MSG_LC_AUTH_VT_ENL_REP_V3_InfoType, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3_InfoType));
}

PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo *MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo_New(HLIST *list_comp_info)
{
	return MSG_New_List(list_comp_info, MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo, sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo));
}

// ITERATEURS

PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3_InfoType *MSG_LC_AUTH_VT_ENL_REP_V3_Get_First_InfoType(HLIST list_info_type)
{
	return MSG_Get_First(list_info_type);
}

PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3_InfoType *MSG_LC_AUTH_VT_ENL_REP_V3_Get_Next_InfoType(HLIST list_info_type, struct MSG_LC_AUTH_VT_ENL_REP_V3_InfoType *p_type)
{
	return MSG_Get_Next(list_info_type, p_type);
}

PUBLIC struct MSG_LC_AUTH_VT_ENL_REP_V3_ElemInfo *MSG_LC_AUTH_VT_ENL_REP_V3_Get_First_ElemInfo(HLIST list_comp_info)
{
	return MSG_Get_First(list_comp_info);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_LC_AUTH_VT_ENL_REP"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_LC_AUTH_VT_ENL_REP de MSG_LC_AUTH_VT_ENL_REP
// et MSG_LC_AUTH_VT_ENL_REP_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_LC_AUTH_VT_ENL_REP_V3_Write(struct MSG_LC_AUTH_VT_ENL_REP_V3 *p_auth_vt_enl_rep,
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write (p_auth_vt_enl_rep, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_LC_AUTH_VT_ENL_REP
PUBLIC BOOL MSG_LC_AUTH_VT_ENL_REP_V3_Read(struct MSG_LC_AUTH_VT_ENL_REP_V3 *p_auth_vt_enl_rep,
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_auth_vt_enl_rep, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_LC_AUTH_VT_ENL_REP_V3_New_Record(HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_HEADER_Compare_CD, 
						  (PVOID)MSG_LC_AUTH_VT_ENL_REP_V3_CD,
						  MSG_LC_AUTH_VT_ENL_REP_V3,
						  sizeof(struct MSG_LC_AUTH_VT_ENL_REP_V3), "MSG_LC_AUTH_VT_ENL_REP_V3");
}

