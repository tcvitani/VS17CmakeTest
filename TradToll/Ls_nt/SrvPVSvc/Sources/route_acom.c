/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : route
 * FILE       : route_acom.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module principal du service
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <route_acom.h>
#include <ntsvc.h>
#include <srvpv_main.h>
#include <stdio.h>

#include <memclass.h>

// -------------- DEFINES --------------------

#define ROUTE_WORKINGSET_TIMEOUT 5000

#define ROUTE_PIPE			"\\\\%s\\pipe\\RouteSvc\\LS"
// ------------- VARIABLES -------------------

PRIVATE struct AcomParams
{
    // parametres
    IN DWORD dwNbComServer;
    IN DWORD dwWorkers;
    IN DWORD dwTimeToReconnect;
    IN DWORD dwMaxQueuedMessages;
    IN DWORD dwMaxMsgSize;
    IN DWORD dwPipeBufferSize;
    IN DWORD dwMaxLife;
    IN DWORD dwLifeTime;

    // instances
    OUT ACOM_WKS_HANDLE  hWks;
    OUT ACOM_INST_HANDLE hInst;
}
ACOM_PARAMS;

// --------------- CODE ----------------------

PRIVATE DWORD ROUTE_ACOM_Init (DWORD dwThreadId)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams;
    CHAR szServerName[MAX_PATH];
    CHAR szServerNameRegKey[MAX_PATH];
    CHAR szServerPipeName[MAX_PATH];
    DWORD i;

    // Définition des paramètres
    psParams = NTSVCOpenParameters( 
            SVC_REG_VAL_COM_SERVER_NB   , REG_DWORD,        4, SVC_REG_VAL_COM_SERVER_NB_DEFAULT  , &ACOM_PARAMS.dwNbComServer,
            SVC_REG_VAL_WORKERS         , REG_DWORD,        4, SVC_REG_VAL_WORKERS_DEFAULT        , &ACOM_PARAMS.dwWorkers,
            SVC_REG_VAL_RECONNECTTIME   , REG_DWORD,        4, SVC_REG_VAL_RECONNECTTIME_DEFAULT  , &ACOM_PARAMS.dwTimeToReconnect,
            SVC_REG_VAL_QUEUESIZE       , REG_DWORD,        4, SVC_REG_VAL_QUEUESIZE_DEFAULT      , &ACOM_PARAMS.dwMaxQueuedMessages,
            SVC_REG_VAL_MAXMSGSIZE      , REG_DWORD,        4, SVC_REG_VAL_MAXMSGSIZE_DEFAULT     , &ACOM_PARAMS.dwMaxMsgSize,
            SVC_REG_VAL_BUFFERSIZE      , REG_DWORD,        4, SVC_REG_VAL_BUFFERSIZE_DEFAULT     , &ACOM_PARAMS.dwPipeBufferSize,
            SVC_REG_VAL_MAXLIFE         , REG_DWORD,        4, SVC_REG_VAL_MAXLIFE_DEFAULT        , &ACOM_PARAMS.dwMaxLife, 
            SVC_REG_VAL_LIFETIME        , REG_DWORD,        4, SVC_REG_VAL_LIFETIME_DEFAULT       , &ACOM_PARAMS.dwLifeTime,
            NULL );
    if ( psParams == NULL )
        return FALSE;

    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;

    NTSVCCloseParameters( psParams );

    // ASSERT
    if (ACOM_PARAMS.dwNbComServer > ROUTE_ACOM_MAX_CONNECTIONS)
    {
        NTSVC_ERR2( "ERROR: ROUTE_ACOM_Init(), number of ComServer %d > to max %d authorized", ACOM_PARAMS.dwNbComServer, ROUTE_ACOM_MAX_CONNECTIONS);
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DU WORKING SET 

    NTSVCInfo( "ROUTE_ACOM_Init() => creating the working set" );
    ACOM_PARAMS.hWks = AComOpenWorkingSetTQ(
            0, // InstUserKey
            ACOM_PARAMS.dwNbComServer, // MaxCnx
            ACOM_PARAMS.dwNbComServer, // MaxInst
            THREAD_PRIORITY_ABOVE_NORMAL,
            1000, // ConnectLoopDelay
            ACOM_PARAMS.dwWorkers, // Workers
            ACOM_PROTOCOL_LIFE,
            ACOM_PARAMS.dwLifeTime,
            ACOM_PARAMS.dwMaxLife,
            0, // ALlowedTimeShift
            dwThreadId );
    if ( ACOM_PARAMS.hWks == NULL )
    {
        NTSVC_ERR( "ERROR: ROUTE_ACOM_Init() => impossible to create the working set" );
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE LA CONNEXION AVEC LES SRV ROUTE 

    for (i = 1; i <= ACOM_PARAMS.dwNbComServer; i++)
    {
        // création du nom de la clef de registre
        _snprintf_s (szServerNameRegKey, _countof(szServerNameRegKey), sizeof(szServerNameRegKey), SVC_REG_VAL_COM_SERVER_NAME, i);

        psParams = NTSVCOpenParameters( 
                szServerNameRegKey , REG_SZ		, MAX_PATH, SVC_REG_VAL_COM_SERVER_NAME_DEFAULT, &szServerName,
				NULL );
        if ( psParams == NULL )
            return FALSE;

        if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
            return FALSE;

        NTSVCCloseParameters( psParams );

        NTSVCInfo( "ROUTE_ACOM_Init() => creating the SVC ROUTE client to the server '%s'", szServerName);
    
        // REM : on passe le handle du Working Set par le User Key pour s'en servir sur un AcomDisconnectPeer...
        _snprintf_s (szServerPipeName, _countof(szServerPipeName), sizeof(szServerPipeName), ROUTE_PIPE, szServerName);
        ACOM_PARAMS.hInst = AComOpenPipeClientInstanceTQ(
                ACOM_PARAMS.hWks,
                szServerPipeName,
                (DWORD) ACOM_PARAMS.hWks, // InstUsrKey
                ACOM_PARAMS.dwTimeToReconnect,
                ACOM_PARAMS.dwMaxQueuedMessages,
                ACOM_PARAMS.dwMaxMsgSize,
                dwThreadId );
        if ( ACOM_PARAMS.hInst == NULL )
        {
            NTSVC_ERR1( "ERROR: ROUTE_ACOM_Init() => impossible to create the client pipe instance: '%s'", szServerPipeName);
            AComCloseWorkingSet( ACOM_PARAMS.hWks, ROUTE_WORKINGSET_TIMEOUT );
            return ERROR_INVALID_DATA;
        }
    }

    NTSVCInfo( "ROUTE_ACOM_Init() => all instances are created" );

    return NO_ERROR;
}

PROTECTED DWORD ROUTE_ACOM_Get_Nb_Com_Server (void)
{
    return ACOM_PARAMS.dwNbComServer;
}

PROTECTED BOOL ROUTE_ACOM_Start (DWORD dwThreadId)
{
    if (ROUTE_ACOM_Init (dwThreadId ) != NO_ERROR)
        return FALSE;

    return TRUE;
}

PROTECTED BOOL ROUTE_ACOM_Terminate (void)
{
    AComCloseInstance( ACOM_PARAMS.hInst );
    AComCloseWorkingSet( ACOM_PARAMS.hWks, ROUTE_WORKINGSET_TIMEOUT );

    return TRUE;
}

PROTECTED BOOL ROUTE_ACOM_Send (ACOM_CNX_HANDLE hCnxHandle,
                                HMSG hMsg)
{
    BYTE buffer[SVC_MSG_BUFFER_SIZE];
    DWORD buffer_size;

    // conversion du message en "ASCII"
    buffer_size = MSG_Write (hMsg, buffer, sizeof(buffer));
    if (buffer_size == 0UL)
        return FALSE;

	buffer[buffer_size] = 0;

    // Envoi du buffer vers un PV sur le réseau
    return ROUTE_ACOM_Send_Buffer (hCnxHandle,
                                   buffer_size,
                                   buffer );
}

PROTECTED BOOL ROUTE_ACOM_Send_Buffer (ACOM_CNX_HANDLE hCnxHandle,
                                       DWORD dwDataSize,
                                       void * pvData )
{
    CHAR *string = pvData;

	if (pvData != NULL)
		NTSVCInfo("ROUTE_ACOM_Send_Buffer(%s)", pvData);

    return AComSendMessage (ACOM_PARAMS.hWks, 
                            hCnxHandle,
                            (ULONG) ACOM_PARAMS.hWks,
                            dwDataSize,
                            pvData ) == NO_ERROR;
}

PROTECTED BOOL ROUTE_ACOM_Disconnect (ACOM_CNX_HANDLE hCnxHandle)
{
    return AComDisconnectPeer( ACOM_PARAMS.hWks, hCnxHandle, FALSE );
}
