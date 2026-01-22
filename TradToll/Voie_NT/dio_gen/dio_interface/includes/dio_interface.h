/***************** (v) 2012 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 																 */
/* FILE:     dio_inteface.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_INTERFACE_H
#define DIO_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------- INCLUDES:  --------------------------*/

#include <public.h>

/*--------------------------- RESERVED:  --------------------------*/


/*--------------------------- EXTERNALS: --------------------------*/


/*--------------------------- DEFINES:   --------------------------*/

#define DIO_REG_KEYn_IO_ROOT		"DIO\\"
#define DIO_REG_KEYn_PLUGINS		"PLUGINS\\"

#define DIO_REGKEY_USE_SIMULATOR	"UseSimulator"
#define DIO_REGKEY_DLL_NAME			"dll_Name"
#define DIO_REGKEY_DISABLED			"Disabled"
#define DIO_REGKEY_DIO_BOARD		"DioBoard"
#define DIO_REGKEY_NB_OF_INPUTS		"NbOfInputs"
#define DIO_REGKEY_NB_OF_OUTPUTS	"NbOfOutputs"
#define DIO_REGKEY_FIRST_INPUT_IDX	"FirstInputIndex"
#define DIO_REGKEY_FIRST_OUTPUT_IDX	"FirstOutputIndex"

#define DIO_NB_PLUGINS_MAX			32	// max number of plug-ins
#define DIO_NB_BOARDS_MAX			32	// max number of DIO boards
#define DIO_NB_INPUTS_MAX			32	// max number of inputs per DIO board
#define DIO_NB_OUTPUTS_MAX			32	// max number of outputs per DIO board

/*--------------------------- TYPEDEFS:  --------------------------*/

// INPUT/OUTPUT STATE
typedef enum
{
	DIO_STATE_LOW	= 0,		
	DIO_STATE_HIGH	= 1
}
DIO_STATE;

// FEEDBACK
typedef enum
{
    DIO_SUCCESS = 0,
	DIO_REG_KEY_MISSING,
	DIO_INVALID_REG_CONFIG,
	DIO_BOARD_INIT_ERROR,
	DIO_INDEX_OUT_OF_RANGE,
	DIO_FAILED_TO_LOAD_LIBRARY,
	DIO_FAILED_TO_FREE_LIBRARY,
	DIO_FAILED_TO_LOAD_FUNCTION,
	DIO_PLUGIN_CONNECTION_ERROR,
	DIO_PLUGIN_API_ERROR,
	DIO_OTHER_ERROR
}
DIO_ERROR;

// STATUS EVENTS FOR 
// CALLBACK FUNCTION
typedef enum
{
	DIO_PLUGIN_CONFIG_EVT = 1000,
    DIO_DEVICE_STATUS_EVT,
	DIO_DI_STATUS_EVT,
	DIO_DO_STATUS_EVT
}
DIO_STATUS_EVENT;

// PLUGIN CONFIG
typedef struct 
{	
	DWORD	dwNbInputs;
	DWORD	dwNbOutputs;
	
	DWORD	dwFirstInputIdx;
	DWORD	dwFirstOutputIdx;
}
struct_dio_board_config, *P_DIO_BOARD_CONFIG;

typedef struct 
{
	DWORD					dwNbBoards;
	struct_dio_board_config	sBoardConfig[DIO_NB_BOARDS_MAX];
}
struct_dio_plugin_config, *P_DIO_PLUGIN_CONFIG;

// DIO DEVICE STATUS
typedef struct 
{
	DWORD	dwBoardIdx;

	BOOL	bDeviceError;
	BOOL	bLinkError;
}
struct_dio_board_status, *P_DIO_BOARD_STATUS;

typedef struct 
{
    DWORD					dwNbBoards;
    struct_dio_board_status	sBoard[DIO_NB_BOARDS_MAX]; 
}
struct_dio_plugin_status, *P_DIO_PLUGIN_STATUS;

typedef struct 
{
	DWORD						dwNbPlugins;
	struct_dio_plugin_status	sPlugin[DIO_NB_PLUGINS_MAX];
}
struct_dio_dev_status, *P_DIO_DEV_STATUS;

// DIO DI/DO STATUS
typedef struct 
{
	DWORD		dwDioIdx;	// DI/DO index
	DIO_STATE	eState;
}
struct_dio_status, *P_DIO_STATUS;

/*--------------------------- FUNCTIONS: --------------------------*/


//************************ DIO MAIN FUNCTIONS *********************//

// CALLBACK FUNCTION FOR STATUS EVENT
// If eStatusEvt == DIO_DEVICE_STATUS_EVT then pData == P_DIO_DEV_STATUS for Main DIO
// If eStatusEvt == DIO_DI_STATUS_EVT else if eStatusEvt == DIO_DO_STATUS_EVT then pData == P_DIO_STATUS
typedef void (WINAPI *MDioStatusOccuredEvent)(OUT DIO_STATUS_EVENT eStatusEvt,
											  OUT PVOID pData);

// INITIALIZATION OF DIO MAIN
EXPORT DIO_ERROR WINAPI MDioInit(IN HKEY hKey,									// NULL = HKEY_LOCAL_MACHINE
								 IN CHAR *szRoot,								// NULL = default registry path
								 IN MDioStatusOccuredEvent fncStatusOccured);	// NULL if don't care 

// RELEASE OF DIO MAIN 
EXPORT DIO_ERROR WINAPI MDioRelease(void);

// GET THE STATE OF THE PARTICULAR INPUT:
EXPORT DIO_STATE WINAPI MDioGetInputState(IN DWORD dwInputIndex, 
										  OUT DIO_ERROR *pDioErr); // NULL if don't want detailed error 

// SET THE STATE OF THE PARTICULAR INPUT:
// This function is only meaningful if the DIO MAIN module operates in the 
// simulation mode. Only in simulation mode, it makes sense to control the 
// state of the inputs.
EXPORT DIO_ERROR WINAPI MDioSetInputState(IN DWORD dwInputIndex, 
										  IN DIO_STATE eState);

// GET THE STATE OF THE PARTICULAR OUTPUT:
EXPORT DIO_STATE WINAPI MDioGetOutputState(IN DWORD dwOutputIndex,
											OUT DIO_ERROR *pDioErr); // NULL if don't want detailed error 

// SET THE STATE OF THE PARTICULAR OUTPUT:
EXPORT DIO_ERROR WINAPI MDioSetOutputState(IN DWORD dwOutputIndex, 
										   IN DIO_STATE eState);

// GET RANGE:
EXPORT DIO_ERROR WINAPI MDioGetRange(OUT DWORD *pdwFirstInputIndex,		// NULL if don't care 
									 OUT DWORD *pdwLastInputIndex,		// NULL if don't care
									 OUT DWORD *pdwFirstOutputIndex,	// NULL if don't care 
									 OUT DWORD *pdwLastOutputIndex);	// NULL if don't care

//************************ PLUGINS FUNCTIONS **********************//

// If eStatusEvt == DIO_DEVICE_STATUS_EVT then pData == P_DIO_PLUGIN_STATUS for pligins
// If eStatusEvt == DIO_DI_STATUS_EVT or eStatusEvt == DIO_DO_STATUS_EVT then pData == P_DIO_STATUS
typedef void (WINAPI *PDioStatusOccuredEvent)(OUT DIO_STATUS_EVENT eStatusEvt,
											  OUT PVOID pData,
											  OUT HINSTANCE hPlugin); // plug-in instance handle used for plug-in identification

// INITIALIZATION OF PLUGIN
EXPORT DIO_ERROR WINAPI PDioInit(IN HKEY hKey,									// NULL = HKEY_LOCAL_MACHINE
								 IN CHAR *szRoot,								// NULL = default registry path
								 IN HINSTANCE hPlugin,							// plug-in instance handle used for plug-in identification
								 IN PDioStatusOccuredEvent fncStatusOccured);	// NULL if don't care 

// RELEASE OF DIO PLUGIN 
EXPORT DIO_ERROR WINAPI PDioRelease(void);

// GET THE STATE OF THE PARTICULAR INPUT:
EXPORT DIO_STATE WINAPI PDioGetInputState(IN DWORD dwInputIndex, 
										  OUT DIO_ERROR *pDioErr); // NULL if don't want detailed error 

// GET THE STATE OF THE PARTICULAR OUTPUT:
EXPORT DIO_STATE WINAPI PDioGetOutputState(IN DWORD dwOutputIndex,
											OUT DIO_ERROR *pDioErr); // NULL if don't want detailed error 

// SET THE STATE OF THE PARTICULAR OUTPUT:
EXPORT DIO_ERROR WINAPI PDioSetOutputState(IN DWORD dwOutputIndex, 
										   IN DIO_STATE eState);

// GET RANGE:
EXPORT DIO_ERROR WINAPI PDioGetRange(OUT DWORD *pdwFirstInputIndex,		// NULL if don't care 
									 OUT DWORD *pdwLastInputIndex,		// NULL if don't care
									 OUT DWORD *pdwFirstOutputIndex,	// NULL if don't care 
									 OUT DWORD *pdwLastOutputIndex);	// NULL if don't care

/*--------------------------- VARIABLES: --------------------------*/

#ifdef __cplusplus
}
#endif

#endif