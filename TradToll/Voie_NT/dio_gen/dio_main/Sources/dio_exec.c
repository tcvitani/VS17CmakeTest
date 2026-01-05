/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_MAIN														 */
/* FILE:	 dio_exec.c														 */
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
		
	// Create message queue
	PeekMessage(&sMsg, NULL, FIRST_MDIO_EXEC_MSG, LAST_MDIO_EXEC_MSG, PM_NOREMOVE);

	while (TRUE)
	{
		// Wait for instructions what to do
		iMsgRet = GetMessage(&sMsg, NULL, FIRST_MDIO_EXEC_MSG, LAST_MDIO_EXEC_MSG);
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
	}

	return 0;
}

/*-------------------------------- END OF FILE ------------------------------*/