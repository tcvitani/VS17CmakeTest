// acom_unittest.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <conio.h>


#include "..\\..\\includes\\acom.h"

#define MAIN_PIPESERVER 0
#define MAIN_PIPECLIENT 1
#define MAIN_MAILSERVER 2
#define MAIN_MAILCLIENT 3
#define MAIN_TCPSERVER  4
#define MAIN_TCPCLIENT  5
#define MAIN_SERIAL     6


#define MAIM_MAXINST 64
#define MAIN_MAX_CNX 100

#define MAX_PACKET_BYTES 1500


ACOM_WKS_HANDLE ghWks = NULL;
ACOM_INST_HANDLE gthInst[MAIM_MAXINST];

// Working set specific
DWORD gdwMaxWksCnx = 64;
DWORD gdwPriority = THREAD_PRIORITY_NORMAL;
DWORD gdwProtocol = ACOM_PROTOCOL_LIFE;
DWORD gdwLifeTime = 1000;
DWORD gdwMaxLife = 1;
DWORD gdwConnectLoopDelay = 1000;
DWORD gdwWorkers = 2;
DWORD gdwNoGetPeerName = ACOM_FLAG_GET_PEER_NAME;
DWORD gdwLifeTimeFactor = ACOM_FLAG_FIXED_LIFETIME;

// Instance specific
DWORD gdwInstances = 0;
DWORD64 gtdwType[MAIM_MAXINST];
char  gtszFileName[MAIM_MAXINST][MAX_PATH];
char  gtszTo[MAIM_MAXINST][MAX_PATH];
DWORD gtdwTimeToReconnect[MAIM_MAXINST];
DWORD gtdwMaxInstCnx[MAIM_MAXINST];
DWORD gtdwOutBufferSize[MAIM_MAXINST];
DWORD gtdwInBufferSize[MAIM_MAXINST];
DWORD gtdwQueueSize[MAIM_MAXINST];
DWORD gtdwMaxMessageSize[MAIM_MAXINST];
DWORD gtdwConnectLoopDelay[MAIM_MAXINST];
WORD gtwPort[MAIM_MAXINST];
DWORD gtdwIntervalRead[MAIM_MAXINST];
DWORD gtdwRecvBurstCount[MAIM_MAXINST];
char gtszRecvLast[MAIM_MAXINST][MAX_PACKET_BYTES+1];

CRITICAL_SECTION gsCritical;
DWORD gtbConnected[MAIN_MAX_CNX] = {0};
ACOM_CNX_HANDLE gthHandles[MAIN_MAX_CNX];
DWORD gdwBurst = 1;
DWORD gdwBurstInterval = 10;
DWORD gdwTraces = 0;
DWORD gdwWL = 10;


DWORD gdwEcho = TRUE;
BOOL gbStop = FALSE;

void UPDOverlappedIoTest();


void CALLBACK MainCallbackShut(DWORD64 pWksUsrKey, DWORD dwError)
{
    if ( gdwEcho ) printf(
            "CALLBACK : SHUTDOWN(dwError=%d)\n", 
            dwError );
    gbStop = TRUE;
}

DWORD64 CALLBACK MainCallbackCnx(DWORD64 pInstUsrKey, ACOM_CNX_HANDLE hCnxHandle)
{
/*  
    DWORD dwErr;
    DWORD dwBurst;
*/
    DWORD dwVal = 1;
    DWORD dwIndex;
    char szName[256] = "";

    if ( AComGetPeer( ghWks, hCnxHandle, szName, sizeof(szName) ) != NO_ERROR )
		strcpy_s(szName, sizeof(szName), "#ERR#");

    if ( gdwEcho ) printf( 
            "CALLBACK : CONNECTION(pInstUsrKey=%I64d,hCnxHandle=%d) : %d / Id = [%s]\n", 
            pInstUsrKey, 
            hCnxHandle, 
            hCnxHandle,
            szName );

    EnterCriticalSection( &gsCritical );
    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
        if ( ! gtbConnected[dwIndex] )
        {
            gtbConnected[dwIndex] = TRUE;
            gthHandles[dwIndex] = hCnxHandle;
            break;
        }
    LeaveCriticalSection( &gsCritical );
/*
    if ( ( pInstUsrKey == MAIN_PIPECLIENT ) ||
         ( pInstUsrKey == MAIN_MAILCLIENT ) ||
         ( pInstUsrKey == MAIN_TCPCLIENT  ) )
    {
        for ( dwBurst = 0 ; dwBurst < gdwBurst ; dwBurst ++ )
        {
            dwErr = AComSendMessage( ghWks, hCnxHandle, dwVal, sizeof(dwVal), &dwVal );
            if ( dwErr != NO_ERROR )
            {
                printf( "ERREUR %d\n", dwErr );
                break;
            }
        }
    }
*/

    gtdwRecvBurstCount[dwIndex] = 0;
    gtszRecvLast[dwIndex][0] = 0;

    printf( "CONNEXION : %d\n", dwIndex );
	return (DWORD64)dwIndex;
}

void CALLBACK MainCallbackRecv(DWORD64 pInstUsrKey,
								DWORD64 pCnxUsrKey,
                                DWORD dwDataSize,
                                void * pvData )
{
//    DWORD dwErr;
    DWORD dwVal = *((DWORD*)pvData);
    ACOM_CNX_HANDLE hCnxHandle = gthHandles[(DWORD)pCnxUsrKey];
    DWORD dwIndex;
    BYTE bCur;
    char szRecv[MAX_PACKET_BYTES+1];
    char szFrom[32];

    szFrom[0] = 0;

    if ( gdwEcho ) printf( 
            "CALLBACK : RECEIVED(pInstUsrKey=%I64d,pCnxUsrKey=%I64d,dwDataSize=%d) %s\n",
            pInstUsrKey, 
            pCnxUsrKey, 
            dwDataSize,
            szFrom );
    
/*    if ( ( pInstUsrKey == MAIN_PIPESERVER ) || 
         ( pInstUsrKey == MAIN_PIPECLIENT ) ||
         ( pInstUsrKey == MAIN_TCPSERVER  ) ||
         ( pInstUsrKey == MAIN_TCPCLIENT  ) ||
         ( pInstUsrKey == MAIN_SERIAL     ) )
    {
        dwVal ++;
        dwErr = AComSendMessage( ghWks, hCnxHandle, dwVal, sizeof(dwVal), &dwVal );
        if ( dwErr != NO_ERROR )
            printf( "ERREUR %d\n", dwErr );
        Sleep(10);
    }
    */

    for ( dwIndex = 0 ; dwIndex < dwDataSize ; dwIndex ++ )
    {
        bCur = ((BYTE*)pvData)[dwIndex];
        if ( bCur < 32 ) 
            bCur = '.';
        szRecv[dwIndex] = bCur;
    }
    szRecv[dwIndex] = 0;
    if ( strcmp( gtszRecvLast[(DWORD)pCnxUsrKey], szRecv ) != 0 )
    {
        strcpy( gtszRecvLast[(DWORD)pCnxUsrKey], szRecv );
        gtdwRecvBurstCount[(DWORD)pCnxUsrKey] = 1;
    }
    else
        gtdwRecvBurstCount[(DWORD)pCnxUsrKey] += 1;

    printf( "RECEPTION : %I64d [%s] Burst item %d\n", pCnxUsrKey, szRecv, gtdwRecvBurstCount[(DWORD)pCnxUsrKey] );

    if ( gdwWL != 0 )
        Sleep( gdwWL );
}

void CALLBACK MainCallbackSent(DWORD64 pInstUsrKey,
								DWORD64 pCnxUsrKey,
								DWORD64 pMsgUsrKey,
                                DWORD dwError )
{
    DWORD dwErr;
	ACOM_CNX_HANDLE hCnxHandle = gthHandles[pCnxUsrKey];

    if ( gdwEcho ) printf( 
            "CALLBACK : SENT(pInstUsrKey=%I64d,pCnxUsrKey=%I64d,pMsgUsrKey=%I64d,dwError=%d)\n",
            pInstUsrKey, 
            pCnxUsrKey, 
            pMsgUsrKey,
            dwError );
    if ( dwError != NO_ERROR )
        printf( "NACK %I64d\n", pMsgUsrKey );
    else
    {
        if ( (DWORD)pInstUsrKey == MAIN_MAILCLIENT )
        {
			pMsgUsrKey = pMsgUsrKey + 1;
			dwErr = AComSendMessage(ghWks, hCnxHandle, pMsgUsrKey, sizeof(pMsgUsrKey), &pMsgUsrKey);
            if ( dwErr != NO_ERROR )
                printf( "ERREUR %d\n", dwErr );
        }
    }
}


void CALLBACK MainCallbackDcnx(DWORD64 pInstUsrKey,
								DWORD64 pCnxUsrKey)

{
    ACOM_CNX_HANDLE hCnxHandle = gthHandles[pCnxUsrKey];
    if ( gdwEcho ) printf( 
            "CALLBACK : DISCONNECTION(pInstUsrKey=%I64d,hCnxHandle=%ld)\n", 
            pInstUsrKey, 
            hCnxHandle );
    EnterCriticalSection( &gsCritical );
    gtbConnected[pCnxUsrKey] = FALSE;
    LeaveCriticalSection( &gsCritical );
}


void MainSendData()
{
    DWORD dwIndex;
    DWORD dwErr;
    DWORD dwBurst;
    char * pcMsg;
    static DWORD dwMsgIndex = 0;
    static char ttcMsg[10][MAX_PACKET_BYTES] = { 
        "ABCD", 
        "12345678", 
        "XYZ", 
        "Bonjour", 
        "Au revoir", 
        "Salut", 
        "OK",
        "A plus",
        "Coucou",
        "Bye" };

	dwMsgIndex++;
	pcMsg = ttcMsg[dwMsgIndex % 10];


    EnterCriticalSection( &gsCritical );
    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
    {
        if ( gtbConnected[dwIndex] )
        {
            printf( "FORCESEND %u x %s with %u ms intervals\n", gdwBurst, pcMsg, gdwBurstInterval );

            for ( dwBurst = 0 ; dwBurst < gdwBurst ; dwBurst ++ )
            {
                if ( ( dwBurst != 0 ) && ( gdwBurstInterval != 0 ) )
                    Sleep( gdwBurstInterval );

                dwErr = AComSendMessage( ghWks, gthHandles[dwIndex], (DWORD64)(strlen(pcMsg)), (DWORD)strlen(pcMsg), pcMsg );

                if ( dwErr != NO_ERROR )
                    printf( "ERREUR %d\n", dwErr );
            }
        }
    }
    LeaveCriticalSection( &gsCritical );
}

void MainDisconnect()
{
    DWORD dwIndex;
	DWORD dwErr = NO_ERROR;

    EnterCriticalSection( &gsCritical );
    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
    {
        if ( gtbConnected[dwIndex] )
        {
            dwErr = AComDisconnectPeer( ghWks, gthHandles[dwIndex], FALSE );
            if ( dwErr != NO_ERROR )
                printf( "ERREUR %d\n", dwErr );
            else if ( gdwEcho ) 
                printf( "FORCEDISCONNECT\n" );
        }
    }
    LeaveCriticalSection( &gsCritical );
}


void MainStatistics()
{
    DWORD dwIndex;
	DWORD dwErr = NO_ERROR;
    DWORD dwMaxThreads;
    ULONGLONG ullDelai;
    ULONGLONG ullNow;
    ULONGLONG ullNowCnx;
    ULONGLONG ullCnxDate;
    ULONGLONG ullInBytes;
    ULONGLONG ullOutBytes;
    ULONGLONG ullInMsg;
    ULONGLONG ullOutMsg;
    ULONGLONG ullIdle;
    ULONGLONG ullUser;
    ULONGLONG ullKernel;
    ULONGLONG ullSumInBytes = 0;
    ULONGLONG ullSumOutBytes = 0;
    ULONGLONG ullSumInMsg = 0;
    ULONGLONG ullSumOutMsg = 0;
    static ULONGLONG ullLastSumInBytes = 0;
    static ULONGLONG ullLastSumOutBytes = 0;
    static ULONGLONG ullLastSumInMsg = 0;
    static ULONGLONG ullLastSumOutMsg = 0;
    static ULONGLONG ullLastNow = 0;

    GetSystemTimeAsFileTime( (LPFILETIME)&ullNow );
    dwErr = AComGetWorkersStatistics(
        ghWks,
        &dwMaxThreads,
        &ullIdle,
        &ullUser,
        &ullKernel );
    if ( dwErr == NO_ERROR )
    {
        printf( "WORKING SET :\n" );
        printf( "  Max concurent threads : %d\n", dwMaxThreads );
        printf( "  Idle                  : %I64d ms / %I64d x100ns\n", ullIdle / (ULONGLONG)10000, ullIdle );
        printf( "  User                  : %I64d ms / %I64d x100ns\n", ullUser / (ULONGLONG)10000, ullUser );
        printf( "  Kernel                : %I64d ms / %I64d x100ns\n", ullKernel / (ULONGLONG)10000, ullKernel );
    }
    else
    {
        printf( "WORKING SET : ERREUR STATISTIQUES\n" );
    }
        

    EnterCriticalSection( &gsCritical );

    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
        if ( gtbConnected[dwIndex] )
        {
            dwErr = AComGetConnectionStatistics( 
                ghWks,
                gthHandles[dwIndex],
                &ullNowCnx,
                &ullCnxDate,
                &ullInBytes,
                &ullOutBytes,
                &ullInMsg,
                &ullOutMsg );
            if ( dwErr == NO_ERROR )
            {
                ullDelai = ullNowCnx - ullCnxDate;
                printf( "\n" );
                printf( "  CONNEXION %d (hCnxHandle=%d) :\n", dwIndex, gthHandles[dwIndex] );
                printf( "    Durée connexion          : %I64d ms\n", ullDelai / (ULONGLONG)10000);
                printf( "    In = %I64d bytes / %I64d msgs   -   Out = %I64d bytes / %I64d msgs\n", ullInBytes, ullInMsg, ullOutBytes, ullOutMsg );
                ullSumInBytes += ullInBytes;
                ullSumOutBytes += ullOutBytes;
                ullSumInMsg += ullInMsg;
                ullSumOutMsg += ullOutMsg;
            }
            else
            {
                printf( "\n" );
                printf( "  CONNEXION %d (hCnxHandle=%d) ERREUR STATISTIQUES\n", dwIndex, gthHandles[dwIndex] );
            }
        }
    LeaveCriticalSection( &gsCritical );
    printf( "\n" );
    printf( 
        "  SumIn = %I64d bytes / %I64d msgs   -   SumOut = %I64d bytes / %I64d msgs\n", 
        ullSumInBytes, 
        ullSumInMsg, 
        ullSumOutBytes, 
        ullSumOutMsg );
    ullDelai = ullNow - ullLastNow;
    printf( "  Dernière période : %I64d ms\n", ullDelai / (ULONGLONG)10000 );
    printf( 
        "  PerIn = %I64d bytes / %I64d msgs   -   PerOut = %I64d bytes / %I64d msgs\n", 
        ullSumInBytes - ullLastSumInBytes, 
        ullSumInMsg - ullLastSumInMsg, 
        ullSumOutBytes - ullLastSumOutBytes, 
        ullSumOutMsg - ullLastSumOutMsg );
    printf( 
        "  RateIn = %I64d bytes/s / %I64d msgs/s   -   RateOut = %I64d bytes/s / %I64d msgs/s\n", 
        ( ( ullSumInBytes - ullLastSumInBytes ) * (ULONGLONG)10000000 ) / ullDelai, 
        ( ( ullSumInMsg - ullLastSumInMsg ) * (ULONGLONG)10000000 ) / ullDelai, 
        ( ( ullSumOutBytes - ullLastSumOutBytes ) * (ULONGLONG)10000000 ) / ullDelai, 
        ( ( ullSumOutMsg - ullLastSumOutMsg ) * (ULONGLONG)10000000 ) / ullDelai );
    ullLastSumInBytes = ullSumInBytes;
    ullLastSumOutBytes = ullSumOutBytes;
    ullLastSumInMsg = ullSumInMsg;
    ullLastSumOutMsg = ullSumOutMsg;
    ullLastNow = ullNow;
}


char * MainParamNextItem( char * pcParam, char * pcItem )
{
    char cCurrent = (*pcParam);
    while ( ( cCurrent != ',' ) && ( cCurrent != '\0' ) )
    {
        (*pcItem) = cCurrent;
        pcItem++;
        pcParam++;
        cCurrent = (*pcParam);
    }
    (*pcItem) = '\0';
    if ( cCurrent == ',' )
        return pcParam + 1;
    else
        return pcParam;
}
 
BOOL MainParamList( char * pcParam, char * pcDef, char * pcTypes, ... )
{
    char cType;
    DWORD dwLen = (DWORD)strlen(pcDef);
    void * pvVal;
    char * pcNext;
    va_list sList;
    char szItem[MAX_PATH];

    if ( _strnicmp( pcDef, pcParam, dwLen ) != 0 )
        return FALSE;

    pcParam += dwLen;

    va_start( sList, pcTypes );

    while ( ( cType = (*pcTypes) ) != '\0' )
    {
        pvVal = va_arg( sList, void * );
        if ( cType == 'I' )
        {
            pcNext = MainParamNextItem( pcParam, szItem );
            if ( pcNext == NULL )
                return FALSE;
            *((DWORD*)pvVal) = atol( szItem );
            pcParam = pcNext;
        }
        else if ( cType == 'S' )
        {
            pcNext = MainParamNextItem( pcParam, szItem );
            if ( pcNext == NULL )
                return FALSE;
			strcpy_s(pvVal, sizeof(szItem), szItem);
            pcParam = pcNext;
        }
        else 
            return FALSE;
        pcTypes ++;
    }
    if ( (*pcParam) != '\0' )
        return FALSE;
    return TRUE;
}

            
BOOL MainParamString( char * pcParam, char * pcDef, char * pcVal, DWORD dwSize )
{
    DWORD dwLen = (DWORD)strlen(pcDef);

    if ( _strnicmp( pcDef, pcParam, dwLen ) != 0 )
        return FALSE;
	strncpy_s(pcVal, sizeof(*pcVal), pcParam + dwLen, dwSize);
    pcVal[dwSize-1] = '\0';

    return TRUE;
}

BOOL MainParamDword( char * pcParam, char * pcDef, DWORD * pdwVal )
{
    DWORD dwLen = (DWORD)strlen(pcDef);
    char szVal[100];

    if ( _strnicmp( pcDef, pcParam, dwLen ) != 0 )
        return FALSE;
	strncpy_s(szVal, sizeof(szVal), pcParam + dwLen, sizeof(szVal));
    szVal[sizeof(szVal)-1] = '\0';
    *pdwVal = atol( szVal );

    return TRUE;
}

BOOL MainParamEnum( char * pcParam, char * pcDef, DWORD * pdwVal, DWORD dwCount, ... )
{
    DWORD dwLen = (DWORD)strlen(pcDef);
    char szVal[100];
    char * pcVal;
    DWORD dwVal;
    va_list sList;

    if ( _strnicmp( pcDef, pcParam, dwLen ) != 0 )
        return FALSE;
	strncpy_s(szVal, sizeof(szVal), pcParam + dwLen, sizeof(szVal));
    szVal[sizeof(szVal)-1] = '\0';
    va_start( sList, dwCount );

    while ( dwCount > 0 )
    {
        pcVal = va_arg( sList, char * );
        dwVal = va_arg( sList, DWORD );

        if ( _stricmp( szVal, pcVal ) == 0 )
        {
            *pdwVal = dwVal;
            return TRUE;
        }
        dwCount --;
    }

    return FALSE;

}

void MainHelp()
{
    printf( "Paramètres attendus :\n" );
}


int __cdecl main( int iArgc, char * * ppcArgv )
{
    BOOL bParam;
    DWORD dwIndex;
    DWORD dwKeyPressed;
    char * pcParam;
	BOOL bTraceInitialized = FALSE;


    // Valeurs par défaut

    for ( dwIndex = 1 ; dwIndex < (DWORD)iArgc ; dwIndex ++ )
    {
        bParam = FALSE;
        pcParam = ppcArgv[dwIndex];

        if ( strcmp( pcParam, "/?" ) == 0 )
        {
            MainHelp();
            return 0;
        }

        
        if ( MainParamList(
                pcParam, "/PS=", "SIIIIII", 
                gtszFileName[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwMaxInstCnx[gdwInstances],
                &gtdwOutBufferSize[gdwInstances],
                &gtdwInBufferSize[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_PIPESERVER;
            gdwInstances ++;
            bParam = TRUE;

			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_PS.trc"); bTraceInitialized = TRUE;
			}

            continue;
        }
        
        if ( MainParamList(
                pcParam, "/PC=", "SIII", 
                gtszFileName[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_PIPECLIENT;
            gdwInstances ++;
            bParam = TRUE;
			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_PC.trc"); bTraceInitialized = TRUE;
			}

            continue;
        }

        if ( MainParamList(
                pcParam, "/MS=", "SII", 
                gtszFileName[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_MAILSERVER;
            gdwInstances ++;
            bParam = TRUE;
			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_MS.trc"); bTraceInitialized = TRUE;
			}
            continue;
        }

        if ( MainParamList(
                pcParam, "/MC=", "SIII", 
                gtszFileName[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_MAILCLIENT;
            gdwInstances ++;
            bParam = TRUE;
			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_MC.trc"); bTraceInitialized = TRUE;
			}
            continue;
        }

        if ( MainParamList(
                pcParam, "/TS=", "SIIIIIII", 
                gtszFileName[gdwInstances],
                &gtwPort[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwMaxInstCnx[gdwInstances],
                &gtdwOutBufferSize[gdwInstances],
                &gtdwInBufferSize[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_TCPSERVER;
            gdwInstances ++;
            bParam = TRUE;
			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_TS.trc"); bTraceInitialized = TRUE;
			}

            continue;
        }
        
        if ( MainParamList(
                pcParam, "/TC=", "SIIIIII", 
                gtszFileName[gdwInstances],
                &gtwPort[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwOutBufferSize[gdwInstances],
                &gtdwInBufferSize[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_TCPCLIENT;
            gdwInstances ++;
            bParam = TRUE;
			if (!bTraceInitialized)
			{
				AComOpenTrace(TRUE, TRUE, "c:\\csr\\traces\\AcomTest_TC.trc"); bTraceInitialized = TRUE;
			}
            continue;
        }

        if ( MainParamList(
                pcParam, "/S=", "ISIIIIII",
                &gtwPort[gdwInstances],
                gtszFileName[gdwInstances],
                &gtdwTimeToReconnect[gdwInstances],
                &gtdwOutBufferSize[gdwInstances],
                &gtdwInBufferSize[gdwInstances],
                &gtdwQueueSize[gdwInstances],
                &gtdwMaxMessageSize[gdwInstances],
                &gtdwIntervalRead[gdwInstances] ) )
        {
            gtdwType[gdwInstances] = MAIN_SERIAL;
            gdwInstances ++;
            bParam = TRUE;
            continue;
        }

         bParam |= MainParamEnum(
            pcParam, "/ECHO=", &gdwEcho, 2,
            "ON", TRUE,
            "OFF", FALSE );

        bParam |= MainParamDword( pcParam, "/BURST=", &gdwBurst );
        bParam |= MainParamDword( pcParam, "/BURSTINT=", &gdwBurstInterval );
        bParam |= MainParamDword( pcParam, "/TRACES=", &gdwTraces );
        bParam |= MainParamDword( pcParam, "/WL=", &gdwWL );
        bParam |= MainParamDword( pcParam, "/MAXWKSCNX=", &gdwMaxWksCnx );
        bParam |= MainParamDword( pcParam, "/WORKERS=", &gdwWorkers );
        bParam |= MainParamDword( pcParam, "/CONNECTLOOP=", &gdwConnectLoopDelay );
        bParam |= MainParamDword( pcParam, "/LIFETIME=", &gdwLifeTime );
        bParam |= MainParamDword( pcParam, "/MAXLIFE=", &gdwMaxLife );
        bParam |= MainParamEnum(
            pcParam, "/PRIO=", &gdwPriority, 7,
            "ABOVE",    THREAD_PRIORITY_ABOVE_NORMAL,
            "BELOW",    THREAD_PRIORITY_BELOW_NORMAL,
            "HIGHEST",  THREAD_PRIORITY_HIGHEST,
            "IDLE",     THREAD_PRIORITY_IDLE,
            "LOWEST",   THREAD_PRIORITY_LOWEST,
            "NORMAL",   THREAD_PRIORITY_NORMAL,
            "CRITICAL", THREAD_PRIORITY_TIME_CRITICAL );
        bParam |= MainParamEnum(
            pcParam, "/PROTOCOL=", &gdwProtocol, 2,
            "DEFAULT",  ACOM_PROTOCOL_DEFAULT,
            "LIFE",      ACOM_PROTOCOL_LIFE );
        bParam |= MainParamEnum(
            pcParam, "/GETPEER=", &gdwNoGetPeerName, 2,
            "NO" ,  ACOM_FLAG_NO_GET_PEER_NAME,
            "YES",  ACOM_FLAG_GET_PEER_NAME );
        bParam |= MainParamDword( pcParam, "/LTFACTOR=", &gdwLifeTimeFactor );

        if ( bParam ) continue;

        printf( "ERREUR : Paramètre inattendu %s\n", pcParam );
        MainHelp();

        return -1;
    }

    if ( gdwInstances == 0 )
    {
        printf( "ERREUR : Aucune instance définie\n" );
        return -1;
    }

    InitializeCriticalSection( &gsCritical );

    if ( gdwTraces != 0 )
        AComOpenTrace( TRUE, TRUE, NULL );

    printf( "\nInitialisation du working set ... " );
    ghWks = AComOpenWorkingSet(
        (DWORD64)0,
        gdwMaxWksCnx,
        gdwInstances,
        gdwPriority,
        gdwConnectLoopDelay,
        gdwWorkers,
        gdwProtocol | gdwNoGetPeerName | ACOM_FLAG_LIFETIME_FACTOR(gdwLifeTimeFactor),
        gdwLifeTime,
        gdwMaxLife,
        0,
        MainCallbackShut );
    if ( ghWks == NULL )
    {
        printf( "ERREUR\n" );
        return 0;
    }
    printf( "OK\n" );

    printf( "\nInitialisation des instances (%d) :\n", gdwInstances );
    for ( dwIndex = 0 ; dwIndex < gdwInstances ; dwIndex ++ )
    {
        if ( gtdwType[dwIndex] == MAIN_PIPESERVER )
        {
            printf( "\tPIPE SERVER : %s ... ", gtszFileName[dwIndex] );
            gthInst[dwIndex] = AComOpenPipeServerInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwMaxInstCnx[dwIndex],
                gtdwOutBufferSize[dwIndex],
                gtdwInBufferSize[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv,
                MainCallbackSent,
                NULL ) ;
        }
        else if ( gtdwType[dwIndex] == MAIN_PIPECLIENT )
        {
            printf( "\tPIPE CLIENT : %s ... ", gtszFileName[dwIndex] );
            gthInst[dwIndex] = AComOpenPipeClientInstance(
                ghWks,
                gtszFileName[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv,
                MainCallbackSent,
                NULL );
        }
        else if ( gtdwType[dwIndex] == MAIN_MAILSERVER )
        {
            printf( "\tMAIL SERVER : %s ... ", gtszFileName[dwIndex] );
            gthInst[dwIndex] = AComOpenMailslotServerInstance(
                ghWks,
                gtszFileName[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv );
        }
        else if ( gtdwType[dwIndex] == MAIN_MAILCLIENT )
        {
            printf( "\tMAIL CLIENT : %s ... \n", gtszFileName[dwIndex] );
            gthInst[dwIndex] = AComOpenMailslotClientInstance(
                ghWks,
                gtszFileName[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackSent );
        }
        else if ( gtdwType[dwIndex] == MAIN_TCPSERVER )
        {
            printf( "\tTCP SERVER : Port %u ... ", (DWORD)gtwPort[dwIndex] );

            gthInst[dwIndex] = AComOpenTcpServerInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtwPort[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwMaxInstCnx[dwIndex],
                gtdwOutBufferSize[dwIndex],
                gtdwInBufferSize[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv,
                MainCallbackSent,
                NULL ) ;
        }
        else if ( gtdwType[dwIndex] == MAIN_TCPCLIENT )
        {
            printf( "\tTCP CLIENT : %s / Port %u... ", gtszFileName[dwIndex], (DWORD)gtwPort[dwIndex] );
            gthInst[dwIndex] = AComOpenTcpClientInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtwPort[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwOutBufferSize[dwIndex],
                gtdwInBufferSize[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv,
                MainCallbackSent,
                NULL );
        }
        else if ( gtdwType[dwIndex] == MAIN_SERIAL )
        {
            printf( "\tSERIAL : Port %u... ", (DWORD)gtwPort[dwIndex] );
            gthInst[dwIndex] = AComOpenSerialPortInstance(
                ghWks,
                gtwPort[dwIndex],
                gtszFileName[dwIndex],
				(DWORD64)gtdwType[dwIndex] /* pInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwOutBufferSize[dwIndex],
                gtdwInBufferSize[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                gtdwIntervalRead[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv,
                MainCallbackSent,
                NULL );
        }


        if ( gthInst[dwIndex] == NULL )
        {
            printf( "ERREUR\n" );
            AComCloseWorkingSet( ghWks, 10000 );
            return 0;
        }
        printf( "OK\n" );
    }


    dwIndex = 0;
    while( ! gbStop )
    {
        Sleep( 10 ) ;

        if ( _kbhit() )
        {
            dwKeyPressed = toupper( _getch() );

            if ( dwKeyPressed == 'S' )
                MainStatistics();
            else if ( dwKeyPressed == ' ' )
                MainSendData();
            else if ( dwKeyPressed == 'D' )
                MainDisconnect();
            else if ( dwKeyPressed == 27  )
                break;
        }
    }

    printf( "\nFermeture des instances (%d) :\n", gdwInstances );
    for ( dwIndex = 0 ; dwIndex < gdwInstances ; dwIndex ++ )
    {
        printf( "\t%s ... \n", gtszFileName[dwIndex] );
        AComCloseInstance( gthInst[dwIndex] );
        printf( "OK\n" );
    }

    printf( "\nFermeture du working set ..." );
    AComCloseWorkingSet( ghWks, 10000 );
    printf( "OK\n" );

    return 0;
}


