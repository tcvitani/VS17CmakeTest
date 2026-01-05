/***************** (v) 2012 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_UNIDAQ														 */
/* FILE:	 dio_unidaq.c													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*--------------------------- INCLUDES:  --------------------------*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include <CSRLC32.H>
#include <reg.h>
#include <trc.h>

#include <UniDAQ.h>

#define PCL_DEF
#include <dio_unidaq.h>
#undef PCL_DEF



#define LOC_DEF
#include <dio_unidaq_loc.h>
#define LOC_DEF

#define LOC_DEF
#include <dio_unidaq_glob.h>
#define LOC_DEF
/*--------------------------- RESERVED:  --------------------------*/

#include <memclass.h>

/*--------------------------- EXTERNALS: --------------------------*/


/*--------------------------- DEFINES:   --------------------------*/



/*--------------------------- TYPEDEFS:  --------------------------*/



/*--------------------------- FUNCTIONS: --------------------------*/


PRIVATE DIO_ERROR RegistryParametres(IN HKEY hKey, IN CHAR *szRoot);
PRIVATE WORD GetGlobalOutputByte(DWORD dwBoardIdx, WORD wOffset);

PRIVATE LPSTR GetErrDesc(LONG Err);

/*--------------------------- VARIABLES: --------------------------*/


// Index of description must match return code
static struct_err err_tbl[] =
{
	// Common
	{ Ixud_NoErr                    ,"Correct" },
	{ Ixud_OpenDriverErr            ,"Open driver error" },
	{ Ixud_PnPDriverErr				,"Plug & Play error" },
	{ Ixud_DriverNoOpen             ,"The driver was not open" },
	{ Ixud_GetDriverVersionErr      ,"Recieve driver version error" },
	{ Ixud_ExceedBoardNumber		,"Board number error" },
	{ Ixud_FindBoardErr				,"No board found" },
	{ Ixud_BoardMappingErr			,"Board Mapping error" },
	{ Ixud_DIOModesErr				,"Digital input/output mode setting error" },
	{ Ixud_InvalidAddress			,"Invalid address" },
	{ Ixud_InvalidSize				,"Invalid size" },
	{ Ixud_InvalidPortNumber		,"Invalid port number" },
	{ Ixud_UnSupportedModel			,"This board model is not supported" },
	{ Ixud_UnSupportedFun			,"This function is not supported" },
	{ Ixud_InvalidChannelNumber		,"Invalid channel number" },
	{ Ixud_InvalidValue				,"Invalid value" },
	{ Ixud_InvalidMode				,"Invalid mode" },
	{ Ixud_GetAIStatusTimeOut		,"A timeout occurred while receiving the status of the analog input" },
	{ Ixud_TimeOutErr				,"Timeout error" },
	{ Ixud_CfgCodeIndexErr			,"A compatible configuration code table index could not be found" },
	{ Ixud_ADCCTLTimeoutErr			,"ADC controller a timeout error" },
	{ Ixud_FindPCIIndexErr			,"A compatible PCI table index value could not be found" },
	{ Ixud_InvalidSetting			,"Invalid setting value" },
	{ Ixud_AllocateMemErr	    	,"Error while allocating the memory space" },
	{ Ixud_InstallEventErr			,"Error while installing the interrupt event" },
	{ Ixud_InstallIrqErr			,"Error while installing the interrupt IRQ" },
	{ Ixud_RemoveIrqErr				,"Error while removing the interrupt IRQ" },
	{ Ixud_ClearIntCountErr			,"Error while the clear interrupt count" },
	{ Ixud_GetSysBufferErr			,"Error while retrieving the system buffer" },
	{ Ixud_CreateEventErr			,"Error while create the event" },
	{ Ixud_UnSupportedResolution	,"Resolution not supported" },
	{ Ixud_CreateThreadErr			,"Error while create the thread" },
	{ Ixud_ThreadTimeOutErr			,"Thread timeout error" },
	{ Ixud_FIFOOverFlowErr			,"FIFO overflow error" },
	{ Ixud_FIFOTimeOutErr			,"FIFO timeout error" },
	{ Ixud_GetIntInstStatus			,"Retrieves the status of the interrupt installation" },
	{ Ixud_GetBufStatus				,"Retrieves the status of the system buffer" },
	{ Ixud_SetBufCountErr			,"Error while setting the buffer count" },
	{ Ixud_SetBufInfoErr            ,"Error while setting the buffer data" },
	{ Ixud_FindCardIDErr			,"Card ID code could not be found" },
	{ Ixud_EventThreadErr			,"Event Thread error" },
	{ Ixud_AutoCreateEventErr		,"Error while automatically creating an event" },
	{ Ixud_RegThreadErr				,"Register Thread error" },
	{ Ixud_SearchEventErr			,"Search Event error" },
	{ Ixud_FifoResetErr				,"Error while resetting the FIFO" },
	{ Ixud_InvalidBlock				,"Invalid EEPROM block" },
	{ Ixud_InvalidAddr				,"Invalid EEPROM address" },
	{ Ixud_AcqireSpinLock			,"Error while acquiring spin lock" },
	{ Ixud_ReleaseSpinLock			,"Error while releasing spin lock" },
	{ Ixud_SetControlErr			,"Analog input setting error" },
	{ Ixud_InvalidChannels			,"Invalid channel number" },
	{ Ixud_SearchCardErr			,"Invalid model number" },
	{ Ixud_SetMapAddressErr			,"Error while setting the mapping address" },
	{ Ixud_ReleaseMapAddressErr		,"Error while releasing the mapping address" },
	{ Ixud_InvalidOffset			,"Invalid memory offset" },
	{ Ixud_ShareHandleErr			,"Open the share memory fail" },
	{ Ixud_InvalidDataCount			,"Invalid data count" },
	{ Ixud_WriteEEPErr				,"Error while writing the EEPROM" },
	{ Ixud_CardIOErr                ,"CardIO error" },
	{ Ixud_IOErr                    ,"MemoryIO error" },

};

/*------------------------------ CODE: ----------------------------*/

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
/*																	*/
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
	DIO_ERROR			eRet;
	DWORD				dwBoard;
	CHAR				szDeviceNameBuffer[MAX_PATH]	= {0};
	CHAR				szDeviceTypeBuffer[MAX_PATH]	= {0};
	BOOL				bFound							= FALSE;	
	WORD				wTotalBoards = 0, wRtn = 0, wSelect = 0;
	IXUD_DEVICE_INFO	sDevInfo[MAX_BOARD_NUMBER];
	IXUD_CARD_INFO		sCardInfo[MAX_BOARD_NUMBER];
	char szModelName[MAX_PATH]="Unknown Device";
 	WORD				wInstID;
   
    DIO_InitTraces (&DBG);

	DIO_Trace(&DBG, "PDioInit(): Start...!");
    
	eRet = RegistryParametres(hKey, szRoot);
	
	if(eRet != DIO_SUCCESS)
		return eRet;

	if(!InitGlobalRegion())
		return -1;

	EnterGlobalRegion();
	   
		if (UNIDAQ_NB_INSTANCES == 0)
		  memset(&DIO_GLOB, 0, sizeof (struct_dio_global));
	   
	   wInstID = UNIDAQ_NB_INSTANCES++;
		
	LeaveGlobalRegion();


	DIO_Trace(&DBG, "PDioInit(): Start instance ID:%d !", wInstID);
	DIO.pCallbackFnc = fncStatusOccured;
	
	if(DIO.pCallbackFnc== NULL)
		DIO_Trace(&DBG, "PDioInit(): DIO.pCallbackFnc = NULL! Not using callback!");
	
	
	DIO.hPlugin = hPlugin;
	
	InitializeCriticalSection(&DioCriticalSection);

	// Initialization of driver
	wRtn = Ixud_DriverInit(&wTotalBoards);

	if(wRtn != Ixud_NoErr) 
	{
		DIO_Error(&DBG, "InitDio() : Ixud_DriverInit() return error %s ", GetErrDesc(wRtn));
		
		return DIO_BOARD_INIT_ERROR;
	}

	if ( wTotalBoards < DIO.dwNbBoards ) 
	{
		DIO_Error (&DBG, "InitDio() : detected DIO cards: %u ; defined Dio cards in Registry: %u", wTotalBoards, DIO.dwNbBoards);
		return DIO_BOARD_INIT_ERROR;
	}
	
	// check do the card type set in Registry match to 
	// real card type of boards
	for(dwBoard=0; dwBoard<DIO.dwNbBoards; dwBoard++)
	{
		wSelect = (WORD)dwBoard;
		wRtn = Ixud_GetCardInfo(wSelect,&sDevInfo[wSelect],&sCardInfo[wSelect],szModelName);
        
		if(wRtn != Ixud_NoErr) 
		{
			DIO_Error(&DBG, "InitDio() : DioQueryDeviceName() return error %s for card index %d!", GetErrDesc(wRtn), dwBoard);
			
			return DIO_BOARD_INIT_ERROR;
		}
		else
			DIO_Trace(&DBG, "PDioInit() for DIO board [%d]: SUCCESSFUL, DIO card model name = %s", dwBoard, szModelName);

	}
    
	DIO_Trace(&DBG, "PDioInit(): Start threads ...!");

	if (DIO.pCallbackFnc != NULL)
		DIO.hExec = CreateThread(NULL, 0, DIO_ExecThread, NULL, 0, &DIO.dwExecThreadId);

	DIO.hPoll = CreateThread(NULL, 0, DIO_PollThread, NULL, 0, NULL);
	

	DIO_Trace(&DBG, "PDioInit(): Init done...!");

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
/*            Deinitialisation of DIO UNIDAQ module.               */
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
	DIO_Trace(&DBG, "PDioRelease(): start...!");
	SetShouldExit();

	
	Sleep(DIO.dwPollingDelayMs*2);
	
	if (DIO.pCallbackFnc != NULL)
		TerminateThread(DIO.hExec, 0);

	TerminateThread(DIO.hPoll, 0);
				

	Ixud_DriverClose();

	DIO_Trace(&DBG, "PDioRelease(): end!");
	DIO_DeinitTraces(&DBG);
	
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
	DWORD	dwSize					= 0;
	CHAR	*pCh					= NULL;
	CHAR	szMsg[MAX_PATH]			= {0};
	CHAR	szSubKey[MAX_PATH]		= {0};
	CHAR	szTempKey[MAX_PATH]		= {0};
	CHAR	szExtendedKey[MAX_PATH]	= {0};
	HKEY	hRootKey				= CSR_REG_KEYi_ROOT;
	CHAR	szDioKey[MAX_PATH]		= {0};
	
	if(hKey != NULL)
		hRootKey = hKey;
	
	if(szRoot != NULL)
	{
		strncpy_s(szDioKey, sizeof(szDioKey), szRoot, sizeof(szDioKey));
		strcat_s(szDioKey, sizeof(szDioKey), DIO_UNIDAQ_REG_KEYn);
	}
	else
	{
		sprintf_s(szDioKey, sizeof(szDioKey), "%s%s%s%s", KEY_MODULE, DIO_REG_KEYn_IO_ROOT, DIO_REG_KEYn_PLUGINS, DIO_UNIDAQ_REG_KEYn);
	}

	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
						szDioKey, 
						REGKEY_POLLING_DELAY, 
						&DIO.dwPollingDelayMs) != ERROR_SUCCESS)
	{
		DIO.dwPollingDelayMs = 1;

		DIO_Error(&DBG, 
				"Incorrect or missing registry value: %s => %s ! Setting default to 1 ms", 
				szDioKey,
				REGKEY_POLLING_DELAY);
	}	
	
	dwSize = sizeof(szTempKey);
	while(REG_Enum_Cles(hRootKey, szDioKey, Idx, szTempKey, &dwSize) == ERROR_SUCCESS)
	{			
		// SubKeys should be in following order: "Carte0", "Carte1", "Carte2", etc...
		pCh = strstr(szTempKey, DIO_REGKEY_DIO_BOARD);
		if (pCh == NULL || (unsigned)atoi(pCh + strlen(DIO_REGKEY_DIO_BOARD)) != Idx)
		{
			DIO_Error (&DBG, "Incorrect registry key: %s !", szTempKey);

			return DIO_INVALID_REG_CONFIG;
		}

		sprintf_s(szSubKey, sizeof(szSubKey), "%s%s\\", szDioKey, szTempKey);
		
		// reading DioBoardX number of INPUTS
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, DIO_REGKEY_NB_OF_INPUTS, &DIO.sBoardDetails[Idx].dwNbInputs) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_NB_OF_INPUTS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX number of OUTPUTS
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, DIO_REGKEY_NB_OF_OUTPUTS, &DIO.sBoardDetails[Idx].dwNbOutputs) != ERROR_SUCCESS )
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_NB_OF_OUTPUTS);
			
			return DIO_REG_KEY_MISSING;
		} 

		// reading DioBoardX first input index
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, DIO_REGKEY_FIRST_INPUT_IDX, &DIO.sBoardDetails[Idx].dw1stInputIdx) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_FIRST_INPUT_IDX);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading DioBoardX first output index
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, DIO_REGKEY_FIRST_OUTPUT_IDX, &DIO.sBoardDetails[Idx].dw1stOutputIdx) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, DIO_REGKEY_FIRST_OUTPUT_IDX);
			
			return DIO_REG_KEY_MISSING;
		}

	
		// reading DioBoardx index
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, REGKEY_INDEX, &DIO.sBoardDetails[Idx].dwCardIndex) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s !", szSubKey, REGKEY_INDEX);
			
			return DIO_REG_KEY_MISSING;
		}

		// should inputs be inverted
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey, REGKEY_INVERT_INPUTS, &DIO.sBoardDetails[Idx].bInvertInputs) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, "Incorrect or missing registry value: %s => %s!. Default value is set to 0", szSubKey, REGKEY_INVERT_INPUTS);			
			DIO.sBoardDetails[Idx].bInvertInputs = 0;
		}

		// counting total number of inputs and outputs
		DIO.dwNbInputs += DIO.sBoardDetails[Idx].dwNbInputs;
		DIO.dwNbOutputs += DIO.sBoardDetails[Idx].dwNbOutputs;
		
		dwSize = sizeof(szTempKey);
		Idx++;	
	}
	
	DIO.dwNbBoards = Idx;

	DIO_Trace(&DBG, "Total : inputs = %d, outputs = %d, DIO boards = %d", DIO.dwNbInputs, DIO.dwNbOutputs, DIO.dwNbBoards);

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
PUBLIC DIO_STATE WINAPI PDioGetInputState(IN DWORD dwInputIndex, OUT DIO_ERROR *pDioErr)
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
EXPORT DIO_ERROR WINAPI PDioSetOutputState(IN DWORD dwOutputIndex, IN DIO_STATE eState)
{
	DWORD		dwOutput;
	WORD		wOffset;
	DWORD		dwMask = 0;
	DWORD		i;
	WORD		wVal = 0;
	LONG		lRtn;
	BOOL		bFound	= FALSE;
	WORD		wDOVal = 0;
	WORD		wBoardNo = 0;
	WORD		wBitNo = 0;
	
	// search for the card that handles requested output
	// if we have more than one card with outputs (eg: Carte0 has 16 inputs (0-15),
	// Carte1 16 outputs (0-15) and Carte2 32 inputs (0-31) and 32 outputs (0-31), 
	// in this case output 16 has to be on Carte2, output 0
	// eg output 5 => Carte1 - output 5
    for(i=0; i<DIO.dwNbBoards; i++)
    {			
		if(dwOutputIndex >= DIO.sBoardDetails[i].dw1stOutputIdx 
			&& dwOutputIndex < (DIO.sBoardDetails[i].dw1stOutputIdx + DIO.sBoardDetails[i].dwNbOutputs))
		{
			dwOutput = dwOutputIndex - DIO.sBoardDetails[i].dw1stOutputIdx;
			wBoardNo = (WORD)i;		// DIO card selection number			
			wOffset = (WORD)(dwOutput / 8);	// octet that contains requested output
			wBitNo = (WORD)(dwOutput % 8);

			wVal = GetGlobalOutputByte(wBoardNo, wOffset);

			// clear bit
			if (eState == DIO_STATE_LOW)
			{
				dwMask = ~(1 << wBitNo);
				wVal &= dwMask;
			}
			else // set bit
			{
				dwMask = 1 << wBitNo;
				wVal |= dwMask;
			}

			bFound = TRUE;				
			break;
		}
	}

	if(bFound)
	{
		//wDOVal = (eState == DIO_STATE_LOW)?0:1; -Not supported in x64
		//lRtn = Ixud_WriteDOBit(wBoardNo,wOffset,wBitNo, wDOVal);
		lRtn = Ixud_WriteDO(wBoardNo, wOffset, wVal);

		if(lRtn != Ixud_NoErr) 
		{
			DIO_Error(&DBG, "PDioSetOutputState( %d, %d ) : Ixud_WriteDO() return error %s for card %d!", dwOutputIndex, eState, GetErrDesc(lRtn), wBoardNo);

			return DIO_PLUGIN_API_ERROR;
		}
		else
		{
			SetGlobalOutputStatus(wBoardNo, dwOutput, eState);
		}
	}
	else
	{
		DIO_Error(&DBG, "PDioSetOutputState( %d, %d ) -> DIO board that should handle this input has not founded!", dwOutputIndex, eState);
		
		return DIO_INDEX_OUT_OF_RANGE;
	}

    return DIO_SUCCESS;
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
EXPORT DIO_STATE WINAPI PDioGetOutputState(IN DWORD dwOutputIndex, OUT DIO_ERROR *pDioErr)
{
    DWORD		dwOutput;
// 	DWORD		dwMask;
// 	WORD		wOffset; 
	DWORD		i;
	WORD		wBoardNo;
// 	LONG		lRtn;		
    DIO_STATE	eOutputState;
	DIO_ERROR	eDioErr			= DIO_SUCCESS;	
	BOOL		bFound			= FALSE;
	DWORD		dwDOVal = 0;
	
    for(i=0; i<DIO.dwNbBoards; i++)
    {
		if(dwOutputIndex >= DIO.sBoardDetails[i].dw1stOutputIdx 
			&& dwOutputIndex < (DIO.sBoardDetails[i].dw1stOutputIdx + DIO.sBoardDetails[i].dwNbOutputs))
		{
			dwOutput = dwOutputIndex - DIO.sBoardDetails[i].dw1stOutputIdx;
			wBoardNo = (WORD)i;
			bFound = TRUE;
			
			break;
		}	
	}
	
	if(bFound)
	{
		eOutputState = GetGlobalOutputStatus(wBoardNo, dwOutput);

	}
	else
	{
		DIO_Error(&DBG, "PDioGetOutputState( %d ) -> DIO board that should handle this output has not founded!", dwOutputIndex);
		
		eDioErr = DIO_INDEX_OUT_OF_RANGE;
		
		eOutputState = -1;		
	}	

	if(pDioErr != NULL)
		*pDioErr = eDioErr;

	DIO_Trace(&DBG, "PDioGetOutputState( %d ) -> returned:%d!", dwOutputIndex, eOutputState);
	
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

    for(i=0; i<DIO.dwNbBoards; i++)
	{
		for(j=i+1; j<DIO.dwNbBoards; j++)
		{
			if(DIO.sBoardDetails[i].dw1stInputIdx == DIO.sBoardDetails[j].dw1stInputIdx 
				|| DIO.sBoardDetails[i].dw1stOutputIdx == DIO.sBoardDetails[j].dw1stOutputIdx)
			{
				DIO_Error(&DBG, "PDioGetRange() -> First input/output index of DioBoard%d is equal to the first input/output index of DioBoard%d!", i, j);
				
				return DIO_INVALID_REG_CONFIG;
			}
		}
		
		// first and last input index
		if(dwFirstInputIndex > DIO.sBoardDetails[i].dw1stInputIdx)
			dwFirstInputIndex = DIO.sBoardDetails[i].dw1stInputIdx;
		
		if(dwLastInputIndex < (DIO.sBoardDetails[i].dw1stInputIdx + DIO.sBoardDetails[i].dwNbInputs - 1))
			dwLastInputIndex = DIO.sBoardDetails[i].dw1stInputIdx + DIO.sBoardDetails[i].dwNbInputs - 1;
		
		// first and last output index
		if(dwFirstOutputIndex > DIO.sBoardDetails[i].dw1stOutputIdx)
			dwFirstOutputIndex = DIO.sBoardDetails[i].dw1stOutputIdx;
		
		if(dwLastOutputIndex < (DIO.sBoardDetails[i].dw1stOutputIdx + DIO.sBoardDetails[i].dwNbOutputs - 1))
			dwLastOutputIndex = DIO.sBoardDetails[i].dw1stOutputIdx + DIO.sBoardDetails[i].dwNbOutputs - 1;	
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
//------------------------------------------------------------------------------

PROTECTED BOOL IsInitDone(void)
{
	BOOL bInitDone = FALSE;
	
	EnterCriticalSection(&DioCriticalSection);
		bInitDone = DIO.bInitDone;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bInitDone;
}


PROTECTED void SetInitDone(void)
{
	
	EnterCriticalSection(&DioCriticalSection);
		DIO.bInitDone = TRUE;
	LeaveCriticalSection(&DioCriticalSection);	
}



PROTECTED BOOL IsShouldExit(void)
{
	BOOL bShouldExit = FALSE;
	
	EnterCriticalSection(&DioCriticalSection);
		bShouldExit = DIO.bShouldExit;
	LeaveCriticalSection(&DioCriticalSection);
	
	return bShouldExit;
}


PROTECTED void SetShouldExit(void)
{
	
	EnterCriticalSection(&DioCriticalSection);
		DIO.bShouldExit = TRUE;
	LeaveCriticalSection(&DioCriticalSection);	
}





PROTECTED DWORD GetPollingDelayMs(void)
{
	DWORD dwPollingDelayMs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwPollingDelayMs = DIO.dwPollingDelayMs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwPollingDelayMs;
}

PROTECTED DWORD GetNumberOfIOBoards(void)
{
	DWORD dwNbBoards = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbBoards = DIO.dwNbBoards;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbBoards;
}


PROTECTED DWORD GetIOBoardFirstInputIdx(DWORD dwBoardIdx)
{
	DWORD dwFirstInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstInputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stInputIdx;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstInputIdx;
}


PROTECTED DWORD GetIOBoardLastInputIdx(DWORD dwBoardIdx)
{
	DWORD dwLastInputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastInputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stInputIdx + 
		(DIO.sBoardDetails[dwBoardIdx].dwNbInputs - 1);
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastInputIdx;
}


PROTECTED DWORD GetIOBoardNbInputs(DWORD dwBoardIdx)
{
	DWORD dwNbInputs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNbInputs =  DIO.sBoardDetails[dwBoardIdx].dwNbInputs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNbInputs;
}


PROTECTED DWORD GetIOBoardFirstOutputIdx(DWORD dwBoardIdx)
{
	DWORD dwFirstOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwFirstOutputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stOutputIdx;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwFirstOutputIdx;
}


PROTECTED DWORD GetIOBoardLastOutputIdx(DWORD dwBoardIdx)
{
	DWORD dwLastOutputIdx = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLastOutputIdx = DIO.sBoardDetails[dwBoardIdx].dw1stOutputIdx + 
		(DIO.sBoardDetails[dwBoardIdx].dwNbOutputs - 1);
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLastOutputIdx;
}


PROTECTED DWORD GetIOBoardNbOutputs(DWORD dwBoardIdx)
{
	DWORD dwNbOutputs = 0;

	EnterCriticalSection(&DioCriticalSection);
		dwNbOutputs = DIO.sBoardDetails[dwBoardIdx].dwNbOutputs;
	LeaveCriticalSection(&DioCriticalSection);

	return dwNbOutputs;
}


PROTECTED void SetInputStatus(DWORD dwBoardIdx,  DWORD dwInput,  DIO_STATE eDioState)
{
	EnterCriticalSection(&DioCriticalSection);
		DIO.sBoardDetails[dwBoardIdx].abInputStatus[dwInput] = eDioState;
	LeaveCriticalSection(&DioCriticalSection);
}


PROTECTED DIO_STATE GetInputStatus(DWORD dwBoardIdx, DWORD dwInput)
{
	DIO_STATE eInputStatus;

	EnterCriticalSection(&DioCriticalSection);

		eInputStatus = DIO.sBoardDetails[dwBoardIdx].abInputStatus[dwInput];

	LeaveCriticalSection(&DioCriticalSection);

	return eInputStatus;
}

PROTECTED void SetCurrentOutputStatus(DWORD dwBoardIdx,
							   DWORD dwOutput,
							   DIO_STATE eDioState)
{
	EnterCriticalSection(&DioCriticalSection);

		DIO.sBoardDetails[dwBoardIdx].abCurrentOutputStatus[dwOutput] = eDioState;
	
	LeaveCriticalSection(&DioCriticalSection);
}

PROTECTED DIO_STATE GetCurrentOutputStatus(DWORD dwBoardIdx,
									DWORD dwOutput)
{
	DIO_STATE eOutputStatus;

	EnterCriticalSection(&DioCriticalSection);

		eOutputStatus = DIO.sBoardDetails[dwBoardIdx].abCurrentOutputStatus[dwOutput];

	LeaveCriticalSection(&DioCriticalSection);

	return eOutputStatus;
}


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


PROTECTED DWORD GetExecThreadId(void)
{
	DWORD dwThreadId;
	
	EnterCriticalSection(&DioCriticalSection);
	dwThreadId = DIO.dwExecThreadId;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwThreadId;
}

PROTECTED HINSTANCE GetPluginHandle(void)
{
	HINSTANCE hPlugin;
	
	EnterCriticalSection(&DioCriticalSection);
	hPlugin = DIO.hPlugin;
	LeaveCriticalSection(&DioCriticalSection);
	
	return hPlugin;
}


//-------------------------------------------------------------------------------
/**/
/*******************************************************************/
/*SYNTAX: LPSTR GetErrDesc (LONG Err)							   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Test current state of the particular output          */
/*=================================================================*/
/*PARAMETERS:													   */
/*			LONG Err	- Error code returned by PISIDIO_ function */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  Text description of the error								   */
/*******************************************************************/
PRIVATE LPSTR GetErrDesc(LONG Err)
{
	static CHAR szUnk[] = { "Unknown error type" };
	LONG		i;

	for(i = 0; i < sizeof(err_tbl)/sizeof(struct_err); i++)
	{
		if(Err == err_tbl[i].ErrCode)
		{
			return err_tbl[i].ErrDesc;
		}
	}

	return szUnk;
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
PROTECTED BOOL DIO_InitTraces(struct _struct_debug *dbg)
{
    DWORD	dwLen					= 0;
    DWORD	dwTailleMax				= 1400000L;
    CHAR	pcKey[MAX_PATH]			= {0};
    CHAR	rep_debug[MAX_PATH]		= {0};
    CHAR	rep_traces[MAX_PATH]	= {0};
    CHAR	debug[MAX_PATH]			= {0};
    CHAR	traces[MAX_PATH]		= {0};
    
    // chemin de base dans le registre
    sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);
    
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

PROTECTED void DIO_DeinitTraces(struct _struct_debug *dbg)
{
	
	if(dbg->traces!=NULL)
		TRC_Termine_Trace(dbg->traces);

	if(dbg->debug!=NULL)
		TRC_Termine_Trace(dbg->debug);


}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Error (IN struct DBG *dbg, IN char *fmt, ...)  */
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
PROTECTED void DIO_Error(IN struct _struct_debug *dbg, IN char *fmt, ...)
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
PROTECTED void DIO_Trace (IN struct _struct_debug *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start(ap,fmt);

    if(dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);
	
    va_end(ap);
}

//----------------------------------------------------------------------------------


PROTECTED DIO_STATE ReadInput(DWORD dwBoardIdx, DWORD dwInput, OUT DIO_ERROR *pDioErr)
{
	DIO_STATE	eInputState				= 0;	
	DIO_ERROR	eDioErr					= DIO_SUCCESS;
	CHAR		szErrorText[MAX_PATH]	= { 0 };
	byte		bReadFromBoard			= -1;
	int			iPort					= 0;
	int			iBit					= 0;
	WORD		wOffset;
	DWORD		dwMask;
	DWORD       dwDIVal		= 0;
	WORD		wBoardNo	= 0;
	LONG		Rtn;


	wOffset = (WORD)dwInput / 8;	// octet that contains requested input
	dwMask = 1 << (dwInput % 8);	// mask to extract requested input from input octet
		
	//Memo card number
	wBoardNo = (WORD)dwBoardIdx;

	
	// Read octet that contains requested input
	// Digital-Input: [wOffset=0 <=> Ch 0-7], [wOffset=1 <=> Ch 8-15], ...
	Rtn = Ixud_ReadDI(wBoardNo,wOffset,&dwDIVal);

	if(Rtn != Ixud_NoErr) 
	{
		DIO_Error(&DBG, "ReadInput( %d ) : Ixud_ReadDI() return error %s for card board %u!", dwInput, GetErrDesc(Rtn), wBoardNo);
		
		eDioErr = DIO_PLUGIN_API_ERROR;

		eInputState = -1;
	}
	else
	{
		eDioErr = DIO_SUCCESS;
		
		eInputState = (dwDIVal & dwMask) ? DIO_STATE_HIGH : DIO_STATE_LOW;
	}

	

	if(pDioErr != NULL)
		*pDioErr = eDioErr;
	
    return eInputState;
}





PROTECTED DIO_STATE TestOutput(DWORD dwBoardIdx, DWORD dwOutput, OUT DIO_ERROR *pDioErr)
{
	DIO_STATE		eOutputState			= 0;	
	DIO_ERROR		eDioErr					= DIO_SUCCESS;
	CHAR			szErrorText[MAX_PATH]	= { 0 };
	byte			bReadFromBoard			= -1;
	int				iPort					= 0;
	int				iBit					= 0;
	WORD		wOffset;
	DWORD		dwMask;
	DWORD       dwDOVal		= 0;
	WORD		wBoardNo	= 0;
	LONG		lRtn;

	
	wBoardNo = (WORD)dwBoardIdx;
	wOffset = (WORD)dwOutput / 8;		// octet that contains requested input
	dwMask = 1 << (dwOutput % 8);	// mask to extract requested output from output octet
			
	
//	DIO_Trace(&DBG, "TestOutput( %d ) dwBoardIdx:%d: wOffset %d", dwOutput, wBoardNo, wOffset);
			
	// Read octet that contains requested output
	// Digital-Output: [wOffset=0 <=> Ch 0-7], [wOffset=1 <=> Ch 8-15], ...
	lRtn = Ixud_SoftwareReadbackDO(wBoardNo, wOffset, &dwDOVal);
	
	if(lRtn != Ixud_NoErr) 
	{
		DIO_Error(&DBG, "TestOutput( %d ) : Ixud_SoftwareReadbackDO() return error %s for card index %u!", dwOutput, GetErrDesc(lRtn), wBoardNo);
		
		eDioErr = DIO_PLUGIN_API_ERROR;

		eOutputState = -1;
	}
	else
	{
		eDioErr = DIO_SUCCESS;
		
		// extract input from read octet 
		eOutputState = (dwDOVal & dwMask)?(DIO_STATE_HIGH):(DIO_STATE_LOW);	
	}	

	if(pDioErr != NULL)
		*pDioErr = eDioErr;
    
    return eOutputState;
}



//-------------------------------------------------------------------

PROTECTED BOOL InitGlobalRegion (void)
{
    // crée un mutex partagé entre les instances de la DLL pour simuler les régions
    // ATTENTION : pas de blocage de scheduling en région entre les zones en région
    // et les zones hors région contrairement r RTC !
    DIO.hRegion = CreateMutex (NULL, FALSE, "UNIDAQ_GLOBAL_MUTEX");
    if (DIO.hRegion == NULL)
        return FALSE;

    return TRUE;
}


PROTECTED void EnterGlobalRegion (void)
{
   // pas de test car TIME_OUT infini et WAIT_ABANDONNED est OK
   WaitForSingleObject (DIO.hRegion, INFINITE);
}

PROTECTED void LeaveGlobalRegion (void)
{
   if (!ReleaseMutex (DIO.hRegion)) 
   {
	    DIO_Error(&DBG, "ReleaseMutex failed!");
	   exit(-1);
   }
}



PROTECTED void SetGlobalOutputStatus(DWORD dwBoardIdx, DWORD dwOutput, DIO_STATE eDioState)
{
	EnterGlobalRegion();

		DIO_GLOB.sBoardDetails[dwBoardIdx].abOutputStatus[dwOutput] = eDioState;
	
	LeaveGlobalRegion();
}

PROTECTED DIO_STATE GetGlobalOutputStatus(DWORD dwBoardIdx, DWORD dwOutput)
{
	DIO_STATE eOutputStatus;

	EnterGlobalRegion();

		eOutputStatus = DIO_GLOB.sBoardDetails[dwBoardIdx].abOutputStatus[dwOutput];

	LeaveGlobalRegion();

	return eOutputStatus;
}


PRIVATE WORD GetGlobalOutputByte(DWORD dwBoardIdx, WORD wOffset)
{
	WORD wRetVal = 0;
	int i;
	WORD wMask;


	EnterGlobalRegion();

	for (i = 0; i < 8; i++)
	{	
		wMask = DIO_GLOB.sBoardDetails[dwBoardIdx].abOutputStatus[wOffset * 8 + i] << i;
		wRetVal |= wMask;
	}


	LeaveGlobalRegion();

	DIO_Trace(&DBG, "GetGlobalOutputByte(): dwBoardIdx:%d wOffset:%d = %x", dwBoardIdx, wOffset, wRetVal);

	return wRetVal;
}
/*--------------------------- END OF FILE -------------------------*/