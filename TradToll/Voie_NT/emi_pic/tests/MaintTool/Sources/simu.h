/************ (v) 2009 CSEE-Peage -   All rights reserved ********************/
/*																			 */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     simu.c															 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
#ifndef SIMU_H
#define SIMU_H
/*---------------------------------- INCLUDES:  -----------------------------*/
#include <windows.h>
#include <commctrl.h>

// CS Route interface
#include <noyau.h>
#include <reg.h>
// Module interface
#include <csr_emi_pic.h>
// Resources
#include <resource.h>
#include <language.h>
/*---------------------------------- RESERVED:  -----------------------------*/
#include <protect.h>
/*---------------------------------- EXTERNALS: -----------------------------*/

/*---------------------------------- DEFINES:   -----------------------------*/
#define DEFAULT_WND_WIDTH 650
#define DEFAULT_WND_SIZE 350

#define APP_BAL_NAME	"MAINT_TOOL_EMI_PIC"
#define EMI_PIC_INSTANCE_MAX	5

// Define language labels
#define TXT_MAIN_TOOL_NAME				"EMI_PIC_MAIN_TOOL_NAME"

#define TXT_SOURCE						"EMI_PIC_SOURCE" 
#define TXT_DESTINATION					"EMI_PIC_DESTINATION"
#define TXT_BROWSE						"EMI_PIC_BROWSE"
#define TXT_STATUS						"EMI_PIC_STATUS"
#define TXT_START						"EMI_PIC_START"
#define TXT_CANCEL						"EMI_PIC_CANCEL"

#define TXT_READY						"EMI_PIC_READY"
#define TXT_WORKING						"EMI_PIC_WORKING"
#define TXT_COMPLETED					"EMI_PIC_COMPLETED"
#define TXT_INITIALIZING				"EMI_PIC_INITIALIZING"
#define TXT_SELECT_DESTINATION			"EMI_PIC_SELECT_DESTINATION"
#define TXT_FILES_TRANSFERED			"EMI_PIC_FILES_TRANSFERED"

#define TXT_ERR_ERR						"EMI_PIC_ERR_ERR"
#define TXT_ERROR_START_MODULE			"EMI_PIC_ERROR_START_MODULE"

/*---------------------------------- TYPEDEFS:  -----------------------------*/
typedef enum
{
	FIRST_ICON			= 0,
		
		ICON_NONE			= FIRST_ICON,
		ICON_ERROR,
		ICON_OK,
		ICON_WAITING,
		ICON_UNDEFINED,
		ICON_INACTIVE,
		
		END_ICON,
		
		LAST_ICON		= END_ICON - 1,
		NB_ICONS		= END_ICON - FIRST_ICON, 
}icon_type;

typedef enum
{
	FIRST_APP_STATUS	= 0,	

		APP_STATUS_INITIALIZING	= FIRST_APP_STATUS,
		APP_STATUS_READY,
		APP_STATUS_WORKING,
		APP_STATUS_COMPLETED,

		END_APP_STATUS,
		LAST_APP_STATUS	= END_APP_STATUS - 1,
		NB_APP_STATUS	= END_APP_STATUS - FIRST_APP_STATUS,

}enum_app_status;

typedef struct
{
	HWND		hWnd;
	icon_type	enIcon;
}struct_data;

typedef struct
{
	CHAR
		szMailboxName[MAX_PATH],
		szSourceDir[MAX_PATH],
		szDestinationDir[MAX_PATH],
		szTitleLabel[MAX_PATH];

	noyau_bal_id
		MailboxID;

	BOOL
		bModuleWasRunning,
		SubscribedServices[NB_EMI_PIC_SERVICE+1],
		TargetPathSet,
		Completed;

	HWND
		hDlg;	// Dialog tied to a module

	LONG
		NbFilesTransfered;

}struct_module_instance_data, *LP_MODULE_INSTANCE_DATA;

// Global structure
typedef struct
{
	
	noyau_pool_id        pool;
	noyau_bal_id
		module_bal_id,
		MAINT_bal_id;
	CHAR
		MAINT_bal_name[MAX_PATH];
	HANDLE
		hInst;
	HWND
		hDlg,
		hWnd,
		hWndStatus,
		hWndStatusIcon;
	boolean				 bRun;
	
	PMOD_PROC_LANCE		LpFncMODLance;    // Function pointer
	PMOD_PROC_ARRET		LpFncMODArret;    // Function pointer

	struct_emi_pic_message  message;
	
	BOOL
		bInited,
		fin;
	struct_module_instance_data
		ModuleData[EMI_PIC_INSTANCE_MAX];
	LONG
		NbModuleInstances;
	enum_app_status
		enAppStatus;
}struct_simu;
/*---------------------------------- FUNCTIONS: -----------------------------*/
PROTECTED BOOL LanceModule( VOID );
PROTECTED int ArretModule(HWND hDlg);
PROTECTED void SimuEnvoi( noyau_bal_id	MailboxID,
						  short int service,
						  short int type_message);
PROTECTED BOOL OpenMailBox( VOID );

PROTECTED void ExitApplication(HWND hDlg);
HWND hwndInitWindow(HINSTANCE hInstance,int iCmdShow);
// 
PROTECTED VOID InitBrushes( VOID );
PROTECTED VOID FreeBrushes( VOID );
PROTECTED HBRUSH SetIcon( HWND hwnd, icon_type Icon );
PROTECTED VOID SetStatus( enum_app_status enStatus );
PROTECTED BOOL AllInstancesCompleted( VOID );
PROTECTED VOID ReportMaintComplete( VOID );
/*---------------------------------- VARIABLES: -----------------------------*/

struct_simu SIMU
#ifdef LOC_DEF
= { 0 };
#endif
;
/*---------------------------------- END FILE   -----------------------------*/
#endif //SIMU_H