/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     cmd.c															 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <stdio.h>
#include <acom.h>
#include <ntsvc.h>
#include <srvpv_main.h>
#include <cmd_acom.h>
#include <route_wm.h>
#include <pv_wm.h>
#include <pv_acom.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define CMD_MAX_SIZE 256

#define SRVPVSVC_PROMPT_TITLE				"SrvPVSvc Service (c) 2017 EMOVIS"
#define SRVPVSVC_PROMPT_VERSION				"Software release v.11.2.3"
#define SRVPVSVC_PROMPT_DATE				"Compiled " __DATE__ " at " __TIME__
#define SRVPVSVC_PROMPT_COMMENTS			"PV server"

#define CMD_REG_VAL_MAXCMDCNX				"MaxCmdCnx"
#define CMD_REG_VAL_CMDWORKERS				"SrvCmdWorkers"
#define CMD_REG_VAL_CMDENABLED				"CmdSvcEnabled"

#define PV_REG_VAL_MAXPVCNX					"MaxPVCnx"
#define PV_REG_VAL_PVWORKERS				"SrvPVWorkers"
#define PV_REG_VAL_PVWORKERS				"SrvPVWorkers"

#define ALARM_REG_VAL_TEST_DATE				"TestAlarmDate"
#define ALARM_REG_VAL_LANE_COUNTER			"LaneAlarmCounterType"

#define LANE_REG_VAL_RAZ_ALL_INFO			"LaneInfoRazOnComFailure"

#define DB_REG_VAL_DB_CONNECTION            "PermanentDbConnection"
#define DB_REG_VAL_DB_RECONNECTION_DELAY    "DbReconnectionDelay"

#define DB_REG_VAL_DBUSR                    "DbUser"
#define DB_REG_VAL_DBPWD                    "DbPassword"
#define DB_REG_VAL_DBINST                   "DbInstance"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

PRIVATE struct CmdParams
{
	NTSVC_PARAMETER_DEF *psParams;

	// parametres
	IN DWORD dwTimeToReconnect;
	IN DWORD dwMaxQueuedMessages;
	IN DWORD dwMaxMsgSize;
	IN DWORD dwPipeBufferSize;
	IN DWORD dwMaxLife;
	IN DWORD dwLifeTime;

	IN DWORD dwMaxCMDCnx;
	IN DWORD dwCmdWorkers;
	IN DWORD dwCmdSvcEnabled;

	IN DWORD dwMaxPVCnx;
	IN DWORD dwPvWorkers;

	IN BOOL bTestDate;
	IN DWORD LaneAlarmCounterType;

	IN BOOL bRazAllInfo;

	IN DWORD dwPermanentDBConnection;
	IN DWORD dwDBReconnectionDelay;

	IN char szDbUsr[MAX_PATH];
	IN char szDbPwd[MAX_PATH];
	IN char szDbInst[MAX_PATH];
}
CMD_PARAMS;

/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

#pragma warning (disable : 4996)

PROTECTED BOOL CmdOpenParamList(void)
{
	DWORD dwErr = 0;

	// Définition des paramètres

	CMD_PARAMS.psParams = NTSVCOpenParameters(
		CMD_REG_VAL_MAXCMDCNX,				REG_DWORD,	4,			1,									&CMD_PARAMS.dwMaxCMDCnx,
		CMD_REG_VAL_CMDWORKERS,				REG_DWORD,	4,			1,									&CMD_PARAMS.dwCmdWorkers,
		CMD_REG_VAL_CMDENABLED,				REG_DWORD,	4,			1,									&CMD_PARAMS.dwCmdSvcEnabled,
		PV_REG_VAL_MAXPVCNX,				REG_DWORD,	4,			PV_REG_VAL_MAXPVCNX_DEFAULT,		&CMD_PARAMS.dwMaxPVCnx,
		PV_REG_VAL_PVWORKERS,				REG_DWORD,	4,			PV_REG_VAL_PVWORKERS_DEFAULT,		&CMD_PARAMS.dwPvWorkers,
		SVC_REG_VAL_RECONNECTTIME,			REG_DWORD,	4,			SVC_REG_VAL_RECONNECTTIME_DEFAULT,	&CMD_PARAMS.dwTimeToReconnect,
		SVC_REG_VAL_QUEUESIZE,				REG_DWORD,	4,			SVC_REG_VAL_QUEUESIZE_DEFAULT,		&CMD_PARAMS.dwMaxQueuedMessages,
		SVC_REG_VAL_MAXMSGSIZE,				REG_DWORD,	4,			SVC_REG_VAL_MAXMSGSIZE_DEFAULT,		&CMD_PARAMS.dwMaxMsgSize,
		SVC_REG_VAL_BUFFERSIZE,				REG_DWORD,	4,			SVC_REG_VAL_BUFFERSIZE_DEFAULT,		&CMD_PARAMS.dwPipeBufferSize,
		SVC_REG_VAL_MAXLIFE,				REG_DWORD,	4,			SVC_REG_VAL_MAXLIFE_DEFAULT,		&CMD_PARAMS.dwMaxLife,
		SVC_REG_VAL_LIFETIME,				REG_DWORD,	4,			SVC_REG_VAL_LIFETIME_DEFAULT,		&CMD_PARAMS.dwLifeTime,
		ALARM_REG_VAL_TEST_DATE,			REG_DWORD,	4,			0,									&CMD_PARAMS.bTestDate,
		ALARM_REG_VAL_LANE_COUNTER,			REG_DWORD,	4,			1,									&CMD_PARAMS.LaneAlarmCounterType,
		LANE_REG_VAL_RAZ_ALL_INFO,			REG_DWORD,	4,			0,									&CMD_PARAMS.bRazAllInfo,
		DB_REG_VAL_DB_CONNECTION,			REG_DWORD,	4,			1,									&CMD_PARAMS.dwPermanentDBConnection,
		DB_REG_VAL_DB_RECONNECTION_DELAY,	REG_DWORD,	4,			5000,								&CMD_PARAMS.dwDBReconnectionDelay,
		DB_REG_VAL_DBUSR,					REG_SZ,		MAX_PATH,	"PVUSER",							&CMD_PARAMS.szDbUsr,
		DB_REG_VAL_DBPWD,					REG_SZ,		MAX_PATH,	"PVPWD",							&CMD_PARAMS.szDbPwd,
		DB_REG_VAL_DBINST,					REG_SZ,		MAX_PATH,	"ENTER DATA BASE NAME HERE !",		&CMD_PARAMS.szDbInst,
		NULL);

	if (CMD_PARAMS.psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(CMD_PARAMS.psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

PROTECTED BOOL CmdCloseParamList(void)
{
	NTSVCCloseParameters(CMD_PARAMS.psParams);

	return TRUE;
}

PRIVATE char * CmdTrim(char * pcStr)
{
	char * pcSrc = pcStr;
	char * pcDst = pcStr;
	char * pcStop = NULL;

	while (*pcSrc == ' ') pcSrc++;
	while (*pcSrc != '\0')
	{
		*pcDst = *pcSrc;
		if ((pcStop == NULL) && (*pcDst == ' '))
			pcStop = pcDst;
		else if (*pcDst != ' ')
			pcStop = NULL;
		pcDst++;
		pcSrc++;
	}
	if (pcStop != NULL)
		*pcStop = '\0';
	else
		*pcDst = '\0';
	return pcStr;
}

PRIVATE BOOL CmdCheck(char * pcCmd, char * pcMsg, DWORD dwMsgSize, DWORD dwParams, ...)
{
	DWORD dwLen = (DWORD)strlen(pcCmd);      // Longueur du mot de commande
	va_list pMark;
	char * pcParam;
	BOOL bEnd;
	char szMsg[CMD_MAX_SIZE + 1];
	char szParam[CMD_MAX_SIZE + 1];

	if (dwMsgSize > CMD_MAX_SIZE)
		dwMsgSize = CMD_MAX_SIZE;
	memcpy(szMsg, pcMsg, dwMsgSize);
	szMsg[dwMsgSize] = '\0';
	pcMsg = szMsg;

	// Supprimer les espaces de tête et de fin
	pcMsg = CmdTrim(szMsg);

	// Comparer le début du message avec la commande à tester
	if (_strnicmp(pcCmd, pcMsg, dwLen) != 0)
		return FALSE;

	// Si ce n'est pas une fin de chaine ou un espace qui suit la commande, ce n'est pas
	// la command qu'on cherche
	pcMsg += dwLen;
	if ((*pcMsg != '\0') && (*pcMsg != ' '))
		return FALSE;

	if (dwParams != 0)
		va_start(pMark, dwParams);
	bEnd = FALSE;

	// Analyser les paramètres
	do
	{
		// Supprimer les espaces de tête
		CmdTrim(pcMsg);
		dwLen = 0;

		// Rechercher séparateur tout en recopiant
		while ((*pcMsg != ',') && (*pcMsg != '\0'))
			szParam[dwLen++] = *(pcMsg++);
		szParam[dwLen] = '\0';
		dwLen = (DWORD)strlen(CmdTrim(szParam));

		if (*pcMsg == '\0')
			bEnd = TRUE;
		else
			pcMsg++;

		// Il n'y a plus de paramètres et c'est la fin
		if (bEnd && (dwLen == 0) && (dwParams == 0))
			return TRUE;

		if (dwParams == 0)
			return FALSE;

		pcParam = va_arg(pMark, char *);
		strcpy(pcParam, szParam);
		dwParams--;
	} while (!bEnd);

	return (dwParams == 0);
}

PROTECTED void CmdResponse(DWORD hCnx, char * pcRsp, ...)
{
	int iLen;
	va_list lParm;      // Pour les paramètres du format
	char szMsg[1000];   // Buffer du message

	// Placer le curseur des paramètres au premier
	va_start(lParm, pcRsp);

	// Remplir le buffer du message avec la chaine formatée
	iLen = _vsnprintf_s(szMsg, sizeof(szMsg), sizeof(szMsg), pcRsp, lParm);
	if (iLen < 0) iLen = sizeof(szMsg);

	// Envoyer la chaine sur la connexion
	CMD_ACOM_Send_Buffer(hCnx, (DWORD)iLen, szMsg);
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteCommandList(DWORD hCnx)
{
	CmdResponse(hCnx, "COMMANDLISTBEGIN NAME[,PARAMS]");
	CmdResponse(hCnx, "COMMANDLISTITEM COMMANDLIST");
	CmdResponse(hCnx, "COMMANDLISTITEM PROMPT");
	CmdResponse(hCnx, "COMMANDLISTITEM STOP");
	CmdResponse(hCnx, "COMMANDLISTITEM RELOAD");
	CmdResponse(hCnx, "COMMANDLISTITEM PARAMLIST");
	CmdResponse(hCnx, "COMMANDLISTITEM SETPARAM,NAME|VALUE");
	CmdResponse(hCnx, "COMMANDLISTITEM SAVEPARAMS");
	CmdResponse(hCnx, "COMMANDLISTITEM LISTUSERS");
	CmdResponse(hCnx, "COMMANDLISTITEM NOP");
	CmdResponse(hCnx, "COMMANDLISTITEM REFRESH PV");
	CmdResponse(hCnx, "COMMANDLISTEND");
}

// --------------------------------------------------------------------

PRIVATE void CmdExecutePrompt(DWORD hCnx)
{
	CmdResponse(hCnx, "PROMPTBEGIN");
	CmdResponse(hCnx, "PROMPTITEM TITLE    : %s", SRVPVSVC_PROMPT_TITLE);
	CmdResponse(hCnx, "PROMPTITEM VERSION  : %s", SRVPVSVC_PROMPT_VERSION);
	CmdResponse(hCnx, "PROMPTITEM DATE     : %s", SRVPVSVC_PROMPT_TITLE);
	CmdResponse(hCnx, "PROMPTITEM COMMENTS : %s", SRVPVSVC_PROMPT_COMMENTS);
	CmdResponse(hCnx, "PROMPTEND");
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteStop(DWORD hCnx)
{
	// Renvoyer la réponse.
	// Du fait de l'assynchronisme, il y a des chances que l'application
	// soit arrétée avant l'émission du message.
	CmdResponse(hCnx, "STOPOK");

	// Demander la fin
	NTSVCSignalEnd();
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteReload(DWORD hCnx)
{
	// Renvoyer la réponse.
	// Du fait de l'assynchronisme, il y a des chances que l'application
	// soit arrétée avant l'émission du message.
	CmdResponse(hCnx, "RELOADOK");

	MAIN_Set_Reload(TRUE);

	// Demander la fin
	NTSVCSignalEnd();
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteParamList(DWORD hCnx)
{
	DWORD dwScan;
	DWORD dwCount = NTSVCGetParametersCount(CMD_PARAMS.psParams);
	NTSVC_PARAMETER_DEF * psParam;

	CmdResponse(hCnx, "PARAMLISTBEGIN NAME,VALUE");
	for (dwScan = 0; dwScan < dwCount; dwScan++)
	{
		psParam = &CMD_PARAMS.psParams[dwScan];
		if (psParam->dwType == REG_SZ)
			CmdResponse(hCnx, "PARAMLISTITEM %s,%s", psParam->szName, psParam->pvValue);
		else if (psParam->dwType == REG_DWORD)
			CmdResponse(hCnx, "PARAMLISTITEM %s,%u", psParam->szName, *(DWORD*)psParam->pvValue);
		else
			CmdResponse(hCnx, "PARAMLISTITEM %s,-", psParam->szName);
	}
	CmdResponse(hCnx, "PARAMLISTEND");
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteSetParam(DWORD hCnx, char * pcName, char * pcValue)
{
	DWORD dwCount = NTSVCGetParametersCount(CMD_PARAMS.psParams);
	DWORD dwItem;
	NTSVC_PARAMETER_DEF * psParam;

	for (dwItem = 0; dwItem < dwCount; dwItem++)
	{
		psParam = &CMD_PARAMS.psParams[dwItem];
		if (_stricmp(pcName, psParam->szName) == 0)
		{
			if (psParam->dwType == REG_SZ)
			{
				strzcpy(((char*)psParam->pvValue), psParam->dwSize, pcValue, psParam->dwSize);
				((char*)psParam->pvValue)[psParam->dwSize - 1] = '\0';
				CmdResponse(hCnx, "SETPARAMOK");
			}
			else if (psParam->dwType == REG_DWORD)
			{
				*(DWORD*)psParam->pvValue = atol(pcValue);
				CmdResponse(hCnx, "SETPARAMOK");
			}
			else
				CmdResponse(hCnx, "SETPARAMERROR %u , Unknown parameter type %u for [%s]", ERROR_INVALID_PARAMETER, psParam->dwType, pcName);
			break;
		}
	}
	if (dwItem >= dwCount)
		CmdResponse(hCnx, "SETPARAMERROR %u , Unknown parameter [%s]", ERROR_INVALID_PARAMETER, pcName);
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteSaveParams(DWORD hCnx)
{
	DWORD dwErr = 0;
	DWORD dwPos;

	dwErr = NTSVCSaveParameters(CMD_PARAMS.psParams, &dwPos);
	if (dwErr == NO_ERROR)
		CmdResponse(hCnx, "SAVEPARAMSOK");
	else
		CmdResponse(hCnx, "SAVEPARAMSERROR %u , Cannot save parameter %u", dwErr, dwPos);
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteListUsers(DWORD hCnx)
{
	DWORD dwErr = 0;

	if (PV_PostMessage(PV_WM_CMD_LIST_USERS, (PVOID)hCnx) == FALSE)
	{
		dwErr = GetLastError();
		CmdResponse(hCnx, "LISTUSERSERROR %u , Cannot List Users", dwErr);
	}
	else
		CmdResponse(hCnx, "LISTUSERSOK");
}


// --------------------------------------------------------------------

PRIVATE void CmdExecuteLaneList(DWORD hCnx)
{
	/* TO DO
		DWORD dwIndex;
		SRVPVSVC_LANE * psLane;

		EnterCriticalSection( &gsSvcWork.sCritical );

		CmdResponse( hCnx, "LANELISTBEGIN PLAZA,LANE,REFFILE,STATE" );

		for ( dwIndex = 0 ; dwIndex < gsSvcWork.dwMaxLanes ; dwIndex ++ )
		{
		psLane = &gsSvcWork.psList[dwIndex];
		if ( psLane->bBusy )
		CmdResponse( hCnx, "LANELISTITEM %u,%u,%s,%s",
		psLane->dwPlaza,
		psLane->dwLane,
		psLane->szRefFile,
		psLane->bConnected ? "CONNECTED" : "DISCONNECTED" );
		}

		CmdResponse( hCnx, "LANELISTEND" );

		LeaveCriticalSection( &gsSvcWork.sCritical );
		*/
}

// --------------------------------------------------------------------

PRIVATE void CmdExecuteRefreshPV(DWORD hCnx)
{
	DWORD dwErr = 0;

	if (ROUTE_PostMessage(ROUTE_WM_TOTAL_PLAZA_STATE, NULL) == FALSE)
	{
		dwErr = GetLastError();
		CmdResponse(hCnx, "RUNTREATMENTERROR %u , Cannot trigger treatment", dwErr);
	}
	else
		CmdResponse(hCnx, "RUNTREATMENTOK");
}

PROTECTED void CmdReceive(DWORD hCnx, DWORD dwMsgSize, char * pcMsg)
{
	char tszParams[3][CMD_MAX_SIZE];

	NTSVCInfo("CmdReceive() => command received");

	if (CmdCheck("PROMPT", pcMsg, dwMsgSize, 0))
		CmdExecutePrompt(hCnx);
	else if (CmdCheck("STOP", pcMsg, dwMsgSize, 0))
		CmdExecuteStop(hCnx);
	else if (CmdCheck("RELOAD", pcMsg, dwMsgSize, 0))
		CmdExecuteReload(hCnx);
	else if (CmdCheck("PARAMLIST", pcMsg, dwMsgSize, 0))
		CmdExecuteParamList(hCnx);
	else if (CmdCheck("SETPARAM", pcMsg, dwMsgSize, 2, tszParams[0], tszParams[1]))
		CmdExecuteSetParam(hCnx, tszParams[0], tszParams[1]);
	else if (CmdCheck("SAVEPARAMS", pcMsg, dwMsgSize, 0))
		CmdExecuteSaveParams(hCnx);
	else if (CmdCheck("LISTUSERS", pcMsg, dwMsgSize, 0))
		CmdExecuteListUsers(hCnx);
	else if (CmdCheck("REFRESH PV", pcMsg, dwMsgSize, 0))
		CmdExecuteRefreshPV(hCnx);
	else if (CmdCheck("COMMANDLIST", pcMsg, dwMsgSize, 0))
		CmdExecuteCommandList(hCnx);
	else
		CmdResponse(hCnx, "COMMANDERROR %d,Unknown command", ERROR_INVALID_DATA);
}

/*-------------------------------- END OF FILE ------------------------------*/