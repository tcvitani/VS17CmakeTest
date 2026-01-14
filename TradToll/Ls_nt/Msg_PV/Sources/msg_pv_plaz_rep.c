/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_PLAZ_REP.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv_plaz_rep.h>
 
/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_PLAZ_REP_NB_FIELDS MSG_PV_HEADER_NB_FIELDS + 4 + 1
#define MSG_PV_PLAZ_REP_LANE_NB_FIELDS 1 + 1
#define MSG_PV_PLAZ_REP_ALARM_NB_FIELDS 1 + 1
#define MSG_PV_PLAZ_REP_DISPLAY_NB_FIELDS 6 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_PV_PLAZ_REP, MSG_PV_PLAZ_REP_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_PLAZ_REP_Lane, MSG_PV_PLAZ_REP_LANE_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_PLAZ_REP_Alarm, MSG_PV_PLAZ_REP_ALARM_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_PLAZ_REP_Display, MSG_PV_PLAZ_REP_DISPLAY_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PV_PLAZ_REP_Init(HLIST msg_pv_record)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PV_PLAZ_REP[MSG_PV_PLAZ_REP_NB_FIELDS] =  
    {
        { MSG_FIELD_PV_HEADER (MSG_PV_PLAZ_REP, MSG_PV_PLAZ_REP_CD) },

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP, body.plazanum),				0UL,	9999UL },

		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_PV_PLAZ_REP, list_lane),	MSG_PV_PLAZ_REP_Lane, MSG_SIZEOF(MSG_PV_PLAZ_REP_Lane),	0UL, 99UL },
		
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_PV_PLAZ_REP, list_alarm),	MSG_PV_PLAZ_REP_Alarm, MSG_SIZEOF(MSG_PV_PLAZ_REP_Alarm),	0UL, 99UL },
		
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_PV_PLAZ_REP, list_display),	MSG_PV_PLAZ_REP_Display, MSG_SIZEOF(MSG_PV_PLAZ_REP_Display),	0UL, 9UL },
		
        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_LANE[MSG_PV_PLAZ_REP_LANE_NB_FIELDS] =  
    {
        { MSG_FIELD_INCLUDE,MSG_OFFSET(MSG_PV_PLAZ_REP_Lane, msg_pv_lane_rep), msg_pv_record },
		
        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_ALARM[MSG_PV_PLAZ_REP_ALARM_NB_FIELDS] =  
    {
        { MSG_FIELD_INCLUDE,MSG_OFFSET(MSG_PV_PLAZ_REP_Alarm, msg_pv_alar_rep), msg_pv_record },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_DISPLAY[MSG_PV_PLAZ_REP_DISPLAY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, id),		0UL,	99UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, total1),	0UL,    99999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, total2),	0UL,    99999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, total3),	0UL,    99999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, total4),	0UL,    99999999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_PV_PLAZ_REP_Display, total5),	0UL,    99999999UL },
		
        { MSG_FIELD_STOP }
    };
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_PV_PLAZ_REP,              LOCAL_MSG_PV_PLAZ_REP);
    MSG_INIT_REF (MSG_PV_PLAZ_REP_Lane,			LOCAL_LANE);
	MSG_INIT_REF (MSG_PV_PLAZ_REP_Alarm,		LOCAL_ALARM);
	MSG_INIT_REF (MSG_PV_PLAZ_REP_Display,      LOCAL_DISPLAY);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_PV_PLAZ_REP *MSG_PV_PLAZ_REP_New (void)
{
    // la variable MSG_PV_PLAZ_REP de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_PV_PLAZ_REP, sizeof(struct MSG_PV_PLAZ_REP), "MSG_PV_PLAZ_REP");
}

// constructeur d'un element de liste de type "lane"
// le pointeur de liste est normalement l'@ du champ "list_lane" d'un
// message de type "struct MSG_PV_PLAZ_REP" préalablement alloué par MSG_PV_PLAZ_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_PLAZ_REP_Lane *MSG_PV_PLAZ_REP_Lane_New (HLIST *list)
{
    // la variable MSG_PV_PLAZ_REP_Lane de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_PLAZ_REP_Lane, sizeof(struct MSG_PV_PLAZ_REP_Lane));
}

// constructeur d'un element de liste de type "alarm"
// le pointeur de liste est normalement l'@ du champ "list_alarm" d'un
// message de type "struct MSG_PV_PLAZ_REP" préalablement alloué par MSG_PV_PLAZ_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_PLAZ_REP_Alarm *MSG_PV_PLAZ_REP_Alarm_New (HLIST *list)
{
    // la variable MSG_PV_PLAZ_REP_Alarm de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_PLAZ_REP_Alarm, sizeof(struct MSG_PV_PLAZ_REP_Alarm));
}

// constructeur d'un element de liste de type "display"
// le pointeur de liste est normalement l'@ du champ "list_display" d'un
// message de type "struct MSG_PV_PLAZ_REP" préalablement alloué par MSG_PV_PLAZ_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_PLAZ_REP_Display *MSG_PV_PLAZ_REP_Display_New (HLIST *list)
{
    // la variable MSG_PV_PLAZ_REP_Display de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_PLAZ_REP_Display, sizeof(struct MSG_PV_PLAZ_REP_Display));
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_PV_PLAZ_REP_Delete_All (struct MSG_PV_PLAZ_REP *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// ITERATEURS

PUBLIC struct MSG_PV_PLAZ_REP_Lane *MSG_PV_PLAZ_REP_Get_First_Lane (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_PLAZ_REP_Lane *MSG_PV_PLAZ_REP_Get_Next_Lane (HLIST list, struct MSG_PV_PLAZ_REP_Lane *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_PLAZ_REP_Alarm *MSG_PV_PLAZ_REP_Get_First_Alarm (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_PLAZ_REP_Alarm *MSG_PV_PLAZ_REP_Get_Next_Alarm (HLIST list, struct MSG_PV_PLAZ_REP_Alarm *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_PLAZ_REP_Display *MSG_PV_PLAZ_REP_Get_First_Display (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_PLAZ_REP_Display *MSG_PV_PLAZ_REP_Get_Next_Display (HLIST list, struct MSG_PV_PLAZ_REP_Display *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PV_PLAZ_REP"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_PV_PLAZ_REP de MSG_PV_PLAZ_REP_New
// et MSG_PV_PLAZ_REP_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PV_PLAZ_REP_Write (struct MSG_PV_PLAZ_REP *p_ptr, 
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
// de type struct MSG_PV_PLAZ_REP
PUBLIC BOOL MSG_PV_PLAZ_REP_Read (struct MSG_PV_PLAZ_REP *p_ptr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PV_PLAZ_REP_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_PV_HEADER_Compare_CD, 
                          (PVOID) MSG_PV_PLAZ_REP_CD, 
                          MSG_PV_PLAZ_REP, 
                          sizeof(struct MSG_PV_PLAZ_REP),
						  "MSG_PV_PLAZ_REP");
}

/*-------------------------------- END OF FILE ------------------------------*/
