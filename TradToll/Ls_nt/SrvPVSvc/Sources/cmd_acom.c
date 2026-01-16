/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : srvcmd_acom
 * FILE       : srvcmd_acom.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module server de com CMD
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
#include <windows.h>
#include <acom.h>
#include <ntsvc.h>
#include <cmd_acom.h>
#include <srvpv_main.h>

#include <memclass.h>

// -------------- DEFINES --------------------

#define CMD_WORKINGSET_TIMEOUT 5000

#define CMD_PIPE                    "\\\\.\\pipe\\SrvPVSvc\\CMD"

#define CMD_REG_VAL_MAXCMDCNX       "MaxCmdCnx"
#define CMD_REG_VAL_CMDWORKERS      "SrvCmdWorkers"

// ------------- VARIABLES -------------------

PRIVATE struct AcomParams
{
    // parametres
    IN DWORD dwMaxCMDCnx;
    IN DWORD dwServerWorkers;
    IN DWORD dwTimeToReconnect;
    IN DWORD dwMaxQueuedMessages;
    IN DWORD dwMaxMsgSize;
    IN DWORD dwPipeBufferSize;
    IN DWORD dwMaxLife;
    IN DWORD dwLifeTime;
	IN DWORD dwACOMTraceErr;
	IN DWORD dwACOMTraceDbg;
	IN CHAR  szACOMTraceFilePath[MAX_PATH*2];
	// instances
    OUT ACOM_WKS_HANDLE  hWks;
    OUT ACOM_INST_HANDLE hInst;
} 
ACOM_PARAMS;

// --------------- CODE ----------------------

PRIVATE DWORD CMD_ACOM_Init (DWORD dwThreadId)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams;
	
    // Définition des paramètres

    psParams = NTSVCOpenParameters( 
            CMD_REG_VAL_MAXCMDCNX     , REG_DWORD,        4, 1                                  , &ACOM_PARAMS.dwMaxCMDCnx,
            CMD_REG_VAL_CMDWORKERS    , REG_DWORD,        4, 1                                  , &ACOM_PARAMS.dwServerWorkers,
            SVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4, SVC_REG_VAL_RECONNECTTIME_DEFAULT  , &ACOM_PARAMS.dwTimeToReconnect,
            SVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4, SVC_REG_VAL_QUEUESIZE_DEFAULT      , &ACOM_PARAMS.dwMaxQueuedMessages,
            SVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4, SVC_REG_VAL_MAXMSGSIZE_DEFAULT     , &ACOM_PARAMS.dwMaxMsgSize,
            SVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4, SVC_REG_VAL_BUFFERSIZE_DEFAULT     , &ACOM_PARAMS.dwPipeBufferSize,
            SVC_REG_VAL_MAXLIFE       , REG_DWORD,        4, SVC_REG_VAL_MAXLIFE_DEFAULT        , &ACOM_PARAMS.dwMaxLife, 
            SVC_REG_VAL_LIFETIME      , REG_DWORD,        4, SVC_REG_VAL_LIFETIME_DEFAULT       , &ACOM_PARAMS.dwLifeTime,
			SVC_REG_VAL_ACOM_TRACE_DBG, REG_DWORD,			4, SVC_REG_VAL_ACOM_TRACE_DBG_DEFAULT	, &ACOM_PARAMS.dwACOMTraceDbg,
			SVC_REG_VAL_ACOM_TRACE_ERR, REG_DWORD,			4, SVC_REG_VAL_ACOM_TRACE_ERR_DEFAULT	, &ACOM_PARAMS.dwACOMTraceErr,
			NTSVC_REG_VAL_ERRORFILE, REG_SZ, sizeof(ACOM_PARAMS.szACOMTraceFilePath), "c:\\csr\\Traces\\SrvPvSvc_ACOM.trc", ACOM_PARAMS.szACOMTraceFilePath,
			NULL);
    if ( psParams == NULL )
        return ERROR_INVALID_DATA;

    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;

    NTSVCCloseParameters( psParams );
    
	if (ACOM_PARAMS.dwACOMTraceDbg != 0 || ACOM_PARAMS.dwACOMTraceErr != 0)
	{
		DWORD dwACOMTraceResult;
		char* szPos;

		if ((szPos = strstr(ACOM_PARAMS.szACOMTraceFilePath, ".")) != 0)
			*szPos = '\0';

		strcat_s(ACOM_PARAMS.szACOMTraceFilePath, MAX_PATH, "_ACOM.trc");

		dwACOMTraceResult = AComOpenTrace((ACOM_PARAMS.dwACOMTraceDbg != 0), (ACOM_PARAMS.dwACOMTraceErr != 0), ACOM_PARAMS.szACOMTraceFilePath);
		NTSVCInfo("CMD_ACOM_Init(), Initalizing ACOM trace to file:[%s] returned %d", ACOM_PARAMS.szACOMTraceFilePath, dwACOMTraceResult);
	}
	else
	{
		NTSVCInfo("CMD_ACOM_Init(), ACOM trace disabled:parameter [%s, %s] ==0!", SVC_REG_VAL_ACOM_TRACE_ERR, SVC_REG_VAL_ACOM_TRACE_DBG);
	}

	// MISE EN PLACE DU WORKING SET

    NTSVCInfo( "PV_ACOM_Init() => creating the working set" );
    ACOM_PARAMS.hWks = AComOpenWorkingSetTQ(
            0, // InstUserKey
            ACOM_PARAMS.dwMaxCMDCnx,
            2, // MaxInst
            THREAD_PRIORITY_ABOVE_NORMAL,
            1000, // ConnectLoopDelay
            ACOM_PARAMS.dwServerWorkers,
            ACOM_PROTOCOL_LIFE,
            ACOM_PARAMS.dwLifeTime,
            ACOM_PARAMS.dwMaxLife,
            0, // ALlowedTimeShift
            dwThreadId );
    if ( ACOM_PARAMS.hWks == NULL )
    {
        NTSVC_ERR( "ERROR: PV_ACOM_Init() => impossible to create the working set!" );
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE LA CONNEXION AVEC LES CMD

    NTSVCInfo( "PV_ACOM_Init() => creating the server (CMD)" );

    // désactivation des tests de sécurité sur le pipe
    AComSetNullSessionPipe (CMD_PIPE, TRUE);

    // ouverture d'une instance de serveur de PIPE
    ACOM_PARAMS.hInst = AComOpenPipeServerInstanceTQ(
            ACOM_PARAMS.hWks,
            CMD_PIPE,
            (DWORD) ACOM_PARAMS.hWks,
            ACOM_PARAMS.dwTimeToReconnect,
            ACOM_PARAMS.dwMaxCMDCnx,
            ACOM_PARAMS.dwPipeBufferSize,
            ACOM_PARAMS.dwPipeBufferSize,
            ACOM_PARAMS.dwMaxQueuedMessages,
            ACOM_PARAMS.dwMaxMsgSize,
            dwThreadId );
    if ( ACOM_PARAMS.hInst == NULL )
    {
        NTSVC_ERR( "ERROR: PV_ACOM_Init() => impossible to create the client pipe instance!" );
        AComCloseWorkingSet( ACOM_PARAMS.hWks, CMD_WORKINGSET_TIMEOUT );
        return ERROR_INVALID_DATA;
    }

    NTSVCInfo( "PV_ACOM_Init() => all instances are created" );

    return NO_ERROR;
}

PROTECTED BOOL CMD_ACOM_Send_Buffer(DWORD hCnxHandle,
                                     DWORD dwDataSize,
                                     void * pvData )
{
    CHAR *string = pvData;
    string[dwDataSize] = '\0';

    NTSVCInfo( "CMD_ACOM_Send_Buffer(%s)", pvData );

    return AComSendMessage (ACOM_PARAMS.hWks, 
                            hCnxHandle,
                            (ULONG) ACOM_PARAMS.hWks,
                            dwDataSize,
                            pvData ) == NO_ERROR;
}

PROTECTED BOOL CMD_ACOM_Disconnect (ACOM_CNX_HANDLE hCnxHandle)
{
    return AComDisconnectPeer( ACOM_PARAMS.hWks, hCnxHandle, FALSE );
}

PROTECTED BOOL CMD_ACOM_Start (DWORD dwThreadId)
{
    if (CMD_ACOM_Init (dwThreadId) != NO_ERROR)
        return FALSE;

    return TRUE;
}

PROTECTED BOOL CMD_ACOM_Terminate (void)
{
    AComCloseInstance( ACOM_PARAMS.hInst );
    AComCloseWorkingSet( ACOM_PARAMS.hWks, CMD_WORKINGSET_TIMEOUT );

    return TRUE;
}

