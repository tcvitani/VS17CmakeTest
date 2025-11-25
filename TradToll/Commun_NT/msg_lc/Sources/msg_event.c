/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_EVENT.C													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_event.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_EVENT_NB_FIELDS MSG_HEADER_NB_FIELDS + 7

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_EVENT, MSG_EVENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_EVENT_Event_Description, MSG_EVENT_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_EVENT_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_EVENT[MSG_EVENT_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_EVENT, MSG_EVENT_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_EVENT, body.time_of_event) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_EVENT, body.event_category),          0UL,							999UL	},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_EVENT, body.event_sub_category),      0UL,							99UL	},
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_EVENT, body.current_event_status),    0UL,							99UL	},
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_EVENT, body.list_event_descritpion),	 MSG_EVENT_Event_Description,	MSG_SIZEOF(MSG_EVENT_Event_Description), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_EVENT_DESCRIPTION[MSG_EVENT_NB_FIELDS] =
	{
		{ MSG_FIELD_VARIANT, MSG_OFFSET(MSG_EVENT_Event_Description, description), 0UL, MSG_EVENT_ANOMALY_DESCRIPTION_LENGTH },

		{ MSG_FIELD_STOP }
	};

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_EVENT,						LOCAL_MSG_EVENT);
	MSG_INIT_REF(MSG_EVENT_Event_Description,	LOCAL_EVENT_DESCRIPTION)	
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_EVENT *MSG_EVENT_New(void)
{
    // la variable MSG_EVENT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_EVENT, sizeof(struct MSG_EVENT), "MSG_EVENT");
}

PUBLIC struct MSG_EVENT_Event_Description *MSG_EVENT_Event_Description_New(HLIST *list_anomaly_descritpion)
{
	return MSG_New_List(list_anomaly_descritpion, MSG_EVENT_Event_Description, sizeof(struct MSG_EVENT_Event_Description));
}

PUBLIC struct MSG_EVENT_Event_Description *MSG_EVENT_Get_First_Anomaly_Description(HLIST list_anomaly_description)
{
	return MSG_Get_First(list_anomaly_description);
}

PUBLIC struct MSG_EVENT_Event_Description *MSG_EVENT_Get_Next_Anomaly_Description(HLIST list_anomaly_description, struct MSG_EVENT_Event_Description *p_anomaly_descritpion)
{
	return MSG_Get_Next(list_anomaly_description, p_anomaly_descritpion);
}


PUBLIC struct MSG_EVENT_Anomaly *MSG_EVENT_Get_Next_Anomaly(HLIST list_anomalies, struct MSG_EVENT_Anomaly *p_cur)
{
	return MSG_Get_Next(list_anomalies, p_cur);
}


// suppression COMPLETE d'un message de event
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_EVENT_Delete_All(struct MSG_EVENT *p_event)
{
    return MSG_Delete_All(&p_event);
}

// ITERATEURS


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_EVENT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_EVENT de MSG_EVENT_New
// et MSG_EVENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_EVENT_Write(struct MSG_EVENT *p_event, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_event, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_EVENT
PUBLIC BOOL MSG_EVENT_Read(struct MSG_EVENT *p_event, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_event, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_EVENT_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_EVENT_CD, 
 			              MSG_EVENT, 
                          sizeof(struct MSG_EVENT), "MSG_EVENT");
}

/*-------------------------------- END OF FILE ------------------------------*/
