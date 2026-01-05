/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_MAIN														 */
/* FILE:     csr_dio_main.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_MAIN_H
#define DIO_MAIN_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef PCL_DEF
#   include <public.h>
#else
#   include <export.h>
#endif

#include <dio_interface.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define DIO_MAIN_MAX_IO		DIO_NB_PLUGINS_MAX * DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX

// Callback thread instructions
typedef enum
{
	FIRST_MDIO_EXEC_MSG		= WM_USER,	
		
	MDIO_EXEC_INPUT_STATE	= FIRST_MDIO_EXEC_MSG,	// Input state changed, wParam input number, lParam input state
	MDIO_EXEC_OUTPUT_STATE,							// Output state changed, wParam output number, lParam output state
	MDIO_EXEC_DEVICE_STATUS,						// Device status changed, wParam plug-in index, lParam P_DIO_BOARD_STATUS
	
	END_MDIO_EXEC_MSG,
	LAST_MDIO_EXEC_MSG		= END_MDIO_EXEC_MSG - 1,
	NB_MDIO_EXEC_MSGS		= END_MDIO_EXEC_MSG - FIRST_MDIO_EXEC_MSG,		
}
enum_mdio_exec_msg;

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter);
PROTECTED DWORD WINAPI DIO_PollThread(LPVOID lpParameter);

PROTECTED BOOL DIOWorksAsSimulator(void);

PROTECTED DWORD GetNumberOfPlugins(void);

PROTECTED DWORD GetIONbInputs(void);
PROTECTED DWORD GetIOFirstInputIdx(void);
PROTECTED DWORD GetIOLastInputIdx(void);

PROTECTED DWORD GetIONbOutputs(void);
PROTECTED DWORD GetIOFirstOutputIdx(void);
PROTECTED DWORD GetIOLastOutputIdx(void);

PROTECTED DWORD GetIOPluginNbInputs(DWORD dwPluginIdx);
PROTECTED DWORD GetIOPluginFirstInputIdx(DWORD dwPluginIdx);
PROTECTED DWORD GetIOPluginLastInputIdx(DWORD dwPluginIdx);

PROTECTED DWORD GetIOPluginNbOutputs(DWORD dwPluginIdx);
PROTECTED DWORD GetIOPluginFirstOutputIdx(DWORD dwPluginIdx);
PROTECTED DWORD GetIOPluginLastOutputIdx(DWORD dwPluginIdx);

PROTECTED void MDioGetSimuInputStatusAll(DIO_STATE *pDioStates, DWORD dwMaxSize);
PROTECTED void MDioGetSimuOutputStatusAll(DIO_STATE *pDioStates, DWORD dwMaxSize);

PROTECTED DWORD GetExecThreadId(void);

PROTECTED BOOL IsCallbackFncInUse(void);
PROTECTED void ExecuteCallbackFnc(enum_mdio_exec_msg eMsg, WPARAM wParam, LPARAM lParam);

PROTECTED HINSTANCE GetPluginHandle(DWORD dwPluginIdx);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
