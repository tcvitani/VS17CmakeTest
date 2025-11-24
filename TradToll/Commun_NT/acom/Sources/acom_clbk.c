/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_clbk.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des callbacks par défaut pour fonctionnement en WM
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
#include <acom_timer.h>
#include <acom_dbg.h>
#include <acom_block.h>
#include <acom_io.h>
#define LOC_DEF
#include <acom_clbk.h>
#undef LOC_DEF

#include <memclass.h>


PRIVATE BOOL AComClbkPost( 
            DWORD dwMessageId,
            DWORD dwMode,
            HANDLE hObject, 
            DWORD dwType,
            void * pvEvt )
{
    if ( dwMode == ACOM_MODE_WM )
        return PostMessage( 
                hObject, 
                dwMessageId, 
                (WPARAM)dwType,
                (LPARAM)pvEvt );
    else if ( dwMode == ACOM_MODE_TQ )
		return PostThreadMessageA(
                (DWORD)hObject,
                dwMessageId,
                (WPARAM)dwType,
                (LPARAM)pvEvt );
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
}



PROTECTED DWORD AComClbkConnection(
                                ACOM_CONNECTION * psCnx )
{
    ACOM_EVT_CONNECTION * psEvt;
    DWORD dwErr = NO_ERROR;
    BOOL bPost;
    ACOM_LIFE_HEADER sHeader;
    DWORD dwDelay;

	AComDbgInfo(__FILE__, __LINE__, "AComClbkConnection(0x%016X) - Identity[%s] ", psCnx, psCnx->szIdentity);

    GetSystemTimeAsFileTime( (LPFILETIME)(&(psCnx->llLastLocalDate)) );
    psCnx->llLastRemoteDate = psCnx->llLastLocalDate;
    psCnx->bTimeShiftSignaled = FALSE;

    if ( ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
    {
        sHeader.dwType = ACOM_LIFE_REQ;
        sHeader.dwDataSize = 0;

        // Envoyer demande de vie immédiatement
        // pour synchroniser les deux bouts
        dwErr = AComBlockPost(
            psCnx->psWks->hCompletion,
            ACOM_COMMAND_SEND,
            psCnx,
            psCnx->hCnxHandle,
            0,
            0,
            FALSE,
            sizeof( sHeader ),
            &sHeader,
            0,
            NULL );
        if ( dwErr == NO_ERROR )
        {
            if ( psCnx->psWks->dwLifeTime < 0x10000000 )
                dwDelay = psCnx->psWks->dwLifeTime * ( 1 + ACOM_GET_LIFETIME_FACTOR(psCnx->psWks->dwProtocol) );
            else
                dwDelay = psCnx->psWks->dwLifeTime;

            psCnx->dwTimerId = AComTimerAddTimer(
                    dwDelay,
                    psCnx->psWks->hCompletion,
                    psCnx,
                    psCnx->hCnxHandle,
                    ACOM_COMMAND_LIFE );
        }
    }


    if ( dwErr == NO_ERROR )
    {
		psCnx->dwLifeCount = 0;

        if ( psCnx->psInst->dwUseMode != ACOM_MODE_CALLBACK )
        {
            psCnx->dwCnxUsrKey = 0;
            if ( DMEM_ALLOC( psEvt ) == NULL )
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                psCnx->dwCnxUsrKey = psCnx->hCnxHandle;
                psEvt->dwInstUsrKey = psCnx->psInst->dwInstUsrKey;
                psEvt->hCnxHandle = psCnx->hCnxHandle;
                bPost = AComClbkPost(
                        psCnx->psInst->psWks->dwWmAComEvent,
                        psCnx->psInst->dwUseMode,
                        psCnx->psInst->hObjEvent, 
                        ACOM_WM_CONNECTION,
                        psEvt );
                if ( ! bPost )
                {
                    dwErr = GetLastError();
                    DMEM_FREE( psEvt );
                    AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
                }
            }
        }
        else if ( psCnx->psInst->pfCnx != NULL )
                psCnx->dwCnxUsrKey = psCnx->psInst->pfCnx( 
                            psCnx->psInst->dwInstUsrKey, 
                            psCnx->hCnxHandle );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComClbkConnection return %d", dwErr );
    return dwErr;
}



PROTECTED DWORD AComClbkReceived(
                                ACOM_CONNECTION * psCnx,
                                DWORD dwDataSize )
{
    DWORD                   dwErr           = NO_ERROR;
    DWORD                   dwHeaderSize;
    DWORD                   dwBodySize;
    BOOL                    bPost;
    BOOL                    bForward;
    BOOL                    bShift;
    BOOL                    bIncomplete;
    BOOL                    bMove;
    BOOL                    bFinished;
    LONGLONG                llShift;
    ACOM_EVT_RECEIVED     * psEvt;
    ACOM_EVT_TIME_SHIFT   * psEvtTime;
    ACOM_LIFE_HEADER      * psHeader;
    ACOM_LIFE_HEADER        sLife;

    AComDbgInfo( __FILE__, __LINE__, "AComClbkReceived(0x%016X,%d)", psCnx, dwDataSize );

    if ( ( dwDataSize == 0 ) &&
         ( ( psCnx->psInst->dwType == ACOM_INSTANCE_TCP_SERVER ) ||
           ( psCnx->psInst->dwType == ACOM_INSTANCE_TCP_CLIENT ) ) )
    {
        // Il s'agit d'une déconnexion propre TCP
        dwErr = AComBlockPost(
            psCnx->psInst->psWks->hCompletion,
            ACOM_COMMAND_DISCONNECT,
            psCnx,
            psCnx->hCnxHandle,
            0,
            0,
            FALSE,
            0,
            NULL,
            0,
            NULL );
    }
    else
    {
        bFinished = FALSE;

        while ( ( ! bFinished ) && ( dwErr == NO_ERROR ) )
        {
            dwBodySize = dwDataSize;
            dwHeaderSize = 0;
            bForward = FALSE;
            bShift = FALSE;
            bMove = FALSE;
            bIncomplete = FALSE;

            if ( ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
            {
                psHeader = (ACOM_LIFE_HEADER *)psCnx->pbReadBuffer;

                if ( ( dwDataSize <= psCnx->dwReadBufferSize ) &&
                     ( dwDataSize >= sizeof(ACOM_LIFE_HEADER) ) &&
                     ( psHeader->dwDataSize + sizeof(ACOM_LIFE_HEADER) <= dwDataSize ) )
                {
                    GetSystemTimeAsFileTime( (LPFILETIME)(&(psCnx->llLastLocalDate)) );
                    psCnx->llLastRemoteDate = psHeader->llDate;
                    dwHeaderSize = sizeof( *psHeader );
                    dwBodySize = psHeader->dwDataSize;

                    if ( psCnx->psWks->llAllowedTimeShift != (LONGLONG)0 )
                    {
                        llShift = psCnx->llLastRemoteDate - psCnx->llLastLocalDate;
                        if ( llShift < 0 ) 
                            llShift = ( - llShift );

                        bShift = ( ( llShift > psCnx->psWks->llAllowedTimeShift ) && ( ! psCnx->bTimeShiftSignaled ) );
                        psCnx->bTimeShiftSignaled = bShift;
                    }

                    psCnx->dwLifeCount = 0;
                    /*AComTimerRemoveTimer( psCnx->dwTimerId );
                    psCnx->dwTimerId = AComTimerAddTimer(
                            psCnx->psWks->dwLifeTime,
                            psCnx->psWks->hCompletion,
                            psCnx,
                            psCnx->hCnxHandle,
                            ACOM_COMMAND_LIFE );*/

                    // INTERCEPTER MESSAGES PROTOCOLAIRES
                    if ( psHeader->dwType != ACOM_LIFE_DATA )
                    {
                        if ( psHeader->dwType == ACOM_LIFE_REQ )
                        {
                            sLife.dwType = ACOM_LIFE_RSP;
                            sLife.llDate = psCnx->llLastLocalDate;
                            sLife.dwDataSize = 0;

                            dwErr = AComBlockPost(
                                psCnx->psWks->hCompletion,
                                ACOM_COMMAND_SEND,
                                psCnx,
                                psCnx->hCnxHandle,
                                0,
                                0,
                                FALSE,
                                sizeof( sLife ),
                                &sLife,
                                0,
                                NULL );
                            if ( dwErr != NO_ERROR )
                                bFinished = TRUE;
                        }
                        else if ( psHeader->dwType == ACOM_LIFE_RSP )
                        {
                            AComDbgInfo( __FILE__, __LINE__, "Réception réponse vie" );
                            // Rien à faire
                        }
                        else
                        {
                            AComDbgError( ERROR_INVALID_DATA, __FILE__, __LINE__, "Type de message reçu inconnu" );
                            dwErr = AComWorkerDisconnectOnError( psCnx, ERROR_INVALID_DATA );
                            bFinished = TRUE;
                        }
                    }
                    else
                        bForward = TRUE;

                    // Le message est complet, à la fin de la fonction, il faut placer les octets recus apres
                    // le message en tête de buffer
                    bMove = TRUE;
                }
                else
                {
                    // Si le message est incomplet, on attend la suite. aprés avoir déplacé
                    // la position courante du curseur dans le buffer de reception.
                    if ( dwDataSize <= psCnx->dwReadBufferSize )
                    {
                        if ( dwDataSize >= sizeof(ACOM_LIFE_HEADER) )
                            bIncomplete = ( ( psHeader->dwDataSize + sizeof(ACOM_LIFE_HEADER) >  dwDataSize              ) &&
                                            ( psHeader->dwDataSize + sizeof(ACOM_LIFE_HEADER) <= psCnx->dwReadBufferSize ) );
                        else
                            bIncomplete = TRUE;
                    }

                    if ( ! bIncomplete )
                    {
                        AComDbgError( ERROR_INVALID_DATA, __FILE__, __LINE__, "Structure de message non valide" );
                        dwErr = AComWorkerDisconnectOnError( psCnx, ERROR_INVALID_DATA );
                    }
                    else
                        psCnx->dwReadPos = dwDataSize;

                    bFinished = TRUE;
                }

				AComTimerRemoveTimer(psCnx->dwTimerId);
				psCnx->dwTimerId = AComTimerAddTimer(
					psCnx->psWks->dwLifeTime,
					psCnx->psWks->hCompletion,
					psCnx,
					psCnx->hCnxHandle,
					ACOM_COMMAND_LIFE);

			}
            else
                bForward = TRUE;

			AComDbgTraceHexBuffer("AComClbkReceived:", dwHeaderSize + dwBodySize, (BYTE*)psCnx->pbReadBuffer);
			//some data received reset life protocol timer
			psCnx->dwLifeCount = 0;

            if ( psCnx->psInst->dwUseMode != ACOM_MODE_CALLBACK )
            {
                if ( bForward )
                {
                    // WINDOW MESSAGE DE RECEPTION
                    if ( DMEM_ALLOC_BUFFER( psEvt, sizeof(*psEvt) + dwBodySize ) == NULL )
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        bFinished = TRUE;
                    }
                    else
                    {
                        psEvt->dwInstUsrKey = psCnx->psInst->dwInstUsrKey;
                        psEvt->dwCnxUsrKey = psCnx->dwCnxUsrKey;
                        psEvt->tdwData[0] = dwBodySize;
						CopyMemory(&(psEvt->tdwData[1]), psCnx->pbReadBuffer + dwHeaderSize, dwBodySize);


                        bPost = AComClbkPost(
                                psCnx->psInst->psWks->dwWmAComEvent,
                                psCnx->psInst->dwUseMode,
                                psCnx->psInst->hObjEvent, 
                                ACOM_WM_RECEIVED,
                                psEvt );
                        if ( ! bPost )
                        {
                            dwErr = GetLastError();
                            DMEM_FREE( psEvt );
                            AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
                            bFinished = TRUE;
                        }
                    }
                }

                // WINDOW MESSAGE DE SHIFT
                if ( bShift && ( dwErr == NO_ERROR ) )
                {
                    if ( DMEM_ALLOC( psEvtTime ) == NULL )
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        bFinished = TRUE;
                    }
                    else
                    {
                        psEvtTime->dwInstUsrKey = psCnx->psInst->dwInstUsrKey;
                        psEvtTime->dwCnxUsrKey = psCnx->dwCnxUsrKey;
                        bPost = AComClbkPost(
                                psCnx->psInst->psWks->dwWmAComEvent,
                                psCnx->psInst->dwUseMode,
                                psCnx->psInst->hObjEvent, 
                                ACOM_WM_TIME_SHIFT,
                                psEvtTime );
                        if ( ! bPost )
                        {
                            dwErr = GetLastError();
                            DMEM_FREE( psEvtTime );
                            AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
                            bFinished = TRUE;
                        }
                    }
                }
            }
            else
            {

                if ( ( psCnx->psInst->pfRecv != NULL ) && bForward )
                    psCnx->psInst->pfRecv( 
                            psCnx->psInst->dwInstUsrKey, 
                            psCnx->dwCnxUsrKey,
                            dwBodySize,
                            psCnx->pbReadBuffer + dwHeaderSize );
                if ( ( psCnx->psInst->pfTime != NULL ) && bShift )
                    psCnx->psInst->pfTime( 
                            psCnx->psInst->dwInstUsrKey, 
                            psCnx->dwCnxUsrKey );
            }

            if ( ( dwErr == NO_ERROR ) && ( ! bIncomplete ) )
            {
                dwDataSize -= ( dwHeaderSize + dwBodySize );
                psCnx->dwReadPos = dwDataSize;
                MoveMemory( psCnx->pbReadBuffer, psCnx->pbReadBuffer + dwHeaderSize + dwBodySize, psCnx->dwReadPos );
                if ( ! bFinished )
                    bFinished = ( psCnx->dwReadPos == 0 );
            }
            else
                bFinished = TRUE;
        }
    }

    AComDbgInfo( __FILE__, __LINE__, "AComClbkReceived return %d", dwErr );
    return dwErr;
}


PROTECTED DWORD AComClbkSent(
                                ACOM_CONNECTION * psCnx,
                                DWORD dwError )
{
    ACOM_EVT_SENT * psEvt;
    DWORD dwErr = NO_ERROR;
    BOOL bPost;

    AComDbgInfo( __FILE__, __LINE__, "AComClbkSent(0x%016X,%d)", psCnx, dwError );

    if ( psCnx->psWriteCurrent->dwMsgUsrKey != 0 )
    {
		if (dwError == NO_ERROR) //reset life event timer since the communication is ok
		{
			psCnx->dwLifeCount = 0;

			if (ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE)
			{	
				AComTimerRemoveTimer(psCnx->dwTimerId);
				psCnx->dwTimerId = AComTimerAddTimer(
				psCnx->psWks->dwLifeTime,
				psCnx->psWks->hCompletion,
				psCnx,
				psCnx->hCnxHandle,
				ACOM_COMMAND_LIFE);
			}
		}


        if ( psCnx->psInst->dwUseMode != ACOM_MODE_CALLBACK )
        {
            if ( DMEM_ALLOC(psEvt) == NULL )
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                psEvt->dwError = dwError;
                psEvt->dwInstUsrKey = psCnx->psInst->dwInstUsrKey;
                psEvt->dwMsgUsrKey = psCnx->psWriteCurrent->dwMsgUsrKey;
                psEvt->dwCnxUsrKey = psCnx->dwCnxUsrKey;
                bPost = AComClbkPost(
                        psCnx->psInst->psWks->dwWmAComEvent,
                        psCnx->psInst->dwUseMode,
                        psCnx->psInst->hObjEvent, 
                        ACOM_WM_SENT,
                        psEvt );
                if ( ! bPost )
                {
                    dwErr = GetLastError();
                    DMEM_FREE( psEvt );
                    AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
                }
            }
        }
        else if ( psCnx->psInst->pfSent != NULL )
            psCnx->psInst->pfSent( 
                        psCnx->psInst->dwInstUsrKey, 
                        psCnx->dwCnxUsrKey,
                        psCnx->psWriteCurrent->dwMsgUsrKey,
                        dwError );
    }
    AComDbgInfo( __FILE__, __LINE__, "AComClbkSent return %d", dwErr );
    return dwErr;
}


PROTECTED DWORD AComClbkDisconnection(
                                ACOM_CONNECTION * psCnx )
{
    ACOM_EVT_DISCONNECTION * psEvt;
    DWORD dwErr = NO_ERROR;
    BOOL bPost;

    AComDbgInfo( __FILE__, __LINE__, "AComClbkDisconnection(0x%016X)", psCnx );

    if ( ACOM_GET_PROTOCOL(psCnx->psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
    {
        AComTimerRemoveTimer( psCnx->dwTimerId );
    }

    if ( psCnx->psInst->dwUseMode != ACOM_MODE_CALLBACK )
    {
        if ( DMEM_ALLOC(psEvt) == NULL )
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            psEvt->dwCnxUsrKey = psCnx->dwCnxUsrKey;
            psEvt->dwInstUsrKey = psCnx->psInst->dwInstUsrKey;
            bPost = AComClbkPost(
                    psCnx->psInst->psWks->dwWmAComEvent,
                    psCnx->psInst->dwUseMode,
                    psCnx->psInst->hObjEvent, 
                    ACOM_WM_DISCONNECTION,
                    psEvt );
            if ( ! bPost )
            {
                dwErr = GetLastError();
                DMEM_FREE( psEvt );
                AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
            }
        }
    }
    else if ( psCnx->psInst->pfDcnx != NULL )
        psCnx->psInst->pfDcnx( 
                    psCnx->psInst->dwInstUsrKey, 
                    psCnx->dwCnxUsrKey );

    AComDbgInfo( __FILE__, __LINE__, "AComClbkDisconnection return %d", dwErr );
    return dwErr;
}


PROTECTED DWORD AComClbkShutdown(
                                ACOM_WORKING_SET * psWks,
                                DWORD dwError )
{
    ACOM_EVT_SHUTDOWN * psEvt;
    DWORD dwErr = NO_ERROR;
    BOOL bPost;

    AComDbgInfo( __FILE__, __LINE__, "AComClbkShutdown(0x%016X,%d)", psWks, dwError );

    // Eviter la récursivité et les appels multiples
    if ( ! psWks->bShutDown )
    {
        psWks->bShutDown = TRUE;
        if ( psWks->dwUseMode != ACOM_MODE_CALLBACK )
        {
            if ( DMEM_ALLOC(psEvt) == NULL )
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                psEvt->dwError = dwError;
                psEvt->dwWksUsrKey = psWks->dwWksUsrKey;
                bPost = AComClbkPost(
                        psWks->dwWmAComEvent,
                        psWks->dwUseMode,
                        psWks->hObjEvent, 
                        ACOM_WM_SHUTDOWN,
                        psEvt );
                if ( ! bPost )
                {
                    dwErr = GetLastError();
                    DMEM_FREE( psEvt );
                    AComDbgInfo( __FILE__, __LINE__, "PostMessage retourne %d", dwErr );
                }
            }
        }
        else if ( psWks->pfShut != NULL )
            psWks->pfShut(
                        psWks->dwWksUsrKey,
                        dwError );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComClbkShutdown return %d", dwErr );
    return dwErr;
}


/* -------------  FIN DU FICHIER : acom_clbk.c ------------- */ 
