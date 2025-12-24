/******************* (v) 2006 CSSI - All rights reserved *********************/
/*				                                                             */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     global.h											 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
#ifndef GLOBAL_H
#define GLOBAL_H
/*--------------------------- INCLUDES:  ------------------------------------*/
#include <windows.h>

#include <run.h>

#include <resource.h>

#include <csrlc32.h>
#include <noyau.h>
#include <debug.h>
#include <run.h>
#include <module.h>
#include <reg.h>
#include <run.h>
#include <csr_srv.h>
#include <csr_emi_pic.h>

#include <simu.h>
/*--------------------------- RESERVED:  ------------------------------------*/

/*--------------------------- EXTERNALS: ------------------------------------*/

/*--------------------------- DEFINES:   ------------------------------------*/
#define DLL_FILE	"CSR_EMI_PIC10_VS12_X64.dll"

#define FNC_LANCE	"_MODLance@12"
#define FNC_ARRET	"_MODArret@4"

#define	FNC_MOD_LANCE	"MODLance"
#define	FNC_MOD_ARRET	"MODArret"

#define MAINT_BAL_NAME	"BL_LOADER_EMI_PIC"
#define MODULE_MAILBOX	"BL_EMI_PIC"
/*--------------------------- TYPEDEFS:  ------------------------------------*/
typedef enum
{
	FIRST_TIME_ELEMENT			= 0,

	YEAR			= FIRST_TIME_ELEMENT,
	MONTH,
	DAY,
	HOUR,
	MINUTE,
	SECOND,
	MILLISECOND,

	END_TIME_ELEMENT,

	LAST_TIME_ELEMENT	= END_TIME_ELEMENT - 1,
	NB_TIME_ELEMENTS	= END_TIME_ELEMENT - FIRST_TIME_ELEMENT, 
}enum_;

typedef enum_instance_result (WINAPI* LPFNC_LANCE)(IN char * pcKey, 
                                                   IN char * pcBalName, 
                                                   OUT noyau_bal_id * piBalId );
typedef enum_instance_result (WINAPI* LPFNC_ARRET)(IN noyau_bal_id iBalId );

typedef struct
{
	HANDLE
		hInst;

	noyau_bal_id
		MAINT_bal_id,
		module_bal_id;

	HWND
		hDlg;
	
	LPFNC_LANCE
		LpFncMODLance;    // Function pointer
	LPFNC_ARRET
		LpFncMODArret;    // Function pointer

	BOOL
		bRun;

	CHAR
		MailBoxName[MAX_PATH];

	struct_emi_pic_message
		message;

	HWND
		from_to_wnd[2][NB_TIME_ELEMENTS];

	SYSTEMTIME
		sFromTime,
		sToTime;
	LONG
		lRecordID;
}struct_simu;
/*--------------------------- FUNCTIONS: ------------------------------------*/

/*--------------------------- VARIABLES: ------------------------------------*/
struct_simu SIMU;
/*--------------------------- END OF FILE -----------------------------------*/
#endif // GLOBAL_H