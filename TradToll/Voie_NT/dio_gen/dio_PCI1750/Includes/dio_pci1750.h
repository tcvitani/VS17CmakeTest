/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 dio_pcl1750													 */
/* FILE:	 dio_pci1750.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_PCI1750_H
#define DIO_PCI1750_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef PCI_DEF
 	#include <public.h>
#else
 	#include <export.h>
#endif

#include <dio_interface.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define DIO_PCI1750_REG_KEYn	"DIO_PCI1750\\"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

// Callback thread instructions
typedef enum
{
	FIRST_PDIO_EXEC_MSG		= WM_USER,	
		
	PDIO_EXEC_SEND_FULL_SET	= FIRST_PDIO_EXEC_MSG,	// Send current state of all inputs and outputs, wParam DIO board index, lParam unused
	PDIO_EXEC_INPUT_STATE,							// Input state changed, wParam input number, lParam input state
	PDIO_EXEC_OUTPUT_STATE,							// Output state changed, wParam output number, lParam output state
	PDIO_EXEC_DEVICE_STATUS,						// Device status changed, wParam DIO board index, lParam device status
	PDIO_EXEC_PLUGIN_CONFIG,						// Send plug-in config, wParam input number, lParam input state

	END_PDIO_EXEC_MSG,
	LAST_PDIO_EXEC_MSG		= END_PDIO_EXEC_MSG - 1,
	NB_PDIO_EXEC_MSGS		= END_PDIO_EXEC_MSG - FIRST_PDIO_EXEC_MSG,		
}
enum_pdio_exec_msg;


typedef struct
{
	DWORD dwFilter;	
}
struct_input_settings;

typedef struct
{	
	DWORD	dwNbInputs;
	DWORD	dwNbOutputs;
	
	DWORD	dw1stInputIdx;
	DWORD	dw1stOutputIdx;
	
	BYTE	bIntputStatus[DIO_NB_INPUTS_MAX];
	BYTE	bOutputStatus[DIO_NB_OUTPUTS_MAX];

	BOOL	bInvertInputs;

	INT		iHandle;				// connection handle

	BOOL	bLinkError;				// connection status - there is no real link error because this card is in PCI slot, serves just to reset link error on main app
	
	char	szBoardDescription[64];	// PCI1750 board description
	DWORD	dwDeviceNumber;			// PCI1750 device number

	struct InstantDiCtrl * instantDiCtrl;
	struct InstantDoCtrl * instantDoCtrl;
}
struct_board_details;



typedef struct 
{
    DWORD					dwNbBoards;
	
    struct_board_details	sBoardDetails[DIO_NB_BOARDS_MAX];
    
    DWORD					dwNbInputs;			// total number of inputs
    DWORD					dwNbOutputs;		// total number of outputs

	BOOL					bInitDone;

	DWORD					dwPollingDelayMs;	// max socket errors
	DWORD					dwLinkCheckDelayMs;	// max socket errors

	DWORD					dwExecThreadId;		// id of thread used to execute callback function
	HINSTANCE				hPlugin;			// used to identify plug-in on execution of callback function
	PDioStatusOccuredEvent	pCallbackFnc;		// callback function

	HANDLE					hExec;
	HANDLE					hPoll;


}
struct_dio;

typedef struct 
{
    // infos de debug
    void *debug;
    void *traces;
} 
struct_debug;

typedef struct
{
	LONG	ErrCode;
	LPSTR	ErrDesc;
}
struct_err, *LP_ERR;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD WINAPI DIO_PollThread(LPVOID lpParameter);
PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter);

PROTECTED DWORD GetPollingDelayMs(void);

PROTECTED DWORD GetNumberOfIOBoards(void);

PROTECTED void SetIOBoardConnectionError(DWORD dwBoardIdx);
PROTECTED void ResetIOBoardConnectionError(DWORD dwBoardIdx);
PROTECTED BOOL IOBoardConnectionOK(DWORD dwBoardIdx);

PROTECTED DWORD GetIOBoardNbInputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx);

PROTECTED DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx);

PROTECTED void SetInputStatus(DWORD dwBoardIdx, DWORD dwInput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetInputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED void SetOutputStatus(DWORD dwBoardIdx, DWORD dwOutput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetOutputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED BOOL IsCallbackFncInUse(void);
PROTECTED DWORD GetExecThreadId(void);
PROTECTED void ExecuteCallbackFnc(enum_pdio_exec_msg eMsg, WPARAM wParam, LPARAM lParam);

PROTECTED HINSTANCE GetPluginHandle(void);

PROTECTED BOOL IsInitDone(void);
PROTECTED void SetInitDone(void);

PROTECTED DIO_STATE ReadInput(DWORD dwBoardIdx, DWORD dwInput, OUT DIO_ERROR *pDioErr);
PROTECTED DIO_STATE TestOutput(DWORD dwBoardIdx, DWORD dwOutput, OUT DIO_ERROR *pDioErr);

PROTECTED BOOL ShouldInputStatusBeInverted(DWORD dwBoardIdx);

PROTECTED void DisposeIOBoardCtrl_DI_Instance(DWORD dwBoardIdx);
PROTECTED struct InstantDiCtrl * GetInstance_DI_Ctrl(DWORD dwBoardIdx);

PROTECTED void DisposeIOBoardCtrl_DO_Instance(DWORD dwBoardIdx);
PROTECTED struct InstantDoCtrl * GetInstance_DO_Ctrl(DWORD dwBoardIdx);

/*-------------------------------- VARIABLES: -------------------------------*/

PROTECTED CRITICAL_SECTION DioCriticalSection;

#endif

/*-------------------------------- END OF FILE ------------------------------*/