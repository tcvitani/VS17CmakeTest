/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 dio_pcl1750													 */
/* FILE:	 dio_pci1750_Exec.c												 */
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


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/


/*******************************************************************/
/*SYNTAX: DWORD WINAPI DIO_ExecThread(LPVOID lpParameter)		   */
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
/*  0                     This function always returns zero.	   */
/*******************************************************************/
PROTECTED DWORD WINAPI DIO_ExecThread(LPVOID lpParameter)
{
	MSG	sMsg	= {0};
	INT	iMsgRet	= FALSE;
	
	// send configuration
	ExecuteCallbackFnc(PDIO_EXEC_PLUGIN_CONFIG, 0, 0);

	// Create message queue
	PeekMessage(&sMsg, NULL, FIRST_PDIO_EXEC_MSG, LAST_PDIO_EXEC_MSG, PM_NOREMOVE);

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

/*-------------------------------- END OF FILE ------------------------------*/