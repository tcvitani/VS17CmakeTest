/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_tcp.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des connexions pour les liaisons socket TCP
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <winsock2.h>
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
#include <acom_tcp.h>
#undef LOC_DEF

#include <memclass.h>


typedef struct _ACOM_TCP_LISTEN
{
    SOCKADDR_IN                 sAddr;
    DWORD                       dwBindCount;
    SOCKET                      hListen;
    struct _ACOM_TCP_LISTEN   * psNext;
}
    ACOM_TCP_LISTEN;


PRIVATE ACOM_TCP_LISTEN * gpsListen = NULL;
PRIVATE CRITICAL_SECTION gsListenProtect;


PRIVATE DWORD gdwTcpInit = 0;

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpInit ( void )
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie l'API socket
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpInit ( void )
{
    DWORD   dwInit;
    DWORD   dwErr = NO_ERROR;
    DWORD   dwIntErr;
    WSADATA sWsaData;

    // On doit tout d'abord tester si WSA a été initialisé. Pour cela, on utilise un
    // pseudo jeton avec les fonctions InterlockedXXX.
    dwInit = InterlockedIncrement( &gdwTcpInit );
    if ( dwInit == 1 )
    {
        // C'est le premier appel, on effectue l'init WSA
        dwIntErr = WSAStartup( MAKEWORD(2,1), &sWsaData );
        if ( dwIntErr == SOCKET_ERROR )
        {
            InterlockedExchange( &gdwTcpInit, 0x00000000 );
            AComDbgInfo( __FILE__, __LINE__, "WSAStartup retourne %d", dwIntErr );
            dwErr = ERROR_REVISION_MISMATCH;
        }
        else
            InterlockedExchange( &gdwTcpInit, 0x80000000 );
        
        if ( dwErr == NO_ERROR )
            InitializeCriticalSection( &gsListenProtect );
    }
    else if ( dwInit < 0x80000000 )
    {
        // On est en cours d'init dans un autre thread, il faut en attendre la fin
        while ( InterlockedCompareExchange( &gdwTcpInit, 
                                            0x80000000, 
                                            0x80000000 ) != 0x80000000 ) 
            Sleep(10);
    }
    else
    {
        // WSA est déjà initialisé
        InterlockedDecrement( &gdwTcpInit );
    }


    return dwErr;
}

 
 
/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpServerPrepare ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initie la mise en écoute d'un serveur tcp.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpServerPrepare( ACOM_INSTANCE * psInst )
{
    DWORD dwInit;
    DWORD dwIntErr;
    SOCKADDR_IN sLocal;
    struct hostent * psHost;
    DWORD dwErr = NO_ERROR;
    ACOM_TCP_LISTEN * psScan;
    SOCKET hListen = INVALID_SOCKET;

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerPrepare(0x%016X)", psInst );

    ZeroMemory( &sLocal,sizeof(sLocal) );
    sLocal.sin_port = htons( psInst->sParams.sTcpServer.wPort );
    sLocal.sin_family = AF_INET;
    if ( psInst->szFileName[0] != 0 )
    {
        psHost = gethostbyname( psInst->szFileName );
        if ( psHost == NULL )
        {
            *(ULONG*)&sLocal.sin_addr = inet_addr( psInst->szFileName );
            if ( *(ULONG*)&sLocal.sin_addr == -1 )
                dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            sLocal.sin_addr = *(struct in_addr*)(psHost->h_addr);
        }
    }

    EnterCriticalSection( &gsListenProtect );

    psScan = gpsListen;

    while ( psScan != NULL )
        if ( memcmp( &psScan->sAddr, &sLocal, sizeof(sLocal) ) == 0 )
            break;
        else
            psScan = psScan->psNext;
        
    if ( psScan != NULL )
    {
        psScan->dwBindCount ++;
        hListen = psScan->hListen;
    }
    else
    {
        if ( DMEM_ALLOC( psScan ) == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            AComDbgInfo( __FILE__, __LINE__, "allocation mémoire retourne %d", dwErr );
        }

        if ( dwErr == NO_ERROR )
        {
            psScan->sAddr = sLocal;
            psScan->dwBindCount = 1;
            psScan->hListen = INVALID_SOCKET;

            psScan->hListen = socket( AF_INET, SOCK_STREAM, 0 );
            if ( psScan->hListen == INVALID_SOCKET )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "socket retourne %d", dwIntErr );
                dwErr = ERROR_UNEXP_NET_ERR;
            }
        }

        if ( dwErr == NO_ERROR )
        {
            dwIntErr = bind( psScan->hListen, (PSOCKADDR)&sLocal, sizeof(sLocal) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "bind retourne %d", dwIntErr );
                if ( dwIntErr == WSAEADDRINUSE )
                    dwErr = ERROR_ALREADY_EXISTS;
                else
                    dwErr = ERROR_UNEXP_NET_ERR;
            }
        }

        if ( dwErr == NO_ERROR )
        {
            dwIntErr = listen( psScan->hListen, psInst->sParams.sTcpServer.dwMaxConnections + 1 );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "listen retourne %d", dwIntErr );
                if ( dwIntErr == WSAEADDRINUSE )
                    dwErr = ERROR_ALREADY_EXISTS;
                else
                    dwErr = ERROR_UNEXP_NET_ERR;
            }
        }

        if ( dwErr == NO_ERROR )
        {
            // L'attente de connexion ne doit pas être blocante
            dwInit = 1;
            dwIntErr = ioctlsocket( psScan->hListen, FIONBIO, &dwInit );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "ioctlsocket retourne %d", dwIntErr );
                dwErr = ERROR_UNEXP_NET_ERR;
            }
        }

        if ( dwErr != NO_ERROR ) 
        {
            if ( psScan != NULL )
            {
                if ( psScan->hListen != INVALID_SOCKET )
                    closesocket( (SOCKET)psScan->hListen );
                DMEM_FREE(psScan);
            }
        }
        else
        {
            hListen = psScan->hListen;
            psScan->psNext = gpsListen;
            gpsListen = psScan;
        }
    }

    LeaveCriticalSection( &gsListenProtect );

    psInst->hListener = hListen;

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerPrepate return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComTcpServerFinish ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Termine l'écoute d'un serveur tcp.
 * --------------------------------------------------------------------
 */
PROTECTED void AComTcpServerFinish( ACOM_INSTANCE * psInst )
{
    ACOM_TCP_LISTEN * psScan;
    ACOM_TCP_LISTEN * psPrev;

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerFinish(0x%016X)", psInst );

    EnterCriticalSection( &gsListenProtect );

    psScan = gpsListen;
    psPrev = NULL;
    while ( psScan != NULL )
    {
        if ( psScan->hListen == psInst->hListener )
            break;
        else
        {
            psPrev = psScan;
            psScan = psScan->psNext;
        }
    }

    if ( psScan != NULL )
    {
        psScan->dwBindCount --;
        if ( psScan->dwBindCount == 0 )
        {
            closesocket( psScan->hListen );
            if ( psPrev != NULL )
                psPrev->psNext = psScan->psNext;
            else
                gpsListen = psScan->psNext;
            DMEM_FREE( psScan );
        }
    }

    LeaveCriticalSection( &gsListenProtect );

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerFinish return" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpServerConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un serveur tcp. En fait,
 *             tente de créér le serveur et de le mettre en attente
 *             de connexion. Si la création échoue, la connexion reste à l'état
 *             "déconnecté", sinon, elle passe à l'état "en cours de connexion".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpServerConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    DWORD dwIntErr;
    BOOL bFoo;
    DWORD dwErr = NO_ERROR;
    int iLen;
    struct sockaddr_in sAddr;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : TCP SERVER

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerConnect(0x%016X)", psCnx );

    iLen = sizeof( sAddr );
    psCnx->hFile = (HANDLE)accept( psCnx->psInst->hListener, (struct sockaddr*)&sAddr, &iLen );
    if ( psCnx->hFile != (HANDLE)INVALID_SOCKET )
    {
        dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                               SOL_SOCKET, 
                               SO_RCVBUF,
                               (char*)&psCnx->psInst->sParams.sTcpServer.dwInBufferSize,
                               sizeof(psCnx->psInst->sParams.sTcpServer.dwInBufferSize) );
        if ( dwIntErr == SOCKET_ERROR )
        {
            dwIntErr = WSAGetLastError();
            AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_RCVBUF) retourne %d", dwIntErr );
            dwErr = ERROR_INVALID_DATA;
        }

        if ( dwErr == NO_ERROR )
        {
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   SOL_SOCKET, 
                                   SO_SNDBUF,
                                   (char*)&psCnx->psInst->sParams.sTcpServer.dwOutBufferSize,
                                   sizeof(psCnx->psInst->sParams.sTcpServer.dwOutBufferSize) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_SNDBUF) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( dwErr == NO_ERROR )
        {
            bFoo = TRUE;
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   SOL_SOCKET, 
                                   SO_DONTLINGER,
                                   (char*)&bFoo,
                                   sizeof(bFoo) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_DONTLINGER) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( dwErr == NO_ERROR )
        {
            bFoo = TRUE;
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   IPPROTO_TCP, 
                                   TCP_NODELAY,
                                   (char*)&bFoo,
                                   sizeof(bFoo) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(TCP_NODELAY) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( dwErr == NO_ERROR )
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
                closesocket( (SOCKET)psCnx->hFile );
                psCnx->hFile = NULL;
                AComDbgInfo( __FILE__, __LINE__, "CreateIoCompletionPort retourne %d", dwIntErr );
                // On reste à l'état déconnecté, ce n'est pas une erreur
            }
            else
            {
                psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
                AComTcpGetIdentity( psCnx, &sAddr );
                AComStatsReset( psCnx );
                dwErr = AComClbkConnection( psCnx );
                if ( dwErr == NO_ERROR )
                    dwErr = AComWorkerReceiveNext( psCnx );
            }
        }
    }
    else
    {
        psCnx->hFile = NULL;
        dwIntErr = WSAGetLastError();
        AComDbgInfo( __FILE__, __LINE__, "accept retourne %d", dwIntErr );
        // Aucun client en attente, ce n'est pas une erreur ...
    }

    if ( ( dwErr != NO_ERROR ) && ( psCnx->hFile != NULL ) )
    {
        closesocket( (SOCKET)psCnx->hFile );
        psCnx->hFile = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComTcpServerConnect return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComTcpClientConnect ( ACOM_CONNECTION * psCnx )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 * RETURN    : NO_ERROR si ok, une erreur win32 si erreur critique
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la "connexion" d'un client tcp. En fait,
 *             tente de créér le client. Si la création échoue,
 *             la connexion reste à l'état "déconnecté", sinon, elle
 *             passe à l'état "connecté".
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComTcpClientConnect( ACOM_CONNECTION * psCnx )
{
    HANDLE hCompletion;
    DWORD dwErr = NO_ERROR;
    DWORD dwIntErr;
    BOOL bFoo;
    BOOL bPending = FALSE;
    DWORD dwFoo;
    BOOL bValidAddress = FALSE;
    SOCKADDR_IN sAddr;
    struct hostent * psHost;

    // A l'entrée de cette fonction, on suppose que :
    //    *  ETAT : DISCONNECTED
    //    *  TYPE : CLIENT TCP

    AComDbgInfo( __FILE__, __LINE__, "AComTcpClientConnect(0x%016X)", psCnx );

    if ( psCnx->hFile == NULL )
    {
        //
        // Construire l'adresse
        //
        ZeroMemory( &sAddr, sizeof(sAddr) );
        sAddr.sin_family = AF_INET;
        sAddr.sin_port = htons( psCnx->psInst->sParams.sTcpClient.wPort );
        psHost = gethostbyname( psCnx->psInst->szFileName );
        if ( psHost == NULL )
        {
            *(ULONG*)&sAddr.sin_addr = inet_addr( psCnx->psInst->szFileName );
            bValidAddress = ( *(ULONG*)&sAddr.sin_addr != -1 );
        }
        else
        {
            sAddr.sin_addr = *(struct in_addr*)(psHost->h_addr);
            bValidAddress = TRUE;
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            psCnx->hFile = (HANDLE)socket( AF_INET, SOCK_STREAM, 0 );
            if ( psCnx->hFile == (HANDLE)INVALID_SOCKET )
            {
                psCnx->hFile = NULL;
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "socket retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   SOL_SOCKET, 
                                   SO_RCVBUF,
                                   (char*)&psCnx->psInst->sParams.sTcpClient.dwInBufferSize,
                                   sizeof(psCnx->psInst->sParams.sTcpClient.dwInBufferSize) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_RCVBUF) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   SOL_SOCKET, 
                                   SO_SNDBUF,
                                   (char*)&psCnx->psInst->sParams.sTcpClient.dwOutBufferSize,
                                   sizeof(psCnx->psInst->sParams.sTcpClient.dwOutBufferSize) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_SNDBUF) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            bFoo = TRUE;
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   SOL_SOCKET, 
                                   SO_DONTLINGER,
                                   (char*)&bFoo,
                                   sizeof(bFoo) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(SO_DONTLINGER) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            bFoo = TRUE;
            dwIntErr = setsockopt( (SOCKET)psCnx->hFile,
                                   IPPROTO_TCP, 
                                   TCP_NODELAY,
                                   (char*)&bFoo,
                                   sizeof(bFoo) );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "setsockopt(TCP_NODELAY) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if ( ( dwErr == NO_ERROR ) && bValidAddress )
        {
            dwFoo = 1;
            dwIntErr = ioctlsocket( (SOCKET)psCnx->hFile, FIONBIO, &dwFoo );
            if ( dwIntErr == SOCKET_ERROR )
            {
                dwIntErr = WSAGetLastError();
                AComDbgInfo( __FILE__, __LINE__, "ioctlsocket(FIONBIO) retourne %d", dwIntErr );
                dwErr = ERROR_INVALID_DATA;
            }
        }
    }
    else
        bValidAddress = TRUE;

    if ( ( dwErr == NO_ERROR ) && bValidAddress )
    {
        dwIntErr = connect( (SOCKET)psCnx->hFile, (PSOCKADDR)&sAddr, sizeof(sAddr) );
        if ( dwIntErr == SOCKET_ERROR )
        {
            dwIntErr = WSAGetLastError();
            if ( ( dwIntErr == WSAEWOULDBLOCK ) ||
                 ( dwIntErr == WSAEINVAL      ) ||
                 ( dwIntErr == WSAEALREADY    ) )
            {
                // Connection pending
                bPending = TRUE;
            }
            else if ( dwIntErr == WSAEISCONN )
            {
                // Connection effectuée
            }
            else
            {
                AComDbgInfo( __FILE__, __LINE__, "connect retourne %d", dwIntErr );
                closesocket( (SOCKET)psCnx->hFile );
                psCnx->hFile = NULL;
                // Echec de connexion, le serveur n'est pas la, ce n'est pas une erreur
            }
        }
    }

    if ( ( dwErr == NO_ERROR    ) && 
         ( bValidAddress        ) && 
         ( ! bPending           ) &&
         ( psCnx->hFile != NULL ) )
    {
        // On repasse en bloquant
        dwFoo = 0;
        dwIntErr = ioctlsocket( (SOCKET)psCnx->hFile, FIONBIO, &dwFoo );
        if ( dwIntErr == SOCKET_ERROR )
        {
            dwIntErr = WSAGetLastError();
            AComDbgInfo( __FILE__, __LINE__, "ioctlsocket(FIONBIO) retourne %d", dwIntErr );
            dwErr = ERROR_INVALID_DATA;
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
                closesocket( (SOCKET)psCnx->hFile );
                psCnx->hFile = NULL;
                AComDbgInfo( __FILE__, __LINE__, "CreateIoCompletionPort retourne %d", dwIntErr );
            }
            else
            {
                psCnx->dwState = ACOM_CONNECTION_STATE_CONNECTED;
                AComTcpGetIdentity( psCnx, NULL );
                AComStatsReset( psCnx );
                dwErr = AComClbkConnection( psCnx );
                if ( dwErr == NO_ERROR )
                    dwErr = AComWorkerReceiveNext( psCnx );
            }
        }
    }

    if ( ( dwErr != NO_ERROR ) && ( psCnx->hFile != NULL ) )
    {
        closesocket( (SOCKET)psCnx->hFile );
        psCnx->hFile = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComTcpClientConnect return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComTcpGetIdentity( ACOM_CONNECTION * psCnx, 
 *                                                struct sockaddr_in * psAddr )
 * PARAMETERS: ACOM_CONNECTION * psCnx : Structure de connexion
 *             struct sockaddr_in * psAddr : Pointeur sur la structure d'adresse du connecté ou NULL
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise l'identité de la connexion (tcp = identifié pour serveur uniquement)
 * REMARQUE  : Suppose que le slot de connexion est bien associé à
 *             une connexion TCP (client ou serveur), dans un état "connecté" et
 *             qu'il est en accés exclusif.
 * --------------------------------------------------------------------
 */
PROTECTED void AComTcpGetIdentity( ACOM_CONNECTION * psCnx, 
                                   struct sockaddr_in * psAddr )
{
    struct sockaddr_in sAddr;
    int iLen;
    // struct hostent * psHost;
    char * pcAddr;

    if ( psCnx->psInst->dwType == ACOM_INSTANCE_TCP_SERVER )
    {
        if ( ( psCnx->psWks->dwProtocol & ACOM_FLAG_NO_GET_PEER_NAME ) == 0 )
        {
            iLen = sizeof(sAddr);
            if ( getpeername( (SOCKET)psCnx->hFile, (struct sockaddr*)&sAddr, &iLen ) == SOCKET_ERROR )
            {
                sprintf_s( psCnx->szIdentity, 256, "#TCPSERVER-WSAERR%u#", WSAGetLastError() );
            }
            else if ( sAddr.sin_family != AF_INET )
            {
                sprintf_s( psCnx->szIdentity, 256, "#TCPSERVER-ERR%u#", ERROR_INVALID_DATA );
            }
            else
            {
                pcAddr = inet_ntoa(sAddr.sin_addr);
                strcpy_s( psCnx->szIdentity, 256, pcAddr == NULL ? "#TCPSERVER-ERR#" : pcAddr );
            }
        }
        else if ( psAddr != NULL )
        {
            pcAddr = inet_ntoa(psAddr->sin_addr);
            strcpy_s( psCnx->szIdentity, 256, pcAddr == NULL ? "#TCPSERVER-ERR#" : pcAddr );
        }
        else
            strcpy_s( psCnx->szIdentity, 256, "#TCPCLIENT#" );
    }
    else
        strcpy_s( psCnx->szIdentity, 256, "#TCPCLIENT#" );
}



/* -------------  FIN DU FICHIER : acom_tcp.c ------------- */ 


