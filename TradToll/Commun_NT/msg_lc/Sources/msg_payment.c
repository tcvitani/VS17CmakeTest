/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_PAYMENT.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_payment.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PAYMENT_NB_FIELDS MSG_HEADER_NB_FIELDS + 21

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// fonction de customisation en écriture du champ "Sign of payment" 
// de la struct de liste "Sold Product"
PRIVATE BOOL Sold_Product_Sign_Write(LONG *sign, HMSG hRootMsg)
{
    BOOL ret;

    // symbolise >= 0 par '+' et <0 par '-'
    if(*sign < 0)
        ret = MSG_Write_String(hRootMsg, "-", 1UL, 1UL);
    else
        ret = MSG_Write_String(hRootMsg, "+", 1UL, 1UL);

    return ret;
}

// fonction de customisation en lecture du champ "Sign of payment" 
// de la struct de liste "Sold Product"
PRIVATE BOOL Sold_Product_Sign_Read(LONG *sign, HMSG hRefMsg)
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
MSG_DECLARE_REF(MSG_PAYMENT, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Sold_Product, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Official_Currency, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Anomaly, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Info_Type, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Elem_Info, MSG_PAYMENT_NB_FIELDS)
MSG_DECLARE_REF(MSG_PAYMENT_Anomaly_Description, MSG_PAYMENT_NB_FIELDS)


// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PAYMENT_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PAYMENT[MSG_PAYMENT_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_PAYMENT, MSG_PAYMENT_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_PAYMENT, body.time_of_payment) },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, ref_trs.internal_id),                    0UL,        99999UL },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_PAYMENT, ref_trs.start_time) },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PAYMENT, list_sold_products), MSG_PAYMENT_Sold_Product, MSG_SIZEOF(MSG_PAYMENT_Sold_Product), 1UL, 99UL },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, value.currency_id),                      0UL,           99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PAYMENT, value.in_currency),										10UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, value.balance_type),                     0UL,            1UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PAYMENT, value.balance_value_after_trs),							10UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, value.valid),                            0UL,            1UL },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, type.type),                              0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, type.sub_type),                          0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, type.acquisition_mode),                  0UL,            9UL },
        { MSG_FIELD_STRING, MSG_OFFSET(MSG_PAYMENT, type.primary_product_code),              0UL,            MSG_PAYMENT_PRODUCT_CODE_LENGTH },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, type.primary_commercial_type),           0UL,           99UL },
        { MSG_FIELD_STRING, MSG_OFFSET(MSG_PAYMENT, type.product_code_used),                 0UL,            MSG_PAYMENT_PRODUCT_CODE_LENGTH },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT, type.commercial_type_used),              0UL,           99UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PAYMENT, list_anomalies), MSG_PAYMENT_Anomaly, MSG_SIZEOF(MSG_PAYMENT_Anomaly), 0UL, 99UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PAYMENT, list_comp_inf), MSG_PAYMENT_Info_Type, MSG_SIZEOF(MSG_PAYMENT_Info_Type), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_SOLD_PRODUCT[MSG_PAYMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Sold_Product, id),                        0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Sold_Product, quantity),                  0UL,          999UL },
        { MSG_FIELD_CUSTOM, MSG_OFFSET(MSG_PAYMENT_Sold_Product, sign), Sold_Product_Sign_Write, Sold_Product_Sign_Read, NULL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PAYMENT_Sold_Product, list_official_currencies), MSG_PAYMENT_Official_Currency, MSG_SIZEOF(MSG_PAYMENT_Official_Currency), 1UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_OFFICIAL_CURRENCY[MSG_PAYMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Official_Currency, id),                   0UL,           99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PAYMENT_Official_Currency, unit_price),							10UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ANOMALY[MSG_PAYMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Anomaly, id),                             0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Anomaly, value),                          0UL,           99UL },
		{ MSG_FIELD_LIST,	MSG_OFFSET(MSG_PAYMENT_Anomaly, list_anomaly_descritpion), MSG_PAYMENT_Anomaly_Description, MSG_SIZEOF(MSG_PAYMENT_Anomaly_Description), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };


    union MSG_Field LOCAL_INFO_TYPE[MSG_PAYMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Info_Type, type),                        0UL,           99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PAYMENT_Info_Type, list_info_type), MSG_PAYMENT_Elem_Info, MSG_SIZEOF(MSG_PAYMENT_Elem_Info), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ELEM_INFO[MSG_PAYMENT_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PAYMENT_Elem_Info, code),                   0UL,           99UL },
        { MSG_FIELD_VARIANT,    MSG_OFFSET(MSG_PAYMENT_Elem_Info, info_value), 0UL,         MSG_PAYMENT_INFO_VALUE_LENGTH },

        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_ANOMALY_DESCRIPTION[MSG_PAYMENT_NB_FIELDS] =
	{
		{ MSG_FIELD_VARIANT, MSG_OFFSET(MSG_PAYMENT_Anomaly_Description, description), 0UL, MSG_PAYMENT_ANOMALY_DESCRIPTION_LENGTH },

		{ MSG_FIELD_STOP }
	};

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_PAYMENT,						LOCAL_MSG_PAYMENT);
    MSG_INIT_REF(MSG_PAYMENT_Sold_Product,			LOCAL_SOLD_PRODUCT);
    MSG_INIT_REF(MSG_PAYMENT_Official_Currency,		LOCAL_OFFICIAL_CURRENCY);
    MSG_INIT_REF(MSG_PAYMENT_Anomaly,				LOCAL_ANOMALY);
    MSG_INIT_REF(MSG_PAYMENT_Info_Type,				LOCAL_INFO_TYPE);
    MSG_INIT_REF(MSG_PAYMENT_Elem_Info,   			LOCAL_ELEM_INFO); 
	MSG_INIT_REF(MSG_PAYMENT_Anomaly_Description,	LOCAL_ANOMALY_DESCRIPTION);
}



// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_PAYMENT *MSG_PAYMENT_New(void)
{
    // la variable MSG_PAYMENT de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_PAYMENT, sizeof(struct MSG_PAYMENT), "MSG_PAYMENT");
}

// constructeur d'un element de liste de type "sold product"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_PAYMENT" préalablement alloué par MSG_PAYMENT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PAYMENT_Sold_Product *MSG_PAYMENT_Sold_Product_New(HLIST *list_sold_products)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_sold_products, MSG_PAYMENT_Sold_Product, sizeof(struct MSG_PAYMENT_Sold_Product));
}

// constructeur d'un element de liste de type "official currency"
// le pointeur de liste est normalement l'@ du champ "list_official_currencies" d'un
// autre element de liste(imbrication) de type "struct MSG_PAYMENT_Sold_Product" 
// préalablement alloué par MSG_PAYMENT_Sold_Product_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PAYMENT_Official_Currency *MSG_PAYMENT_Official_Currency_New(HLIST *list_official_currencies)
{
    // la variable MSG_PAYMENT_Official_Currency de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_official_currencies, MSG_PAYMENT_Official_Currency, sizeof(struct MSG_PAYMENT_Official_Currency));
}

// constructeur d'un element de liste de type "anomaly"
// le pointeur de liste est normalement l'@ du champ "list_anomalies" d'un
// message de type "struct MSG_PAYMENT" préalablement alloué par MSG_PAYMENT_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PAYMENT_Anomaly *MSG_PAYMENT_Anomaly_New(HLIST *list_anomalies)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_anomalies, MSG_PAYMENT_Anomaly, sizeof(struct MSG_PAYMENT_Anomaly));
}

PUBLIC struct MSG_PAYMENT_Anomaly_Description *MSG_PAYMENT_Anomaly_Description_New(HLIST *list_anomaly_descritpion)
{
	return MSG_New_List(list_anomaly_descritpion, MSG_PAYMENT_Anomaly_Description, sizeof(struct MSG_PAYMENT_Anomaly_Description));
}

// suppression COMPLETE d'un message de paiement
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_PAYMENT_Delete_All(struct MSG_PAYMENT *p_payment)
{
    return MSG_Delete_All(&p_payment);
}

// ITERATEURS

PUBLIC struct MSG_PAYMENT_Sold_Product *MSG_PAYMENT_Get_First_Sold_Product(HLIST list_sold_products)
{
    return MSG_Get_First(list_sold_products);
}

PUBLIC struct MSG_PAYMENT_Sold_Product *MSG_PAYMENT_Get_Next_Sold_Product(HLIST list_sold_products, struct MSG_PAYMENT_Sold_Product *p_sold)
{
    return MSG_Get_Next(list_sold_products, p_sold);
}

PUBLIC struct MSG_PAYMENT_Official_Currency *MSG_PAYMENT_Get_First_Official_Currency(HLIST list_official_currencies)
{
    return MSG_Get_First(list_official_currencies);
}

PUBLIC struct MSG_PAYMENT_Official_Currency *MSG_PAYMENT_Get_Next_Official_Currency(HLIST list_official_currencies, struct MSG_PAYMENT_Official_Currency *p_cur)
{
    return MSG_Get_Next(list_official_currencies, p_cur);
}

PUBLIC struct MSG_PAYMENT_Anomaly *MSG_PAYMENT_Get_First_Anomaly(HLIST list_anomalies)
{
    return MSG_Get_First(list_anomalies);
}

PUBLIC struct MSG_PAYMENT_Anomaly *MSG_PAYMENT_Get_Next_Anomaly(HLIST list_anomalies, struct MSG_PAYMENT_Anomaly *p_cur)
{
    return MSG_Get_Next(list_anomalies, p_cur);
}

PUBLIC struct MSG_PAYMENT_Anomaly_Description *MSG_PAYMENT_Get_First_Anomaly_Description(HLIST list_anomaly_description)
{
	return MSG_Get_First(list_anomaly_description);
}

PUBLIC struct MSG_PAYMENT_Anomaly_Description *MSG_PAYMENT_Get_Next_Anomaly_Description(HLIST list_anomaly_description, struct MSG_PAYMENT_Anomaly_Description *p_anomaly_descritpion)
{
	return MSG_Get_Next(list_anomaly_description, p_anomaly_descritpion);
}

// constructeur d'un element de liste de type "list_info_type"
// le pointeur de liste est normalement l'@ du champ "list_sold_products" d'un
// message de type "struct MSG_PAYMENT_Info_Type"
// préalablement alloué par MSG_PAYMENT_Info_Type_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_PAYMENT_Info_Type *MSG_PAYMENT_Info_Type_New(HLIST *list_info_type)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_info_type, MSG_PAYMENT_Info_Type, sizeof(struct MSG_PAYMENT_Info_Type));
}

// constructeur d'un element de liste de type "list_comp_inf"
// le pointeur de liste est normalement l'@ du champ "list_official_currencies" d'un
// autre element de liste(imbrication) de type "struct MSG_PAYMENT_Elem_Info" 
// préalablement alloué par MSG_PAYMENT_Elem_Info_New
// rem : l'allocation du handle de liste est automatique

PUBLIC struct MSG_PAYMENT_Elem_Info *MSG_PAYMENT_Elem_Info_New(HLIST *list_comp_inf)
{
    // la variable MSG_PAYMENT_Sold_Product de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_comp_inf, MSG_PAYMENT_Elem_Info, sizeof(struct MSG_PAYMENT_Elem_Info));
}


// ITERATEURS

PUBLIC struct MSG_PAYMENT_Info_Type *MSG_PAYMENT_Get_First_Info_Type(HLIST list_info_type)
{
    return MSG_Get_First(list_info_type);
}

PUBLIC struct MSG_PAYMENT_Info_Type *MSG_PAYMENT_Get_Next_Info_Type(HLIST list_info_type, struct MSG_PAYMENT_Info_Type *p_type)
{
    return MSG_Get_Next(list_info_type, p_type);
}


PUBLIC struct MSG_PAYMENT_Elem_Info *MSG_PAYMENT_Get_First_Elem_Info(HLIST list_comp_inf)
{
    return MSG_Get_First(list_comp_inf);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_PAYMENT"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_PAYMENT de MSG_PAYMENT_New
// et MSG_PAYMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_PAYMENT_Write(struct MSG_PAYMENT *p_payment, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_payment, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_PAYMENT
PUBLIC BOOL MSG_PAYMENT_Read(struct MSG_PAYMENT *p_payment, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_payment, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PAYMENT_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_PAYMENT_CD, 
                          MSG_PAYMENT, 
                          sizeof(struct MSG_PAYMENT), "MSG_PAYMENT");
}

/*-------------------------------- END OF FILE ------------------------------*/
