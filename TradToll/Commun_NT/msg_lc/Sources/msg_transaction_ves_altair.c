/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRANSACTION_VES_ALTAIR.C								 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_transaction_ves_altair.h> 

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS MSG_HEADER_NB_FIELDS + 24

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// fonction de customisation en écriture du champ "Toll Fare Correcting Sign" of MSG_TRANSACTION_VES_ALTAIR_VES
PRIVATE BOOL Transaction_Sign_Write(LONG *sign, HMSG hRootMsg)
{
    BOOL ret;

    // symbolise >= 0 par '+' et <0 par '-'
    if(*sign < 0)
        ret = MSG_Write_String(hRootMsg, "-", 1UL, 1UL);
    else
        ret = MSG_Write_String(hRootMsg, "+", 1UL, 1UL);

    return ret;
}

// fonction de customisation en lecture du champ "Toll Fare Correcting Sign" of MSG_TRANSACTION_VES_ALTAIR 
PRIVATE BOOL Transaction_Sign_Read(LONG *sign, HMSG hRefMsg)
{
    CHAR ascii_sign;

    if(MSG_Read_String(hRefMsg, &ascii_sign, 1, 1) == FALSE)
        return FALSE;
    
    // convertit '+' par 1L et '-' par -1L
    switch(ascii_sign)
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
MSG_DECLARE_REF(MSG_TRANSACTION_VES_ALTAIR, MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRANSACTION_VES_ALTAIR_Info_Detail, MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_TRANSACTION_VES_ALTAIR_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

	// Structure generale du message
    union MSG_Field LOCAL_MSG_TRANSACTION_VES_ALTAIR[MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS] =  
    {
// Message Header
		{ MSG_FIELD_HEADER(MSG_TRANSACTION_VES_ALTAIR, MSG_TRANSACTION_VES_ALTAIR_CD) },

// Message Body
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, body.command), 0UL, 99UL },

// Transaction details
        { MSG_FIELD_TIME,  MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.time_of_transaction) },
		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.transaction_class), 0UL, 99UL },
		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.keyed_class),       0UL, 99UL },
		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.detected_class),    0UL, 99UL },
		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.type_of_payment),   0UL, 99UL },
        { MSG_FIELD_FLOAT, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.toll_fare),         10UL },
        { MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, det_trans.anomaly),           0UL, 99UL },

// Complementary information
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR, complementary_info), MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, MSG_SIZEOF(MSG_TRANSACTION_VES_ALTAIR_Complementary_Info), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };


	// Structure pour l'information complementaire
    union MSG_Field LOCAL_COMPLEMENTARY_INFO[MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS] =  
    {
// Information type
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, info_type),               0UL,        99UL },

// Elementary information detail
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, list_info_detail), MSG_TRANSACTION_VES_ALTAIR_Info_Detail, MSG_SIZEOF(MSG_TRANSACTION_VES_ALTAIR_Info_Detail), 0UL, 99UL },
        { MSG_FIELD_STOP }
    };


	// Structure pour le detail de l'information elementaire
    union MSG_Field LOCAL_INFO_DETAIL[MSG_TRANSACTION_VES_ALTAIR_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR_Info_Detail, info_code),   0UL, 99UL },
        { MSG_FIELD_VARIANT, MSG_OFFSET(MSG_TRANSACTION_VES_ALTAIR_Info_Detail, info_value), 0UL, MSG_TRANSACTION_VES_ALTAIR_INFO_VALUE_LENGTH },

        { MSG_FIELD_STOP }
    };

    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des variables globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_TRANSACTION_VES_ALTAIR,                    LOCAL_MSG_TRANSACTION_VES_ALTAIR);
    MSG_INIT_REF(MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, LOCAL_COMPLEMENTARY_INFO)
    MSG_INIT_REF(MSG_TRANSACTION_VES_ALTAIR_Info_Detail,        LOCAL_INFO_DETAIL)
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de transaction
PUBLIC struct MSG_TRANSACTION_VES_ALTAIR *MSG_TRANSACTION_VES_ALTAIR_New(void)
{
    // la variable MSG_TRANSACTION_VES_ALTAIR de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_TRANSACTION_VES_ALTAIR, sizeof(struct MSG_TRANSACTION_VES_ALTAIR), "MSG_TRANSACTION_VES_ALTAIR");
}


// constructeur d'un element de liste de type "info detail"
// le pointeur de liste est normalement l'@ du champ "list_info_detail" d'un
// message de type "struct MSG_TRANSACTION_VES_ALTAIR" préalablement alloué par MSG_TRANSACTION_VES_ALTAIR_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail *MSG_TRANSACTION_VES_ALTAIR_Info_Detail_New(HLIST *list_info_detail)
{
    // la variable MSG_TRANSACTION_VES_ALTAIR_Anomaly de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_info_detail, MSG_TRANSACTION_VES_ALTAIR_Info_Detail, sizeof(struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail));
}

// constructeur d'un element de liste de type "entry info"
PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info *MSG_TRANSACTION_VES_ALTAIR_Complementary_Info_New(HLIST *complementary_info)
{
    return MSG_New_List(complementary_info, MSG_TRANSACTION_VES_ALTAIR_Complementary_Info, sizeof(struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info));
}

// suppression COMPLETE d'un message de transaction
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_TRANSACTION_VES_ALTAIR_Delete_All(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction)
{
    return MSG_Delete_All(&p_transaction);
}

// ITERATEURS


PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail *MSG_TRANSACTION_VES_ALTAIR_Get_First_Info_Detail(HLIST list_info_detail)
{
    return MSG_Get_First(list_info_detail);
}

PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail *MSG_TRANSACTION_VES_ALTAIR_Get_Next_Info_Detail(HLIST list_info_detail, struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail *p_cur)
{
    return MSG_Get_Next(list_info_detail, p_cur);
}

PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_First_Complementary_Info(HLIST complementary_info)
{
    return MSG_Get_First(complementary_info);
}

PUBLIC struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_Next_Complementary_Info(HLIST complementary_info, struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info *p_entry)
{
    return MSG_Get_Next(complementary_info, p_entry);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_TRANSACTION_VES_ALTAIR"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_TRANSACTION_VES_ALTAIR de MSG_TRANSACTION_VES_ALTAIR_New
// et MSG_PAYMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_TRANSACTION_VES_ALTAIR_Write(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_transaction, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_TRANSACTION_VES_ALTAIR
PUBLIC BOOL MSG_TRANSACTION_VES_ALTAIR_Read(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_transaction, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_TRANSACTION_VES_ALTAIR_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_TRANSACTION_VES_ALTAIR_CD, 
                          MSG_TRANSACTION_VES_ALTAIR, 
                          sizeof(struct MSG_TRANSACTION_VES_ALTAIR), "MSG_TRANSACTION_VES_ALTAIR");
}

/*-------------------------------- END OF FILE ------------------------------*/
