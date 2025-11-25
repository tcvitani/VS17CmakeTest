/*------   (v) 2014 Sanef ITS Technologies  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL MSG
* FICHIER: msg_lc_auth_vt_exl_rep.h
* MSGGAGE: C
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module MSG
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef MSG_LC_AUTH_VT_EXL_REP_H
#define MSG_LC_AUTH_VT_EXL_REP_H

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

#define MSG_LC_AUTH_VT_EXL_REP_CD 86L

#define MSG_LC_AUTH_VT_EXL_REP_ENL_TRS_ID_LENGTH		12UL
#define MSG_LC_AUTH_VT_EXL_REP_ENL_VRN_LENGTH			20UL
#define MSG_LC_AUTH_VT_EXL_REP_ENL_VRN_COUNTRY_LENGTH	3UL
#define MSG_LC_AUTH_VT_EXL_REP_ENL_PROVIDER_LENGTH		6UL
#define MSG_LC_AUTH_VT_EXL_REP_TRS_ID_LENGTH			12UL
#define MSG_LC_AUTH_VT_EXL_REP_VRN_LENGTH				20UL
#define MSG_LC_AUTH_VT_EXL_REP_VRN_COUNTRY_LENGTH		3UL
#define MSG_LC_AUTH_VT_EXL_REP_PROVIDER_LENGTH			6UL

/*--------------- TYPEDEFS: ---------------*/

struct MSG_LC_AUTH_VT_EXL_REP
{
    // Message Header
    struct MSG_HEADER header;
    
    // Message Body
    struct MSG_LC_AUTH_EXL_REP_Body
    {
		SYSTEMTIME	dte_ref_entry;															//			Entry transaction reference date and time (internal)
		DWORD		entry_trs_ref_num;														//0-99999	Entry transaction reference ID (internal) 
		DWORD		entry_plaza_id;															//0-9999	Entry plaza identifier
		DWORD		entry_lane_id;															//0-9999	Entry lane identifier
		CHAR		entry_trs_id[MSG_LC_AUTH_VT_EXL_REP_ENL_TRS_ID_LENGTH + 1];				// VARSTR	(12)	Entry transaction identifier
		SYSTEMTIME	dte_entry;																//			Date and time of entry
		CHAR		entry_vrn[MSG_LC_AUTH_VT_EXL_REP_ENL_VRN_LENGTH + 1];					// VARSTR	(20)	VRN detected at the entry lane
		CHAR		entry_vrn_country[MSG_LC_AUTH_VT_EXL_REP_ENL_VRN_COUNTRY_LENGTH + 1];	// VARSTR	(3)	VRN country detected by the entry lane ALPR camera
		CHAR		entry_provider[MSG_LC_AUTH_VT_EXL_REP_ENL_PROVIDER_LENGTH + 1];			// VARSTR	(6)	VT service provider ID detected at the entry lane
		SYSTEMTIME	dte_ref_exit;															//			Exit transaction reference date and time (internal)
		DWORD		exit_trs_ref_num;														//0-99999	Exit transaction reference ID (internal)
		DWORD		exit_plaza_id;															//0-9999	Exit plaza identifier
		DWORD		exit_lane_id;															//0-9999	Exit lane identifier
		CHAR		exit_trs_id[MSG_LC_AUTH_VT_EXL_REP_TRS_ID_LENGTH + 1];					// VARSTR	(12)	Exit transaction identifier
		SYSTEMTIME	dte_exit;																//			Date and time of exit
		CHAR		exit_vrn[MSG_LC_AUTH_VT_EXL_REP_VRN_LENGTH + 1];						// VARSTR	(20)	Referent VRN detected at the exit lane or corrected by supervisor
		CHAR		exit_vrn_country[MSG_LC_AUTH_VT_EXL_REP_VRN_COUNTRY_LENGTH + 1];		// VARSTR	(3)	VRN country detected by the exit lane ALPR camera
		CHAR		exit_provider[MSG_LC_AUTH_VT_EXL_REP_PROVIDER_LENGTH + 1];				// VARSTR	(6)	VT service provider ID detected at the exit lane
		DWORD		flag_eticket;															// 0-9		Flag indicates the message type:	0 – VT user		1 – E - ticket(for users who pay by other MOP)
		DWORD		flag_result;															// 0-9		0 – NOK, 1 – OK 
		DWORD		reason_id;																// 0-999999	1 – if the database is offline (DB connection error) Otherwise, it returns ORA error number

    } 
    body;

};

/*--------------- FUNCTIONS: ---------------*/

EXPORT struct MSG_LC_AUTH_VT_EXL_REP * WINAPI MSG_LC_AUTH_VT_EXL_REP_New (void);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_EXL_REP_Delete_All (struct MSG_LC_AUTH_VT_EXL_REP *p_pmtauthrep);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_REP_EXL_Write (struct MSG_LC_AUTH_VT_EXL_REP *p_pmtauthrep, 
                                      BYTE *p_msg, 
                                      DWORD msg_size_max, 
                                      DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_LC_AUTH_VT_EXL_REP_Read (struct MSG_LC_AUTH_VT_EXL_REP *p_pmtauthrep, 
                                     BYTE *p_msg, 
                                     DWORD msg_size_max, 
                                     DWORD *final_msg_size);

EXPORT BOOL MSG_LC_AUTH_VT_EXL_REP_New_Record (HLIST *hList);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_LC_AUTH_VT_EXL_REP.H */
