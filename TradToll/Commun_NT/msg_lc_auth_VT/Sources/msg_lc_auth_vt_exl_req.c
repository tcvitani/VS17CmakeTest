/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: MSG LC AUTH VT EXL REQ
* FICHIER: 
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <msg_lc_auth_vt_exl_req.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"
 
/*--------------- DEFINES: ---------------*/

#define MSG_LC_AUTH_VT_EXL_REQ_NB_FIELDS MSG_HEADER_NB_FIELDS + 10 + 1 //header + num of body elements + stop

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_LC_AUTH_VT_EXL_REQ, MSG_LC_AUTH_VT_EXL_REQ_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_LC_AUTH_VT_EXL_REQ_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_LC_AUTH_VT_EXL_REQ[MSG_LC_AUTH_VT_EXL_REQ_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_LC_AUTH_VT_EXL_REQ, MSG_LC_AUTH_VT_EXL_REQ_CD) },

		{ MSG_FIELD_TIME,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.dte_ref_exit)																					},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_trs_ref_num),					0UL,			99999UL										},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_plaza_id),						0UL,			9999UL										},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_lane_id),						0UL,			9999UL										},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_trs_id ),						0UL,			MSG_LC_AUTH_VT_EXL_REQ_TRS_ID_LENGTH		},

		{ MSG_FIELD_TIME,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.dte_exit)																						},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_vrn ),							0UL,			MSG_LC_AUTH_VT_EXL_REQ_VRN_LENGTH			},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_vrn_country ),					0UL,			MSG_LC_AUTH_VT_EXL_REQ_VRN_COUNTRY_LENGTH	},

		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.exit_provider ),					0UL,			MSG_LC_AUTH_VT_EXL_REQ_PROVIDER_LENGTH		},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_LC_AUTH_VT_EXL_REQ, body.flag_eticket),						0UL,			9UL											},
		
        { MSG_FIELD_STOP }
    };


    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_LC_AUTH_VT_EXL_REQ,                    LOCAL_MSG_LC_AUTH_VT_EXL_REQ);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_LC_AUTH_VT_EXL_REQ *MSG_LC_AUTH_VT_EXL_REQ_New (void)
{
    // la variable MSG_LC_AUTH_VT_EXL_REQ de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_LC_AUTH_VT_EXL_REQ, sizeof(struct MSG_LC_AUTH_VT_EXL_REQ), "MSG_LC_AUTH_VT_EXL_REQ");
}



// suppression COMPLETE d'un message de event
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_LC_AUTH_VT_EXL_REQ_Delete_All (struct MSG_LC_AUTH_VT_EXL_REQ *p_auth_vt_exl_req)
{
    return MSG_Delete_All (&p_auth_vt_exl_req);
}

// ITERATEURS


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_LC_AUTH_VT_EXL_REQ"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_LC_AUTH_VT_EXL_REQ de MSG_LC_AUTH_VT_EXL_REQ
// et MSG_LC_AUTH_VT_EXL_REQ)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_LC_AUTH_VT_EXL_REQ_Write (struct MSG_LC_AUTH_VT_EXL_REQ *p_auth_vt_exl_req, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write (p_auth_vt_exl_req, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_LC_AUTH_VT_EXL_REQ
PUBLIC BOOL MSG_LC_AUTH_VT_EXL_REQ_Read (struct MSG_LC_AUTH_VT_EXL_REQ *p_auth_vt_exl_req, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_auth_vt_exl_req, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_LC_AUTH_VT_EXL_REQ_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_HEADER_Compare_CD, 
                          (PVOID) MSG_LC_AUTH_VT_EXL_REQ_CD, 
 			              MSG_LC_AUTH_VT_EXL_REQ, 
                          sizeof(struct MSG_LC_AUTH_VT_EXL_REQ), "MSG_LC_AUTH_VT_EXL_REQ");
}

