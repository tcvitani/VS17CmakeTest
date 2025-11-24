/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_mail.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des "connexions" pour les mailslots
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
#include <stdio.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#include <acom_key.h>
#include <acom_io.h>
#include <acom_stats.h>
#include <acom_clbk.h>
#define LOC_DEF
#include <acom_mail.h>
#undef LOC_DEF

#include <memclass.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComMailServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un mailslot serveur. En fait,
 *             tente de créér le mailslot en écoute. Si la création
 *             échoue, la connexion reste à l'état "déconnecté", sinon,
 *             elle passe à l'état connecté.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComMailServerConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    DWORD dwErr = NO_ERROR;
    DWORD dwIntErr;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : MAIL SERVER

    AComDbgInfo( __FILE__, __LINE__, "AComMailServerConnect(0x%016X)", psCnx );

    psCnx->hFile = CreateMailslot(
        psCnx->psInst->szFileName,
        psCnx->psInst->sParams.sPipeServer.dwMaxMessageSize,
        MAILSLOT_WAIT_FOREVER,
        psCnx->psWks->psSA  );
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
            psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
            AComMailGetIdentity( psCnx );
            AComStatsReset( psCnx );
            dwErr = AComClbkConnection( psCnx );
            if ( dwErr == NO_ERROR )
                dwErr = AComWorkerReceiveNext( psCnx );
        }
    }
    else
    {
        dwIntErr = GetLastError();
        AComDbgInfo( __FILE__, __LINE__, "CreateMailslot retourne %d", dwIntErr );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComMailServerConnect return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComMailClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un mailslot client. En fait,
 *             tente de créér le client pour émission. Si la création
 *             échoue, la connexion reste à l'état "déconnecté", sinon,
 *             elle passe à l'état connecté.
 *             ATTENTION : En raison d'un bug sur les clients de mailslots distants
 *             ( le bug ne se déclare pas lors de communications loacales par mailslot )
 *             en overlapped i/o, le client de mailslot est ouvert en mode synchrone
 *             et n'est pas associé à un i/o completion port. Les completion packets
 *             seront postés par PostQueuedCompletionStatus().
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComMailClientConnect( ACOM_CONNECTION * psCnx )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIntErr;

    // Pas d'overlapped i/o donc pas de completion port
    // HANDLE hCompletion;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : MAIL CLIENT

    AComDbgInfo( __FILE__, __LINE__, "AComMailClientConnect(0x%016X)", psCnx );

    // Pas d'overlapped i/o donc pas de FILE_FLAG_OVERLAPPED dans le CreateFile
    psCnx->hFile = CreateFile(
        psCnx->psInst->szFileName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        psCnx->psWks->psSA,
        OPEN_EXISTING,
        0, 
        NULL );
    if ( psCnx->hFile == INVALID_HANDLE_VALUE )
    {
        // On reste à l'état déconnecté
        dwIntErr = GetLastError();
        psCnx->hFile = NULL;
        AComDbgInfo( __FILE__, __LINE__, "CreateFile retourne %d", dwIntErr );
    }
    else
    {
        psCnx->hCnxHandle = AComKeyGetNextKey( psCnx );

        // Pas d'overlapped i/o donc pas de completion port
        // hCompletion = CreateIoCompletionPort(
        //     psCnx->hFile,
        //     psCnx->psWks->hCompletion,
        //     psCnx->hCnxHandle,
        //     psCnx->psWks->dwWorkers );
        // if ( hCompletion == NULL )
        // {
        //     // On reste à l'état déconnecté
        //     CloseHandle( psCnx->hFile );
        //     psCnx->hFile = NULL;
        //     return NO_ERROR;
        // }

        psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
        AComMailGetIdentity( psCnx );
        AComStatsReset( psCnx );
        dwErr = AComClbkConnection( psCnx );
    }
     
    AComDbgInfo( __FILE__, __LINE__, "AComMailClientConnect return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComMailGetIdentity( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (mailslot = non identifié)
 * REMARQUE  : Suppose que le slot de connexion est associé à un mailslot,
 *             dans un état "connecté" et qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComMailGetIdentity( ACOM_CONNECTION * psCnx )
{
    strcpy_s( psCnx->szIdentity, 256, "#MAILSLOT#" );
}



/* -------------  FIN DU FICHIER : acom_mail.c ------------- */ 
