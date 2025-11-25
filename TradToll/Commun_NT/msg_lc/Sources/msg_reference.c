/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_REFERENCE.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_reference.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

//5 is number of message elements
#define MSG_REFERENCE_NB_FIELDS MSG_HEADER_NB_FIELDS + 4 

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_REFERENCE, MSG_REFERENCE_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_REFERENCE_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_REFERENCE[MSG_REFERENCE_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_REFERENCE, MSG_REFERENCE_CD) },

		{ MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_REFERENCE,  body.reference_name),  0UL,            MSG_REFERENCE_REFERENCE_NAME_LENGTH  },

        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_REFERENCE,							 LOCAL_MSG_REFERENCE);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// creation of reference message
PUBLIC struct MSG_REFERENCE *MSG_REFERENCE_New(void)
{
    // la variable MSG_REFERENCE de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_REFERENCE, sizeof(struct MSG_REFERENCE), "MSG_REFERENCE");
}

// destroying reference messge
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_REFERENCE_Delete_All(struct MSG_REFERENCE *p_reference)
{
    return MSG_Delete_All(&p_reference);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_REFERENCE"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_REFERENCE de MSG_REFERENCE_New
// et MSG_REFERENCE_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_REFERENCE_Write(struct MSG_REFERENCE *p_reference, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_reference, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_REFERENCE
PUBLIC BOOL MSG_REFERENCE_Read(struct MSG_REFERENCE *p_reference, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_reference, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_REFERENCE_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_REFERENCE_CD, 
	                      MSG_REFERENCE, 
                          sizeof(struct MSG_REFERENCE), "MSG_REFERENCE");
}

/*-------------------------------- END OF FILE ------------------------------*/
