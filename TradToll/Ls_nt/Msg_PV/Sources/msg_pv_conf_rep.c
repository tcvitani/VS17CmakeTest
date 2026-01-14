/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_CONF_REP.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv_conf_rep.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_CONF_REP_NB_FIELDS MSG_PV_HEADER_NB_FIELDS + 4 + 1
#define MSG_PV_CONF_REP_LANE_NB_FIELDS 4 + 1
#define MSG_PV_CONF_REP_EQUIPMENT_NB_FIELDS 1 + 1
#define MSG_PV_CONF_REP_DISPLAY_NB_FIELDS 2 + 1
#define MSG_PV_CONF_REP_ZONE_NB_FIELDS 2 + 1
#define MSG_PV_CONF_REP_LANENUM_NB_FIELDS 1 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_PV_CONF_REP, MSG_PV_CONF_REP_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_CONF_REP_Lane, MSG_PV_CONF_REP_LANE_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_CONF_REP_Equipment, MSG_PV_CONF_REP_EQUIPMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_CONF_REP_Display, MSG_PV_CONF_REP_DISPLAY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_CONF_REP_Zone, MSG_PV_CONF_REP_ZONE_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_CONF_REP_Lanenum, MSG_PV_CONF_REP_LANENUM_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PV_CONF_REP_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PV_CONF_REP[MSG_PV_CONF_REP_NB_FIELDS] =  
    {
        { MSG_FIELD_PV_HEADER (MSG_PV_CONF_REP, MSG_PV_CONF_REP_CD) },

		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_CONF_REP, body.plazanum),		0UL,	9999UL },
        { MSG_FIELD_VARSTR, MSG_OFFSET(MSG_PV_CONF_REP, body.name),	0UL,	MSG_PV_MAX_PLAZA_NAME - 1UL },
	
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_CONF_REP, list_lane), MSG_PV_CONF_REP_Lane, MSG_SIZEOF(MSG_PV_CONF_REP_Lane), 0UL, 99UL },
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_CONF_REP, list_display), MSG_PV_CONF_REP_Display, MSG_SIZEOF(MSG_PV_CONF_REP_Display), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_LANE[MSG_PV_CONF_REP_LANE_NB_FIELDS] =  
    {
		{ MSG_FIELD_DWORD,	 MSG_OFFSET(MSG_PV_CONF_REP_Lane, lanenum),		0UL,	9999UL },
		{ MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_PV_CONF_REP_Lane, name),	0UL,	MSG_PV_MAX_LANE_NAME - 1UL },
		{ MSG_FIELD_DWORD,	 MSG_OFFSET(MSG_PV_CONF_REP_Lane, type),	0UL,	9UL },

		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_CONF_REP_Lane, list_equipment), MSG_PV_CONF_REP_Equipment, MSG_SIZEOF(MSG_PV_CONF_REP_Equipment), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };


	union MSG_Field LOCAL_EQUIPMENT[MSG_PV_CONF_REP_EQUIPMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_CONF_REP_Equipment, id),		0UL,	99UL },
        
        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_DISPLAY[MSG_PV_CONF_REP_DISPLAY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_CONF_REP_Display, id),	0UL,	99UL },
        
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_CONF_REP_Display, list_zone), MSG_PV_CONF_REP_Zone, MSG_SIZEOF(MSG_PV_CONF_REP_Zone), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ZONE[MSG_PV_CONF_REP_ZONE_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_CONF_REP_Zone, id),	0UL,	99UL },

		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_CONF_REP_Zone, list_lanenum), MSG_PV_CONF_REP_Lanenum, MSG_SIZEOF(MSG_PV_CONF_REP_Lanenum), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_LANENUM[MSG_PV_CONF_REP_LANENUM_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_CONF_REP_Lanenum, id),	0UL,	9999UL },

        { MSG_FIELD_STOP }
    };
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_PV_CONF_REP,              LOCAL_MSG_PV_CONF_REP);
    MSG_INIT_REF (MSG_PV_CONF_REP_Lane,			LOCAL_LANE);
	MSG_INIT_REF (MSG_PV_CONF_REP_Equipment,	LOCAL_EQUIPMENT);
    MSG_INIT_REF (MSG_PV_CONF_REP_Display,		LOCAL_DISPLAY);
    MSG_INIT_REF (MSG_PV_CONF_REP_Zone,			LOCAL_ZONE);
	MSG_INIT_REF (MSG_PV_CONF_REP_Lanenum,		LOCAL_LANENUM);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_PV_CONF_REP *MSG_PV_CONF_REP_New (void)
{
    // la variable MSG_PV_CONF_REP de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_PV_CONF_REP, sizeof(struct MSG_PV_CONF_REP), "MSG_PV_CONF_REP");
}

// constructeur d'un element de liste de type "lane"
// le pointeur de liste est normalement l'@ du champ "list_lane" d'un
// message de type "struct MSG_PV_CONF_REP" préalablement alloué par MSG_PV_CONF_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_CONF_REP_Lane *MSG_PV_CONF_REP_Lane_New (HLIST *list)
{
    // la variable MSG_PV_CONF_REP_Lane de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_CONF_REP_Lane, sizeof(struct MSG_PV_CONF_REP_Lane));
}

// constructeur d'un element de liste de type "equipment"
// le pointeur de liste est normalement l'@ du champ "list_equipment" d'un
// message de type "struct MSG_PV_CONF_REP" préalablement alloué par MSG_PV_CONF_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_CONF_REP_Equipment *MSG_PV_CONF_REP_Equipment_New (HLIST *list)
{
    // la variable MSG_PV_CONF_REP_Command de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_CONF_REP_Equipment, sizeof(struct MSG_PV_CONF_REP_Equipment));
}

// constructeur d'un element de liste de type "display"
// le pointeur de liste est normalement l'@ du champ "list_display" d'un
// message de type "struct MSG_PV_CONF_REP" préalablement alloué par MSG_PV_CONF_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_CONF_REP_Display *MSG_PV_CONF_REP_Display_New (HLIST *list)
{
    // la variable MSG_PV_CONF_REP_Command de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_CONF_REP_Display, sizeof(struct MSG_PV_CONF_REP_Display));
}

// constructeur d'un element de liste de type "Zone"
// le pointeur de liste est normalement l'@ du champ "list_zone" d'un
// autre element de liste (imbrication) de type "struct MSG_PV_CONF_REP_Display" 
// préalablement alloué par MSG_PV_CONF_REP_Display_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_CONF_REP_Zone *MSG_PV_CONF_REP_Zone_New (HLIST *list)
{
    // la variable MSG_PV_CONF_REP_Equipment de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_CONF_REP_Zone, sizeof(struct MSG_PV_CONF_REP_Zone));
}

// constructeur d'un element de liste de type "Lanenum"
// le pointeur de liste est normalement l'@ du champ "list_lanenum" d'un
// autre element de liste (imbrication) de type "struct MSG_PV_CONF_REP_Zone" 
// préalablement alloué par MSG_PV_CONF_REP_Zone_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_CONF_REP_Lanenum *MSG_PV_CONF_REP_Lanenum_New (HLIST *list)
{
    // la variable MSG_PV_CONF_REP_Equipment de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_CONF_REP_Lanenum, sizeof(struct MSG_PV_CONF_REP_Lanenum));
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_PV_CONF_REP_Delete_All (struct MSG_PV_CONF_REP *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// ITERATEURS

PUBLIC struct MSG_PV_CONF_REP_Lane *MSG_PV_CONF_REP_Get_First_Lane (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_CONF_REP_Lane *MSG_PV_CONF_REP_Get_Next_Lane (HLIST list, struct MSG_PV_CONF_REP_Lane *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_CONF_REP_Equipment *MSG_PV_CONF_REP_Get_First_Equipment (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_CONF_REP_Equipment *MSG_PV_CONF_REP_Get_Next_Equipment (HLIST list, struct MSG_PV_CONF_REP_Equipment *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_CONF_REP_Display *MSG_PV_CONF_REP_Get_First_Display (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_CONF_REP_Display *MSG_PV_CONF_REP_Get_Next_Display (HLIST list, struct MSG_PV_CONF_REP_Display *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_CONF_REP_Zone *MSG_PV_CONF_REP_Get_First_Zone (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_CONF_REP_Zone *MSG_PV_CONF_REP_Get_Next_Zone (HLIST list, struct MSG_PV_CONF_REP_Zone *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_CONF_REP_Lanenum *MSG_PV_CONF_REP_Get_First_Lanenum (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_CONF_REP_Lanenum *MSG_PV_CONF_REP_Get_Next_Lanenum (HLIST list, struct MSG_PV_CONF_REP_Lanenum *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PV_CONF_REP"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_PV_CONF_REP de MSG_PV_CONF_REP_New
// et MSG_PV_CONF_REP_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PV_CONF_REP_Write (struct MSG_PV_CONF_REP *p_ptr, 
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
// de type struct MSG_PV_CONF_REP
PUBLIC BOOL MSG_PV_CONF_REP_Read (struct MSG_PV_CONF_REP *p_ptr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PV_CONF_REP_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_PV_HEADER_Compare_CD, 
                          (PVOID) MSG_PV_CONF_REP_CD, 
                          MSG_PV_CONF_REP, 
                          sizeof(struct MSG_PV_CONF_REP),
						  "MSG_PV_CONF_REP");
}

/*-------------------------------- END OF FILE ------------------------------*/
