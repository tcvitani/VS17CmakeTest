/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRACE.C													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_trace.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_TRACE_NB_FIELDS MSG_HEADER_NB_FIELDS + 6

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_TRACE, MSG_TRACE_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRACE_Info_Type, MSG_TRACE_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRACE_Elem_Info, MSG_TRACE_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_TRACE_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_TRACE[MSG_TRACE_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_TRACE, MSG_TRACE_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TRACE, body.time_of_trace) },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRACE, list_trace), MSG_TRACE_Info_Type, MSG_SIZEOF(MSG_TRACE_Info_Type), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_INFO_TYPE[MSG_TRACE_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRACE_Info_Type, type),                        0UL,           99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRACE_Info_Type, list_info_type), MSG_TRACE_Elem_Info, MSG_SIZEOF(MSG_TRACE_Elem_Info), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ELEM_INFO[MSG_TRACE_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRACE_Elem_Info, code),                   0UL,           99UL },
        { MSG_FIELD_VARIANT,    MSG_OFFSET(MSG_TRACE_Elem_Info, info_value), 0UL, MSG_TRACE_INFO_VALUE_LENGTH },

        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_TRACE,				 LOCAL_MSG_TRACE);
    MSG_INIT_REF(MSG_TRACE_Info_Type,		 LOCAL_INFO_TYPE);
    MSG_INIT_REF(MSG_TRACE_Elem_Info,		 LOCAL_ELEM_INFO);
  }

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_TRACE *MSG_TRACE_New(void)
{
    // la variable MSG_TRACE de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_TRACE, sizeof(struct MSG_TRACE), "MSG_TRACE");
}

// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_TRACE_Info_Type *MSG_TRACE_Info_Type_New(HLIST *list_info_type)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_info_type, MSG_TRACE_Info_Type, sizeof(struct MSG_TRACE_Info_Type));
}

// rem : l'allocation du handle de liste est automatique

PUBLIC struct MSG_TRACE_Elem_Info *MSG_TRACE_Elem_Info_New(HLIST *list_trace)
{
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_trace, MSG_TRACE_Elem_Info, sizeof(struct MSG_TRACE_Elem_Info));
}

// suppression COMPLETE d'un message de trace
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_TRACE_Delete_All(struct MSG_TRACE *p_trace)
{
    return MSG_Delete_All(&p_trace);
}

// ITERATEURS

PUBLIC struct MSG_TRACE_Info_Type *MSG_TRACE_Get_First_Info_Type(HLIST list_info_type)
{
    return MSG_Get_First(list_info_type);
}

PUBLIC struct MSG_TRACE_Info_Type *MSG_TRACE_Get_Next_Info_Type(HLIST list_info_type, struct MSG_TRACE_Info_Type *p_type)
{
    return MSG_Get_Next(list_info_type, p_type);
}


PUBLIC struct MSG_TRACE_Elem_Info *MSG_TRACE_Get_First_Elem_Info(HLIST list_trace)
{
    return MSG_Get_First(list_trace);
}

PUBLIC struct MSG_TRACE_Elem_Info *MSG_TRACE_Get_Next_Elem_Info(HLIST list_trace, struct MSG_TRACE_Elem_Info *p_info)
{
    return MSG_Get_Next(list_trace, p_info);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_TRACE"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_TRACE de MSG_TRACE_New
// et MSG_TRACE_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_TRACE_Write(struct MSG_TRACE *p_trace, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_trace, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_TRACE
PUBLIC BOOL MSG_TRACE_Read(struct MSG_TRACE *p_trace, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_trace, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_TRACE_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_TRACE_CD, 
	                      MSG_TRACE, 
                          sizeof(struct MSG_TRACE), "MSG_TRACE");
}

/*-------------------------------- END OF FILE ------------------------------*/
