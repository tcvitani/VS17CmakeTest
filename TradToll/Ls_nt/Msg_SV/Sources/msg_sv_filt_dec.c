/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV_FILT_DEC.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_sv_filt_dec.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_SV_FILT_DEC_NB_FIELDS MSG_SV_HEADER_NB_FIELDS + 1 + 1
#define MSG_SV_FILT_DEC_IDCD_NB_FIELDS 2 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_SV_FILT_DEC, MSG_SV_FILT_DEC_NB_FIELDS)
MSG_DECLARE_REF(MSG_SV_FILT_DEC_Idcd, MSG_SV_FILT_DEC_IDCD_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_SV_FILT_DEC_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_SV_FILT_DEC[MSG_SV_FILT_DEC_NB_FIELDS] =  
    {
        { MSG_FIELD_SV_HEADER (MSG_SV_FILT_DEC, MSG_SV_FILT_DEC_CD) },

		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_SV_FILT_DEC, list_idcd),	MSG_SV_FILT_DEC_Idcd, MSG_SIZEOF(MSG_SV_FILT_DEC_Idcd),	0UL, 9999UL },
		
        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_IDCD[MSG_SV_FILT_DEC_IDCD_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,	MSG_OFFSET(MSG_SV_FILT_DEC_Idcd, id),	0UL,	999UL },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_SV_FILT_DEC_Idcd, cd),	0UL,    999UL },
		
        { MSG_FIELD_STOP }
    };
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_SV_FILT_DEC,            LOCAL_MSG_SV_FILT_DEC);
    MSG_INIT_REF (MSG_SV_FILT_DEC_Idcd,		LOCAL_IDCD);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_SV_FILT_DEC *MSG_SV_FILT_DEC_New (void)
{
    // la variable MSG_SV_FILT_DEC de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_SV_FILT_DEC, sizeof(struct MSG_SV_FILT_DEC), "MSG_SV_FILT_DEC");
}

// constructeur d'un element de liste de type "idcd"
// le pointeur de liste est normalement l'@ du champ "list_idcd" d'un
// message de type "struct MSG_SV_FILT_DEC" préalablement alloué par MSG_SV_FILT_DEC_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_SV_FILT_DEC_Idcd *MSG_SV_FILT_DEC_Idcd_New (HLIST *list)
{
    // la variable MSG_SV_FILT_DEC_Idcd de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_SV_FILT_DEC_Idcd, sizeof(struct MSG_SV_FILT_DEC_Idcd));
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_SV_FILT_DEC_Delete_All (struct MSG_SV_FILT_DEC *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// ITERATEURS

PUBLIC struct MSG_SV_FILT_DEC_Idcd *MSG_SV_FILT_DEC_Get_First_Idcd (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_SV_FILT_DEC_Idcd *MSG_SV_FILT_DEC_Get_Next_Idcd (HLIST list, struct MSG_SV_FILT_DEC_Idcd *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_SV_FILT_DEC"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_SV_FILT_DEC de MSG_SV_FILT_DEC_New
// et MSG_SV_FILT_DEC_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_SV_FILT_DEC_Write (struct MSG_SV_FILT_DEC *p_ptr, 
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
// de type struct MSG_SV_FILT_DEC
PUBLIC BOOL MSG_SV_FILT_DEC_Read (struct MSG_SV_FILT_DEC *p_ptr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_SV_FILT_DEC_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_SV_HEADER_Compare_CD, 
                          (PVOID) MSG_SV_FILT_DEC_CD, 
                          MSG_SV_FILT_DEC, 
                          sizeof(struct MSG_SV_FILT_DEC),
						  "MSG_SV_FILT_DEC");
}

/*-------------------------------- END OF FILE ------------------------------*/