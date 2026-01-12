/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_ioLogikE4200												 */
/* FILE:	 dio_ioLogikE4200_Poll.c										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

#include <mxio.h>

#include <dio_ioLogikE4200.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


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
	DIO_STATE	eIOState_Old					= DIO_STATE_LOW;
	DWORD		dwNbBoards						= 0;
	DWORD		dwIdx							= 0;
	DWORD		dwIdx2							= 0;
	DWORD		dwFirstIO						= 0;
	DWORD		dwDioIdx						= 0;
	DWORD		dwSocketErr[DIO_NB_BOARDS_MAX]	= {0};
	DWORD		dwIO_Value[16]					= {0};
	DWORD		dwMaxSocketErrors				= 0;
	DWORD		dwPollingDelayMs				= 0;
	static BOOL bInitDone						= FALSE;
	BYTE		bytCount						= 0;
	BYTE		bytStartChannel					= 0;
	INT			iRet							= 0;
	INT			iHandle							= 0;
	INT			iSlot							= 0;
	INT			iNbSlots						= 0;
	INT			iNbSlotInputs					= 0;
	INT			i								= 0;
	struct_slot_settings* sSlotSettings			= {0};

	dwPollingDelayMs = GetPollingDelayMs();
	
	dwNbBoards = GetNumberOfIOBoards();

	while (TRUE)
	{
		if(!bInitDone)
		{
			bInitDone = IsInitDone();
			continue;
		}

		for (dwIdx = 0; dwIdx < dwNbBoards; dwIdx++)
		{
			dwMaxSocketErrors = GetMaxSocketErrors(dwIdx);

			if (IOBoardConnectionOK(dwIdx))
			{
				iHandle = GetIOBoardConnectionHandle(dwIdx);

				do 
				{
					// Read digital inputs
					ZeroMemory(dwIO_Value, sizeof(dwIO_Value));
					bytCount = (BYTE)GetIOBoardNbInputs(dwIdx);
					bytStartChannel = 0;

					iNbSlots =  GetIOBoardNbSlots(dwIdx);

					// For every slot check that it is slot with inputs and then check inputs
					for (i = 0; i < iNbSlots; i++)
					{
						sSlotSettings = GetSlotSettingsPtr(dwIdx, i);

						// If slot contains inputs, read input value
						if (sSlotSettings->dwSlotNbInputs > 0)
						{
							EnterCriticalSection(&DioCriticalSection);
							iRet = E42_DI_Reads(iHandle, (BYTE)sSlotSettings->iSlotPosition, bytStartChannel, (BYTE)sSlotSettings->dwSlotNbInputs, dwIO_Value);
							LeaveCriticalSection(&DioCriticalSection);

							if (iRet == MXIO_OK)
							{	
								dwSocketErr[dwIdx] = 0;
								
								for(dwIdx2 = bytStartChannel; dwIdx2 < (BYTE)sSlotSettings->dwSlotNbInputs; dwIdx2++)
								{
									if ((dwIO_Value[0]&(1 << dwIdx2)?1:0) > 0)
										eIOState = DIO_STATE_HIGH;
									else
										eIOState = DIO_STATE_LOW;
									
									eIOState_Old = GetInputStatus(dwIdx, dwIdx2 + sSlotSettings->dwSlot1stInputIdx);
									dwFirstIO = GetIOBoardFirstInputIdx(dwIdx);
									
									if(eIOState != eIOState_Old)
									{
										dwDioIdx = dwFirstIO + dwIdx2 + sSlotSettings->dwSlot1stInputIdx;
										PostThreadMessage(GetExecThreadId(), 
											PDIO_EXEC_INPUT_STATE, 
											dwDioIdx, 
											eIOState);
										
										SetInputStatus(dwIdx, dwIdx2 + sSlotSettings->dwSlot1stInputIdx, eIOState);
									}	
								}
							}
							else
							{
								if (iRet == EIO_SOCKET_DISCONNECT)
								{
									dwSocketErr[dwIdx]++;
									
									if(dwSocketErr[dwIdx] == dwMaxSocketErrors)
									{
										dwSocketErr[dwIdx] = 0;
										
										if (IOBoardConnectionOK(dwIdx))
											SetIOBoardConnectionError(dwIdx);
										
										break;
									}	
								}
							}
						}
						// If slot contains outputs, read output value
						else if (sSlotSettings->dwSlotNbOutputs > 0)
						{
							// Read digital outputs
							ZeroMemory(dwIO_Value, sizeof(dwIO_Value));
							bytCount = (BYTE)GetIOBoardNbOutputs(dwIdx);
							bytStartChannel = 0;
							
							EnterCriticalSection(&DioCriticalSection);
							iRet = E42_DO_Reads(iHandle, (BYTE)sSlotSettings->iSlotPosition, bytStartChannel, (BYTE)sSlotSettings->dwSlotNbOutputs, dwIO_Value);
							LeaveCriticalSection(&DioCriticalSection);
							if (iRet == MXIO_OK)
							{
								dwSocketErr[dwIdx] = 0;
								
								for(dwIdx2 = bytStartChannel; dwIdx2 < (BYTE)sSlotSettings->dwSlotNbOutputs; dwIdx2++)
								{
									if ((dwIO_Value[0]&(1 << dwIdx2)?1:0) > 0)
										eIOState = DIO_STATE_HIGH;
									else
										eIOState = DIO_STATE_LOW;
									
									eIOState_Old = GetOutputStatus(dwIdx, dwIdx2 + sSlotSettings->dwSlot1stOutputIdx);
									dwFirstIO = GetIOBoardFirstOutputIdx(dwIdx);
									
									if(eIOState != eIOState_Old)
									{
										dwDioIdx = dwFirstIO + dwIdx2 + sSlotSettings->dwSlot1stOutputIdx;
										PostThreadMessage(GetExecThreadId(), 
											PDIO_EXEC_OUTPUT_STATE, 
											dwDioIdx, 
											eIOState);
										
										SetOutputStatus(dwIdx, dwIdx2 + sSlotSettings->dwSlot1stOutputIdx, eIOState);
									}	
								}
							}
							else
							{
								if (iRet == EIO_SOCKET_DISCONNECT)
								{
									dwSocketErr[dwIdx]++;
									
									if(dwSocketErr[dwIdx] == dwMaxSocketErrors)
									{
										dwSocketErr[dwIdx] = 0;
										
										if (IOBoardConnectionOK(dwIdx))
											SetIOBoardConnectionError(dwIdx);
										
										break;
									}	
								}
							}
						}
						else
						{
							PostThreadMessage(GetExecThreadId(), 
								PDIO_EXEC_INPUT_OUTPUT_CONFIG_ERR, 
								dwDioIdx,
								i);
						}
					}
					
				}while(FALSE);	
			}
		}

		Sleep(dwPollingDelayMs);
	}

	return 0;
}

/*-------------------------------- END OF FILE ------------------------------*/