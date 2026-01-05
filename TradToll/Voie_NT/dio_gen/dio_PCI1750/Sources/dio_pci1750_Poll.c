/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 dio_pcl1750													 */
/* FILE:	 dio_pci1750_Poll.c												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

#include <dio_pci1750.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE void InvertInputState(DIO_STATE *pInputState);

/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/
 

/*******************************************************************/
/*SYNTAX: DWORD WINAPI DIO_PollThread(LPVOID lpParameter)		   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*														           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*													               */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  0                     This function always returns zero.       */
/*******************************************************************/
PROTECTED DWORD WINAPI DIO_PollThread(LPVOID lpParameter)
{
	DIO_ERROR	eRet							= DIO_SUCCESS;
	DIO_STATE	eIOState						= DIO_STATE_LOW;
	DIO_STATE	eIOStateToSend					= DIO_STATE_LOW;
	DIO_STATE	eIOState_Old					= DIO_STATE_LOW;
	DIO_STATE	eRetState						= 0;
	DWORD		dwNbBoards						= 0;
	DWORD		dwBoardIdx						= 0;
	DWORD		dwIdx2							= 0;
	DWORD		dwFirstIO						= 0;
	DWORD		dwDioIdx						= 0;
	DWORD		dwIO_Value[16]					= {0};
	DWORD		dwPollingDelayMs				= 0;
	static BOOL bInitDone						= FALSE;
	BYTE		bytCount						= 0;
	BYTE		bytStartChannel					= 0;
	INT			iRet							= 0;
	INT			iHandle							= 0;
	BOOL		bInvert							= FALSE;
	
	dwPollingDelayMs = GetPollingDelayMs();
	
	dwNbBoards = GetNumberOfIOBoards();

	// Reset link error for all boards.
	// This board is connected in PCI slot and has no link error,
	// this is just to ensure that there is no link error on main app

	// Sleep a little bit while other threads start (they have to be started to read messages from this thread).
	// This isn't proof that other threads has been started but it helps
	Sleep(dwPollingDelayMs);
	for (dwBoardIdx = 0; dwBoardIdx < dwNbBoards; dwBoardIdx++)
	{
		ResetIOBoardConnectionError(dwBoardIdx);
	}

	while (TRUE)
	{
		if(!bInitDone)
		{
			bInitDone = IsInitDone();
			continue;
		}

		for (dwBoardIdx = 0; dwBoardIdx < dwNbBoards; dwBoardIdx++)
		{
			bInvert = ShouldInputStatusBeInverted(dwBoardIdx);

			// Read digital inputs
			ZeroMemory(dwIO_Value, sizeof(dwIO_Value));
			bytCount = (BYTE)GetIOBoardNbInputs(dwBoardIdx);
			bytStartChannel = 0;
			
			for(dwIdx2 = bytStartChannel; dwIdx2 < bytCount; dwIdx2++)
			{
				EnterCriticalSection(&DioCriticalSection);
				eRetState = ReadInput(dwBoardIdx, dwIdx2, &eRet);
				LeaveCriticalSection(&DioCriticalSection);
				
				if(eRet == DIO_SUCCESS)
				{
					eIOState = eRetState;
					
					eIOState_Old = GetInputStatus(dwBoardIdx, dwIdx2);

					if(bInvert)
						InvertInputState(&eIOState_Old);

					dwFirstIO = GetIOBoardFirstInputIdx(dwBoardIdx);
					
					if(eIOState != eIOState_Old)
					{
						dwDioIdx = dwFirstIO + dwIdx2;

						eIOStateToSend = eIOState;

						if(bInvert)
							InvertInputState(&eIOStateToSend);

						PostThreadMessage(	GetExecThreadId(), 
											PDIO_EXEC_INPUT_STATE, 
											dwDioIdx, 
											eIOStateToSend);
						
						SetInputStatus(dwBoardIdx, dwIdx2, eIOStateToSend);
					}
				}
			}
			
			/*------------------------------------------------------------*/
			
			// Read digital outputs
			ZeroMemory(dwIO_Value, sizeof(dwIO_Value));
			bytCount = (BYTE)GetIOBoardNbOutputs(dwBoardIdx);
			bytStartChannel = 0;
			
			for(dwIdx2 = bytStartChannel; dwIdx2 < bytCount; dwIdx2++)
			{
				EnterCriticalSection(&DioCriticalSection);
				eRetState = TestOutput(dwBoardIdx, dwIdx2, &eRet);
				LeaveCriticalSection(&DioCriticalSection);

				if(eRet == DIO_SUCCESS)
				{
					eIOState = eRetState;

					eIOState_Old = GetOutputStatus(dwBoardIdx, dwIdx2);
					dwFirstIO = GetIOBoardFirstOutputIdx(dwBoardIdx);
					
					if(eIOState != eIOState_Old)
					{
						dwDioIdx = dwFirstIO + dwIdx2;
						PostThreadMessage(	GetExecThreadId(), 
											PDIO_EXEC_OUTPUT_STATE, 
											dwDioIdx, 
											eIOState);
						
						SetOutputStatus(dwBoardIdx, dwIdx2, eIOState);
					}
				}
			}
		}
		
		Sleep(dwPollingDelayMs);
	}
	

	for (dwBoardIdx = 0; dwBoardIdx < dwNbBoards; dwBoardIdx++)
	{
		EnterCriticalSection(&DioCriticalSection);
			DisposeIOBoardCtrl_DI_Instance(dwBoardIdx);
			DisposeIOBoardCtrl_DO_Instance(dwBoardIdx);
		LeaveCriticalSection(&DioCriticalSection);

	}


	return 0;
}


PRIVATE void InvertInputState(DIO_STATE *pInputState)
{
	if(*pInputState == DIO_STATE_LOW)
		*pInputState = DIO_STATE_HIGH;
	else if(*pInputState == DIO_STATE_HIGH)
		*pInputState = DIO_STATE_LOW;
}
/*-------------------------------- END OF FILE ------------------------------*/