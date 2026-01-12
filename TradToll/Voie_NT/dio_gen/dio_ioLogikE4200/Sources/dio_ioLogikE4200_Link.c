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
/*SYNTAX: DWORD WINAPI DIO_LinkThread(LPVOID lpParameter)		   */
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
PROTECTED DWORD WINAPI DIO_LinkThread(LPVOID lpParameter)
{
	DIO_ERROR	eRet				= DIO_SUCCESS;
	DWORD		dwNbBoards			= 0;
	DWORD		dwIdx				= 0;
	DWORD		dwLinkCheckDelayMs	= 0;
	static BOOL	bInitDone			= FALSE;

	dwLinkCheckDelayMs = GetLinkCheckDelayMs();

	dwNbBoards = GetNumberOfIOBoards();
	
	while(TRUE)
	{
		// check connection status and try to connect
		// if link is broken
		for (dwIdx = 0; dwIdx < dwNbBoards; dwIdx++)
		{
			if (!IOBoardConnectionOK(dwIdx))
			{
				eRet = TryToConnect(dwIdx);
				if (eRet == DIO_SUCCESS)
					ResetIOBoardConnectionError(dwIdx);
				else
				{
					if(!bInitDone) // just to send link error on init
						SetIOBoardConnectionError(dwIdx);
				}
			}
		}
		
		if(!bInitDone)
		{
			bInitDone = TRUE;
			SetInitDone();
		}

		Sleep(dwLinkCheckDelayMs);
	}

	return 0;
}

/*-------------------------------- END OF FILE ------------------------------*/