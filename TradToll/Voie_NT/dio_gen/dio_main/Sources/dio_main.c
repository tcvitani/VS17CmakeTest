/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_MAIN														 */
/* FILE:	 dio_main.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include <CSRLC32.H>
#include <reg.h>
#include "trc.h"
#include <noyau.h>


#define PCL_DEF
#include <dio_main.h>
#undef PCL_DEF

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

PRIVATE CRITICAL_SECTION DioCriticalSection = {0};

#define DBG_NAME		"CSR_DIO_MAIN"

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
		CSR_REG_KEYn_LANE_BASE \
		CSR_REG_KEYn_CONFIG \
		MOD_REG_KEYn_MODULES

#define FNC_NAME_INIT_PLUGIN		"PDioInit"
#define FNC_NAME_RELEASE_PLUGIN		"PDioRelease"
#define FNC_NAME_GET_INPUT			"PDioGetInputState"
#define FNC_NAME_GET_OUTPUT			"PDioGetOutputState"
#define FNC_NAME_SET_OUTPUT			"PDioSetOutputState"
#define FNC_NAME_GET_RANGE			"PDioGetRange"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef DIO_ERROR (WINAPI *pFuncInit)(IN HKEY hKey, 
									  IN CHAR *szRoot,
									  IN HINSTANCE hPlugin,
									  IN PDioStatusOccuredEvent fncStatusOccured);

typedef DIO_ERROR (WINAPI *pFuncRelease)(void);

typedef DIO_STATE (WINAPI *pFuncGetInput)(IN DWORD dwInputIndex, 
										  OUT DIO_ERROR *pDioErr);

typedef DIO_STATE (WINAPI *pFuncGetOutput)(IN DWORD dwOutputIndex, 
										   OUT DIO_ERROR *pDioErr);

typedef DIO_ERROR (WINAPI *pFuncSetOutput)(IN DWORD dwOutputIndex, 
										   IN DIO_STATE eState);

typedef DIO_ERROR (WINAPI *pFuncGetRange)(OUT DWORD *pdwFirstInputIndex,
										   OUT DWORD *pdwLastInputIndex,
										   OUT DWORD *pdwFirstOutputIndex,
										   OUT DWORD *pdwLastOutputIndex);

typedef struct
{
	DWORD	dwNbOfInputs;
	DWORD	dwNbOfOutputs;
	
	DWORD	dwFirstInputIndex;
	DWORD	dwFirstOutputIndex;

	BOOL	bLinkError;
	BOOL	bDeviceError;
}
struct_boards_details;

typedef struct
{
	CHAR					szDllName[MAX_PATH];
	
	HINSTANCE				hDioLib;
	
	pFuncInit				pFncInitPlugin;
	pFuncRelease			pFncReleasePlugin;
	pFuncGetInput			pFncGetInputState;
	pFuncGetOutput			pFncGetOutputState;
	pFuncSetOutput			pFncSetOutputState;
	pFuncGetRange			pFncGetIORange;
	
	DWORD					dwFirstInputIndex;	// index of first input
    DWORD					dwLastInputIndex;	// index of last input
	DWORD					dwFirstOutputIndex;	// index of first output
    DWORD					dwLastOutputIndex;	// index of last output	
	
	DWORD					dwNbOfBoards;
	struct_boards_details	sBoardDetails[DIO_NB_BOARDS_MAX];
}
struct_plugin_details;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE BOOL DIO_InitTraces (struct DBG *dbg);
PRIVATE void DIO_Error (IN struct DBG *dbg, IN char *fmt, ...);
PRIVATE void DIO_Trace (IN struct DBG *dbg, IN char *fmt, ...);

PRIVATE DIO_ERROR InitPlugins(IN HKEY hKey, IN CHAR *szRoot);
PRIVATE DIO_ERROR RegistryParameters (IN HKEY hKey, IN CHAR *szRoot);

PRIVATE void WINAPI P_DioStatusOccuredEvent(DIO_STATUS_EVENT eStatusEvt, 
											PVOID pData,
											HINSTANCE hPlugin);

PRIVATE void MDioSetSimuInputStatus(DWORD dwInputIndex, DIO_STATE eState);
PRIVATE DIO_STATE MDioGetSimuInputStatus(DWORD dwInputIndex);

PRIVATE void MDioSetSimuOutputStatus(DWORD dwOutputIndex, DIO_STATE eState);
PRIVATE DIO_STATE MDioGetSimuOutputStatus(DWORD dwOutputIndex);

/*-------------------------------- VARIABLES: -------------------------------*/

#define LOC_DEF
#include <global.h>

// shared static structure of this DLL
struct  
{
	DIO_STATE	eInputsState[DIO_NB_PLUGINS_MAX * DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX];	// table of inputs for the simulator
    DIO_STATE	eOutputsState[DIO_NB_PLUGINS_MAX * DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX];	// table of outputs for the simulator
}
SIMU = {0};

#include <global.h>

struct  
{
	BOOL					bUseSimulator;				
	
	DWORD					dwNbPlugins;
    struct_plugin_details	sPluginDetails[DIO_NB_PLUGINS_MAX];
    
    DWORD					dwNbInputs;			// number of available inputs
    DWORD					dwNbOutputs;		// number of available outputs
	
	DWORD					dwFirstInputIndex;	// index of first input
    DWORD					dwLastInputIndex;	// index of last input
	DWORD					dwFirstOutputIndex;	// index of first output
    DWORD					dwLastOutputIndex;	// index of last output
	
	DWORD					dwExecThreadId;		// id of thread used to execute callback function
	MDioStatusOccuredEvent	pCallbackFnc;		// callback function
}
GLOBAL = {0};

struct DBG 
{
	// error file data
    void *debug;
	// trace file data
    void *traces;
} DBG;

/*---------------------------------- CODE: ----------------------------------*/

/**/
/*******************************************************************/
/*SYNTAX: BOOL WINAPI DllMain(IN HINSTANCE hInst,                  */
/*                            IN DWORD dwReason,                   */
/*                            IN LPVOID lpReserved)                */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            DLL entry point.                                     */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      IN HINSTANCE hInst      - A handle to the DLL.             */
/*      IN DWORD dwReason       - Specifies a flag indicating why  */
/*                                the DLL entry-point function is  */
/*                                being called.                    */
/*      IN LPVOID lpReserved    - Specifies further aspects of DLL */
/*                                initialization and cleanup.      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  TRUE                  Initialization succeeds.                 */
/*  FALSE                 Initialization fails.                    */
/*******************************************************************/
extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{		
    switch(dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
	case DLL_PROCESS_ATTACH:			
		break;
        
        // The attached process creates a new thread.  
	case DLL_THREAD_ATTACH:  
		break; 
        
        // The thread of the attached process terminates. 
	case DLL_THREAD_DETACH:  
		break;  
        
        // The DLL unloading due to process termination or call to FreeLibrary. 
	case DLL_PROCESS_DETACH:			
		break;          
        
	default:             
		break;     
    }      
    
    return TRUE; 
    
    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR PDioInit(IN HKEY hKey, IN CHAR *szRoot)		   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Initialisation of DIO CONTEC module.                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */ 
/*	DIO_INDEX_OUT_OF_RANGE	- Input/Output index out of range	   */
/*******************************************************************/
EXPORT DIO_ERROR WINAPI MDioInit(IN HKEY hKey, 
								 IN CHAR *szRoot, 
								 IN MDioStatusOccuredEvent fncStatusOccured)
{  
	HKEY		hRootKey			= CSR_REG_KEYi_ROOT;
	CHAR		szDioKey[MAX_PATH]	= {0};
	DIO_ERROR	eRet;
	
	DIO_InitTraces(&DBG);
	
	if(hKey != NULL)
		hRootKey = hKey;
	
	if(szRoot != NULL)
		strncpy_s(szDioKey, sizeof(szDioKey), szRoot, sizeof(szDioKey));
	else
		sprintf_s(szDioKey, sizeof(szDioKey), "%s%s", KEY_MODULE, DIO_REG_KEYn_IO_ROOT);
	    
	eRet = RegistryParameters(hRootKey, szDioKey);
	if(eRet != DIO_SUCCESS)
		return eRet;
	
	GLOBAL.pCallbackFnc = fncStatusOccured;

	InitializeCriticalSection(&DioCriticalSection);
	
	if (GLOBAL.pCallbackFnc != NULL)
		CreateThread(NULL, 0, DIO_ExecThread, NULL, 0, &GLOBAL.dwExecThreadId);

	if(!GLOBAL.bUseSimulator)
	{
		eRet = InitPlugins(hRootKey, szDioKey);
		if(eRet != DIO_SUCCESS)
			return eRet;
	}
	else
	{
		CreateThread(NULL, 0, DIO_PollThread, NULL, 0, 0);
	}
	
    return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR RegistryParametres(HKEY hKey, CHAR *szRoot)    */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function reads registry parameters.			   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */
/*	DIO_INDEX_OUT_OF_RANGE	- Input/O index out of range		   */
/*******************************************************************/
PRIVATE DIO_ERROR RegistryParameters (IN HKEY hKey, IN CHAR *szRoot)
{
    DWORD	dwResult;
    DWORD	dwIdx					= 0;
	DWORD	dwIdx2					= 0;	
	DWORD	dwSize					= 0;
	DWORD	dwDisabled				= 0;
	DWORD	dwTmp					= 0;
	DWORD	dwFirstInputIndex		= 0;
	DWORD	dwFirstOutputIndex		= 0;	
	CHAR	*pCh					= NULL;
	CHAR	szSubKey[MAX_PATH]		= {0};
	CHAR	szExtendedKey[MAX_PATH]	= {0};
	CHAR	szPluginsKey[MAX_PATH]	= {0};
	CHAR	szTempKey[MAX_PATH]		= {0};
	CHAR	szDioBoard[MAX_PATH]	= {0};
	
	if(szRoot == NULL || hKey == NULL)
		return DIO_REG_KEY_MISSING;

    // use simulator 
    dwResult = REG_Lire_Entier (hKey, szRoot, DIO_REGKEY_USE_SIMULATOR, &GLOBAL.bUseSimulator);
    if (dwResult != ERROR_SUCCESS)
        GLOBAL.bUseSimulator = FALSE;
    
	GLOBAL.dwNbPlugins = 0;
	
	sprintf_s(szPluginsKey, sizeof(szPluginsKey), "%s%s", szRoot, DIO_REG_KEYn_PLUGINS);
	
	if(GLOBAL.bUseSimulator)
	{
		GLOBAL.dwFirstInputIndex = DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX;
		GLOBAL.dwFirstOutputIndex = DIO_NB_BOARDS_MAX * DIO_NB_OUTPUTS_MAX;
	}
		
	dwSize = sizeof(szSubKey);
	while(REG_Enum_Cles(hKey, szPluginsKey, dwIdx++, szSubKey, &dwSize) == ERROR_SUCCESS)
	{		
		sprintf_s(szExtendedKey, sizeof(szExtendedKey), "%s%s", szPluginsKey, szSubKey);
		
		// is plug-in disabled
		if(REG_Lire_Entier(hKey, szExtendedKey, DIO_REGKEY_DISABLED, &dwDisabled) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, "Incorrect registry key: %s\\%s !", szExtendedKey, DIO_REGKEY_DISABLED);
			
			return DIO_INVALID_REG_CONFIG;
		}

		if(dwDisabled > 0)
		{
			dwSize = sizeof(szSubKey);
			continue;
		}
		
		// reading dll name of plug-in
		dwSize = sizeof(GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].szDllName);
		if(REG_Lire_Chaine(hKey, szExtendedKey, DIO_REGKEY_DLL_NAME, GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].szDllName, &dwSize) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, "Incorrect registry key: %s\\%s !", szExtendedKey, DIO_REGKEY_DLL_NAME);
			
			return DIO_INVALID_REG_CONFIG;				
		}

		// If simulator mode, read IO configuration from registry, 
		// otherwise get it from plug-in using PDioGetRange() function
		if(GLOBAL.bUseSimulator)
		{	
			GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstInputIndex = DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX;
			GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstOutputIndex = DIO_NB_BOARDS_MAX * DIO_NB_OUTPUTS_MAX;

			dwIdx2 = 0;
			dwSize = sizeof(szTempKey);
			while(REG_Enum_Cles(hKey, szExtendedKey, dwIdx2, szTempKey, &dwSize) == ERROR_SUCCESS)
			{			
				// SubKeys should be in following order: "Carte0", "Carte1", "Carte2", etc...
				pCh = strstr(szTempKey, DIO_REGKEY_DIO_BOARD);
				if (pCh == NULL || (unsigned)atoi(pCh + strlen(DIO_REGKEY_DIO_BOARD)) != dwIdx2)
				{
					DIO_Error (&DBG, "Incorrect registry key: %s%s !", szExtendedKey, szTempKey);
					
					return DIO_INVALID_REG_CONFIG;
				}
				
				sprintf_s(szDioBoard, sizeof(szDioBoard), "%s\\%s\\", szExtendedKey, szTempKey);
				
				// reading DioBoardX first input index
				if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szDioBoard, DIO_REGKEY_FIRST_INPUT_IDX, &dwFirstInputIndex) != ERROR_SUCCESS)
				{
					DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_FIRST_INPUT_IDX);
					
					return DIO_REG_KEY_MISSING;
				}
				
				// First input index 
				if(GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstInputIndex > dwFirstInputIndex)
					GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstInputIndex = dwFirstInputIndex;
				
				// reading DioBoardX number of INPUTS
				if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szDioBoard, DIO_REGKEY_NB_OF_INPUTS, &dwTmp) != ERROR_SUCCESS )
				{
					DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_NB_OF_INPUTS);
					
					return DIO_REG_KEY_MISSING;
				}
				
				// Last input index
				if(GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastInputIndex <=  dwFirstInputIndex + dwTmp - 1)
					GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastInputIndex = dwFirstInputIndex + dwTmp - 1;
				
				GLOBAL.dwNbInputs += dwTmp;
				
				// reading DioBoardX first output index
				if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szDioBoard, DIO_REGKEY_FIRST_OUTPUT_IDX, &dwFirstOutputIndex) != ERROR_SUCCESS)
				{
					DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_FIRST_OUTPUT_IDX);
					
					return DIO_REG_KEY_MISSING;
				}

				if(GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstOutputIndex > dwFirstOutputIndex)
					GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstOutputIndex = dwFirstOutputIndex;
				
				// reading DioBoardX number of OUTPUTS
				if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szDioBoard, DIO_REGKEY_NB_OF_OUTPUTS, &dwTmp) != ERROR_SUCCESS )
				{
					DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_NB_OF_OUTPUTS);
					
					return DIO_REG_KEY_MISSING;
				} 
								
				// Last output index
				if(GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastOutputIndex <=  dwFirstOutputIndex + dwTmp - 1)
					GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastOutputIndex = dwFirstOutputIndex + dwTmp - 1;
				
				GLOBAL.dwNbOutputs += dwTmp;
				
				// first and last input index
				if(GLOBAL.dwFirstInputIndex >= GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstInputIndex)
					GLOBAL.dwFirstInputIndex = GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstInputIndex;
				
				if(GLOBAL.dwLastInputIndex <= GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastInputIndex)
					GLOBAL.dwLastInputIndex = GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastInputIndex;
				
				// first and last output index
				if(GLOBAL.dwFirstOutputIndex >= GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstOutputIndex)
					GLOBAL.dwFirstOutputIndex = GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwFirstOutputIndex;
				
				if(GLOBAL.dwLastOutputIndex <= GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastOutputIndex)
					GLOBAL.dwLastOutputIndex = GLOBAL.sPluginDetails[GLOBAL.dwNbPlugins].dwLastOutputIndex;

				dwSize = sizeof(szTempKey);

				dwIdx2++;	
			}
		}
		
		GLOBAL.dwNbPlugins++;
	}
	
	if(dwIdx == 1)
	{
		DIO_Error(&DBG, "Invalid registry configuration: %s!", szPluginsKey);

		return DIO_INVALID_REG_CONFIG;
	}
	
	return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR InitPlugins(IN HKEY hKey, IN CHAR *szRoot)	   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Initialisation of DIO CONTEC module.                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */ 
/*	DIO_INDEX_OUT_OF_RANGE	- Input/Output index out of range	   */
/*******************************************************************/
PRIVATE DIO_ERROR InitPlugins(IN HKEY hKey, IN CHAR *szRoot)
{
	DWORD		dwIdx					= 0;
	DWORD		dwFirstInputIndex		= 0;
	DWORD		dwLastInputIndex		= 0;
	DWORD		dwFirstOutputIndex		= 0;
	DWORD		dwLastOutputIndex		= 0;	
	CHAR		szPluginKey[MAX_PATH]	= {0};
	DIO_ERROR	eRet					= DIO_SUCCESS;
	
	if(szRoot == NULL || hKey == NULL)
		return DIO_REG_KEY_MISSING;
	
	sprintf_s(szPluginKey, sizeof(szPluginKey), "%s%s", szRoot, DIO_REG_KEYn_PLUGINS);
				
	// Load all plug-ins libraries
	for(dwIdx = 0; dwIdx < GLOBAL.dwNbPlugins; dwIdx++)
	{
		if(GLOBAL.sPluginDetails[dwIdx].szDllName[0] != 0)
		{
			// Load plug-in library
			GLOBAL.sPluginDetails[dwIdx].hDioLib = LoadLibrary(GLOBAL.sPluginDetails[dwIdx].szDllName);
			if(GLOBAL.sPluginDetails[dwIdx].hDioLib == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load library: %s", GLOBAL.sPluginDetails[dwIdx].szDllName);

				return DIO_FAILED_TO_LOAD_LIBRARY;
			}
			
			// Get pointers to the plug-in functions 
			GLOBAL.sPluginDetails[dwIdx].pFncInitPlugin = (pFuncInit)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_INIT_PLUGIN);
			if(GLOBAL.sPluginDetails[dwIdx].pFncInitPlugin == NULL)
			{				
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_INIT_PLUGIN, GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				return DIO_FAILED_TO_LOAD_FUNCTION;
			}
			
			GLOBAL.sPluginDetails[dwIdx].pFncReleasePlugin = (pFuncRelease)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_RELEASE_PLUGIN);
			if(GLOBAL.sPluginDetails[dwIdx].pFncReleasePlugin == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_RELEASE_PLUGIN, GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				return DIO_FAILED_TO_LOAD_FUNCTION;
			}
			
			GLOBAL.sPluginDetails[dwIdx].pFncGetInputState = (pFuncGetInput)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_GET_INPUT);
			if(GLOBAL.sPluginDetails[dwIdx].pFncGetInputState == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_GET_INPUT, GLOBAL.sPluginDetails[dwIdx].szDllName);

				return DIO_FAILED_TO_LOAD_FUNCTION;
			}
			
			GLOBAL.sPluginDetails[dwIdx].pFncGetOutputState = (pFuncGetOutput)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_GET_OUTPUT);
			if(GLOBAL.sPluginDetails[dwIdx].pFncGetOutputState == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_GET_OUTPUT, GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				return DIO_FAILED_TO_LOAD_FUNCTION;
			}
			
			GLOBAL.sPluginDetails[dwIdx].pFncSetOutputState = (pFuncSetOutput)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_SET_OUTPUT);
			if(GLOBAL.sPluginDetails[dwIdx].pFncSetOutputState == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_SET_OUTPUT, GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				return DIO_FAILED_TO_LOAD_FUNCTION;
			}

			GLOBAL.sPluginDetails[dwIdx].pFncGetIORange = (pFuncGetRange)GetProcAddress(GLOBAL.sPluginDetails[dwIdx].hDioLib, FNC_NAME_GET_RANGE);
			if(GLOBAL.sPluginDetails[dwIdx].pFncGetIORange == NULL)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to load function %s from library %s", FNC_NAME_GET_RANGE, GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				return DIO_FAILED_TO_LOAD_FUNCTION;
			}
			
			// Call init plug-in function
			eRet = GLOBAL.sPluginDetails[dwIdx].pFncInitPlugin(hKey, szPluginKey, GLOBAL.sPluginDetails[dwIdx].hDioLib, P_DioStatusOccuredEvent);
			if(eRet != DIO_SUCCESS)
			{
				DIO_Error(&DBG, "InitPlugins() -> Failed to init plug-in: library %s", GLOBAL.sPluginDetails[dwIdx].szDllName);	
				
				return eRet;
			}	
			
			// Get IO range
			eRet = GLOBAL.sPluginDetails[dwIdx].pFncGetIORange(&dwFirstInputIndex, &dwLastInputIndex, &dwFirstOutputIndex, &dwLastOutputIndex);
			if(eRet != DIO_SUCCESS)
			{
				DIO_Error(&DBG, "InitPlugins() -> Get IO range function failed: library %s", GLOBAL.sPluginDetails[dwIdx].szDllName);	
				
				return eRet;
			}
			
			// Count number of inputs and outputs
			GLOBAL.dwNbInputs += dwLastInputIndex - dwFirstInputIndex + 1;
			GLOBAL.dwNbOutputs += dwLastOutputIndex - dwFirstOutputIndex + 1;

			GLOBAL.sPluginDetails[dwIdx].dwFirstInputIndex = dwFirstInputIndex;
			GLOBAL.sPluginDetails[dwIdx].dwLastInputIndex = dwLastInputIndex;

			GLOBAL.sPluginDetails[dwIdx].dwFirstOutputIndex = dwFirstOutputIndex;
			GLOBAL.sPluginDetails[dwIdx].dwLastOutputIndex = dwLastOutputIndex;
			
			// first and last input index
			if(GLOBAL.dwFirstInputIndex >= dwFirstInputIndex)
				GLOBAL.dwFirstInputIndex = dwFirstInputIndex;
			
			if(GLOBAL.dwLastInputIndex <= dwLastInputIndex)
				GLOBAL.dwLastInputIndex = dwLastInputIndex;
			
			// first and last output index
			if(GLOBAL.dwFirstOutputIndex >= dwFirstOutputIndex)
				GLOBAL.dwFirstOutputIndex = dwFirstOutputIndex;
			
			if(GLOBAL.dwLastOutputIndex <= dwLastOutputIndex)
				GLOBAL.dwLastOutputIndex = dwLastOutputIndex;			
		}
		else
		{
			DIO_Error(&DBG, "InitPlugins() -> PLUGIN: %d -> Dll name empty!!!", dwIdx);	

			return DIO_OTHER_ERROR;
		}
	}
	
	return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR ReleasePlugins(void)						   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Initialization of DIO CONTEC module.                 */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */ 
/*	DIO_INDEX_OUT_OF_RANGE	- Input/Output index out of range	   */
/*******************************************************************/
PRIVATE DIO_ERROR ReleasePlugins(void)
{
	DWORD		dwIdx	= 0;
	DIO_ERROR	eRet	= DIO_SUCCESS;


	for(dwIdx = 0; dwIdx < GLOBAL.dwNbPlugins; dwIdx++)
	{
		if(GLOBAL.sPluginDetails[dwIdx].szDllName[0] != 0)
		{
			// call release function from plug-in
			if (GLOBAL.sPluginDetails[dwIdx].pFncReleasePlugin() != DIO_SUCCESS)
			{
				DIO_Error(&DBG, 
					"ReleasePlugins() -> pFncReleasePlugin() function returned error: %s!!!", 
					GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				eRet = DIO_FAILED_TO_FREE_LIBRARY;
			}

			// Free plug-in library
			if(!FreeLibrary(GLOBAL.sPluginDetails[dwIdx].hDioLib))
			{
				DIO_Error(&DBG, 
					"ReleasePlugins() -> FreeLibrary() function returned FALSE for Dll name %s!!!", 
					GLOBAL.sPluginDetails[dwIdx].szDllName);
				
				eRet = DIO_FAILED_TO_FREE_LIBRARY;
			}
		}
		else
		{
			DIO_Error(&DBG, "ReleasePlugins() PLUGIN: %d -> Dll name empty!!!", dwIdx);
		}	
	}
	
	return eRet;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR MDioRelease(void)							   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Deinitialisation of DIO CONTEC module.               */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */ 
/*	DIO_INDEX_OUT_OF_RANGE	- Input/O index out of range		   */
/*******************************************************************/
PUBLIC DIO_ERROR WINAPI MDioRelease(void)
{
	DIO_ERROR eRet = DIO_SUCCESS;

	if(!GLOBAL.bUseSimulator) 
		eRet = ReleasePlugins();

    return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_STATE WINAPI MDioGetInputState(IN DWORD dwInputIndex,*/
/*											OUT DIO_ERROR *pDioErr)*/
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Read current state of the particular input           */
/*=================================================================*/
/*PARAMETERS:													   */
/*			DWORD		dwInputIndex	- index of digital input   */
/*			DIO_ERROR	*pDioErr		- Pointer to the variable  */
/*										  that contains error code.*/
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_STATE_ LOW	-	Low state of digital input/output.		   */
/*	DIO_STATE_HIGH	-	High state of digital input/output.		   */
/*******************************************************************/
PUBLIC DIO_STATE WINAPI MDioGetInputState(IN DWORD dwInputIndex, 
										  OUT DIO_ERROR *pDioErr)
{
	DIO_STATE	eInputState			= -1;
	DIO_ERROR	eDioErr				= DIO_SUCCESS;
	DWORD		dwIdx				= 0;
	DWORD		dwNbPlugins			= 0;
	DWORD		dwFirstInputIndex	= 0;
	DWORD		dwLastInputIndex	= 0;

	dwFirstInputIndex = GetIOFirstInputIdx();
	dwLastInputIndex = GetIOLastInputIdx();

	if((dwInputIndex > dwLastInputIndex) || (dwInputIndex < dwFirstInputIndex))
	{
		DIO_Error(&DBG, 
			"MDioGetInputState(): input index %d -> less than %d or greater than %d!", 
			dwInputIndex, 
			dwFirstInputIndex, 
			dwLastInputIndex);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;
		
		eInputState = -1;
	}
	else
	{
		if(!DIOWorksAsSimulator()) 
		{	
			dwNbPlugins = GetNumberOfPlugins();

			for(dwIdx = 0; dwIdx < dwNbPlugins; dwIdx++)
			{
				if(dwInputIndex >= GetIOPluginFirstInputIdx(dwIdx) 
					&& dwInputIndex <= GetIOPluginLastInputIdx(dwIdx))
				{
					eInputState = GLOBAL.sPluginDetails[dwIdx].pFncGetInputState(dwInputIndex, &eDioErr);

					break;
				}	
			}

			if(eInputState == -1)
			{
				DIO_Error(&DBG, 
					"MDioGetInputState(): -> DIO board that should handle input index %d has not found!", 
					dwInputIndex);
				
				eDioErr = DIO_INDEX_OUT_OF_RANGE;
			}	
		}
		else
		{
			eInputState = MDioGetSimuInputStatus(dwInputIndex);
			
			eDioErr = DIO_SUCCESS;
		}
	}

	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
	return eInputState;   
}

/**/
/*******************************************************************/
/*SYNTAX DIO_ERROR MDioSetInputState(IN DWORD dwInputIndex,		   */ 
/*										IN DIO_STATE eState)	   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*           Set state of the particular output.				   */
/*=================================================================*/
/*PARAMETERS:													   */
/*			DWORD		dwInputIndex	- index of digital input   */
/*			DIO_STATE	eState			- The desired state of	   */
/*										digital output.			   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */
/*	DIO_INDEX_OUT_OF_RANGE	- Input/O index out of range		   */
/*******************************************************************/
EXPORT DIO_ERROR WINAPI MDioSetInputState(IN DWORD dwInputIndex, 
										  IN DIO_STATE eState)
{	
	DIO_ERROR	eDioErr				= DIO_SUCCESS;
	DWORD		dwFirstInputIndex	= 0;
	DWORD		dwLastInputIndex	= 0;

	dwFirstInputIndex = GetIOFirstInputIdx();
	dwLastInputIndex = GetIOLastInputIdx();

	if(GLOBAL.bUseSimulator) 
	{
		if((dwInputIndex > dwLastInputIndex) 
			|| (dwInputIndex < dwFirstInputIndex) 
			|| (eState > DIO_STATE_HIGH) 
			|| (eState < DIO_STATE_LOW))
		{
			DIO_Error(&DBG, "MDioSetInputState( ): -> input index %d less than %d or greater than %d, or state %d is wrong!", 
					dwInputIndex, 
					dwFirstInputIndex, 
					dwLastInputIndex, 
					eState);
			
			eDioErr = DIO_INDEX_OUT_OF_RANGE;
		}
		else
		{
			MDioSetSimuInputStatus(dwInputIndex, eState);

			eDioErr = DIO_SUCCESS;
		}	
	}
	else
	{
		DIO_Error(&DBG, 
			"MDioSetInputState(): => It is impossible to set input state if not SUMULATION mode");
		
		eDioErr = DIO_OTHER_ERROR;
	}
	
	return eDioErr;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR MDioGetOutputState(IN DWORD dwOutputIndex,	   */
/*										OUT DIO_ERROR *pDioErr)	   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Get current state of the particular output           */
/*=================================================================*/
/*PARAMETERS:													   */
/*			DWORD		dwOutputIndex	- index of digital output  */
/*			DIO_ERROR	*pDioErr		- Pointer to the variable  */
/*										  that contains error code.*/
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_STATE_ LOW	-	Low state of digital input/output.		   */
/*	DIO_STATE_HIGH	-	High state of digital input/output.		   */
/*******************************************************************/
EXPORT DIO_STATE WINAPI MDioGetOutputState(IN DWORD dwOutputIndex, 
										   OUT DIO_ERROR *pDioErr)
{
	DIO_STATE	eOutputState		= DIO_STATE_LOW;
	DIO_ERROR	eDioErr				= DIO_OTHER_ERROR;
	DWORD		dwIdx				= 0;
	DWORD		dwNbPlugins			= 0;
	DWORD		dwFirstOutputIndex	= 0;
	DWORD		dwLastOutputIndex	= 0;
	
	dwFirstOutputIndex = GetIOFirstOutputIdx();
	dwLastOutputIndex = GetIOLastOutputIdx();

	if((dwOutputIndex > dwLastOutputIndex) || (dwOutputIndex < dwFirstOutputIndex))
	{
		DIO_Error(&DBG, "MDioGetOutputState(): -> output index %d less than %d or greater than %d!", 
			dwOutputIndex, 
			dwFirstOutputIndex, 
			dwLastOutputIndex);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;
	}
	else
	{	
		if(!DIOWorksAsSimulator()) 
		{	
			dwNbPlugins = GetNumberOfPlugins();

			for(dwIdx = 0; dwIdx < dwNbPlugins; dwIdx++)
			{
				if(dwOutputIndex >=  GetIOPluginFirstOutputIdx(dwIdx) 
					&& dwOutputIndex <= GetIOPluginLastOutputIdx(dwIdx))
				{
					eOutputState = GLOBAL.sPluginDetails[dwIdx].pFncGetOutputState(dwOutputIndex, &eDioErr);
					
					break;
				}	
			}
			
			if(eDioErr != DIO_SUCCESS)
			{
				DIO_Error(&DBG, 
					"MDioGetOutputState( %d ): -> PDioGetOutputState() on plug-in has not passed!", 
					dwOutputIndex);			
			}	
		}
		else
		{
			eOutputState = MDioGetSimuOutputStatus(dwOutputIndex);
			
			eDioErr = DIO_SUCCESS;
		}
	}
	
	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
	return eOutputState;
}

/**/
/*******************************************************************/
/*SYNTAX DIO_ERROR MDioSetOutputState(IN DWORD dwOutputIndex,	   */ 
/*										IN DIO_STATE eState)	   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*           Set state of the particular output.				   */
/*=================================================================*/
/*PARAMETERS:													   */
/*			DWORD		dwOutputIndex	- index of digital output  */
/*			DIO_STATE	eState			- The desired state of	   */
/*										digital output.			   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */
/*	DIO_INDEX_OUT_OF_RANGE	- Input/O index out of range		   */
/*******************************************************************/
EXPORT DIO_ERROR WINAPI MDioSetOutputState(IN DWORD dwOutputIndex, 
										   IN DIO_STATE eState)
{
	DIO_ERROR	eDioErr				= DIO_OTHER_ERROR;
	DWORD		dwIdx				= 0;	
	DWORD		dwNbPlugins			= 0;
	DWORD		dwFirstOutputIndex	= 0;
	DWORD		dwLastOutputIndex	= 0;

	dwFirstOutputIndex = GetIOFirstOutputIdx();
	dwLastOutputIndex = GetIOLastOutputIdx();

	if((dwOutputIndex > dwLastOutputIndex) 
		|| (dwOutputIndex < dwFirstOutputIndex) 
		|| (eState > DIO_STATE_HIGH)
		|| (eState < DIO_STATE_LOW))
	{
		DIO_Error(&DBG, 
			"MDioSetOutputState( ): -> output index %d less than %d or greater than %d, or state %d is wrong!", 
			dwOutputIndex, 
			dwFirstOutputIndex, 
			dwLastOutputIndex, 
			eState);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;
	}
	else
	{
		if(!DIOWorksAsSimulator()) 
		{	
			dwNbPlugins = GetNumberOfPlugins();

			for(dwIdx = 0; dwIdx < GLOBAL.dwNbPlugins; dwIdx++)
			{
				if(dwOutputIndex >= GetIOPluginFirstOutputIdx(dwIdx) 
					&& dwOutputIndex <= GetIOPluginLastOutputIdx(dwIdx))
				{
					eDioErr = GLOBAL.sPluginDetails[dwIdx].pFncSetOutputState(dwOutputIndex, eState);
					
					break;
				}	
			}
			
			if(eDioErr != DIO_SUCCESS)
			{
				DIO_Error(&DBG, 
					"MDioSetOutputState( %d, %d ): -> PDioSetOutputState() on plug-in has not passed!", 
					dwOutputIndex, 
					eState);			
			}	
		}
		else
		{
			MDioSetSimuOutputStatus(dwOutputIndex, eState);
			
			eDioErr	= DIO_SUCCESS;
		}
	}
	
    return eDioErr;
}

/**/
/*******************************************************************/
/*SYNTAX DIO_ERROR MDioGetRange(OUT DWORD *pdwFirstInputIndex,	   */
/*								OUT DWORD *pdwLastInputIndex,	   */
/*								OUT DWORD *pdwFirstOutputIndex,	   */
/*								OUT DWORD *pdwLastOutputIndex)	   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*           Get a range of controlled digital inputs and outputs. */
/*=================================================================*/
/*PARAMETERS:													   */
/*			DWORD *pdwFirstInputIndex	- first input index		   */ 
/*			DWORD *pdwLastInputIndex	- last input index		   */
/*			DWORD *pdwFirstOutputIndex	- first output index	   */
/*			DWORD *pdwLastOutputIndex	- last output index		   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_SUCCESS				- successful initialization			   */
/*	DIO_REG_KEY_MISSING		- registry key missing				   */
/*	DIO_INVALID_REG_CONFIG	- invalid registry configuration	   */
/*	DIO_BOARD_INIT_ERROR	- DIO board initialization error	   */
/*	DIO_INDEX_OUT_OF_RANGE	- Input/O index out of range		   */
/*******************************************************************/
EXPORT DIO_ERROR WINAPI MDioGetRange(OUT DWORD *pdwFirstInputIndex,
									 OUT DWORD *pdwLastInputIndex,
									 OUT DWORD *pdwFirstOutputIndex,
									 OUT DWORD *pdwLastOutputIndex)
{			
	if(pdwFirstInputIndex != NULL)
		*pdwFirstInputIndex = GetIOFirstInputIdx();
	
	if(pdwLastInputIndex != NULL)
		*pdwLastInputIndex = GetIOLastInputIdx();
	
	if(pdwFirstOutputIndex != NULL)
		*pdwFirstOutputIndex = GetIOFirstOutputIdx();
	
	if(pdwLastOutputIndex != NULL)
		*pdwLastOutputIndex = GetIOLastOutputIdx();
	
    return DIO_SUCCESS;	
}

/*******************************************************************/
/*SYNTAX: void SetPluginConfig(DWORD dwPluginIdx,				   */
/*							P_DIO_PLUGIN_CONFIG pPluginConfig)	   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void SetPluginConfig(DWORD dwPluginIdx, 
							 P_DIO_PLUGIN_CONFIG pPluginConfig)
{
	DWORD dwIdx = 0;

	if (pPluginConfig != NULL)
	{
		EnterCriticalSection(&DioCriticalSection);
		GLOBAL.sPluginDetails[dwPluginIdx].dwNbOfBoards = pPluginConfig->dwNbBoards;

		for(dwIdx = 0; dwIdx < pPluginConfig->dwNbBoards; dwIdx++)
		{
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].dwFirstInputIndex =
				pPluginConfig->sBoardConfig[dwIdx].dwFirstInputIdx;
			
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].dwFirstOutputIndex =
				pPluginConfig->sBoardConfig[dwIdx].dwFirstOutputIdx;
			
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].dwNbOfInputs =
				pPluginConfig->sBoardConfig[dwIdx].dwNbInputs;
			
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].dwNbOfOutputs =
				pPluginConfig->sBoardConfig[dwIdx].dwNbOutputs;

			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].bLinkError = -1;
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[dwIdx].bDeviceError = -1;
		}
		LeaveCriticalSection(&DioCriticalSection);
	}
}

/*******************************************************************/
/*SYNTAX: void ProcessDeviceStatusMsg(P_DIO_BOARD_STATUS pStatus,  */
/*									  DWORD dwPluginIdx)		   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void ProcessDeviceStatusMsg(P_DIO_BOARD_STATUS pDioStatus, 
									DWORD dwPluginIdx)
{
	struct_dio_dev_status	sDevStatus		= {0};
	DWORD					dwIdx			= 0;
	DWORD					dwIdx2			= 0;
	DWORD					dwNbPlugins		= 0;
	BOOL					bLinkError		= FALSE;
	BOOL					bDeviceError	= FALSE;

	if (pDioStatus != NULL)
	{
		dwNbPlugins = GetNumberOfPlugins();
		
		EnterCriticalSection(&DioCriticalSection);
		bLinkError = GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[pDioStatus->dwBoardIdx].bLinkError;
		bDeviceError = GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[pDioStatus->dwBoardIdx].bDeviceError;
		LeaveCriticalSection(&DioCriticalSection);

		if (pDioStatus->bLinkError !=  bLinkError ||
			pDioStatus->bDeviceError != bDeviceError)
		{
			EnterCriticalSection(&DioCriticalSection);
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[pDioStatus->dwBoardIdx].bLinkError = pDioStatus->bLinkError;
			GLOBAL.sPluginDetails[dwPluginIdx].sBoardDetails[pDioStatus->dwBoardIdx].bDeviceError = pDioStatus->bDeviceError;

			sDevStatus.dwNbPlugins = dwNbPlugins;
			LeaveCriticalSection(&DioCriticalSection);

			for (dwIdx = 0; dwIdx < dwNbPlugins; dwIdx++)
			{
				EnterCriticalSection(&DioCriticalSection);
				sDevStatus.sPlugin[dwIdx].dwNbBoards = GLOBAL.sPluginDetails[dwIdx].dwNbOfBoards;

				for (dwIdx2 = 0; dwIdx2 < sDevStatus.sPlugin[dwIdx].dwNbBoards; dwIdx2++)
				{
					sDevStatus.sPlugin[dwIdx].sBoard[dwIdx2].dwBoardIdx = dwIdx2;
					
					sDevStatus.sPlugin[dwIdx].sBoard[dwIdx2].bLinkError = 
						GLOBAL.sPluginDetails[dwIdx].sBoardDetails[dwIdx2].bLinkError;
					
					sDevStatus.sPlugin[dwIdx].sBoard[dwIdx2].bDeviceError = 
						GLOBAL.sPluginDetails[dwIdx].sBoardDetails[dwIdx2].bDeviceError;	
				}
				LeaveCriticalSection(&DioCriticalSection);
			}
			
			GLOBAL.pCallbackFnc(DIO_DEVICE_STATUS_EVT, &sDevStatus);
		}
	}
}

/*******************************************************************/
/*SYNTAX: void MDioStatusOccuredEvent(DIO_STATUS_EVENT eStatusEvt, */
/*									PVOID pData,				   */
/*									HINSTANCE hPlugin)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void WINAPI P_DioStatusOccuredEvent(DIO_STATUS_EVENT eStatusEvt, 
											PVOID pData,
											HINSTANCE hPlugin)
{
	P_DIO_STATUS		pDioStatus		= NULL;
	P_DIO_BOARD_STATUS	pDeviceStatus	= NULL;
	P_DIO_PLUGIN_CONFIG pPluginConfig	= NULL;
	HINSTANCE			hPluginInst;
	DWORD				dwIdx			= 0;
	DWORD				dwNbPlugins		= 0;
	DWORD				dwNbPluginIdx	= 0;

	switch(eStatusEvt)
	{
		case DIO_PLUGIN_CONFIG_EVT:
			pPluginConfig = (struct_dio_plugin_config*)pData;
			
			dwNbPlugins = GetNumberOfPlugins();
			for (dwIdx = 0; dwIdx < dwNbPlugins; dwIdx++)
			{
				hPluginInst = GetPluginHandle(dwIdx);
				if (hPlugin == hPluginInst)
				{
					SetPluginConfig(dwIdx, pPluginConfig);
					break;
				}
			}
			break;

		case DIO_DEVICE_STATUS_EVT:
			pDeviceStatus = malloc(sizeof(struct_dio_board_status));
			memcpy(pDeviceStatus, pData, sizeof(struct_dio_board_status));

			dwNbPlugins = GetNumberOfPlugins();
			for (dwIdx = 0; dwIdx < dwNbPlugins; dwIdx++)
			{
				hPluginInst = GetPluginHandle(dwIdx);
				if (hPlugin == hPluginInst)
				{
					dwNbPluginIdx = dwIdx;
					break;
				}
			}

			PostThreadMessage(GetExecThreadId(), 
							MDIO_EXEC_DEVICE_STATUS, 
							dwNbPluginIdx,
							(LPARAM)pDeviceStatus);
			break;
		
		case DIO_DI_STATUS_EVT:
			pDioStatus = (struct_dio_status*)pData;

			PostThreadMessage(GetExecThreadId(), 
							MDIO_EXEC_INPUT_STATE, 
							pDioStatus->dwDioIdx, 
							pDioStatus->eState);
			break;
	
		case DIO_DO_STATUS_EVT:
			pDioStatus = (struct_dio_status*)pData;

			PostThreadMessage(GetExecThreadId(), 
							MDIO_EXEC_OUTPUT_STATE, 
							pDioStatus->dwDioIdx, 
							pDioStatus->eState);
			break;

		default:
			DIO_Error(&DBG, 
				"MDioStatusOccuredEvent(): Unknown status event: %d!", 
				eStatusEvt);			
			break;
	}
}

/*******************************************************************/
/*SYNTAX: void ExecuteCallbackFnc(enum_pdio_exec_msg eMsg,		   */
/*								WPARAM wParam,					   */
/*								LPARAM lParam)					   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED void ExecuteCallbackFnc(enum_mdio_exec_msg eMsg,
								  WPARAM wParam,
								  LPARAM lParam)
{
	struct_dio_status	sDioStatus	= {0};
	P_DIO_BOARD_STATUS	pDioStatus	= NULL;
	
	switch (eMsg)
	{		
		case MDIO_EXEC_INPUT_STATE:
			sDioStatus.dwDioIdx = (DWORD)wParam;
			sDioStatus.eState = (DIO_STATE)lParam;
			
			GLOBAL.pCallbackFnc(DIO_DI_STATUS_EVT, &sDioStatus);
			break;
			
		case MDIO_EXEC_OUTPUT_STATE:
			sDioStatus.dwDioIdx = (DWORD)wParam;
			sDioStatus.eState = (DIO_STATE)lParam;
			
			GLOBAL.pCallbackFnc(DIO_DO_STATUS_EVT, &sDioStatus);
			break;
			
		case MDIO_EXEC_DEVICE_STATUS:
			pDioStatus = (struct_dio_board_status*)lParam;
			
			ProcessDeviceStatusMsg(pDioStatus, (DWORD)wParam);

			free(pDioStatus);
			break;
			
		default:
			DIO_Error(&DBG, 
				"ExecuteCallbackFnc(): received unhandled message type %d",
				eMsg);
			break;
	}
}

/**/
/*******************************************************************/
/*SYNTAX: BOOL DIO_InitTraces (struct DBG *dbg)					   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function initializes the module trace.          */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      struct DBG *dbg        - trace structure.                  */
/*=================================================================*/
/*  Return                       Description                       */
/*-----------------------------------------------------------------*/
/*  TRUE						 Trace initialization is           */
/*                               successful.                       */
/*  FALES						 Trace initialization is not       */
/*                               successful.                       */
/*******************************************************************/
PRIVATE BOOL DIO_InitTraces (struct DBG *dbg)
{
    DWORD	dwLen					= 0;
    DWORD	dwTailleMax				= 1400000L;
    char	pcKey[MAX_PATH]			= {0};
    char	rep_debug[MAX_PATH]		= {0};
    char	rep_traces[MAX_PATH]	= {0};
    char	debug[MAX_PATH]			= {0};
    char	traces[MAX_PATH]		= {0};
    
    // prepare path to the registry parameters
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);
    
    // max file size  
    REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax);
    
    // get path to the error file
    dwLen = sizeof(rep_debug );
    REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, rep_debug, &dwLen);
    
    // create error file name
	_snprintf_s(debug, sizeof(debug), MAX_PATH, "%s\\%s.ERR", rep_debug, DBG_NAME);
    
    // init error file
    TRC_Initialise_Trace("DEBUG", debug, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_TEXTE_SEUL|TRC_OPT_CONSOLE, (TRC_EMETTEUR *) &dbg->debug);
    TRC_Taille_Max_Fichier(dbg->debug, dwTailleMax);
    
    // get path to the trace file
    dwLen = sizeof(rep_traces);
    REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, rep_traces, &dwLen);
    
    /// create trace file name
	_snprintf_s(traces, sizeof(traces), MAX_PATH, "%s\\%s.TRC", rep_traces, DBG_NAME);
    
    // init trace file
    TRC_Initialise_Trace("TRACE", traces, TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *) &dbg->traces);
    TRC_Taille_Max_Fichier(dbg->traces, dwTailleMax);
    
    return TRUE;
}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Error (IN struct DBG *dbg, IN char *fmt, ...)   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function writes the string in the trace file.   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      IN struct DBG *dbg        - trace structure.               */
/*      IN char *fmt              - Pointer to the format		   */
/*                                  string.						   */
/*=================================================================*/
/*  Return                Description							   */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void DIO_Error(IN struct DBG *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);

    // write string in the error file
    if(dbg->debug != NULL)
        TRC_Direct_Trace_V(dbg->debug, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, NULL, 0, fmt, ap);

    // write string in the trace file
    if(dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);

    va_end(ap);
}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Trace(IN struct DBG *dbg, IN char *fmt, ...)    */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function writes the string in the error file.   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      IN struct DBG *dbg			- trace structure.             */
/*      IN char		  *fmt          - Pointer to the format        */
/*                                    string.					   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void DIO_Trace(IN struct DBG *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start(ap,fmt);

	// write string in the trace file	
    if(dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);

    va_end(ap);
}

/*******************************************************************/
/*SYNTAX: void MDioSetSimuInputStatus(DWORD dwInputIndex,		   */
/*									DIO_STATE eState)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void MDioSetSimuInputStatus(DWORD dwInputIndex, DIO_STATE eState)
{
	EnterCriticalSection(&DioCriticalSection);
	SIMU.eInputsState[dwInputIndex] = eState;
	LeaveCriticalSection(&DioCriticalSection);

}

/*******************************************************************/
/*SYNTAX: DIO_STATE MDioGetSimuInputStatus(DWORD dwInputIndex)	   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE DIO_STATE MDioGetSimuInputStatus(DWORD dwInputIndex)
{
	DIO_STATE eState;

	EnterCriticalSection(&DioCriticalSection);
	eState = SIMU.eInputsState[dwInputIndex];
	LeaveCriticalSection(&DioCriticalSection);

	return eState;	
}

/*******************************************************************/
/*SYNTAX: void MDioGetSimuInputStatusAll(DWORD dwInputIndex)	   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED void MDioGetSimuInputStatusAll(DIO_STATE *pDioStates, 
										 DWORD dwMaxSize)
{	
	if (pDioStates != NULL)
	{
		EnterCriticalSection(&DioCriticalSection);
		memcpy(pDioStates, SIMU.eInputsState, dwMaxSize);
		LeaveCriticalSection(&DioCriticalSection);
	}	
}

/*******************************************************************/
/*SYNTAX: void MDioSetSimuOutputStatus(DWORD dwOutputIndex,		   */
/*									DIO_STATE eState)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void MDioSetSimuOutputStatus(DWORD dwOutputIndex, DIO_STATE eState)
{
	EnterCriticalSection(&DioCriticalSection);
	SIMU.eOutputsState[dwOutputIndex] = eState;
	LeaveCriticalSection(&DioCriticalSection);
	
}

/*******************************************************************/
/*SYNTAX: DIO_STATE MDioGetSimuOutputStatus(DWORD dwOutputIndex)   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE DIO_STATE MDioGetSimuOutputStatus(DWORD dwOutputIndex)
{
	DIO_STATE eState;
	
	EnterCriticalSection(&DioCriticalSection);
	eState = SIMU.eOutputsState[dwOutputIndex];
	LeaveCriticalSection(&DioCriticalSection);
	
	return eState;	
}

/*******************************************************************/
/*SYNTAX: void MDioGetSimuOutputStatusAll(DWORD dwInputIndex)	   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED void MDioGetSimuOutputStatusAll(DIO_STATE *pDioStates, 
										  DWORD dwMaxSize)
{	
	if (pDioStates != NULL)
	{
		EnterCriticalSection(&DioCriticalSection);
		memcpy(pDioStates, SIMU.eOutputsState, dwMaxSize);
		LeaveCriticalSection(&DioCriticalSection);
	}	
}

/*******************************************************************/
/*SYNTAX: BOOL DIOWorksAsSimulator(void)						   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED BOOL DIOWorksAsSimulator(void)
{
	BOOL bUseSimulator = FALSE;
	
	EnterCriticalSection(&DioCriticalSection);
	bUseSimulator = GLOBAL.bUseSimulator;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bUseSimulator;
}

/*******************************************************************/
/*SYNTAX: DWORD GetNumberOfPlugins(void)						   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetNumberOfPlugins(void)
{
	DWORD dwNbPlugins = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbPlugins = GLOBAL.dwNbPlugins;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbPlugins;
}

/*******************************************************************/
/*SYNTAX: DWORD GetExecThreadId(void)							   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetExecThreadId(void)
{
	DWORD dwThreadId;
	
	EnterCriticalSection(&DioCriticalSection);
	dwThreadId = GLOBAL.dwExecThreadId;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwThreadId;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOFirstInputIdx(void)						   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOFirstInputIdx(void)
{
	DWORD dwFirstInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstInputIdx = GLOBAL.dwFirstInputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOLastInputIdx(void)							   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOLastInputIdx(void)
{
	DWORD dwLastInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastInputIdx = GLOBAL.dwLastInputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIONbInputs(void)								   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIONbInputs(void)
{
	DWORD dwNbInputs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbInputs = GLOBAL.dwNbInputs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNbInputs;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOFirstOutputIdx(void)						   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOFirstOutputIdx(void)
{
	DWORD dwFirstOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstOutputIdx = GLOBAL.dwFirstOutputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOLastOutputIdx(void)						   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOLastOutputIdx(void)
{
	DWORD dwLastOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastOutputIdx = GLOBAL.dwLastOutputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIONbOutputs(void)							   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIONbOutputs(void)
{
	DWORD dwNbOutputs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbOutputs = GLOBAL.dwNbOutputs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNbOutputs;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginFirstInputIdx(DWORD dwPluginIdx)		   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginFirstInputIdx(DWORD dwPluginIdx)
{
	DWORD dwFirstInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstInputIdx = GLOBAL.sPluginDetails[dwPluginIdx].dwFirstInputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginLastInputIdx(DWORD dwPluginIdx)		   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginLastInputIdx(DWORD dwPluginIdx)
{
	DWORD dwLastInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastInputIdx = GLOBAL.sPluginDetails[dwPluginIdx].dwLastInputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginNbInputs(DWORD dwPluginIdx)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginNbInputs(DWORD dwPluginIdx)
{
	DWORD dwNbInputs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbInputs =  GLOBAL.sPluginDetails[dwPluginIdx].dwLastInputIndex -
		GLOBAL.sPluginDetails[dwPluginIdx].dwFirstInputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNbInputs;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginFirstOutputIdx(DWORD dwPluginIdx)	   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginFirstOutputIdx(DWORD dwPluginIdx)
{
	DWORD dwFirstOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstOutputIdx = GLOBAL.sPluginDetails[dwPluginIdx].dwFirstOutputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginLastOutputIdx(DWORD dwPluginIdx)		   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginLastOutputIdx(DWORD dwPluginIdx)
{
	DWORD dwLastOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastOutputIdx = GLOBAL.sPluginDetails[dwPluginIdx].dwLastOutputIndex;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOPluginNbOutputs(DWORD dwPluginIdx)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED DWORD GetIOPluginNbOutputs(DWORD dwPluginIdx)
{
	DWORD dwNbOutputs = 0;

	EnterCriticalSection(&DioCriticalSection);
	dwNbOutputs = GLOBAL.sPluginDetails[dwPluginIdx].dwLastOutputIndex -
		GLOBAL.sPluginDetails[dwPluginIdx].dwFirstOutputIndex;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbOutputs;
}

/*******************************************************************/
/*SYNTAX: BOOL IsCallbackFncInUse(void)							   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED BOOL IsCallbackFncInUse(void)
{
	BOOL bFncInUse = FALSE;
	
	EnterCriticalSection(&DioCriticalSection);
	if(GLOBAL.pCallbackFnc != NULL)
		bFncInUse = TRUE;
	else
		bFncInUse = FALSE;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bFncInUse;
}

/*******************************************************************/
/*SYNTAX: HINSTANCE GetPluginHandle(DWORD dwPluginIdx)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*														           */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PROTECTED HINSTANCE GetPluginHandle(DWORD dwPluginIdx)
{
	HINSTANCE hPlugin;
	
	EnterCriticalSection(&DioCriticalSection);
	hPlugin = GLOBAL.sPluginDetails[dwPluginIdx].hDioLib;
	LeaveCriticalSection(&DioCriticalSection);
	
	return hPlugin;
}

/*-------------------------------- END OF FILE ------------------------------*/