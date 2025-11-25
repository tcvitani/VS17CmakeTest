/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_PARTLY.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_partly.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PARTLY_NB_FIELDS MSG_HEADER_NB_FIELDS + 12

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_PARTLY, MSG_PARTLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PARTLY_Traffic_Det_Payment, MSG_PARTLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PARTLY_Detailed_Traffic, MSG_PARTLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PARTLY_Currency_Det_Payment, MSG_PARTLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_PARTLY_Currency, MSG_PARTLY_NB_FIELDS)


// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_PARTLY_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_PARTLY[MSG_PARTLY_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_PARTLY, MSG_PARTLY_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_PARTLY, body.time_of_inferior_limit) },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_PARTLY, body.time_of_superior_limit) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY, body.primary_currency_id),                    0UL,        99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PARTLY, body.primary_revenue),									12UL },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY, general_traffic.total_traffic),                      0UL,           99999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY, general_traffic.total_violation),                      0UL,           99999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY, general_traffic.maintenance_traffic),                      0UL,           99999UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PARTLY, general_traffic.non_dispatched_revenue),									12UL },
		
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PARTLY, detailed_traffic), MSG_PARTLY_Detailed_Traffic, MSG_SIZEOF(MSG_PARTLY_Detailed_Traffic), 0UL, 99UL },
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_PARTLY, currency), MSG_PARTLY_Currency, MSG_SIZEOF(MSG_PARTLY_Currency), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_DETAILED_TRAFFIC[MSG_PARTLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY_Detailed_Traffic, class_id),                        0UL,           999UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PARTLY_Detailed_Traffic, detailed_payment), MSG_PARTLY_Traffic_Det_Payment, MSG_SIZEOF(MSG_PARTLY_Traffic_Det_Payment), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_TRAFFIC_DET_PAYMENT[MSG_PARTLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY_Traffic_Det_Payment, payment_id),                   0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY_Traffic_Det_Payment, traffic),                   0UL,           99999UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_CURRENCY[MSG_PARTLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY_Currency, currency_id),                             0UL,           99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_PARTLY_Currency, detailed_payment), MSG_PARTLY_Currency_Det_Payment, MSG_SIZEOF(MSG_PARTLY_Currency_Det_Payment), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };


    union MSG_Field LOCAL_CURRENCY_DET_PAYMENT[MSG_PARTLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_PARTLY_Currency_Det_Payment, payment_id),                        0UL,           99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_PARTLY_Currency_Det_Payment, revenue),									12UL },

        { MSG_FIELD_STOP }
    };

   
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_PARTLY,                        LOCAL_MSG_PARTLY);
    MSG_INIT_REF(MSG_PARTLY_Detailed_Traffic,       LOCAL_DETAILED_TRAFFIC);
    MSG_INIT_REF(MSG_PARTLY_Traffic_Det_Payment,    LOCAL_TRAFFIC_DET_PAYMENT);
    MSG_INIT_REF(MSG_PARTLY_Currency,               LOCAL_CURRENCY);
    MSG_INIT_REF(MSG_PARTLY_Currency_Det_Payment,	 LOCAL_CURRENCY_DET_PAYMENT);
}



// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_PARTLY *MSG_PARTLY_New(void)
{
    // la variable MSG_PARTLY de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_PARTLY, sizeof(struct MSG_PARTLY), "MSG_PARTLY");
}

// constructors for lists

PUBLIC struct MSG_PARTLY_Traffic_Det_Payment *MSG_PARTLY_Traffic_Det_Payment_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_PARTLY_Traffic_Det_Payment, sizeof(struct MSG_PARTLY_Traffic_Det_Payment));
}

PUBLIC struct MSG_PARTLY_Detailed_Traffic *MSG_PARTLY_Detailed_Traffic_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_PARTLY_Detailed_Traffic, sizeof(struct MSG_PARTLY_Detailed_Traffic));
}

PUBLIC struct MSG_PARTLY_Currency *MSG_PARTLY_Currency_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_PARTLY_Currency, sizeof(struct MSG_PARTLY_Currency));
}

PUBLIC struct MSG_PARTLY_Currency_Det_Payment *MSG_PARTLY_Currency_Det_Payment_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_PARTLY_Currency_Det_Payment, sizeof(struct MSG_PARTLY_Currency_Det_Payment));
}

//deleting of entire message  
PUBLIC BOOL MSG_PARTLY_Delete_All(struct MSG_PARTLY *p_partly)
{
    return MSG_Delete_All(&p_partly);
}

// ITERATEURS

PUBLIC struct MSG_PARTLY_Traffic_Det_Payment *MSG_PARTLY_Get_First_Traffic_Det_Payment(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_PARTLY_Traffic_Det_Payment *MSG_PARTLY_Get_Next_Traffic_Det_Payment(HLIST hList, struct MSG_PARTLY_Traffic_Det_Payment *p_det_pay)
{
    return MSG_Get_Next(hList, p_det_pay);
}

PUBLIC struct MSG_PARTLY_Detailed_Traffic *MSG_PARTLY_Get_First_Detailed_Traffic(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_PARTLY_Detailed_Traffic *MSG_PARTLY_Get_Next_Detailed_Traffic(HLIST hList, struct MSG_PARTLY_Detailed_Traffic *p_det_traffic)
{
    return MSG_Get_Next(hList, p_det_traffic);
}

PUBLIC struct MSG_PARTLY_Currency *MSG_PARTLY_Get_First_Currency(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_PARTLY_Currency *MSG_PARTLY_Get_Next_Currency(HLIST hList, struct MSG_PARTLY_Currency *p_cur)
{
    return MSG_Get_Next(hList, p_cur);
}

PUBLIC struct MSG_PARTLY_Currency_Det_Payment *MSG_PARTLY_Get_First_Currency_Det_Payment(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_PARTLY_Currency_Det_Payment *MSG_PARTLY_Get_Next_Currency_Det_Payment(HLIST hList, struct MSG_PARTLY_Currency_Det_Payment *p_det_pay)
{
    return MSG_Get_Next(hList, p_det_pay);
}

// LECTURE / ECRITURE

PUBLIC BOOL MSG_PARTLY_Write(struct MSG_PARTLY *p_partly, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_partly, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

PUBLIC BOOL MSG_PARTLY_Read(struct MSG_PARTLY *p_partly, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_partly, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_PARTLY_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_PARTLY_CD, 
                          MSG_PARTLY, 
                          sizeof(struct MSG_PARTLY), "MSG_PARTLY");
}

/*-------------------------------- END OF FILE ------------------------------*/