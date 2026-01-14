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

#include <createdir.h>
#include <csr_rfa_svc.h>
#include <csr_rfa.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>
#include <trfsvc_protocol.h>
#include <trfsvc_main.h>

#define LOC_DEF
#include <trfsvc_clt_callback.h>
#undef LOC_DEF

#include <memclass.h>


PRIVATE DWORD CltProcessConnection( TRFSVC_CLIENT * psClient );
PRIVATE DWORD CltProcessIdRsp( TRFSVC_CLIENT * psClient, FSYNC_MSG_ID_RSP * psRsp );
PRIVATE DWORD CltProcessFullNf( TRFSVC_CLIENT * psClient, FSYNC_MSG_FULL_NF * psNf );
PRIVATE DWORD CltProcessPartialNf( TRFSVC_CLIENT * psClient, FSYNC_MSG_PARTIAL_NF * psNf );
PRIVATE DWORD CltProcessPingReq( TRFSVC_CLIENT * psClient, FSYNC_MSG_PING_REQ * psReq );
PRIVATE DWORD CltProcessPingRsp( TRFSVC_CLIENT * psClient, FSYNC_MSG_PING_RSP * psRsp );


PRIVATE DWORD gdwCltRequestId = 0;


// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CALLBACK CltCallbackConnection(
 *                     DWORD64 dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: Cf. doc ACOM
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Le client vient de se connecter. 
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CltCallbackConnection(
	DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD               dwErr = NO_ERROR;
    TRFSVC_FILE       * psFile = (TRFSVC_FILE*)dwInstUsrKey;
    TRFSVC_CLIENT     * psClient = NULL;
    HCOLLECTIONITEM     hItem = NULL;

    __try 
    {
        NTSVCInfo( "CltCallbackConnection(0x%08X)", hCnxHandle );

        psClient = HeapAlloc( 
            GetProcessHeap(), 
            HEAP_ZERO_MEMORY, 
            sizeof(*psClient) + gsSvcWork.sParmWork.dwMaxMsgSize - TRFSVC_PROTOCOL_BYTES );
        if ( psClient == NULL ) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            NTSVCInfo( "CltCallbackConnection Erreur allocation" );
            __leave; 
        }

        psClient->dwMaxBufferBytes = gsSvcWork.sParmWork.dwMaxMsgSize - TRFSVC_PROTOCOL_BYTES;
        psClient->hCnx = hCnxHandle;
        psClient->psFile = psFile;

        ColLock( gsSvcWork.hColClients );

        hItem = ColItemAdd( 
            gsSvcWork.hColClients, 
            &psClient->hCnx, 
            psClient, 
            sizeof(*psClient) + psClient->dwMaxBufferBytes );
        if ( hItem != NULL )
        {
            dwErr = CltProcessConnection( psClient );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackConnection Erreur connection" );
        }
        else
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            NTSVCInfo( "CltCallbackConnection Erreur collection" );
        }

        ColUnlock( gsSvcWork.hColClients );
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( ( psClient != NULL ) && ( hItem == NULL ) )
                HeapFree( GetProcessHeap(), 0, psClient );
            AComDisconnectPeer( gsSvcWork.hWks, hCnxHandle, FALSE );
            NTSVCInfo( "CltCallbackConnection Erreur" );
        }

        NTSVCInfo( "CltCallbackConnection Fin" );
    }

	return (DWORD64)hItem;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CltCallbackDisconnection(
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
PROTECTED void CALLBACK CltCallbackDisconnection(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey)
{
    HCOLLECTIONITEM     hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_CLIENT     * psClient;

    NTSVCInfo( "CltCallbackDisconnection(0x%08X)", dwCnxUsrKey );
    if ( hItem != NULL )
    {
        ColLock( gsSvcWork.hColClients );

        psClient = ColItemData( hItem );
        ColItemRemove( hItem );

        ColUnlock( gsSvcWork.hColClients );

        HeapFree( GetProcessHeap(), 0, psClient );
    }
    NTSVCInfo( "CltCallbackDisconnection Fin" );
    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CltCallbackReceived(
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
PROTECTED void CALLBACK CltCallbackReceived(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{
    DWORD dwErr = NO_ERROR;
    HCOLLECTIONITEM     hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_CLIENT     * psClient;

    NTSVCInfo( "CltCallbackReceived(0x%08X)", dwCnxUsrKey );

    if ( hItem != NULL )
    {
        ColLock( gsSvcWork.hColClients );

        psClient = ColItemData( hItem );

        if ( FSYNC_IS_ID_RSP( pbMsg, dwMsgSize ) )
        {
            dwErr = CltProcessIdRsp( psClient, (FSYNC_MSG_ID_RSP*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackReceived Erreur IdRsp" );
        }

        else if ( FSYNC_IS_FULL_NF( pbMsg, dwMsgSize ) )
        {
            dwErr = CltProcessFullNf( psClient, (FSYNC_MSG_FULL_NF*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackReceived Erreur FullNf" );
        }

        else if ( FSYNC_IS_PARTIAL_NF( pbMsg, dwMsgSize ) )
        {
            dwErr = CltProcessPartialNf( psClient, (FSYNC_MSG_PARTIAL_NF*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackReceived Erreur PartialNf" );
        }

        else if ( FSYNC_IS_PING_REQ( pbMsg, dwMsgSize ) )
        {
            dwErr = CltProcessPingReq( psClient, (FSYNC_MSG_PING_REQ*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackReceived Erreur PingReq" );
        }

        else if ( FSYNC_IS_PING_RSP( pbMsg, dwMsgSize ) )
        {
            dwErr = CltProcessPingRsp( psClient, (FSYNC_MSG_PING_RSP*)pbMsg );
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltCallbackReceived Erreur PingRsp" );
        }
        else
        {
            NTSVCInfo( "CltCallbackReceived Erreur Message inconnu" );
            dwErr = ERROR_INVALID_DATA;
        }

        if ( dwErr != NO_ERROR )
            AComDisconnectPeer( gsSvcWork.hWks, psClient->hCnx, FALSE );

        ColUnlock( gsSvcWork.hColClients );
    }

    NTSVCInfo( "CltCallbackReceived Fin" );

    return;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CltCallbackSent(
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
PROTECTED void CALLBACK CltCallbackSent(
	DWORD64 dwInstUsrKey,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    HCOLLECTIONITEM     hItem = (HCOLLECTIONITEM)dwCnxUsrKey;
    TRFSVC_CLIENT     * psClient;

    NTSVCInfo( "CltCallbackSent(0x%08X)", dwCnxUsrKey );

    // En cas d'erreur, on déconnecte, sinon, rien à faire
    if ( ( dwError != NO_ERROR ) && ( hItem != NULL ) )
    {
        NTSVCInfo( "CltCallbackSent Erreur" );
        ColLock( gsSvcWork.hColClients );

        psClient = ColItemData( hItem );
        AComDisconnectPeer( gsSvcWork.hWks, psClient->hCnx, FALSE );

        ColUnlock( gsSvcWork.hColClients );
    }

    NTSVCInfo( "CltCallbackSent Fin" );

    return;
}




PRIVATE DWORD CltProcessConnection( TRFSVC_CLIENT * psClient )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize;
    DWORD dwRemain;
    FSYNC_MSG_ID_REQ * psReq = NULL;
    char szComp[MAX_PATH];
    char szName[MAX_PATH];

    __try
    {
        NTSVCInfo( "CltCallbackSent Début" );

        if ( psClient->dwMaxBufferBytes < sizeof(*psReq) ) 
        { 
            dwErr = ERROR_INSUFFICIENT_BUFFER; 
            NTSVCInfo( "CltCallbackSent Erreur taille buffer" );
            __leave; 
        }

        psReq = (FSYNC_MSG_ID_REQ*)psClient->tbBuffer;

        dwSize = sizeof(szComp);
        if ( ! GetComputerName( szComp, &dwSize ) )
            strcpy_s( szComp, MAX_PATH, "#ERR#" );
        szComp[sizeof(szComp)-1] = '\0';

        _snprintf_s( 
            szName,
			MAX_PATH,
            sizeof(szName), 
            "\\\\%s\\%s",
            szComp,
            psClient->psFile->szFileKey );
        szName[sizeof(szName)-1] = '\0';

        psReq->dwMessageCode = FSYNC_CODE_ID_REQ;
        psReq->dwRequestId = InterlockedIncrement( &gdwCltRequestId );
        
        dwRemain = psClient->dwMaxBufferBytes - sizeof(*psReq);
        dwSize = (DWORD)strlen( szName );
        psReq->dwNameSize = __min( dwSize, dwRemain );

        dwRemain -= dwSize;
        dwSize = (DWORD)strlen( psClient->psFile->szFileKey );
        psReq->dwKeySize = __min( dwSize, dwRemain );
    
        memcpy( psReq->tbNameAndKey, szName, psReq->dwNameSize );
        memcpy( psReq->tbNameAndKey + psReq->dwNameSize, psClient->psFile->szFileKey, psReq->dwKeySize );
    }
    __finally
    {
        if ( psReq != NULL )
        {
            dwErr = AComSendMessage(
                gsSvcWork.hWks, 
                psClient->hCnx, 
                0,
                FSYNC_SIZEOF_ID_REQ( psReq ),
                psReq );
        }

        if ( dwErr != NO_ERROR )
            NTSVCInfo( "CltCallbackSent Erreur" );

        NTSVCInfo( "CltCallbackSent Fin" );
    }

	return dwErr;
}



PRIVATE DWORD CltProcessIdRsp( TRFSVC_CLIENT * psClient, FSYNC_MSG_ID_RSP * psRsp )
{
    DWORD dwErr = NO_ERROR;

    __try
    {
        NTSVCInfo( "CltProcessIdRsp Début" );

        if ( psRsp->dwResultCode != NO_ERROR ) 
        { 
            dwErr = psRsp->dwResultCode; 
            NTSVCInfo( "CltProcessIdRsp Erreur reportée par réponse" );
            __leave; 
        }
    }
    __finally
    {
        NTSVCInfo( "CltProcessIdRsp Fin" );
    }

	return dwErr;
}


PRIVATE DWORD CltProcessFullNf( TRFSVC_CLIENT * psClient, FSYNC_MSG_FULL_NF * psNf )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwWritten;
    BOOL bResult;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hFileRFA = INVALID_HANDLE_VALUE;
    ULONGLONG ullOffset;

    __try
    {
        NTSVCInfo( "CltProcessFullNf Début" );

        if ( ( psNf->ullOffset + (ULONGLONG)psNf->dwDataSize ) > psNf->ullFileSize ) 
        { 
            dwErr = ERROR_INVALID_DATA; 
            NTSVCInfo( "CltProcessFullNf Erreur incohérence sur les tailles" );
            __leave; 
        }

        if ( psNf->ullFileSize == (ULONGLONG)(-1) )
        {
            hFileRFA = RFACreateFile(
                psClient->psFile->szFileKey,
                RFA_SERVICE_ACCESS,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                (HANDLE)RFA_SERVICE_ACCESS );
            if ( hFileRFA == INVALID_HANDLE_VALUE ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessFullNf Erreur ouverture fichier RFA" );
                __leave; 
            }

            bResult = RFADeleteAndCloseHandle( hFileRFA );
            if ( ! bResult ) 
            {
                dwErr = GetLastError();
                NTSVCInfo( "CltProcessFullNf Erreur effacement fichier RFA" );
            }

            hFileRFA = INVALID_HANDLE_VALUE;
            __leave;
        }

        if ( psNf->ullOffset == 0 )
        {

            CreateFileDirectories( psClient->psFile->szTemp );

            hFile = CreateFile(
                psClient->psFile->szTemp,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL );
        }
        else
        {
            hFile = CreateFile(
                psClient->psFile->szTemp,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );
        }

        if ( hFile == INVALID_HANDLE_VALUE ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "CltProcessFullNf Erreur ouverture fichier Win32" );
            __leave; 
        }

        ullOffset = psNf->ullOffset;
        RFA_LL_LO(ullOffset) = SetFilePointer(
            hFile,
            RFA_LL_LO(ullOffset),
            &RFA_LL_HI(ullOffset),
            FILE_BEGIN );

        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullOffset) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) )
        {
            NTSVCInfo( "CltProcessFullNf Erreur placement dans fichier Win32" );
            __leave;
        }
        dwErr = NO_ERROR;

        bResult = WriteFile(
            hFile,
            psNf->tbData,
            psNf->dwDataSize,
            &dwWritten,
            NULL );
        if ( ! bResult ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "CltProcessFullNf Erreur écriture fichier Win32" );
            __leave; 
        }

		
		psClient->psFile->bIsReceivingFileInProgress = TRUE;
		psClient->psFile->ullLastTimeFullFileChunkReceived = GetSystemULLTime();

        if ( ( psNf->ullOffset + (ULONGLONG)psNf->dwDataSize ) == psNf->ullFileSize ) 
        {
			//this is the last part whatever the result enable CheckPeriod for the file
			psClient->psFile->bIsReceivingFileInProgress = FALSE;

            bResult = SetEndOfFile( hFile );
            if ( ! bResult ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessFullNf Erreur troncature fichier Win32" );
                __leave; 
            }

            bResult = SetFileTime( hFile, NULL, NULL, (LPFILETIME)&psNf->ullLastWrite );
            if ( ! bResult ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessFullNf Erreur changement heure fichier Win32" );
                __leave; 
            }

            CloseHandle( hFile );

            hFile = INVALID_HANDLE_VALUE;

            bResult = RFAMoveFileEx( 
                psClient->psFile->szTemp, 
                psClient->psFile->szFileKey, 
                MOVEFILE_REPLACE_EXISTING );
            if ( ! bResult ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessFullNf Erreur remplacement fichier RFA" );
                __leave; 
            }

			psClient->psFile->ullLastTime = psNf->ullLastWrite;
			NTSVCInfo("CltProcessFullNf ullLastTime:0x%016X .100ns", psClient->psFile->ullLastTime);
		}
    }
    __finally
    {
        if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );

        if ( dwErr != NO_ERROR )
            NTSVCInfo( "CltProcessFullNf Erreur" );

        NTSVCInfo( "CltProcessFullNf Fin" );
    }

	return dwErr;
}



PRIVATE DWORD CltProcessPartialNf( TRFSVC_CLIENT * psClient, FSYNC_MSG_PARTIAL_NF * psNf )
{
    DWORD dwErr = NO_ERROR;
    BOOL bResult;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwWritten;
    ULONGLONG ullOffset;

    __try
    {
        NTSVCInfo( "CltProcessPartialNf Début" );

        if ( ( psNf->ullOffset + (ULONGLONG)psNf->dwDataSize ) > psNf->ullFileSize ) 
        { 
            dwErr = ERROR_INVALID_DATA; 
            NTSVCInfo( "CltProcessPartialNf Erreur incohérence sur les tailles" );
            __leave; 
        }

        if ( psNf->ullFileSize == (ULONGLONG)(-1) )
        {
            hFile = RFACreateFile(
                psClient->psFile->szFileKey,
                RFA_SERVICE_ACCESS,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                (HANDLE)RFA_SERVICE_ACCESS );
            if ( hFile == INVALID_HANDLE_VALUE ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessPartialNf Erreur ouverture fichier RFA pour effacement" );
                __leave; 
            }

            bResult = RFADeleteAndCloseHandle( hFile );
            hFile = INVALID_HANDLE_VALUE;
            if ( ! bResult )
                dwErr = GetLastError();
            if ( dwErr == ERROR_FILE_NOT_FOUND ) dwErr = NO_ERROR;
            if ( dwErr != NO_ERROR )
                NTSVCInfo( "CltProcessPartialNf Erreur effacement fichier RFA" );

            __leave;
        }

        hFile = RFACreateFile(
            psClient->psFile->szFileKey,
            RFA_SERVICE_ACCESS,
            0,
            NULL,
            OPEN_ALWAYS,
            0,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "CltProcessPartialNf Erreur ouverture fichier RFA" );
            __leave; 
        }

        ullOffset = psNf->ullOffset;

        RFA_LL_LO(ullOffset) = RFASetFilePointer(
            hFile,
            RFA_LL_LO(ullOffset),
            &RFA_LL_HI(ullOffset),
            FILE_BEGIN );
        dwErr = GetLastError();
        if ( ( RFA_LL_LO(ullOffset) == INVALID_SET_FILE_POINTER ) && ( dwErr != NO_ERROR ) )
        {
            NTSVCInfo( "CltProcessPartialNf Erreur placement dans fichier RFA" );
            __leave;
        }
        dwErr = NO_ERROR;

        if ( psNf->dwDataSize )
        {
            bResult = RFAWriteFile( 
                hFile,
                psNf->tbData,
                psNf->dwDataSize,
                &dwWritten,
                NULL );
            if ( ! bResult ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessPartialNf Erreur écriture fichier RFA" );
                __leave; 
            }
            if ( dwWritten != psNf->dwDataSize ) 
            { 
                dwErr = ERROR_INVALID_DATA; 
                NTSVCInfo( "CltProcessPartialNf Erreur écriture incomplète fichier RFA" );
                __leave; 
            }
        }
        
        if ( ( psNf->ullOffset + (ULONGLONG)psNf->dwDataSize ) == psNf->ullFileSize ) 
        {
            bResult = RFASetEndOfFile( hFile );
            if ( ! bResult ) 
            { 
                dwErr = GetLastError(); 
                NTSVCInfo( "CltProcessPartialNf Erreur troncature fichier RFA" );
                __leave; 
            }
        }

        bResult = RFASetFileTime( hFile, NULL, NULL, (LPFILETIME)&psNf->ullLastWrite );
        if ( ! bResult ) 
        { 
            dwErr = GetLastError(); 
            NTSVCInfo( "CltProcessPartialNf Erreur changement heure fichier RFA" );
            __leave; 
        }
    }
    __finally
    {
        if ( hFile != INVALID_HANDLE_VALUE ) RFACloseHandle( hFile );

        if ( dwErr != NO_ERROR )
            NTSVCInfo( "CltProcessPartialNf Erreur" );

        NTSVCInfo( "CltProcessPartialNf Fin" );
    }

	return dwErr;
}






PRIVATE DWORD CltProcessPingReq( TRFSVC_CLIENT * psClient, FSYNC_MSG_PING_REQ * psReq )
{
    FSYNC_MSG_PING_RSP      sRsp;
    FSYNC_MSG_PING_RSP    * psRsp = &sRsp;
    DWORD                   dwErr = NO_ERROR;

    NTSVCInfo( "CltProcessPingReq Début" );

    psRsp->dwMessageCode = FSYNC_CODE_PING_RSP;
    psRsp->dwCnxId = psReq->dwCnxId;
    
    dwErr = AComSendMessage( gsSvcWork.hWks, psClient->hCnx, 0, FSYNC_SIZEOF_PING_RSP( psRsp ), psRsp );
    if ( dwErr != NO_ERROR )
        NTSVCInfo( "CltProcessPingReq Erreur %u sur renvoi du pong", dwErr );

    NTSVCInfo( "CltProcessPingReq Fin" );

    return dwErr;
}





PRIVATE DWORD CltProcessPingRsp( TRFSVC_CLIENT * psClient, FSYNC_MSG_PING_RSP * psRsp )
{
    NTSVCInfo( "CltProcessPingRsp Début" );

    AComSendMessage( gsSvcWork.hWks, psRsp->dwCnxId, 0, 12, "PINGRESPONSE" );

    NTSVCInfo( "CltProcessPingRsp Fin" );

    return NO_ERROR;
}


