/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_STATUS.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_status.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_STATUS_NB_FIELDS MSG_HEADER_NB_FIELDS + 6

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_STATUS, MSG_STATUS_NB_FIELDS)
MSG_DECLARE_REF(MSG_STATUS_Event_Information, MSG_STATUS_NB_FIELDS)
// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_STATUS_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_STATUS[MSG_STATUS_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_STATUS, MSG_STATUS_CD) },
        
		{ MSG_FIELD_TIME,   MSG_OFFSET(MSG_STATUS, body.time_of_event) },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_STATUS, list_event_info), MSG_STATUS_Event_Information, MSG_SIZEOF(MSG_STATUS_Event_Information), 0UL, 999UL },
       
		{ MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_EVENT_INFORMATION[MSG_STATUS_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_STATUS_Event_Information, event_category),                        0UL,           127UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_STATUS_Event_Information, event_sub_category),                  0UL,          99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_STATUS_Event_Information, current_event_status),			    	0UL,          99UL },
        { MSG_FIELD_STOP }
    };

    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_STATUS,                     LOCAL_MSG_STATUS);
    MSG_INIT_REF(MSG_STATUS_Event_Information,   LOCAL_EVENT_INFORMATION);

}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de status
PUBLIC struct MSG_STATUS *MSG_STATUS_New(void)
{
    // la variable MSG_STATUS de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_STATUS, sizeof(struct MSG_STATUS), "MSG_STATUS");
}

// constructeur d'un element de liste de type "event information"
// le pointeur de liste est normalement l'@ du champ "list_event_info" d'un
// message de type "struct MSG_STATUS" préalablement alloué par MSG_STATUS_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_STATUS_Event_Information *MSG_STATUS_Event_Information_New(HLIST *list_event_info)
{
    // la variable MSG_STATUS_Event_Information de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_event_info, MSG_STATUS_Event_Information, sizeof(struct MSG_STATUS_Event_Information));
}


// suppression COMPLETE d'un message de status
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_STATUS_Delete_All(struct MSG_STATUS *p_status)
{
    return MSG_Delete_All(&p_status);
}

// ITERATEURS

PUBLIC struct MSG_STATUS_Event_Information *MSG_STATUS_Get_First_Event_Information(HLIST list_event_info)
{
    return MSG_Get_First(list_event_info);
}

PUBLIC struct MSG_STATUS_Event_Information *MSG_STATUS_Get_Next_Event_Information(HLIST list_event_info, struct MSG_STATUS_Event_Information *p_event_info)
{
    return MSG_Get_Next(list_event_info, p_event_info);
}



// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_STATUS"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_STATUS de MSG_STATUS_New
// et MSG_STATUS_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_STATUS_Write(struct MSG_STATUS *p_status, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_status, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_STATUS
PUBLIC BOOL MSG_STATUS_Read(struct MSG_STATUS *p_status, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_status, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_STATUS_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_STATUS_CD, 
	                      MSG_STATUS, 
                          sizeof(struct MSG_STATUS), "MSG_STATUS");
}

/*-------------------------------- END OF FILE ------------------------------*/
