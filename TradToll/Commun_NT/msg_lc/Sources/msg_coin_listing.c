/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COIN_LISTING.C											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_coin_listing.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_COIN_LISTING_NB_FIELDS MSG_HEADER_NB_FIELDS + 5

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_COIN_LISTING, MSG_COIN_LISTING_NB_FIELDS)
MSG_DECLARE_REF(MSG_COIN_LISTING_Coin_Information, MSG_COIN_LISTING_NB_FIELDS)
// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_COIN_LISTING_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )

    union MSG_Field LOCAL_MSG_COIN_LISTING[MSG_COIN_LISTING_NB_FIELDS] =
    {
		{ MSG_FIELD_HEADER(MSG_COIN_LISTING, MSG_COIN_LISTING_CD) },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COIN_LISTING, body.rejected_coins),		0UL,    99999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COIN_LISTING, body.currency_id),			0UL,    99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_COIN_LISTING, body.toll_fare),					10UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_COIN_LISTING, list_coin_info), MSG_COIN_LISTING_Coin_Information, MSG_SIZEOF(MSG_COIN_LISTING_Coin_Information), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_COIN_INFORMATION[MSG_COIN_LISTING_NB_FIELDS] =
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COIN_LISTING_Coin_Information, coin_id),         0UL,    99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COIN_LISTING_Coin_Information, coin_counter),	0UL,	99999999UL },

        { MSG_FIELD_STOP }
    };


    #pragma warning( pop )

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_COIN_LISTING,                    LOCAL_MSG_COIN_LISTING);
    MSG_INIT_REF(MSG_COIN_LISTING_Coin_Information,   LOCAL_COIN_INFORMATION);

}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de coin_listing
PUBLIC struct MSG_COIN_LISTING *MSG_COIN_LISTING_New(void)
{
    // la variable MSG_COIN_LISTING de type union MSG_Field[] est globale au module
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_COIN_LISTING, sizeof(struct MSG_COIN_LISTING), "MSG_COIN_LISTING");
}

// constructeur d'un element de liste de type "event information"
// le pointeur de liste est normalement l'@ du champ "list_event_info" d'un
// message de type "struct MSG_COIN_LISTING" préalablement alloué par MSG_COIN_LISTING_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_COIN_LISTING_Coin_Information *MSG_COIN_LISTING_Coin_Information_New(HLIST *list_coin_info)
{
    // la variable MSG_COIN_LISTING_Event_Information de type union MSG_Field[] est globale au module
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_coin_info, MSG_COIN_LISTING_Coin_Information, sizeof(struct MSG_COIN_LISTING_Coin_Information));
}


// suppression COMPLETE d'un message de coin_listing
// Une passe récursive est effectuée sur chaque champ du message
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_COIN_LISTING_Delete_All(struct MSG_COIN_LISTING *p_coin_listing)
{
    return MSG_Delete_All(&p_coin_listing);
}

// ITERATEURS

PUBLIC struct MSG_COIN_LISTING_Coin_Information *MSG_COIN_LISTING_Get_First_Coin_Information(HLIST list_coin_info)
{
    return MSG_Get_First(list_coin_info);
}

PUBLIC struct MSG_COIN_LISTING_Coin_Information *MSG_COIN_LISTING_Get_Next_Coin_Information(HLIST list_coin_info, struct MSG_COIN_LISTING_Coin_Information *p_coin_info)
{
    return MSG_Get_Next(list_coin_info, p_coin_info);
}



// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_COIN_LISTING"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_COIN_LISTING de MSG_COIN_LISTING_New
// et MSG_COIN_LISTING_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_COIN_LISTING_Write(struct MSG_COIN_LISTING *p_coin_listing,
                               BYTE *p_msg,
                               DWORD msg_size_max,
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_coin_listing, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_COIN_LISTING
PUBLIC BOOL MSG_COIN_LISTING_Read(struct MSG_COIN_LISTING *p_coin_listing,
                              BYTE *p_msg,
                              DWORD msg_size_max,
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_coin_listing, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_COIN_LISTING_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList,
                         (MSG_Compare) MSG_HEADER_Compare_CD,
                         (PVOID) MSG_COIN_LISTING_CD,
	                      MSG_COIN_LISTING,
                          sizeof(struct MSG_COIN_LISTING),
						  "MSG_COIN_LISTING");
}

/*-------------------------------- END OF FILE ------------------------------*/