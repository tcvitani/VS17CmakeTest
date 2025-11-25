/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TAB_ACK.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_tab_ack.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_TAB_ACK_NB_FIELDS MSG_HEADER_NB_FIELDS + 6

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_TAB_ACK, MSG_TAB_ACK_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_TAB_ACK_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_TAB_ACK[MSG_TAB_ACK_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_TAB_ACK, MSG_TAB_ACK_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TAB_ACK, body.time_of_tab_ack) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TAB_ACK, body.type),                               0UL,           99UL },
		{ MSG_FIELD_VARSTR,	MSG_OFFSET(MSG_TAB_ACK, body.reference), 0UL,           MSG_TAB_ACK_REFERENCE_NAME_LENGTH },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TAB_ACK, body.update_indicator),                   0UL,            9UL },

        { MSG_FIELD_STOP }
    };

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_TAB_ACK,                    LOCAL_MSG_TAB_ACK);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_TAB_ACK *MSG_TAB_ACK_New(void)
{
    // la variable MSG_TAB_ACK de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_TAB_ACK, sizeof(struct MSG_TAB_ACK), "MSG_TAB_ACK");
}

// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_TAB_ACK_Delete_All(struct MSG_TAB_ACK *p_tab_ack)
{
    return MSG_Delete_All(&p_tab_ack);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_TAB_ACK"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_TAB_ACK de MSG_TAB_ACK_New
// et MSG_TAB_ACK_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_TAB_ACK_Write(struct MSG_TAB_ACK *p_tab_ack, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_tab_ack, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_TAB_ACK
PUBLIC BOOL MSG_TAB_ACK_Read(struct MSG_TAB_ACK *p_tab_ack, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_tab_ack, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_TAB_ACK_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_TAB_ACK_CD, 
                          MSG_TAB_ACK, 
                          sizeof(struct MSG_TAB_ACK), "MSG_TAB_ACK");
}

/*-------------------------------- END OF FILE ------------------------------*/