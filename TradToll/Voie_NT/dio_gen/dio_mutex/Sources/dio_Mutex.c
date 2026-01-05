/***************** (v) 2016 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_ioLogik1200, DIO_ioLogikE4200								 */
/* FILE:	 dio_Mutex.c													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/*		This file contains functions for handling with Windows WINAPI mutex. */
/*		It is used to resolve problem with Windows freezing when several	 */
/*		instances of dlls that handles MOXA ioLogik DIO cards are started.	 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <stdio.h>

#include <CSRLC32.H>
#include <trc.h>
#include <reg.h>
#include <noyau.h>

#define DIO_MUTEX_DEF
#include <dio_Mutex.h>
#undef DIO_MUTEX_DEF
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define DBG_NAME		"DIO_MUTEX"

#define MUTEX_NAME		"DIOMutex"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE BOOL DIO_InitTraces(struct_dio_mutex_debug *dbg);
PRIVATE void DIO_Error(IN struct_dio_mutex_debug *dbg, IN char *fmt, ...);
PRIVATE void DIO_Trace(IN struct_dio_mutex_debug *dbg, IN char *fmt, ...);

PRIVATE enum_dio_mutex InitDioMutex();
PRIVATE enum_dio_mutex DeinitDioMutex();

/*-------------------------------- VARIABLES: -------------------------------*/

struct_dio_mutex_debug	DBG		= {0};

HANDLE			ghMutex = NULL;

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
			DIO_InitTraces(&DBG);
			InitDioMutex();
			break;
			
		// The attached process creates a new thread.
		case DLL_THREAD_ATTACH:
			break;
		
		// The thread of the attached process terminates.
		case DLL_THREAD_DETACH:
			break;
		
		// The DLL unloading due to process termination or call to FreeLibrary.
		case DLL_PROCESS_DETACH:
			DeinitDioMutex();
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
/*SYNTAX: BOOL DIO_InitTraces (struct_dio_mutex_debug *dbg)		   */
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
PRIVATE BOOL DIO_InitTraces(struct_dio_mutex_debug *dbg)
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
/*SYNTAX: enum_dio_mutex InitDioMutex()							   */
/*=================================================================*/
/*TYPE:   PROTECTED			                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Initialization of DIO Mutex.      			       */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_MUTEX_OK			- successful initialization			   */
/*	DIO_MUTEX_NOK			- initialization failed				   */
/*******************************************************************/
PROTECTED enum_dio_mutex InitDioMutex()
{
    // Create a mutex with no initial owner, shared between the DLL instances to simulate regions.
	// WARNING: This function may not write to error file if called before InitTraces().
	ghMutex = CreateMutex(	NULL,              // default security attributes
							FALSE,             // initially not owned
							MUTEX_NAME);       // name of mutex
	if (ghMutex == NULL)
	{
		DIO_Error(&DBG, "InitDioMutex() -> Mutex handle is NULL, GetLastError returned %d. This dll will exit bad!, ghMutex = [%p], Thread id = [%d]", GetLastError(), ghMutex, GetCurrentThreadId());
		ExitBad();
	}
	
	DIO_Trace(&DBG, "InitDioMutex() -> CreateMutex OK");
	DIO_Error(&DBG, "InitDioMutex() -> CreateMutex OK, ghMutex = [%p], Thread id = [%d]", ghMutex, GetCurrentThreadId());
	return DIO_MUTEX_OK;
}

/**/
/*******************************************************************/
/*SYNTAX: enum_dio_mutex DeinitDioMutex()						   */
/*=================================================================*/
/*TYPE:   PROTECTED			                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            Deinitialization of DIO Mutex (closing handle).      */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_MUTEX_OK			- successful deinitialization		   */
/*	DIO_MUTEX_NOK			- deinitialization failed			   */
/*******************************************************************/
PROTECTED enum_dio_mutex DeinitDioMutex()
{
	if(ghMutex != NULL && CloseHandle(ghMutex) == 0)
	{
		DIO_Error(&DBG, "DeinitDioMutex() -> CloseHandle() function failed. GetLastError returned %d., ghMutex = [%p], Thread id = [%d]", GetLastError(), ghMutex, GetCurrentThreadId());
		return DIO_MUTEX_NOK;
	}

	
	DIO_Trace(&DBG, "DeinitDioMutex() -> CloseHandle OK");
	DIO_Error(&DBG, "DeinitDioMutex() -> CloseHandle OK, ghMutex = [%p], Thread id = [%d]", ghMutex, GetCurrentThreadId());
	return DIO_MUTEX_OK;
}

/**/
/*******************************************************************/
/*SYNTAX: enum_dio_mutex DioMutexStartRegion()					   */
/*=================================================================*/
/*TYPE:   PUBLIC			                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function gets ownership of the mutex 		   */
/*			  when other threads to finish with it.			       */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_MUTEX_OK			- successful						   */
/*******************************************************************/
PUBLIC enum_dio_mutex WINAPI DioMutexStartRegion()
{
	enum_dio_mutex	bRet			= DIO_MUTEX_NOK;
	DWORD			dwWaitResult	= 0;
	
	dwWaitResult = WaitForSingleObject(	ghMutex,    // handle to mutex
										INFINITE);	// no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			bRet = DIO_MUTEX_OK;
			break;

		// The thread got ownership of an abandoned mutex. Function is in an indeterminate state.
		case WAIT_ABANDONED:
			DIO_Error(&DBG, "DioMutexStartRegion() -> Thread got ownership of an abandoned mutex. Check that some other threads released mutex before terminated. This dll will exit bad!, ghMutex = [%p], Thread id = [%d]", ghMutex, GetCurrentThreadId());
			ExitBad();
			break;
			
		// The time-out interval elapsed, and the object's state is nonsignaled.	
		case WAIT_TIMEOUT:
			DIO_Error(&DBG, "DioMutexStartRegion() -> The time-out interval elapsed, and the object's state is nonsignaled. This dll will exit bad!, ghMutex = [%p], Thread id = [%d]", ghMutex, GetCurrentThreadId());
			ExitBad();
			break;

		// The function has failed. To get extended error information, call GetLastError.
		case WAIT_FAILED:
			DIO_Error(&DBG, "DioMutexStartRegion() -> WaitForSingleObject function has failed. GetLastError returned %d. This dll will exit bad!, ghMutex = [%p], Thread id = [%d]", GetLastError(), ghMutex, GetCurrentThreadId());
			ExitBad();
			break;
	}
	return bRet;
}

/**/
/*******************************************************************/
/*SYNTAX: enum_dio_mutex DioMutexEndRegion()					   */
/*=================================================================*/
/*TYPE:   PUBLIC			                                       */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function gets ownership of the mutex 		   */
/*			  when other threads to finish with it.			       */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*	DIO_MUTEX_OK			- successful						   */
/*******************************************************************/
PUBLIC enum_dio_mutex WINAPI DioMutexEndRegion()
{
	// The ReleaseMutex function fails if the calling thread does not own the mutex object.
	if(ReleaseMutex(ghMutex) == 0)
	{
		DIO_Error(&DBG, "DioMutexEndRegion() -> ReleaseMutex function failed. Calling thread does not own the mutex object. GetLastError returned [%d], ghMutex = [%p], Thread id = [%d]", GetLastError(), ghMutex, GetCurrentThreadId());
		return DIO_MUTEX_NOK;
	}
	
	return DIO_MUTEX_OK;
}

/**/
/*******************************************************************/
/*SYNTAX: void DIO_Error (struct_dio_mutex_debug *dbg,			   */
/*							IN char *fmt, ...)					   */
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
PRIVATE void DIO_Error(struct_dio_mutex_debug *dbg, IN char *fmt, ...)
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
/*SYNTAX: void DIO_Trace(struct_dio_mutex_debug,				   */
/*							IN char *fmt, ...)					   */
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
PRIVATE void DIO_Trace (struct_dio_mutex_debug *dbg, IN char *fmt, ...)
{
    va_list ap;
    
    va_start(ap,fmt);

    if(dbg->traces != NULL)
        TRC_Trace_V(dbg->traces, TRC_OPT_MASK, NULL, 0, fmt, ap);
	
    va_end(ap);
}
/*-------------------------------- END OF FILE ------------------------------*/