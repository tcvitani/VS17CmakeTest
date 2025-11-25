/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRANSACTION.C											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_transaction.h> 

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_TRANSACTION_NB_FIELDS MSG_HEADER_NB_FIELDS + 32

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

// DECLARATION STRUCTURELLE ET COMPORTEMENTALE

// fonction de customisation en écriture du champ "Toll Fare Correcting Sign" of MSG_TRANSACTION
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

// fonction de customisation en lecture du champ "Toll Fare Correcting Sign" of MSG_TRANSACTION 
PRIVATE BOOL Transaction_Sign_Read(LONG *sign, HMSG hRefMsg)
{
    CHAR ascii_sign[2];

    if(MSG_Read_String(hRefMsg, ascii_sign, 1, 1) == FALSE)
        return FALSE;
    
    // convertit '+' par 1L et '-' par -1L
    switch(ascii_sign[0])
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
MSG_DECLARE_REF(MSG_TRANSACTION,						MSG_TRANSACTION_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRANSACTION_Anomaly,				MSG_TRANSACTION_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRANSACTION_Entry_Info,				MSG_TRANSACTION_NB_FIELDS)
MSG_DECLARE_REF(MSG_TRANSACTION_Anomaly_Description,	MSG_TRANSACTION_NB_FIELDS)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_TRANSACTION_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    union MSG_Field LOCAL_MSG_TRANSACTION[MSG_TRANSACTION_NB_FIELDS] =  
    {
		{ MSG_FIELD_HEADER(MSG_TRANSACTION, MSG_TRANSACTION_CD) },

        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TRANSACTION, body.time_of_transaction) },

		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, ref_shift.start_of_msg_id),		0UL,            99999UL		},
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TRANSACTION, ref_shift.start_time) },

		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, ref_trans.id),					0UL,            99999UL		},
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TRANSACTION, ref_trans.start_time) },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION, ref_trans.total_traffic),       0UL,            99999999UL	},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION, ref_trans.total_violation),     0UL,            99999999UL	},

		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION, det_trans.transaction_class),   0UL,            999UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION, det_trans.keyed_class),         0UL,            999UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION, det_trans.detected_class),      0UL,            999UL		},
        { MSG_FIELD_VARSTR, MSG_OFFSET(MSG_TRANSACTION, det_trans.transaction_id),		0UL,			999UL		},
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.type_of_payment),     0UL,			99UL		},
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.sub_type_of_payment), 0UL,			99UL		},	
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.currency_id),         0UL,			99UL		},
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION, det_trans.toll_fare),			10UL						},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.correcting_type),     0UL,			99UL		},
        { MSG_FIELD_CUSTOM, MSG_OFFSET(MSG_TRANSACTION, det_trans.correcting_sign), Transaction_Sign_Write, Transaction_Sign_Read, NULL },
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION, det_trans.correcting_value),	10UL						},
		{ MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION, det_trans.toll_fare_ht),		10UL						},	
		{ MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION, det_trans.tax),					10UL						},	
		{ MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION, det_trans.tax_rate),			10UL						},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.valid_traffic),       0UL,            1UL			},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION, det_trans.valid_payment),       0UL,            1UL			},
		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_TRANSACTION, det_trans.exit_lic_plate),		0UL,			MSG_TRANSACTION_LIC_PLATE_LENGTH },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRANSACTION, entry_info), MSG_TRANSACTION_Entry_Info, MSG_SIZEOF(MSG_TRANSACTION_Entry_Info), 0UL, 1UL },

        { MSG_FIELD_LIST,   MSG_OFFSET(MSG_TRANSACTION, list_anomalies), MSG_TRANSACTION_Anomaly, MSG_SIZEOF(MSG_TRANSACTION_Anomaly), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ENTRY_INFO[MSG_TRANSACTION_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_Entry_Info, plaza_number),               0UL,      9999UL	},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_Entry_Info, lane_number),                0UL,      9999UL	},
        { MSG_FIELD_TIME,   MSG_OFFSET(MSG_TRANSACTION_Entry_Info, time_of_entry_trans) },
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, collector_id),               0UL,      999999UL	},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, transaction_entry_class),    0UL,      999UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, keyed_entry_class),          0UL,      999UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, detected_entry_class),       0UL,      999UL		},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, entry_fare_point),           0UL,      9999UL	},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, destination_fare_point),		0UL,      9999UL	},
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_Entry_Info, destination_plaza_number),	0UL,      9999UL	},
        { MSG_FIELD_FLOAT,  MSG_OFFSET(MSG_TRANSACTION_Entry_Info, prepaid_amount),				10UL				},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, country_id),                 0UL,      999UL		},
        { MSG_FIELD_STRING, MSG_OFFSET(MSG_TRANSACTION_Entry_Info, company_id),					0UL,      MSG_TRANSACTION_ENTRY_INFO_COMPANY_ID_LENGTH},
		{ MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, info_getting_mode),          0UL,      99UL		},
        { MSG_FIELD_DWORD,	MSG_OFFSET(MSG_TRANSACTION_Entry_Info, distribution_level),         0UL,      1UL		},
		{ MSG_FIELD_DWORD, MSG_OFFSET(MSG_TRANSACTION_Entry_Info, enl_trs_id),					0UL,	  99999999UL },
		{ MSG_FIELD_VARSTR, MSG_OFFSET(MSG_TRANSACTION_Entry_Info, entry_lic_plate),			0UL,	  MSG_TRANSACTION_LIC_PLATE_LENGTH },

        { MSG_FIELD_STOP }
    };

    union MSG_Field LOCAL_ANOMALY[MSG_TRANSACTION_NB_FIELDS] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_Anomaly, id),           0UL,				99UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_TRANSACTION_Anomaly, value),        0UL,				99UL },
		{ MSG_FIELD_LIST, MSG_OFFSET(MSG_TRANSACTION_Anomaly, list_anomaly_descritpion), MSG_TRANSACTION_Anomaly_Description, MSG_SIZEOF(MSG_TRANSACTION_Anomaly_Description), 0UL, 99UL },

        { MSG_FIELD_STOP }
    };

	union MSG_Field LOCAL_ANOMALY_DESCRIPTION[MSG_TRANSACTION_NB_FIELDS] =
	{
		{ MSG_FIELD_VARIANT, MSG_OFFSET(MSG_TRANSACTION_Anomaly_Description, description), 0UL, MSG_TRANSACTION_ANOMALY_DESCRIPTION_LENGTH },

		{ MSG_FIELD_STOP }
	};

    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_TRANSACTION,						LOCAL_MSG_TRANSACTION);
	MSG_INIT_REF(MSG_TRANSACTION_Anomaly,				LOCAL_ANOMALY);
	MSG_INIT_REF(MSG_TRANSACTION_Entry_Info,			LOCAL_ENTRY_INFO);
	MSG_INIT_REF(MSG_TRANSACTION_Anomaly_Description,	LOCAL_ANOMALY_DESCRIPTION);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de transaction
PUBLIC struct MSG_TRANSACTION *MSG_TRANSACTION_New(void)
{
    // la variable MSG_TRANSACTION de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_TRANSACTION, sizeof(struct MSG_TRANSACTION), "MSG_TRANSACTION");
}


// constructeur d'un element de liste de type "anomaly"
// le pointeur de liste est normalement l'@ du champ "list_anomalies" d'un
// message de type "struct MSG_TRANSACTION" préalablement alloué par MSG_TRANSACTION_New
// rem : l'allocation du handle de liste est automatique
PUBLIC struct MSG_TRANSACTION_Anomaly *MSG_TRANSACTION_Anomaly_New(HLIST *list_anomalies)
{
    // la variable MSG_TRANSACTION_Anomaly de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New_List(list_anomalies, MSG_TRANSACTION_Anomaly, sizeof(struct MSG_TRANSACTION_Anomaly));
}

// constructeur d'un element de liste de type "entry info"
PUBLIC struct MSG_TRANSACTION_Entry_Info *MSG_TRANSACTION_Entry_Info_New(HLIST *entry_info)
{
    return MSG_New_List(entry_info, MSG_TRANSACTION_Entry_Info, sizeof(struct MSG_TRANSACTION_Entry_Info));
}

PUBLIC struct MSG_TRANSACTION_Anomaly_Description *MSG_TRANSACTION_Anomaly_Description_New(HLIST *list_anomaly_descritpion)
{
	return MSG_New_List(list_anomaly_descritpion, MSG_TRANSACTION_Anomaly_Description, sizeof(struct MSG_TRANSACTION_Anomaly_Description));
}

// suppression COMPLETE d'un message de transaction
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_TRANSACTION_Delete_All(struct MSG_TRANSACTION *p_transaction)
{
    return MSG_Delete_All(&p_transaction);
}

// ITERATEURS


PUBLIC struct MSG_TRANSACTION_Anomaly *MSG_TRANSACTION_Get_First_Anomaly(HLIST list_anomalies)
{
    return MSG_Get_First(list_anomalies);
}

PUBLIC struct MSG_TRANSACTION_Anomaly *MSG_TRANSACTION_Get_Next_Anomaly(HLIST list_anomalies, struct MSG_TRANSACTION_Anomaly *p_cur)
{
    return MSG_Get_Next(list_anomalies, p_cur);
}

PUBLIC struct MSG_TRANSACTION_Entry_Info * WINAPI MSG_TRANSACTION_Get_First_Entry_Info(HLIST entry_info)
{
    return MSG_Get_First(entry_info);
}

PUBLIC struct MSG_TRANSACTION_Entry_Info * WINAPI MSG_TRANSACTION_Get_Next_Entry_Info(HLIST entry_info, struct MSG_TRANSACTION_Entry_Info *p_entry)
{
    return MSG_Get_Next(entry_info, p_entry);
}

PUBLIC struct MSG_TRANSACTION_Anomaly_Description *MSG_TRANSACTION_Get_First_Anomaly_Description(HLIST list_anomaly_description)
{
	return MSG_Get_First(list_anomaly_description);
}

PUBLIC struct MSG_TRANSACTION_Anomaly_Description *MSG_TRANSACTION_Get_Next_Anomaly_Description(HLIST list_anomaly_description, struct MSG_TRANSACTION_Anomaly_Description *p_anomaly_descritpion)
{
	return MSG_Get_Next(list_anomaly_description, p_anomaly_descritpion);
}

// LECTURE / ECRITURE

// passe a la moulinette une structure de donnée "struct MSG_TRANSACTION"
// a travers des parcours récursif des tableaux de description des champs
// de type union MSG_Field[](cf variable MSG_TRANSACTION de MSG_TRANSACTION_New
// et MSG_PAYMENT_Init)
// La variable p_msg contient en retour le mesasge sous forme ASCII
PUBLIC BOOL MSG_TRANSACTION_Write(struct MSG_TRANSACTION *p_transaction, 
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
// de type struct MSG_TRANSACTION
PUBLIC BOOL MSG_TRANSACTION_Read(struct MSG_TRANSACTION *p_transaction, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_transaction, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE

// Enregistrement sur une liste de type "MSG_Record"
PUBLIC BOOL MSG_TRANSACTION_New_Record(HLIST *hList)
{
    return MSG_New_Record(hList, 
                         (MSG_Compare) MSG_HEADER_Compare_CD, 
                         (PVOID) MSG_TRANSACTION_CD, 
                          MSG_TRANSACTION, 
                          sizeof(struct MSG_TRANSACTION), "MSG_TRANSACTION");
}

/*-------------------------------- END OF FILE ------------------------------*/