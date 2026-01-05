/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_MAIN														 */
/* FILE:	 dio_poll.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

#include <dio_main.h>

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
	DWORD		dwIdx								= 0;
	DWORD		dwMaxSize							= 0;
	DWORD		dwNbInputs							= 0;
	DWORD		dwNbOutputs							= 0;
	DIO_STATE	eInputsState[DIO_MAIN_MAX_IO]		= {0};
	DIO_STATE	eInputsState_Old[DIO_MAIN_MAX_IO]	= {0};
    DIO_STATE	eOutputsState[DIO_MAIN_MAX_IO]		= {0};
    DIO_STATE	eOutputsState_Old[DIO_MAIN_MAX_IO]	= {0};

	dwNbInputs = GetIONbInputs();
	dwNbOutputs = GetIONbOutputs();

	while (TRUE)
	{
		// Read digital inputs
		dwMaxSize = sizeof(eInputsState);
		ZeroMemory(eInputsState, dwMaxSize);

		MDioGetSimuInputStatusAll(eInputsState, dwMaxSize);
		
		if (memcmp(eInputsState, eInputsState_Old, sizeof(eInputsState)) != 0)
		{
			for(dwIdx = 0; dwIdx < dwNbInputs; dwIdx++)
			{					
				if(eInputsState[dwIdx] != eInputsState_Old[dwIdx])
				{
					PostThreadMessage(GetExecThreadId(), 
									MDIO_EXEC_INPUT_STATE, 
									dwIdx, 
									eInputsState[dwIdx]);
				}	
			}

			memcpy(eInputsState_Old, eInputsState, sizeof(eInputsState_Old));
		}


		// Read digital outputs
		dwMaxSize = sizeof(eOutputsState);
		ZeroMemory(eOutputsState, dwMaxSize);
		
		MDioGetSimuOutputStatusAll(eOutputsState, dwMaxSize);
		
		if (memcmp(eOutputsState, eOutputsState_Old, sizeof(eInputsState)) != 0)
		{
			for(dwIdx = 0; dwIdx < dwNbInputs; dwIdx++)
			{					
				if(eOutputsState[dwIdx] != eOutputsState_Old[dwIdx])
				{
					PostThreadMessage(GetExecThreadId(), 
									MDIO_EXEC_OUTPUT_STATE, 
									dwIdx, 
									eOutputsState[dwIdx]);
				}	
			}

			memcpy(eOutputsState_Old, eOutputsState, sizeof(eOutputsState_Old));
		}

		Sleep(10);
	}

	return 0;
}

/*-------------------------------- END OF FILE ------------------------------*/