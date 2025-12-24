/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     csr_emi_pic.h                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The "Picture transfer" module interface file.                 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef EMI_PIC_H
#define EMI_PIC_H
/*-------------------------------- INCLUDES:  -------------------------------*/
#include <noyau.h>
#include <run.h>
#include <module.h>

// Type of declaration
#ifdef EMI_PIC_DEF
#	include <public.h>
#else
#	include <export.h>
#endif
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define EMI_REG_KEYn_ModEMI_PIC       "EMI_PIC\\"
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/**----- List of services --------------------------------------------------**/
typedef enum
{
	M_EMI_PIC_RESTITUTION = M_SRV_USER,
	M_EMI_PIC_TRANSFER,
	M_EMI_PIC_EMISSION,
	M_EMI_PIC_MAINT_MODE,

	BUTEE_EMI_PIC_SERVICE,
	DERNIER_EMI_PIC_SERVICE = BUTEE_EMI_PIC_SERVICE - 1,
	NB_EMI_PIC_SERVICE = BUTEE_EMI_PIC_SERVICE - PREMIER_SRV_SERVICE
}
enum_emi_pic_service;
/**----- List of message types -----------------------------------**/
typedef enum
{
	M_EMI_PIC_TYP_RESTIT_BY_DATE = SRV_TYP_USER,
	M_EMI_PIC_TYP_RESTIT_BY_DATE_EFFECTUE,
	M_EMI_PIC_TYP_RESTIT_BY_DATE_NON_EFFECTUE,

	M_EMI_PIC_TYP_RESTIT_BY_FILE,
	M_EMI_PIC_TYP_RESTIT_BY_FILE_EFFECTUE,
	M_EMI_PIC_TYP_RESTIT_BY_FILE_NON_EFFECTUE,

	M_EMI_PIC_TYP_TRANSFER_SEND,
	M_EMI_PIC_TYP_TRANSFER_SEND_EFFECTUE,
	M_EMI_PIC_TYP_TRANSFER_SEND_NON_EFFECTUE,

	M_EMI_PIC_TYP_TRANSFER_DEL,
	M_EMI_PIC_TYP_TRANSFER_DEL_EFFECTUE,
	M_EMI_PIC_TYP_TRANSFER_DEL_NON_EFFECTUE,

	M_EMI_PIC_TYP_EMISSION_PURGE,
	M_EMI_PIC_TYP_EMISSION_PURGE_EFFECTUE,
	M_EMI_PIC_TYP_EMISSION_PURGE_NON_EFFECTUE,

	// MFR start 23/7/2008
	M_EMI_PIC_TYP_RESTIT_BY_DATETIME,
	M_EMI_PIC_TYP_RESTIT_BY_DATETIME_EFFECTUE,
	M_EMI_PIC_TYP_RESTIT_BY_DATETIME_NON_EFFECTUE,

	M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID,
	M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_EFFECTUE,
	M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_NON_EFFECTUE,	
	// MFR end   23/7/2008

	// MFR start 2009/10/29
	M_EMI_PIC_MAINT_MODE_START,
	M_EMI_PIC_MAINT_MODE_START_ACK,
	M_EMI_PIC_MAINT_MODE_START_NACK,
	M_EMI_PIC_MAINT_MODE_UPDATE,	
	M_EMI_PIC_MAINT_MODE_EFFECTUE,
	M_EMI_PIC_MAINT_MODE_NON_EFFECTUE,
	M_EMI_PIC_MAINT_MODE_STOP,
	// MFR end

	BUTEE_EMI_PIC_TYPE,
	DERNIER_EMI_PIC_TYPE = BUTEE_EMI_PIC_TYPE - 1,
	NB_EMI_PIC_TYPE = BUTEE_EMI_PIC_TYPE - SRV_TYP_USER
}
enum_emi_pic_type;
/**----- Structures of the RESTITUTION service  ----------------------------**/
/***---- Def. of the M_EMI_PIC_RESTIT_BY_DATE message structure ----------****/
typedef struct
{
	char szBegin[9];
	char szEnd[9];
} struct_emi_pic_restit_date;
/***---- Def. of the M_EMI_PIC_RESTIT_BY_FILE message structure ----------****/
typedef struct
{
	char szFileName[MAX_PATH];
} struct_emi_pic_restit_file;
/***---- Def. of the M_EMI_PIC_RESTIT_BY_DATETIME message structure ----------****/
typedef struct
{
	SYSTEMTIME
		sBegin,
		sEnd;
} struct_emi_pic_restit_datetime;
/***---- Def. of the M_EMI_PIC_RESTIT_BY_DATETIME message structure ----------****/
typedef struct
{
	LONG
		lRecordID;
} struct_emi_pic_restit_record_id;
/***---- The RESTITUTION service message structure -----------------------****/
typedef struct
{
	union
	{
		struct_emi_pic_restit_date		sRequestByDate;
		struct_emi_pic_restit_file		sRequestByFile;
		struct_emi_pic_restit_datetime	sRequestByDateTime;
		struct_emi_pic_restit_record_id sRequestByRecordID;
	} u;
} struct_emi_pic_restit;
/**----- Structures of the TRANSFER service  -------------------------------**/
/***---- The TRANSFER service message structure --------------------------****/
typedef struct
{
	char szDirName[MAX_PATH];
	char szFileMask[MAX_PATH];
} struct_emi_pic_trf;
/**----- Structures of the EMISSION service  -------------------------------**/
/***---- The EMISSION service message structure --------------------------****/
typedef struct
{
	char cDummy;
}
struct_emi_pic_emi;
/**----- Structures of the ETAT service  -------------------------------**/
/***---- The ETAT service message structure --------------------------****/
typedef struct
{
	// First octet
	unsigned int errorLink		: 1;
	unsigned int reserved		: 15;
	// Second octet
}struct_emi_pic_etat, *LP_EMI_PIC_ETAT;
/**----- Structures of the MAINT_MODE service  -------------------------------**/
/***---- The MAINT_MODE service message structure --------------------------****/
typedef struct
{
	LONG NbDirs;
}struct_dirs_data, *LP_DIRS_DATA;

typedef struct
{
	union
	{
		struct_dirs_data	dirs_data;
		CHAR				szTargetDir[MAX_PATH];
		LONG NbFiles;
	}u;
}struct_maint_mode, *LP_MAINT_MODE;
/**----- The module message structure  -------------------------------------**/
typedef struct
{
	struct_srv_entete entete;
	union
	{
		struct_emi_pic_restit   srv_restit;
		struct_emi_pic_trf      srv_trf;
		struct_emi_pic_emi      srv_emi;
		struct_srv_etat			srv_etat;
		struct_maint_mode		srv_maint_mode;
	}u;
} struct_emi_pic_message;
/*-------------------------------- FUNCTIONS: -------------------------------*/
EXPORT enum_instance_result WINAPI EmiPicLance(char * pcKey,
											   char * pcBalNam,
											   noyau_bal_id * piBalId);
EXPORT enum_instance_result WINAPI EmiPicArret(noyau_bal_id iBalId);
/*-------------------------------- VARIABLES: -------------------------------*/
#include <undef.h>
#endif
/*-------------------------------- END OF FILE ------------------------------*/