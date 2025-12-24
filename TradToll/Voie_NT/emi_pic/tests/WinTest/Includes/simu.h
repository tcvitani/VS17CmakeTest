/********** (v) 2000 CSEE-Peage -   All rights reserved ************/
/*                                                                 */
/* ----------------------------------------------------------------*/
/* MODULE:   Printer Module Test Application                       */
/* FILE:     simu.h                                                */
/* LANGUAGE: C                                                     */
/* ----------------------------------------------------------------*/
/* DESCRIPTION:                                                    */
/* ----------------------------------------------------------------*/
/* HISTORY:                                                        */
/*******************************************************************/

#ifndef SIMU_H
#define SIMU_H
/*--------------------------- INCLUDES:  --------------------------*/
#include <windows.h>

// CS Route interface
#include <noyau.h>
// Module interface
//#include <csr_pyl.h>
/*--------------------------- RESERVED:  --------------------------*/
#include <protect.h>
/*--------------------------- EXTERNALS: --------------------------*/
/*--------------------------- DEFINES:   --------------------------*/
#define TAB_NUM    4
//#define MAINT_BAL_NAME	"BL_MAINT_PYL"
//#define MODULE_BAL_NAME	"BL_PYL"
//
//#define	FNC_MOD_LANCE	"_MODLance@12"
//#define	FNC_MOD_ARRET	"_MODArret@4"

#define DEFAULT_WND_WIDTH 650
#define DEFAULT_WND_SIZE 350

// Translation strings
#define TXT_MAIN_TOOL_NAME				"PYL_MAIN_TOOL_NAME"

#define TXT_HOPP_DEVICE_NB				"PYL_HOPP_DEVICE_NB"
#define TXT_HOPP_DEVICE_ERR				"PYL_HOPP_DEVICE_ERR"
#define TXT_HOPP_LINK_ERR				"PYL_HOPP_LINK_ERR"
#define TXT_HOPP_ALMOST_EMPTY			"PYL_HOPP_ALMOST_EMPTY"
#define TXT_HOPP_EMPTY					"PYL_HOPP_EMPTY"
#define TXT_HOPP_JAMMED					"PYL_HOPP_JAMMED"
#define TXT_HOPP_FRAUD					"PYL_HOPP_FRAUD"
#define TXT_HOPP_MISSING				"PYL_HOPP_MISSING"

#define TXT_COIN_ACC_NB					"PYL_COIN_ACC_NB"
#define TXT_COIN_ACC_DEVICE_ERR			"PYL_COIN_ACC_DEVICE_ERR"
#define TXT_COIN_ACC_LINK_ERR			"PYL_COIN_ACC_LINK_ERR"
#define TXT_COIN_ACC_JAMMED				"PYL_COIN_ACC_JAMMED"
#define TXT_COIN_ACC_BLOCKED			"PYL_COIN_ACC_BLOCKED"

#define TXT_BILL_ACC_NB					"PYL_BILL_ACC_NB"
#define TXT_BILL_ACC_DEVICE_ERR			"PYL_BILL_ACC_DEVICE_ERR"
#define TXT_BILL_ACC_LINK_ERR			"PYL_BILL_ACC_LINK_ERR"
#define TXT_BILL_ACC_ALMOST_FULL		"PYL_BILL_ACC_ALMOST_FULL"
#define TXT_BILL_ACC_FULL				"PYL_BILL_ACC_FULL"
#define TXT_BILL_ACC_JAMMED				"PYL_BILL_ACC_JAMMED"
#define TXT_BILL_ACC_MISSING			"PYL_BILL_ACC_MISSING"

#define TXT_COIN_ACC1					"PYL_COIN_ACC1"
#define TXT_COIN_ACC2					"PYL_COIN_ACC2"

#define TXT_COIN_ACC_VAUL_NB			"PYL_COIN_ACC_VAUL_NB"
#define TXT_COIN_ACC_VAUL_ALMOST_FULL	"PYL_COIN_ACC_VAUL_ALMOST_FULL"
#define TXT_COIN_ACC_VAUL_FULL			"PYL_COIN_ACC_VAUL_FULL"
#define TXT_COIN_ACC_VAUL_MISSING		"PYL_COIN_ACC_VAUL_MISSING"

#define TXT_REQUEST_STOCK_STATUS		"PYL_REQUEST_STOCK_STATUS"

#define TXT_REQUEST_PAY_IN				"PYL_REQUEST_PAY_IN"
#define TXT_REQUEST_PAY_OUT				"PYL_REQUEST_PAY_OUT"

#define TXT_STOP_PAY_IN					"PYL_STOP_PAY_IN"

#define TXT_ERR_ERR						"PYL_ERR_ERR"
#define TXT_ERROR_START_MODULE			"PYL_ERROR_START_MODULE"

#define TXT_STOCK_STATUS				"PYL_STOCK_STATUS"
#define TXT_STATUS_OK					"PYL_STATUS_OK"

// Stock status text
#define TXT_COIN_ACCEPTOR_NB			"PYL_COIN_ACCEPTOR_NB"
#define TXT_VAULT						"PYL_VAULT"
#define TXT_TOTAL_COINS					"PYL_TOTAL_COINS"
#define TXT_COIN_TYPE					"PYL_COIN_TYPE"
#define TXT_COIN_NUMBER					"PYL_COIN_NUMBER"
#define TXT_BILL_ACCEPTOR_NB			"PYL_BILL_ACCEPTOR_NB"
#define TXT_BILL_TOTAL					"PYL_BILL_TOTAL"
#define TXT_BILL_TYPE					"PYL_BILL_TYPE"
#define TXT_BILL_NUMBER					"PYL_BILL_NUMBER"
#define TXT_HOPPERS						"PYL_HOPPERS"
#define TXT_HOPPER						"PYL_HOPPER"
#define TXT_VALUE						"PYL_VALUE"
/*--------------------------- TYPEDEFS:  --------------------------*/
/*typedef struct
{
	noyau_bal_id bal_id;
	char bal_name[6];
}simu_multi_user;*/
// Pointer to the module functions
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

	LAST_ICON	= END_ICON - 1,
	NB_ICONS		= END_ICON - FIRST_ICON, 
}icon_type;

typedef struct
{
	HWND		hWnd;
	icon_type	enIcon;
}struct_data;
/*--------------------------- FUNCTIONS: --------------------------*/
PROTECTED int LanceModule(HWND hDlg, char *pcMbName);
PROTECTED int ArretModule(HWND hDlg);
PROTECTED void SimuEnvoi(short int service,
						    short int type_message);
PROTECTED BOOL OpenMailBox( VOID );
PROTECTED VOID ManageStatusReport( VOID );
PROTECTED VOID ManageStockReport( VOID );
PROTECTED void ExitApplication(HWND hDlg);
HWND hwndInitWindow(HINSTANCE hInstance,int iCmdShow);
VOID ManageGraphWindow( IN int iControl, IN BOOL bChecked );
VOID InitGraphWindows( VOID );
/*--------------------------- VARIABLES: --------------------------*/
#include <undef.h>
#endif
/*--------------------------- END FILE ----------------------------*/
