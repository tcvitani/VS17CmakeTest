/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_AUTH_REQ.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv_auth_req.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_AUTH_REQ_NB_FIELDS MSG_PV_HEADER_NB_FIELDS + 5 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF (MSG_PV_AUTH_REQ, MSG_PV_AUTH_REQ_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PV_AUTH_REQ_Init(void)
{ 
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PV_AUTH_REQ[MSG_PV_AUTH_REQ_NB_FIELDS] =  
    {
        { MSG_FIELD_PV_HEADER (MSG_PV_AUTH_REQ, MSG_PV_AUTH_REQ_CD) },

        { MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_PV_AUTH_REQ, body.name),      0UL,           MSG_PV_MAX_USER_NAME - 1UL },
        { MSG_FIELD_DWORD,	 MSG_OFFSET(MSG_PV_AUTH_REQ, body.matricule), 0UL,           999999UL },
		{ MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_PV_AUTH_REQ, body.password),  0UL,           MSG_PV_MAX_USER_PASSWORD - 1UL },
		{ MSG_FIELD_DWORD,   MSG_OFFSET(MSG_PV_AUTH_REQ, body.type),  0UL,           9UL },
		{ MSG_FIELD_DWORD,   MSG_OFFSET(MSG_PV_AUTH_REQ, body.id),  0UL,           99UL },
		
        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_PV_AUTH_REQ,                    LOCAL_MSG_PV_AUTH_REQ);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_PV_AUTH_REQ *MSG_PV_AUTH_REQ_New (void)
{
    // la variable MSG_PV_AUTH_REQ de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_PV_AUTH_REQ, sizeof(struct MSG_PV_AUTH_REQ), "MSG_PV_AUTH_REQ");
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_PV_AUTH_REQ_Delete_All (struct MSG_PV_AUTH_REQ *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PV_AUTH_REQ"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_PV_AUTH_REQ de MSG_PV_AUTH_REQ_New
// et MSG_PV_AUTH_REQ_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PV_AUTH_REQ_Write (struct MSG_PV_AUTH_REQ *p_ptr, 
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
// de type struct MSG_PV_AUTH_REQ
PUBLIC BOOL MSG_PV_AUTH_REQ_Read (struct MSG_PV_AUTH_REQ *p_ptr, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PV_AUTH_REQ_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
                          (MSG_Compare) MSG_PV_HEADER_Compare_CD, 
                          (PVOID) MSG_PV_AUTH_REQ_CD, 
                          MSG_PV_AUTH_REQ, 
                          sizeof(struct MSG_PV_AUTH_REQ),
						  "MSG_PV_AUTH_REQ");
}

/*-------------------------------- END OF FILE ------------------------------*/
