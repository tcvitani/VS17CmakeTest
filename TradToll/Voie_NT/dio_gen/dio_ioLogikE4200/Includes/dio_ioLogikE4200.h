/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_ioLogikE4200												 */
/* FILE:	 dio_ioLogikE4200.h												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_IOLOGIK_E4200_H
#define DIO_IOLOGIK_E4200_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef DIO_ioLogikE4200_EXPORTS
 	#include <public.h>
#else
 	#include <export.h>
#endif

#include <dio_interface.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define DIO_IOLOGIK_E4200_REG_KEYn	"DIO_IOLOGIK_E4200\\"

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
	PDIO_EXEC_INPUT_OUTPUT_CONFIG_ERR,				// Input or output registry error. Number of slot inputs and outputs are set to <= 0.

	END_PDIO_EXEC_MSG,
	LAST_PDIO_EXEC_MSG		= END_PDIO_EXEC_MSG - 1,
	NB_PDIO_EXEC_MSGS		= END_PDIO_EXEC_MSG - FIRST_PDIO_EXEC_MSG,		
}
enum_pdio_exec_msg;

typedef struct
{
	DWORD	dwSlotNbInputs;
	DWORD	dwSlotNbOutputs;
	
	DWORD	dwSlot1stInputIdx;
	DWORD	dwSlot1stOutputIdx;

	INT		iSlotPosition;
}
struct_slot_settings;

// Not used in this version, left if needed in future
// typedef struct
// {
//  	DWORD dwFilter;	
// }
// struct_input_settings;

typedef struct
{	
	DWORD	dwNbInputs;
	DWORD	dwNbOutputs;
	
	DWORD	dw1stInputIdx;
	DWORD	dw1stOutputIdx;
	
	BYTE	bIntputStatus[DIO_NB_INPUTS_MAX];
	BYTE	bOutputStatus[DIO_NB_OUTPUTS_MAX];

	INT		iHandle;				// connection handle
	
	BOOL	bLinkError;				// connection status
	
	DWORD	dwMaxSocketErrors;		// max socket errors
	DWORD	dwResetSafeStatus;		// reset safe status

	WORD	wTcpPortNum;			// TCP port number
	CHAR	szIpAddress[MAX_PATH];	// IP address
	
	DWORD	dwConnTimeOut;			// connection time out
	
	CHAR	szPassword[MAX_PATH];	// login password

// 	struct_input_settings	sInputSettings[DIO_NB_INPUTS_MAX];

	struct_slot_settings	sSlotSettings[DIO_NB_BOARDS_MAX];

	DWORD	dwNbSlots;				// total number of slots on board
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

	HANDLE					hExecThread;
	HANDLE					hLinkThread;
	HANDLE					hPollThread;

	DWORD					dwExecThreadId;		// id of thread used to execute callback function
	DWORD					dwLinkThreadId;		// id of thread used to execute callback function
	DWORD					dwPollThreadId;		// id of thread used to execute callback function

	HINSTANCE				hPlugin;			// used to identify plug-in on execution of callback function
	PDioStatusOccuredEvent	pCallbackFnc;		// callback function		
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
PROTECTED DWORD WINAPI DIO_LinkThread(LPVOID lpParameter);
PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter);

PROTECTED DWORD GetPollingDelayMs(void);
PROTECTED DWORD GetLinkCheckDelayMs(void);

PROTECTED DWORD GetNumberOfIOBoards(void);
PROTECTED DIO_ERROR TryToConnect(DWORD dwBoardIdx);

PROTECTED void SetIOBoardConnectionError(DWORD dwBoardIdx);
PROTECTED void ResetIOBoardConnectionError(DWORD dwBoardIdx);
PROTECTED BOOL IOBoardConnectionOK(DWORD dwBoardIdx);

PROTECTED INT GetIOBoardConnectionHandle(DWORD dwBoardIdx);
PROTECTED void SetIOBoardConnectionHandle(DWORD dwBoardIdx, INT iHandle);

PROTECTED void GetIOBoardIPAddress(DWORD dwBoardIdx, CHAR *pIpAddress, DWORD dwMaxSize);
PROTECTED void GetIOBoardPassword(DWORD dwBoardIdx, CHAR *pPassword, DWORD dwMaxSize);
PROTECTED WORD GetIOBoardTcpPortNumber(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardConnectionTimeout(DWORD dwBoardIdx);

PROTECTED DWORD GetIOBoardNbInputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx);

PROTECTED DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx);

PROTECTED DWORD GetDI_Filter(DWORD dwBoardIdx, DWORD dwInput);

PROTECTED void SetInputStatus(DWORD dwBoardIdx, DWORD dwInput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetInputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED void SetOutputStatus(DWORD dwBoardIdx, DWORD dwOutput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetOutputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED BOOL IsCallbackFncInUse(void);
PROTECTED DWORD GetExecThreadId(void);
PROTECTED void ExecuteCallbackFnc(enum_pdio_exec_msg eMsg, WPARAM wParam, LPARAM lParam);

PROTECTED DWORD GetMaxSocketErrors(DWORD dwBoardIdx);

PROTECTED HINSTANCE GetPluginHandle(void);

PROTECTED BOOL IsInitDone(void);
PROTECTED void SetInitDone(void);

PROTECTED INT GetIOBoardNbSlots(DWORD dwBoardIdx);
PROTECTED struct_slot_settings* GetSlotSettingsPtr(DWORD dwBoardIdx, DWORD dwSlotIdx);
/*-------------------------------- VARIABLES: -------------------------------*/

PROTECTED CRITICAL_SECTION DioCriticalSection;

#endif

/*-------------------------------- END OF FILE ------------------------------*/