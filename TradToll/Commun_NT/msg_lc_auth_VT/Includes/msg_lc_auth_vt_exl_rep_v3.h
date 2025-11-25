/*------   (v) 2014 Sanef ITS Technologies  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL MSG
* FICHIER: msg_lc_auth_vt_exl_rep_v3.h
* MSGGAGE: C
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module MSG
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef MSG_LC_AUTH_VT_EXL_REP_V3_H
#define MSG_LC_AUTH_VT_EXL_REP_V3_H

/*--------------- INCLUDES: ---------------*/

#include <csr_msg.h>
#include <msg_lc_header.h>
#include <time.h>

/*--------------- RESERVED: ---------------*/

#ifdef MSG_LC_AUTH_VT_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define MSG_LC_AUTH_VT_EXL_REP_V3_CD 94L

#define MSG_LC_AUTH_VT_EXL_REP_V3_ENL_TRS_ID_LENGTH			30UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_ENL_VRN_LENGTH			20UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_ENL_VRN_COUNTRY_LENGTH	3UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_ENL_PROVIDER_LENGTH		6UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_TRS_ID_LENGTH				30UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_VRN_LENGTH				20UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_VRN_COUNTRY_LENGTH		3UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_PROVIDER_LENGTH			6UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_INFO_VALUE_LENGTH			4000UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_ACCEPTED_DAYS_LENGTH		2UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_USER_ACC					10UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_ACC_BALANCE				10UL
#define MSG_LC_AUTH_VT_EXL_REP_V3_DISCOUNT					2UL
/*--------------- TYPEDEFS: ---------------*/

struct MSG_LC_AUTH_VT_EXL_REP_V3_InfoType
{
	DWORD		type;														//0-99
	HLIST		list_info_type;												//>= 0
};

struct MSG_LC_AUTH_VT_EXL_REP_V3_ElemInfo
{
	DWORD		code;															// 0-99
	MSG_VARIANT(MSG_LC_AUTH_VT_EXL_REP_V3_INFO_VALUE_LENGTH)	info_value;
};

struct MSG_LC_AUTH_VT_EXL_REP_V3
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
	struct MSG_LC_AUTH_EXL_REP_V3_Body
    {
		SYSTEMTIME	dte_ref_entry;																//			Entry transaction reference date and time (internal)
		DWORD		entry_trs_ref_num;															//0-99999	Entry transaction reference ID (internal) 
		DWORD		entry_plaza_id;																//0-9999	Entry plaza identifier
		DWORD		entry_lane_id;																//0-9999	Entry lane identifier
		CHAR		entry_trs_id[MSG_LC_AUTH_VT_EXL_REP_V3_ENL_TRS_ID_LENGTH + 1];				// VARSTR	(30)	Entry transaction identifier
		SYSTEMTIME	dte_entry;																	//			Date and time of entry
		CHAR		entry_vrn[MSG_LC_AUTH_VT_EXL_REP_V3_ENL_VRN_LENGTH + 1];					// VARSTR	(20)	VRN detected at the entry lane
		CHAR		entry_vrn_country[MSG_LC_AUTH_VT_EXL_REP_V3_ENL_VRN_COUNTRY_LENGTH + 1];	// VARSTR	(3)	VRN country detected by the entry lane ALPR camera
		CHAR		entry_provider[MSG_LC_AUTH_VT_EXL_REP_V3_ENL_PROVIDER_LENGTH + 1];			// VARSTR	(6)	VT service provider ID detected at the entry lane
		SYSTEMTIME	dte_ref_exit;																//			Exit transaction reference date and time (internal)
		DWORD		exit_trs_ref_num;															//0-99999	Exit transaction reference ID (internal)
		DWORD		exit_plaza_id;																//0-9999	Exit plaza identifier
		DWORD		exit_lane_id;																//0-9999	Exit lane identifier
		CHAR		exit_trs_id[MSG_LC_AUTH_VT_EXL_REP_V3_TRS_ID_LENGTH + 1];					// VARSTR	(30)	Exit transaction identifier
		SYSTEMTIME	dte_exit;																	//			Date and time of exit
		CHAR		exit_vrn[MSG_LC_AUTH_VT_EXL_REP_V3_VRN_LENGTH + 1];							// VARSTR	(20)	Referent VRN detected at the exit lane or corrected by supervisor
		CHAR		exit_vrn_country[MSG_LC_AUTH_VT_EXL_REP_V3_VRN_COUNTRY_LENGTH + 1];			// VARSTR	(3)	VRN country detected by the exit lane ALPR camera
		CHAR		exit_provider[MSG_LC_AUTH_VT_EXL_REP_V3_PROVIDER_LENGTH + 1];				// VARSTR	(6)	VT service provider ID detected at the exit lane
		DWORD		flag_eticket;																// 0-9		Flag indicates the message type:	0 – VT user		1 – E - ticket(for users who pay by other MOP)
		DWORD		flag_delete;																// 0-9		0 – NO, 1 – YES
		DWORD		flag_open_system;															// 0-9		0 – NO, 1 – YES
		DWORD		flag_no_update;																// 0-9		0 – NO, 1 – YES
		DWORD		flag_result;																// 0-9		0 – NOK, 1 – OK 
		DWORD		reason_id;																	// 0-999999	1 – if the database is offline (DB connection error), 2 - entry record not found, 9 - ALPR not VT user,
																								// Otherwise, it returns ORA error number
		DWORD		ticket_type;																//0-99		Ticket type
		DWORD		vehicle_class;																//0-99		Vehicle class
		DWORD		payment_subtype;															//			VT service payment subtype
		CHAR		accepted_days[MSG_LC_AUTH_VT_EXL_REP_V3_ACCEPTED_DAYS_LENGTH + 1];			//			Byte to represent accepted days (like in white list)
		DWORD		exempt_acccount_number;														//0-9999999	Exempt account number (like in VRN table)
		DWORD		exempt_user_number;															//0-9999	Exempt user number (like in VRN table)
		DWORD		exempt_renewal_number;														//0-99		Exempt renewal number /Id (like CIN in VRN table)
		CHAR		account_balance[MSG_LC_AUTH_VT_EXL_REP_V3_ACC_BALANCE + 1];					//VARSTR	(10)	Account balance
		CHAR		user_balance[MSG_LC_AUTH_VT_EXL_REP_V3_USER_ACC + 1];						//VARSTR	(10)	User balance
		CHAR		discount_group[MSG_LC_AUTH_VT_EXL_REP_V3_DISCOUNT + 1];						//VARSTR	(2)	Discount group
		DWORD		contract_type;																//0-99		VT service contract type
		DWORD		request_id;																	//0-999999	VT service request id

    } 
    body;

	HLIST		list_comp_info;
};

/*--------------- FUNCTIONS: ---------------*/

EXPORT struct MSG_LC_AUTH_VT_EXL_REP_V3 * WINAPI MSG_LC_AUTH_VT_EXL_REP_V3_New(void);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(struct MSG_LC_AUTH_VT_EXL_REP_V3 *p_pmtauthrep);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_EXL_REP_V3_Write(struct MSG_LC_AUTH_VT_EXL_REP_V3 *p_pmtauthrep,
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_EXL_REP_V3_Read(struct MSG_LC_AUTH_VT_EXL_REP_V3 *p_pmtauthrep,
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_LC_AUTH_VT_EXL_REP_V3_New_Record(HLIST *hList);

EXPORT struct MSG_LC_AUTH_VT_EXL_REP_V3_InfoType *MSG_LC_AUTH_VT_EXL_REP_V3_InfoType_New(HLIST *list_info_type);
EXPORT struct MSG_LC_AUTH_VT_EXL_REP_V3_ElemInfo *MSG_LC_AUTH_VT_EXL_REP_V3_ElemInfo_New(HLIST *list_comp_info);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_AUTH_VT_EXL_REP.H */
