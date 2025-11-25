/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_END_SHIFT.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_endshift.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_END_SHIFT_NB_FIELDS MSG_HEADER_NB_FIELDS + 8

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_END_SHIFT, MSG_END_SHIFT_NB_FIELDS)
MSG_DECLARE_REF(MSG_END_SHIFT_Coin_Information, MSG_END_SHIFT_NB_FIELDS)
MSG_DECLARE_REF(MSG_END_SHIFT_Bowl_Information, MSG_END_SHIFT_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_END_SHIFT_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_END_SHIFT[MSG_END_SHIFT_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_END_SHIFT, MSG_END_SHIFT_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_END_SHIFT, body.time_of_end_shift) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT, body.close_type),                    0UL,        9UL },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT, start_ref.start_shift_msg_id),                      0UL,           99999UL },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_END_SHIFT, start_ref.start_shift_time),},

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_END_SHIFT, list_coin_information), MSG_END_SHIFT_Coin_Information, MSG_SIZEOF(MSG_END_SHIFT_Coin_Information), 0UL, 99UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_END_SHIFT, list_bowl_information), MSG_END_SHIFT_Bowl_Information, MSG_SIZEOF(MSG_END_SHIFT_Bowl_Information), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_COIN_INFORMATION[MSG_END_SHIFT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT_Coin_Information, coin_id),                        0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT_Coin_Information, coin_counter),                  0UL,          99999999UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_BOWL_INFORMATION[MSG_END_SHIFT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT_Bowl_Information, bowl_position),                   0UL,      9UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT_Bowl_Information, coin_id),                         0UL,     99UL },
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_END_SHIFT_Bowl_Information, coin_change_counter),             0UL,     99999999UL },

        { MSG_FIELD_STOP }
    };

    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_END_SHIFT,                   LOCAL_MSG_END_SHIFT);
    MSG_INIT_REF(MSG_END_SHIFT_Coin_Information,  LOCAL_COIN_INFORMATION);
    MSG_INIT_REF(MSG_END_SHIFT_Bowl_Information,  LOCAL_BOWL_INFORMATION);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_END_SHIFT *MSG_END_SHIFT_New(void)
{
    // la variable MSG_END_SHIFT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_END_SHIFT, sizeof(struct MSG_END_SHIFT), "MSG_END_SHIFT");
}

// constructeur d'un element de liste de type "sold product"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_END_SHIFT" préalablement alloué par MSG_END_SHIFT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_END_SHIFT_Coin_Information *MSG_END_SHIFT_Coin_Information_New(HLIST *list_coin_information)
{
    // la variable MSG_END_SHIFT_Coin_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_coin_information, MSG_END_SHIFT_Coin_Information, sizeof(struct MSG_END_SHIFT_Coin_Information));
}

// constructeur d'un element de liste de type "anomaly"
// le pointeur de liste est normalement l'@ du champ "list_anomalies" d'un
// message de type "struct MSG_END_SHIFT" préalablement alloué par MSG_END_SHIFT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_END_SHIFT_Bowl_Information *MSG_END_SHIFT_Bowl_Information_New(HLIST *list_bowl_information)
{
    // la variable MSG_END_SHIFT_Coin_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_bowl_information, MSG_END_SHIFT_Bowl_Information, sizeof(struct MSG_END_SHIFT_Bowl_Information));
}



// suppression COMPLETE d'un end of shift message
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_END_SHIFT_Delete_All(struct MSG_END_SHIFT *p_endshift)
{
    return MSG_Delete_All(&p_endshift);
}

// ITERATEURS

PUBLIC struct MSG_END_SHIFT_Coin_Information *MSG_END_SHIFT_Get_First_Coin_Information(HLIST list_coin_information)
{
    return MSG_Get_First(list_coin_information);
}

PUBLIC struct MSG_END_SHIFT_Coin_Information *MSG_END_SHIFT_Get_Next_Coin_Information(HLIST list_coin_information, struct MSG_END_SHIFT_Coin_Information *p_coin)
{
    return MSG_Get_Next(list_coin_information, p_coin);
}

PUBLIC struct MSG_END_SHIFT_Bowl_Information *MSG_END_SHIFT_Get_First_Bowl_Information(HLIST list_bowl_information)
{
    return MSG_Get_First(list_bowl_information);
}

PUBLIC struct MSG_END_SHIFT_Bowl_Information *MSG_END_SHIFT_Get_Next_Bowl_Information(HLIST list_bowl_information, struct MSG_END_SHIFT_Bowl_Information *p_bowl)
{
    return MSG_Get_Next(list_bowl_information, p_bowl);
}


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_END_SHIFT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_END_SHIFT de MSG_END_SHIFT_New
// et MSG_END_SHIFT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_END_SHIFT_Write(struct MSG_END_SHIFT *p_endshift, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_endshift, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_END_SHIFT
PUBLIC BOOL MSG_END_SHIFT_Read(struct MSG_END_SHIFT *p_endshift, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_endshift, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Création et lecture d'un message de type MSG_END_SHIFT
//PUBLIC struct MSG_END_SHIFT *MSG_END_SHIFT_New_Read(BYTE *p_msg, DWORD msg_size_max)
//{
//    struct MSG_END_SHIFT *p_endshift;
//    DWORD size;

//   if((p_endshift = MSG_END_SHIFT_New()) == NULL)
//        return NULL;
    
//    if(MSG_END_SHIFT_Read(p_endshift, p_msg, msg_size_max, &size) == FALSE)
//    {
//        MSG_END_SHIFT_Delete_All(p_endshift);
//        return NULL;
//    }

//    return p_endshift;
//}

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_END_SHIFT_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_END_SHIFT_CD, 
		                  MSG_END_SHIFT, 
                          sizeof(struct MSG_END_SHIFT), "MSG_END_SHIFT");
}

/*-------------------------------- END OF FILE ------------------------------*/
