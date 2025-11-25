/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_COUNTER_STATUS.C										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_counter_status.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

//5 is number of message elements
#define MSG_COUNTER_STATUS_NB_FIELDS MSG_HEADER_NB_FIELDS + 3 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_COUNTER_STATUS, MSG_COUNTER_STATUS_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_COUNTER_STATUS_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_COUNTER_STATUS[MSG_COUNTER_STATUS_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_COUNTER_STATUS, MSG_COUNTER_STATUS_CD) },
        
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COUNTER_STATUS, body.total_traffic),               0UL,   99999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COUNTER_STATUS, body.total_violation),             0UL,   99999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_COUNTER_STATUS, body.reset),			              0UL,   1UL },

        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_COUNTER_STATUS,							 LOCAL_MSG_COUNTER_STATUS);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// creating of Permission request to send file message
PUBLIC struct MSG_COUNTER_STATUS *MSG_COUNTER_STATUS_New(void)
{
    // la variable MSG_COUNTER_STATUS de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_COUNTER_STATUS, sizeof(struct MSG_COUNTER_STATUS), "MSG_COUNTER_STATUS");
}

// complete deletion of message
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_COUNTER_STATUS_Delete_All(struct MSG_COUNTER_STATUS *p_perm_rq_send_file)
{
    return MSG_Delete_All(&p_perm_rq_send_file);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_COUNTER_STATUS"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_COUNTER_STATUS de MSG_COUNTER_STATUS_New
// et MSG_COUNTER_STATUS_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_COUNTER_STATUS_Write(struct MSG_COUNTER_STATUS *p_perm_rq_send_file, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_perm_rq_send_file, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_COUNTER_STATUS
PUBLIC BOOL MSG_COUNTER_STATUS_Read(struct MSG_COUNTER_STATUS *p_perm_rq_send_file, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_perm_rq_send_file, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_COUNTER_STATUS_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_COUNTER_STATUS_CD, 
	                      MSG_COUNTER_STATUS, 
                          sizeof(struct MSG_COUNTER_STATUS), "MSG_COUNTER_STATUS");
}

/*-------------------------------- END OF FILE ------------------------------*/