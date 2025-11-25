/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_PAYMENT.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PAYMENT_H
#define MSG_PAYMENT_H

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

#define MSG_PAYMENT_CD 42L


#define MSG_PAYMENT_INFO_VALUE_LENGTH			4000UL
#define MSG_PAYMENT_PRODUCT_CODE_LENGTH			8UL
#define MSG_PAYMENT_ANOMALY_DESCRIPTION_LENGTH	256UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PAYMENT_Official_Currency
{
    DWORD       id;                     // 0-99
    DOUBLE      unit_price;             // 0-99 999 999
};

struct MSG_PAYMENT_Sold_Product
{
    DWORD       id;                     // 0-99
    DWORD       quantity;               // 0-255
    LONG        sign;                   // -1 ou 1
    HLIST       list_official_currencies; // >= 1
};

struct MSG_PAYMENT_Anomaly_Description
{
	MSG_VARIANT(MSG_PAYMENT_ANOMALY_DESCRIPTION_LENGTH)		description;
};

struct MSG_PAYMENT_Anomaly
{
	DWORD       id;							// 0-99
	DWORD       value;						// 0-99
	HLIST		list_anomaly_descritpion;   // >= 0	
};

struct MSG_PAYMENT_Elem_Info	// Elementary Information Detail
{
    DWORD       code;					// 0-99
	MSG_VARIANT(MSG_PAYMENT_INFO_VALUE_LENGTH)		info_value;
};

struct MSG_PAYMENT_Info_Type	// Informatin Type
{
    DWORD       type;                   // 0-99
    HLIST       list_info_type;			// >= 1
};

struct MSG_PAYMENT
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_PAYMENT_Body
    {
        SYSTEMTIME  time_of_payment;
    } 
    body;

    // Transaction Reference
    struct MSG_PAYMENT_Ref_Trs 
    {
        DWORD       internal_id;   
        SYSTEMTIME  start_time;
    } 
    ref_trs;

    // Payment detail
    HLIST           list_sold_products;

    // Payment Value
    struct MSG_PAYMENT_Value 
    {
        DWORD       currency_id;                // 0-99
        DOUBLE      in_currency;                // 0-99 999 999
        DWORD       balance_type;               // 0-1
        DOUBLE      balance_value_after_trs;    // 0-99 999 999
        BOOL        valid;                      
    } 
    value;

    // Payment Type
    struct MSG_PAYMENT_Type
    {
        DWORD       type;                       // 0-999
        DWORD       sub_type;                   // 0-999
        DWORD       acquisition_mode;           // 0-9
        CHAR        primary_product_code[MSG_PAYMENT_PRODUCT_CODE_LENGTH+1];
        DWORD       primary_commercial_type;    // 0-99
        CHAR        product_code_used[MSG_PAYMENT_PRODUCT_CODE_LENGTH+1];
        DWORD       commercial_type_used;       // 0-99
    } 
    type;

    HLIST           list_anomalies;   // >= 0

    HLIST           list_comp_inf;     // >= 0
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PAYMENT * WINAPI MSG_PAYMENT_New(void);

EXPORT struct MSG_PAYMENT_Sold_Product * WINAPI MSG_PAYMENT_Sold_Product_New(HLIST *hList);

EXPORT struct MSG_PAYMENT_Sold_Product * WINAPI MSG_PAYMENT_Get_First_Sold_Product(HLIST hList);

EXPORT struct MSG_PAYMENT_Sold_Product * WINAPI MSG_PAYMENT_Get_Next_Sold_Product(HLIST hList, struct MSG_PAYMENT_Sold_Product *p_sold);

EXPORT struct MSG_PAYMENT_Official_Currency * WINAPI MSG_PAYMENT_Official_Currency_New(HLIST *hList);

EXPORT struct MSG_PAYMENT_Official_Currency * WINAPI MSG_PAYMENT_Get_First_Official_Currency(HLIST hList);

EXPORT struct MSG_PAYMENT_Official_Currency * WINAPI MSG_PAYMENT_Get_Next_Official_Currency(HLIST hList, struct MSG_PAYMENT_Official_Currency *p_cur);

EXPORT struct MSG_PAYMENT_Anomaly * WINAPI MSG_PAYMENT_Anomaly_New(HLIST *list_anomalies);

EXPORT struct MSG_PAYMENT_Anomaly * WINAPI MSG_PAYMENT_Get_First_Anomaly(HLIST list_anomalies);

EXPORT struct MSG_PAYMENT_Anomaly * WINAPI MSG_PAYMENT_Get_Next_Anomaly(HLIST list_anomalies, struct MSG_PAYMENT_Anomaly *p_cur);

EXPORT struct MSG_PAYMENT_Info_Type * WINAPI MSG_PAYMENT_Info_Type_New(HLIST *hList);

EXPORT struct MSG_PAYMENT_Info_Type * WINAPI MSG_PAYMENT_Get_First_Info_Type(HLIST hList);

EXPORT struct MSG_PAYMENT_Info_Type * WINAPI MSG_PAYMENT_Get_Next_Info_Type(HLIST hList, struct MSG_PAYMENT_Info_Type *p_type);

EXPORT struct MSG_PAYMENT_Elem_Info * WINAPI MSG_PAYMENT_Elem_Info_New(HLIST *hList);

EXPORT struct MSG_PAYMENT_Elem_Info * WINAPI MSG_PAYMENT_Get_First_Elem_Info(HLIST hList);

EXPORT struct MSG_PAYMENT_Elem_Info * WINAPI MSG_PAYMENT_Get_Next_Elem_Info(HLIST hList, struct MSG_PAYMENT_Elem_Info *p_elem);

EXPORT struct MSG_PAYMENT_Anomaly_Description * WINAPI MSG_PAYMENT_Anomaly_Description_New(HLIST *list_anomaly_descritpion);

EXPORT struct MSG_PAYMENT_Anomaly_Description * WINAPI MSG_PAYMENT_Get_First_Anomaly_Description(HLIST list_anomaly_descritpion);

EXPORT struct MSG_PAYMENT_Anomaly_Description * WINAPI MSG_PAYMENT_Get_Next_Anomaly_Description(HLIST list_anomaly_descritpion, struct MSG_PAYMENT_Anomaly_Description *p_anomaly_descritpion);

EXPORT BOOL WINAPI MSG_PAYMENT_Delete_All(struct MSG_PAYMENT *p_payment);

EXPORT BOOL WINAPI MSG_PAYMENT_Write(struct MSG_PAYMENT *p_payment, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PAYMENT_Read(struct MSG_PAYMENT *p_payment, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_PAYMENT_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_PAYMENT.H */

/*-------------------------------- END OF FILE ------------------------------*/