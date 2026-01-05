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
#include <dio_unidaq_loc.h>
#include <dio_unidaq_glob.h>

/*--------------------------- RESERVED:  --------------------------*/

#include <memclass.h>

/*--------------------------- EXTERNALS: --------------------------*/


/*--------------------------- DEFINES:   --------------------------*/



/*--------------------------- TYPEDEFS:  --------------------------*/



/*--------------------------- FUNCTIONS: --------------------------*/


PRIVATE void InvertInputState(DIO_STATE *pInputState)
{
	if(*pInputState == DIO_STATE_LOW)
		*pInputState = DIO_STATE_HIGH;
	else if(*pInputState == DIO_STATE_HIGH)
		*pInputState = DIO_STATE_LOW;
}


PRIVATE BOOL ShouldInputStatusBeInverted(DWORD dwBoardIdx)
{
	return(DIO.sBoardDetails[dwBoardIdx].bInvertInputs);
}



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
	BOOL		bInitDone						= FALSE;
	BYTE		bytCount						= 0;
	BYTE		bytStartChannel					= 0;
	INT			iRet							= 0;
	INT			iHandle							= 0;
	BOOL		bInvert							= FALSE;
	
	dwPollingDelayMs = GetPollingDelayMs();
	
	dwNbBoards = GetNumberOfIOBoards();

	DIO_Trace(&DBG, "DIO_PollThread: dwNbBoards=%d, dwPollingDelayMs %d", dwNbBoards, dwPollingDelayMs);
	
	// Reset link error for all boards.
	// This board is connected in PCI slot and has no link error,
	// this is just to ensure that there is no link error on main app
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
		}
			
		//------------------------------------------------------------
		for (dwBoardIdx = 0; dwBoardIdx < dwNbBoards; dwBoardIdx++)
		{
			BYTE	abBoardGlobalOutputStatusCpy[DIO_NB_OUTPUTS_MAX] = {0};
			
			// Read digital outputs
			ZeroMemory(dwIO_Value, sizeof(dwIO_Value));
			bytCount = (BYTE)GetIOBoardNbOutputs(dwBoardIdx);
			bytStartChannel = 0;
			
			//For the optimization copy all the global statuses for the board 
			EnterGlobalRegion();
				memcpy(abBoardGlobalOutputStatusCpy, DIO_GLOB.sBoardDetails[dwBoardIdx].abOutputStatus, sizeof(abBoardGlobalOutputStatusCpy));
			LeaveGlobalRegion();


			for(dwIdx2 = bytStartChannel; dwIdx2 < bytCount; dwIdx2++)
			{
				//eRetState = GetGlobalOutputStatus(dwBoardIdx, dwIdx2);
				eRetState = abBoardGlobalOutputStatusCpy[dwIdx2];

				eIOState = eRetState;

				eIOState_Old = GetCurrentOutputStatus(dwBoardIdx, dwIdx2);
				dwFirstIO = GetIOBoardFirstOutputIdx(dwBoardIdx);
				
				if(eIOState != eIOState_Old)
				{
					dwDioIdx = dwFirstIO + dwIdx2;
					
					if(GetExecThreadId()!=0)
					{	
						PostThreadMessage(	GetExecThreadId(), 
											PDIO_EXEC_OUTPUT_STATE, 
											dwDioIdx, 
											eIOState);
					}

					SetCurrentOutputStatus(dwBoardIdx, dwIdx2, eIOState);
				}
				
			}
		}
		

		if(IsShouldExit())
		{
			if(GetExecThreadId()!=0)
			{
				PostThreadMessage(GetExecThreadId(), 
										WM_QUIT, 
										dwDioIdx, 
										eIOState);
			}

			break; //Exit the poll thread loop
		}
		
		Sleep(dwPollingDelayMs);
	}
	

	return 0;
}


/*--------------------------- END OF FILE -------------------------*/