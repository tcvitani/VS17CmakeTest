/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     NTSVC_INSTALL.H                                                 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef NTSVC_MGMT_H
#define NTSVC_MGMT_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define NTSVC_MAX_WND_ITEMS 16

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct _NTSVC_WORK
{
    BOOL                        bInitialized;
    BOOL                        bRestart;
    DWORD                       dwStartTimeout;
    DWORD                       dwStopTimeout;
    SERVICE_STATUS_HANDLE       hStatus;
    CRITICAL_SECTION            sIconSerializer;
    char                        szServiceName[256];
    LPSERVICE_MAIN_FUNCTION     pfServiceProc;
    BOOL                        bIsRunning;
    HANDLE                      hEndEvent;
    DWORD                       dwExcptRestart;
    char                        szErrorFile[MAX_PATH];
    char                        szInfoFile[MAX_PATH];
	DWORD						dwTraceMaxSizeMb;
    TRC_EMETTEUR                hError;
    TRC_EMETTEUR                hInfo;
    SERVICE_STATUS              sStatus;
    HWND                        hWnd;
    HANDLE                      hWndThread;
    DWORD                       dwWndhread;
    UINT                        uiWndNotify;
    BOOL                        fWndClose;
    NTSVCTrayIconHandler      * pfWndHandler;
    DWORD                       dwWndItems;
    struct 
	{
        UINT uiId;
        char szText[64];
    }tsWndItem[NTSVC_MAX_WND_ITEMS];
}
NTSVC_WORK;

typedef struct _NTSVC_FLAGS
{
    BOOL    bConsole;
    BOOL    bSimulated;
    char	*pcServiceName;
}
NTSVC_FLAGS;
   
typedef struct _NTSVC_MAIN
{
    NTSVC_WORK sWork;
    NTSVC_FLAGS sFlags;
}
NTSVC_MAIN;

typedef struct _NTSVC_THREAD_DATA
{
    LPTHREAD_START_ROUTINE pfThread;
    LPVOID      pvParameter;
    char        szName[200];
}
NTSVC_THREAD_DATA;

PROTECTED NTSVC_MAIN gsMain;
PROTECTED NTSVC_MAIN * gpsMain;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD WINAPI NTSVCStart( 
        IN      char * pcServiceName, 
        IN      LPSERVICE_MAIN_FUNCTION pfServiceProc, 
        IN      DWORD dwStartTimeout,
        IN      DWORD dwStopTimeout,
        IN      char * pcSimulateParams );

PROTECTED void WINAPI NTSVCThreadExceptionHandler(DWORD dwExcpt, char *szThreadName);

#endif

/*-------------------------------- END OF FILE ------------------------------*/