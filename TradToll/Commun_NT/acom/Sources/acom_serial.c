/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_serial.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des com sur port serie
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#include <acom_key.h>
#include <acom_io.h>
#include <acom_stats.h>
#include <acom_clbk.h>
#define LOC_DEF
#include <acom_serial.h>
#undef LOC_DEF

#include <memclass.h>





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComSerialConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un port série. En fait,
 *             tente de créér le handle. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComSerialConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    DWORD           dwErr = NO_ERROR;
    BOOL            bResult;
    DWORD           dwIntErr;
    DCB             sDcb;
    COMMTIMEOUTS    sTimeouts;
    char            szPort[16];

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : PIPE CLIENT

    AComDbgInfo( __FILE__, __LINE__, "AComSerialConnect(0x%016X)", psCnx );

    sprintf_s( szPort, 16, "\\\\.\\COM%u", psCnx->psInst->sParams.sSerial.dwPort );

    psCnx->hFile = CreateFile(
        szPort,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL );
    bResult = ( psCnx->hFile != INVALID_HANDLE_VALUE );
    if ( ! bResult )
    {
        dwIntErr = GetLastError();
        psCnx->hFile = NULL;
        AComDbgInfo( __FILE__, __LINE__, "CreateFile retourne %d", dwIntErr );
    }

    if ( bResult )
    {
        bResult = SetupComm( psCnx->hFile, 
                             psCnx->psInst->sParams.sSerial.dwInBufferSize, 
                             psCnx->psInst->sParams.sSerial.dwOutBufferSize );
        if ( ! bResult )
        {
            dwIntErr = GetLastError();
            AComDbgInfo( __FILE__, __LINE__, "SetupComm retourne %d", dwIntErr );
        }
    }
        
    if ( bResult )
    {
        ZeroMemory( &sDcb, sizeof(sDcb) );
        sDcb.DCBlength = sizeof(sDcb);

        bResult = BuildCommDCB( psCnx->psInst->szFileName, &sDcb );
        if ( ! bResult )
        {
            dwIntErr = GetLastError();
            AComDbgInfo( __FILE__, __LINE__, "BuildCommDCB retourne %d", dwIntErr );
        }
        else
        {
            sDcb.fBinary = TRUE;
            sDcb.XonChar = 2;
            sDcb.XoffChar = 3;
        }
    }

    if ( bResult )
    {
        bResult = SetCommState( psCnx->hFile, &sDcb );
        if ( ! bResult )
        {
            dwIntErr = GetLastError();
            AComDbgInfo( __FILE__, __LINE__, "SetCommState retourne %d", dwIntErr );
        }
    }

    if ( bResult )
    {
        if ( psCnx->psInst->sParams.sSerial.dwReadInterval == 0 )
            psCnx->psInst->sParams.sSerial.dwReadInterval = 1;

        sTimeouts.ReadIntervalTimeout = psCnx->psInst->sParams.sSerial.dwReadInterval;
        sTimeouts.ReadTotalTimeoutMultiplier = 0;
        sTimeouts.ReadTotalTimeoutConstant = 0;
        sTimeouts.WriteTotalTimeoutMultiplier = 0;
        sTimeouts.WriteTotalTimeoutConstant = 0;
        bResult = SetCommTimeouts( psCnx->hFile, &sTimeouts );
        if ( ! bResult )
        {
            dwIntErr = GetLastError();
            AComDbgInfo( __FILE__, __LINE__, "SetCommTimeouts retourne %d", dwIntErr );
        }
    }

    if ( bResult )
    {
        psCnx->hCnxHandle = AComKeyGetNextKey( psCnx );
        hCompletion = CreateIoCompletionPort(
            psCnx->hFile,
            psCnx->psWks->hCompletion,
            psCnx->hCnxHandle,
            psCnx->psWks->dwWorkers );
        if ( hCompletion == NULL )
        {
            dwIntErr = GetLastError();
            AComDbgInfo( __FILE__, __LINE__, "CreateIoCompletionPort retourne %d", dwIntErr );
        }
        else
        {
            psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;

            sprintf_s( psCnx->szIdentity, 256, "#SERIAL%u#", psCnx->psInst->sParams.sSerial.dwPort );

            AComStatsReset( psCnx );
            dwErr = AComClbkConnection( psCnx );
            if ( dwErr == NO_ERROR )
                dwErr = AComWorkerReceiveNext( psCnx );
        }
    }

    if ( ( ! bResult ) && ( psCnx->hFile != NULL ) )
    {
        CloseHandle( psCnx->hFile );
        psCnx->hFile = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComSerialConnect return %d", dwErr );
    return dwErr;
}



/* -------------  FIN DU FICHIER : acom_pipe.c ------------- */ 
