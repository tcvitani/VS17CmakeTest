/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
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
#include <col.h>
#include <ntsvc.h>

#include <csr_rfa_svc.h>
#include <csr_rfa.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>
#include <trfsvc_protocol.h>

#define LOC_DEF
#include <trfsvc_srv_callback.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

PRIVATE DWORD WINAPI SrvProcessIdReq( TRFSVC_SERVER * psServer, FSYNC_MSG_ID_REQ * psReq );
PRIVATE DWORD SrvSendFull( TRFSVC_SERVER * psServer );
PRIVATE DWORD SrvProcessPingReq( TRFSVC_SERVER * psServer, FSYNC_MSG_PING_REQ * psReq );
PRIVATE DWORD SrvProcessPingRsp( TRFSVC_SERVER * psServer, FSYNC_MSG_PING_RSP * psRsp );


// --------------- CODE ----------------------

DWORD WINAPI AComSendMessageCb(ACOM_WKS_HANDLE hWks,
							   ACOM_CNX_HANDLE hCnxHandle,
							   DWORD64 dwMsgUsrKey,
							   DWORD dwDataSize,
							   void * pvData,
							   TRFSVC_SERVER * psServer)
{
	FSYNC_MSG_FULL_NF * psNF = (FSYNC_MSG_FULL_NF *)pvData;
	char szPeerName[MAX_PATH] = { 0 };

	if (psServer->dwQueueCounter + 1 >= gsSvcWork.sParmWork.dwMaxQueuedMessages)
	{
		NTSVCInfo("AComSendMessageCb: dwQueueCounter >= QueueSize force disconnection!");
		return ERROR_BUFFER_OVERFLOW;
	}
	
	psServer->dwQueueCounter++;
	
	return( AComSendMessage( hWks, hCnxHandle, dwMsgUsrKey, dwDataSize, psNF ) );
	
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK SrvCallbackConnection(
 *                     DWORD64 dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. 
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK SrvCallbackConnection(
		DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD dwErr = NO_ERROR;
    HCOLLECTIONITEM hItem = NULL;
    TRFSVC_SERVER * psServer;

    __try
    {
        NTSVCInfo( "SrvCallbackConnection(0x%08X)", hCnxHandle );

        psServer = HeapAlloc( 
            GetProcessHeap(), 
            HEAP_ZERO_MEMORY, 
            sizeof(*psServer) + gsSvcWork.sParmWork.dwMaxMsgSize - TRFSVC_PROTOCOL_BYTES );
        if ( psServer == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            NTSVCInfo( "SrvCallbackConnection Erreur allocation" );
            __leave; 
        }

        psServer->dwMaxBufferBytes = gsSvcWork.sParmWork.dwMaxMsgSize - TRFSVC_PROTOCOL_BYTES;
        psServer->hCnx = hCnxHandle;
    
        hItem = ColItemAdd( 
            gsSvcWork.hColServers, 
            &psServer->hCnx, 
            psServer, 
            sizeof(*psServer) + psServer->dwMaxBufferBytes );
        if ( hItem == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            NTSVCInfo( "SrvCallbackConnection Erreur ajout collection" );
            __leave; 
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            NTSVCInfo( "SrvCallbackConnection Erreur" );
            if ( psServer != NULL )
                HeapFree( GetProcessHeap(), 0, psServer );
            AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
        }
     
        NTSVCInfo( "SrvCallbackConnection Fin" );
    }

	return (DWORD64)hItem;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK SrvCallbackDisconnection(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se déconnecter.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK SrvCallbackDisconnection(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey)
{
    HCOLLECTIONITEM hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_SERVER * psServer = NULL;

    NTSVCInfo( "SrvCallbackDisconnection(0x%08X)", dwCnxUsrKey );

    if ( hItem != NULL )
    {
        ColLock( gsSvcWork.hColServers );

        psServer = ColItemData( hItem );

        NTSVCInfo( "SrvCallbackDisconnection(), déconnexion hCnx = 0x%08X", psServer->hCnx );

        ColItemRemove( hItem );

        ColUnlock( gsSvcWork.hColServers );

        HeapFree( GetProcessHeap(), 0, psServer );
    }

    NTSVCInfo( "SrvCallbackDisconnection Fin" );

    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK SrvCallbackReceived(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey,
 *                     DWORD dwMsgSize,
 *                     BYTE * pbMsg )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : On vient de recevoir un message d'un client
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK SrvCallbackReceived(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    DWORD dwErr = NO_ERROR;
    HCOLLECTIONITEM hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_SERVER * psServer;

    NTSVCInfo( "SrvCallbackReceived(0x%08X)", dwCnxUsrKey );

    if ( hItem != NULL )
    {
        ColLock( gsSvcWork.hColServers );

        psServer = ColItemData( hItem );

		// TMA
        // NTSVCInfo( "SrvCallbackDisconnection(), reception sur hCnx = 0x%08X", psServer->hCnx );
		// end of TMA

        if      ( FSYNC_IS_ID_REQ( pbMsg, dwMsgSize ) )
        {
            dwErr = SrvProcessIdReq( psServer, (FSYNC_MSG_ID_REQ*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "SrvCallbackReceived Erreur IdReq" );
        }
        else if ( FSYNC_IS_PING_REQ( pbMsg, dwMsgSize ) )
        {
            dwErr = SrvProcessPingReq( psServer, (FSYNC_MSG_PING_REQ*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "SrvCallbackReceived Erreur PingReq" );
        }
        else if ( FSYNC_IS_PING_RSP( pbMsg, dwMsgSize ) )
        {
            dwErr = SrvProcessPingRsp( psServer, (FSYNC_MSG_PING_RSP*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "SrvCallbackReceived Erreur PingRsp" );
        }
        else
        {
            dwErr = ERROR_INVALID_DATA;
            NTSVCInfo( "SrvCallbackReceived Erreur message inconnu" );
        }

        if ( dwErr != NO_ERROR )
            AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );

        ColUnlock( gsSvcWork.hColServers );
    }

    NTSVCInfo( "SrvCallbackReceived Fin" );

    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK SrvCallbackSent(
 *                    DWORD64 dwInstUsrKey,
 *                    DWORD64 dwCnxUsrKey,
 *                    DWORD64 dwMsgUsrKey,
 *                    DWORD dwError )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : On vient d'envoyer un message
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK SrvCallbackSent(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    HCOLLECTIONITEM hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_SERVER * psServer = NULL;


	if (hItem == NULL)
	{
		NTSVCInfo("ERROR: SrvCallbackSent(0x%08X): hItem(dwCnxUsrKey)==NULL");
		return;
	}

	NTSVCInfo("SrvCallbackSent(0x%08X)", dwCnxUsrKey);

    // En cas d'erreur, on déconnecte, sinon, rien à faire
    if ( dwError != NO_ERROR )
    {
        NTSVCInfo( "SrvCallbackSent Erreur = %d", dwError );
        ColLock( gsSvcWork.hColServers );

        psServer = ColItemData( hItem );
        AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
		psServer->dwQueueCounter = 0;

        ColUnlock( gsSvcWork.hColServers );
    }
	else
	{
		ColLock( gsSvcWork.hColServers );	

		psServer = ColItemData( hItem );
		
		if(psServer->dwQueueCounter > 0)
			psServer->dwQueueCounter--;

		ColUnlock( gsSvcWork.hColServers );
	 }

    NTSVCInfo( "SrvCallbackSent fin" );

    return;
}


PRIVATE DWORD WINAPI SrvProcessIdReq( TRFSVC_SERVER * psServer, FSYNC_MSG_ID_REQ * psReq )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    FSYNC_MSG_ID_RSP * psRsp = NULL;
    char * pcScan;
    HCOLLECTIONITEM hItem;
    TRFSVC_FILE * psFile = NULL;

    __try
    {
        NTSVCInfo( "SrvProcessIdReq Début" );

        if ( psServer == NULL )
        { 
            dwErr = ERROR_INVALID_HANDLE; 
            NTSVCInfo( "SrvProcessIdReq handle de serveur non valide" );
            __leave; 
        }
        if ( psServer->dwMaxBufferBytes < sizeof(*psRsp) ) 
        { 
            dwErr = ERROR_INSUFFICIENT_BUFFER; 
            NTSVCInfo( "SrvProcessIdReq taille de buffer insuffisante" );
            __leave; 
        }

        psRsp =  (void*)psServer->tbBuffer;

        if ( psServer->psFile != NULL ) 
        { 
            dwErr = ERROR_ALREADY_EXISTS; 
            NTSVCInfo( "SrvProcessIdReq client déjà connecté" );
            __leave; 
        }

        pcScan = psReq->tbNameAndKey;

        dwSize = __min( sizeof(psServer->szClientId) - 1, psReq->dwNameSize );
		strncpy_s(psServer->szClientId, sizeof(psServer->szClientId), pcScan, dwSize);
        psServer->szClientId[dwSize] = '\0';
        pcScan += psReq->dwNameSize;

        dwSize = __min( sizeof(psServer->szFileKey) - 1, psReq->dwKeySize );
		strncpy_s(psServer->szFileKey, sizeof(psServer->szFileKey), pcScan, dwSize);
        psServer->szFileKey[dwSize] = '\0';
        pcScan += psReq->dwKeySize;

        hItem = COL_SCAN_BEGIN;
        while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
        {
            psFile = ColItemData( hItem );
            if ( _stricmp( psServer->szFileKey, psFile->szFileKey ) == 0 )
            {
                psServer->psFile = psFile;
                break;
            }
        }
    }
    __finally
    {
        if ( ( dwErr == NO_ERROR ) && ( psRsp != NULL ) )
        {
            psRsp->dwMessageCode = FSYNC_CODE_ID_RSP;
            psRsp->dwRequestId = psReq->dwRequestId;
            psRsp->dwResultCode = ( psServer->psFile == NULL ? ERROR_FILE_NOT_FOUND : NO_ERROR );

			// TMA
            dwErr = AComSendMessage( 
                gsSvcWork.hWks, 
                psServer->hCnx, 
                0,
                FSYNC_SIZEOF_ID_RSP( psRsp ),
                psRsp );

			// dwErr = AComSendMessageCb( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_ID_RSP( psRsp ), psRsp, psServer );
			// end of TMA

            if ( ( dwErr == NO_ERROR ) && ( psServer->psFile != NULL ) )
                dwErr = SrvSendFull( psServer );
        }

        if ( dwErr != NO_ERROR )
            NTSVCInfo( "SrvProcessIdReq Erreur" );

        NTSVCInfo( "SrvProcessIdReq Fin" );
    }

	return dwErr;
}


PRIVATE DWORD SrvSendFull( TRFSVC_SERVER * psServer )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwMaxDataSize;
    DWORD dwDataSize;
    DWORD dwRead;
    BOOL bResult;
    BOOL bLeave = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    RFA_STRUCT * psHandle = INVALID_HANDLE_VALUE;
    FSYNC_MSG_FULL_NF * psNF = (FSYNC_MSG_FULL_NF*)psServer->tbBuffer;
    ULONGLONG ullFileSize;
    ULONGLONG ullOffset;

    __try
    {
        NTSVCInfo( "SrvSendFull Début" );

        psHandle = RFACreateFile(
            psServer->psFile->szFileKey,
            0,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            (HANDLE)RFA_SERVICE_ACCESS );
        if ( psHandle == INVALID_HANDLE_VALUE ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "SrvSendFull Erreur ouverture fichier RFA" );
            __leave; 
        }

        if ( ! RFAEnter( psHandle ) ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "SrvSendFull Erreur vérouillage fichier RFA" );
            __leave; 
        }
        bLeave = TRUE;

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            dwErr = GetLastError();
            if ( dwErr != ERROR_FILE_NOT_FOUND ) 
            {
                NTSVCInfo( "SrvSendFull Erreur ouveture fichier WIN32" );
                __leave;
            }

            psNF->dwMessageCode = FSYNC_CODE_FULL_NF;
            psNF->ullLastWrite = 0;
            psNF->ullFileSize = (ULONGLONG)(-1);
            psNF->ullOffset = 0;
            psNF->dwDataSize = 0;

            NTSVCInfo( "SrvSendFull(), Send FULL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                psNF->ullLastWrite,
                psNF->ullFileSize,
                psNF->ullOffset,
                psNF->dwDataSize );
	
            dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_FULL_NF( psNF ), psNF );
          
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "SrvSendFull Erreur envoi message FULL_NF vide" );
            __leave;
        }

        dwMaxDataSize = psServer->dwMaxBufferBytes - sizeof(*psNF);
        RFA_LL_LO(ullFileSize) = GetFileSize( hFile, &RFA_LL_HI(ullFileSize) );
        dwErr = GetLastError();
        if ( RFA_LL_LO(ullFileSize) == 0xFFFFFFFF ) 
        {
            NTSVCInfo( "SrvSendFull Erreur taille fichier WIN32" );
            __leave;
        }
        dwErr = NO_ERROR;

        bResult = GetFileTime( hFile, NULL, NULL, (LPFILETIME)&psNF->ullLastWrite );
        if ( ! bResult ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "SrvSendFull Erreur heure fichier WIN32" );
            __leave; 
        }

        psNF->dwMessageCode = FSYNC_CODE_FULL_NF;
        psNF->ullFileSize = ullFileSize;
        ullOffset = 0;

        do
        {
            dwDataSize = ( ullFileSize > (ULONGLONG)dwMaxDataSize ? dwMaxDataSize : RFA_LL_LO(ullFileSize) );

            if ( dwDataSize != 0 )
            {
                bResult = ReadFile(
                    hFile,
                    psNF->tbData,
                    dwDataSize,
                    &dwRead,
                    NULL );
                if ( ! bResult ) 
                { 
                    dwErr = GetLastError(); 
                    NTSVCInfo( "SrvSendFull Erreur lecture fichier WIN32" );
                    __leave; 
                }
            }

            psNF->ullOffset = ullOffset;
            psNF->dwDataSize = dwDataSize;


            NTSVCInfo( "SrvSendFull(), Send FULL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                psNF->ullLastWrite,
                psNF->ullFileSize,
                psNF->ullOffset,
                psNF->dwDataSize );

			// TMA
            // dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_FULL_NF( psNF ), psNF );
			dwErr = AComSendMessageCb( gsSvcWork.hWks, psServer->hCnx, 1, FSYNC_SIZEOF_FULL_NF( psNF ), psNF, psServer );
			// end of TMA
            if ( dwErr != NO_ERROR ) 
            {
                NTSVCInfo( "SrvSendFull Erreur envoi message FULL_NF" );
                __leave;
            }

            ullFileSize -= (ULONGLONG)dwDataSize;
            ullOffset += (ULONGLONG)dwDataSize;
        }
        while ( ullFileSize != 0 );
    }
    __finally
    {
        if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );
        if ( psHandle != INVALID_HANDLE_VALUE )
        {
            if ( bLeave ) RFALeave( psHandle );
            RFACloseHandle( psHandle );
        }
        if ( dwErr != NO_ERROR )
            NTSVCInfo( "SrvSendFull Erreur" );

        NTSVCInfo( "SrvSendFull Fin" );
    }

	return dwErr;
}




PRIVATE DWORD SrvProcessPingReq( TRFSVC_SERVER * psServer, FSYNC_MSG_PING_REQ * psReq )
{
    FSYNC_MSG_PING_RSP      sRsp;
    FSYNC_MSG_PING_RSP    * psRsp = &sRsp;
    DWORD                   dwErr = NO_ERROR;

    NTSVCInfo( "SrvProcessPingReq Début" );

    psRsp->dwMessageCode = FSYNC_CODE_PING_RSP;
    psRsp->dwCnxId = psReq->dwCnxId;
    
    dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_PING_RSP( psRsp ), psRsp );
    if ( dwErr != NO_ERROR )
        NTSVCInfo( "SrvProcessPingReq Erreur %u sur renvoi du pong", dwErr );

    NTSVCInfo( "SrvProcessPingReq Fin" );

    return dwErr;
}




PRIVATE DWORD SrvProcessPingRsp( TRFSVC_SERVER * psServer, FSYNC_MSG_PING_RSP * psRsp )
{
    NTSVCInfo( "SrvProcessPingRsp Début" );

    AComSendMessage( gsSvcWork.hWks, psRsp->dwCnxId, 0, 12, "PINGRESPONSE" );

    NTSVCInfo( "SrvProcessPingRsp Fin" );

    return NO_ERROR;
}


