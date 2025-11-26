/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TIMER
 * FILE       : TIMER_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for TIMER
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
#include <conio.h>

#include <CSR_TIMER.h>

#define LOC_DEF
#include <TIMER_test.h>
#undef LOC_DEF

#include <memclass.h>

#define NB_TIMERS   30
#define NB_COUNT    10

PRIVATE DWORD MyGetTickCount()
{
    ULONGLONG ullFreq;
    ULONGLONG ullCount;

    //if ( QueryPerformanceFrequency( (LARGE_INTEGER*)&ullFreq ) &&
    //     QueryPerformanceCounter( (LARGE_INTEGER*)&ullCount ) )
    //{
    //    ullCount = ( ullCount * (ULONGLONG)1000 ) / ullFreq;
    //    return (DWORD)ullCount;
    //}
    //else
        return GetTickCount();
}



PROTECTED int __cdecl main(
    IN int iArgc,
    IN char ** ppcArgc )
{
    int iErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwErr;
    DWORD dwResult;
    DWORD dwFlags;
    DWORD dwCounter;
    DWORD dwDelay;
    DWORD dwTick;
    TIMER_INSTANCE * tpsTimer[NB_TIMERS];
    DWORD            tdwCounters[NB_TIMERS];
    DWORD            tdwTick[NB_TIMERS];
    DWORD            tdwDelay[NB_TIMERS];
    HANDLE           thWait[NB_TIMERS];

    __try
    {
        while ( ! _kbhit() )
        {
            dwDelay = 10;

            for ( dwIndex = 0 ; dwIndex < NB_TIMERS ; dwIndex ++ )
            {
                printf( "\nCreation timer %u", dwIndex );

                dwFlags = ( ( dwIndex < ( NB_TIMERS / 2 ) ) ? TIMER_TYPE_WAITABLE : TIMER_TYPE_SPECIFIC_THREAD )
                          | TIMER_MAKE_RESOLUTION( 5 );

                // dwErr = TimerOpenEx( dwFlags, &tpsTimer[dwIndex] );
                // dwErr = TimerOpen( &tpsTimer[dwIndex] );
                dwErr = TimerOpenEx(TIMER_TYPE_WAITABLE | TIMER_MAKE_RESOLUTION( 1 ), &tpsTimer[dwIndex] );
                if ( dwErr != NO_ERROR )
                {
                    printf( "\nErreur %u : TimerOpenEx", dwErr );
                    __leave;
                }

                tdwCounters[dwIndex] = 0;
                thWait[dwIndex] = TimerGetWaitableHandle( tpsTimer[dwIndex] );
                tdwDelay[dwIndex] = dwDelay;

                dwDelay = ( ( dwDelay * dwIndex + 11 ) % 100 ) + 3;
            }

            Sleep( 500 );

            for ( dwIndex = 0 ; dwIndex < NB_TIMERS ; dwIndex ++ )
            {
                printf( "\nArmement timer %u", dwIndex );
                tdwTick[dwIndex] = MyGetTickCount();
                dwErr = TimerActivate( tpsTimer[dwIndex], tdwDelay[dwIndex] );
                if ( dwErr != NO_ERROR )
                {
                    printf( "\nErreur %u : TimerActivate/1", dwErr );
                    __leave;
                }
            }

            for ( dwIndex = 0 ; dwIndex < NB_TIMERS ; dwIndex ++ )
            {
                printf( "\nAnnulation timer %u", dwIndex );
                dwErr = TimerCancel( tpsTimer[dwIndex] );
                if ( dwErr != NO_ERROR )
                {
                    printf( "\nErreur %u : TimerCancel", dwErr );
                    __leave;
                }
            }

            for ( dwIndex = 0 ; dwIndex < NB_TIMERS ; dwIndex ++ )
            {
                printf( "\nArmement timer %u", dwIndex );
                tdwTick[dwIndex] = MyGetTickCount();
                dwErr = TimerActivate( tpsTimer[dwIndex], tdwDelay[dwIndex] );
                if ( dwErr != NO_ERROR )
                {
                    printf( "\nErreur %u : TimerActivate/2", dwErr );
                    __leave;
                }
            }

            dwCounter = 0;

            while ( TRUE )
            {
                dwResult = WaitForMultipleObjects( NB_TIMERS, thWait, FALSE, INFINITE );
                if ( ( dwResult >= WAIT_OBJECT_0 ) &&
                     ( dwResult < ( WAIT_OBJECT_0 + NB_TIMERS ) ) )
                {
                    dwIndex = dwResult - WAIT_OBJECT_0;
                    dwTick = MyGetTickCount();
                    tdwCounters[dwIndex] ++;

                    printf( "\n -> Timer %u - n=%u : Theorique=%u / Reel=%u",
                            dwIndex,
                            tdwCounters[dwIndex],
                            tdwDelay[dwIndex],
                            dwTick - tdwTick[dwIndex] );

                    if ( ( dwTick - tdwTick[dwIndex] ) < tdwDelay[dwIndex] )
                        printf( " ### TROP COURT ### " );
                    else if ( ( dwTick - tdwTick[dwIndex] ) > ( tdwDelay[dwIndex] + 16 ) ) //if 16ms is timer resolution
                        printf( " ### TROP LONG ### " );

                    dwErr = TimerAcknowledge( tpsTimer[dwIndex] );
                    if ( dwErr != NO_ERROR )
                    {
                        printf( "\nErreur %u : TimerAcknowledge", dwErr );
                        __leave;
                    }

                    tdwTick[dwIndex] = dwTick;

                    if ( tdwCounters[dwIndex] < NB_COUNT )
                    {
                        printf( "\nRe-armement timer %u", dwIndex );
                        dwErr = TimerActivate( tpsTimer[dwIndex], tdwDelay[dwIndex] );
                        if ( dwErr != NO_ERROR )
                        {
                            printf( "\nErreur %u : TimerActivate/3", dwErr );
                            __leave;
                        }
                    }
                    else
                    {
                        printf( "\nCompte atteint timer %u", dwIndex );
                    }

                    dwCounter ++;
                    if ( dwCounter == ( NB_COUNT * NB_TIMERS ) )
                        break;
                }
                else
                {
                    dwErr = GetLastError();
                    printf( "\nErreur %u : WaitForMultipleObjects > %u", dwErr, dwResult );
                    __leave;
                }
            }

            for ( dwIndex = 0 ; dwIndex < NB_TIMERS ; dwIndex ++ )
            {
                printf( "\nFermeture timer %u", dwIndex );
                TimerClose( tpsTimer[dwIndex] );
            }
        }

        printf( "\nFin" );
    }
    __finally
    {
    }
	return iErr;
}
