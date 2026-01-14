/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_VAUL_REP.C                                               */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv_vaul_rep.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_VAUL_REP_NB_FIELDS MSG_PV_HEADER_NB_FIELDS + 6 + 1
#define MSG_PV_VAUL_REP_VAULT_NB_FIELDS 9 + 1
#define MSG_PV_VAUL_REP_CURRENCY_NB_FIELDS 3 + 1
#define MSG_PV_VAUL_REP_COIN_NB_FIELDS 2 + 1

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_PV_VAUL_REP, MSG_PV_VAUL_REP_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_VAUL_REP_Vault, MSG_PV_VAUL_REP_VAULT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_VAUL_REP_Currency, MSG_PV_VAUL_REP_CURRENCY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PV_VAUL_REP_Coin, MSG_PV_VAUL_REP_COIN_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur (champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale (erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PV_VAUL_REP_Init(void)
{
    // suppression (localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions (?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union (ici  struct MSG_Field_Stop)
#pragma warning( push )
#pragma warning( disable : 4047 )  
	
    union MSG_Field LOCAL_MSG_PV_VAUL_REP[MSG_PV_VAUL_REP_NB_FIELDS] =  
    {
        { MSG_FIELD_PV_HEADER (MSG_PV_VAUL_REP, MSG_PV_VAUL_REP_CD) },
			
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP, body.lanenum),					0UL,	9999UL },
        { MSG_FIELD_TIME,	MSG_OFFSET(MSG_PV_VAUL_REP, body.date) },
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP, body.collector_id),				0UL,	999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP, body.active_vault),				0UL,	999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP, body.rejected_coin_counter),	0UL,	99999999UL },
        
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_VAUL_REP, list_vault), MSG_PV_VAUL_REP_Vault, MSG_SIZEOF(MSG_PV_VAUL_REP_Vault), 0UL, 9UL },
		
        { MSG_FIELD_STOP }
    };
	
	union MSG_Field LOCAL_VAULT[MSG_PV_VAUL_REP_VAULT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, position),	0UL,	9UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, percent),		0UL,	999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, id),			0UL,	999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, state),		0UL,	99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, type),		0UL,	9UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, rejected),	0UL,	99999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Vault, activated),	0UL,	99UL },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_PV_VAUL_REP_Vault, date) },
        
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_VAUL_REP_Vault, list_currency), MSG_PV_VAUL_REP_Currency, MSG_SIZEOF(MSG_PV_VAUL_REP_Currency), 0UL, 99UL },
		
        { MSG_FIELD_STOP }
    };
	
    union MSG_Field LOCAL_CURRENCY[MSG_PV_VAUL_REP_CURRENCY_NB_FIELDS] =  
    {
        { MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_PV_VAUL_REP_Currency, label),	0UL,	MSG_PV_MAX_CURRENCY_LABEL - 1UL },
		{ MSG_FIELD_FLOAT,	 MSG_OFFSET(MSG_PV_VAUL_REP_Currency, value),	11UL},
		
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PV_VAUL_REP_Currency, list_coin), MSG_PV_VAUL_REP_Coin, MSG_SIZEOF(MSG_PV_VAUL_REP_Coin), 0UL, 99UL },
		
        { MSG_FIELD_STOP }
    };
	
	union MSG_Field LOCAL_COIN[MSG_PV_VAUL_REP_COIN_NB_FIELDS] =  
    {
        { MSG_FIELD_VARSTR,  MSG_OFFSET(MSG_PV_VAUL_REP_Coin, label),	0UL,	MSG_PV_MAX_COIN_LABEL - 1UL },
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PV_VAUL_REP_Coin, counter),	0UL,	99999999UL },
		
        { MSG_FIELD_STOP }
    };
    
#pragma warning( pop )  
	
    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF (MSG_PV_VAUL_REP,          LOCAL_MSG_PV_VAUL_REP);
    MSG_INIT_REF (MSG_PV_VAUL_REP_Vault,    LOCAL_VAULT);
    MSG_INIT_REF (MSG_PV_VAUL_REP_Currency,	LOCAL_CURRENCY);
    MSG_INIT_REF (MSG_PV_VAUL_REP_Coin,		LOCAL_COIN);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces ! (ajout d'un entete devant la structure)

// constructeur d'un message
PUBLIC struct MSG_PV_VAUL_REP *MSG_PV_VAUL_REP_New (void)
{
    // la variable MSG_PV_VAUL_REP de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New (MSG_PV_VAUL_REP, sizeof(struct MSG_PV_VAUL_REP), "MSG_PV_VAUL_REP");
}

// constructeur d'un element de liste de type "vault"
// le pointeur de liste est normalement l'@ du champ "list_vault" d'un
// message de type "struct MSG_PV_VAUL_REP" préalablement alloué par MSG_PV_VAUL_REP_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_VAUL_REP_Vault *MSG_PV_VAUL_REP_Vault_New (HLIST *list)
{
    // la variable MSG_PV_VAUL_REP_Command de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_VAUL_REP_Vault, sizeof(struct MSG_PV_VAUL_REP_Vault));
}

// constructeur d'un element de liste de type "Currency"
// le pointeur de liste est normalement l'@ du champ "list_currency" d'un
// autre element de liste (imbrication) de type "struct MSG_PV_VAUL_REP_Vault" 
// préalablement alloué par MSG_PV_VAUL_REP_Vault_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_VAUL_REP_Currency *MSG_PV_VAUL_REP_Currency_New (HLIST *list)
{
    // la variable MSG_PV_VAUL_REP_Equipment de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_VAUL_REP_Currency, sizeof(struct MSG_PV_VAUL_REP_Currency));
}

// constructeur d'un element de liste de type "Coin"
// le pointeur de liste est normalement l'@ du champ "list_coin" d'un
// autre element de liste (imbrication) de type "struct MSG_PV_VAUL_REP_Currency" 
// préalablement alloué par MSG_PV_VAUL_REP_Currency_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PV_VAUL_REP_Coin *MSG_PV_VAUL_REP_Coin_New (HLIST *list)
{
    // la variable MSG_PV_VAUL_REP_Equipment de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List (list, MSG_PV_VAUL_REP_Coin, sizeof(struct MSG_PV_VAUL_REP_Coin));
}

// suppression COMPLETE d'un message
// Une passe récursive est effectuée sur chaque champ du message 
// (champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes (handles + elements) sont supprimes
PUBLIC BOOL MSG_PV_VAUL_REP_Delete_All (struct MSG_PV_VAUL_REP *p_ptr)
{
    return MSG_Delete_All (&p_ptr);
}

// ITERATEURS

PUBLIC struct MSG_PV_VAUL_REP_Vault *MSG_PV_VAUL_REP_Get_First_Vault (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_VAUL_REP_Vault *MSG_PV_VAUL_REP_Get_Next_Vault (HLIST list, struct MSG_PV_VAUL_REP_Vault *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_VAUL_REP_Currency *MSG_PV_VAUL_REP_Get_First_Currency (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_VAUL_REP_Currency *MSG_PV_VAUL_REP_Get_Next_Currency (HLIST list, struct MSG_PV_VAUL_REP_Currency *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

PUBLIC struct MSG_PV_VAUL_REP_Coin *MSG_PV_VAUL_REP_Get_First_Coin (HLIST list)
{
    return MSG_Get_First (list);
}

PUBLIC struct MSG_PV_VAUL_REP_Coin *MSG_PV_VAUL_REP_Get_Next_Coin (HLIST list, struct MSG_PV_VAUL_REP_Coin *p_ptr)
{
    return MSG_Get_Next (list, p_ptr);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PV_VAUL_REP"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[] (cf variable MSG_PV_VAUL_REP de MSG_PV_VAUL_REP_New
// et MSG_PV_VAUL_REP_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PV_VAUL_REP_Write (struct MSG_PV_VAUL_REP *p_ptr, 
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
// de type struct MSG_PV_VAUL_REP
PUBLIC BOOL MSG_PV_VAUL_REP_Read (struct MSG_PV_VAUL_REP *p_ptr, 
								  BYTE *p_msg, 
								  DWORD msg_size_max, 
								  DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read (p_ptr, p_msg, msg_size_max);
	
    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PV_VAUL_REP_New_Record (HLIST *hList)
{
    return MSG_New_Record (hList, 
		(MSG_Compare) MSG_PV_HEADER_Compare_CD, 
		(PVOID) MSG_PV_VAUL_REP_CD, 
		MSG_PV_VAUL_REP, 
		sizeof(struct MSG_PV_VAUL_REP),
						  "MSG_PV_VAUL_REP");
}

/*-------------------------------- END OF FILE ------------------------------*/
