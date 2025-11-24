/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_timer.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions internes de gestion des timers.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dmem.h>
#include <acom_block.h>

#define LOC_DEF
#include <acom_timer.h>
#undef LOC_DEF

#include <acom_dbg.h>

#include <memclass.h>

// local private variables

PRIVATE DWORD WINAPI AComTimerApcThread( LPVOID param );

#ifdef _WIN64
PRIVATE void APIENTRY AComTimerApcSetTimer( ULONG_PTR dwTimer );
#else
PRIVATE void APIENTRY AComTimerApcSetTimer( DWORD dwTimer );
#endif

PRIVATE void APIENTRY AComTimerApcTimeOut(
                        DWORD dwTimer,
                        DWORD dwTimerLowValue,
                        DWORD dwTimerHighValue);



PRIVATE DWORD               gdwTimerId = 0;
PRIVATE DWORD               gdwThreadCount = 0;
PRIVATE HANDLE              ghTimerThread = 0;
PRIVATE ACOM_TIMER_DATA   * gpsTimers = NULL;
PRIVATE CRITICAL_SECTION    gsTimerProtect;



PROTECTED BOOL WINAPI AComTimerStartThread()
{
    DWORD dwThreadId;

    __try
    {
        EnterCriticalSection( &gsGlobalProtect );
   
        if ( gdwThreadCount == 0 )
            ghTimerThread = CreateThread (NULL, 0, AComTimerApcThread, NULL, 0, &dwThreadId );

        if ( ghTimerThread != NULL )
        {
            if ( gdwThreadCount == 0 )
                InitializeCriticalSection( &gsTimerProtect );
            gdwThreadCount ++;
        }
    }
    __finally
    {
        LeaveCriticalSection( &gsGlobalProtect );
//        return ( gdwThreadCount > 0 );
    }

    return ( gdwThreadCount > 0 );
}




PROTECTED void WINAPI AComTimerStopThread()
{
    ACOM_TIMER_DATA   * psTimers;

    __try
    {
        EnterCriticalSection( &gsGlobalProtect );
   
        if ( gdwThreadCount == 0 )
            __leave;

        gdwThreadCount --;
        if ( gdwThreadCount > 0 )
            __leave;

        if ( QueueUserAPC( AComTimerApcSetTimer, ghTimerThread, 0 ) )
        {
            if ( WaitForSingleObject( ghTimerThread, 10000 ) != NO_ERROR )
                TerminateThread( ghTimerThread, 0xDEADBEEF );
        }
        else
            TerminateThread( ghTimerThread, 0xDEADBEEF );
        CloseHandle( ghTimerThread );
        ghTimerThread = NULL;

        EnterCriticalSection( &gsTimerProtect );
        while ( gpsTimers != NULL )
        {
            psTimers = gpsTimers;
            gpsTimers = psTimers->psNext;

            CloseHandle( psTimers->hTimer );
            DMEM_FREE( psTimers );
        }
        LeaveCriticalSection( &gsTimerProtect );
        DeleteCriticalSection( &gsTimerProtect );
    }
    __finally
    {
        LeaveCriticalSection( &gsGlobalProtect );
    }
}





PROTECTED ACOM_TIMER_ID AComTimerAddTimer(
                        DWORD               dwDelay,
                        HANDLE              hCompletion,
                        ACOM_CONNECTION   * psCnx,
                        ACOM_CNX_HANDLE     hCnx,
                        DWORD               dwType )
{
    ACOM_TIMER_DATA   * psTimer = NULL;
    DWORD               dwId = 0;
    BOOL                fSuccess = FALSE;

    __try
    {
        EnterCriticalSection( &gsGlobalProtect );

        if ( gdwThreadCount == 0 )
            __leave;

        //
        // On prépare tout d'abord la structure utilisée pour le timer.
        //
        DMEM_ZALLOC( psTimer );
        if ( psTimer == NULL )
            __leave;

        //
        // On recherche ensuite le prochain identifiant valide pour le timer
        //
        do
            gdwTimerId ++;
        while ( ( gdwTimerId == 0 ) || ( gdwTimerId == 0xFFFFFFFF ) );

        dwId = gdwTimerId;
        psTimer->dwId = gdwTimerId;
        psTimer->hCompletion = hCompletion;
        psTimer->psCnx = psCnx;
        psTimer->hCnx = hCnx;
        psTimer->dwType = dwType;
        psTimer->sDueTime.QuadPart = -(LONGLONG) dwDelay * 1000 * 10;
        psTimer->hTimer = CreateWaitableTimer( NULL, FALSE, NULL);
        if ( psTimer->hTimer == NULL )
            __leave;

        //
        // Puis on empile l'APC qui va armer le timer.
        //
        if ( ! QueueUserAPC( AComTimerApcSetTimer, ghTimerThread, psTimer->dwId ) )
            __leave;

        //
        // C'est tout bon, on peut ajouter le timer dans la liste
        //
        EnterCriticalSection( &gsTimerProtect );
        psTimer->psNext = gpsTimers;
        gpsTimers = psTimer;
        LeaveCriticalSection( &gsTimerProtect );

        fSuccess = TRUE;
    }
    __finally
    {
        LeaveCriticalSection( &gsGlobalProtect );

        if ( ! fSuccess )
        {
            if ( psTimer != NULL )
            {
                if ( psTimer->hTimer != NULL )
                    CloseHandle( psTimer->hTimer );
                DMEM_FREE( psTimer );
            }
 
            dwId = 0;
        }

        //
        // Forcer une commutation noyau hors section critique
        // pour faciliter l'exécution de l'APC
        //
        if ( dwId != 0 )
            Sleep( 0 );

//        return dwId;
    }

    return dwId;
}




PROTECTED void AComTimerRemoveTimer( 
                        ACOM_TIMER_ID       dwId )
{
    ACOM_TIMER_DATA   * psScan;
    ACOM_TIMER_DATA   * psPrev;

    __try
    {
        EnterCriticalSection( &gsGlobalProtect );

        if ( gdwThreadCount == 0 )
            __leave;

        EnterCriticalSection( &gsTimerProtect );
    
        psPrev = NULL;
        psScan = gpsTimers;

        while ( psScan != NULL )
        {
            if ( psScan->dwId == dwId )
            {
                if ( psPrev != NULL )
                    psPrev->psNext = psScan->psNext;
                else
                    gpsTimers = psScan->psNext;
                break;
            }

            psPrev = psScan;
            psScan = psScan->psNext;
        }

        if ( psScan != NULL )
        {
            CancelWaitableTimer( psScan->hTimer );
            CloseHandle( psScan->hTimer );
            DMEM_FREE( psScan );
        }

        LeaveCriticalSection( &gsTimerProtect );
    }
    __finally
    {
        LeaveCriticalSection( &gsGlobalProtect );
    }
}





PRIVATE DWORD WINAPI AComTimerApcThread( LPVOID param )
{
   while (1)
      SleepEx(INFINITE, TRUE);

   return 0;
} 




#ifdef _WIN64
PRIVATE void APIENTRY AComTimerApcSetTimer( ULONG_PTR dwTimer )
#else
PRIVATE void APIENTRY AComTimerApcSetTimer( DWORD dwTimer )
#endif
{
    ACOM_TIMER_DATA * psTimer;

    if ( dwTimer == 0 )
    {
        ExitThread( 0 );
        return;
    }
    else
    {
        EnterCriticalSection( &gsTimerProtect );

        psTimer = gpsTimers;
        while ( psTimer != NULL )
        {
            if ( psTimer->dwId == dwTimer )
                break;
            psTimer = psTimer->psNext;
        }

        if ( psTimer != NULL )
            SetWaitableTimer( psTimer->hTimer, 
                              &psTimer->sDueTime,
                              0,
                              (PTIMERAPCROUTINE)AComTimerApcTimeOut, 
                              (LPVOID)dwTimer, 
                              FALSE );

        LeaveCriticalSection( &gsTimerProtect );
    }
}





PRIVATE void APIENTRY AComTimerApcTimeOut(
                        DWORD dwTimer,
                        DWORD dwTimerLowValue,
                        DWORD dwTimerHighValue)
{
    ACOM_TIMER_DATA   * psScan;
    ACOM_TIMER_DATA   * psPrev;

    EnterCriticalSection( &gsTimerProtect );

    psPrev = NULL;
    psScan = gpsTimers;

    while ( psScan != NULL )
    {
        if ( psScan->dwId == dwTimer )
        {
            if ( psPrev != NULL )
                psPrev->psNext = psScan->psNext;
            else
                gpsTimers = psScan->psNext;

            break;
        }

        psPrev = psScan;
        psScan = psScan->psNext;
    }

    LeaveCriticalSection( &gsTimerProtect );

    if ( psScan != NULL )
    {
        AComBlockPost(  psScan->hCompletion,
                        psScan->dwType,
                        psScan->psCnx,
                        psScan->hCnx,
                        0,
                        0,
                        FALSE,
                        0,
                        NULL,
                        0,
                        NULL );
        CloseHandle( psScan->hTimer );
        DMEM_FREE( psScan );
    }
}




