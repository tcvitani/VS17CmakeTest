/*------   (v) 2014 Sanef ITS Technologies  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL MSG
* FICHIER: msg_lc_auth_vt_enl_req_v2.h
* MSGGAGE: C
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module MSG
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef MSG_LC_AUTH_VT_ENL_REQ_V2_H
#define MSG_LC_AUTH_VT_ENL_REQ_V2_H

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

#define MSG_LC_AUTH_VT_ENL_REQ_V2_CD 87L

#define MSG_LC_AUTH_VT_ENL_REQ_V2_TRS_ID_LENGTH			12UL
#define MSG_LC_AUTH_VT_ENL_REQ_V2_VRN_LENGTH			20UL
#define MSG_LC_AUTH_VT_ENL_REQ_V2_VRN_COUNTRY_LENGTH	3UL
#define MSG_LC_AUTH_VT_ENL_REQ_V2_PROVIDER_LENGTH		6UL
#define MSG_LC_AUTH_VT_ENL_REQ_V2_INFO_VALUE_LENGHT		4000UL


/*--------------- TYPEDEFS: ---------------*/
struct MSG_LC_AUTH_VT_ENL_REQ_V2_InfoType
{
	DWORD		type;														//0-99
	HLIST		list_info_type;												//>= 0
};

struct MSG_LC_AUTH_VT_ENL_REQ_V2_ElemInfo
{
	DWORD		code;															// 0-99
	MSG_VARIANT(MSG_LC_AUTH_VT_ENL_REQ_V2_INFO_VALUE_LENGHT)	info_value;
};

struct MSG_LC_AUTH_VT_ENL_REQ_V2
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_LC_AUTH_VT_ENL_REQ_V2_Body
    {
		SYSTEMTIME	dte_ref_entry;															//			Entry transaction reference date and time (internal)
		DWORD		entry_trs_ref_num;														//0-99999	Entry transaction reference ID (internal) 
		DWORD		entry_plaza_id;															//0-9999	Entry plaza identifier
		DWORD		entry_lane_id;															//0-9999	Entry lane identifier
		CHAR		entry_trs_id[MSG_LC_AUTH_VT_ENL_REQ_V2_TRS_ID_LENGTH + 1];				// VARSTR	(12)	Entry transaction identifier
		SYSTEMTIME	dte_entry;																//			Date and time of entry
		CHAR		entry_vrn[MSG_LC_AUTH_VT_ENL_REQ_V2_VRN_LENGTH + 1];					// VARSTR	(20)	VRN detected at the entry lane
		CHAR		entry_vrn_country[MSG_LC_AUTH_VT_ENL_REQ_V2_VRN_COUNTRY_LENGTH + 1];	// VARSTR	(3)	VRN country detected by the entry lane ALPR camera
		CHAR		entry_provider[MSG_LC_AUTH_VT_ENL_REQ_V2_PROVIDER_LENGTH + 1];			// VARSTR	(6)	VT service provider ID detected at the entry lane
		DWORD		ticket_type;															//0-99		Ticket type
	}
    body;

	HLIST		list_comp_info;
};

/*--------------- FUNCTIONS: ---------------*/

EXPORT struct MSG_LC_AUTH_VT_ENL_REQ_V2 * WINAPI MSG_LC_AUTH_VT_ENL_REQ_V2_New(void);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_ENL_REQ_V2_Delete_All(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *p_pmt_auth_req);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_ENL_REQ_V2_Write(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *p_pmt_auth_req,
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_ENL_REQ_V2_Read(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *p_pmt_auth_req,
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_LC_AUTH_VT_ENL_REQ_V2_New_Record(HLIST *hList);

EXPORT struct MSG_LC_AUTH_VT_ENL_REQ_V2_InfoType *MSG_LC_AUTH_VT_ENL_REQ_V2_InfoType_New(HLIST *list_info_type);
EXPORT struct MSG_LC_AUTH_VT_ENL_REQ_V2_ElemInfo *MSG_LC_AUTH_VT_ENL_REQ_V2_ElemInfo_New(HLIST *list_comp_info);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_AUTH_VT_ENL_REQ.H */
