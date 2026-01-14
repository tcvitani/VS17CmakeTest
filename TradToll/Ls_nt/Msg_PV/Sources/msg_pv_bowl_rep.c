/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_BOWL_REP.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv_bowl_rep.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_BOWL_REP_NB_FIELDS MSG_PV_HEADER_NB_FIELDS + 5 + 1
#define MSG_PV_BOWL_REP_BOWL_NB_FIELDS 8 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_PV_BOWL_REP, MSG_PV_BOWL_REP_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_BOWL_REP_Bowl, MSG_PV_BOWL_REP_BOWL_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PV_BOWL_REP_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PV_BOWL_REP[MSG_PV_BOWL_REP_NB_FIELDS] =  
    {
        { MSG_FIELD_PV_HEADER (MSG_PV_BOWL_REP, MSG_PV_BOWL_REP_CD) },

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP, body.lanenum),				0UL,	9999UL },
		{ MSG_FIELD_TIME,	MSG_OFFSET(MSG_PV_BOWL_REP, body.date) },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP, body.collector_id),			0UL,	999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP, body.active_vault),			0UL,	999999UL },
		
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_PV_BOWL_REP, list_bowl),	MSG_PV_BOWL_REP_Bowl, MSG_SIZEOF(MSG_PV_BOWL_REP_Bowl),	0UL, 9UL },
		
        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_BOWL[MSG_PV_BOWL_REP_BOWL_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, position),					0UL,	9UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, capacity),					0UL,    9999UL },
		{ MSG_FIELD_VARSTR,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, currency_label),			0UL,    MSG_PV_MAX_CURRENCY_LABEL - 1UL },
		{ MSG_FIELD_VARSTR,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, coin_label),				0UL,    MSG_PV_MAX_COIN_LABEL - 1UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, remaining_coin_counter),	0UL,    99999999UL },
		{ MSG_FIELD_FLOAT,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, value),					8UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, bag),						0UL,    99999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_BOWL_REP_Bowl, returned_coin_counter),	0UL,    99999999UL },
		
        { MSG_FIELD_STOP }
    };
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_PV_BOWL_REP,            LOCAL_MSG_PV_BOWL_REP);
    MSG_INIT_REF (MSG_PV_BOWL_REP_Bowl,       LOCAL_BOWL);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_PV_BOWL_REP *MSG_PV_BOWL_REP_New (void)
{
    // la variable MSG_PV_BOWL_REP de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_PV_BOWL_REP, sizeof(struct MSG_PV_BOWL_REP), "MSG_PV_BOWL_REP");
}

// constructeur d'un element de liste de type "bowl"
// le pointeur de liste est normalement l'@ du champ "list_bowl" d'un
// message de type "struct MSG_PV_BOWL_REP" préalablement alloué par MSG_PV_BOWL_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_BOWL_REP_Bowl *MSG_PV_BOWL_REP_Bowl_New (HLIST *list)
{
    // la variable MSG_PV_BOWL_REP_Bowl de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_BOWL_REP_Bowl, sizeof(struct MSG_PV_BOWL_REP_Bowl));
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_PV_BOWL_REP_Delete_All (struct MSG_PV_BOWL_REP *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// ITERATEURS

PUBLIC struct MSG_PV_BOWL_REP_Bowl *MSG_PV_BOWL_REP_Get_First_Bowl (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_BOWL_REP_Bowl *MSG_PV_BOWL_REP_Get_Next_Bowl (HLIST list, struct MSG_PV_BOWL_REP_Bowl *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PV_BOWL_REP"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_PV_BOWL_REP de MSG_PV_BOWL_REP_New
// et MSG_PV_BOWL_REP_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PV_BOWL_REP_Write (struct MSG_PV_BOWL_REP *p_ptr, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_PV_BOWL_REP
PUBLIC BOOL MSG_PV_BOWL_REP_Read (struct MSG_PV_BOWL_REP *p_ptr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PV_BOWL_REP_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_PV_HEADER_Compare_CD, 
                          (PVOID) MSG_PV_BOWL_REP_CD, 
                          MSG_PV_BOWL_REP, 
                          sizeof(struct MSG_PV_BOWL_REP),
						  "MSG_PV_BOWL_REP");
}

/*-------------------------------- END OF FILE ------------------------------*/
