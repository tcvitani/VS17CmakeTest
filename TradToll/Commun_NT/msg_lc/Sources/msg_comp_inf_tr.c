/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COMP_INF_TR.C											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_comp_inf_tr.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_COMP_INF_TR_NB_FIELDS MSG_HEADER_NB_FIELDS + 6

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_COMP_INF_TR, MSG_COMP_INF_TR_NB_FIELDS)
MSG_DECLARE_REF(MSG_COMP_INF_TR_Info_Type, MSG_COMP_INF_TR_NB_FIELDS)
MSG_DECLARE_REF(MSG_COMP_INF_TR_Elem_Info, MSG_COMP_INF_TR_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_COMP_INF_TR_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_COMP_INF_TR[MSG_COMP_INF_TR_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_COMP_INF_TR, MSG_COMP_INF_TR_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_COMP_INF_TR, body.time_of_comp_inf) },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COMP_INF_TR, ref_trs.internal_id),                    0UL,        99999UL },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_COMP_INF_TR, ref_trs.start_time) },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_COMP_INF_TR, list_comp_inf), MSG_COMP_INF_TR_Info_Type, MSG_SIZEOF(MSG_COMP_INF_TR_Info_Type), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_INFO_TYPE[MSG_COMP_INF_TR_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COMP_INF_TR_Info_Type, type),                        0UL,           99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_COMP_INF_TR_Info_Type, list_info_type), MSG_COMP_INF_TR_Elem_Info, MSG_SIZEOF(MSG_COMP_INF_TR_Elem_Info), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ELEM_INFO[MSG_COMP_INF_TR_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COMP_INF_TR_Elem_Info, code),                   0UL,           99UL },
        { MSG_FIELD_VARIANT,    MSG_OFFSET(MSG_COMP_INF_TR_Elem_Info, info_value), 0UL, MSG_COMP_INF_TR_INFO_VALUE_LENGTH },

        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_COMP_INF_TR,					 LOCAL_MSG_COMP_INF_TR);
    MSG_INIT_REF(MSG_COMP_INF_TR_Info_Type,		 LOCAL_INFO_TYPE);
    MSG_INIT_REF(MSG_COMP_INF_TR_Elem_Info,		 LOCAL_ELEM_INFO);
  }

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_COMP_INF_TR *MSG_COMP_INF_TR_New(void)
{
    // la variable MSG_PAYMENT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_COMP_INF_TR, sizeof(struct MSG_COMP_INF_TR), "MSG_COMP_INF_TR");
}

// constructeur d'un element de liste de type "sold product"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_PAYMENT" préalablement alloué par MSG_PAYMENT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_COMP_INF_TR_Info_Type *MSG_COMP_INF_TR_Info_Type_New(HLIST *list_info_type)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_info_type, MSG_COMP_INF_TR_Info_Type, sizeof(struct MSG_COMP_INF_TR_Info_Type));
}

// constructeur d'un element de liste de type "official currency"
// le pointeur de liste est normalement l'@ du champ "list_official_currencies" d'un
// autre element de liste(imbrication) de type "struct MSG_PAYMENT_Sold_Product" 
// préalablement alloué par MSG_PAYMENT_Sold_Product_New
// rem : l'allocation du handle de liste est automatique

PUBLIC struct MSG_COMP_INF_TR_Elem_Info *MSG_COMP_INF_TR_Elem_Info_New(HLIST *list_comp_inf)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_comp_inf, MSG_COMP_INF_TR_Elem_Info, sizeof(struct MSG_COMP_INF_TR_Elem_Info));
}

// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_COMP_INF_TR_Delete_All(struct MSG_COMP_INF_TR *p_comp_inf_tr)
{
    return MSG_Delete_All(&p_comp_inf_tr);
}

// ITERATEURS

PUBLIC struct MSG_COMP_INF_TR_Info_Type *MSG_COMP_INF_TR_Get_First_Info_Type(HLIST list_info_type)
{
    return MSG_Get_First(list_info_type);
}

PUBLIC struct MSG_COMP_INF_TR_Info_Type *MSG_COMP_INF_TR_Get_Next_Info_Type(HLIST list_info_type, struct MSG_COMP_INF_TR_Info_Type *p_type)
{
    return MSG_Get_Next(list_info_type, p_type);
}


PUBLIC struct MSG_COMP_INF_TR_Elem_Info *MSG_COMP_INF_TR_Get_First_Elem_Info(HLIST list_comp_inf)
{
    return MSG_Get_First(list_comp_inf);
}

PUBLIC struct MSG_COMP_INF_TR_Elem_Info *MSG_COMP_INF_TR_Get_Next_Elem_Info(HLIST list_comp_inf, struct MSG_COMP_INF_TR_Elem_Info *p_info)
{
    return MSG_Get_Next(list_comp_inf, p_info);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PAYMENT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_PAYMENT de MSG_PAYMENT_New
// et MSG_PAYMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_COMP_INF_TR_Write(struct MSG_COMP_INF_TR *p_comp_inf_tr, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_comp_inf_tr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_PAYMENT
PUBLIC BOOL MSG_COMP_INF_TR_Read(struct MSG_COMP_INF_TR *p_comp_inf_tr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_comp_inf_tr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_COMP_INF_TR_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_COMP_INF_TR_CD, 
	                      MSG_COMP_INF_TR, 
                          sizeof(struct MSG_COMP_INF_TR), "MSG_COMP_INF_TR");
}

/*-------------------------------- END OF FILE ------------------------------*/