/***************** (v) 2012 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_UNIDAQ														 */
/* FILE:	 dio_unidaq.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_UNIDAQ_LOC_H
#define DIO_UNIDAQ_LOC_H

/*--------------- INCLUDES: ---------------*/

#include <protect.h>

/*--------------- RESERVED: ---------------*/


/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/

#define MAX_IO_PER_CARD	64

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					MOD_REG_KEYn_MODULES

#define DBG_NAME				"dio_unidaq"

#define REGKEY_INDEX			"CardIndex"
#define REGKEY_INVERT_INPUTS	"InvertInputs"
#define REGKEY_POLLING_DELAY	"PollingDelayMs"


/*--------------- TYPEDEFS: ---------------*/

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



typedef struct _struct_board_details
{
	DWORD	dwCardIndex;
	
	DWORD	dwNbInputs;
	DWORD	dwNbOutputs;
	
	DWORD	dw1stInputIdx;
	DWORD	dw1stOutputIdx;	

	BYTE	abInputStatus[DIO_NB_INPUTS_MAX];
	BYTE	abCurrentOutputStatus[DIO_NB_OUTPUTS_MAX];

	BOOL	bInvertInputs;

}
struct_board_details;

typedef struct _struct_err
{
	LONG	ErrCode;
	LPSTR	ErrDesc;
}
struct_err, *LP_ERR;

typedef struct _struct_dio
{
    DWORD					dwNbBoards;
	HANDLE					hRegion; // mutex inter-process

    struct_board_details	sBoardDetails[DIO_NB_BOARDS_MAX];
    
    DWORD					dwNbInputs;    // total number of inputs
    DWORD					dwNbOutputs;   // total number of outputs

	BOOL					bInitDone;
	BOOL					bShouldExit;


	DWORD					dwPollingDelayMs;	

	DWORD					dwExecThreadId;		// id of thread used to execute callback function
	HINSTANCE				hPlugin;			// used to identify plug-in on execution of callback function
	PDioStatusOccuredEvent	pCallbackFnc;		// callback function

	HANDLE					hExec;
	HANDLE					hPoll;

}
struct_dio;

typedef struct _struct_debug
{
    // infos de debug
    void *debug;
    void *traces;
} 
struct_debug;


/*--------------- FUNCTIONS: ---------------*/

PROTECTED BOOL DIO_InitTraces (struct _struct_debug *dbg);
PROTECTED void DIO_DeinitTraces(struct _struct_debug *dbg);

PROTECTED void DIO_Error (IN struct _struct_debug *dbg, IN char *fmt, ...);
PROTECTED void DIO_Trace (IN struct _struct_debug *dbg, IN char *fmt, ...);

PROTECTED BOOL IsInitDone(void);
PROTECTED void SetInitDone(void);

PROTECTED BOOL IsShouldExit(void);
PROTECTED void SetShouldExit(void);


PROTECTED DWORD WINAPI DIO_PollThread(LPVOID lpParameter);
PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter);

PROTECTED DWORD GetPollingDelayMs(void);

PROTECTED DWORD GetNumberOfIOBoards(void);
PROTECTED DWORD GetIOBoardNbInputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx);

PROTECTED DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx);
PROTECTED DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx);

PROTECTED void SetInputStatus(DWORD dwBoardIdx, DWORD dwInput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetInputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED void SetCurrentOutputStatus(DWORD dwBoardIdx, DWORD dwOutput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetCurrentOutputStatus(DWORD dwBoardIdx, DWORD dwOutput);

PROTECTED BOOL IsCallbackFncInUse(void);
PROTECTED DWORD GetExecThreadId(void);
PROTECTED HINSTANCE GetPluginHandle(void);

PROTECTED DIO_STATE ReadInput(DWORD dwBoardIdx, DWORD dwInput, OUT DIO_ERROR *pDioErr);
PROTECTED DIO_STATE TestOutput(DWORD dwBoardIdx, DWORD dwOutput, OUT DIO_ERROR *pDioErr);

//-------------------------------------------------
PROTECTED BOOL InitGlobalRegion(void);
PROTECTED void EnterGlobalRegion(void);
PROTECTED void LeaveGlobalRegion(void);
PROTECTED void SetGlobalOutputStatus(DWORD dwBoardIdx, DWORD dwOutput, DIO_STATE eDioState);
PROTECTED DIO_STATE GetGlobalOutputStatus(DWORD dwBoardIdx, DWORD dwOutput);

/*--------------- VARIABLES: ---------------*/

PROTECTED CRITICAL_SECTION DioCriticalSection INIT(0);
PROTECTED struct_dio DIO INIT(0);
PROTECTED struct_debug DBG INIT(0);

#include <undef.h>
#endif
