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

#include <dio_unidaq.h>
#include <dio_unidaq_glob.h>
#include <dio_unidaq_loc.h>

/*--------------------------- RESERVED:  --------------------------*/

#include <memclass.h>

/*--------------------------- EXTERNALS: --------------------------*/


/*--------------------------- DEFINES:   --------------------------*/



/*--------------------------- TYPEDEFS:  --------------------------*/



/*--------------------------- FUNCTIONS: --------------------------*/

PRIVATE void CallbackFnc_SendFullSet(DWORD dwBoardIdx)
{
	DWORD				dwNbIO								= 0;
	DWORD				dwFirstIO							= 0;
	DWORD				dwIdx								= 0;
	BYTE				abInputStatus[DIO_NB_INPUTS_MAX]	= {0};
	BYTE				abOutputStatusCopy[DIO_NB_INPUTS_MAX]	= {0};
	HINSTANCE			hPlugin;
	struct_dio_status	sDioStatus							= {0};
	
	// Send whole status of inputs for requested DIO board, output by output
	hPlugin = GetPluginHandle();

	// Grab current card state
	EnterCriticalSection(&DioCriticalSection);
		memcpy(abInputStatus, DIO.sBoardDetails[dwBoardIdx].abInputStatus, sizeof(abInputStatus));
	LeaveCriticalSection(&DioCriticalSection);

	dwNbIO = GetIOBoardNbInputs(dwBoardIdx);
	dwFirstIO = GetIOBoardFirstInputIdx(dwBoardIdx);

	for (dwIdx = 0; dwIdx < dwNbIO; dwIdx++)
	{
		sDioStatus.dwDioIdx = dwFirstIO + dwIdx;
		sDioStatus.eState = abInputStatus[dwIdx];
		
		if (DIO.pCallbackFnc != NULL)		
			DIO.pCallbackFnc(DIO_DI_STATUS_EVT, &sDioStatus, hPlugin);
	}

	// Send whole status of outputs, output by output

	// Grab current card state
	EnterGlobalRegion();
		memcpy(abOutputStatusCopy, DIO_GLOB.sBoardDetails[dwBoardIdx].abOutputStatus, sizeof(abOutputStatusCopy));
	LeaveGlobalRegion();
	
	dwNbIO = GetIOBoardNbOutputs(dwBoardIdx);
	dwFirstIO = GetIOBoardFirstOutputIdx(dwBoardIdx);
	
	for (dwIdx = 0; dwIdx < dwNbIO; dwIdx++)
	{
		sDioStatus.dwDioIdx = dwFirstIO + dwIdx;
		sDioStatus.eState = abOutputStatusCopy[dwIdx];
		
		if (DIO.pCallbackFnc != NULL)		
			DIO.pCallbackFnc(DIO_DO_STATUS_EVT, &sDioStatus, hPlugin);
	}
}


PRIVATE void SendPluginConfig(void)
{
	struct_dio_plugin_config	sConfig = {0};
	DWORD						dwIdx	= 0;

	DIO_Trace(&DBG, "SendPluginConfig: ..");


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

	DIO_Trace(&DBG, "SendPluginConfig: OK");

	if (DIO.pCallbackFnc != NULL)		
		DIO.pCallbackFnc(DIO_PLUGIN_CONFIG_EVT, &sConfig, GetPluginHandle());

}



PRIVATE void ExecuteCallbackFnc(enum_pdio_exec_msg eMsg,
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

			if (DIO.pCallbackFnc != NULL)		
				DIO.pCallbackFnc(DIO_DI_STATUS_EVT, &sDioStatus, GetPluginHandle());
			break;
	
		case PDIO_EXEC_OUTPUT_STATE:
			sDioStatus.dwDioIdx = (DWORD)wParam;
			sDioStatus.eState = (DIO_STATE)lParam;

			if (DIO.pCallbackFnc != NULL)		
				DIO.pCallbackFnc(DIO_DO_STATUS_EVT, &sDioStatus, GetPluginHandle());
			break;

		case PDIO_EXEC_DEVICE_STATUS:
			sDevStatus.dwBoardIdx = (DWORD)wParam;
			sDevStatus.bLinkError = (BOOL)lParam;

			if (DIO.pCallbackFnc != NULL)		
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



PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter)
{
	MSG	sMsg	= {0};
	INT	iMsgRet	= FALSE;
	
	DIO_Trace(&DBG, "DIO_ExecThread: start");
	// send configuration
	ExecuteCallbackFnc(PDIO_EXEC_PLUGIN_CONFIG, 0, 0);

	DIO_Trace(&DBG, "DIO_ExecThread: ExecuteCallbackFnc OK!");
	// Create message queue
	PeekMessage(&sMsg, NULL, FIRST_PDIO_EXEC_MSG, LAST_PDIO_EXEC_MSG, PM_NOREMOVE);

	DIO_Trace(&DBG, "DIO_ExecThread: entering loop...");


	while (TRUE)
	{
		// Wait for instructions what to do
		iMsgRet = GetMessage(&sMsg, NULL, FIRST_PDIO_EXEC_MSG, LAST_PDIO_EXEC_MSG);
		switch(iMsgRet)
		{
			case 0: // Received WM_QUIT
				return 0;
		
			case -1: // Error
				break;
		
			default: // All other messages
				if(!IsCallbackFncInUse())
					break;	// Nothing to do
				
				ExecuteCallbackFnc(sMsg.message, sMsg.wParam, sMsg.lParam);
				break;
		}

		Sleep(1);
	}

	return 0;
}

/*--------------------------- END OF FILE -------------------------*/