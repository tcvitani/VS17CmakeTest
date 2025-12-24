/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_glob.h                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             Definition of the "EMI_PIC" module internal typedefs and      */
/*             global values.                                                */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef EMI_PIC_GLOB_H
#define EMI_PIC_GLOB_H
/*-------------------------------- INCLUDES:  -------------------------------*/
// CS Route interface
#include <csrlc32.h>
#include <noyau.h>
#include <debug.h>
#include <run.h>
#include <module.h>
#include <reg.h>
#include <run.h>
#include <csr_srv.h>
// Module interface
#include <csr_emi_pic.h>

#include <emi_pic_import.h>
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define EMI_PIC_NOM_FICHIER_TRACES         "EMI_PIC"
#define EmiPicFichierDebug                 EMI_PIC_FILE=__FILE__,\
                                           EMI_PIC_LINE=__LINE__,\
                                           DEFINE_EmiPicFichierDebug
#define EMI_PIC_INSTANCE_MAX               5
// Registry entries specific for the EMI_PIC module
#define EMI_PIC_REG_KEYv_LOCAL_DIR        "LocalDirectory"
#define EMI_PIC_REG_KEYv_BACKUP_DIR       "BackupDirectory"
#define EMI_PIC_REG_KEYv_EMI_PERIOD       "EmissionPeriodMS"
#define EMI_PIC_REG_KEYv_LAST_PURGE_TIME  "LastPurgeDateTime"
#define EMI_PIC_REG_KEYv_PURGE_PERIOD     "PurgePeriodDays"
#define EMI_PIC_REG_KEYv_PURGE_TIME       "PurgeTime"
#define EMI_PIC_REG_KEYv_PURGE_FILE_DELAY "PurgeFileDelayMS"

#define EMI_PIC_REG_KEYv_IMPORT_PERIOD		"ImportPeriodMS"
#define EMI_PIC_REG_KEYv_IMPORT_DIR			"ImportFromDir"

#define EMI_PIC_REG_KEYv_REEMISSION_PERIOD	"ReemissionPeriodMS"

#define EMI_PIC_REG_KEYn_DESTINATION_CONFIG	"ConfigDestinations\\"

#define EMI_PIC_REG_KEYv_DESTINATION_DIR	"DestinationDir"
#define EMI_PIC_REG_KEYv_PATH_FORMAT		"PathFormat"
#define EMI_PIC_REG_KEYv_TEMP_EXTENSION		"ExtensionDuringCopy"

#define EMI_PIC_REG_KEYv_FILENAME_FORMAT	"FileNameFormat"
#define EMI_PIC_REG_KEYv_LOCAL_FORMAT		"ImportDirFormat"

#define EMI_PIC_REG_KEYv_USE_COUNTER		"UseTransmissionCounter"

#define DEFAULT_DELAY	-1
/*-------------------------------- TYPEDEFS:  -------------------------------*/
enum index_traces
{
	EMI_PIC_TRC,
	EMI_PIC_NB_TRACES
};

typedef enum
{
	EMI_PIC_FIRST_ID_THREAD = 0,

	EMI_PIC_ID_THREAD_ANI = EMI_PIC_FIRST_ID_THREAD,
	EMI_PIC_ID_THREAD_EMI,
	EMI_PIC_ID_THREAD_RESTIT,
	EMI_PIC_ID_THREAD_TRF,
	EMI_PIC_ID_THREAD_IMPORT,

	EMI_PIC_END_ID_THREAD,
	EMI_PIC_LAST_ID_THREAD = EMI_PIC_END_ID_THREAD - 1,
	EMI_PIC_NB_ID_THREAD = EMI_PIC_END_ID_THREAD - EMI_PIC_FIRST_ID_THREAD
}
enum_emi_pic_id_thread;

typedef struct
{
	short int siInstId;
	struct_emi_pic_message  *pMsg;
}struct_emi_pic_envoi_service;

/**----- List of internal messages between module threads ------------------**/
typedef enum
{
	M_PREMIER_TYPE_MESSAGE = DERNIER_EMI_PIC_TYPE,
	M_RESTIT_DATE_REQUEST,
	M_RESTIT_DATE_EFFECTUE,
	M_RESTIT_DATE_NON_EFFECTUE,
	M_RESTIT_FILE_REQUEST,
	M_RESTIT_FILE_EFFECTUE,
	M_RESTIT_FILE_NON_EFFECTUE,

	M_TRANSFER_SEND,
	M_TRANSFER_SEND_EFFECTUE,
	M_TRANSFER_SEND_NON_EFFECTUE,
	M_TRANSFER_DEL,
	M_TRANSFER_DEL_EFFECTUE,
	M_TRANSFER_DEL_NON_EFFECTUE,
	
	M_EMISSION_PURGE,
	M_EMISSION_PURGE_EFFECTUE,

	M_ARRET_DEMANDE,
	M_ARRET_EFFECTUE,
	// MFR start 23/7/2008
	M_RESTIT_DATETIME_REQUEST,
	M_RESTIT_DATETIME_EFFECTUE,
	M_RESTIT_DATETIME_NON_EFFECTUE,
	// MFR start 2010/1/11
	M_RESTIT_PURGE_BACKUP,
	// MFR end

	M_EMI_PIC_MAINT_REQUEST,
	M_EMI_PIC_MAINT_ACCEPTED,
	M_EMI_PIC_MAINT_EFFECTUE,
	M_EMI_PIC_MAINT_NON_EFFECTUE,
	M_EMI_PIC_MAINT_UPDATE,

	M_RESTIT_RECORD_ID_REQUEST,
	M_RESTIT_RECORD_ID_EFFECTUE,
	M_RESTIT_RECORD_ID_NON_EFFECTUE,

	M_STATUS_CHANGED,	
	// MFR end   23/7/2008
}
enum_emi_pic_internal_type;

typedef enum
{
	FIRST_CHRONO			= 0,

	CHRONO_EMISSION			= FIRST_CHRONO,
	CHRONO_IMPORT,
	CHRONO_REEMISSION,

	END_CHRONO,

	LAST_CHRONO	= END_CHRONO - 1,
	NB_CHRONOS	= END_CHRONO - FIRST_CHRONO, 
}enum_chrono;

typedef enum
{
	FIRST_DATA_TYPE		= 0,	

		DATA_TYPE_FILE		= FIRST_DATA_TYPE,
		DATA_TYPE_DIR,

		END_DATA_TYPE,
		LAST_DATA_TYPE	= END_DATA_TYPE - 1,
		NB_DATA_TYPES	= END_DATA_TYPE - FIRST_DATA_TYPE,

}enum_data_type;

typedef struct
{
	struct_emi_pic_restit sRestit;
}
struct_emi_pic_ani_restit;

typedef struct
{
	struct_emi_pic_trf sTrf;
}
struct_emi_pic_ani_trf;

typedef struct
{
	struct_emi_pic_emi sEmi;
}
struct_emi_pic_ani_emi;

typedef struct
{
	union
	{
		CHAR szPath[MAX_PATH];
		LONG NbDirectories;
		LONG NbFiles;
	}u;
}struct_emi_pic_maint_mode, *LP_EMI_PIC_MAINT_MODE;

typedef struct
{
	struct_srv_entete entete;

	union
	{
		struct_emi_pic_ani_restit	sAniRestit;
		struct_emi_pic_ani_trf		sAniTrf;
		struct_emi_pic_ani_emi		sAniEmi;
		struct_emi_pic_maint_mode	sAniMaintMode;
	}u;
}
struct_emi_pic_int_message;

typedef struct
{
	noyau_chrono_id
		chrono_id;

	CHAR
		szChronoName[MAX_PATH],
		szRegKey[MAX_PATH];

	noyau_delai
		chrono_dalay;
}struct_chrono_data;

typedef struct
{
	CHAR
		szFileName[MAX_PATH];
	LPVOID
		lpNextFile;
	enum_data_type
		enDataType;
}struct_file_found, *LPFILEFOUND;
/**----- Public structure for each instance --------------------------------**/
typedef struct
{
	noyau_priorite_tache
		dwPrioMax;

	noyau_bal_id
		dwAniBalId,
		dwEmiBalId,
		dwRestitBalId,
		dwTrfBalId,
		dwImportBalId,
		dwArretBalId;

	CHAR
		szAniBalName[MAX_PATH+1],
		szEmiBalName[MAX_PATH+1],
		szRestitBalName[MAX_PATH+1],
		szTrfBalName[MAX_PATH+1],
		szImportBalName[MAX_PATH+1],
		szKey[MAX_PATH+1],
		szImportDir[MAX_PATH+1],
		szNameFormat[MAX_PATH+1];

	noyau_pool_id
		hPool;

	noyau_delai	
		dwChronoEmiPeriod,
		dwChronoEmiPeriodBackup,
		dwChronoImportPeriod;

	BOOL
		bDispatchByYear,
		bDispatchByMonth,
		bDispatchByDay,
		bDispatchByLaneNum,
		bYearIsSubDir,
		bMonthIsSubDir,
		bDayIsSubDir,
		bRestitStopped,
		bTrfStopped,
		bEmiStopped,
		bMainModeON,
		bUsingCounter,
	// MFR start 2010/1/13
		bAbort,
		bPurgeInProgress;
	// MFR end
	
	DWORD
		dwLaneNumBegin,
		dwLaneNumLength,
		dwPurgePeriod,
		dwPurgeHour,
		dwPurgeMinute,
	// MFR start 2010/1/25
		dwPurgeFileDelay;	
	// MFR end

	SYSTEMTIME
		sLastPurgeDTime,
		sResendFrom,
		sResendTo;

	dbg_struct_debug
		sDbg;

	dbg_struct_trace
		sTabTraces[EMI_PIC_NB_TRACES];

	struct_tache
		sTaches[EMI_PIC_NB_ID_THREAD+1];	
	
	HANDLE
		hService[NB_EMI_PIC_SERVICE+1];

	struct_config_data
		ConfigDest[MAX_DESTINATIONS],
		ConfigLocal,
		ConfigBackup,
		BackupCopy[MAX_DESTINATIONS];

	INT
		iNbDefinedDest;	// Number of destinationes read from registry

	struct_format_data
		sFileNameFormat;

	struct_chrono_data
		sChronoData[NB_CHRONOS];

	struct_emi_pic_etat
		sStatus,
		sOldStatus;

	LONG
		NbSubdirs,
	// MFR start 2010/1/13
		NbFilesMaint;

	CRITICAL_SECTION
		CS_Abort;
	// MFR end
}
struct_emi_pic_public;
/*-------------------------------- FUNCTIONS: -------------------------------*/
// Module threads
PROTECTED DWORD WINAPI EmiPicAni(PVOID pvParam);
PROTECTED DWORD WINAPI EmiPicEmi(PVOID pvParam);
PROTECTED DWORD WINAPI EmiPicRestit(PVOID pvParam);
PROTECTED DWORD WINAPI EmiPicTrf(PVOID pvParam);
PROTECTED DWORD WINAPI EmiPicImport(PVOID pvParam);

PROTECTED enum_instance_result EmiPicInitTrace(char *pcBal,
                                         short siInstId);
PROTECTED char * escapeCStringForSPrintf(char *szInput ,char * szOutput, DWORD dwSize);

PROTECTED void EmiPicFichierTrace(short siInstId, char *szFmt,...);
PROTECTED void DEFINE_EmiPicFichierDebug(short siInstId,
                                         char *szFmt,...);

PROTECTED BOOL LaunchTimer( short siInstId, enum_chrono enChrono, noyau_bal_id balDest, int iDelay );

PROTECTED BOOL GetFilesFromDir( short siInstId, LPSTR szPath, LPFILEFOUND* lpFiles, CONST LPSTR szSearchMask );
PROTECTED BOOL TransferFileToDestinations( short siInstId, LPSTR szFileSource, size_t uiFileSourceSize);
PROTECTED int Eval_Exception( short instance, int n_except, LPSTR szFile, int iLine );
// MFR start 2010/1/11
PROTECTED void PurgeOldFiles( LPSTR szDir, LONG lPurgePeriod, LONG lPurgeFileDelay );
PROTECTED BOOL IsOkToRemoveDir( short siInstId, LPSTR szDir, DWORD dwMaxDaysOld );

PROTECTED VOID SetAbort( IN SHORT instance, IN BOOL Value );
PROTECTED BOOL GetAbort( IN SHORT instance );
// MFR end

#define START_TRY	__try{
#define	END_TRY		}__except( Eval_Exception( siInstId, GetExceptionCode(), __FILE__, __LINE__ ) ){ ExitBad(); }

/*-------------------------------- VARIABLES: -------------------------------*/

/**----- Begin of shared data section --------------------------------------**/
#include "global.h"
PROTECTED struct_emi_pic_public EMI_PIC[EMI_PIC_INSTANCE_MAX] INIT(0);
PROTECTED WORD                  EMI_PIC_NB_INSTANCES          INIT(0);
#include "global.h"
/**----- End of shared data section ----------------------------------------**/

/**----- DEBUG mode definitions --------------------------------------------**/
PROTECTED char *EMI_PIC_FILE;
PROTECTED int  EMI_PIC_LINE;

#include <undef.h>
#endif
/*-------------------------------- END OF FILE ------------------------------*/