/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     NTSVC.H                                                         */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef NTSVC_H
#define NTSVC_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef NTSVC_LIB
	#include <public.h>
	#define NTSVC_EXT_LINK extern
#else
	#include <export.h>
	#define NTSVC_EXT_LINK
#endif

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

// Délai imparti au démarrage du service. Peut être étendu par
// appels successifs de NTSVCSetCurrentState(SERVICE_START_PENDING,x,y)
#define NTSVC_START_ACKNOWLEDGE_DELAY 10000

// Délai imparti à l'arret du service. Peut être étendu par
// appels successifs de NTSVCSetCurrentState(SERVICE_STOP_PENDING,x,y)
#define NTSVC_STOP_ACKNOWLEDGE_DELAY  10000


// Actions sur le menu contextuel
#define NTSVC_MENU_INSERT_FIRST       1
#define NTSVC_MENU_INSERT_LAST        2
#define NTSVC_MENU_REMOVE             3
#define NTSVC_MENU_CLEAR_ALL          4


#define NTSVC_REG_KEY_SERVICE       "SYSTEM\\CurrentControlSet\\Services"
#define NTSVC_REG_KEY_PARAM         "Parameters"
#define NTSVC_REG_VAL_INFOFILE      "TraceInfoFile"
#define NTSVC_REG_VAL_ERRORFILE     "TraceErrorFile"
#define NTSVC_REG_VAL_TRACEFILE_MAX_SIZE     "TraceFileMaxSizeMb"
#define NTSVC_REG_VAL_EXCPTRESTART  "ExceptionRestart"


#define NTSVC_ERR(txt)           NTSVCError( "%s(%u):" txt, __FILE__, __LINE__ )
#define NTSVC_ERR1(txt,a)        NTSVCError( "%s(%u):" txt, __FILE__, __LINE__, a )
#define NTSVC_ERR2(txt,a,b)      NTSVCError( "%s(%u):" txt, __FILE__, __LINE__, a, b )
#define NTSVC_ERR3(txt,a,b,c)    NTSVCError( "%s(%u):" txt, __FILE__, __LINE__, a, b, c )
#define NTSVC_ERR4(txt,a,b,c,d)  NTSVCError( "%s(%u):" txt, __FILE__, __LINE__, a, b, c, d )

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct NTSVC_PARAMETER_DEF
{
    char   szName[MAX_PATH];
    DWORD  dwType;

    DWORD  dwSize;

    void * pvDefault;
    void * pvValue;
}
NTSVC_PARAMETER_DEF;

typedef struct NTSVC_TRAY_ICON
{
    HICON hIcon;
    HWINSTA hUsedStation;
    HDESK hUsedDesktop;
}
NTSVC_TRAY_ICON;

typedef DWORD WINAPI NTSVCCommandMain( char * pcParams );

typedef void WINAPI NTSVCTrayIconHandler( UINT uiId );

/*-------------------------------- FUNCTIONS: -------------------------------*/

NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(OUT char **ppcServiceName,
												  OUT NTSVCCommandMain **ppfCommand);

NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(IN DWORD dwArgc,
											 IN char **ppcArgv);

PUBLIC void * WINAPI NTSVCGetContext();

PUBLIC void WINAPI NTSVCSetContext(void *pvContext);

PUBLIC void WINAPI NTSVCSignalEnd();

PUBLIC void WINAPI NTSVCResetEnd();

PUBLIC DWORD WINAPI NTSVCWaitForEnd(IN DWORD dwTimeout);

PUBLIC DWORD WINAPI NTSVCWaitForEndOrMultipleObjects(IN DWORD dwEventCount,
													 IN HANDLE *phEvents,
													 IN DWORD dwTimeout);

PUBLIC DWORD WINAPI NTSVCSetCurrentState(IN DWORD dwState, 
										 IN DWORD dwWait, 
										 IN DWORD dwError);

PUBLIC BOOL WINAPI NTSVCIsDebugMode();

PUBLIC void WINAPI NTSVCInfo(char * pcFormat, ...);

PUBLIC void WINAPI NTSVCError(char * pcFormat, ...);

PUBLIC NTSVC_PARAMETER_DEF * WINAPI NTSVCOpenParameters(char * pcName, ...);

PUBLIC void WINAPI NTSVCCloseParameters(NTSVC_PARAMETER_DEF *psParams);

PUBLIC DWORD WINAPI NTSVCLoadParameters(NTSVC_PARAMETER_DEF *psParams, DWORD *pdwErrPos);

PUBLIC DWORD WINAPI NTSVCSaveParameters(NTSVC_PARAMETER_DEF *psParams, DWORD *pdwErrPos);

PUBLIC DWORD WINAPI NTSVCGetParametersCount(NTSVC_PARAMETER_DEF *psParams);

PUBLIC HANDLE WINAPI NTSVCCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
									   DWORD dwStackSize,
									   LPTHREAD_START_ROUTINE lpStartAddress,
									   LPVOID lpParameter,
									   DWORD dwCreationFlags,
									   LPDWORD lpThreadId,
									   char *pcThreadName );

PUBLIC void WINAPI NTSVCRefreshTrayIcon(HICON hIcon, char *pcFormat, ...);

PUBLIC void WINAPI NTSVCSetTrayIconHandler(NTSVCTrayIconHandler *pfHandler);

PUBLIC DWORD WINAPI NTSVCDefineTrayIconMenu(DWORD dwAction, UINT uiId, char *szText);

#endif

/*-------------------------------- END OF FILE ------------------------------*/