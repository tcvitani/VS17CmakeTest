/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_io.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des i/o + déconnexion
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#include <acom_queue.h>
#include <acom_block.h>
#include <acom_clbk.h>
#define LOC_DEF
#include <acom_io.h>
#undef LOC_DEF

#include <memclass.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerDisconnectOnError ( ACOM_CONNECTION * psCnx, DWORD dwErr )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 *             DWORD dwErr             : Erreur ayant provoqué la déconnexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la déconnexion d'une connexion lorsqu'une erreur est survenue
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerDisconnectOnError( ACOM_CONNECTION * psCnx, DWORD dwErr )
{
	AComDbgInfo(__FILE__, __LINE__, "AComWorkerDisconnectOnError(0x%016X) - Identity[%s] - err:%d", psCnx, psCnx->szIdentity, dwErr);

    // Annuler les i/o en cour pour notre com
    // Les i/o en cours arriveront donc avant la demande de déconnexion
    // dans la i/o completion queue
    CancelIo( psCnx->hFile );

    // Récuperer le code d'erreur
    psCnx->dwError = dwErr;
    psCnx->dwState = ACOM_CONNECTION_STATE_ERROR;

    // Poster la demande de déconnexion.
    dwErr = AComBlockPost(
        psCnx->psWks->hCompletion,
        ACOM_COMMAND_DISCONNECT,
        psCnx,
        psCnx->hCnxHandle,
        psCnx->dwCnxUsrKey,
        0,
        FALSE, // FALSE : Ne change pas l'état disabled / enabled de la connexion
        0,
        NULL,
        0,
        NULL );
    if ( dwErr != NO_ERROR )
    {
        // En cas d'erreur de post, c'est le shutdown
        psCnx->dwState = ACOM_CONNECTION_STATE_SHUTDOWN;
        psCnx->dwError = dwErr;

        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur AComBlockPost" );

        AComClbkShutdown( psCnx->psWks, dwErr );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerDisconnectOnError return %d", dwErr );
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerReceiveNext ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie une i/o de lecture, sauf si l'attribut bIsOverlapped de la
 *             structure de connexion est à FALSE. Dans ce dernier cas, la fonction
 *             est bloquée jusqu'à reception, un i/o completion packet est alors
 *             envoyé comme si la lecture avait été faite de manière assynchrone.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerReceiveNext( ACOM_CONNECTION * psCnx )
{
    BOOL  bRead;
    DWORD dwErr = NO_ERROR;
    DWORD dwRead;

	AComDbgInfo(__FILE__, __LINE__, "AComWorkerReceiveNext(0x%016X) - Identity[%s] ", psCnx, psCnx->szIdentity);

	// Mettre à zéro la structure OVERLLAPED
    DMEM_ZERO( &(psCnx->sOverIn.sOver) );
    // Lancer la lecture, non bloquante si psCnx->bIsOverlapped est TRUE,
    // bloquante si psCnx->bIsOverlapped est FALSE
    bRead = ReadFile(
        psCnx->hFile,
        psCnx->pbReadBuffer + psCnx->dwReadPos,
        psCnx->dwReadBufferSize - psCnx->dwReadPos,
        &dwRead,
        psCnx->bIsOverlapped ? &psCnx->sOverIn.sOver : NULL );



    if ( ! bRead )
        dwErr = GetLastError();
    // Trapper le cas ERROR_IO_PENDING qui n'est pas une erreur
    dwErr = ( dwErr == ERROR_IO_PENDING ? NO_ERROR : dwErr );
    if ( dwErr != NO_ERROR )
		AComDbgError(dwErr, __FILE__, __LINE__, "AComWorkerReceiveNext - ReadFile - (0x%016X) - Identity[%s] ", psCnx, psCnx->szIdentity);

    // Si la connexion n'est pas gérèe en assynchrone
    if ( dwErr == NO_ERROR && ! psCnx->bIsOverlapped )
    {
        // Simuler l'i/o completion
        bRead = PostQueuedCompletionStatus(
            psCnx->psWks->hCompletion, 
            dwRead, 
            psCnx->hCnxHandle,
            &psCnx->sOverIn.sOver );
        if ( ! bRead )
            dwErr = GetLastError();
        if ( dwErr != NO_ERROR )
            AComDbgError( dwErr, __FILE__, __LINE__, "PostQueuedCompletionStatus" );
    }

    if ( dwErr != NO_ERROR )
        dwErr = AComWorkerDisconnectOnError( psCnx, dwErr );

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerReceiveNext return %d", dwErr );
    return dwErr;
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComWorkerSendNext ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, un erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie une i/o d'écriture, sauf si l'attribut bIsOverlapped de la
 *             structure de connexion est à FALSE. Dans ce dernier cas, la fonction
 *             est bloquée jusqu'à émission, un i/o completion packet est alors
 *             envoyé comme si l'écriture avait été faite de manière assynchrone.
 *             REMARQUE : L'i/o d'écriture n'est lancée que lorsqu'il y a un
 *             bloc dans la file des envois. Sinon, la fonction sort immédiatement
 *             sans rien lancer.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComWorkerSendNext( ACOM_CONNECTION * psCnx )
{
    BOOL  bWrite;
    DWORD dwErr = NO_ERROR;
    DWORD dwWrite;
    ACOM_LIFE_HEADER * psHeader;

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerSendNext(0x%016X)", psCnx );

    if ( psCnx->psWriteCurrent == NULL )
    {
        psCnx->psWriteCurrent = AComQueueRemoveItem( psCnx->psWriteQueue );

        if ( psCnx->psWriteCurrent != NULL )
        {
            // Si protocol life, mettre à jour heure système.
            if ( ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
            {
                psHeader = (ACOM_LIFE_HEADER*)AComBlockDataPtr( psCnx->psWriteCurrent );
                GetSystemTimeAsFileTime( (LPFILETIME)( &(psHeader->llDate) ) );
            }

            // Mettre à zéro la structure OVERLLAPED
            DMEM_ZERO( &(psCnx->sOverOut.sOver) );
            // Lancer l'écriture, non bloquante si psCnx->bIsOverlapped est TRUE,
            // bloquante si psCnx->bIsOverlapped est FALSE

			AComDbgTraceHexBuffer("AComWorkerSendNext() -WriteFile ", psCnx->psWriteCurrent->dwDataSize, (BYTE*)AComBlockDataPtr(psCnx->psWriteCurrent));


            bWrite = WriteFile(
                psCnx->hFile,
                AComBlockDataPtr( psCnx->psWriteCurrent ),
                psCnx->psWriteCurrent->dwDataSize,
                &dwWrite,
                psCnx->bIsOverlapped ? &psCnx->sOverOut.sOver : NULL );
            if ( ! bWrite )
                dwErr = GetLastError();
            dwErr = ( dwErr == ERROR_IO_PENDING ? NO_ERROR : dwErr );
            if ( dwErr != NO_ERROR )
                AComDbgError( dwErr, __FILE__, __LINE__, "WriteFile" );
            // Si la connexion n'est pas gérèe en assynchrone
            if ( dwErr == NO_ERROR && ! psCnx->bIsOverlapped )
            {
                // Simuler l'i/o completion
                bWrite = PostQueuedCompletionStatus(
                    psCnx->psWks->hCompletion, 
                    dwWrite, 
                    psCnx->hCnxHandle,
                    &psCnx->sOverOut.sOver );
                if ( ! bWrite )
                    dwErr = GetLastError();
                if ( dwErr != NO_ERROR )
                    AComDbgError( dwErr, __FILE__, __LINE__, "PostQueuedCompletionStatus" );
            }
            if ( dwErr != NO_ERROR )
                dwErr = AComWorkerDisconnectOnError( psCnx, dwErr );

        }
    }

    AComDbgInfo( __FILE__, __LINE__, "AComWorkerSendNext return %d", dwErr );
    return dwErr;
}

/* -------------  FIN DU FICHIER : acom_io.c ------------- */ 
