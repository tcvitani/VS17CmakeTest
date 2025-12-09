/******************* (v) 2024 emovis - All rights reserved ********************/
/*				                                                             */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     dmv_oblak_main.c															 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
/*--------------------------- INCLUDES:  ------------------------------------*/
#include <windows.h>

#define LOC_DEF
	#include <dmv_oblak.h>
#undef LOC_DEF
/*--------------------------- RESERVED:  ------------------------------------*/
/*--------------------------- EXTERNALS: ------------------------------------*/
/*--------------------------- DEFINES:   ------------------------------------*/
/*--------------------------- TYPEDEFS:  ------------------------------------*/
/*--------------------------- FUNCTIONS: ------------------------------------*/
/*--------------------------- VARIABLES: ------------------------------------*/
/*--------------------------- CODE:		 ------------------------------------*/

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
__declspec(dllexport) BOOL  DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
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

/*--------------------------- END OF FILE -----------------------------------*/