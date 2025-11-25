/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_VAULT_STAT.C											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_Vault_Stat.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_VAULT_STAT_NB_FIELDS MSG_HEADER_NB_FIELDS + 7

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- CODE:      -------------------------------*/

// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF

// Necessary MACRO to the calculations of the offset by MSG_OFFSET and has the initialisation
// local of type union MSG_Field structures [] by MSG_INIT_REF

MSG_DECLARE_REF(MSG_VAULT_STAT, MSG_VAULT_STAT_NB_FIELDS)
MSG_DECLARE_REF(MSG_VAULT_STAT_Coin_Info, MSG_VAULT_STAT_NB_FIELDS)
MSG_DECLARE_REF(MSG_VAULT_STAT_Vault_Info, MSG_VAULT_STAT_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ

PROTECTED void MSG_VAULT_STAT_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_VAULT_STAT[MSG_VAULT_STAT_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_VAULT_STAT, MSG_VAULT_STAT_CD) },
		{MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VAULT_STAT,body.rejected_coin_counter),	0UL,	99999999UL },
        {MSG_FIELD_LIST,   MSG_OFFSET(MSG_VAULT_STAT, list_vaults_in_lane), MSG_VAULT_STAT_Vault_Info, MSG_SIZEOF(MSG_VAULT_STAT_Vault_Info), 0UL, 9UL },

		{MSG_FIELD_STOP}
    };


    union MSG_Field LOCAL_Vault_Info[MSG_VAULT_STAT_NB_FIELDS]=
	{
		{MSG_FIELD_TIME, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, time_of_insert)},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, position),		0UL,		9UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, type),			0UL,		9UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, id),			0UL,		999999UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, fill_percent), 0UL,100UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, rejected_coins_since_insertion), 0UL,99999999UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, nb_of_activation_since_insertion), 0UL,9999UL},
		{MSG_FIELD_DWORD, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, vault_state), 0UL,9999UL},
		{MSG_FIELD_LIST, MSG_OFFSET(MSG_VAULT_STAT_Vault_Info, list_coin_info), MSG_VAULT_STAT_Coin_Info, MSG_SIZEOF(MSG_VAULT_STAT_Coin_Info), 0UL,99UL},
		{MSG_FIELD_STOP}
	};

	

	union MSG_Field LOCAL_Coin_Info[MSG_VAULT_STAT_NB_FIELDS] =  
    {
        {MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VAULT_STAT_Coin_Info, coin_id),						0UL,        99UL },
		{MSG_FIELD_DWORD,  MSG_OFFSET(MSG_VAULT_STAT_Coin_Info, coin_counter),					0UL,        99999999UL },
		{MSG_FIELD_STOP}
	};   
    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_VAULT_STAT,                    LOCAL_MSG_VAULT_STAT);
    MSG_INIT_REF(MSG_VAULT_STAT_Coin_Info,			 LOCAL_Coin_Info);
	MSG_INIT_REF(MSG_VAULT_STAT_Vault_Info,	     LOCAL_Vault_Info);
    
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_VAULT_STAT *MSG_VAULT_STAT_New(void)
{
    // la variable MSG_PAYMENT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_VAULT_STAT, sizeof(struct MSG_VAULT_STAT), "MSG_VAULT_STAT");
}

// constructeur d'un element de liste de type "sold product"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_PAYMENT" préalablement alloué par MSG_PAYMENT_New
// rem : l'allocation du handle de liste est automatique

//PUBLIC struct MSG_VAULT_STAT_Vault_Info *MSG_VAULT_STAT_Vault_Info_New(HLIST *list_vaults_in_lane)
//{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
//    return MSG_New_List(list_vaults_in_lane, MSG_VAULT_STAT_Vault_Info, sizeof(struct MSG_VAULT_STAT_Vault_Info));
//}


PUBLIC struct MSG_VAULT_STAT_Vault_Info *MSG_VAULT_STAT_Vault_Info_New(HLIST *list_vaults_in_lane)
{
    return MSG_New_List(list_vaults_in_lane, MSG_VAULT_STAT_Vault_Info, sizeof(struct MSG_VAULT_STAT_Vault_Info));
}


// constructeur d'un element de liste de type "official currency"
// le pointeur de liste est normalement l'@ du champ "list_official_currencies" d'un
// autre element de liste(imbrication) de type "struct MSG_PAYMENT_Sold_Product" 
// préalablement alloué par MSG_PAYMENT_Sold_Product_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_VAULT_STAT_Coin_Info *MSG_VAULT_STAT_Coin_Info_New(HLIST *list_coin_info)
{
    // la variable MSG_PAYMENT_Official_Currency de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_coin_info, MSG_VAULT_STAT_Coin_Info, sizeof(struct MSG_VAULT_STAT_Coin_Info));
}


// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_VAULT_STAT_Delete_All(struct MSG_VAULT_STAT *p_vault_stat)
{
    return MSG_Delete_All(&p_vault_stat);
}

// ITERATEURS

PUBLIC struct MSG_VAULT_STAT_Vault_Info *MSG_VAULT_STAT_Get_First_Vault_Info(HLIST list_vaults_in_lane)
{
    return MSG_Get_First(list_vaults_in_lane);
}

PUBLIC struct MSG_VAULT_STAT_Vault_Info *MSG_VAULT_STAT_Get_Next_Vault_Info(HLIST list_vaults_in_lane, struct MSG_VAULT_STAT_Vault_Info *p_vault)
{
    return MSG_Get_Next(list_vaults_in_lane, p_vault);
}

PUBLIC struct MSG_VAULT_STAT_Coin_Info *MSG_VAULT_STAT_Get_First_Coin_Info(HLIST list_coin_info)
{
    return MSG_Get_First(list_coin_info);
}

PUBLIC struct MSG_VAULT_STAT_Coin_Info *MSG_VAULT_STAT_Get_Next_Coin_Info(HLIST list_coin_info, struct MSG_VAULT_STAT_Coin_Info *p_coin)
{
    return MSG_Get_Next(list_coin_info, p_coin);
}


// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PAYMENT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_PAYMENT de MSG_PAYMENT_New
// et MSG_PAYMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_VAULT_STAT_Write(struct MSG_VAULT_STAT *p_vault_stat, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_vault_stat, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_PAYMENT
PUBLIC BOOL MSG_VAULT_STAT_Read(struct MSG_VAULT_STAT *p_vault_stat, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_vault_stat, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_VAULT_STAT_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_VAULT_STAT_CD, 
			              MSG_VAULT_STAT, 
                          sizeof(struct MSG_VAULT_STAT), "MSG_VAULT_STAT");
}

/*-------------------------------- END OF FILE ------------------------------*/
