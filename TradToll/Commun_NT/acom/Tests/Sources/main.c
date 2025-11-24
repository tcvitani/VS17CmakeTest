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
DWORD gtdwTimeToReconnect[MAIM_MAXINST];
DWORD gtdwMaxInstCnx[MAIM_MAXINST];
DWORD gtdwOutBufferSize[MAIM_MAXINST];
DWORD gtdwInBufferSize[MAIM_MAXINST];
DWORD gtdwQueueSize[MAIM_MAXINST];
DWORD gtdwMaxMessageSize[MAIM_MAXINST];
DWORD gtdwConnectLoopDelay[MAIM_MAXINST];
WORD gtwPort[MAIM_MAXINST];
DWORD gtdwIntervalRead[MAIM_MAXINST];

CRITICAL_SECTION gsCritical;
DWORD gtbConnected[MAIN_MAX_CNX] = {0};
DWORD gtdwHandles[MAIN_MAX_CNX];
DWORD gdwBurst = 1;


BOOL gbEcho = TRUE;
BOOL gbStop = FALSE;


void CALLBACK MainCallbackShut(DWORD64 dwWksUsrKey, DWORD dwError)
{
    if ( gbEcho ) printf(
            "CALLBACK : SHUTDOWN(dwError=%d)\n", 
            dwError );
    gbStop = TRUE;
}

DWORD64 CALLBACK MainCallbackCnx(DWORD64 dwInstUsrKey, ACOM_CNX_HANDLE hCnxHandle)
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

    if ( gbEcho ) printf( 
            "CALLBACK : CONNECTION(dwInstUsrKey=%I64d,hCnxHandle=%ld) : %ld / Id = [%s]\n", 
            dwInstUsrKey, 
            hCnxHandle, 
            hCnxHandle,
            szName );

    EnterCriticalSection( &gsCritical );
    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
        if ( ! gtbConnected[dwIndex] )
        {
            gtbConnected[dwIndex] = TRUE;
            gtdwHandles[dwIndex] = hCnxHandle;
            break;
        }
    LeaveCriticalSection( &gsCritical );
/*
    if ( ( dwInstUsrKey == MAIN_PIPECLIENT ) ||
         ( dwInstUsrKey == MAIN_MAILCLIENT ) ||
         ( dwInstUsrKey == MAIN_TCPCLIENT  ) )
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
    printf( "CONNEXION : %d\n", dwIndex );
    return dwIndex;
}

void CALLBACK MainCallbackRecv(DWORD64 dwInstUsrKey,
                                DWORD64 dwCnxUsrKey,
                                DWORD dwDataSize,
                                void * pvData )
{
/*
    DWORD dwErr;
*/
    DWORD dwVal = *((DWORD*)pvData);
    ACOM_CNX_HANDLE hCnxHandle = gtdwHandles[dwCnxUsrKey];
    DWORD dwIndex;
    BYTE bCur;
    char szRecv[1000];

    if ( gbEcho ) printf( 
            "CALLBACK : RECEIVED(dwInstUsrKey=%I64d,dwCnxUsrKey=%I64d,dwDataSize=%ld)\n",
            dwInstUsrKey, 
            dwCnxUsrKey, 
            dwDataSize );
    
/*    if ( ( dwInstUsrKey == MAIN_PIPESERVER ) || 
         ( dwInstUsrKey == MAIN_PIPECLIENT ) ||
         ( dwInstUsrKey == MAIN_TCPSERVER  ) ||
         ( dwInstUsrKey == MAIN_TCPCLIENT  ) ||
         ( dwInstUsrKey == MAIN_SERIAL     ) )
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

    printf( "RECEPTION : %I64d [%s]\n", dwCnxUsrKey, szRecv );

}

void CALLBACK MainCallbackSent(DWORD64 dwInstUsrKey,
                                DWORD64 dwCnxUsrKey,
								DWORD64 dwMsgUsrKey,
                                DWORD dwError )
{
	DWORD dwErr = NO_ERROR;
    ACOM_CNX_HANDLE hCnxHandle = gtdwHandles[dwCnxUsrKey];

    if ( gbEcho ) printf( 
            "CALLBACK : SENT(dwInstUsrKey=%I64d,dwCnxUsrKey=%I64d,dwMsgUsrKey=%I64d,dwError=%d)\n",
            dwInstUsrKey, 
            dwCnxUsrKey, 
            dwMsgUsrKey,
            dwError );
    if ( dwError != NO_ERROR )
        printf( "NACK %I64d\n", dwMsgUsrKey );
    else
    {
        if ( dwInstUsrKey == MAIN_MAILCLIENT )
        {
            dwMsgUsrKey ++;
            dwErr = AComSendMessage( ghWks, hCnxHandle, dwMsgUsrKey, sizeof(dwMsgUsrKey), &dwMsgUsrKey );
            if ( dwErr != NO_ERROR )
                printf( "ERREUR %d\n", dwErr );
        }
    }
}


void CALLBACK MainCallbackDcnx(DWORD64 dwInstUsrKey,
                                 DWORD64 dwCnxUsrKey )

{
    ACOM_CNX_HANDLE hCnxHandle = gtdwHandles[dwCnxUsrKey];
    if ( gbEcho ) printf( 
            "CALLBACK : DISCONNECTION(dwInstUsrKey=%I64d,dwCnxUsrKey=%I64d) : %ld\n",
			dwCnxUsrKey,
            dwInstUsrKey, 
            hCnxHandle );
    EnterCriticalSection( &gsCritical );
    gtbConnected[dwCnxUsrKey] = FALSE;
    LeaveCriticalSection( &gsCritical );
}


void MainSendData()
{
    DWORD dwIndex;
	DWORD dwErr = NO_ERROR;
    char * pcMsg;
    static DWORD dwMsgIndex = 0;
    static char ttcMsg[10][10] = { "ABCD", 
                                   "12345678", 
                                   "XYZ", 
                                   "Bonjour", 
                                   "Au revoir", 
                                   "Salut", 
                                   "OK",
                                   "A plus",
                                   "Coucou",
                                   "Bye" };

    EnterCriticalSection( &gsCritical );
    for ( dwIndex = 0 ; dwIndex < MAIN_MAX_CNX ; dwIndex ++ )
    {
        if ( gtbConnected[dwIndex] )
        {
            dwMsgIndex ++;
            pcMsg = ttcMsg[dwMsgIndex%10];
			dwErr = AComSendMessage(ghWks, gtdwHandles[dwIndex], 12345, (DWORD)strlen(pcMsg), pcMsg);
			if (dwErr != NO_ERROR)
				printf("ERREUR %d\n", dwErr);
            else if ( gbEcho ) 
                printf( "FORCESEND %s\n", pcMsg );
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
            dwErr = AComDisconnectPeer( ghWks, gtdwHandles[dwIndex], FALSE );
            if ( dwErr != NO_ERROR )
                printf( "ERREUR %d\n", dwErr );
            else if ( gbEcho ) 
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
                gtdwHandles[dwIndex],
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
                printf( "  CONNEXION %d (hCnxHandle=%d) :\n", dwIndex, gtdwHandles[dwIndex] );
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
                printf( "  CONNEXION %d (hCnxHandle=%d) ERREUR STATISTIQUES\n", dwIndex, gtdwHandles[dwIndex] );
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
            pcParam, "/ECHO=", &gbEcho, 2,
            "ON", TRUE,
            "OFF", FALSE );

        bParam |= MainParamDword( pcParam, "/BURST=", &gdwBurst );
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

    printf( "\nInitialisation du working set ... " );
    ghWks = AComOpenWorkingSet(
        0,
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
			printf("\tPIPE SERVER : %s ... ", gtszFileName[dwIndex]);
            gthInst[dwIndex] = AComOpenPipeServerInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtdwType[dwIndex] /* dwInstUsrKey */,
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
			printf("\tPIPE CLIENT : %s ... ", gtszFileName[dwIndex]);
            gthInst[dwIndex] = AComOpenPipeClientInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtdwType[dwIndex] /* dwInstUsrKey */,
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
			printf("\tMAIL SERVER : %s ... ", gtszFileName[dwIndex]);
            gthInst[dwIndex] = AComOpenMailslotServerInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtdwType[dwIndex] /* dwInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackRecv );
        }
        else if ( gtdwType[dwIndex] == MAIN_MAILCLIENT )
        {
			printf("\tMAIL CLIENT : %s ... \n", gtszFileName[dwIndex]);
            gthInst[dwIndex] = AComOpenMailslotClientInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtdwType[dwIndex] /* dwInstUsrKey */,
                gtdwTimeToReconnect[dwIndex],
                gtdwQueueSize[dwIndex],
                gtdwMaxMessageSize[dwIndex],
                MainCallbackCnx,
                MainCallbackDcnx,
                MainCallbackSent );
        }
        else if ( gtdwType[dwIndex] == MAIN_TCPSERVER )
        {
			printf("\tTCP SERVER : Port %u ... ", (DWORD)gtwPort[dwIndex]);

            gthInst[dwIndex] = AComOpenTcpServerInstance(
                ghWks,
                gtszFileName[dwIndex],
                gtwPort[dwIndex],
                gtdwType[dwIndex] /* dwInstUsrKey */,
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
                gtdwType[dwIndex] /* dwInstUsrKey */,
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
                gtdwType[dwIndex] /* dwInstUsrKey */,
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
