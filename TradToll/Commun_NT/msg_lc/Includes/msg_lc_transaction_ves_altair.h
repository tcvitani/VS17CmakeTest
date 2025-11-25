/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_TRANSACTION_VES_ALTAIR.H								 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_TRANSACTION_VES_ALTAIR_H
#define MSG_TRANSACTION_VES_ALTAIR_H

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

#define MSG_TRANSACTION_VES_ALTAIR_CD 95L

#define MSG_TRANSACTION_VES_ALTAIR_DET_TRANS_TRANSACTION_VES_ID_LENGHT		999UL
#define MSG_TRANSACTION_VES_ALTAIR_ENTRY_INFO_COMPANY_ID_LENGTH			2UL
#define MSG_TRANSACTION_VES_ALTAIR_INFO_VALUE_LENGTH	                   		4000UL

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail  // Elementary Information Detail
{
    DWORD                                           info_code;	// 0-99
	MSG_VARIANT(MSG_TRANSACTION_VES_ALTAIR_INFO_VALUE_LENGTH)		info_value;
};


// Complementary Information
struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info
{
    DWORD       info_type;               // 0-9999

	HLIST		list_info_detail;		 
}; 

struct MSG_TRANSACTION_VES_ALTAIR
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_TRANSACTION_VES_ALTAIR_Body
    {
        DWORD  command;
    } 
    body;

    // Transaction Details
    struct MSG_TRANSACTION_VES_ALTAIR_Det_Trans 
    {
	    SYSTEMTIME  time_of_transaction;         
        DWORD       transaction_class;          //0-99
        DWORD       keyed_class;                //0-99
        DWORD       detected_class;             //0-99
        DWORD       type_of_payment;            //0-99
        DOUBLE      toll_fare;                  //0.00000001-9 999 999 999
        DWORD       anomaly;                    //0-99
    } 
    det_trans;

	HLIST			complementary_info;			

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_TRANSACTION_VES_ALTAIR * WINAPI MSG_TRANSACTION_VES_ALTAIR_New(void);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail * WINAPI MSG_TRANSACTION_VES_ALTAIR_Info_Detail_New(HLIST *list_info_detail);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_First_Info_Detail(HLIST list_info_detail);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_Next_Info_Detail(HLIST list_info_detail, struct MSG_TRANSACTION_VES_ALTAIR_Info_Detail *p_cur);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info * WINAPI MSG_TRANSACTION_VES_ALTAIR_Complementary_Info_New(HLIST *complementary_info);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_First_Complementary_Info(HLIST complementary_info);

EXPORT struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info * WINAPI MSG_TRANSACTION_VES_ALTAIR_Get_Next_Complementary_Info(HLIST complementary_info, struct MSG_TRANSACTION_VES_ALTAIR_Complementary_Info *p_entry);

EXPORT BOOL WINAPI MSG_TRANSACTION_VES_ALTAIR_Delete_All(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction);

EXPORT BOOL WINAPI MSG_TRANSACTION_VES_ALTAIR_Write(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_TRANSACTION_VES_ALTAIR_Read(struct MSG_TRANSACTION_VES_ALTAIR *p_transaction, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_TRANSACTION_VES_ALTAIR_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_TRANSACTION_VES.H */

/*-------------------------------- END OF FILE ------------------------------*/