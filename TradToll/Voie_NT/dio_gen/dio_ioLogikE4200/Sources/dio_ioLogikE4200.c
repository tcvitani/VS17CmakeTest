/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_ioLogikE4200												 */
/* FILE:	 dio_ioLogikE4200.c												 */
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
//#include <noyau.h>

#include <mxio.h>

#include <dio_Mutex.h>

#define PCL_DEF
#include <dio_ioLogikE4200.h>
#undef PCL_DEF

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define KEY_MODULE	CSR_REG_KEYn_CSRBASE \
					CSR_REG_KEYn_LANE_BASE \
					CSR_REG_KEYn_CONFIG \
					MOD_REG_KEYn_MODULES

#define DBG_NAME					"DIO_IOLOGIK_E4200"

#define REGKEY_SLOT					"Slot"
#define REGKEY_DI					"DI"

#define REGKEY_INPUT_POSITION		"InputPosition"
#define REGKEY_POLLING_DELAY		"PollingDelayMs"
#define REGKEY_LINK_CHECK_DELAY		"LinkCheckDelayMs"
#define REGKEY_TCP_PORT_NUM			"TcpPortNumber"
#define REGKEY_CONNECTION_TIMEOUT	"ConnectionTimeoutMs"
#define REGKEY_IP_ADDRESS			"IpAddress"
#define REGKEY_LOGIN_PASSWORD		"LoginPassword"
#define REGKEY_MAX_SOCKET_ERRORS	"MaxSocketErrors"
#define REGKEY_RESET_SAFE_STATUS	"ResetSafeStatus"
#define REGKEY_SLOT_NB_OF_INPUTS	"SlotNbOfInputs"
#define REGKEY_SLOT_NB_OF_OUTPUTS	"SlotNbOfOutputs"
#define REGKEY_SLOT_FIRST_INPUT_IDX	"SlotFirstInputIndex"
#define REGKEY_SLOT_FIRST_OUTPUT_IDX "SlotFirstOutputIndex"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE BOOL DIO_InitTraces(struct_debug *dbg);
PRIVATE void DIO_Error(IN struct_debug *dbg, IN char *fmt, ...);
PRIVATE void DIO_Trace(IN struct_debug *dbg, IN char *fmt, ...);

PRIVATE LPSTR GetErrDesc(INT Err);

PRIVATE DIO_ERROR RegistryParametres(IN HKEY hKey, IN CHAR *szRoot);

PRIVATE DIO_ERROR ResetSafeStatusIfNecessary(DWORD dwBoardIdx);

PRIVATE void SendPluginConfig(void);

/*-------------------------------- VARIABLES: -------------------------------*/

PUBLIC CRITICAL_SECTION DioCriticalSection = {0};

struct_dio		DIO = {0};
struct_debug	DBG = {0};

// Index of description must match error code
static struct_err err_tbl[] =
{
	// Common
	{	ILLEGAL_FUNCTION				, "illegal function"				},
	{	ILLEGAL_DATA_ADDRESS			, "illegal data address"			},
	{	ILLEGAL_DATA_VALUE				, "illegal data value"				},
	{	SLAVE_DEVICE_FAILURE			, "slave device failure"			},
	{	SLAVE_DEVICE_BUSY				, "slave device busy"				},
	
	{	EIO_TIME_OUT					, "EIO - time out"					},
	{	EIO_INIT_SOCKETS_FAIL			, "EIO - init sockets fail"			},
	{	EIO_CREATING_SOCKET_ERROR		, "EIO - creating socket error"		},
	{	EIO_RESPONSE_BAD				, "EIO - bad response"				},
	{	EIO_SOCKET_DISCONNECT			, "EIO - socket disconnected"		},
	{	PROTOCOL_TYPE_ERROR				, "EIO - error protocol type"		},
	{	EIO_PASSWORD_INCORRECT			, "EIO - incorrect password"		},

	{	SIO_OPEN_FAIL					, "SIO - open fail"					},
	{	SIO_TIME_OUT					, "SIO - time out"					},
	{	SIO_CLOSE_FAIL					, "SIO - close fail"				},
	{	SIO_PURGE_COMM_FAIL				, "SIO - purge comm fail"			},
	{	SIO_FLUSH_FILE_BUFFERS_FAIL		, "SIO - flush file buffers fail"	},
	{	SIO_GET_COMM_STATE_FAIL			, "SIO - get comm state fail"		},
	{	SIO_SET_COMM_STATE_FAIL			, "SIO - set comm state fail"		},
	{	SIO_SETUP_COMM_FAIL				, "SIO - setup comm fail"			},
	{	SIO_SET_COMM_TIME_OUT_FAIL		, "SIO - set comm time out fail"	},
	{	SIO_CLEAR_COMM_FAIL				, "SIO - clear comm  fail"			},
	{	SIO_RESPONSE_BAD				, "SIO - bad response"				},
	{	SIO_TRANSMISSION_MODE_ERROR		, "SIO - error transmission mode"	},
	{	SIO_BAUDRATE_NOT_SUPPORT		, "SIO - baud rate not support"		},
	
	{	PRODUCT_NOT_SUPPORT				, "product not supported"			},
	{	HANDLE_ERROR					, "handle error"					},
	{	SLOT_OUT_OF_RANGE				, "slot out of range"				},
	{	CHANNEL_OUT_OF_RANGE			, "channel out of range"			},
	{	COIL_TYPE_ERROR					, "coil type error"					},
	{	REGISTER_TYPE_ERROR				, "register type error"				},
	{	FUNCTION_NOT_SUPPORT			, "function not supported"			},
	{	OUTPUT_VALUE_OUT_OF_RANGE		, "output value out of range"		},
	{	INPUT_VALUE_OUT_OF_RANGE		, "input value out of range"		},
	{	SLOT_NOT_EXIST					, "slot not exist"					},
	{	FIRMWARE_NOT_SUPPORT			, "firmware not supported"			},
	{	CREATE_MUTEX_FAIL				, "create mutex fail"				},
	
	{	ENUM_NET_INTERFACE_FAIL			, "enum net interface fail"			},
	{	ADD_INFO_TABLE_FAIL				, "add info table fail"				},
	{	UNKNOWN_NET_INTERFACE_FAIL		, "unknown net interface fail"		},
	{	TABLE_NET_INTERFACE_FAIL		, "table net interface fail"		},
};

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
/*            Initialization of DIO ioLogikE4200 module.           */
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
	DIO_ERROR	eRet	= DIO_SUCCESS;
	DWORD		dwIdx	= 0;
	INT			iRet	= 0;

    DIO_InitTraces(&DBG);
    
	eRet = RegistryParametres(hKey, szRoot);
	
	if (eRet != DIO_SUCCESS)
		return eRet;		

	DioMutexStartRegion();
	iRet = MXEIO_Init();
	DioMutexEndRegion();

	if (iRet != MXIO_OK)
	{
		DIO_Error(&DBG, 
			"PDioInit(): MXEIO_Init() => error: %s!", 
			GetErrDesc(iRet));
		
		return DIO_BOARD_INIT_ERROR;
	}
	
	for (dwIdx = 0; dwIdx < DIO.dwNbBoards; dwIdx++)
		DIO.sBoardDetails[dwIdx].bLinkError = TRUE;

	DIO.pCallbackFnc = fncStatusOccured;
	DIO.hPlugin = hPlugin;
	
	DIO_Trace(&DBG, 
		"PDioInit(): MXEIO_Init() => Initiate the socket succeed!");
    
	InitializeCriticalSection(&DioCriticalSection);

	if (DIO.pCallbackFnc != NULL)
		DIO.hExecThread = CreateThread(NULL, 0, DIO_ExecThread, NULL, 0, &DIO.dwExecThreadId);

	DIO.hLinkThread = CreateThread(NULL, 0, DIO_LinkThread, NULL, 0, &DIO.dwLinkThreadId);	
	DIO.hPollThread = CreateThread(NULL, 0, DIO_PollThread, NULL, 0, &DIO.dwPollThreadId);	
	
    return eRet;
}

/**/
/*******************************************************************/
/*SYNTAX: DIO_ERROR PDioRelease(void)							   */
/*=================================================================*/
/*TYPE:   Private function.                                        */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Deinitialization of DIO ioLogikE4200 module.         */
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
	DWORD	dwBoardIdx	= 0;
	DWORD	dwNbBoards	= 0;
	INT		iHandle		= 0;

	dwNbBoards = GetNumberOfIOBoards();

	for(dwBoardIdx = 0; dwBoardIdx < dwNbBoards; dwBoardIdx++)
	{
		iHandle = GetIOBoardConnectionHandle(dwBoardIdx);

		MXEIO_Disconnect(iHandle);
	}

	MXEIO_Exit();

	TerminateThread(&DIO.hExecThread, 0);
	TerminateThread(&DIO.hLinkThread, 0);
	TerminateThread(&DIO.hPollThread, 0);

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
	DWORD	Idx3					= 0;
	DWORD	dwSize					= 0;
	DWORD	dwSize2					= 0;
	DWORD	dwSize3					= 0;
	DWORD	dwTmp					= 0;
	DWORD	dwInputPosition			= 0;
	CHAR	*pCh					= NULL;
	CHAR	szMsg[MAX_PATH]			= {0};
	CHAR	szSubKey[MAX_PATH]		= {0};
	CHAR	szSubKey2[MAX_PATH]		= {0};
	CHAR	szSubKey3[MAX_PATH]		= {0};
	CHAR	szTempKey[MAX_PATH]		= {0};
	CHAR	szTempKey2[MAX_PATH]	= {0};
	CHAR	szTempKey3[MAX_PATH]	= {0};
	CHAR	szSettingsKey[MAX_PATH]	= {0};
	CHAR	szExtendedKey[MAX_PATH]	= {0};
	HKEY	hRootKey				= CSR_REG_KEYi_ROOT;
	CHAR	szDioKey[MAX_PATH]		= {0};
	
	if(hKey != NULL)
		hRootKey = hKey;
	
	if(szRoot != NULL)
	{
		strncpy_s(szDioKey, sizeof(szDioKey), szRoot, sizeof(szDioKey));
		strcat_s(szDioKey, sizeof(szDioKey), DIO_IOLOGIK_E4200_REG_KEYn);
	}
	else
	{
		sprintf_s(szDioKey, sizeof(szDioKey),
				"%s%s%s%s", 
				KEY_MODULE, 
				DIO_REG_KEYn_IO_ROOT, 
				DIO_REG_KEYn_PLUGINS, 
				DIO_IOLOGIK_E4200_REG_KEYn);
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

	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
						szDioKey, 
						REGKEY_LINK_CHECK_DELAY, 
						&DIO.dwLinkCheckDelayMs) != ERROR_SUCCESS)
	{
		DIO_Error(&DBG, 
				"Incorrect or missing registry value: %s => %s !", 
				szSubKey, 
				REGKEY_LINK_CHECK_DELAY);
		
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

		sprintf_s(szSubKey, sizeof(szSubKey), "%s%s\\", szDioKey, szTempKey);
		
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
		
		// reading TCP port number
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_TCP_PORT_NUM, 
							&dwTmp) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					REGKEY_TCP_PORT_NUM);
			
			return DIO_REG_KEY_MISSING;
		}
		DIO.sBoardDetails[Idx].wTcpPortNum = (WORD)dwTmp;

		// reading connection timeout
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_CONNECTION_TIMEOUT, 
							&DIO.sBoardDetails[Idx].dwConnTimeOut) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					REGKEY_CONNECTION_TIMEOUT);
			
			return DIO_REG_KEY_MISSING;
		}

		// max socket errors
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_MAX_SOCKET_ERRORS, 
							&DIO.sBoardDetails[Idx].dwMaxSocketErrors) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					REGKEY_MAX_SOCKET_ERRORS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reset safe status
		if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_RESET_SAFE_STATUS, 
							&DIO.sBoardDetails[Idx].dwResetSafeStatus) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey, 
					REGKEY_RESET_SAFE_STATUS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading IP address
		dwTmp = sizeof(DIO.sBoardDetails[Idx].szIpAddress);
		if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_IP_ADDRESS, 
							DIO.sBoardDetails[Idx].szIpAddress,
							&dwTmp) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
				"Incorrect or missing registry value: %s => %s !", 
				szSubKey, 
				REGKEY_IP_ADDRESS);
			
			return DIO_REG_KEY_MISSING;
		}

		// reading login password
		dwTmp = sizeof(DIO.sBoardDetails[Idx].szPassword);
		if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, 
							szSubKey, 
							REGKEY_LOGIN_PASSWORD, 
							DIO.sBoardDetails[Idx].szPassword,
							&dwTmp) != ERROR_SUCCESS)
		{
			DIO_Error(&DBG, 
				"Incorrect or missing registry value: %s => %s !", 
				szSubKey, 
				REGKEY_LOGIN_PASSWORD);
			
			return DIO_REG_KEY_MISSING;
		}

		// counting total number of inputs and outputs
		DIO.dwNbInputs += DIO.sBoardDetails[Idx].dwNbInputs;
		DIO.dwNbOutputs += DIO.sBoardDetails[Idx].dwNbOutputs;

		// Slot registry subkey
		ZeroMemory(szSettingsKey, sizeof(szSettingsKey));
		sprintf_s(szSettingsKey, sizeof(szSettingsKey), "%s", szSubKey);
		
		Idx2 = 0;
		dwSize2 = sizeof(szTempKey2);
		ZeroMemory(szTempKey2, dwSize2);
		while (REG_Enum_Cles(hRootKey, szSettingsKey, Idx2, szTempKey2, &dwSize2) == ERROR_SUCCESS)
		{	
			// SubKeys should be in following order: "Slot1", "Slot2", etc...
			pCh = strstr(szTempKey2, REGKEY_SLOT);
			if (pCh == NULL || (unsigned)atoi(pCh + strlen(REGKEY_SLOT)) != (Idx2 + 1))
			{
				DIO_Error (&DBG, "Incorrect registry key: %s !", szTempKey2);
				
				return DIO_INVALID_REG_CONFIG;
			}
			
			sprintf_s(szSubKey2, sizeof(szSubKey2), "%s%s\\", szSettingsKey, szTempKey2);

			// Storing SlotX position. Position written in name is stored (ex. number 1 is stored for Slot1)
			DIO.sBoardDetails[Idx].sSlotSettings[Idx2].iSlotPosition = atoi(pCh + strlen(REGKEY_SLOT));

			// Reading SlotX SlotNbInputs
			if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey2, REGKEY_SLOT_NB_OF_INPUTS, 
				&DIO.sBoardDetails[Idx].sSlotSettings[Idx2].dwSlotNbInputs) != ERROR_SUCCESS)
			{
				DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey2, 
					REGKEY_SLOT_NB_OF_INPUTS);
				
				return DIO_REG_KEY_MISSING;
			}
			
			// Reading SlotX SlotFirstInputIndex
			if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey2, REGKEY_SLOT_FIRST_INPUT_IDX, 
				&DIO.sBoardDetails[Idx].sSlotSettings[Idx2].dwSlot1stInputIdx) != ERROR_SUCCESS)
			{
				DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey2, 
					REGKEY_SLOT_FIRST_INPUT_IDX);
				
				return DIO_REG_KEY_MISSING;
			}

			// Reading SlotX SlotNbOutputs
			if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey2, REGKEY_SLOT_NB_OF_OUTPUTS, 
				&DIO.sBoardDetails[Idx].sSlotSettings[Idx2].dwSlotNbOutputs) != ERROR_SUCCESS)
			{
				DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey2, 
					REGKEY_SLOT_NB_OF_OUTPUTS);
				
				return DIO_REG_KEY_MISSING;
			}
			
			// Reading SlotX SlotFirstOutputIndex
			if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey2, REGKEY_SLOT_FIRST_OUTPUT_IDX, 
				&DIO.sBoardDetails[Idx].sSlotSettings[Idx2].dwSlot1stOutputIdx) != ERROR_SUCCESS)
			{
				DIO_Error(&DBG, 
					"Incorrect or missing registry value: %s => %s !", 
					szSubKey2, 
					REGKEY_SLOT_FIRST_OUTPUT_IDX);
				
				return DIO_REG_KEY_MISSING;
			}
/* - This isn't needed in this version but it is left if needed in future
			// DI registry subkey
			ZeroMemory(szExtendedKey, sizeof(szExtendedKey));
			sprintf(szExtendedKey, "%s", szSubKey2);

			Idx3 = 0;
			dwSize3 = sizeof(szTempKey3);
			ZeroMemory(szTempKey3, dwSize3);
			while (REG_Enum_Cles(hRootKey, szExtendedKey, Idx3, szTempKey3, &dwSize3) == ERROR_SUCCESS)
			{	
				// SubKeys should be in following order: "DI_00", "DI_01", "DI_02", etc...
				pCh = strstr(szTempKey3, REGKEY_DI);
				if (pCh == NULL || (unsigned)atoi(pCh + strlen(REGKEY_DI) + 1) != Idx3)
				{
					DIO_Error (&DBG, "Incorrect registry key: %s !", szTempKey3);
					
					return DIO_INVALID_REG_CONFIG;
				}
					
				sprintf(szSubKey3, "%s%s\\", szExtendedKey, szTempKey3);

				// SOME REGKEY READING UNDER DI_XX
				if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, szSubKey3, REGKEY_XXX, 
					&DIO.sBoardDetails[Idx].sInputSettings[dwInputPosition].dwFilter) != ERROR_SUCCESS)
				{
					DIO_Error(&DBG, 
						"Incorrect or missing registry value: %s => %s !", 
						szSubKey3, 
						REGKEY_XXX);
					
					return DIO_REG_KEY_MISSING;
				}

				dwSize3 = sizeof(szTempKey3);
				Idx3++;	
			}
*/
			dwSize2 = sizeof(szTempKey2);
			Idx2++;
		}

		DIO.sBoardDetails[Idx].dwNbSlots = Idx2;
		
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
	CHAR		szIpAddress[30]	= {0};

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
		if (IOBoardConnectionOK(dwBoardIdx))
		{
			eInputState = GetInputStatus(dwBoardIdx, dwInput);

			eDioErr = DIO_SUCCESS;
		}
		else
		{
			GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));

			DIO_Error(&DBG, 
				"PDioGetInputState(): Error Connection! Input=%d, IP=%s, Card index=%d", 
				dwInput,
				szIpAddress,
				dwBoardIdx);

			eDioErr = DIO_PLUGIN_CONNECTION_ERROR;
		}
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
	DWORD		dwOutput		= 0;
	DWORD		dwIdx			= 0;
	DWORD		dwBoardIdx		= 0;
	DWORD		dwNbBoards		= 0;
	DWORD		dwFirstOutpuIdx = 0;
	DWORD		dwLastOutputIdx	= 0;
	DWORD		dwIO_Value[16]	= {0};
	BYTE		bytCount		= 0;
	INT			iRet			= 0;
	INT			iHandle			= 0;
	DIO_STATE	eInputState		= 0;
	DIO_ERROR	eDioErr			= DIO_SUCCESS;
	BOOL		bFound			= FALSE;
	CHAR		szIpAddress[30]	= {0};
	INT			iNbSlots		= 0;
	INT			i				= 0;
	struct_slot_settings* sSlotSettings	= {0};

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

	if(bFound)
	{
		if (IOBoardConnectionOK(dwBoardIdx))
		{
			iHandle = GetIOBoardConnectionHandle(dwBoardIdx);

			if (eState == DIO_STATE_LOW)
				dwIO_Value[0] = 0;
			else
				dwIO_Value[0] = 0xFFFFFFFF;
			
			// set just requested output
			bytCount = 1;

			iNbSlots =  GetIOBoardNbSlots(dwIdx);

			// For every slot check that it is slot with inputs/outputs and then check/write inputs/outputs
			for (i = 0; i < iNbSlots; i++)
			{
				sSlotSettings = GetSlotSettingsPtr(dwIdx, i);
				
				// If slot contains outputs AND if output is located on proper slot, write output value 
				if (sSlotSettings->dwSlotNbOutputs > 0 && (sSlotSettings->dwSlot1stOutputIdx + sSlotSettings->dwSlotNbOutputs - 1) >= dwOutput)
				{
 					EnterCriticalSection(&DioCriticalSection);
					iRet = E42_DO_Writes(iHandle, (BYTE)sSlotSettings->iSlotPosition,(BYTE)(dwOutput - sSlotSettings->dwSlot1stOutputIdx), bytCount, dwIO_Value[0]);
 					LeaveCriticalSection(&DioCriticalSection);
					if (iRet == MXIO_OK)
					{
						eDioErr = DIO_SUCCESS;
						break;
					}
					else
					{
						eDioErr = DIO_PLUGIN_API_ERROR;

						GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));

						DIO_Error(&DBG, 
							"PDioSetOutputState(): Error=%s! Output=%d, IP=%s, Card index=%d",
							GetErrDesc(iRet),
							dwOutput,
							szIpAddress,
							dwBoardIdx);
					}
				}
			}
		}
		else
		{	
			GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));

			DIO_Error(&DBG, 
				"PDioSetOutputState(): Error Connection! Output=%d, IP=%s, Card index=%d", 
				dwOutput,
				szIpAddress,
				dwBoardIdx);

			eDioErr = DIO_PLUGIN_CONNECTION_ERROR;
		}
	}
	else
	{
		DIO_Error(&DBG, 
			"PDioSetOutputState( %d, %d ) -> DIO board that should handle this output has not founded!", 
			dwOutputIndex, 
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
	CHAR		szIpAddress[30]	= {0};

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
		if (IOBoardConnectionOK(dwBoardIdx))
		{
			eOutputState = GetOutputStatus(dwBoardIdx, dwOutput);
			
			eDioErr = DIO_SUCCESS;
		}
		else
		{
			GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));
			
			DIO_Error(&DBG, 
					"PDioGetOutputState(): Error Connection! Output=%d, IP=%s, Card index=%d", 
					dwOutput,
					szIpAddress,
					dwBoardIdx);
			
			eDioErr = DIO_PLUGIN_CONNECTION_ERROR;
		}
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
/*SYNTAX: LPSTR GetErrDesc (INT Err)							   */
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
PRIVATE LPSTR GetErrDesc(INT Err)
{
	static CHAR szUnk[] = { "Unknown error type" };
	INT			i;

	for(i = 0; i < sizeof(err_tbl)/sizeof(struct_err); i++)
	{
		if(Err == err_tbl[i].ErrCode)
			return err_tbl[i].ErrDesc;
	}

	return szUnk;
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
/*SYNTAX: DIO_ERROR TryToConnect(DWORD dwBoardIdx)				   */
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
PROTECTED DIO_ERROR TryToConnect(DWORD dwBoardIdx)
{
	INT			iRet			= 0;
	WORD		wTcpPortNum		= 0;
	DWORD		dwConnTimeOut	= 0;
	BYTE		bytStatus		= 0;	 // stored connection status
	CHAR		szIpAddress[30]	= {0};
	CHAR		szPassword[50]	= {0};
	INT			iHandle			= 0;

	DIO_Trace(&DBG, "TryToConnect(): Card index=%d", dwBoardIdx);

	GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));
	GetIOBoardPassword(dwBoardIdx, szPassword, sizeof(szPassword));
	
	dwConnTimeOut = GetIOBoardConnectionTimeout(dwBoardIdx);
	wTcpPortNum = GetIOBoardTcpPortNumber(dwBoardIdx);

	DioMutexStartRegion();
	iRet = MXEIO_Connect(szIpAddress, wTcpPortNum, dwConnTimeOut, &iHandle);
	DioMutexEndRegion();
	if (iRet != MXIO_OK)
	{
		DIO_Error(&DBG, 
				"TryToConnect(): MXEIO_Connect() => error: %s, card index: %d, IP: %s!",
				GetErrDesc(iRet), 
				dwBoardIdx, 
				szIpAddress);
		
		return DIO_PLUGIN_CONNECTION_ERROR;
	}
	
	SetIOBoardConnectionHandle(dwBoardIdx, iHandle);

	DIO_Trace(&DBG, 
			"TryToConnect(): MXEIO_Connect() => Connection succeed! IP=%s, Card index=%d, TimeOut=%d, Password=%s", 
			szIpAddress,
			dwBoardIdx,
			dwConnTimeOut,
			szPassword);
	
	iRet = MXEIO_CheckConnection(iHandle, dwConnTimeOut, &bytStatus);
	if (iRet != MXIO_OK)
	{
		DIO_Error(&DBG, 
				"TryToConnect(): MXEIO_CheckConnection() => error: %s, card index: %d, IP: %s!",
				GetErrDesc(iRet), 
				dwBoardIdx, 
				szIpAddress);
		
		return DIO_PLUGIN_CONNECTION_ERROR;
	}
	
	switch (bytStatus)
	{
 		case CHECK_CONNECTION_OK:
			if ((ResetSafeStatusIfNecessary(dwBoardIdx) != DIO_SUCCESS))
			{
				return DIO_PLUGIN_CONNECTION_ERROR;
			}
			break;
		
		case CHECK_CONNECTION_FAIL:
			DIO_Error(&DBG, 
					"TryToConnect(): MXEIO_CheckConnection() => error: <<CHECK_CONNECTION_FAIL>>, card index: %d, IP: %s!",
					dwBoardIdx, 
					szIpAddress);
			return DIO_PLUGIN_CONNECTION_ERROR;
			
		case CHECK_CONNECTION_TIME_OUT:
			DIO_Error(&DBG, 
					"TryToConnect(): MXEIO_CheckConnection() => error: <<CHECK_CONNECTION_TIME_OUT>>, card index: %d, IP: %s!",
					dwBoardIdx, 
					szIpAddress);
			return DIO_PLUGIN_CONNECTION_ERROR;
	}
	
	DIO_Trace(&DBG, 
			"TryToConnect(): MXEIO_CheckConnection() => Check connection ok! IP=%s, Card index=%d, TimeOut=%d, Password=%s", 
			szIpAddress,
			dwBoardIdx,
			dwConnTimeOut,
			szPassword);

	return DIO_SUCCESS;
}

/*******************************************************************/
/*SYNTAX: DIO_ERROR ResetSafeStatusIfNecessary(DWORD dwBoardIdx)   */
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
PRIVATE DIO_ERROR ResetSafeStatusIfNecessary(DWORD dwBoardIdx)
{
	INT			iRet			= 0;
	INT			iHandle			= 0;
	WORD		wSafeStatus		= 0;	// safe status
	CHAR		szIpAddress[30]	= {0};

	if(DIO.sBoardDetails[dwBoardIdx].dwResetSafeStatus == 1)
	{
		iHandle = GetIOBoardConnectionHandle(dwBoardIdx);
		
		if (wSafeStatus == 1)
		{
			// Clear Safe status
			iRet = E42_ClearSafeStatus(iHandle);
			if (iRet != MXIO_OK)
			{
				GetIOBoardIPAddress(dwBoardIdx, szIpAddress, sizeof(szIpAddress));

				DIO_Error(&DBG, 
					"ResetSafeStatusIfNecessary(): E42_ClearSafeStatus() => error: %s, card index: %d, IP: %s!",
					GetErrDesc(iRet), 
					dwBoardIdx, 
					szIpAddress);
				
				return DIO_PLUGIN_API_ERROR;
			}
		}
	}

	return DIO_SUCCESS;
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

		case PDIO_EXEC_INPUT_OUTPUT_CONFIG_ERR:
			DIO_Error(&DBG,
				"DIO_PollThread(): DIO registry entries SlotNbOfInputs neither SlotNbOfOutputs aren't specified for DioBoard%d Slot%d! At least one entry must be > 0",
				wParam, lParam);
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
/*SYNTAX: DWORD GetLinkCheckDelayMs(void)						   */
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
PROTECTED DWORD GetLinkCheckDelayMs(void)
{
	DWORD dwLinkCheckDelayMs = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwLinkCheckDelayMs = DIO.dwLinkCheckDelayMs;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwLinkCheckDelayMs;
}

/*******************************************************************/
/*SYNTAX: void GetIOBoardIPAddress(DWORD dwBoardIdx,			   */
/*									CHAR *pIpAddress,			   */
/*									DWORD dwMaxSize)			   */
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
PROTECTED void GetIOBoardIPAddress(DWORD dwBoardIdx, 
								   CHAR *pIpAddress,
								   DWORD dwMaxSize)
{
	if (pIpAddress != NULL)
	{
		EnterCriticalSection(&DioCriticalSection);
		strncpy_s(pIpAddress, dwMaxSize, DIO.sBoardDetails[dwBoardIdx].szIpAddress, dwMaxSize);
		LeaveCriticalSection(&DioCriticalSection);
	}	
}

/*******************************************************************/
/*SYNTAX: void GetIOBoardPassword(DWORD dwBoardIdx,				   */
/*									CHAR *pPassword,			   */
/*									DWORD dwMaxSize)			   */
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
PROTECTED void GetIOBoardPassword(DWORD dwBoardIdx,
								  CHAR *pPassword,
								  DWORD dwMaxSize)
{
	if (pPassword != NULL)
	{
		EnterCriticalSection(&DioCriticalSection);
		strncpy_s(pPassword, dwMaxSize, DIO.sBoardDetails[dwBoardIdx].szPassword, dwMaxSize);
		LeaveCriticalSection(&DioCriticalSection);
	}	
}

/*******************************************************************/
/*SYNTAX: WORD GetIOBoardTcpPortNumber(DWORD dwBoardIdx)		   */
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
PROTECTED WORD GetIOBoardTcpPortNumber(DWORD dwBoardIdx)
{
	WORD wTcpPortNum;

	EnterCriticalSection(&DioCriticalSection);
	wTcpPortNum = DIO.sBoardDetails[dwBoardIdx].wTcpPortNum;
	LeaveCriticalSection(&DioCriticalSection);

	return wTcpPortNum;
}

/*******************************************************************/
/*SYNTAX: DWORD GetIOBoardConnectionTimeout(DWORD dwBoardIdx)	   */
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
PROTECTED DWORD GetIOBoardConnectionTimeout(DWORD dwBoardIdx)
{
	DWORD dwConnTimeOut;
	
	EnterCriticalSection(&DioCriticalSection);
	dwConnTimeOut = DIO.sBoardDetails[dwBoardIdx].dwConnTimeOut;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwConnTimeOut;
}

/*******************************************************************/
/*SYNTAX: DWORD GetMaxSocketErrors(DWORD dwBoardIdx)			   */
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
PROTECTED DWORD GetMaxSocketErrors(DWORD dwBoardIdx)
{
	DWORD dwMaxSocketErrors;
	
	EnterCriticalSection(&DioCriticalSection);
	dwMaxSocketErrors = DIO.sBoardDetails[dwBoardIdx].dwMaxSocketErrors;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwMaxSocketErrors;
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
	BOOL			bLinkError_Old	= FALSE;
	BOOL			bLinkError		= FALSE;

	EnterCriticalSection(&DioCriticalSection);
	bLinkError_Old = DIO.sBoardDetails[dwBoardIdx].bLinkError;
	DIO.sBoardDetails[dwBoardIdx].bLinkError = TRUE;

	if(!bLinkError_Old || !IsInitDone())
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
	BOOL bLinkError_Old	= FALSE;
	BOOL bLinkError		= FALSE;

	EnterCriticalSection(&DioCriticalSection);
	bLinkError_Old = DIO.sBoardDetails[dwBoardIdx].bLinkError;
	DIO.sBoardDetails[dwBoardIdx].bLinkError = FALSE;

	if(bLinkError_Old)
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

	if(bLinkError)
		return FALSE;

	return TRUE;
}

/*******************************************************************/
/*SYNTAX: INT GetIOBoardConnectionHandle(DWORD dwBoardIdx)		   */
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
PROTECTED INT GetIOBoardConnectionHandle(DWORD dwBoardIdx)
{
	INT iHandle = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	iHandle = DIO.sBoardDetails[dwBoardIdx].iHandle;
	LeaveCriticalSection(&DioCriticalSection);
	
	return iHandle;
}

/*******************************************************************/
/*SYNTAX: void SetIOBoardConnectionHandle(DWORD dwBoardIdx)		   */
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
PROTECTED void SetIOBoardConnectionHandle(DWORD dwBoardIdx, INT iHandle)
{	
	EnterCriticalSection(&DioCriticalSection);
	DIO.sBoardDetails[dwBoardIdx].iHandle = iHandle;
	LeaveCriticalSection(&DioCriticalSection);	
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
/*SYNTAX: INT GetIOBoardNbSlots(DWORD dwBoardIdx)				   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:  dwBoardIdx - index of the board which contain slots */
/*=================================================================*/
/*  Return                Description                              */
/*		- Number of slots on board.								   */
/*******************************************************************/
PROTECTED INT GetIOBoardNbSlots(DWORD dwBoardIdx)
{
	DWORD dwNoSlots = 0;
	
	EnterCriticalSection(&DioCriticalSection);
	dwNoSlots = DIO.sBoardDetails[dwBoardIdx].dwNbSlots;
	LeaveCriticalSection(&DioCriticalSection);
	
	return dwNoSlots;
}

/*******************************************************************/
/*SYNTAX: struct_slot_settings GetSlotSettingsPtr(DWORD dwBoardIdx,*/
/*												   DWORD dwSlotIdx)*/
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:  dwBoardIdx - index of the board which contain slots */
/*			   dwSlotIdx - slot to be checked					   */
/*=================================================================*/
/*  Return                Description                              */
/*	0						If slot don't contain DO			   */
/*	Nb of outputs			If slot contain DO					   */
/*******************************************************************/
PROTECTED struct_slot_settings* GetSlotSettingsPtr(DWORD dwBoardIdx, DWORD dwSlotIdx)
{
	struct_slot_settings *sSlotSettings = {0};
	
	EnterCriticalSection(&DioCriticalSection);
	sSlotSettings = &DIO.sBoardDetails[dwBoardIdx].sSlotSettings[dwSlotIdx];
	LeaveCriticalSection(&DioCriticalSection);
	
	return sSlotSettings;
}

/*-------------------------------- END OF FILE ------------------------------*/