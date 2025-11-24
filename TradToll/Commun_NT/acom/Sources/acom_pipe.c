/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_pipe.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des connexions pour les pipes
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
#include <acom_pipe.h>
#undef LOC_DEF

#include <memclass.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComPipeServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un pipe serveur. En fait,
 *             tente de créér le serveur de pipe et de le mettre en attente
 *             de connexion. Si la création échoue, la connexion reste à l'état
 *             "déconnecté", sinon, elle passe à l'état "en cours de connexion".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComPipeServerConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    BOOL  bResult;
    DWORD dwIntErr;
    DWORD dwErr = NO_ERROR;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : PIPE SERVER

    AComDbgInfo( __FILE__, __LINE__, "AComPipeServerConnect(0x%016X)", psCnx );

    psCnx->hFile = CreateNamedPipe(
        psCnx->psInst->szFileName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
        PIPE_UNLIMITED_INSTANCES,
        psCnx->psInst->sParams.sPipeServer.dwOutBufferSize,
        psCnx->psInst->sParams.sPipeServer.dwInBufferSize,
        INFINITE,
        psCnx->psWks->psSA );
    if ( psCnx->hFile != INVALID_HANDLE_VALUE )
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
            // On reste à l'état déconnecté
            CloseHandle( psCnx->hFile );
            psCnx->hFile = NULL;
            AComDbgInfo( __FILE__, __LINE__, "CreateIoCompletionPort retourne %d", dwIntErr );
        }
        else
        {
            DMEM_ZERO( &psCnx->sOverCnx.sOver );
            bResult = ConnectNamedPipe( psCnx->hFile, &psCnx->sOverCnx.sOver );
            dwIntErr = ( bResult ? NO_ERROR : GetLastError() );
            dwIntErr = ( dwIntErr == ERROR_IO_PENDING ? NO_ERROR : dwIntErr );
            if ( dwIntErr != NO_ERROR )
            {
                AComDbgInfo( __FILE__, __LINE__, "ConnectNamedPipe retourne %d", dwIntErr );
                // On reste à l'état déconnecté
                CloseHandle( psCnx->hFile );
                psCnx->hFile = NULL;
                if ( ( dwIntErr != ERROR_PIPE_CONNECTED ) && 
                     ( dwIntErr != ERROR_NO_DATA        ) &&
                     ( dwIntErr != ERROR_PIPE_LISTENING ) )
                    dwErr = dwIntErr;
            }
            else
            {
                psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTING;
                // Ne pas faire de callback de connexion, il sera fait
                // lors du passage à l'état connecté
            }
        }
    }
    else
    {
        dwIntErr = GetLastError();
        psCnx->hFile = NULL;
        AComDbgInfo( __FILE__, __LINE__, "CreateNamedPipe retourne %d", dwIntErr );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComPipeServerConnect return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComPipeClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un pipe client. En fait,
 *             tente de créér le client de pipe. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComPipeClientConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    DWORD dwErr = NO_ERROR;
    DWORD dwMode;
    BOOL bResult;
    DWORD dwIntErr;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : PIPE CLIENT

    AComDbgInfo( __FILE__, __LINE__, "AComPipeClientConnect(0x%016X)", psCnx );

    psCnx->hFile = CreateFile(
        psCnx->psInst->szFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        psCnx->psWks->psSA,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL );
    if ( psCnx->hFile != INVALID_HANDLE_VALUE )
    {
    
        dwMode = PIPE_READMODE_MESSAGE;
    
        bResult = SetNamedPipeHandleState(
            psCnx->hFile,
            &dwMode,
            NULL,
            NULL );
        if ( ! bResult )
        {
            dwIntErr = GetLastError();
            // On reste à l'état déconnecté
            CloseHandle( psCnx->hFile );
            psCnx->hFile = NULL;
            AComDbgInfo( __FILE__, __LINE__, "SetNamedPipeHandleState retourne %d", dwIntErr );
        }
        else
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
                // On reste à l'état déconnecté
                CloseHandle( psCnx->hFile );
                psCnx->hFile = NULL;
                AComDbgInfo( __FILE__, __LINE__, "CreateIoCompletionPort retourne %d", dwIntErr );
            }
            else
            {
                psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
                AComPipeGetIdentity( psCnx );
                AComStatsReset( psCnx );
                dwErr = AComClbkConnection( psCnx );
                if ( dwErr == NO_ERROR )
                    dwErr = AComWorkerReceiveNext( psCnx );
            }
        }
    }
    else
    {
        dwIntErr = GetLastError();
        psCnx->hFile = NULL;
        AComDbgInfo( __FILE__, __LINE__, "CreateFile retourne %d", dwIntErr );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComPipeClientConnect return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComPipeGetIdentity( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (pipe = identifié pour serveur uniquement)
 * REMARQUE  : Suppose que le slot de connexion est bien associé à
 *             un pipe (client ou serveur), dans un état "connecté" et
 *             qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComPipeGetIdentity( ACOM_CONNECTION * psCnx )
{
    if ( psCnx->psInst->dwType == ACOM_INSTANCE_PIPE_SERVER )
    {
        if ( ( psCnx->psWks->dwProtocol & ACOM_FLAG_NO_GET_PEER_NAME ) == 0 )
        {
            if ( ! GetNamedPipeHandleState( psCnx->hFile, 
                                            NULL, NULL, NULL, NULL, 
                                            psCnx->szIdentity, sizeof(psCnx->szIdentity) ) )
            {
                sprintf_s( psCnx->szIdentity, 256, "#PIPESERVER-ERR%u#", GetLastError() );
            }
        }
        else
            strcpy_s( psCnx->szIdentity, 256, "#PIPECLIENT#" );
    }
    else
        strcpy_s( psCnx->szIdentity, 256, "#PIPECLIENT#" );
}



/* -------------  FIN DU FICHIER : acom_pipe.c ------------- */ 
