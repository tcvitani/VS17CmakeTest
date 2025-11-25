/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRANSACTION.H											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_TRANSACTION_H
#define MSG_TRANSACTION_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>
#include <msg_lc_header.h>
#include <time.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_TRANSACTION_CD 41L

#define MSG_TRANSACTION_DET_TRANS_TRANSACTION_ID_LENGHT		 999UL
#define MSG_TRANSACTION_ENTRY_INFO_COMPANY_ID_LENGTH		 2UL
#define MSG_TRANSACTION_ANOMALY_DESCRIPTION_LENGTH			 256UL
#define MSG_TRANSACTION_LIC_PLATE_LENGTH					 15UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_TRANSACTION_Anomaly_Description
{
	MSG_VARIANT(MSG_TRANSACTION_ANOMALY_DESCRIPTION_LENGTH)		description;
};

struct MSG_TRANSACTION_Anomaly
{
    DWORD       id;							// 0-99
    DWORD       value;						// 0-99
	HLIST       list_anomaly_descritpion;   // >= 0
};

// Entry Information
struct MSG_TRANSACTION_Entry_Info
{
    DWORD       plaza_number;               // 0-9999
    DWORD       lane_number;                // 0-9999
    SYSTEMTIME  time_of_entry_trans;         
    DWORD       collector_id;               // 0-999999
    DWORD       transaction_entry_class;    // 0-999
    DWORD       keyed_entry_class;          // 0-999
    DWORD       detected_entry_class;       // 0-999
    DWORD       entry_fare_point;           
    DWORD       destination_fare_point;     // 0-9999
    DWORD       destination_plaza_number;   // 0-9999    
    DOUBLE      prepaid_amount;             // 0-99 999 999
    DWORD       country_id;
    CHAR        company_id[MSG_TRANSACTION_ENTRY_INFO_COMPANY_ID_LENGTH+1];
    DWORD       info_getting_mode;          // 0-99
    BOOL        distribution_level;         // 0 or 1
	DWORD		enl_trs_id;					// 0-99999999
	CHAR		entry_lic_plate[MSG_TRANSACTION_LIC_PLATE_LENGTH + 1];
}; 

struct MSG_TRANSACTION
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_TRANSACTION_Body
    {
        SYSTEMTIME  time_of_transaction;
    } 
    body;

    // Shift Reference
    struct MSG_TRANSACTION_Ref_Shift 
    {
        DWORD       start_of_msg_id;   // 0-99999
        SYSTEMTIME  start_time;
    } 
    ref_shift;

    // Transaction Reference
    struct MSG_TRANSACTION_Ref_Trans
    {
        DWORD       id;             // 0-99999
        SYSTEMTIME  start_time;
        DWORD       total_traffic;   // 0-99999999
        DWORD       total_violation; // 0-99999999
    }
    ref_trans;

    // Transaction Details
    struct MSG_TRANSACTION_Det_Trans 
    {
        DWORD       transaction_class;          // 0-999
        DWORD       keyed_class;                // 0-999
        DWORD       detected_class;             // 0-999
        CHAR        transaction_id[MSG_TRANSACTION_DET_TRANS_TRANSACTION_ID_LENGHT+1];        // VARSTR
        DWORD       type_of_payment;            // 0-99
		DWORD		sub_type_of_payment;		// 0-99
        DWORD       currency_id;                // 0-99
        DOUBLE      toll_fare;                  // 0-99 999 999
        DWORD       correcting_type;            // 0-99
        LONG        correcting_sign;            // -1 or 1
        DOUBLE      correcting_value;           // 0-99 999 999
		DOUBLE		toll_fare_ht;				// 0-99 999 999	DHY 12/03/01
		DOUBLE		tax;						// 0-99 999 999	DHY 12/03/01
		DOUBLE		tax_rate;					// 0-99 999 999	DHY 12/03/01
        BOOL        valid_traffic;              // 0 or 1
        BOOL        valid_payment;              // 0 or  1
		CHAR		exit_lic_plate[MSG_TRANSACTION_LIC_PLATE_LENGTH + 1];

    } 
    det_trans;

	HLIST			entry_info;

    HLIST           list_anomalies;   // >= 0
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_TRANSACTION * WINAPI MSG_TRANSACTION_New(void);

EXPORT struct MSG_TRANSACTION_Anomaly * WINAPI MSG_TRANSACTION_Anomaly_New(HLIST *list_anomalies);

EXPORT struct MSG_TRANSACTION_Anomaly * WINAPI MSG_TRANSACTION_Get_First_Anomaly(HLIST list_anomalies);

EXPORT struct MSG_TRANSACTION_Anomaly * WINAPI MSG_TRANSACTION_Get_Next_Anomaly(HLIST list_anomalies, struct MSG_TRANSACTION_Anomaly *p_cur);

EXPORT struct MSG_TRANSACTION_Anomaly_Description * WINAPI MSG_TRANSACTION_Anomaly_Description_New(HLIST *list_anomaly_descritpion);

EXPORT struct MSG_TRANSACTION_Anomaly_Description * WINAPI MSG_TRANSACTION_Get_First_Anomaly_Description(HLIST list_anomaly_descritpion);

EXPORT struct MSG_TRANSACTION_Anomaly_Description * WINAPI MSG_TRANSACTION_Get_Next_Anomaly_Description(HLIST list_anomaly_descritpion, struct MSG_TRANSACTION_Anomaly_Description *p_anomaly_descritpion);

EXPORT struct MSG_TRANSACTION_Entry_Info * WINAPI MSG_TRANSACTION_Entry_Info_New(HLIST *entry_info);

EXPORT struct MSG_TRANSACTION_Entry_Info * WINAPI MSG_TRANSACTION_Get_First_Entry_Info(HLIST entry_info);

EXPORT struct MSG_TRANSACTION_Entry_Info * WINAPI MSG_TRANSACTION_Get_Next_Entry_Info(HLIST entry_info, struct MSG_TRANSACTION_Entry_Info *p_entry);

EXPORT BOOL WINAPI MSG_TRANSACTION_Delete_All(struct MSG_TRANSACTION *p_transaction);

EXPORT BOOL WINAPI MSG_TRANSACTION_Write(struct MSG_TRANSACTION *p_transaction, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_TRANSACTION_Read(struct MSG_TRANSACTION *p_transaction, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_TRANSACTION_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_TRANSACTION.H */

/*-------------------------------- END OF FILE ------------------------------*/