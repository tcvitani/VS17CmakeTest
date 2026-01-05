/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 dio_pcl1750													 */
/* FILE:	 dio_pci1750.c													 */
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
#include <trc.h>
#include <bdaqctrl.h>

#define PCI_DEF
#include <dio_pci1750.h>
#undef PCI_DEF

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					MOD_REG_KEYn_MODULES

#define DBG_NAME					"DIO_PCI1750"

#define REGKEY_INPUT_POSITION		"InputPosition"
#define REGKEY_DI_FILTER			"Filter"
#define REGKEY_POLLING_DELAY		"PollingDelayMs"
#define REGKEY_INVERT_INPUTS		"InvertInputs"
#define REGKEY_BOARD_DESC			"BoardDescription"
#define REGKEY_DEVICE_NUMBER		"DeviceNumber"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE BOOL DIO_InitTraces(struct_debug *dbg);
PRIVATE void DIO_Error(IN struct_debug *dbg, IN char *fmt, ...);
PRIVATE void DIO_Trace(IN struct_debug *dbg, IN char *fmt, ...);

PRIVATE DIO_ERROR RegistryParametres(IN HKEY hKey, IN CHAR *szRoot);
PRIVATE void SendPluginConfig(void);

PRIVATE void GetDioCardEnumTxt(	IN		ErrorCode	iErrorEnum,
								IN	OUT	CHAR		*pszEnumText,
								IN		size_t		enumTextSize);

/*-------------------------------- VARIABLES: -------------------------------*/

PUBLIC CRITICAL_SECTION DioCriticalSection = {0};

struct_dio		DIO = {0};
struct_debug	DBG = {0};

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
	switch (dwReason) 
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
/*SYNTAX: DIO_ERROR PDioInit(IN HKEY hKey,						   */
/*						IN CHAR *szRoot,						   */
/*						IN HINSTANCE hPlugin					   */
/*						IN DioStatusOccuredEvent fncStatusOccured  */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Initialization of DIO pci1750 module.	               */
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
PUBLIC DIO_ERROR WINAPI PDioInit(IN HKEY hKey, 
								 IN CHAR *szRoot,
								 IN HINSTANCE hPlugin,
								 IN PDioStatusOccuredEvent fncStatusOccured)
{

	DWORD		eRetTemp		= 0;
	DWORD		carte	= 0;
	DIO_ERROR	eRet	= DIO_SUCCESS;
	DWORD		dwIdx	= 0;

	DIO_InitTraces(&DBG);

	eRet = RegistryParametres(hKey, szRoot);

	for (dwIdx = 0; dwIdx < DIO.dwNbBoards; dwIdx++)
		DIO.sBoardDetails[dwIdx].bLinkError = TRUE;

	DIO.pCallbackFnc = fncStatusOccured;
	DIO.hPlugin = hPlugin;

	DIO_Trace(&DBG, "PDioInit(): init success!");
    
	InitializeCriticalSection(&DioCriticalSection);

	if (DIO.pCallbackFnc != NULL)
		DIO.hExec = CreateThread(NULL, 0, DIO_ExecThread, NULL, 0, &DIO.dwExecThreadId);

	DIO.hPoll = CreateThread(NULL, 0, DIO_PollThread, NULL, 0, NULL);
	
	SetInitDone();

	return eRet;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR PDioRelease(void)							   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Deinitialization of DIO pci1750 module.			   */
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
PUBLIC DIO_ERROR WINAPI PDioRelease(void)
{
	if(DIO.hExec != NULL)
		TerminateThread(DIO.hExec, 0);
	
	if(DIO.hPoll != NULL)
		TerminateThread(DIO.hPoll, 0);

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
PRIVATE DIO_ERROR RegistryParametres(IN HKEY hKey, IN CHAR *szRoot)
{
    DWORD	Idx						= 0;
	DWORD	Idx2					= 0;
	DWORD	dwSize					= 0;
	DWORD	dwSize2					= 0;
	DWORD	dwTmp					= 0;
	DWORD	dwInputPosition			= 0;
	CHAR	*pCh					= NULL;
	CHAR	szMsg[MAX_PATH]			= {0};
	CHAR	szSubKey[MAX_PATH]		= {0};
	CHAR	szSubKey2[MAX_PATH]		= {0};
	CHAR	szTempKey[MAX_PATH]		= {0};
	CHAR	szTempKey2[MAX_PATH]	= {0};
	CHAR	szSettingsKey[MAX_PATH]	= {0};
	CHAR	szExtendedKey[MAX_PATH]	= {0};
	HKEY	hRootKey				= CSR_REG_KEYi_ROOT;
	CHAR	szDioKey[MAX_PATH]		= {0};
	
	if(hKey != NULL)
		hRootKey = hKey;
	
	if(szRoot != NULL)
	{
		strncpy_s(szDioKey, sizeof(szDioKey), szRoot, sizeof(szDioKey));
		strcat_s(szDioKey, sizeof(szDioKey), DIO_PCI1750_REG_KEYn);
	}
	else
	{
		sprintf_s(szDioKey,
				_countof(szDioKey),
				"%s%s%s%s", 
				KEY_MODULE, 
				DIO_REG_KEYn_IO_ROOT, 
				DIO_REG_KEYn_PLUGINS, 
				DIO_PCI1750_REG_KEYn);
	}
	
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
						szDioKey, 
						REGKEY_POLLING_DELAY, 
						&DIO.dwPollingDelayMs) != ERROR_SUCCESS)
	{
		DIO_Error(&DBG, 
				"Incorrect or missing registry value: %s => %s !", 
				szSubKey, 
				REGKEY_POLLING_DELAY);
		
		return DIO_REG_KEY_MISSING;
	}

	dwSize = sizeof(szTempKey);
	while(REG_Enum_Cles(hRootKey, szDioKey, Idx, szTempKey, &dwSize) == ERROR_SUCCESS)
	{			
		// SubKeys should be in following order: "DioBoard0", "DioBoard1", "DioBoard2", etc...
		pCh = strstr(szTempKey, DIO_REGKEY_DIO_BOARD);
		if (pCh == NULL || (unsigned)atoi(pCh + strlen(DIO_REGKEY_DIO_BOARD)) != Idx)
		{
			DIO_Error (&DBG, "Incorrect registry key: %s !", szTempKey);

			return DIO_INVALID_REG_CONFIG;
		}

		sprintf_s(szSubKey, _countof(szSubKey), "%s%s\\", szDioKey, szTempKey);
		
		// reading DioBoardX number of INPUTS
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							DIO_REGKEY_NB_OF_INPUTS, 
							&DIO.sBoardDetails[Idx].dwNbInputs) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					DIO_REGKEY_NB_OF_INPUTS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX number of OUTPUTS
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							DIO_REGKEY_NB_OF_OUTPUTS, 
							&DIO.sBoardDetails[Idx].dwNbOutputs) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					DIO_REGKEY_NB_OF_OUTPUTS);
			
			return DIO_REG_KEY_MISSING;
		} 

		// reading DioBoardX first input index
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey,
							DIO_REGKEY_FIRST_INPUT_IDX, 
							&DIO.sBoardDetails[Idx].dw1stInputIdx) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					DIO_REGKEY_FIRST_INPUT_IDX);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX first output index
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							DIO_REGKEY_FIRST_OUTPUT_IDX, 
							&DIO.sBoardDetails[Idx].dw1stOutputIdx) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					DIO_REGKEY_FIRST_OUTPUT_IDX);
			
			return DIO_REG_KEY_MISSING;
		}

		// should inputs be inverted for DioBoardX
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_INVERT_INPUTS, 
							&dwTmp) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					REGKEY_INVERT_INPUTS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX board description
		dwSize = sizeof(DIO.sBoardDetails[Idx].szBoardDescription);
		if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT,
								szSubKey,
								REGKEY_BOARD_DESC,
								DIO.sBoardDetails[Idx].szBoardDescription,
								&dwSize) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG,
					"Incorrect or missing registry value: %s => %s !",
					szSubKey,
					REGKEY_BOARD_DESC);

			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX device number
		if (REG_Lire_Entier(CSR_REG_KEYi_ROOT,
								szSubKey,
								REGKEY_DEVICE_NUMBER,
								&DIO.sBoardDetails[Idx].dwDeviceNumber) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG,
					"Incorrect or missing registry value: %s => %s !",
					szSubKey,
					REGKEY_DEVICE_NUMBER);

			return DIO_REG_KEY_MISSING;
		}

		if(dwTmp != 0)
			DIO.sBoardDetails[Idx].bInvertInputs = TRUE;
		else
			DIO.sBoardDetails[Idx].bInvertInputs = FALSE;

		// counting total number of inputs and outputs
		DIO.dwNbInputs += DIO.sBoardDetails[Idx].dwNbInputs;
		DIO.dwNbOutputs += DIO.sBoardDetails[Idx].dwNbOutputs;
		
		dwSize = sizeof(szTempKey);
		Idx++;	
	}
	
	DIO.dwNbBoards = Idx;

	DIO_Trace(&DBG, 
			"Total: inputs=%d, outputs=%d, DIO boards=%d", 
			DIO.dwNbInputs, 
			DIO.dwNbOutputs, 
			DIO.dwNbBoards);

	return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_STATE WINAPI PDioGetInputState(IN DWORD dwInputIndex,*/
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
PUBLIC DIO_STATE WINAPI PDioGetInputState(IN DWORD dwInputIndex, 
										  OUT DIO_ERROR *pDioErr)
{
	DWORD		dwInput			= 0;
	DWORD		dwIdx			= 0;
	DWORD		dwBoardIdx		= 0;
	DWORD		dwNbBoards		= 0;
	DWORD		dwFirstInputIdx = 0;
	DWORD		dwLastInputIdx	= 0;
 	DIO_STATE	eInputState		= 0;	
	DIO_ERROR	eDioErr			= DIO_SUCCESS;
	BOOL		bFound			= FALSE;

	dwNbBoards = GetNumberOfIOBoards();

	// search for the card that handles requested input
	// if we have more than one card with inputs (eg: Carte0 has 16 inputs (0-15),
	// Carte1 16 outputs (0-15) and Carte2 32 inputs (0-31) and 32 outputs (0-31), 
	// in this case input 20 has to be on Carte2, input 3
	// eg input 10 => Carte0 - input 10
    for (dwIdx = 0; dwIdx < dwNbBoards; dwIdx++)
    {
		dwFirstInputIdx = GetIOBoardFirstInputIdx(dwIdx);
		dwLastInputIdx = GetIOBoardLastInputIdx(dwIdx);

		if (dwInputIndex >= dwFirstInputIdx && dwInputIndex <= dwLastInputIdx)
		{
			dwInput = dwInputIndex - dwFirstInputIdx;
			dwBoardIdx = dwIdx;

			bFound = TRUE;
			break;
		}	
	}
	
	if (bFound)
	{		
		eInputState = GetInputStatus(dwBoardIdx, dwInput);
		
		eDioErr = DIO_SUCCESS;
	}
	else
	{
		DIO_Error(&DBG, 
			"PDioGetInputState( %d ) -> DIO board that should handle this input has not founded!", 
			dwInputIndex);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;	
	}	
	
	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
    return eInputState;
}

/**/
/*******************************************************************/
/*SYNTAX DIO_ERROR PDioSetOutputState(IN DWORD dwOutputIndex,	   */ 
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
EXPORT DIO_ERROR WINAPI PDioSetOutputState(IN DWORD dwOutputIndex, 
										   IN DIO_STATE eState)
{
	ErrorCode	eRetTemp				= Success;
	DWORD		dwOutput				= 0;
	DWORD		dwIdx					= 0;
	DWORD		dwBoardIdx				= 0;
	DWORD		dwNbBoards				= 0;
	DWORD		dwFirstOutpuIdx			= 0;
	DWORD		dwLastOutputIdx			= 0;
 	DIO_ERROR	eDioErr					= DIO_SUCCESS;
	BOOL		bFound					= FALSE;
	CHAR		szErrorText[MAX_PATH]	= { 0 };
	byte		bReadFromBoard			= -1;
	int			iPort					= 0;
	int			iBit					= 0;

	dwNbBoards = GetNumberOfIOBoards();

	// search for the card that handles requested output
	// if we have more than one card with outputs (eg: Carte0 has 16 inputs (0-15),
	// Carte1 16 outputs (0-15) and Carte2 32 inputs (0-31) and 32 outputs (0-31), 
	// in this case output 16 has to be on Carte2, output 0
	// eg output 5 => Carte1 - output 5
    for(dwIdx = 0; dwIdx < dwNbBoards; dwIdx++)
    {	
		dwFirstOutpuIdx = GetIOBoardFirstOutputIdx(dwIdx);
		dwLastOutputIdx = GetIOBoardLastOutputIdx(dwIdx);

		if(dwOutputIndex >= dwFirstOutpuIdx && dwOutputIndex <= dwLastOutputIdx)
		{
			dwOutput = dwOutputIndex - dwFirstOutpuIdx;
			dwBoardIdx = dwIdx;
			
 			bFound = TRUE;				
			break;
		}
	}
	
	// Determine bit and port depending of dwOutput
	iPort = dwOutput / 8;
	iBit = dwOutput % 8;
	
	if(bFound)
	{
		// Step 1: create 'InstantDoCtrl' object
		InstantDoCtrl *instantDoCtrl = InstantDoCtrl_Create();

		// Step 2: select the device with 'ModeWrite' mode
		DeviceInformation devInfo;
		devInfo.DeviceNumber = DIO.sBoardDetails[dwBoardIdx].dwDeviceNumber;
		devInfo.DeviceMode = ModeWrite;
		devInfo.ModuleIndex = 0;

		size_t sizeReturnValue = 0;
		mbstowcs_s(	&sizeReturnValue, devInfo.Description,
					sizeof(devInfo.Description) / sizeof(WORD),
					DIO.sBoardDetails[dwBoardIdx].szBoardDescription,
					_countof(DIO.sBoardDetails[dwBoardIdx].szBoardDescription));

		// check if board description is copied properly
		if (sizeReturnValue != (strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1))
		{
			DIO_Error(&DBG, "PDioSetOutputState : error while copying BoardDescription to devInfo.Description! mbstowcs_s copied %d characters to devInfo.Description but BoardDescription id %d characters long (including null termination char)",
						sizeReturnValue,
						strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1);
			eDioErr = DIO_OTHER_ERROR;
		}

		EnterCriticalSection(&DioCriticalSection);

		eRetTemp = InstantDoCtrl_setSelectedDevice(instantDoCtrl, &devInfo);
		if (eRetTemp != Success)
		{
			GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
			DIO_Error(&DBG, "PDioSetOutputState : InstantDoCtrl_setSelectedDevice failed!, dwBoardIdx = %d, dwOutput = %d, iPort = %d, iBit = %d, returned: %s", dwBoardIdx, dwOutput, iPort, iBit, szErrorText);
			eDioErr = DIO_PLUGIN_API_ERROR;
		}
		else
		{
			// to do - check casting from eState do uint8
			uint8 temp = (uint8)eState;
			eRetTemp = InstantDoCtrl_WriteBit(instantDoCtrl, iPort, iBit, temp);
			if (eRetTemp == Success)
			{
				eDioErr = DIO_SUCCESS;
			}
			else
			{
				GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
				DIO_Error(&DBG, "PDioSetOutputState : InstantDoCtrl_WriteBit failed!, dwBoardIdx = %d, dwOutput = %d, iPort = %d, iBit = %d, returned: %s", dwBoardIdx, dwOutput, iPort, iBit, szErrorText);
				eDioErr = DIO_PLUGIN_API_ERROR;
			}
		}
		// Step 4: close device and release any allocated resource.
		InstantDoCtrl_Cleanup(instantDoCtrl);

		// Step 5: destroy the object
		InstantDoCtrl_Dispose(instantDoCtrl);
		
		LeaveCriticalSection(&DioCriticalSection);
	}
	else
	{
		DIO_Error(&DBG, 
			"PDioSetOutputState( %d, %d ) -> DIO board that should handle this output has not founded!", 
			dwOutput,
			eState);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;
	}

    return eDioErr;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR PDioGetOutputState(IN DWORD dwOutputIndex,	   */
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
EXPORT DIO_STATE WINAPI PDioGetOutputState(IN DWORD dwOutputIndex, 
										   OUT DIO_ERROR *pDioErr)
{
    DWORD		dwOutput		= 0;
	DWORD		dwIdx			= 0;
	DWORD		dwBoardIdx		= 0;
	DWORD		dwNbBoards		= 0;
	DWORD		dwFirstOutpuIdx = 0;
	DWORD		dwLastOutputIdx	= 0;
	DIO_STATE	eOutputState	= 0;
	DIO_ERROR	eDioErr			= DIO_SUCCESS;	
	BOOL		bFound			= FALSE;

	dwNbBoards = GetNumberOfIOBoards();

    for(dwIdx = 0; dwIdx < dwNbBoards; dwIdx++)
    {
		dwFirstOutpuIdx = GetIOBoardFirstOutputIdx(dwIdx);
		dwLastOutputIdx = GetIOBoardLastOutputIdx(dwIdx);

		if(dwOutputIndex >= dwFirstOutpuIdx && dwOutputIndex <= dwLastOutputIdx)
		{
			dwOutput = dwOutputIndex - dwFirstOutpuIdx;
			dwBoardIdx = dwIdx;
			
			bFound = TRUE;
			break;
		}	
	}
	
	if (bFound)
	{		
		eOutputState = GetOutputStatus(dwBoardIdx, dwOutput);
		
		eDioErr = DIO_SUCCESS;
	}
	else
	{
		DIO_Error(&DBG, 
				"PDioGetOutputState( %d ) -> DIO board that should handle this output has not founded!", 
				dwOutputIndex);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;	
	}	

	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
    return eOutputState;
}

/**/
/*******************************************************************/
/*SYNTAX DIO_ERROR PDioGetRange(OUT DWORD *pdwFirstInputIndex,	   */
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
EXPORT DIO_ERROR WINAPI PDioGetRange(OUT DWORD *pdwFirstInputIndex,
									 OUT DWORD *pdwLastInputIndex,
									 OUT DWORD *pdwFirstOutputIndex,
									 OUT DWORD *pdwLastOutputIndex)
{
	DWORD i;
	DWORD j;
	DWORD dwFirstInputIndex		= 0;
	DWORD dwLastInputIndex		= 0;
	DWORD dwFirstOutputIndex	= 0;
	DWORD dwLastOutputIndex		= 0;

	dwFirstInputIndex = DIO_NB_BOARDS_MAX * DIO_NB_INPUTS_MAX;
	dwFirstOutputIndex = DIO_NB_BOARDS_MAX * DIO_NB_OUTPUTS_MAX;

    for(i = 0; i < DIO.dwNbBoards; i++)
	{
		for(j = i+1; j < DIO.dwNbBoards; j++)
		{
			if(GetIOBoardFirstInputIdx(i) == GetIOBoardFirstInputIdx(j) || 
				GetIOBoardFirstOutputIdx(i) == GetIOBoardFirstOutputIdx(j))
			{
				DIO_Error(&DBG, 
					"PDioGetRange() -> 1st I/O index of DioBoard%d is equal to the 1sr I/O index of DioBoard%d!", 
					i, 
					j);
				
				return DIO_INVALID_REG_CONFIG;
			}
		}
		
		// first and last input index
		if(dwFirstInputIndex > GetIOBoardFirstInputIdx(i))
			dwFirstInputIndex = GetIOBoardFirstInputIdx(i);
		
		if(dwLastInputIndex < GetIOBoardLastInputIdx(i))
			dwLastInputIndex = GetIOBoardLastInputIdx(i);
		
		// first and last output index
		if(dwFirstOutputIndex > GetIOBoardFirstOutputIdx(i))
			dwFirstOutputIndex = GetIOBoardFirstOutputIdx(i);
		
		if(dwLastOutputIndex < GetIOBoardLastOutputIdx(i))
			dwLastOutputIndex = GetIOBoardLastOutputIdx(i);	
	}
	
	if(pdwFirstInputIndex != NULL)
		*pdwFirstInputIndex = dwFirstInputIndex;

	if(pdwLastInputIndex != NULL)
		*pdwLastInputIndex = dwLastInputIndex;

	if(pdwFirstOutputIndex != NULL)
		*pdwFirstOutputIndex = dwFirstOutputIndex;

	if(pdwLastOutputIndex != NULL)
		*pdwLastOutputIndex = dwLastOutputIndex;
	
    return DIO_SUCCESS;
}

/**/
/*******************************************************************/
/*SYNTAX: BOOL DIO_InitTraces (struct_debug *dbg)				   */
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
PRIVATE BOOL DIO_InitTraces(struct_debug *dbg)
{
    DWORD	dwLen					= 0;
    DWORD	dwTailleMax				= 1400000L;
    CHAR	pcKey[MAX_PATH]			= {0};
    CHAR	rep_debug[MAX_PATH]		= {0};
    CHAR	rep_traces[MAX_PATH]	= {0};
    CHAR	debug[MAX_PATH]			= {0};
    CHAR	traces[MAX_PATH]		= {0};
    
    // chemin de base dans le registre
    sprintf_s(pcKey, _countof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);
    
    // Taille max des fichiers    
    REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax);
    
    // chemin erreurs 
    dwLen = sizeof(rep_debug);
    REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, rep_debug, &dwLen);
    
    // creation du chemin complet vers le fichier de debug 
    _snprintf_s(debug, sizeof(debug), MAX_PATH, "%s\\%s.ERR", rep_debug, DBG_NAME);
    
    // Init du fichier de debug
    TRC_Initialise_Trace("DEBUG", debug, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION|TRC_OPT_TEXTE_SEUL|TRC_OPT_CONSOLE, (TRC_EMETTEUR *) &dbg->debug);
    TRC_Taille_Max_Fichier(dbg->debug, dwTailleMax);
    
    // chemin traces
    dwLen = sizeof(rep_traces);
    REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, rep_traces, &dwLen);
    
    // creation du chemin complet vers le fichier de traces
    _snprintf_s(traces, sizeof(traces), MAX_PATH, "%s\\%s.TRC", rep_traces, DBG_NAME);
    
    // Init du fichier de traces
    TRC_Initialise_Trace("TRACE", traces, TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *) &dbg->traces);
    TRC_Taille_Max_Fichier(dbg->traces, dwTailleMax);
    
    return TRUE;
}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Error (struct_debug *dbg, IN char *fmt, ...)    */
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
PRIVATE void DIO_Error(struct_debug *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start (ap,fmt);

    // dans le fichier de debug
    if (dbg->debug != NULL)
        TRC_Direct_Trace_V(dbg->debug, TRC_OPT_CREER_FICHIER|TRC_OPT_FICHIER|TRC_OPT_NUMEROTATION, NULL, 0, fmt, ap);

    // et dans le fichier de traces
    if (dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);

    va_end(ap);
}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Trace(struct_debug, IN char *fmt, ...)		   */
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
PRIVATE void DIO_Trace (struct_debug *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start(ap,fmt);

    if(dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);
	
    va_end(ap);
}

/*******************************************************************/
/*SYNTAX: void SendPluginConfig(void)							   */
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
PRIVATE void SendPluginConfig(void)
{
	struct_dio_plugin_config	sConfig = {0};
	DWORD						dwIdx	= 0;

	EnterCriticalSection(&DioCriticalSection);
	sConfig.dwNbBoards = DIO.dwNbBoards;

	for (dwIdx = 0; dwIdx < DIO.dwNbBoards; dwIdx++)
	{
		sConfig.sBoardConfig[dwIdx].dwNbInputs = DIO.sBoardDetails[dwIdx].dwNbInputs;
		sConfig.sBoardConfig[dwIdx].dwNbOutputs = DIO.sBoardDetails[dwIdx].dwNbOutputs;
		sConfig.sBoardConfig[dwIdx].dwFirstInputIdx = DIO.sBoardDetails[dwIdx].dw1stInputIdx;
		sConfig.sBoardConfig[dwIdx].dwFirstOutputIdx = DIO.sBoardDetails[dwIdx].dw1stOutputIdx;
	}
	LeaveCriticalSection(&DioCriticalSection);

	DIO.pCallbackFnc(DIO_PLUGIN_CONFIG_EVT, &sConfig, GetPluginHandle());
}

/*******************************************************************/
/*SYNTAX: void CallbackFnc_SendFullSet(DWORD dwBoardIdx)		   */
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
PRIVATE void CallbackFnc_SendFullSet(DWORD dwBoardIdx)
{
	DWORD				dwNbIO								= 0;
	DWORD				dwFirstIO							= 0;
	DWORD				dwIdx								= 0;
	BYTE				bIntputStatus[DIO_NB_INPUTS_MAX]	= {0};
	BYTE				bOutputStatus[DIO_NB_INPUTS_MAX]	= {0};
	HINSTANCE			hPlugin;
	struct_dio_status	sDioStatus							= {0};
	
	// Send whole status of inputs for requested DIO board, output by output
	ZeroMemory(bIntputStatus, sizeof(bIntputStatus));

	hPlugin = GetPluginHandle();

	// Grab current card state
	EnterCriticalSection(&DioCriticalSection);
	memcpy(bIntputStatus, DIO.sBoardDetails[dwBoardIdx].bIntputStatus, sizeof(bIntputStatus));
	LeaveCriticalSection(&DioCriticalSection);

	dwNbIO = GetIOBoardNbInputs(dwBoardIdx);
	dwFirstIO = GetIOBoardFirstInputIdx(dwBoardIdx);

	for (dwIdx = 0; dwIdx < dwNbIO; dwIdx++)
	{
		sDioStatus.dwDioIdx = dwFirstIO + dwIdx;
		sDioStatus.eState = bIntputStatus[dwIdx];
		
		DIO.pCallbackFnc(DIO_DI_STATUS_EVT, &sDioStatus, hPlugin);
	}

	// Send whole status of outputs, output by output
	ZeroMemory(bOutputStatus, sizeof(bOutputStatus));
	
	// Grab current card state
	EnterCriticalSection(&DioCriticalSection);
	memcpy(bOutputStatus, DIO.sBoardDetails[dwBoardIdx].bOutputStatus, sizeof(bOutputStatus));
	LeaveCriticalSection(&DioCriticalSection);
	
	dwNbIO = GetIOBoardNbOutputs(dwBoardIdx);
	dwFirstIO = GetIOBoardFirstOutputIdx(dwBoardIdx);
	
	for (dwIdx = 0; dwIdx < dwNbIO; dwIdx++)
	{
		sDioStatus.dwDioIdx = dwFirstIO + dwIdx;
		sDioStatus.eState = bOutputStatus[dwIdx];
		
		DIO.pCallbackFnc(DIO_DO_STATUS_EVT, &sDioStatus, hPlugin);
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
PROTECTED void ExecuteCallbackFnc(enum_pdio_exec_msg eMsg,
								  WPARAM wParam,
								  LPARAM lParam)
{
	struct_dio_status			sDioStatus	= {0};
	struct_dio_board_status		sDevStatus	= {0};

	switch (eMsg)
	{
		case PDIO_EXEC_SEND_FULL_SET:
			CallbackFnc_SendFullSet((DWORD)wParam);
			break;
	
		case PDIO_EXEC_INPUT_STATE:
			sDioStatus.dwDioIdx = (DWORD)wParam;
			sDioStatus.eState = (DIO_STATE)lParam;

			DIO.pCallbackFnc(DIO_DI_STATUS_EVT, &sDioStatus, GetPluginHandle());
			break;
	
		case PDIO_EXEC_OUTPUT_STATE:
			sDioStatus.dwDioIdx = (DWORD)wParam;
			sDioStatus.eState = (DIO_STATE)lParam;

			DIO.pCallbackFnc(DIO_DO_STATUS_EVT, &sDioStatus, GetPluginHandle());
			break;

		case PDIO_EXEC_DEVICE_STATUS:
			sDevStatus.dwBoardIdx = (DWORD)wParam;
			sDevStatus.bLinkError = (BOOL)lParam;

			DIO.pCallbackFnc(DIO_DEVICE_STATUS_EVT, &sDevStatus, GetPluginHandle());
			break;

		case PDIO_EXEC_PLUGIN_CONFIG:
			SendPluginConfig();
			break;

		default:
			DIO_Error(&DBG, 
				"ExecuteCallbackFnc(): received unhandled message type %d",
				eMsg);
			break;
	}
}

/*******************************************************************/
/*SYNTAX: DWORD GetNumberOfIOBoards(void)						   */
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
PROTECTED DWORD GetNumberOfIOBoards(void)
{
	DWORD dwNbBoards = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbBoards = DIO.dwNbBoards;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbBoards;
}

/*******************************************************************/
/*SYNTAX: DWORD GetPollingDelayMs(void)							   */
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
PROTECTED DWORD GetPollingDelayMs(void)
{
	DWORD dwPollingDelayMs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwPollingDelayMs = DIO.dwPollingDelayMs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwPollingDelayMs;
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
	dwThreadId = DIO.dwExecThreadId;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwThreadId;
}

/*******************************************************************/
/*SYNTAX: void SetIOBoardConnectionError(DWORD dwBoardIdx)		   */
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
PROTECTED void SetIOBoardConnectionError(DWORD dwBoardIdx)
{
	BOOL			bLinkError_Old = FALSE;
	BOOL			bLinkError = FALSE;

	EnterCriticalSection(&DioCriticalSection);
	bLinkError_Old = DIO.sBoardDetails[dwBoardIdx].bLinkError;
	DIO.sBoardDetails[dwBoardIdx].bLinkError = TRUE;

	if (!bLinkError_Old || !IsInitDone())
	{
		bLinkError = TRUE;
		PostThreadMessage(GetExecThreadId(), PDIO_EXEC_DEVICE_STATUS, dwBoardIdx, bLinkError);
	}
	LeaveCriticalSection(&DioCriticalSection);
}

/*******************************************************************/
/*SYNTAX: void ResetIOBoardConnectionError(DWORD dwBoardIdx)	   */
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
PROTECTED void ResetIOBoardConnectionError(DWORD dwBoardIdx)
{
	BOOL bLinkError_Old = FALSE;
	BOOL bLinkError = FALSE;

	EnterCriticalSection(&DioCriticalSection);
	bLinkError_Old = DIO.sBoardDetails[dwBoardIdx].bLinkError;
	DIO.sBoardDetails[dwBoardIdx].bLinkError = FALSE;

	if (bLinkError_Old)
	{
		bLinkError = FALSE;
		PostThreadMessage(GetExecThreadId(), PDIO_EXEC_DEVICE_STATUS, dwBoardIdx, bLinkError);
	}
	LeaveCriticalSection(&DioCriticalSection);
}

/*******************************************************************/
/*SYNTAX: BOOL IOBoardConnectionOK(DWORD dwBoardIdx)			   */
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
PROTECTED BOOL IOBoardConnectionOK(DWORD dwBoardIdx)
{
	BOOL bLinkError = FALSE;

	EnterCriticalSection(&DioCriticalSection);
	bLinkError = DIO.sBoardDetails[dwBoardIdx].bLinkError;
	LeaveCriticalSection(&DioCriticalSection);

	if (bLinkError)
		return FALSE;

	return TRUE;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx)		   */
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
PROTECTED DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx)
{
	DWORD dwFirstInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstInputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stInputIdx;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx)		   */
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
PROTECTED DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx)
{
	DWORD dwLastInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastInputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stInputIdx + 
		(DIO.sBoardDetails[dwBoardIdx].dwNbInputs - 1);
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastInputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardNbInputs(DWORD dwBoardIdx)			   */
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
PROTECTED DWORD GetIOBoardNbInputs(DWORD dwBoardIdx)
{
	DWORD dwNbInputs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbInputs =  DIO.sBoardDetails[dwBoardIdx].dwNbInputs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNbInputs;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx)		   */
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
PROTECTED DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx)
{
	DWORD dwFirstOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstOutputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stOutputIdx;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx)		   */
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
PROTECTED DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx)
{
	DWORD dwLastOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastOutputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stOutputIdx + 
		(DIO.sBoardDetails[dwBoardIdx].dwNbOutputs - 1);
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastOutputIdx;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx)			   */
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
PROTECTED DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx)
{
	DWORD dwNbOutputs = 0;

	EnterCriticalSection(&DioCriticalSection);
	dwNbOutputs = DIO.sBoardDetails[dwBoardIdx].dwNbOutputs;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbOutputs;
}

/*******************************************************************/
/*SYNTAX: void SetInputStatus(DWORD dwBoardIdx,					   */
/*								DWORD dwInput,					   */
/*								DIO_STATE eDioState)			   */
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
PROTECTED void SetInputStatus(DWORD dwBoardIdx,
							  DWORD dwInput,
							  DIO_STATE eDioState)
{
	EnterCriticalSection(&DioCriticalSection);
	DIO.sBoardDetails[dwBoardIdx].bIntputStatus[dwInput] = eDioState;
	LeaveCriticalSection(&DioCriticalSection);
}

/*******************************************************************/
/*SYNTAX: DIO_STATE GetInputStatus(DWORD dwBoardIdx,			   */
/*									DWORD dwInput)				   */
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
PROTECTED DIO_STATE GetInputStatus(DWORD dwBoardIdx,
								   DWORD dwInput)
{
	DIO_STATE eInputStatus;

	EnterCriticalSection(&DioCriticalSection);
	eInputStatus = DIO.sBoardDetails[dwBoardIdx].bIntputStatus[dwInput];
	LeaveCriticalSection(&DioCriticalSection);

	return eInputStatus;
}

/*******************************************************************/
/*SYNTAX: void SetOutputStatus(DWORD dwBoardIdx,				   */
/*								DWORD dwOutput,					   */
/*								DIO_STATE eDioState)			   */
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
PROTECTED void SetOutputStatus(DWORD dwBoardIdx,
							   DWORD dwOutput,
							   DIO_STATE eDioState)
{
	EnterCriticalSection(&DioCriticalSection);
	DIO.sBoardDetails[dwBoardIdx].bOutputStatus[dwOutput] = eDioState;
	LeaveCriticalSection(&DioCriticalSection);
}

/*******************************************************************/
/*SYNTAX: DIO_STATE GetOutputStatus(DWORD dwBoardIdx,			   */
/*									DWORD dwOutput)				   */
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
PROTECTED DIO_STATE GetOutputStatus(DWORD dwBoardIdx,
									DWORD dwOutput)
{
	DIO_STATE eOutputStatus;

	EnterCriticalSection(&DioCriticalSection);
	eOutputStatus = DIO.sBoardDetails[dwBoardIdx].bOutputStatus[dwOutput];
	LeaveCriticalSection(&DioCriticalSection);

	return eOutputStatus;
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
	if(DIO.pCallbackFnc != NULL)
		bFncInUse = TRUE;
	else
		bFncInUse = FALSE;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bFncInUse;
}

/*******************************************************************/
/*SYNTAX: HINSTANCE GetPluginHandle(void)						   */
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
PROTECTED HINSTANCE GetPluginHandle(void)
{
	HINSTANCE hPlugin;
	
	EnterCriticalSection(&DioCriticalSection);
	hPlugin = DIO.hPlugin;
	LeaveCriticalSection(&DioCriticalSection);
	
	return hPlugin;
}

/*******************************************************************/
/*SYNTAX: BOOL IsInitDone(void)									   */
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
PROTECTED BOOL IsInitDone(void)
{
	BOOL bInitDone = FALSE;;
	
	EnterCriticalSection(&DioCriticalSection);
	bInitDone = DIO.bInitDone;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bInitDone;
}

/*******************************************************************/
/*SYNTAX: void SetInitDone(void)								   */
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
PROTECTED void SetInitDone(void)
{
	
	EnterCriticalSection(&DioCriticalSection);
	DIO.bInitDone = TRUE;
	LeaveCriticalSection(&DioCriticalSection);	
}

/*******************************************************************/
/*SYNTAX: BOOL ShouldInputStatusBeInverted(DWORD dwBoardIdx)	   */
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
PROTECTED BOOL ShouldInputStatusBeInverted(DWORD dwBoardIdx)
{
	return(DIO.sBoardDetails[dwBoardIdx].bInvertInputs);
}


PROTECTED struct InstantDiCtrl * GetInstance_DI_Ctrl(DWORD dwBoardIdx)
{
	ErrorCode	eRetTemp = Success;
	CHAR		szErrorText[MAX_PATH] = { 0 };

	if (DIO.sBoardDetails[dwBoardIdx].instantDiCtrl == NULL)
	{
		DeviceInformation devInfo;
		devInfo.DeviceNumber = DIO.sBoardDetails[dwBoardIdx].dwDeviceNumber;
		devInfo.DeviceMode = ModeRead;
		devInfo.ModuleIndex = 0;

		size_t sizeReturnValue = 0;
		mbstowcs_s(&sizeReturnValue,
			devInfo.Description,
			sizeof(devInfo.Description) / sizeof(WORD),
			DIO.sBoardDetails[dwBoardIdx].szBoardDescription,
			_countof(DIO.sBoardDetails[dwBoardIdx].szBoardDescription));

		// check if board description is copied properly
		if (sizeReturnValue != (strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1))
		{
			DIO_Error(&DBG, "GetInstance_DI_Ctrl : error while copying BoardDescription to devInfo.Description! mbstowcs_s copied %d characters to devInfo.Description but BoardDescription id %d characters long (including null termination char)",
				sizeReturnValue,
				strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1);
			return NULL;
		}

		// Step 1: create 'InstantDiCtrl' object
		InstantDiCtrl *instantDiCtrl = InstantDiCtrl_Create();

		// Step 2: select the device with 'ModeRead' mode
		eRetTemp = InstantDiCtrl_setSelectedDevice(instantDiCtrl, &devInfo);
		if (eRetTemp != Success)
		{
			GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
			DIO_Error(&DBG, "GetInstance_DI_Ctrl :  dwBoardIdx = %d, returned: %s", dwBoardIdx, szErrorText);
			
			InstantDiCtrl_Cleanup(instantDiCtrl);
			InstantDiCtrl_Dispose(instantDiCtrl);

			return NULL;
		}

		DIO.sBoardDetails[dwBoardIdx].instantDiCtrl = instantDiCtrl;

	}

	return DIO.sBoardDetails[dwBoardIdx].instantDiCtrl;

}

PROTECTED void DisposeIOBoardCtrl_DI_Instance(DWORD dwBoardIdx)
{
	if (DIO.sBoardDetails[dwBoardIdx].instantDiCtrl != NULL)
	{
		//// Step 4: close device and release any allocated resource.
		InstantDiCtrl_Cleanup(DIO.sBoardDetails[dwBoardIdx].instantDiCtrl);

		//// Step 5: destroy the object
		InstantDiCtrl_Dispose(DIO.sBoardDetails[dwBoardIdx].instantDiCtrl);

		DIO.sBoardDetails[dwBoardIdx].instantDiCtrl = NULL;
	}
}



PROTECTED DIO_STATE ReadInput(DWORD dwBoardIdx, DWORD dwInput, OUT DIO_ERROR *pDioErr)
{
	ErrorCode	eRetTemp				= Success;
	DIO_STATE	eInputState				= 0;	
	DIO_ERROR	eDioErr					= DIO_SUCCESS;
	CHAR		szErrorText[MAX_PATH]	= { 0 };
	byte		bReadFromBoard			= -1;
	int			iPort					= 0;
	int			iBit					= 0;

	// Determine bit and port depending of dwInput
	iPort = dwInput / 8;
	iBit = dwInput % 8;

	InstantDiCtrl *instantDiCtrl = GetInstance_DI_Ctrl(dwBoardIdx);

	if (instantDiCtrl!=NULL)
	{
		// Step 3: Scan the ports
		eRetTemp = InstantDiCtrl_ReadBit(instantDiCtrl, iPort, iBit, &bReadFromBoard);
		if (eRetTemp == Success)
		{
			eInputState = bReadFromBoard;
			eDioErr = DIO_SUCCESS;
		}
		else
		{
			GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
			DIO_Error(&DBG, "ReadInput : InstantDiCtrl_ReadBit failed!, dwBoardIdx = %d, dwInput = %d, iPort = %d, iBit = %d, returned: %s", dwBoardIdx, dwInput, iPort, iBit, szErrorText);
			eDioErr = DIO_PLUGIN_API_ERROR;
		}
	}
	else
	{
		eDioErr = DIO_BOARD_INIT_ERROR;
	}

   
	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
    return eInputState;
}



PROTECTED struct InstantDoCtrl * GetInstance_DO_Ctrl(DWORD dwBoardIdx)
{
	ErrorCode		eRetTemp = Success;
	CHAR			szErrorText[MAX_PATH] = { 0 };

	if (DIO.sBoardDetails[dwBoardIdx].instantDoCtrl == NULL)
	{
		DeviceInformation devInfo;
		devInfo.DeviceNumber = DIO.sBoardDetails[dwBoardIdx].dwDeviceNumber;
		devInfo.DeviceMode = ModeRead;
		devInfo.ModuleIndex = 0;

		size_t sizeReturnValue = 0;
		mbstowcs_s(&sizeReturnValue,
			devInfo.Description,
			sizeof(devInfo.Description) / sizeof(WORD),
			DIO.sBoardDetails[dwBoardIdx].szBoardDescription,
			_countof(DIO.sBoardDetails[dwBoardIdx].szBoardDescription));

		// check if board description is copied properly
		if (sizeReturnValue != (strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1))
		{
			DIO_Error(&DBG, "TestOutput : error while copying BoardDescription to devInfo.Description! mbstowcs_s copied %d characters to devInfo.Description but BoardDescription id %d characters long (including null termination char)",
				sizeReturnValue,
				strlen(DIO.sBoardDetails[dwBoardIdx].szBoardDescription) + 1);
			return NULL;
		}

		// Step 1: create 'InstantDoCtrl' object
		InstantDoCtrl *instantDoCtrl = InstantDoCtrl_Create();

		// Step 2: select the device with 'ModeRead' mode
		eRetTemp = InstantDoCtrl_setSelectedDevice(instantDoCtrl, &devInfo);
		if (eRetTemp != Success)
		{
			GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
			DIO_Error(&DBG, "TestOutput : InstantDoCtrl_setSelectedDevice failed!, dwBoardIdx = %d,  returned: %s", dwBoardIdx, szErrorText);

			InstantDoCtrl_Cleanup(instantDoCtrl);
			InstantDoCtrl_Dispose(instantDoCtrl);

			return NULL;
		}

		DIO.sBoardDetails[dwBoardIdx].instantDoCtrl = instantDoCtrl;

	}

	return DIO.sBoardDetails[dwBoardIdx].instantDoCtrl;
}

PROTECTED void DisposeIOBoardCtrl_DO_Instance(DWORD dwBoardIdx)
{
	if (DIO.sBoardDetails[dwBoardIdx].instantDoCtrl != NULL)
	{
		// Step 4: close device and release any allocated resource.
		InstantDoCtrl_Cleanup(DIO.sBoardDetails[dwBoardIdx].instantDoCtrl);

		// Step 5: destroy the object
		InstantDoCtrl_Dispose(DIO.sBoardDetails[dwBoardIdx].instantDoCtrl);

		DIO.sBoardDetails[dwBoardIdx].instantDoCtrl = NULL;
	}
}


PROTECTED DIO_STATE TestOutput(DWORD dwBoardIdx, DWORD dwOutput, OUT DIO_ERROR *pDioErr)
{
	ErrorCode		eRetTemp				= Success;
	DIO_STATE		eOutputState			= 0;	
	DIO_ERROR		eDioErr					= DIO_SUCCESS;
	CHAR			szErrorText[MAX_PATH]	= { 0 };
	byte			bReadFromBoard			= -1;
	int				iPort					= 0;
	int				iBit					= 0;
	
	// Determine bit and port depending of dwOutput
	iPort = dwOutput / 8;
	iBit = dwOutput % 8;

	InstantDoCtrl *instantDoCtrl = GetInstance_DO_Ctrl(dwBoardIdx);

	if (instantDoCtrl != NULL)

	{		
		// Step 3: Scan the ports
		eRetTemp = InstantDoCtrl_ReadBit(instantDoCtrl, iPort, iBit, &bReadFromBoard);
		if (eRetTemp == Success)
		{
			eOutputState = bReadFromBoard;
			eDioErr = DIO_SUCCESS;
		}
		else
		{
			GetDioCardEnumTxt(eRetTemp, szErrorText, _countof(szErrorText));
			DIO_Error(&DBG, "TestOutput : InstantDoCtrl_ReadBit failed!, dwBoardIdx = %d, dwOutput = %d, iPort = %d, iBit = %d, returned: %s", dwBoardIdx, dwOutput, iPort, iBit, szErrorText);
			eDioErr = DIO_PLUGIN_API_ERROR;
		}
	}
	else
	{
		eDioErr = DIO_BOARD_INIT_ERROR;
	}


	
	if(pDioErr != NULL)
		*pDioErr = eDioErr;
    
    return eOutputState;
}

/*******************************************************************/
/*SYNTAX: void GetDioCardEnumTxt(								   */
/*								IN		ErrorCode	iErrorEnum,	   */
/*								IN	OUT	CHAR		*pszEnumText,  */
/*								IN		size_t		enumTextSize)  */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*		Translates DIO card error code to text (useuful for trace) */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*	IN		ErrorCode	iErrorEnum		- Error to translate	   */
/*	IN	OUT	CHAR		*pszEnumText	- Destination buffer	   */
/*	IN		size_t		enumTextSize)	- Destination buffer size  */
/*=================================================================*/
/*  Return                Description                              */
/*******************************************************************/
PRIVATE void GetDioCardEnumTxt(	IN		ErrorCode	iErrorEnum,
								IN	OUT	CHAR		*pszEnumText,
								IN		size_t		enumTextSize)
{
	switch (iErrorEnum)
	{
		case Success:								strcpy_s(pszEnumText, enumTextSize, "Success");									break;
		case WarningIntrNotAvailable:				strcpy_s(pszEnumText, enumTextSize, "WarningIntrNotAvailable");					break;
		case WarningParamOutOfRange:				strcpy_s(pszEnumText, enumTextSize, "WarningParamOutOfRange");					break;
		case WarningPropValueOutOfRange:			strcpy_s(pszEnumText, enumTextSize, "WarningPropValueOutOfRange");				break;
		case WarningPropValueNotSpted:				strcpy_s(pszEnumText, enumTextSize, "WarningPropValueNotSpted");				break;
		case WarningPropValueConflict:				strcpy_s(pszEnumText, enumTextSize, "WarningPropValueConflict");				break;
		case WarningVrgOfGroupNotSame:				strcpy_s(pszEnumText, enumTextSize, "WarningVrgOfGroupNotSame");				break;
		case WarningPropPartialFailed:				strcpy_s(pszEnumText, enumTextSize, "WarningPropPartialFailed");				break;
		case WarningFuncStopped:					strcpy_s(pszEnumText, enumTextSize, "WarningFuncStopped");						break;
		case WarningFuncTimeout:					strcpy_s(pszEnumText, enumTextSize, "WarningFuncTimeout");						break;
		case WarningCacheOverflow:					strcpy_s(pszEnumText, enumTextSize, "WarningCacheOverflow");					break;
		case WarningBurnout:						strcpy_s(pszEnumText, enumTextSize, "WarningBurnout");							break;
		case WarningRecordEnd:						strcpy_s(pszEnumText, enumTextSize, "WarningRecordEnd");						break;
		case WarningProfileNotValid:				strcpy_s(pszEnumText, enumTextSize, "WarningProfileNotValid");					break;
		case ErrorHandleNotValid:					strcpy_s(pszEnumText, enumTextSize, "ErrorHandleNotValid");						break;
		case ErrorParamOutOfRange:					strcpy_s(pszEnumText, enumTextSize, "ErrorParamOutOfRange");					break;
		case ErrorParamNotSpted:					strcpy_s(pszEnumText, enumTextSize, "ErrorParamNotSpted");						break;
		case ErrorParamFmtUnexpted:					strcpy_s(pszEnumText, enumTextSize, "ErrorParamFmtUnexpted");					break;
		case ErrorMemoryNotEnough:					strcpy_s(pszEnumText, enumTextSize, "ErrorMemoryNotEnough");					break;
		case ErrorBufferIsNull:						strcpy_s(pszEnumText, enumTextSize, "ErrorBufferIsNull");						break;
		case ErrorBufferTooSmall:					strcpy_s(pszEnumText, enumTextSize, "ErrorBufferTooSmall");						break;
		case ErrorDataLenExceedLimit:				strcpy_s(pszEnumText, enumTextSize, "ErrorDataLenExceedLimit");					break;
		case ErrorFuncNotSpted:						strcpy_s(pszEnumText, enumTextSize, "ErrorFuncNotSpted");						break;
		case ErrorEventNotSpted:					strcpy_s(pszEnumText, enumTextSize, "ErrorEventNotSpted");						break;
		case ErrorPropNotSpted:						strcpy_s(pszEnumText, enumTextSize, "ErrorPropNotSpted");						break;
		case ErrorPropReadOnly:						strcpy_s(pszEnumText, enumTextSize, "ErrorPropReadOnly");						break;
		case ErrorPropValueConflict:				strcpy_s(pszEnumText, enumTextSize, "ErrorPropValueConflict");					break;
		case ErrorPropValueOutOfRange:				strcpy_s(pszEnumText, enumTextSize, "ErrorPropValueOutOfRange");				break;
		case ErrorPropValueNotSpted:				strcpy_s(pszEnumText, enumTextSize, "ErrorPropValueNotSpted");					break;
		case ErrorPrivilegeNotHeld:					strcpy_s(pszEnumText, enumTextSize, "ErrorPrivilegeNotHeld");					break;
		case ErrorPrivilegeNotAvailable:			strcpy_s(pszEnumText, enumTextSize, "ErrorPrivilegeNotAvailable");				break;
		case ErrorDriverNotFound:					strcpy_s(pszEnumText, enumTextSize, "ErrorDriverNotFound");						break;
		case ErrorDriverVerMismatch:				strcpy_s(pszEnumText, enumTextSize, "ErrorDriverVerMismatch");					break;
		case ErrorDriverCountExceedLimit:			strcpy_s(pszEnumText, enumTextSize, "ErrorDriverCountExceedLimit");				break;
		case ErrorDeviceNotOpened:					strcpy_s(pszEnumText, enumTextSize, "ErrorDeviceNotOpened");					break;
		case ErrorDeviceNotExist:					strcpy_s(pszEnumText, enumTextSize, "ErrorDeviceNotExist");						break;
		case ErrorDeviceUnrecognized:				strcpy_s(pszEnumText, enumTextSize, "ErrorDeviceUnrecognized");					break;
		case ErrorConfigDataLost:					strcpy_s(pszEnumText, enumTextSize, "ErrorConfigDataLost");						break;
		case ErrorFuncNotInited:					strcpy_s(pszEnumText, enumTextSize, "ErrorFuncNotInited");						break;
		case ErrorFuncBusy:							strcpy_s(pszEnumText, enumTextSize, "ErrorFuncBusy");							break;
		case ErrorIntrNotAvailable:					strcpy_s(pszEnumText, enumTextSize, "ErrorIntrNotAvailable");					break;
		case ErrorDmaNotAvailable:					strcpy_s(pszEnumText, enumTextSize, "ErrorDmaNotAvailable");					break;
		case ErrorDeviceIoTimeOut:					strcpy_s(pszEnumText, enumTextSize, "ErrorDeviceIoTimeOut");					break;
		case ErrorSignatureNotMatch:				strcpy_s(pszEnumText, enumTextSize, "ErrorSignatureNotMatch");					break;
		case ErrorFuncConflictWithBfdAi:			strcpy_s(pszEnumText, enumTextSize, "ErrorFuncConflictWithBfdAi");				break;
		case ErrorVrgNotAvailableInSeMode:			strcpy_s(pszEnumText, enumTextSize, "ErrorVrgNotAvailableInSeMode");			break;
		case ErrorVrgNotAvailableIn50ohmMode:		strcpy_s(pszEnumText, enumTextSize, "ErrorVrgNotAvailableIn50ohmMode");			break;
		case ErrorCouplingNotAvailableIn50ohmMode:	strcpy_s(pszEnumText, enumTextSize, "ErrorCouplingNotAvailableIn50ohmMode");	break;
		case ErrorCouplingNotAvailableInIEPEMode:	strcpy_s(pszEnumText, enumTextSize, "ErrorCouplingNotAvailableInIEPEMode");		break;
		case ErrorDeviceCommunicationFailed:		strcpy_s(pszEnumText, enumTextSize, "ErrorDeviceCommunicationFailed");			break;
		case ErrorFixNumberConflict:				strcpy_s(pszEnumText, enumTextSize, "ErrorFixNumberConflict");					break;
		case ErrorTrigSrcConflict:					strcpy_s(pszEnumText, enumTextSize, "ErrorTrigSrcConflict");					break;
		case ErrorPropAllFailed:					strcpy_s(pszEnumText, enumTextSize, "ErrorPropAllFailed");						break;
		case ErrorUndefined:						strcpy_s(pszEnumText, enumTextSize, "ErrorUndefined");							break;
	}
}
/*-------------------------------- END OF FILE ------------------------------*/