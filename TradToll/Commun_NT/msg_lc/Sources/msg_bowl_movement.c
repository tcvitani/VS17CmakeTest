/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_BOWL_MOVEMENT.C                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_bowl_movement.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

//5 is number of message elements
#define MSG_BOWL_MOVEMENT_NB_FIELDS MSG_HEADER_NB_FIELDS + 6 

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE
// fonction de customisation en écriture du champ "Sign of movement" 
// de la struct de liste "Filling Information"
PRIVATE BOOL Filling_Sign_Write(LONG *sign, HMSG hRootMsg)
{
    BOOL ret;

    // symbolise >= 0 par '+' et <0 par '-'
    if(*sign < 0)
        ret = MSG_Write_String(hRootMsg, "-", 1UL, 1UL);
    else
        ret = MSG_Write_String(hRootMsg, "+", 1UL, 1UL);

    return ret;
}

// fonction de customisation en lecture du champ "Sign of movement" 
// de la struct de liste "Filling Information"
PRIVATE BOOL Filling_Sign_Read(LONG *sign, HMSG hRefMsg)
{
	CHAR ascii_sign[2] = {0};

    if (MSG_Read_String(hRefMsg, ascii_sign, 1, 1) == FALSE)
        return FALSE;
    
    // convertit '+' par 1L et '-' par -1L
    switch(ascii_sign[0])
    {
    case '-':
        *sign = -1L;
        break;

    case '+':
        *sign = 1L;
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_BOWL_MOVEMENT, MSG_BOWL_MOVEMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_BOWL_MOVEMENT_Filling_Information, MSG_BOWL_MOVEMENT_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_BOWL_MOVEMENT_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_BOWL_MOVEMENT[MSG_BOWL_MOVEMENT_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_BOWL_MOVEMENT, MSG_BOWL_MOVEMENT_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_BOWL_MOVEMENT, body.time_of_movement) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_BOWL_MOVEMENT, body.agent_identifier),                     0UL,            999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_BOWL_MOVEMENT, body.movement_type),                        0UL,            9UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_BOWL_MOVEMENT, list_fill_informations), MSG_BOWL_MOVEMENT_Filling_Information, MSG_SIZEOF(MSG_BOWL_MOVEMENT_Filling_Information), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_FILLING_INFORMATION[MSG_BOWL_MOVEMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_BOWL_MOVEMENT_Filling_Information, bowl_position),                        0UL,            9UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_BOWL_MOVEMENT_Filling_Information, coin_id),                  0UL,           99UL },
        { MSG_FIELD_CUSTOM, MSG_OFFSET(MSG_BOWL_MOVEMENT_Filling_Information, sign), Filling_Sign_Write, Filling_Sign_Read, NULL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_BOWL_MOVEMENT_Filling_Information, coin_counter),                  0UL,     99999999UL },

        { MSG_FIELD_STOP }
    };

    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_BOWL_MOVEMENT,							 LOCAL_MSG_BOWL_MOVEMENT);
    MSG_INIT_REF(MSG_BOWL_MOVEMENT_Filling_Information,      LOCAL_FILLING_INFORMATION);

}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_BOWL_MOVEMENT *MSG_BOWL_MOVEMENT_New(void)
{
    // la variable MSG_BOWL_MOVEMENT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_BOWL_MOVEMENT, sizeof(struct MSG_BOWL_MOVEMENT), "MSG_BOWL_MOVEMENT");
}

// constructeur d'un element de liste de type "sold product"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_BOWL_MOVEMENT" préalablement alloué par MSG_BOWL_MOVEMENT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_BOWL_MOVEMENT_Filling_Information *MSG_BOWL_MOVEMENT_Filling_Information_New(HLIST *list_fill_informations)
{
    // la variable MSG_BOWL_MOVEMENT_Filling_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_fill_informations, MSG_BOWL_MOVEMENT_Filling_Information, sizeof(struct MSG_BOWL_MOVEMENT_Filling_Information));
}

// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_BOWL_MOVEMENT_Delete_All(struct MSG_BOWL_MOVEMENT *p_bowl_movement)
{
    return MSG_Delete_All(&p_bowl_movement);
}

// ITERATEURS

PUBLIC struct MSG_BOWL_MOVEMENT_Filling_Information *MSG_BOWL_MOVEMENT_Get_First_Filling_Information(HLIST list_fill_informations)
{
    return MSG_Get_First(list_fill_informations);
}

PUBLIC struct MSG_BOWL_MOVEMENT_Filling_Information *MSG_BOWL_MOVEMENT_Get_Next_Filling_Information(HLIST list_fill_informations, struct MSG_BOWL_MOVEMENT_Filling_Information *p_movement)
{
    return MSG_Get_Next(list_fill_informations, p_movement);
}


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_BOWL_MOVEMENT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_BOWL_MOVEMENT de MSG_BOWL_MOVEMENT_New
// et MSG_BOWL_MOVEMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_BOWL_MOVEMENT_Write(struct MSG_BOWL_MOVEMENT *p_bowl_movement, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_bowl_movement, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_BOWL_MOVEMENT
PUBLIC BOOL MSG_BOWL_MOVEMENT_Read(struct MSG_BOWL_MOVEMENT *p_bowl_movement, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_bowl_movement, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_BOWL_MOVEMENT_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_BOWL_MOVEMENT_CD, 
                          MSG_BOWL_MOVEMENT, 
                          sizeof(struct MSG_BOWL_MOVEMENT), "MSG_BOWL_MOVEMENT");
}

/*-------------------------------- END OF FILE ------------------------------*/