/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : srvpv_acom
 * FILE       : srvpv_acom.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module server de com PV
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <acom.h> 
#include <ntsvc.h>
#include <pv_acom.h>
#include <srvpv_main.h>
#include <pv_wm.h>

#include <memclass.h>

// -------------- DEFINES --------------------

#define PV_WORKINGSET_TIMEOUT 5000

#define PV_PIPE                 "\\\\.\\pipe\\PV\\LS"


// ------------- VARIABLES -------------------

PRIVATE struct AcomParams
{
    // parametres
    IN DWORD dwMaxPVCnx;
    IN DWORD dwServerWorkers;
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

PRIVATE DWORD PV_ACOM_Init (DWORD dwThreadId)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams;

    // Définition des paramètres

    psParams = NTSVCOpenParameters( 
            PV_REG_VAL_MAXPVCNX       , REG_DWORD,        4, PV_REG_VAL_MAXPVCNX_DEFAULT        , &ACOM_PARAMS.dwMaxPVCnx,
            PV_REG_VAL_PVWORKERS      , REG_DWORD,        4, PV_REG_VAL_PVWORKERS_DEFAULT       , &ACOM_PARAMS.dwServerWorkers,
            SVC_REG_VAL_RECONNECTTIME , REG_DWORD,        4, SVC_REG_VAL_RECONNECTTIME_DEFAULT  , &ACOM_PARAMS.dwTimeToReconnect,
            SVC_REG_VAL_QUEUESIZE     , REG_DWORD,        4, SVC_REG_VAL_QUEUESIZE_DEFAULT      , &ACOM_PARAMS.dwMaxQueuedMessages,
            SVC_REG_VAL_MAXMSGSIZE    , REG_DWORD,        4, SVC_REG_VAL_MAXMSGSIZE_DEFAULT     , &ACOM_PARAMS.dwMaxMsgSize,
            SVC_REG_VAL_BUFFERSIZE    , REG_DWORD,        4, SVC_REG_VAL_BUFFERSIZE_DEFAULT     , &ACOM_PARAMS.dwPipeBufferSize,
            SVC_REG_VAL_MAXLIFE       , REG_DWORD,        4, SVC_REG_VAL_MAXLIFE_DEFAULT        , &ACOM_PARAMS.dwMaxLife, 
            SVC_REG_VAL_LIFETIME      , REG_DWORD,        4, SVC_REG_VAL_LIFETIME_DEFAULT       , &ACOM_PARAMS.dwLifeTime,
            NULL );
    if ( psParams == NULL )
        return ERROR_INVALID_DATA;

    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;

    NTSVCCloseParameters( psParams );

	if (ACOM_PARAMS.dwMaxPVCnx > PV_ACOM_MAX_CONNECTIONS)
	{
		NTSVC_ERR1("ERROR: PV_ACOM_Init() => Number of PV connections (MaxPVCnx parameter) cannot be greater than %d", PV_ACOM_MAX_CONNECTIONS);
		return ERROR_INVALID_DATA;
	}
    
    // MISE EN PLACE DU WORKING SET

    NTSVCInfo( "PV_ACOM_Init() => creating the working set" );
    ACOM_PARAMS.hWks = AComOpenWorkingSetTQ(
            0, // InstUserKey
            ACOM_PARAMS.dwMaxPVCnx,
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
        NTSVC_ERR("ERROR: PV_ACOM_Init() => impossible to create the working set");
        return ERROR_INVALID_DATA;
    }

    // MISE EN PLACE DE LA CONNEXION AVEC LES PV

    NTSVCInfo( "PV_ACOM_Init() => creating the server (PV)" );

    // désactivation des tests de sécurité sur le pipe
    AComSetNullSessionPipe (PV_PIPE, TRUE);

    // ouverture d'une instance de serveur de PIPE
    ACOM_PARAMS.hInst = AComOpenPipeServerInstanceTQ(
            ACOM_PARAMS.hWks,
            PV_PIPE,
            (DWORD) ACOM_PARAMS.hWks,
            ACOM_PARAMS.dwTimeToReconnect,
            ACOM_PARAMS.dwMaxPVCnx,
            ACOM_PARAMS.dwPipeBufferSize,
            ACOM_PARAMS.dwPipeBufferSize,
            ACOM_PARAMS.dwMaxQueuedMessages,
            ACOM_PARAMS.dwMaxMsgSize,            
            dwThreadId );
    if ( ACOM_PARAMS.hInst == NULL )
    {
        NTSVC_ERR( "ERROR: PV_ACOM_Init() => impossible to create client pipe instance" );
        AComCloseWorkingSet( ACOM_PARAMS.hWks, PV_WORKINGSET_TIMEOUT );
        return ERROR_INVALID_DATA;
    }

	// Set number of connected PVs to zero
	dwNbOfPVConnection = 0;

    NTSVCInfo( "PV_ACOM_Init() => all instances are created" );

    return NO_ERROR;
}

PROTECTED BOOL PV_ACOM_Send (ACOM_CNX_HANDLE hCnxHandle,
                             HMSG hMsg)
{
    BYTE buffer[SVC_MSG_BUFFER_SIZE];
    DWORD buffer_size;

    // conversion du message en "ASCII"
    buffer_size = MSG_Write (hMsg, buffer, sizeof(buffer));
    if (buffer_size == 0UL)
        return FALSE;

    // Envoi du buffer vers un PV sur le réseau
    return PV_ACOM_Send_Buffer (hCnxHandle,
                                   buffer_size,
                                   buffer );
}

PROTECTED BOOL PV_ACOM_Send_Buffer (ACOM_CNX_HANDLE hCnxHandle,
                                    DWORD dwDataSize,
                                    void * pvData )
{
    CHAR *string = pvData;
    string[dwDataSize] = '\0';

	if (dwDataSize > 200)
	{
		NTSVCInfo("PV_ACOM_Send_Buffer(%lu / %.200s)", dwDataSize, string);
		NTSVCInfo("PV_ACOM_Send_Buffer(%lu / %s)", dwDataSize - 200, string + 200);
	}
	else
		NTSVCInfo( "PV_ACOM_Send_Buffer(%lu / %s)", dwDataSize, pvData );

    return AComSendMessage (ACOM_PARAMS.hWks, 
                            hCnxHandle,
                            (ULONG) ACOM_PARAMS.hWks,
                            dwDataSize,
                            pvData ) == NO_ERROR;
}

PROTECTED BOOL PV_ACOM_Disconnect (ACOM_CNX_HANDLE hCnxHandle)
{
    return AComDisconnectPeer( ACOM_PARAMS.hWks, hCnxHandle, FALSE );
}

PROTECTED BOOL PV_ACOM_Start (DWORD dwThreadId)
{
    if (PV_ACOM_Init (dwThreadId) != NO_ERROR)
        return FALSE;

    return TRUE;
}

PROTECTED BOOL PV_ACOM_Terminate (void)
{
    AComCloseInstance( ACOM_PARAMS.hInst );
    AComCloseWorkingSet( ACOM_PARAMS.hWks, PV_WORKINGSET_TIMEOUT );

    return TRUE;
}

