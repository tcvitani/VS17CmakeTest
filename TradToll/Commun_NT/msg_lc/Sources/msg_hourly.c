/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_HOURLY.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_hourly.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_HOURLY_NB_FIELDS MSG_HEADER_NB_FIELDS + 12

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_HOURLY, MSG_HOURLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_HOURLY_Traffic_Det_Payment, MSG_HOURLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_HOURLY_Detailed_Traffic, MSG_HOURLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_HOURLY_Currency_Det_Payment, MSG_HOURLY_NB_FIELDS)
MSG_DECLARE_REF(MSG_HOURLY_Currency, MSG_HOURLY_NB_FIELDS)


// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_HOURLY_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_HOURLY[MSG_HOURLY_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_HOURLY, MSG_HOURLY_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_HOURLY, body.time_of_inferior_limit) },
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_HOURLY, body.time_of_superior_limit) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY, body.primary_currency_id),                    0UL,        99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_HOURLY, body.primary_revenue),									12UL },

        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY, general_traffic.total_traffic),                      0UL,           99999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY, general_traffic.total_violation),                      0UL,           99999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY, general_traffic.maintenance_traffic),                      0UL,           99999UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_HOURLY, general_traffic.non_dispatched_revenue),									12UL },
		
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_HOURLY, detailed_traffic), MSG_HOURLY_Detailed_Traffic, MSG_SIZEOF(MSG_HOURLY_Detailed_Traffic), 0UL, 99UL },
		{ MSG_FIELD_LIST,   MSG_OFFSET(MSG_HOURLY, currency), MSG_HOURLY_Currency, MSG_SIZEOF(MSG_HOURLY_Currency), 0UL, 9UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_DETAILED_TRAFFIC[MSG_HOURLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY_Detailed_Traffic, class_id),                        0UL,           999UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_HOURLY_Detailed_Traffic, detailed_payment), MSG_HOURLY_Traffic_Det_Payment, MSG_SIZEOF(MSG_HOURLY_Traffic_Det_Payment), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_TRAFFIC_DET_PAYMENT[MSG_HOURLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY_Traffic_Det_Payment, payment_id),                   0UL,           99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY_Traffic_Det_Payment, traffic),                   0UL,           99999UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_CURRENCY[MSG_HOURLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY_Currency, currency_id),                             0UL,           99UL },
        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_HOURLY_Currency, detailed_payment), MSG_HOURLY_Currency_Det_Payment, MSG_SIZEOF(MSG_HOURLY_Currency_Det_Payment), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };


    union MSG_Field LOCAL_CURRENCY_DET_PAYMENT[MSG_HOURLY_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HOURLY_Currency_Det_Payment, payment_id),                        0UL,           99UL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_HOURLY_Currency_Det_Payment, revenue),									12UL },

        { MSG_FIELD_STOP }
    };

   
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_HOURLY,                        LOCAL_MSG_HOURLY);
    MSG_INIT_REF(MSG_HOURLY_Detailed_Traffic,       LOCAL_DETAILED_TRAFFIC);
    MSG_INIT_REF(MSG_HOURLY_Traffic_Det_Payment,    LOCAL_TRAFFIC_DET_PAYMENT);
    MSG_INIT_REF(MSG_HOURLY_Currency,               LOCAL_CURRENCY);
    MSG_INIT_REF(MSG_HOURLY_Currency_Det_Payment,	 LOCAL_CURRENCY_DET_PAYMENT);
}



// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_HOURLY *MSG_HOURLY_New(void)
{
    // la variable MSG_HOURLY de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_HOURLY, sizeof(struct MSG_HOURLY), "MSG_HOURLY");
}

// constructors for lists

PUBLIC struct MSG_HOURLY_Traffic_Det_Payment *MSG_HOURLY_Traffic_Det_Payment_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_HOURLY_Traffic_Det_Payment, sizeof(struct MSG_HOURLY_Traffic_Det_Payment));
}

PUBLIC struct MSG_HOURLY_Detailed_Traffic *MSG_HOURLY_Detailed_Traffic_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_HOURLY_Detailed_Traffic, sizeof(struct MSG_HOURLY_Detailed_Traffic));
}

PUBLIC struct MSG_HOURLY_Currency *MSG_HOURLY_Currency_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_HOURLY_Currency, sizeof(struct MSG_HOURLY_Currency));
}

PUBLIC struct MSG_HOURLY_Currency_Det_Payment *MSG_HOURLY_Currency_Det_Payment_New(HLIST *hList)
{
    return MSG_New_List(hList, MSG_HOURLY_Currency_Det_Payment, sizeof(struct MSG_HOURLY_Currency_Det_Payment));
}

//deleting of entire message  
PUBLIC BOOL MSG_HOURLY_Delete_All(struct MSG_HOURLY *p_hourly)
{
    return MSG_Delete_All(&p_hourly);
}

// ITERATEURS

PUBLIC struct MSG_HOURLY_Traffic_Det_Payment *MSG_HOURLY_Get_First_Traffic_Det_Payment(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_HOURLY_Traffic_Det_Payment *MSG_HOURLY_Get_Next_Traffic_Det_Payment(HLIST hList, struct MSG_HOURLY_Traffic_Det_Payment *p_det_pay)
{
    return MSG_Get_Next(hList, p_det_pay);
}

PUBLIC struct MSG_HOURLY_Detailed_Traffic *MSG_HOURLY_Get_First_Detailed_Traffic(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_HOURLY_Detailed_Traffic *MSG_HOURLY_Get_Next_Detailed_Traffic(HLIST hList, struct MSG_HOURLY_Detailed_Traffic *p_det_traffic)
{
    return MSG_Get_Next(hList, p_det_traffic);
}

PUBLIC struct MSG_HOURLY_Currency *MSG_HOURLY_Get_First_Currency(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_HOURLY_Currency *MSG_HOURLY_Get_Next_Currency(HLIST hList, struct MSG_HOURLY_Currency *p_cur)
{
    return MSG_Get_Next(hList, p_cur);
}

PUBLIC struct MSG_HOURLY_Currency_Det_Payment *MSG_HOURLY_Get_First_Currency_Det_Payment(HLIST hList)
{
    return MSG_Get_First(hList);
}

PUBLIC struct MSG_HOURLY_Currency_Det_Payment *MSG_HOURLY_Get_Next_Currency_Det_Payment(HLIST hList, struct MSG_HOURLY_Currency_Det_Payment *p_det_pay)
{
    return MSG_Get_Next(hList, p_det_pay);
}

// LECTURE / ECRITURE

PUBLIC BOOL MSG_HOURLY_Write(struct MSG_HOURLY *p_hourly, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_hourly, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

PUBLIC BOOL MSG_HOURLY_Read(struct MSG_HOURLY *p_hourly, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_hourly, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_HOURLY_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_HOURLY_CD, 
                          MSG_HOURLY, 
                          sizeof(struct MSG_HOURLY), "MSG_HOURLY");
}

/*-------------------------------- END OF FILE ------------------------------*/
