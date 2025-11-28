//******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                            */
/* ---------------------------------------------------------------------------*/
/* FILE:     rfr_appli.h                                                      */
/* LANGUAGE: C                                                                */
/* ---------------------------------------------------------------------------*/
/* DESCRIPTION:                                                               */
/*             The REFERENCE module extension libraries use these header      */
/*             definitions to signalise to the main application when the new  */
/*             file is downloaded.                                            */
/*             These definitions are created according to the list of events  */
/*             doccument (Liste des etats Events 7.0.0).                      */
/*                                                                            */
/*                 DO NOT CHANGE IT BEFORE YOU CONSULT THE DOCUMENT!          */
/* ---------------------------------------------------------------------------*/
/* HISTORY:                                                                   */
/*                                                                            */
/******************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
#ifndef RFR_APPLI_H
#define RFR_APPLI_H
/*-------------------------------- RESERVED:  -------------------------------*/
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef enum
{
	M_RFR_APPLI_FILE
}
enum_rfr_appli_srv;

typedef enum
{
	RFR_APPLI_NEW
}
enum_rfr_appli_type;

// File types
typedef enum
{
	FIRST_FILE_TYPE,

	FILE_TYPE_RFR = FIRST_FILE_TYPE,
	FILE_TYPE_CAL,			// 1  Calendar
	FILE_TYPE_DAY,			// 2  Day types
	FILE_TYPE_TFT,			// 3  Fares
	FILE_TYPE_PRD,			// 4  Product fares
	FILE_TYPE_TCI,			// 5  Collectors identification    
	FILE_TYPE_CUR,			// 6  Currency
	FILE_TYPE_BAS,			// 7  Tag white list
	FILE_TYPE_AUT,			// 8  Magnetic card white list
	FILE_TYPE_BLCCR,		// 9  Contactless card white list
	FILE_TYPE_ETAVI,		// 10 AVI account status
	FILE_TYPE_OTL,			// 11 Tag black list
	FILE_TYPE_OCM,			// 12 Magnetic card blacklist 
	FILE_TYPE_OPCCR,		// 13 Contactless card blacklist
	FILE_TYPE_ORMAG,		// 14 Magnetic card orange list
	FILE_TYPE_ETCCR,		// 15 Contactless card status list
	FILE_TYPE_PLT,			// 16 Plaza type list
	FILE_TYPE_VRN,			// 17 Vehicle registration number list
	FILE_TYPE_TCL,			// 18 Tag conversion list
	FILE_TYPE_CIPPRD,		// 19 CIP product codes
	FILE_TYPE_CIPCAL,		// 20 CIP calendar list
	FILE_TYPE_CIPCLS,		// 21 CIP class list
	FILE_TYPE_CIPPLZ,		// 22 CIP plaza list
	FILE_TYPE_CIPHOL,		// 23 CIP holiday list
	FILE_TYPE_CIPOCM,		// 24 CIP card opposition list
	FILE_TYPE_CIPOTL,		// 25 CIP tag opposition list
	FILE_TYPE_CIPWHT,		// 26 CIP white list
	FILE_TYPE_VSW,			// 27 Vault switching calendar
	FILE_TYPE_LTB,			// 28 Tag accepted list
	FILE_TYPE_EMC,			// 29 Contract issuer list
	FILE_TYPE_CNT,			// 30 Contract type list
	FILE_TYPE_CARDMECLS,	// 31 CARDME class list
	FILE_TYPE_CARDMEOTL,	// 32 CARDME opposition list
	FILE_TYPE_CLS,			// 33 Class definition list
	FILE_TYPE_AF1,			// 34 Patron external display 1 text
	FILE_TYPE_AF2,			// 35 Patron external display 2 text
	FILE_TYPE_EPT,			// 36 List of entry points
	FILE_TYPE_DPA,			// 37 List of daily pass users
	FILE_TYPE_COMID,		// 38 List of companies ordered by identifiers
	FILE_TYPE_COMNAME,		// 39 List of companies ordered by name
	FILE_TYPE_ITL,			// 40 TAG issuer list
	FILE_TYPE_ETL,			// 41 TAG exception list
	FILE_TYPE_REC,			// 42 List of ETC user recharges 
	FILE_TYPE_DSG,			// 43 Discount group list
	FILE_TYPE_FREE,			// 44 List of exempts
	FILE_TYPE_ETCW,			// 45 ETC white list
	FILE_TYPE_TIS,			// 46 TIS list
	FILE_TYPE_EXT,			// 47
	FILE_TYPE_CLM,			// 48 Class mapping list
	FILE_TYPE_GTC,			// 49 GTC black list
	FILE_TYPE_DKV,			// 50 DKV black list
	FILE_TYPE_UTA,			// 51 UTA black list
	FILE_TYPE_RTX,			// 52 RTX black list
	FILE_TYPE_SHL,			// 53 SHL black list
	FILE_TYPE_E10,			// 54 E10 black list
	FILE_TYPE_ORL,			// 55 ORL black list
	FILE_TYPE_LGP,			// 56 LogPay black list
	FILE_TYPE_LOT,			// 57 Lotos black list
	FILE_TYPE_WAG,			// 58 EUROWAG black list
	FILE_TYPE_OTCL,			// 59 LIST OF REJECTED GTC CARDS
	FILE_TYPE_VTP,			// 60 List of Video Toll Providers
	FILE_TYPE_VTBM,			// 61 Blue Media White list
	FILE_TYPE_VTW,			// 62 Video Toll (Generic) White List
	FILE_TYPE_EVRN,			// 63 Extended Vehicle registration number list
	FILE_TYPE_EXL,			// 64 Exception list
	FILE_TYPE_TCE,			// 65 Collectors identification
    FILE_TYPE_FCP,			// 66 Fiscalization common parameters 
	FILE_TYPE_MCL,			// 67 Media conversion list 

	//--------------------- GENERIC FILE TYPES TO BE USED with CSR_RFR11_GENERIC
	FILE_TYPE_GENERIC_1,		// 68 
	FILE_TYPE_GENERIC_2,		// 69 
	FILE_TYPE_GENERIC_3,		// 70 
	FILE_TYPE_GENERIC_4,		// 71 
	FILE_TYPE_GENERIC_5,		// 72 
	FILE_TYPE_GENERIC_6,		// 73 
	FILE_TYPE_GENERIC_7,		// 74 
	FILE_TYPE_GENERIC_8,		// 75 
	FILE_TYPE_GENERIC_9,		// 76 
	FILE_TYPE_GENERIC_10,		// 77 
	FILE_TYPE_GENERIC_11,		// 78 
	FILE_TYPE_GENERIC_12,		// 79 
	FILE_TYPE_GENERIC_13,		// 80 
	FILE_TYPE_GENERIC_14,		// 81 
	FILE_TYPE_GENERIC_15,		// 82 
	FILE_TYPE_GENERIC_16,		// 83 
	FILE_TYPE_GENERIC_17,		// 84 
	FILE_TYPE_GENERIC_18,		// 85 
	FILE_TYPE_GENERIC_19,		// 86 
	FILE_TYPE_GENERIC_20,		// 87 

	BUTEE_FILE_TYPE,
	LAST_FILE_TYPE = BUTEE_FILE_TYPE - 1,
	NB_FILE_TYPE = BUTEE_FILE_TYPE - FIRST_FILE_TYPE
}
enum_file_type;

typedef struct
{
	enum_file_type				file_id;
	char						file_name[MAX_PATH + 1];
	BOOL						manual;
}
struct_rfr_appli_file;

typedef struct
{
	struct_neutre				neutre;
	enum_rfr_appli_srv			service;
	enum_rfr_appli_type			type;
}
struct_rfr_appli_entete;

typedef struct
{
	struct_rfr_appli_entete		entete;
	union
	{
		struct_rfr_appli_file	file;
	}u;
}
struct_rfr_appli_msg;
/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- VARIABLES: -------------------------------*/
#include <undef.h>
#endif
/*-------------------------------- END OF FILE ------------------------------*/