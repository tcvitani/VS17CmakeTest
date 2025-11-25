/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_HOURLY.H												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_HOURLY_H
#define MSG_HOURLY_H

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

#define MSG_HOURLY_CD 44L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_HOURLY_Traffic_Det_Payment
{
	DWORD		payment_id;				// 0-99
	DWORD		traffic;				// 0-99999
};

struct MSG_HOURLY_Detailed_Traffic
{
    DWORD       class_id;                     // 0-999
	HLIST		detailed_payment;			  // 0-99
};

struct MSG_HOURLY_Currency_Det_Payment
{
	DWORD		payment_id;					//0-99
	DOUBLE		revenue;					
};

struct MSG_HOURLY_Currency
{
	DWORD		currency_id;				//0-99
	HLIST		detailed_payment;			//0-99
};

struct MSG_HOURLY
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_HOURLY_Body
    {
        SYSTEMTIME  time_of_inferior_limit;
        SYSTEMTIME  time_of_superior_limit;
		DWORD		primary_currency_id; //0-99
		DOUBLE		primary_revenue;
    } 
    body;

    // General traffic
    struct MSG_HOURLY_General_Traffic 
    {
        DWORD       total_traffic;    //0-99999
		DWORD		total_violation;  //0-99999
		DWORD		maintenance_traffic; //0-99999
		DOUBLE		non_dispatched_revenue;
    } 
    general_traffic;

    HLIST	detailed_traffic;
	HLIST   currency; //0-9
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_HOURLY * WINAPI MSG_HOURLY_New(void);

//traffic detailed payment
EXPORT struct MSG_HOURLY_Traffic_Det_Payment * WINAPI MSG_HOURLY_Traffic_Det_Payment_New(HLIST *hList);

EXPORT struct MSG_HOURLY_Traffic_Det_Payment * WINAPI MSG_HOURLY_Get_First_Traffic_Det_Payment(HLIST hList);

EXPORT struct MSG_HOURLY_Traffic_Det_Payment * WINAPI MSG_HOURLY_Get_Next_Traffic_Det_Payment(HLIST hList, struct MSG_HOURLY_Traffic_Det_Payment *p_det_pay);

//detailed traffic
EXPORT struct MSG_HOURLY_Detailed_Traffic * WINAPI MSG_HOURLY_Detailed_Traffic_New(HLIST *hList);

EXPORT struct MSG_HOURLY_Detailed_Traffic * WINAPI MSG_HOURLY_Get_First_Detailed_Traffic(HLIST hList);

EXPORT struct MSG_HOURLY_Detailed_Traffic * WINAPI MSG_HOURLY_Get_Next_Detailed_Traffic(HLIST hList, struct MSG_HOURLY_Detailed_Traffic *p_det_traffic);

//currency detailed payment
EXPORT struct MSG_HOURLY_Currency_Det_Payment * WINAPI MSG_HOURLY_Currency_Det_Payment_New(HLIST *hList);

EXPORT struct MSG_HOURLY_Currency_Det_Payment * WINAPI MSG_HOURLY_Get_First_Currency_Det_Payment(HLIST hList);

EXPORT struct MSG_HOURLY_Currency_Det_Payment * WINAPI MSG_HOURLY_Get_Next_Currency_Det_Payment(HLIST hList, struct MSG_HOURLY_Currency_Det_Payment *p_det_pay);

//currency
EXPORT struct MSG_HOURLY_Currency * WINAPI MSG_HOURLY_Currency_New(HLIST *hList);

EXPORT struct MSG_HOURLY_Currency * WINAPI MSG_HOURLY_Get_First_Currency(HLIST hList);

EXPORT struct MSG_HOURLY_Currency * WINAPI MSG_HOURLY_Get_Next_Currency(HLIST hList, struct MSG_HOURLY_Currency *p_curr);


EXPORT BOOL WINAPI MSG_HOURLY_Delete_All(struct MSG_HOURLY *p_hourly);

EXPORT BOOL WINAPI MSG_HOURLY_Write(struct MSG_HOURLY *p_hourly, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_HOURLY_Read(struct MSG_HOURLY *p_hourly, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_HOURLY_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_HOURLY.H */

/*-------------------------------- END OF FILE ------------------------------*/