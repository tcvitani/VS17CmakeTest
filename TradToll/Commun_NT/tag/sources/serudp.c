/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SERUdp
 * FILE       : SERUdp.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */


#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#include "ser.h"
#define LOC_DEF
#include "SERUdp.h"
#undef LOC_DEF

#include <memclass.h>


#pragma comment( lib, "WS2_32.LIB" )


//
// Définition de l'objet SER, seul le nom du type de la structure est
// exporté, la définition complète reste privée
//
typedef struct _SERUDP_INSTANCE
{
    SER_TYPE            eType;

    // Numéro du port série émulé
    DWORD               dwPort;

    // Socket du port UDP
    SOCKET              hSock;

    // Handle de l'événement utilisé pour les overlapped i/o
    HANDLE              hEventRead;

    // Structure utilisée pour les overlapped i/o
    OVERLAPPED          sOverRead;

    // Handle de l'événement utilisé pour les overlapped i/o
    HANDLE              hEventWrite;
    
    // Structure utilisée pour les overlapped i/o
    OVERLAPPED          sOverWrite;
    
    // Indicateur d'input pending
    BOOL                fPending;

    // Position du prochain octet à lire
    DWORD               dwNextByte;

    // Nombre d'octets à partir de la position du prochain
    DWORD               dwBytes;

    // Peer
    struct sockaddr_in  listenAddrIn;
    DWORD               dwListenAddrInBytes;

    // Last packet number received
    WORD                 wLastInPacketNumber;
    DWORD                dwLastInTick;

    // Peer
    struct sockaddr_in  peerAddrIn;
    DWORD               dwPeerAddrInBytes;

    // Reception
    struct sockaddr_in  inAddrIn;
    DWORD               dwInAddrInBytes;
    DWORD               dwInBufferBytes;
    WSABUF              inBufferHeader;
    BYTE                inBuffer[SERUDP_INBUF_BYTES];

    // Emission
    DWORD               dwOutBufferBytes;
    WSABUF              outBufferHeader;
    BYTE                outBuffer[SERUDP_OUTBUF_BYTES];

}
    SERUDP_INSTANCE;


// Last packet number sent
static WORD gwLastOutPacketNumber;



static DWORD SERUdpInitSocket();
static void SERDebug( const char * szFormat, ... );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpOpen(
 *                      OUT SER_INSTANCE    ** ppsPort,
 *                      IN const char *        cszListenAddress,
 *                      IN WORD                wListenPort,
 *                      IN const char *        cszPeerAddress,
 *                      IN WORD                wPeerPort )
 * PARAMETRES: ppsPort     : Récupère un "handle" de l'objet créé
 *             cszListenAddress  : Addresse d'écoute, chaine vide ou null
 *                           si écoute sur toutes les interfaces.
 *             wListenPort : Numéro du port UDP d'écoute
 *             cszPeerAddress  : Remote address
 *             wPeerPort : Numéro du port distant
 * RETOURNE  : NO_ERROR si l'objet a été crée, sinon, un code Win32
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une instance de l'objet SERUdp. Ce même objet
 *             pourra être détruit avec la fonction SERUdpClose().
 *             Pour obtenir un handle sur lequel peuvent être effectuée
 *             des opérations du type WaitForXXXX(), utiliser
 *             la fonction SERUdpGetWaitableHandle().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpOpen(
        OUT SERUDP_INSTANCE ** ppsPort,
        IN const char *        cszListenAddress,
        IN WORD                wListenPort,
        IN const char *        cszPeerAddress,
        IN WORD                wPeerPort )
{
    DWORD           dwErr       = NO_ERROR;     // Code d'erreur courant
    SERUDP_INSTANCE  * psPort      = NULL;         // Pointeur de travail
    ULONG           ulListenAddr;
    ULONG           ulPeerAddr;
    DWORD dwInBufferSize = SERUDP_INNER_INBUF_BYTES;
    DWORD dwOutBufferSize = SERUDP_INNER_OUTBUF_BYTES;
    BOOL fReuseAddr = TRUE;
    DWORD dwFlags;

    __try
    {
        psPort = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psPort ) );
        if ( ! psPort ) 
        { 
            //
            // La tentative d'allocation d'un bloc a échoué
            //
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            __leave; 
        }

        psPort->eType = SER_UDP;
        dwErr = SERUdpInitSocket();
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // On doit créer un événement pour chaque type d'i/o overlapped
        // qu'on va utiliser. Dans notre cas : READ / WRITE
        //
        psPort->hEventRead = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( ! psPort->hEventRead ) 
        {
            //
            // Impossible de créer l'événement de gestions des
            // lectures en mode overlapped
            //
            dwErr = GetLastError(); 
            __leave; 
        }
        psPort->hEventWrite = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( ! psPort->hEventWrite ) 
        { 
            //
            // Impossible de créer l'événement de gestions des
            // écriture en mode overlapped
            //
            dwErr = GetLastError(); 
            __leave; 
        }


        psPort->hSock = WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
        if ( psPort->hSock == INVALID_SOCKET )
        {
            dwErr = WSAGetLastError();
            __leave;
        }

        if ( setsockopt( psPort->hSock, SOL_SOCKET, SO_RCVBUF, (char*)&dwInBufferSize, sizeof(dwInBufferSize) ) == SOCKET_ERROR ) 
        {
            dwErr = WSAGetLastError();
            __leave;
        }
        if ( setsockopt( psPort->hSock, SOL_SOCKET, SO_SNDBUF, (char*)&dwInBufferSize, sizeof(dwOutBufferSize) ) == SOCKET_ERROR )
        {
            dwErr = WSAGetLastError();
            __leave;
        }
        if ( setsockopt( psPort->hSock, SOL_SOCKET, SO_REUSEADDR, (char*)&fReuseAddr, sizeof(fReuseAddr) ) == SOCKET_ERROR )
        {
            dwErr = WSAGetLastError();
            __leave;
        }

        if ( cszListenAddress == NULL )
            ulListenAddr = INADDR_ANY;
        else if ( cszListenAddress[0] == 0 )
            ulListenAddr = INADDR_ANY;
        else
        {
            ulListenAddr = inet_addr( cszListenAddress );
            if ( ulListenAddr == INADDR_NONE )
                ulListenAddr = INADDR_LOOPBACK;
        }

        psPort->listenAddrIn.sin_family = AF_INET;
        psPort->listenAddrIn.sin_addr.s_addr = ulListenAddr;
        psPort->listenAddrIn.sin_port = htons(wListenPort);

        if ( bind( psPort->hSock, (struct sockaddr*)&psPort->listenAddrIn, sizeof(psPort->listenAddrIn) ) == SOCKET_ERROR )
        {
            dwErr = WSAGetLastError();
            __leave;
        }


        if ( cszPeerAddress == NULL )
            ulPeerAddr = INADDR_LOOPBACK;
        else if ( cszPeerAddress[0] == 0 )
            ulPeerAddr = INADDR_LOOPBACK;
        else
        {
            ulPeerAddr = inet_addr( cszPeerAddress );
            if ( ulPeerAddr == INADDR_NONE )
                ulPeerAddr = INADDR_LOOPBACK;
        }

        psPort->peerAddrIn.sin_family = AF_INET;
        psPort->peerAddrIn.sin_addr.s_addr = ulPeerAddr;
        psPort->peerAddrIn.sin_port = htons(wPeerPort);

        //
        // Initialiser la structure overlapped pour la lecture
        //
        SERDebug( "Arming receive" );
        ZeroMemory( &psPort->sOverRead, sizeof( psPort->sOverRead ) );
        ResetEvent( psPort->sOverRead.hEvent = psPort->hEventRead );
        psPort->inBufferHeader.len = sizeof(psPort->inBuffer);
        psPort->inBufferHeader.buf = psPort->inBuffer;
        psPort->dwInBufferBytes = 0;
        dwFlags = 0;
        psPort->dwInAddrInBytes = sizeof(psPort->peerAddrIn);
        if ( WSARecvFrom(    psPort->hSock, 
                             &psPort->inBufferHeader, 
                             1, 
                             &psPort->dwInBufferBytes,
                             &dwFlags,
                             (struct sockaddr*)&psPort->inAddrIn,
                             (int*)&psPort->dwInAddrInBytes,
                             &psPort->sOverRead,
                             NULL ) == SOCKET_ERROR )
        {
            dwErr = WSAGetLastError();
            dwErr = ( dwErr == WSA_IO_PENDING  ? NO_ERROR : dwErr );
            if ( dwErr != NO_ERROR )
            {
                SERDebug( "Receive error %u", dwErr );
                __leave;
            }
        }

        psPort->fPending = TRUE;
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            //
            // Une erreur a eu lieu on libère tout ce qui a été alloué
            //
            SERUdpClose( psPort );

            psPort = NULL;
        }

        //
        // Retourner le handle de l'objet créé ou NULL
        (*ppsPort) = psPort;

    }

    return dwErr;
}


    

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED void WINAPI SERUdpClose(
 *                      IN SER_INSTANCE  * psPort )
 * PARAMETRES: psPort : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Destruction d'une instance de l'objet SER créé
 *             avec SEROpen(). Si un handle a été obtenu par l'intermédiaire
 *             de l'objet avec la fontion SERGetWaitableHandle(), celui-ci
 *             est également fermé.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED void WINAPI SERUdpClose(
        IN SERUDP_INSTANCE  * psPort )
{
    //
    // On s'assure d'abord que le bloc de l'objet a effectivement été alloué
    //
    if ( psPort != NULL )
    {

        if ( ( psPort->hSock != (SOCKET)NULL ) && ( psPort->hSock != INVALID_SOCKET ) )
        {
            //
            // Le port a été ouvert. On annule donc tout i/o en court et
            // surtout, on referme le port
            //
            CancelIo( (HANDLE)(psPort->hSock) );
            closesocket( psPort->hSock );
        }

        //
        // Fermeture des événements pour les overlapped i/o
        //
        if ( psPort->hEventRead != NULL )
            CloseHandle( psPort->hEventRead );
        if ( psPort->hEventWrite != NULL )
            CloseHandle( psPort->hEventWrite );

        //
        // Finalement libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psPort );
    }
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED HANDLE WINAPI SERUdpGetWaitableHandle(
 *                      IN SER_INSTANCE  * psPort )
 * PARAMETRES: psPort : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Un handle d'objet utilisable avec les fontions WaitForXXX
 *             qui passe à l'état signalé lorsque des caractères sont
 *             présents dans le buffer de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir un handle lié au port sur lequel on peut
 *             utiliser les fontions WaitForXXX
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED HANDLE WINAPI SERUdpGetWaitableHandle(
        IN SERUDP_INSTANCE  * psPort )
{
    return psPort->hEventRead;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpWrite(
 *                      IN SERUDP_INSTANCE  * psPort,
 *                      IN BYTE           * pbBuffer,
 *                      IN DWORD            dwBufferBytes )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pbBuffer      : pointe sur le buffer contenant les octets à envoyer
 *             dwBufferBytes : Nombre d'octets pointés par pbBuffer
 * RETOURNE  : NO_ERROR lorsque tous les octets ont été envoyées,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Envoyer une série d'octets sur une liaison série
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpWrite(
        IN SERUDP_INSTANCE * psPort,
        IN BYTE           *  pbBuffer,
        IN DWORD             dwBufferBytes )
{
    DWORD   dwErr           = NO_ERROR; // Code d'erreur courant
    DWORD   dwSentBytes;                // Nombre d'octets envoyées
    DWORD   dwDataBytesToSend;
    DWORD   dwPacketBytesToSend;
    BOOL    bResult;
    DWORD   dwFlags;

    __try
    {
        //
        // Boucler tant qu'il y a des octets à envoyer
        //
        while ( dwBufferBytes > 0 )
        {
            if ( dwBufferBytes > ( sizeof(psPort->outBuffer) - sizeof(WORD) ) )
            {
                dwPacketBytesToSend = sizeof(psPort->outBuffer);
                dwDataBytesToSend = sizeof(psPort->outBuffer) - sizeof(WORD);
            }
            else
            {
                dwPacketBytesToSend = dwBufferBytes + sizeof(WORD);
                dwDataBytesToSend = dwBufferBytes;
            }

            gwLastOutPacketNumber++;
            if ( gwLastOutPacketNumber == 0 )
                gwLastOutPacketNumber = 1;

            *(WORD*)(psPort->outBuffer) = htons(gwLastOutPacketNumber);
            CopyMemory( psPort->outBuffer + sizeof(WORD), pbBuffer, dwDataBytesToSend );

            //
            // Initialiser la structure overlapped d'écriture
            //
            ZeroMemory( &psPort->sOverWrite, sizeof( psPort->sOverWrite ) );

            //
            // S'assurer que l'événement est non signalé
            //
            ResetEvent( psPort->sOverWrite.hEvent = psPort->hEventWrite );


            psPort->outBufferHeader.len = dwPacketBytesToSend;
            psPort->outBufferHeader.buf = psPort->outBuffer;
            dwSentBytes = 0;
            
            if ( WSASendTo( psPort->hSock, 
                            &psPort->outBufferHeader, 
                            1,
                            &dwSentBytes,
                            0,
                            (struct sockaddr*)&psPort->peerAddrIn,
                            sizeof(psPort->peerAddrIn),
                            &psPort->sOverWrite,
                            NULL ) == SOCKET_ERROR )
            {
                dwErr = WSAGetLastError();
                dwErr = ( dwErr == WSA_IO_PENDING  ? NO_ERROR : dwErr );
                if ( dwErr != NO_ERROR )
                    __leave;
            }

            bResult = WSAGetOverlappedResult(
                psPort->hSock,
                &psPort->sOverWrite,
                &dwSentBytes,
                TRUE,
                &dwFlags );
            if ( ! bResult ) 
            { 
                //
                // Une erreur a eu lieu pendant l'émission
                //
                dwErr = WSAGetLastError(); 
                __leave; 
            }

            if ( dwSentBytes != dwPacketBytesToSend )
            { 
                dwErr = ERROR_OPERATION_ABORTED; 
                __leave; 
            };

            //
            // Mettre à jour le nombre d'octet restant à émettre,
            // ainsi que le pointeur du premier octet à émettre.
            //
            dwBufferBytes -= dwDataBytesToSend;
            pbBuffer += dwDataBytesToSend;
        }
    }
    __finally
    {
    }

    return dwErr;
}
            

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpRead(
 *                      IN SERUDP_INSTANCE      * psPort,
 *                      IN OPTIONAL BYTE      * pbBuffer,
 *                      IN OUT OPTIONAL DWORD * pdwBufferBytes )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pbBuffer      : pointe sur le buffer récupérant les octets
 *             pdwBufferBytes: En entrée, nombre max d'octets dans le buffer
 *                             En sortie, nombre d'octets reçus
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Recevoir des octets sur la liaison série.
 *             Si aucun octet disponible, (*pdwBufferBytes) = 0
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpRead(
        IN SERUDP_INSTANCE       * psPort,
        IN OPTIONAL BYTE      * pbBuffer,
        IN OUT OPTIONAL DWORD * pdwBufferBytes )
{
    DWORD   dwErr       = NO_ERROR; // Code d'erreur courant
    DWORD   dwBytes     = 0;        // Nombre d'octets à placer dans le buffer utilisteur
    DWORD   dwRead;                 // Nombre d'octets à placer dans le buffer utilisteur
    DWORD   dwRemain;               // Reste à combler
    BOOL    bResult;                // Résultat d'appel système
    DWORD   dwFlags;
    DWORD   dwTick;

    __try
    {
        //
        // Tant que le buffer de lecture n'est pas saturé
        //
        while ( dwBytes < *pdwBufferBytes )
        {
            //
            // Si le buffer contient déjà des infos, on les récupère
            //
            if ( psPort->dwBytes > 0 )
            {
                //
                // Nombre d'octets restant à récupérer
                //
                dwRemain = ( *pdwBufferBytes - dwBytes );

                if ( dwRemain < psPort->dwBytes )
                {
                    //
                    // Il y a suffisament d'octets, on récupère ce dont on a besoin
                    // et on sort.
                    //

                    CopyMemory( pbBuffer + dwBytes, 
                                psPort->inBuffer + psPort->dwNextByte, 
                                dwRemain );
                    dwBytes += dwRemain;
                    psPort->dwNextByte += dwRemain;
                    psPort->dwBytes -= dwRemain;

                    dwErr = NO_ERROR;
                    __leave;
                }
                else
                {
                    //
                    // Le buffer n'a pas assez d'infos demandées, on récupère ce qu'on
                    // peut sans sortir pour ensuite aller voir ce qu'on a en résultat
                    // overlapped.
                    //

                    CopyMemory( pbBuffer + dwBytes, 
                                psPort->inBuffer + psPort->dwNextByte, 
                                psPort->dwBytes );
                    dwBytes += psPort->dwBytes;
                    psPort->dwBytes = 0;
                    psPort->dwNextByte = 0;
                }
            }       

            //
            // A ce niveau, le buffer est vide.
            //
            psPort->dwNextByte = 0;
            psPort->dwBytes = 0;


            if ( psPort->fPending )
            {
                //
                // Si on est pending, on teste l'état de la lecture assynchrone
                //
                bResult = WSAGetOverlappedResult(
                    psPort->hSock,
                    &psPort->sOverRead,
                    &dwRead,
                    FALSE,
                    &dwFlags);
                if ( ! bResult )
                {
                    dwErr = WSAGetLastError();
                    if ( dwErr != WSA_IO_PENDING )
                    {
                        dwBytes = 0;
                        __leave;
                    }
                    break;
                }

                psPort->fPending = FALSE;
                dwTick = GetTickCount();

                if ( ( psPort->peerAddrIn.sin_addr.s_addr != psPort->inAddrIn.sin_addr.s_addr ) ||
                     ( psPort->peerAddrIn.sin_port        != psPort->inAddrIn.sin_port        ) )
                {
                    psPort->dwBytes = 0;
                    SERDebug( "Packet received from unexpected port/address" );
                }
                else if ( dwRead >= sizeof(WORD) )
                {
                    BOOL fAccepted = FALSE;
                    WORD wPacketNumber = ntohs(*(WORD*)(psPort->inBuffer));

                    SERDebug( "Packet %u received - %u bytes", (int)wPacketNumber, dwRead );
                    
                    if ( wPacketNumber > psPort-> wLastInPacketNumber )
                        fAccepted = ( wPacketNumber - psPort-> wLastInPacketNumber ) < 0x7FFF;
                    else if ( wPacketNumber < psPort-> wLastInPacketNumber )
                        fAccepted = ( psPort-> wLastInPacketNumber - wPacketNumber ) > 0x7FFF;

                    if ( ( ! fAccepted ) && ( ( dwTick - psPort->dwLastInTick ) >= SERUDP_RESET_TIME_MS ) )
                        fAccepted = TRUE;

                    if ( fAccepted )
                    {
                        SERDebug( "Packet %u accepted - %u emulated serial bytes", (int)wPacketNumber, dwRead - sizeof(WORD) );
                        psPort->wLastInPacketNumber = wPacketNumber;
                        psPort->dwLastInTick = dwTick;
                        if ( dwRead > sizeof(WORD) )
                        {
                            psPort->dwNextByte = sizeof(WORD);
                            psPort->dwBytes = dwRead - sizeof(WORD);
                        }
                        else
                        {
                            psPort->dwBytes = 0;
                            psPort->dwNextByte = 0;
                        }

                    }
                    else
                    {
                        SERDebug( "Packet %u filtered", (int)wPacketNumber );
                        psPort->dwBytes = 0;
                    }
                }
                else
                {
                    SERDebug( "Malformed packet receievd (too small)" );
                    psPort->dwBytes = 0;
                }
            }

            if ( ( psPort->dwBytes == 0 ) && ( ! psPort->fPending ) )
            {
                SERDebug( "Rearming receive" );
                //
                // Initialiser la structure overlapped pour la lecture
                //
                ZeroMemory( &psPort->sOverRead, sizeof( psPort->sOverRead ) );
                ResetEvent( psPort->sOverRead.hEvent = psPort->hEventRead );
                psPort->inBufferHeader.len = sizeof(psPort->inBuffer);
                psPort->inBufferHeader.buf = psPort->inBuffer;
                psPort->dwInBufferBytes = 0;
                dwFlags = 0;
                psPort->dwInAddrInBytes = sizeof(psPort->inAddrIn);
                if ( WSARecvFrom(    psPort->hSock, 
                                     &psPort->inBufferHeader, 
                                     1, 
                                     &psPort->dwInBufferBytes,
                                     &dwFlags,
                                     (struct sockaddr*)&psPort->inAddrIn,
                                     (int*)&psPort->dwInAddrInBytes,
                                     &psPort->sOverRead,
                                     NULL ) == SOCKET_ERROR )
                {
                    dwErr = WSAGetLastError();
                    dwErr = ( dwErr == WSA_IO_PENDING  ? NO_ERROR : dwErr );
                    if ( dwErr != NO_ERROR )
                    {
                        SERDebug( "Receive error %u", dwErr );
                        __leave;
                    }

                    psPort->fPending = TRUE;
                    break;
                }
    
                //
                // Si ReadFile retourne OK, l'événement est lié à la structure
                // overlapped est passé à l'état signalé, on conserve donc
                // l'état pending qui va permettre de faire un appel à GetOverlappedResult().
                //
                psPort->fPending = TRUE;
            }
        }

        dwErr = NO_ERROR;

    }
    __finally
    {
        SERDebug( "SERUdpRead returned %u bytes", dwBytes );
        //
        // Mettre à jour l'état du buffer
        //
        (*pdwBufferBytes) = dwBytes;
    }

    return dwErr;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpEmptyBuffer(
 *                      IN SER_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Vider le buffer du port comm
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpEmptyBuffer(
        IN SERUDP_INSTANCE      * psPort )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur courant
    DWORD       dwCount;            // Nombre de caractères à lire
    static BYTE tcChar[sizeof(psPort->inBuffer) + 1];         // Caractères lus

    do
    {
        //
        // Eliminer les caractères immédiatement disponibles
        //
        dwCount = sizeof(tcChar);
        dwErr = SERUdpRead( psPort, tcChar, &dwCount );
        if ( dwErr != NO_ERROR )
            break;
    }
    while ( dwCount == sizeof(tcChar) );
    
    return dwErr;
}





/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpGetInputBufferStatus( 
 *                      IN              SERUDP_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SERUdpOpen().
 * RETOURNE  : Nombre d'octets directement disponible dans le buffer
 *             de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état du buffer de reception.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpGetInputBufferStatus( 
        IN              SERUDP_INSTANCE      * psPort )
{
    return psPort->dwBytes;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERUdpGetLineStatus( 
 *                      IN              SER_INSTANCE      * psPort,
 *                      OPTIONAL OUT    BOOL              * pfCTS,
 *                      OPTIONAL OUT    BOOL              * pfDSR,
 *                      OPTIONAL OUT    BOOL              * pfRing,
 *                      OPTIONAL OUT    BOOL              * pfRLSD )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pfCTS         : Récupère l'état de la ligne CTS  (TRUE=ON / FALSE=OFF)
 *             pfDSR         : Récupère l'état de la ligne DSR  (TRUE=ON / FALSE=OFF)
 *             pfRing        : Récupère l'état de la ligne RING (TRUE=ON / FALSE=OFF)
 *             pfRLSD        : Récupère l'état de la ligne RLSD (TRUE=ON / FALSE=OFF)
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état des lignes d'entrée d'un port série :
 *              CTS  : Clear To Send
 *              DSR  : Data Set Ready
 *              RING : Indicateur de sonnerie
 *              RLSD : Receive Line Signal Detect
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERUdpGetLineStatus( 
        IN              SERUDP_INSTANCE      * psPort,
        OPTIONAL OUT    BOOL              * pfCTS,
        OPTIONAL OUT    BOOL              * pfDSR,
        OPTIONAL OUT    BOOL              * pfRing,
        OPTIONAL OUT    BOOL              * pfRLSD )
{
    if ( pfCTS != NULL )
        *pfCTS = 1;
    if ( pfDSR != NULL )
        *pfDSR = 1;
    if ( pfRing != NULL )
        *pfRing = 0;
    if ( pfRLSD != NULL )
        *pfRLSD = 0;
    return NO_ERROR;
}





DWORD SERUdpInitSocket()
{
	static LONG glInitCount = 0;

	LONG * plInitCount = (LONG*)(&glInitCount);

    DWORD dwErr = ERROR_INVALID_DATA;
    

    while ( TRUE )
    {
		if (InterlockedCompareExchange((LONG*)plInitCount, (LONG)-1, (DWORD)0) == (DWORD)0)
        {
            WSADATA        wsaData;
            int iStartup = WSAStartup(MAKEWORD(2,2), &wsaData);
            if ( iStartup != 0 )
            {
				InterlockedCompareExchange((LONG*)plInitCount, (LONG)0, (LONG)-1);
                break;
            }

			InterlockedCompareExchange((LONG*)plInitCount, (LONG)1, (LONG)-1);

            dwErr = NO_ERROR;
            break;
        }

		if (InterlockedCompareExchange((LONG*)plInitCount, (LONG)1, (LONG)1) == (LONG)1)
        {
            dwErr = NO_ERROR;
            break;
        }

        Sleep(1);
    }

    return dwErr;
}


static void SERDebug( const char * szFormat, ... )
{
    char text[1000];
    va_list args;
    int i;
    va_start( args, szFormat );
    i = _vsnprintf( text, sizeof(text)-3, szFormat, args );
    if ( i < 0 )
        i = sizeof(text)-3;
    strcpy( text + i, "\n" );
    OutputDebugString( text );
}

