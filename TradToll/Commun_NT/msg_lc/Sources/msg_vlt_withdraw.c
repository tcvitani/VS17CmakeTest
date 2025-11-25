/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_VLT_WITHDRAW.C											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_vlt_withdraw.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_VLT_WITHDRAW_NB_FIELDS MSG_HEADER_NB_FIELDS + 7

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_VLT_WITHDRAW, MSG_VLT_WITHDRAW_NB_FIELDS)
MSG_DECLARE_REF(MSG_VLT_WITHDRAW_Coin_Information, MSG_VLT_WITHDRAW_NB_FIELDS)
MSG_DECLARE_REF(MSG_VLT_WITHDRAW_Bowl_Information, MSG_VLT_WITHDRAW_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_VLT_WITHDRAW_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_VLT_WITHDRAW[MSG_VLT_WITHDRAW_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_VLT_WITHDRAW, MSG_VLT_WITHDRAW_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_VLT_WITHDRAW, body.time_of_vault_withdraw) },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_VLT_WITHDRAW, body.time_of_vault_insertion) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW, body.withdrawn_vault_identifier),                    0UL,        999999UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_VLT_WITHDRAW, list_coin_information), MSG_VLT_WITHDRAW_Coin_Information, MSG_SIZEOF(MSG_VLT_WITHDRAW_Coin_Information), 0UL, 99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_VLT_WITHDRAW, list_bowl_information), MSG_VLT_WITHDRAW_Bowl_Information, MSG_SIZEOF(MSG_VLT_WITHDRAW_Bowl_Information), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_COIN_INFORMATION[MSG_VLT_WITHDRAW_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW_Coin_Information, coin_id),                        0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW_Coin_Information, coin_counter),                  0UL,          99999999UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_BOWL_INFORMATION[MSG_VLT_WITHDRAW_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW_Bowl_Information, bowl_pos),			0UL,           9UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW_Bowl_Information, coin_id),                        0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VLT_WITHDRAW_Bowl_Information, coin_change_counter),                  0UL,          99999999UL },

        { MSG_FIELD_STOP }
    };

   
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_VLT_WITHDRAW,                        LOCAL_MSG_VLT_WITHDRAW);
    MSG_INIT_REF(MSG_VLT_WITHDRAW_Coin_Information,       LOCAL_COIN_INFORMATION);
    MSG_INIT_REF(MSG_VLT_WITHDRAW_Bowl_Information,       LOCAL_BOWL_INFORMATION);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de vault withdraw
PUBLIC struct MSG_VLT_WITHDRAW *MSG_VLT_WITHDRAW_New(void)
{
    // la variable MSG_VLT_WITHDRAW de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_VLT_WITHDRAW, sizeof(struct MSG_VLT_WITHDRAW), "MSG_VLT_WITHDRAW");
}

// constructeur d'un element de liste de type "coin information"
// le pointeur de liste est normalement l'@ du champ "list_coin_information" d'un
// message de type "struct MSG_VLT_WITHDRAW" préalablement alloué par MSG_VLT_WITHDRAW_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_VLT_WITHDRAW_Coin_Information *MSG_VLT_WITHDRAW_Coin_Information_New(HLIST *list_coin_information)
{
    // la variable MSG_VLT_WITHDRAW_Coin_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_coin_information, MSG_VLT_WITHDRAW_Coin_Information, sizeof(struct MSG_VLT_WITHDRAW_Coin_Information));
}

// constructeur d'un element de liste de type "coin information"
// le pointeur de liste est normalement l'@ du champ "list_coin_information" d'un
// message de type "struct MSG_VLT_WITHDRAW" préalablement alloué par MSG_VLT_WITHDRAW_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_VLT_WITHDRAW_Bowl_Information *MSG_VLT_WITHDRAW_Bowl_Information_New(HLIST *list_bowl_information)
{
    // la variable MSG_VLT_WITHDRAW_Coin_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_bowl_information, MSG_VLT_WITHDRAW_Bowl_Information, sizeof(struct MSG_VLT_WITHDRAW_Bowl_Information));
}


// suppression COMPLETE d'un message de vault withdraw
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_VLT_WITHDRAW_Delete_All(struct MSG_VLT_WITHDRAW *p_vlt_withdraw)
{
    return MSG_Delete_All(&p_vlt_withdraw);
}

// ITERATEURS

PUBLIC struct MSG_VLT_WITHDRAW_Coin_Information *MSG_VLT_WITHDRAW_Get_First_Coin_Information(HLIST list_coin_information)
{
    return MSG_Get_First(list_coin_information);
}

PUBLIC struct MSG_VLT_WITHDRAW_Coin_Information *MSG_VLT_WITHDRAW_Get_Next_Coin_Information(HLIST list_coin_information, struct MSG_VLT_WITHDRAW_Coin_Information *p_coin)
{
    return MSG_Get_Next(list_coin_information, p_coin);
}

// ITERATEURS

PUBLIC struct MSG_VLT_WITHDRAW_Bowl_Information *MSG_VLT_WITHDRAW_Get_First_Bowl_Information(HLIST list_bowl_information)
{
    return MSG_Get_First(list_bowl_information);
}

PUBLIC struct MSG_VLT_WITHDRAW_Bowl_Information *MSG_VLT_WITHDRAW_Get_Next_Bowl_Information(HLIST list_bowl_information, struct MSG_VLT_WITHDRAW_Bowl_Information *p_bowl)
{
    return MSG_Get_Next(list_bowl_information, p_bowl);
}


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_VLT_WITHDRAW"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_VLT_WITHDRAW de MSG_VLT_WITHDRAW_New
// et MSG_VLT_WITHDRAW_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_VLT_WITHDRAW_Write(struct MSG_VLT_WITHDRAW *p_vlt_withdraw, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_vlt_withdraw, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_VLT_WITHDRAW
PUBLIC BOOL MSG_VLT_WITHDRAW_Read(struct MSG_VLT_WITHDRAW *p_vlt_withdraw, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_vlt_withdraw, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_VLT_WITHDRAW_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_VLT_WITHDRAW_CD, 
                          MSG_VLT_WITHDRAW, 
                          sizeof(struct MSG_VLT_WITHDRAW), "MSG_VLT_WITHDRAW");
}

/*-------------------------------- END OF FILE ------------------------------*/