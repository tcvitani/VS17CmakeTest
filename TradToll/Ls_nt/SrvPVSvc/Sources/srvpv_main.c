/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     srvpvsvc_main.h												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/



#include <windows.h>
#include <imagehlp.h>
#include <csrlc32.h>
#include <reg.h>
#include <trc.h>
#include <stdio.h>

#define LOC_DEF
#include <srvpv_main.h>
#undef LOC_DEF

#include <ntsvc.h>
#include <pv_wm.h>
#include <route_wm.h>
#include <icon.h>
#include <db_wm.h>
#include <cmd_wm.h>
#include <conf_srvpv.h>
#include <db_connection.h>
#include <pvhook.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define SRVPVSVC_SERVICE_NAME				"SrvPVSvc"

#define SRVPVSVC_REG_ROOT					HKEY_LOCAL_MACHINE
#define SRVPVSVC_REG_KEY					NTSVC_REG_KEY_SERVICE "\\" SRVPVSVC_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM

#define SRVPVSVC_REG_VAL_UPDATE				"RealTimeUpdate"
#define SRVPVSVC_REG_VAL_REFRESH			"RefreshPeriod"
#define SRVPVSVC_REG_VAL_MSG_ID				"MsgId"

#define SRVPVSVC_REG_VAL_DB_CHECK_LINK_STAT	"UseCheckOfLinkStatus"
#define SRVPVSVC_REG_VAL_DB_LINK_STAT_TIME	"GetDbLinkStatusTime"
#define SRVPVSVC_REG_VAL_DB_MAX_STAT_ITEMS	"MaxLinkStatusItemInList"

#define TRACE_KEY_PATH						"SYSTEM\\CurrentControlSet\\Services\\SrvPVSvc\\Parameters"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

PROTECTED char *SRVSVC_DBG_FILE;
PROTECTED int SRVSVC_DBG_LINE;
/*-------------------------------- VARIABLES: -------------------------------*/

enum index_traces
{
	SRVPVSVC_TRC,
	SRVPVSVC_NB_TRACES
};


PRIVATE struct
{
	BOOL bIsDebug;			// is debug mode

	BOOL bReload;			// control interface: forcing reload
	HANDLE hWakeupEvent;	// control interface: forcing loop processing

	DWORD dwRefreshPeriod;	// main polling timeout (registry parameter)
	BOOL  dwRealTimeUpdate; // updating plaza state on the fly (registry parameter)
	DWORD dwMsgId;

	HANDLE hSrvPVThread;
	DWORD SrvPVThreadId;
	DWORD WM_SRVPV;
	HANDLE hCmdThread;
	DWORD CmdThreadId;
	DWORD WM_CMD;
	HANDLE hRouteThread;
	DWORD RouteThreadId;
	DWORD WM_ROUTE;

	BOOL  bUseCheckOfLinkStatus;
	DWORD dwGetDbLinkStatusTime;
	DWORD dwMaxLinkStatusItemInList;


}
SVC_PARAMS = { 0 };

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE BOOL SVC_Start(void);
PRIVATE BOOL SVC_Stop(void);
PRIVATE BOOL SVC_MainTreatment(void);

/*-------------------------------- CODE:      -------------------------------*/

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
NTSVC_EXT_LINK BOOL WINAPI NTSVCExternalQueryInfo(OUT char **ppcServiceName,
												  OUT NTSVCCommandMain **ppfCommand)
{
	static char gszSvcName[] = SRVPVSVC_SERVICE_NAME;

	(*ppcServiceName) = gszSvcName;
	(*ppfCommand) = NULL;

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
NTSVC_EXT_LINK void WINAPI NTSVCExternalMain(IN DWORD dwArgc, IN char **ppcArgv)
{
	BOOL	bRet		= FALSE;
	DWORD	dwLoadCount = 0;
	DWORD	dwWait		= 0;

	ICO_Init();

	// loop on start / pause -> Reload parameters at each loop
	do 
	{
		// in the case of the first loop ...
		if (dwLoadCount == 0)
		{
			// switch to the "START" state
			NTSVCSetCurrentState(SERVICE_START_PENDING, 10000, NO_ERROR);

			// wwitch to the "RUNNING" state
			NTSVCSetCurrentState(SERVICE_RUNNING, 0, NO_ERROR);

			NTSVC_ERR("NTSVCExternalMain() => START SERVICE");
		}

		// start service and all its component
		// read configuration from database
		if (SVC_Start() == FALSE)
		{
			bRet = TRUE;
			break;
		}

		NTSVCInfo("NTSVCExternalMain(), starting the main loop");

		// wait for the request to stop while managing the interface with the database
		do
		{
			// wait for the ending request or the polling timeout of the database
			dwWait = NTSVCWaitForEndOrMultipleObjects(1, &SVC_PARAMS.hWakeupEvent, SVC_PARAMS.dwRefreshPeriod);

			// trigger periodic treatment
			if (dwWait != WAIT_OBJECT_0)
				SVC_MainTreatment();
		} 
		while (dwWait == (WAIT_OBJECT_0 + 1) || dwWait == WAIT_TIMEOUT);

		NTSVCInfo("NTSVCExternalMain(), detected stop request");

		// this is a real request to stop
		if (!SVC_PARAMS.bReload)
		{
			// switch to the "STOP" state
			NTSVCSetCurrentState(SERVICE_STOP_PENDING, 10000, NO_ERROR);
		}
		// it is simply a request for reloading
		else
		{
			// send a reloas message to all connected PVs
			PV_PostMessage(PV_WM_RELOAD, NULL);

			// Wait, the time to let this message appear to the VPs
			Sleep(10000);

			NTSVCResetEnd();
		}

		NTSVC_ERR("NTSVCExternalMain() => STOP SERVICE");

		if (SVC_Stop() == FALSE)
		{
			bRet = TRUE;
			break;
		}

		dwLoadCount++;
	} 
	while (SVC_PARAMS.bReload);

	NTSVCSetCurrentState(SERVICE_STOPPED, 0, bRet);

	// all is finished
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL SVC_Start(void)
{
	DWORD				dwErr		= 0;
	NTSVC_PARAMETER_DEF	*psParams	= NULL; // registry parameters

	ZeroMemory(&SVC_PARAMS, sizeof(SVC_PARAMS));

	// get registry parameters
	psParams = NTSVCOpenParameters(
		SRVPVSVC_REG_VAL_MSG_ID,				REG_DWORD,	4,			120,	&SVC_PARAMS.dwMsgId,
		SRVPVSVC_REG_VAL_UPDATE,				REG_DWORD,	4,			1,		&SVC_PARAMS.dwRealTimeUpdate,
		SRVPVSVC_REG_VAL_REFRESH,				REG_DWORD,	4,			-1,		&SVC_PARAMS.dwRefreshPeriod,
		SRVPVSVC_REG_VAL_DB_CHECK_LINK_STAT,	REG_DWORD,	4,			0,		&SVC_PARAMS.bUseCheckOfLinkStatus,
		SRVPVSVC_REG_VAL_DB_LINK_STAT_TIME,		REG_DWORD,	_MAX_PATH,	60000,	&SVC_PARAMS.dwGetDbLinkStatusTime,
		SRVPVSVC_REG_VAL_DB_MAX_STAT_ITEMS,		REG_DWORD,	_MAX_PATH,	10,		&SVC_PARAMS.dwMaxLinkStatusItemInList,
		NULL);

	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	// initialize calculated or pre-initialized values
	SVC_PARAMS.bIsDebug = NTSVCIsDebugMode();
	SVC_PARAMS.bReload = FALSE;

	// initialize the event to wake the main loop
	SVC_PARAMS.hWakeupEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (SVC_PARAMS.hWakeupEvent == NULL)
		return FALSE;

	// initialize NTSVC for DLLs
	DB_CONNECTION_NTSVC_Inherit_Handle(NTSVCGetContext());
	CONF_NTSVC_Inherit_Handle(NTSVCGetContext());
	Pvhook_NTSVC_Inherit_Handle(NTSVCGetContext());


	// create the database thread, used to communicate to DB
	if (DB_Start() == FALSE)
		return FALSE;

	// create the PV thread, used to communicate to PV app.
	if (PV_Start() == FALSE)
		return FALSE;

	// create the ROTE thread, used to communicate to ROUTE service
	if (ROUTE_Start(SVC_PARAMS.dwRealTimeUpdate) == FALSE)
		return FALSE;

	// create the CMD thread
	if (CMD_Start() == FALSE)
		return FALSE;

	// // create the ICO thread, used to show service icon
	if (ICO_Start() == FALSE)
		return FALSE;

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL SVC_Stop(void)
{
	// thread stop => ICO
	if (ICO_Stop(-1) == FALSE)
		return FALSE;

	// thread stop => CMD
	if (CMD_Stop(-1) == FALSE)
		return FALSE;

	// thread stop => ROUTE
	if (ROUTE_Stop(-1) == FALSE)
		return FALSE;

	// thread stop => PV
	if (PV_Stop(-1) == FALSE)
		return FALSE;

	// thread stop => DB
	if (DB_Stop(-1) == FALSE)
		return FALSE;

	// free servie parameters
	CloseHandle(SVC_PARAMS.hWakeupEvent);

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL SVC_MainTreatment(void)
{
	NTSVCInfo("SVC_MainTreatment()");

	// send total plaza status message to PV application
	return ROUTE_PostMessage(ROUTE_WM_TOTAL_PLAZA_STATE, NULL);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED DWORD SVC_Get_Msg_Id(void)
{
	return SVC_PARAMS.dwMsgId;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED void SVC_Get_Time(LPSYSTEMTIME time)
{
	GetLocalTime(time);
	time->wMilliseconds = 0;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED void MAIN_Set_Reload(BOOL bReload)
{
	SVC_PARAMS.bReload = bReload;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL MAIN_Wake_Up(void)
{
	return SetEvent(SVC_PARAMS.hWakeupEvent);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL SVC_Is_Link_Status_Cheking_Used(void)
{
	return SVC_PARAMS.bUseCheckOfLinkStatus;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED DWORD SVC_Get_DB_Link_Status_Time(void)
{
	return SVC_PARAMS.dwGetDbLinkStatusTime;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED DWORD SVC_Get_Max_Nb_Stat_Items(void)
{
	return SVC_PARAMS.dwMaxLinkStatusItemInList;
}



/*-------------------------------- END OF FILE ------------------------------*/